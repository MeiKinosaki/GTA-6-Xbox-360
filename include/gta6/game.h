#ifndef GTA6_GAME_H
#define GTA6_GAME_H

#include <stdint.h>

#define GAME_MAX_VEHICLES 4

typedef struct {
    int32_t x;
    int32_t z;
    int32_t vx;
    int32_t vz;
    uint8_t occupied;
} GameVehicle;

typedef struct {
    int32_t x;
    int32_t z;
    int32_t vx;
    int32_t vz;
    int8_t vehicle_index;
} GamePlayer;

typedef struct {
    int32_t min_x;
    int32_t max_x;
    int32_t min_z;
    int32_t max_z;
    GamePlayer player;
    GameVehicle vehicles[GAME_MAX_VEHICLES];
    uint8_t vehicle_count;
    uint32_t simulation_ticks;
} GameState;

typedef struct {
    uint8_t forward;
    uint8_t backward;
    uint8_t left;
    uint8_t right;
    uint8_t interact_pressed;
} GameInput;

/* Coordinates are integer world units; 1000 units represent one metre. */
void game_init(GameState *game);
void game_tick(GameState *game, const GameInput *input);
int game_interact(GameState *game);
int game_validate(const GameState *game);

#endif
