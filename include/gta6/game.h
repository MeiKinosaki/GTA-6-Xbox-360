#ifndef GTA6_GAME_H
#define GTA6_GAME_H
#include <stdint.h>
#define GAME_MAX_VEHICLES 4
#define GAME_MAX_OBSTACLES 8
#define GAME_MAX_PEDESTRIANS 4
#define GAME_MAX_TRAFFIC 4
#define GAME_MAX_TRAFFIC_LIGHTS 4
#define GAME_MAX_POLICE 2
#define GAME_MAX_MISSIONS 4
#define GAME_MAX_ROADS 8
#define GAME_MAX_BUILDINGS 8
typedef struct { int32_t min_x, max_x, min_z, max_z; } GameObstacle;
typedef struct { int32_t x, z, vx, vz; uint8_t occupied; } GameVehicle;
typedef struct { int32_t x, z, vx, vz; int8_t vehicle_index; } GamePlayer;
typedef struct { int32_t x, z, target_x, target_z; } GameCamera;
typedef struct { int32_t x, z, min_z, max_z, speed; int8_t direction; uint8_t active; } GamePedestrian;
typedef struct { int32_t x, z, min_z, max_z, speed, min_x, max_x; int8_t direction, turn_direction, axis; uint8_t active; } GameTrafficVehicle;
typedef struct { int32_t x, stop_z; uint16_t phase_ticks; uint8_t active; } GameTrafficLight;
typedef struct { int32_t x1, z1, x2, z2, width; uint8_t axis; } GameRoad;
typedef struct { int32_t x, z, width, depth, height; uint8_t active; } GameBuilding;
typedef struct { int32_t x, z, speed; uint8_t active; } GamePoliceUnit;
typedef struct { int32_t target_x, target_z, radius, reward; uint16_t id; uint8_t active, completed; } GameMission;
typedef struct {
    int32_t min_x, max_x, min_z, max_z;
    GamePlayer player;
    GameVehicle vehicles[GAME_MAX_VEHICLES];
    GameObstacle obstacles[GAME_MAX_OBSTACLES];
    GamePedestrian pedestrians[GAME_MAX_PEDESTRIANS];
    GameTrafficVehicle traffic[GAME_MAX_TRAFFIC];
    GameTrafficLight traffic_lights[GAME_MAX_TRAFFIC_LIGHTS];
    GameRoad roads[GAME_MAX_ROADS];
    GameBuilding buildings[GAME_MAX_BUILDINGS];
    GamePoliceUnit police[GAME_MAX_POLICE];
    GameMission missions[GAME_MAX_MISSIONS];
    int32_t money;
    uint16_t wanted_ticks;
    uint8_t health, armor, wanted_level, current_mission;
    GameCamera camera;
    uint8_t vehicle_count, obstacle_count, pedestrian_count, traffic_count, traffic_light_count, road_count, building_count;
    uint32_t simulation_ticks;
} GameState;
typedef struct {
    uint8_t forward, backward, left, right, interact_pressed;
} GameInput;
typedef struct { int16_t left_x, left_y; uint16_t buttons; } GamePadState;
#define GAMEPAD_A 0x1000u
#define GAMEPAD_DPAD_UP 0x0001u
#define GAMEPAD_DPAD_DOWN 0x0002u
#define GAMEPAD_DPAD_LEFT 0x0004u
#define GAMEPAD_DPAD_RIGHT 0x0008u
/* Coordinates are integer world units; 1000 units represent one metre. */
void game_init(GameState *game);
void game_tick(GameState *game, const GameInput *input);
GameInput game_input_from_pad(const GamePadState *pad);
int game_interact(GameState *game);
int game_add_obstacle(GameState *game, const GameObstacle *obstacle);
int game_add_road(GameState *game, const GameRoad *road);
int game_add_building(GameState *game, const GameBuilding *building);
int game_validate(const GameState *game);
int game_start_mission(GameState *game, uint16_t mission_id, int32_t target_x, int32_t target_z, int32_t radius, int32_t reward);
int game_raise_wanted(GameState *game, uint8_t stars);
int game_complete_mission(GameState *game);
#endif
