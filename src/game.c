#include "gta6/game.h"
#define PLAYER_ACCEL 55
#define PLAYER_FRICTION 18
#define PLAYER_MAX_SPEED 320
#define VEHICLE_ACCEL 38
#define VEHICLE_FRICTION 9
#define VEHICLE_MAX_SPEED 680
#define VEHICLE_INTERACT_RADIUS 1800
#define PLAYER_HALF_SIZE 250
#define VEHICLE_HALF_SIZE 650
#define CAMERA_SMOOTH_DIVISOR 4

static void emit_audio(GameState *g, uint8_t type, int32_t x, int32_t z)
{
    uint8_t slot;
    if (g->audio_count >= GAME_AUDIO_QUEUE_CAPACITY) {
        g->audio_read = (uint8_t)((g->audio_read + 1u) % GAME_AUDIO_QUEUE_CAPACITY);
        --g->audio_count;
    }
    slot = g->audio_write;
    g->audio_events[slot].type = type;
    g->audio_events[slot].x = x;
    g->audio_events[slot].z = z;
    g->audio_write = (uint8_t)((g->audio_write + 1u) % GAME_AUDIO_QUEUE_CAPACITY);
    ++g->audio_count;
}
int game_pop_audio_event(GameState *g, GameAudioEvent *event)
{
    if (!g || !event || g->audio_count == 0) return 0;
    *event = g->audio_events[g->audio_read];
    g->audio_read = (uint8_t)((g->audio_read + 1u) % GAME_AUDIO_QUEUE_CAPACITY);
    --g->audio_count;
    return 1;
}
static int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{ return v < lo ? lo : (v > hi ? hi : v); }
static int32_t approach_zero(int32_t v, int32_t amount)
{
    if (v > 0) return v > amount ? v - amount : 0;
    if (v < 0) return v < -amount ? v + amount : 0;
    return 0;
}
static int32_t axis_value(uint8_t positive, uint8_t negative)
{ return (positive != 0u) - (negative != 0u); }
static int overlaps(int32_t x, int32_t z, int32_t half, const GameObstacle *o)
{
    return x + half > o->min_x && x - half < o->max_x &&
           z + half > o->min_z && z - half < o->max_z;
}
static void resolve_obstacles(int32_t old_x, int32_t old_z,
 int32_t *x, int32_t *z, int32_t *vx, int32_t *vz,
 int32_t half, const GameState *g)
{
    uint32_t i;
    for (i = 0; i < g->obstacle_count; ++i) {
        const GameObstacle *o = &g->obstacles[i];
        if (overlaps(*x, *z, half, o)) {
            int32_t dx = *x - old_x;
            int32_t dz = *z - old_z;
            if (dx < 0) dx = -dx;
            if (dz < 0) dz = -dz;
            if (dx <= dz) {
                *x = old_x; *vx = 0;
                if (overlaps(*x, *z, half, o)) { *z = old_z; *vz = 0; }
            } else {
                *z = old_z; *vz = 0;
                if (overlaps(*x, *z, half, o)) { *x = old_x; *vx = 0; }
            }
        }
    }
}
static void move_entity(int32_t *x, int32_t *z, int32_t *vx, int32_t *vz,
 const GameInput *in, int32_t accel, int32_t friction, int32_t max_speed,
 int32_t half, const GameState *g)
{
    int32_t old_x = *x, old_z = *z;
    int32_t ax = axis_value(in->right, in->left);
    int32_t az = axis_value(in->forward, in->backward);
    /* Scale acceleration, not the -1/0/1 input, to preserve diagonal motion. */
    if (ax && az) {
        *vx += ax * (accel * 707 / 1000);
        *vz += az * (accel * 707 / 1000);
    } else {
        if (ax) *vx += ax * accel; else *vx = approach_zero(*vx, friction);
        if (az) *vz += az * accel; else *vz = approach_zero(*vz, friction);
    }
    *vx = clamp_i32(*vx, -max_speed, max_speed);
    *vz = clamp_i32(*vz, -max_speed, max_speed);
    *x += *vx; *z += *vz;
    if (*x < g->min_x + half || *x > g->max_x - half) {
        *x = clamp_i32(*x, g->min_x + half, g->max_x - half);
        *vx = 0;
    }
    if (*z < g->min_z + half || *z > g->max_z - half) {
        *z = clamp_i32(*z, g->min_z + half, g->max_z - half);
        *vz = 0;
    }
    resolve_obstacles(old_x, old_z, x, z, vx, vz, half, g);
}
static void update_camera(GameState *g)
{
    g->camera.target_x = g->player.x;
    g->camera.target_z = g->player.z;
    g->camera.x += (g->camera.target_x - g->camera.x) / CAMERA_SMOOTH_DIVISOR;
    g->camera.z += (g->camera.target_z - g->camera.z) / CAMERA_SMOOTH_DIVISOR;
}
void game_init(GameState *g)
{
    uint32_t i;
    if (!g) return;
    g->min_x = -50000; g->max_x = 50000;
    g->min_z = -50000; g->max_z = 50000;
    g->player.x = 0; g->player.z = 0;
    g->player.vx = 0; g->player.vz = 0; g->player.vehicle_index = -1;
    g->vehicle_count = 2; g->obstacle_count = 0; g->pedestrian_count = 2; g->traffic_count = 2; g->traffic_light_count = 1; g->simulation_ticks = 0;
    g->road_count = 0; g->building_count = 0; g->audio_read = 0; g->audio_write = 0; g->audio_count = 0;
    g->money = 0; g->health = 100; g->armor = 0; g->wanted_level = 0; g->wanted_ticks = 0; g->current_mission = 0;
    g->camera.x = 0; g->camera.z = 0; g->camera.target_x = 0; g->camera.target_z = 0;
    for (i = 0; i < GAME_MAX_VEHICLES; ++i) {
        g->vehicles[i].x = 0; g->vehicles[i].z = 0;
        g->vehicles[i].vx = 0; g->vehicles[i].vz = 0;
        g->vehicles[i].occupied = 0;
    }
    for (i = 0; i < GAME_MAX_OBSTACLES; ++i) {
        g->obstacles[i].min_x = 0; g->obstacles[i].max_x = 0;
        g->obstacles[i].min_z = 0; g->obstacles[i].max_z = 0;
    }
    g->vehicles[0].x = 4000;
    g->vehicles[1].x = -6000; g->vehicles[1].z = 2500;
    for (i = 0; i < GAME_MAX_PEDESTRIANS; ++i) {
        g->pedestrians[i].x = 0; g->pedestrians[i].z = 0;
        g->pedestrians[i].min_z = 0; g->pedestrians[i].max_z = 0;
        g->pedestrians[i].speed = 0; g->pedestrians[i].direction = 1;
        g->pedestrians[i].active = 0;
    }
    g->pedestrians[0].x = 5000; g->pedestrians[0].z = -3000;
    g->pedestrians[0].min_z = -5000; g->pedestrians[0].max_z = 5000;
    g->pedestrians[0].speed = 12; g->pedestrians[0].active = 1;
    g->pedestrians[1].x = -3500; g->pedestrians[1].z = 2000;
    g->pedestrians[1].min_z = -1000; g->pedestrians[1].max_z = 4500;
    g->pedestrians[1].speed = 9; g->pedestrians[1].direction = -1;
    g->pedestrians[1].active = 1;
    for (i = 0; i < GAME_MAX_TRAFFIC; ++i) {
        g->traffic[i].x = 0; g->traffic[i].z = 0;
        g->traffic[i].min_z = 0; g->traffic[i].max_z = 0;
        g->traffic[i].speed = 0; g->traffic[i].direction = 1;
        g->traffic[i].min_x = -50000; g->traffic[i].max_x = 50000;
        g->traffic[i].turn_direction = 1; g->traffic[i].axis = 0; g->traffic[i].active = 0;
    }
    g->traffic[0].x = -12000; g->traffic[0].z = -8000;
    g->traffic[0].min_z = -18000; g->traffic[0].max_z = 18000;
    g->traffic[0].speed = 24; g->traffic[0].min_x = -15000;
    g->traffic[0].max_x = -9000; g->traffic[0].turn_direction = 1; g->traffic[0].axis = 0; g->traffic[0].active = 1;
    g->traffic[1].x = 12000; g->traffic[1].z = 9000;
    g->traffic[1].min_z = -16000; g->traffic[1].max_z = 16000;
    g->traffic[1].speed = 20; g->traffic[1].direction = -1;
    g->traffic[1].min_x = 9000; g->traffic[1].max_x = 15000;
    g->traffic[1].turn_direction = -1; g->traffic[1].axis = 0; g->traffic[1].active = 1;
    {
        const GameRoad r[] = {
            {-30000, -10000, 30000, -10000, 6000, 0},
            {-30000, 10000, 30000, 10000, 6000, 0},
            {-10000, -30000, -10000, 30000, 6000, 1},
            {10000, -30000, 10000, 30000, 6000, 1}
        };
        const GameBuilding b[] = {
            {-20000, -20000, 7000, 7000, 12000, 1},
            {20000, -20000, 7000, 7000, 18000, 1},
            {-20000, 20000, 7000, 7000, 9000, 1},
            {20000, 20000, 7000, 7000, 15000, 1}
        };
        for (i = 0; i < 4; ++i) {
            (void)game_add_road(g, &r[i]);
            (void)game_add_building(g, &b[i]);
        }
    }
    g->traffic_lights[0].x = -12000; g->traffic_lights[0].stop_z = -6000;
    g->traffic_lights[0].phase_ticks = 0; g->traffic_lights[0].active = 1;
    for (i = 0; i < GAME_MAX_POLICE; ++i) {
        g->police[i].x = 0; g->police[i].z = 0;
        g->police[i].speed = 0; g->police[i].active = 0;
    }
    for (i = 0; i < GAME_MAX_MISSIONS; ++i) {
        g->missions[i].target_x = 0; g->missions[i].target_z = 0;
        g->missions[i].radius = 0; g->missions[i].reward = 0;
        g->missions[i].id = 0; g->missions[i].active = 0;
        g->missions[i].completed = 0;
    }
    for (i = 1; i < GAME_MAX_TRAFFIC_LIGHTS; ++i) {
        g->traffic_lights[i].x = 0; g->traffic_lights[i].stop_z = 0;
        g->traffic_lights[i].phase_ticks = 0; g->traffic_lights[i].active = 0;
    }
}
int game_add_obstacle(GameState *g, const GameObstacle *o)
{
    if (!g || !o || g->obstacle_count >= GAME_MAX_OBSTACLES ||
        o->min_x >= o->max_x || o->min_z >= o->max_z) return 0;
    g->obstacles[g->obstacle_count++] = *o;
    return 1;
}

int game_add_road(GameState *g, const GameRoad *r)
{
    if (!g || !r || g->road_count >= GAME_MAX_ROADS ||
        r->width <= 0 || r->axis > 1 ||
        (r->axis == 0 && (r->x1 >= r->x2 || r->z1 != r->z2)) ||
        (r->axis == 1 && (r->z1 >= r->z2 || r->x1 != r->x2))) return 0;
    g->roads[g->road_count++] = *r;
    return 1;
}
int game_add_building(GameState *g, const GameBuilding *b)
{
    GameObstacle o;
    if (!g || !b || !b->active || b->width <= 0 ||
        b->depth <= 0 || b->height <= 0 ||
        g->building_count >= GAME_MAX_BUILDINGS) return 0;
    o.min_x = b->x - b->width / 2;
    o.max_x = b->x + b->width / 2;
    o.min_z = b->z - b->depth / 2;
    o.max_z = b->z + b->depth / 2;
    if (!game_add_obstacle(g, &o)) return 0;
    g->buildings[g->building_count++] = *b;
    return 1;
}

int game_interact(GameState *g)
{
    uint32_t i;
    int32_t best = -1;
    int64_t best_d = (int64_t)VEHICLE_INTERACT_RADIUS * VEHICLE_INTERACT_RADIUS;
    if (!g) return 0;
    if (g->player.vehicle_index >= 0) {
        GameVehicle *v = &g->vehicles[(uint8_t)g->player.vehicle_index];
        v->occupied = 0; g->player.vehicle_index = -1;
        emit_audio(g, GAME_AUDIO_VEHICLE_EXIT, v->x, v->z);
        g->player.x = clamp_i32(v->x + 1200, g->min_x + PLAYER_HALF_SIZE, g->max_x - PLAYER_HALF_SIZE);
        g->player.z = v->z; g->player.vx = 0; g->player.vz = 0;
        return 1;
    }
    for (i = 0; i < g->vehicle_count; ++i) {
        int64_t dx = (int64_t)g->vehicles[i].x - g->player.x;
        int64_t dz = (int64_t)g->vehicles[i].z - g->player.z;
        int64_t d = dx * dx + dz * dz;
        if (!g->vehicles[i].occupied && d <= best_d) { best_d = d; best = (int32_t)i; }
    }
    if (best < 0) return 0;
    g->player.vehicle_index = (int8_t)best;
    g->player.vx = 0; g->player.vz = 0;
    g->vehicles[(uint8_t)best].occupied = 1;
    emit_audio(g, GAME_AUDIO_VEHICLE_ENTER, g->vehicles[(uint8_t)best].x, g->vehicles[(uint8_t)best].z);
    return 1;
}
static int pedestrian_path_blocked(const GameState *g, const GamePedestrian *p, int32_t next_z)
{
    uint32_t i;
    for (i = 0; i < g->obstacle_count; ++i) {
        const GameObstacle *o = &g->obstacles[i];
        if (p->x >= o->min_x && p->x <= o->max_x &&
            next_z >= o->min_z && next_z <= o->max_z) return 1;
    }
    return 0;
}
static void update_pedestrians(GameState *g)
{
    uint32_t i;
    for (i = 0; i < g->pedestrian_count; ++i) {
        GamePedestrian *p = &g->pedestrians[i];
        int32_t next_z;
        if (!p->active || p->speed <= 0 || p->min_z >= p->max_z) continue;
        next_z = p->z + p->speed * p->direction;
        if (next_z >= p->max_z) { next_z = p->max_z; p->direction = -1; }
        else if (next_z <= p->min_z) { next_z = p->min_z; p->direction = 1; }
        if (pedestrian_path_blocked(g, p, next_z)) {
            p->direction = (int8_t)-p->direction;
            next_z = p->z + p->speed * p->direction;
            if (next_z < p->min_z || next_z > p->max_z ||
                pedestrian_path_blocked(g, p, next_z)) continue;
        }
        p->z = next_z;
    }
}
static int traffic_path_blocked(const GameState *g, const GameTrafficVehicle *v, int32_t next_x, int32_t next_z)
{
    uint32_t i;
    for (i = 0; i < g->obstacle_count; ++i) {
        const GameObstacle *o = &g->obstacles[i];
        if (next_x + VEHICLE_HALF_SIZE > o->min_x &&
            next_x - VEHICLE_HALF_SIZE < o->max_x &&
            next_z + VEHICLE_HALF_SIZE > o->min_z &&
            next_z - VEHICLE_HALF_SIZE < o->max_z) return 1;
    }
    for (i = 0; i < g->traffic_count; ++i) {
        const GameTrafficVehicle *other = &g->traffic[i];
        if (other == v || !other->active) continue;
        if (next_x + VEHICLE_HALF_SIZE > other->x - VEHICLE_HALF_SIZE &&
            next_x - VEHICLE_HALF_SIZE < other->x + VEHICLE_HALF_SIZE &&
            next_z + VEHICLE_HALF_SIZE > other->z - VEHICLE_HALF_SIZE &&
            next_z - VEHICLE_HALF_SIZE < other->z + VEHICLE_HALF_SIZE) return 1;
    }
    if (next_x + VEHICLE_HALF_SIZE > g->player.x - PLAYER_HALF_SIZE &&
        next_x - VEHICLE_HALF_SIZE < g->player.x + PLAYER_HALF_SIZE &&
        next_z + VEHICLE_HALF_SIZE > g->player.z - PLAYER_HALF_SIZE &&
        next_z - VEHICLE_HALF_SIZE < g->player.z + PLAYER_HALF_SIZE) return 1;
    for (i = 0; i < g->vehicle_count; ++i) {
        const GameVehicle *parked = &g->vehicles[i];
        if (parked->occupied) continue;
        if (next_x + VEHICLE_HALF_SIZE > parked->x - VEHICLE_HALF_SIZE &&
            next_x - VEHICLE_HALF_SIZE < parked->x + VEHICLE_HALF_SIZE &&
            next_z + VEHICLE_HALF_SIZE > parked->z - VEHICLE_HALF_SIZE &&
            next_z - VEHICLE_HALF_SIZE < parked->z + VEHICLE_HALF_SIZE) return 1;
    }
    return 0;
}
static int traffic_light_blocks(const GameState *g, const GameTrafficVehicle *v, int32_t next_z)
{
    uint32_t i;
    for (i = 0; i < g->traffic_light_count; ++i) {
        const GameTrafficLight *light = &g->traffic_lights[i];
        int same_lane, approaching;
        int green_for_forward;
        if (!light->active) continue;
        same_lane = v->x + VEHICLE_HALF_SIZE > light->x - VEHICLE_HALF_SIZE &&
                    v->x - VEHICLE_HALF_SIZE < light->x + VEHICLE_HALF_SIZE;
        if (!same_lane) continue;
        approaching = v->direction > 0 ?
            (v->z + VEHICLE_HALF_SIZE < light->stop_z &&
             next_z + VEHICLE_HALF_SIZE >= light->stop_z) :
            (v->z - VEHICLE_HALF_SIZE > light->stop_z &&
             next_z - VEHICLE_HALF_SIZE <= light->stop_z);
        if (!approaching) continue;
        green_for_forward = light->phase_ticks < 120u;
        if ((v->direction > 0 && !green_for_forward) ||
            (v->direction < 0 && green_for_forward)) return 1;
    }
    return 0;
}
static void update_traffic_lights(GameState *g)
{
    uint32_t i;
    for (i = 0; i < g->traffic_light_count; ++i) {
        GameTrafficLight *light = &g->traffic_lights[i];
        if (light->active)
            light->phase_ticks = (uint16_t)((light->phase_ticks + 1u) % 240u);
    }
}
static void update_traffic(GameState *g)
{
    uint32_t i;
    for (i = 0; i < g->traffic_count; ++i) {
        GameTrafficVehicle *v = &g->traffic[i];
        int32_t next_x = v->x, next_z = v->z;
        if (!v->active || v->speed <= 0 || v->min_z >= v->max_z ||
            v->min_x >= v->max_x) continue;
        if (v->axis == 0) {
            next_z += v->speed * v->direction;
            if (next_z >= v->max_z) {
                next_z = v->max_z;
                v->axis = 1;
                v->direction = v->turn_direction;
            } else if (next_z <= v->min_z) {
                next_z = v->min_z;
                v->axis = 1;
                v->direction = (int8_t)-v->turn_direction;
            }
        } else {
            next_x += v->speed * v->direction;
            if (next_x >= v->max_x) {
                next_x = v->max_x;
                v->axis = 0;
                v->direction = -1;
            } else if (next_x <= v->min_x) {
                next_x = v->min_x;
                v->axis = 0;
                v->direction = 1;
            }
        }
        if (traffic_path_blocked(g, v, next_x, next_z)) continue;
        if (v->axis == 0 && traffic_light_blocks(g, v, next_z)) continue;
        v->x = next_x;
        v->z = next_z;
    }
}

int game_start_mission(GameState *g, uint16_t id, int32_t x, int32_t z, int32_t radius, int32_t reward)
{
    uint32_t i;
    if (!g || radius <= 0 || reward < 0) return 0;
    for (i = 0; i < GAME_MAX_MISSIONS; ++i) {
        GameMission *m = &g->missions[i];
        if (!m->active && !m->completed) {
            m->id = id; m->target_x = x; m->target_z = z;
            m->radius = radius; m->reward = reward;
            m->active = 1; m->completed = 0;
            g->current_mission = (uint8_t)i;
            return 1;
        }
    }
    return 0;
}
int game_raise_wanted(GameState *g, uint8_t stars)
{
    uint32_t i;
    if (!g || stars == 0) return 0;
    if (stars > 5) stars = 5;
    if (g->wanted_level + stars > 5) g->wanted_level = 5;
    else g->wanted_level = (uint8_t)(g->wanted_level + stars);
    g->wanted_ticks = 0;
    emit_audio(g, GAME_AUDIO_WANTED_RAISED, g->player.x, g->player.z);
    for (i = 0; i < GAME_MAX_POLICE && i < g->wanted_level; ++i) {
        if (!g->police[i].active) {
            g->police[i].x = g->player.x + (int32_t)(i + 1u) * 6000;
            g->police[i].z = g->player.z - 6000;
            g->police[i].speed = 45 + (int32_t)g->wanted_level * 8;
            g->police[i].active = 1;
        }
    }
    return 1;
}
int game_complete_mission(GameState *g)
{
    GameMission *m;
    int64_t dx, dz;
    if (!g || g->current_mission >= GAME_MAX_MISSIONS) return 0;
    m = &g->missions[g->current_mission];
    if (!m->active || m->completed) return 0;
    dx = (int64_t)g->player.x - m->target_x;
    dz = (int64_t)g->player.z - m->target_z;
    if (dx * dx + dz * dz > (int64_t)m->radius * m->radius) return 0;
    m->active = 0; m->completed = 1;
    emit_audio(g, GAME_AUDIO_MISSION_COMPLETE, g->player.x, g->player.z);
    if (g->money <= INT32_MAX - m->reward) g->money += m->reward;
    else g->money = INT32_MAX;
    return 1;
}
static void update_police(GameState *g)
{
    uint32_t i;
    int nearby = 0;
    for (i = 0; i < GAME_MAX_POLICE; ++i) {
        GamePoliceUnit *p = &g->police[i];
        int32_t dx, dz;
        if (!p->active) continue;
        dx = g->player.x - p->x; dz = g->player.z - p->z;
        if (dx > -1800 && dx < 1800 && dz > -1800 && dz < 1800) nearby = 1;
        if (dx > p->speed) p->x += p->speed;
        else if (dx < -p->speed) p->x -= p->speed;
        else p->x = g->player.x;
        if (dz > p->speed) p->z += p->speed;
        else if (dz < -p->speed) p->z -= p->speed;
        else p->z = g->player.z;
    }
    if (g->wanted_level) {
        if (!nearby && ++g->wanted_ticks >= 1800u) {
            --g->wanted_level; g->wanted_ticks = 0;
            for (i = 0; i < GAME_MAX_POLICE; ++i) g->police[i].active = 0;
        }
    }
}


GameInput game_input_from_pad(const GamePadState *pad)
{
    GameInput in = {0,0,0,0,0};
    const int16_t deadzone = 8000;
    if (!pad) return in;
    in.forward = (uint8_t)((pad->left_y > deadzone) || (pad->buttons & GAMEPAD_DPAD_UP));
    in.backward = (uint8_t)((pad->left_y < -deadzone) || (pad->buttons & GAMEPAD_DPAD_DOWN));
    in.right = (uint8_t)((pad->left_x > deadzone) || (pad->buttons & GAMEPAD_DPAD_RIGHT));
    in.left = (uint8_t)((pad->left_x < -deadzone) || (pad->buttons & GAMEPAD_DPAD_LEFT));
    in.interact_pressed = (uint8_t)((pad->buttons & GAMEPAD_A) != 0u);
    return in;
}

void game_tick(GameState *g, const GameInput *input)
{
    GameInput neutral = {0,0,0,0,0};
    const GameInput *in = input ? input : &neutral;
    if (!g) return;
    if (in->interact_pressed) (void)game_interact(g);
    if (g->player.vehicle_index >= 0) {
        GameVehicle *v = &g->vehicles[(uint8_t)g->player.vehicle_index];
        move_entity(&v->x, &v->z, &v->vx, &v->vz, in,
            VEHICLE_ACCEL, VEHICLE_FRICTION, VEHICLE_MAX_SPEED, VEHICLE_HALF_SIZE, g);
        g->player.x = v->x; g->player.z = v->z;
    } else {
        move_entity(&g->player.x, &g->player.z, &g->player.vx, &g->player.vz,
            in, PLAYER_ACCEL, PLAYER_FRICTION, PLAYER_MAX_SPEED, PLAYER_HALF_SIZE, g);
    }
    update_camera(g);
    update_pedestrians(g);
    update_traffic_lights(g);
    update_traffic(g);
    update_police(g);
    (void)game_complete_mission(g);
    ++g->simulation_ticks;
}
int game_validate(const GameState *g)
{
    uint32_t i;
    if (!g || g->audio_count > GAME_AUDIO_QUEUE_CAPACITY || g->audio_read >= GAME_AUDIO_QUEUE_CAPACITY || g->audio_write >= GAME_AUDIO_QUEUE_CAPACITY || g->wanted_level > 5 || g->health > 100 || g->armor > 100 || g->vehicle_count > GAME_MAX_VEHICLES || g->obstacle_count > GAME_MAX_OBSTACLES ||
        g->pedestrian_count > GAME_MAX_PEDESTRIANS ||
        g->traffic_count > GAME_MAX_TRAFFIC ||
        g->traffic_light_count > GAME_MAX_TRAFFIC_LIGHTS || g->road_count > GAME_MAX_ROADS || g->building_count > GAME_MAX_BUILDINGS) return 0;
    if (g->player.x < g->min_x || g->player.x > g->max_x ||
        g->player.z < g->min_z || g->player.z > g->max_z) return 0;
    if (g->player.vehicle_index >= (int8_t)g->vehicle_count) return 0;
    if (g->player.vehicle_index >= 0 &&
        !g->vehicles[(uint8_t)g->player.vehicle_index].occupied) return 0;
    for (i = 0; i < g->vehicle_count; ++i) {
        const GameVehicle *v = &g->vehicles[i];
        if (v->x < g->min_x || v->x > g->max_x ||
            v->z < g->min_z || v->z > g->max_z) return 0;
        if (v->occupied && g->player.vehicle_index != (int8_t)i) return 0;
    }
    for (i = 0; i < g->pedestrian_count; ++i) {
        const GamePedestrian *p = &g->pedestrians[i];
        if (p->active && (p->min_z > p->max_z || p->z < p->min_z || p->z > p->max_z || p->speed < 0)) return 0;
    }
    for (i = 0; i < g->traffic_count; ++i) {
        const GameTrafficVehicle *v = &g->traffic[i];
        if (v->active && (v->min_z >= v->max_z || v->z < v->min_z || v->z > v->max_z || v->speed < 0 || v->min_x >= v->max_x || v->x < v->min_x || v->x > v->max_x || v->axis > 1 || (v->direction != -1 && v->direction != 1) || (v->turn_direction != -1 && v->turn_direction != 1))) return 0;
    }
    for (i = 0; i < g->traffic_light_count; ++i) {
        if (g->traffic_lights[i].phase_ticks >= 240u) return 0;
    }
    for (i = 0; i < g->road_count; ++i)
        if (g->roads[i].width <= 0 || g->roads[i].axis > 1) return 0;
    for (i = 0; i < g->building_count; ++i)
        if (g->buildings[i].width <= 0 || g->buildings[i].depth <= 0 || g->buildings[i].height <= 0) return 0;
    for (i = 0; i < GAME_MAX_POLICE; ++i)
        if (g->police[i].active && g->police[i].speed <= 0) return 0;
    for (i = 0; i < GAME_MAX_MISSIONS; ++i)
        if (g->missions[i].active && (g->missions[i].radius <= 0 || g->missions[i].reward < 0)) return 0;
    return 1;
}
