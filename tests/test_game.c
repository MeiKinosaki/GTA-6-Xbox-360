#include "gta6/game.h"
#include <assert.h>
#include <stdio.h>

static void test_initial_state(void)
{
    GameState game;
    game_init(&game);
    assert(game_validate(&game));
    assert(game.player.vehicle_index == -1);
    assert(game.vehicle_count == 2);
}
static void test_player_movement_and_friction(void)
{
    GameState game;
    GameInput input = {1,0,0,0,0};
    int32_t moved;
    game_init(&game);
    game_tick(&game, &input);
    moved = game.player.z;
    assert(moved > 0);
    input.forward = 0;
    game_tick(&game, &input);
    assert(game.player.vz < 55);
    assert(game_validate(&game));
}
static void test_world_boundaries(void)
{
    GameState game;
    GameInput input = {0,0,0,1,0};
    game_init(&game);
    game.player.x = game.max_x - 250;
    game_tick(&game, &input);
    assert(game.player.x == game.max_x - 250);
    assert(game.player.vx == 0);
    assert(game_validate(&game));
}
static void test_enter_drive_and_exit_vehicle(void)
{
    GameState game;
    GameInput input = {0,0,0,0,1};
    int32_t initial_z;
    game_init(&game);
    game.player.x = game.vehicles[0].x;
    game.player.z = game.vehicles[0].z;
    game_tick(&game, &input);
    assert(game.player.vehicle_index == 0);
    assert(game.vehicles[0].occupied);
    assert(game_validate(&game));
    input.interact_pressed = 0;
    input.forward = 1;
    initial_z = game.vehicles[0].z;
    game_tick(&game, &input);
    assert(game.vehicles[0].z > initial_z);
    assert(game.player.z == game.vehicles[0].z);
    input.forward = 0;
    input.interact_pressed = 1;
    game_tick(&game, &input);
    assert(game.player.vehicle_index == -1);
    assert(!game.vehicles[0].occupied);
    assert(game.player.x == game.vehicles[0].x + 1200);
    assert(game_validate(&game));
}
static void test_cannot_enter_distant_or_occupied_vehicle(void)
{
    GameState game;
    game_init(&game);
    assert(!game_interact(&game));
    game.player.x = game.vehicles[0].x;
    game.player.z = game.vehicles[0].z;
    assert(game_interact(&game));
    game.player.vehicle_index = -1;
    assert(!game_interact(&game));
}
static void test_obstacle_collision_stops_player(void)
{
    GameState game;
    GameInput input = {1,0,0,0,0};
    GameObstacle wall = { -1000, 1000, 300, 1800 };
    game_init(&game);
    assert(game_add_obstacle(&game, &wall));
    game_tick(&game, &input);
    assert(game.player.z == 0);
    assert(game.player.vz == 0);
    assert(game_validate(&game));
}
static void test_camera_follows_smoothly(void)
{
    GameState game;
    GameInput input = {1,0,0,0,0};
    game_init(&game);
    game_tick(&game, &input);
    assert(game.camera.target_z == game.player.z);
    assert(game.camera.z > 0);
    assert(game.camera.z < game.camera.target_z);
}
static void test_obstacle_validation(void)
{
    GameState game;
    GameObstacle bad = { 10, 10, 0, 100 };
    game_init(&game);
    assert(!game_add_obstacle(&game, &bad));
}
static void test_traffic_yields_to_player_and_parked_vehicles(void)
{
    GameState game;
    int32_t traffic_z;
    game_init(&game);
    game.player.x = game.traffic[0].x;
    game.player.z = game.traffic[0].z + game.traffic[0].speed;
    traffic_z = game.traffic[0].z;
    game_tick(&game, 0);
    assert(game.traffic[0].z == traffic_z);

    game_init(&game);
    game.vehicles[2].x = game.traffic[0].x;
    game.vehicles[2].z = game.traffic[0].z + game.traffic[0].speed;
    game.vehicle_count = 3;
    traffic_z = game.traffic[0].z;
    game_tick(&game, 0);
    assert(game.traffic[0].z == traffic_z);
    assert(game_validate(&game));
}
static void test_traffic_avoids_adjacent_lane_overlap(void)
{
    GameState game;
    int32_t original_z;
    game_init(&game);
    game.traffic[1].x = game.traffic[0].x + 500;
    game.traffic[1].z = game.traffic[0].z + game.traffic[0].speed + 500;
    game.traffic[1].min_z = -20000;
    game.traffic[1].max_z = 20000;
    game.traffic[1].direction = -1;
    original_z = game.traffic[0].z;
    game_tick(&game, 0);
    assert(game.traffic[0].z == original_z);
    assert(game_validate(&game));
}
static void test_traffic_obeys_red_and_green_light_phases(void)
{
    GameState game;
    int32_t original_z;
    game_init(&game);
    game.traffic[0].z = -6670;
    game.traffic[0].direction = 1;
    game.traffic_lights[0].phase_ticks = 120;
    original_z = game.traffic[0].z;
    game_tick(&game, 0);
    assert(game.traffic[0].z == original_z);
    game.traffic_lights[0].phase_ticks = 0;
    game_tick(&game, 0);
    assert(game.traffic[0].z > original_z);
    assert(game_validate(&game));
}
static void test_traffic_changes_lane_at_route_endpoint(void)
{
    GameState game;
    int32_t original_x;
    game_init(&game);
    game.traffic[0].z = game.traffic[0].max_z - game.traffic[0].speed;
    game.traffic[0].direction = 1;
    original_x = game.traffic[0].x;
    game_tick(&game, 0);
    assert(game.traffic[0].z == game.traffic[0].max_z);
    assert(game.traffic[0].x != original_x);
    assert(game.traffic[0].x >= game.traffic[0].min_x);
    assert(game.traffic[0].x <= game.traffic[0].max_x);
    assert(game_validate(&game));
}
static void test_traffic_follows_patrol_route(void)
{
    GameState game;
    int32_t z;
    uint32_t i;
    game_init(&game);
    assert(game.traffic_count == 2);
    z = game.traffic[0].z;
    game_tick(&game, 0);
    assert(game.traffic[0].z > z);
    for (i = 0; i < 2000; ++i) game_tick(&game, 0);
    assert(game.traffic[0].z >= game.traffic[0].min_z);
    assert(game.traffic[0].z <= game.traffic[0].max_z);
    assert(game_validate(&game));
}
static void test_pedestrian_avoids_blocked_patrol_path(void)
{
    GameState game;
    GameObstacle wall = { 4500, 5500, -2990, -2500 };
    int32_t old_z;
    game_init(&game);
    assert(game_add_obstacle(&game, &wall));
    old_z = game.pedestrians[0].z;
    game_tick(&game, 0);
    assert(game.pedestrians[0].z < old_z);
    assert(game_validate(&game));
}
static void test_pedestrian_patrol_and_turnaround(void)
{
    GameState game;
    int32_t initial_z;
    uint32_t i;
    game_init(&game);
    assert(game.pedestrian_count == 2);
    initial_z = game.pedestrians[0].z;
    game_tick(&game, 0);
    assert(game.pedestrians[0].z > initial_z);
    for (i = 0; i < 1000; ++i) game_tick(&game, 0);
    assert(game.pedestrians[0].z >= game.pedestrians[0].min_z);
    assert(game.pedestrians[0].z <= game.pedestrians[0].max_z);
    assert(game_validate(&game));
}
int main(void)
{
    test_initial_state();
    test_player_movement_and_friction();
    test_world_boundaries();
    test_enter_drive_and_exit_vehicle();
    test_cannot_enter_distant_or_occupied_vehicle();
    test_obstacle_collision_stops_player();
    test_camera_follows_smoothly();
    test_obstacle_validation();
    test_pedestrian_patrol_and_turnaround();
    test_pedestrian_avoids_blocked_patrol_path();
    test_traffic_follows_patrol_route();
    test_traffic_changes_lane_at_route_endpoint();
    test_traffic_obeys_red_and_green_light_phases();
    test_traffic_avoids_adjacent_lane_overlap();
    test_traffic_yields_to_player_and_parked_vehicles();
    puts("Gameplay core tests: 15/15 passed");
    return 0;
}
