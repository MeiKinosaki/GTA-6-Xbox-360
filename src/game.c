#include "gta6/game.h"

#define PLAYER_ACCEL 55
#define PLAYER_FRICTION 18
#define PLAYER_MAX_SPEED 320
#define VEHICLE_ACCEL 38
#define VEHICLE_FRICTION 9
#define VEHICLE_MAX_SPEED 680
#define VEHICLE_INTERACT_RADIUS 1800

static int32_t clamp_i32(int32_t value, int32_t low, int32_t high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static int32_t approach_zero(int32_t value, int32_t amount)
{
    if (value > 0) return value > amount ? value - amount : 0;
    if (value < 0) return value < -amount ? value + amount : 0;
    return 0;
}

static int32_t axis_value(uint8_t positive, uint8_t negative)
{
    return (positive != 0u) - (negative != 0u);
}

static void move_entity(int32_t *x, int32_t *z, int32_t *vx, int32_t *vz,
                        const GameInput *input, int32_t accel,
                        int32_t friction, int32_t max_speed,
                        const GameState *game)
{
    const int32_t x_axis = axis_value(input->right, input->left);
    const int32_t z_axis = axis_value(input->forward, input->backward);

    if (x_axis != 0) *vx += x_axis * accel;
    else *vx = approach_zero(*vx, friction);

    if (z_axis != 0) *vz += z_axis * accel;
    else *vz = approach_zero(*vz, friction);

    *vx = clamp_i32(*vx, -max_speed, max_speed);
    *vz = clamp_i32(*vz, -max_speed, max_speed);

    *x += *vx;
    *z += *vz;

    if (*x < game->min_x || *x > game->max_x) {
        *x = clamp_i32(*x, game->min_x, game->max_x);
        *vx = 0;
    }
    if (*z < game->min_z || *z > game->max_z) {
        *z = clamp_i32(*z, game->min_z, game->max_z);
        *vz = 0;
    }
}

void game_init(GameState *game)
{
    uint32_t i;
    if (game == 0) return;

    game->min_x = -50000;
    game->max_x = 50000;
    game->min_z = -50000;
    game->max_z = 50000;

    game->player.x = 0;
    game->player.z = 0;
    game->player.vx = 0;
    game->player.vz = 0;
    game->player.vehicle_index = -1;

    game->vehicle_count = 2;
    game->simulation_ticks = 0;

    for (i = 0; i < GAME_MAX_VEHICLES; ++i) {
        game->vehicles[i].x = 0;
        game->vehicles[i].z = 0;
        game->vehicles[i].vx = 0;
        game->vehicles[i].vz = 0;
        game->vehicles[i].occupied = 0;
    }

    /* Temporary test-district parking positions. */
    game->vehicles[0].x = 4000;
    game->vehicles[0].z = 0;
    game->vehicles[1].x = -6000;
    game->vehicles[1].z = 2500;
}

int game_interact(GameState *game)
{
    uint32_t i;
    int32_t best_index = -1;
    int64_t best_distance = (int64_t)VEHICLE_INTERACT_RADIUS *
                            VEHICLE_INTERACT_RADIUS;

    if (game == 0) return 0;

    if (game->player.vehicle_index >= 0) {
        GameVehicle *vehicle = &game->vehicles[(uint8_t)game->player.vehicle_index];
        vehicle->occupied = 0;
        game->player.vehicle_index = -1;
        game->player.x = clamp_i32(vehicle->x + 1200, game->min_x, game->max_x);
        game->player.z = vehicle->z;
        game->player.vx = 0;
        game->player.vz = 0;
        return 1;
    }

    for (i = 0; i < game->vehicle_count; ++i) {
        const int64_t dx = (int64_t)game->vehicles[i].x - game->player.x;
        const int64_t dz = (int64_t)game->vehicles[i].z - game->player.z;
        const int64_t distance = dx * dx + dz * dz;
        if (!game->vehicles[i].occupied && distance <= best_distance) {
            best_distance = distance;
            best_index = (int32_t)i;
        }
    }

    if (best_index < 0) return 0;

    game->player.vehicle_index = (int8_t)best_index;
    game->player.vx = 0;
    game->player.vz = 0;
    game->vehicles[(uint8_t)best_index].occupied = 1;
    return 1;
}

void game_tick(GameState *game, const GameInput *input)
{
    GameInput neutral = {0, 0, 0, 0, 0};
    const GameInput *controls = input != 0 ? input : &neutral;

    if (game == 0) return;

    if (controls->interact_pressed) (void)game_interact(game);

    if (game->player.vehicle_index >= 0) {
        GameVehicle *vehicle = &game->vehicles[(uint8_t)game->player.vehicle_index];
        move_entity(&vehicle->x, &vehicle->z, &vehicle->vx, &vehicle->vz,
                    controls, VEHICLE_ACCEL, VEHICLE_FRICTION,
                    VEHICLE_MAX_SPEED, game);
        game->player.x = vehicle->x;
        game->player.z = vehicle->z;
    } else {
        move_entity(&game->player.x, &game->player.z,
                    &game->player.vx, &game->player.vz,
                    controls, PLAYER_ACCEL, PLAYER_FRICTION,
                    PLAYER_MAX_SPEED, game);
    }

    ++game->simulation_ticks;
}

int game_validate(const GameState *game)
{
    uint32_t i;
    if (game == 0 || game->vehicle_count > GAME_MAX_VEHICLES) return 0;
    if (game->player.x < game->min_x || game->player.x > game->max_x) return 0;
    if (game->player.z < game->min_z || game->player.z > game->max_z) return 0;
    if (game->player.vehicle_index >= (int8_t)game->vehicle_count) return 0;

    if (game->player.vehicle_index >= 0 &&
        !game->vehicles[(uint8_t)game->player.vehicle_index].occupied) return 0;

    for (i = 0; i < game->vehicle_count; ++i) {
        const GameVehicle *vehicle = &game->vehicles[i];
        if (vehicle->x < game->min_x || vehicle->x > game->max_x) return 0;
        if (vehicle->z < game->min_z || vehicle->z > game->max_z) return 0;
        if (vehicle->occupied &&
            game->player.vehicle_index != (int8_t)i) return 0;
    }
    return 1;
}
