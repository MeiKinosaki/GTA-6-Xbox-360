#ifndef GTA6_GAME_H
#define GTA6_GAME_H
#include <stdint.h>
#define GAME_MAX_VEHICLES 4
#define GAME_MAX_OBSTACLES 8
#define GAME_MAX_PEDESTRIANS 4
typedef struct { int32_t min_x, max_x, min_z, max_z; } GameObstacle;
typedef struct { int32_t x, z, vx, vz; uint8_t occupied; } GameVehicle;
typedef struct { int32_t x, z, vx, vz; int8_t vehicle_index; } GamePlayer;
typedef struct { int32_t x, z, target_x, target_z; } GameCamera;
typedef struct { int32_t x, z, min_z, max_z, speed; int8_t direction; uint8_t active; } GamePedestrian;
typedef struct {
    int32_t min_x, max_x, min_z, max_z;
    GamePlayer player;
    GameVehicle vehicles[GAME_MAX_VEHICLES];
    GameObstacle obstacles[GAME_MAX_OBSTACLES];
    GamePedestrian pedestrians[GAME_MAX_PEDESTRIANS];
    GameCamera camera;
    uint8_t vehicle_count, obstacle_count, pedestrian_count;
    uint32_t simulation_ticks;
} GameState;
typedef struct {
    uint8_t forward, backward, left, right, interact_pressed;
} GameInput;
/* Coordinates are integer world units; 1000 units represent one metre. */
void game_init(GameState *game);
void game_tick(GameState *game, const GameInput *input);
int game_interact(GameState *game);
int game_add_obstacle(GameState *game, const GameObstacle *obstacle);
int game_validate(const GameState *game);
#endif
