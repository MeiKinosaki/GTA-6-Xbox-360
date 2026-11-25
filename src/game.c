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
    if (ax) *vx += ax * accel; else *vx = approach_zero(*vx, friction);
    if (az) *vz += az * accel; else *vz = approach_zero(*vz, friction);
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
        g->traffic[i].turn_direction = 1; g->traffic[i].active = 0;
    }
    g->traffic[0].x = -12000; g->traffic[0].z = -8000;
    g->traffic[0].min_z = -18000; g->traffic[0].max_z = 18000;
    g->traffic[0].speed = 24; g->traffic[0].min_x = -15000;
    g->traffic[0].max_x = -9000; g->traffic[0].turn_direction = 1; g->traffic[0].active = 1;
    g->traffic[1].x = 12000; g->traffic[1].z = 9000;
    g->traffic[1].min_z = -16000; g->traffic[1].max_z = 16000;
    g->traffic[1].speed = 20; g->traffic[1].direction = -1;
    g->traffic[1].min_x = 9000; g->traffic[1].max_x = 15000;
    g->traffic[1].turn_direction = -1; g->traffic[1].active = 1;
    g->traffic_lights[0].x = -12000; g->traffic_lights[0].stop_z = -6000;
    g->traffic_lights[0].phase_ticks = 0; g->traffic_lights[0].active = 1;
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
int game_interact(GameState *g)
{
    uint32_t i;
    int32_t best = -1;
    int64_t best_d = (int64_t)VEHICLE_INTERACT_RADIUS * VEHICLE_INTERACT_RADIUS;
    if (!g) return 0;
    if (g->player.vehicle_index >= 0) {
        GameVehicle *v = &g->vehicles[(uint8_t)g->player.vehicle_index];
        v->occupied = 0; g->player.vehicle_index = -1;
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
static int traffic_path_blocked(const GameState *g, const GameTrafficVehicle *v, int32_t next_z)
{
    uint32_t i;
    for (i = 0; i < g->obstacle_count; ++i) {
        const GameObstacle *o = &g->obstacles[i];
        if (v->x + VEHICLE_HALF_SIZE > o->min_x &&
            v->x - VEHICLE_HALF_SIZE < o->max_x &&
            next_z + VEHICLE_HALF_SIZE > o->min_z &&
            next_z - VEHICLE_HALF_SIZE < o->max_z) return 1;
    }
    for (i = 0; i < g->traffic_count; ++i) {
        const GameTrafficVehicle *other = &g->traffic[i];
        if (other == v || !other->active) continue;
        if (v->x + VEHICLE_HALF_SIZE > other->x - VEHICLE_HALF_SIZE &&
            v->x - VEHICLE_HALF_SIZE < other->x + VEHICLE_HALF_SIZE &&
            next_z + VEHICLE_HALF_SIZE > other->z - VEHICLE_HALF_SIZE &&
            next_z - VEHICLE_HALF_SIZE < other->z + VEHICLE_HALF_SIZE) return 1;
    }
    if (v->x + VEHICLE_HALF_SIZE > g->player.x - PLAYER_HALF_SIZE &&
        v->x - VEHICLE_HALF_SIZE < g->player.x + PLAYER_HALF_SIZE &&
        next_z + VEHICLE_HALF_SIZE > g->player.z - PLAYER_HALF_SIZE &&
        next_z - VEHICLE_HALF_SIZE < g->player.z + PLAYER_HALF_SIZE) return 1;
    for (i = 0; i < g->vehicle_count; ++i) {
        const GameVehicle *parked = &g->vehicles[i];
        if (parked->occupied) continue;
        if (v->x + VEHICLE_HALF_SIZE > parked->x - VEHICLE_HALF_SIZE &&
            v->x - VEHICLE_HALF_SIZE < parked->x + VEHICLE_HALF_SIZE &&
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
        int32_t next_z;
        if (!v->active || v->speed <= 0 || v->min_z >= v->max_z) continue;
        next_z = v->z + v->speed * v->direction;
        if (next_z >= v->max_z) {
            next_z = v->max_z;
            v->direction = -1;
            v->x += 2500 * v->turn_direction;
            if (v->x >= v->max_x || v->x <= v->min_x)
                v->turn_direction = (int8_t)-v->turn_direction;
        } else if (next_z <= v->min_z) {
            next_z = v->min_z;
            v->direction = 1;
            v->x += 2500 * v->turn_direction;
            if (v->x >= v->max_x || v->x <= v->min_x)
                v->turn_direction = (int8_t)-v->turn_direction;
        }
        if (traffic_path_blocked(g, v, next_z) || traffic_light_blocks(g, v, next_z)) continue;
        v->z = next_z;
    }
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
    ++g->simulation_ticks;
}
int game_validate(const GameState *g)
{
    uint32_t i;
    if (!g || g->vehicle_count > GAME_MAX_VEHICLES || g->obstacle_count > GAME_MAX_OBSTACLES ||
        g->pedestrian_count > GAME_MAX_PEDESTRIANS ||
        g->traffic_count > GAME_MAX_TRAFFIC ||
        g->traffic_light_count > GAME_MAX_TRAFFIC_LIGHTS) return 0;
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
        if (v->active && (v->min_z > v->max_z || v->z < v->min_z || v->z > v->max_z || v->speed < 0 || v->min_x > v->max_x || v->x < v->min_x || v->x > v->max_x)) return 0;
    }
    for (i = 0; i < g->traffic_light_count; ++i) {
        if (g->traffic_lights[i].phase_ticks >= 240u) return 0;
    }
    return 1;
}
