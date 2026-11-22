# Grand Theft Auto 6 Xbox 360

Independent native Xbox 360 demake prototype inspired by the broad setting and gameplay ideas of Grand Theft Auto VI. This is not a conversion of Rockstar's original game and contains no Rockstar source code or proprietary assets.

**Target:** Xbox 360 RGH/JTAG (PowerPC Xenon)  
**Final executable:** `default.xex`  
**Build system:** XSDK  
**Current milestone:** v0.1b — gameplay foundation

## Gameplay implementation

- `include/gta6/game.h`: world, player, vehicle, camera, obstacle, and input structures.
- `src/game.c`: fixed-point player movement, acceleration/friction, world bounds, vehicle enter/drive/exit, obstacle collision response, and smoothed camera-follow target.
- `tests/test_game.c`: regression coverage for movement, friction, world bounds, vehicle interactions, obstacle collision, camera follow, and obstacle validation.

World coordinates are integer units (1000 units per metre). The current implementation is a top-down simulation foundation; it is not yet a rendered 3D world and does not yet read a physical Xbox controller.

## Validation policy

- Host regression tests run with GCC independently of the Xbox toolchain.
- XSDK is intended to cross-compile and package the final XEX2 executable.
- CI checks the XEX2 magic and publishes `default.xex`.
- Passing host tests does not prove an Xbox executable exists or runs on hardware.

## Current status

- Native project scope and XEX2 target: defined.
- Initial Xenon object compilation: passed in an earlier CI run.
- Gameplay movement and vehicle logic: implemented.
- Camera follow and obstacle collision: implemented in source.
- Previous gameplay CI run passed 5/5 tests before camera/collision changes.
- A run against an earlier commit failed because its boundary assertion expected the old map edge; the assertion was updated for the player's collision radius. The current 8-test run is the one to evaluate.
- XSDK cross-toolchain build: separate CI stage; not yet confirmed successful.
- `default.xex`: not yet confirmed generated.
- Xbox 360 RGH runtime test: not performed.

## Roadmap

### v0.1b — gameplay foundation
- [x] Establish native project and XEX2 target.
- [x] Implement player movement and vehicle enter/drive/exit logic.
- [x] Add smoothed camera-follow state.
- [x] Add rectangular obstacle collision.
- [x] Add regression tests for new systems.
- [ ] Pass current gameplay regression suite in CI.
- [ ] Produce and validate `default.xex`.
- [ ] Test launch on Xbox 360 RGH/JTAG.

### Next gameplay milestones
- Controller mapping and camera orientation.
- More robust collision resolution and vehicle steering/reverse/braking.
- Test district: road grid, sidewalks, building footprints, and streaming blocks.
- Pedestrian and traffic simulation.
- Mission scripting, wanted level, and police response.
- Save/load, pause menu, map, and HUD.

## Legal and asset policy

Grand Theft Auto and related marks are owned by their respective rights holders. This is an independent fan project. Use original or properly licensed code and assets; do not distribute proprietary game files.
