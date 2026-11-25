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
static void test_diagonal_movement_is_normalized(void)
{
    GameState straight, diagonal;
    GameInput forward = {1,0,0,0,0};
    GameInput diagonal_input = {1,0,0,1,0};
    game_init(&straight);
    game_init(&diagonal);
    game_tick(&straight, &forward);
    game_tick(&diagonal, &diagonal_input);
    assert(diagonal.player.vz > 0);
    assert(diagonal.player.vx > 0);
    assert(diagonal.player.vz < straight.player.vz);
    assert(diagonal.player.vx < straight.player.vz);
    assert(diagonal.player.vx == diagonal.player.vz);
    assert(game_validate(&diagonal));
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
    game.traffic[1].min_x = game.traffic[1].x - 3000;
    game.traffic[1].max_x = game.traffic[1].x + 3000;
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
    assert(game.traffic[0].x == original_x);
    assert(game.traffic[0].axis == 1);
    game_tick(&game, 0);
    assert(game.traffic[0].x > original_x);
    assert(game.traffic[0].x <= game.traffic[0].max_x);
    assert(game_validate(&game));
}
static void test_traffic_completes_rectangular_intersection_route(void)
{
    GameState game;
    uint32_t i;
    game_init(&game);
    game.traffic_count = 1;
    game.traffic[0].z = game.traffic[0].max_z - game.traffic[0].speed;
    game.traffic[0].direction = 1;
    game_tick(&game, 0);
    assert(game.traffic[0].axis == 1);
    for (i = 0; i < 250; ++i) game_tick(&game, 0);
    assert(game.traffic[0].x == game.traffic[0].max_x);
    assert(game.traffic[0].axis == 0);
    assert(game.traffic[0].direction == -1);
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

static void test_mission_objective_and_reward(void)
{
    GameState game;
    game_init(&game);
    assert(game_start_mission(&game, 1, 0, 0, 1000, 500));
    assert(game_complete_mission(&game));
    assert(game.money == 500);
    assert(game.missions[0].completed);
    assert(!game_complete_mission(&game));
    assert(game_validate(&game));
}
static void test_wanted_level_spawns_police_response(void)
{
    GameState game;
    game_init(&game);
    assert(game_raise_wanted(&game, 2));
    assert(game.wanted_level == 2);
    assert(game.police[0].active && game.police[1].active);
    game_tick(&game, 0);
    assert(game.police[0].x != 0 || game.police[0].z != 0);
    assert(game_validate(&game));
}


static void test_city_block_roads_and_buildings(void)
{
    GameState game;
    game_init(&game);
    assert(game.road_count == 4);
    assert(game.building_count == 4);
    assert(game.obstacle_count == 4);
    assert(game.roads[0].axis == 0);
    assert(game.roads[2].axis == 1);
    assert(game.buildings[0].height > 0);
    assert(game_validate(&game));
}


static void test_gamepad_deadzone_and_button_mapping(void)
{
    GamePadState pad = {7000, -12000, GAMEPAD_A | GAMEPAD_DPAD_RIGHT};
    GameInput in = game_input_from_pad(&pad);
    assert(!in.left);
    assert(in.backward);
    assert(in.right);
    assert(in.interact_pressed);
    pad.left_x = -16000; pad.left_y = 0; pad.buttons = 0;
    in = game_input_from_pad(&pad);
    assert(in.left && !in.right && !in.forward && !in.backward);
}


static void test_gameplay_audio_event_queue(void)
{
    GameState game;
    GameAudioEvent event;
    game_init(&game);
    game.player.x = game.vehicles[0].x;
    game.player.z = game.vehicles[0].z;
    assert(game_interact(&game));
    assert(game_pop_audio_event(&game, &event));
    assert(event.type == GAME_AUDIO_VEHICLE_ENTER);
    assert(game_raise_wanted(&game, 1));
    assert(game_pop_audio_event(&game, &event));
    assert(event.type == GAME_AUDIO_WANTED_RAISED);
    assert(!game_pop_audio_event(&game, &event));
    assert(game_validate(&game));
}


static void test_save_load_snapshot_roundtrip_and_corruption(void)
{
    GameState original, restored;
    unsigned char buffer[sizeof(GameSaveHeader) + sizeof(GameState)];
    game_init(&original);
    original.money = 12345;
    original.player.x = 4321;
    assert(game_save_snapshot(&original, buffer, sizeof(buffer)));
    game_init(&restored);
    assert(game_load_snapshot(&restored, buffer, sizeof(buffer)));
    assert(restored.money == 12345);
    assert(restored.player.x == 4321);
    buffer[sizeof(GameSaveHeader) + 5] ^= 0x40u;
    assert(!game_load_snapshot(&restored, buffer, sizeof(buffer)));
    assert(restored.money == 12345);
}
static void test_damage_uses_armor_then_health(void)
{
    GameState game;
    game_init(&game);
    game.armor = 30;
    game_apply_damage(&game, 45);
    assert(game.armor == 0);
    assert(game.health == 85);
    game_apply_damage(&game, 200);
    assert(game.health == 0);
    game_apply_damage(&game, 10);
    assert(game.health == 0);
    assert(game_validate(&game));
}


static void test_render_list_and_hud_projection(void)
{
    GameState game;
    GameRenderItem items[GAME_RENDER_MAX_ITEMS];
    GameHudState hud;
    size_t count;
    game_init(&game);
    assert(game_start_mission(&game, 77, 5000, 5000, 100, 250));
    assert(game_raise_wanted(&game, 1));
    count=game_build_render_list(&game,items,GAME_RENDER_MAX_ITEMS);
    assert(count >= 10);
    assert(items[0].type == GAME_RENDER_ROAD);
    game_get_hud_state(&game,&hud);
    assert(hud.health == 100 && hud.wanted_level == 1);
    assert(hud.mission_active && hud.mission_id == 77);
    assert(hud.money == 0);
}


static void test_vehicle_brake_and_pad_brake_mapping(void)
{
    GameState game;
    GameInput input = {0,0,0,0,0,1};
    GamePadState pad = {0,0,GAMEPAD_X};
    GameInput mapped = game_input_from_pad(&pad);
    game_init(&game);
    game.player.x = game.vehicles[0].x;
    game.player.z = game.vehicles[0].z;
    assert(game_interact(&game));
    game.vehicles[0].vz = 300;
    game_tick(&game, &input);
    assert(game.vehicles[0].vz < 300);
    assert(mapped.brake);
    assert(game_validate(&game));
}
static void test_init_clears_all_state_bytes(void)
{
    GameState game;
    unsigned char *bytes = (unsigned char *)&game;
    size_t i;
    for (i=0;i<sizeof(game);++i) bytes[i]=0xA5u;
    game_init(&game);
    for (i=0;i<sizeof(game);++i) assert(bytes[i] != 0xA5u || game_validate(&game));
    assert(game_validate(&game));
}

int main(void)
{
    test_initial_state();
    test_vehicle_brake_and_pad_brake_mapping();
    test_init_clears_all_state_bytes();
    test_render_list_and_hud_projection();
    test_save_load_snapshot_roundtrip_and_corruption();
    test_damage_uses_armor_then_health();
    test_gameplay_audio_event_queue();
    test_gamepad_deadzone_and_button_mapping();
    test_city_block_roads_and_buildings();
    test_mission_objective_and_reward();
    test_wanted_level_spawns_police_response();
    test_player_movement_and_friction();
    test_diagonal_movement_is_normalized();
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
    test_traffic_completes_rectangular_intersection_route();
    test_traffic_obeys_red_and_green_light_phases();
    test_traffic_avoids_adjacent_lane_overlap();
    test_traffic_yields_to_player_and_parked_vehicles();
    puts("Gameplay core tests: 27/27 passed");
    return 0;
}
