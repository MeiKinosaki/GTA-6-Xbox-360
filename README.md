# Grand Theft Auto 6 Xbox 360

Independent native Xbox 360 demake prototype inspired by the broad setting and gameplay ideas of Grand Theft Auto VI. This is not a conversion of Rockstar's original game and contains no Rockstar source code or proprietary assets.

**Target:** Xbox 360 RGH/JTAG (PowerPC Xenon)  
**Final executable:** `default.xex`  
**Build system:** XSDK  
**Current milestone:** v0.1b — gameplay foundation

## Gameplay implementation

- `include/gta6/game.h`: player, vehicle, camera, road, building, pedestrian, traffic, traffic-light, police, mission, controller, and audio-event data.
- `src/game.c`: fixed-point movement and friction; diagonal speed normalization; vehicle enter/drive/exit; obstacle collision; smoothed camera target; pedestrian patrol and avoidance; traffic patrol, rectangular route turns, collision yielding and signal phases; initial road/building district; mission objectives and rewards; wanted level and basic police pursuit; gamepad-state mapping; gameplay audio-event queue.
- `tests/test_game.c`: host regression coverage for these gameplay systems.

The initial district currently contains four road segments and four building footprints. Buildings are represented as collision obstacles and have height metadata; there is not yet a 3D renderer, mesh pipeline, texture system, or streaming world.

World coordinates are integer units (1000 units per metre). The gamepad adapter translates an abstract pad state (left stick, D-pad and A button) into gameplay input; native Xbox controller polling still needs to be connected in the title runtime. The audio queue emits gameplay events, but a real XMA/audio mixer backend is not implemented yet.

## Validation policy

- Host regression tests run with GCC independently of the Xbox toolchain.
- XSDK is intended to cross-compile and package the final XEX2 executable.
- CI checks the XEX2 magic and publishes `default.xex`.
- Passing host tests does not prove an Xbox executable exists or runs on hardware.

## Current status

- Native project scope and XEX2 target: defined.
- Gameplay core, basic traffic/pedestrian simulation, mission/wanted logic, initial district metadata, pad-input adapter, and audio event queue: implemented in source.
- Host test suite: 22 cases in source; the latest CI run must confirm the audio-queue addition.
- XSDK cross-toolchain build: separate CI stage; latest runs are still building the PowerPC toolchain.
- `default.xex`: not yet confirmed generated.
- Xbox 360 RGH runtime test: not performed.
- Full 3D city, actual XInput polling, XMA mixer/output, save/load, mission content, police combat/arrest systems, and full UI remain incomplete.

## Roadmap

### v0.1b — playable foundation
- [x] Establish native project and XEX2 target.
- [x] Implement player movement and vehicle enter/drive/exit logic.
- [x] Add camera-follow state and rectangular collision.
- [x] Add basic pedestrians, traffic routes, turns and traffic-light phases.
- [x] Add first road/building district metadata.
- [x] Add mission objective/reward and wanted/police simulation foundations.
- [x] Add gamepad input adapter and gameplay audio event queue.
- [ ] Confirm latest full host regression suite in CI.
- [ ] Produce and validate `default.xex`.
- [ ] Connect native controller, rendering and audio backends.
- [ ] Implement save/load and complete HUD/menu flow.
- [ ] Test launch on Xbox 360 RGH/JTAG.

### Next gameplay milestones
- Native 3D rendering, city meshes, textures, sidewalks, lighting and world streaming.
- Full vehicle steering, reverse, braking, damage and traffic junction rules.
- NPC schedules, pedestrian interactions and population simulation.
- Mission scripting, police search/chase/arrest and wanted-level decay.
- XMA audio, music, ambience and sound effects.
- HUD, pause menu, map, settings and save/load.

## Legal and asset policy

Grand Theft Auto and related marks are owned by their respective rights holders. This is an independent fan project. Use original or properly licensed code and assets; do not distribute proprietary game files.
