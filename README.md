# Grand Theft Auto 6 Xbox 360

Independent native Xbox 360 demake prototype inspired by the broad setting and gameplay ideas of Grand Theft Auto VI. This is not a conversion of Rockstar's original game and contains no Rockstar source code or proprietary assets.

**Target:** Xbox 360 RGH/JTAG (PowerPC Xenon)  
**Final executable:** `default.xex`  
**Build system:** XSDK  
**Current milestone:** v0.1b — gameplay foundation

## Gameplay implementation

The first playable-systems layer is being developed as platform-independent C code:

- `include/gta6/game.h`: deterministic world, player, vehicle, and input structures.
- `src/game.c`: player movement, acceleration, friction, speed limits, world bounds, vehicle proximity checks, entering, driving, and exiting.
- `tests/test_game.c`: regression tests for initial state, movement, friction, map boundaries, vehicle interaction, and occupied/distant vehicles.

Coordinates use integer millimetres (1000 units per metre) to avoid floating-point drift in this early simulation layer. The current map is a small test area, not the final open world.

## Build and validation

- Host gameplay tests compile with GCC and run in CI independently of the Xbox toolchain.
- XSDK cross-compiles the title and packages the final XEX2 executable.
- CI verifies the XEX2 magic and publishes `default.xex`.
- A successful host test does not prove the Xbox executable builds or runs on hardware; those are tracked separately.

## Current status

- Project scope and build target: defined.
- Earlier Xenon compiler object smoke test: passed.
- Earlier LibXenon ELF link: failed due to PowerPC ABI mismatch; superseded by XSDK.
- XSDK workflow: currently being corrected; previous run stopped because the toolchain build script was invoked without execute permission.
- Player/vehicle gameplay core: implemented in source; awaiting automated test result.
- `default.xex`: not yet produced by a successful XSDK CI run.
- Xbox 360 RGH runtime test: not performed.

## Roadmap

### v0.1b — gameplay foundation
- [x] Establish native project and XEX2 target.
- [x] Implement player movement and basic vehicle enter/drive/exit logic.
- [x] Add host regression tests.
- [ ] Pass gameplay regression tests in CI.
- [ ] Produce and validate `default.xex`.
- [ ] Test launch on Xbox 360 RGH/JTAG.

### Next gameplay milestones
- Camera follow and controller mapping.
- Collision geometry and pedestrian movement.
- Vehicle steering, turning, braking, and collision response.
- Streaming test district with roads, buildings, and sidewalks.
- Mission scripting and basic wanted/police system.
- Save/load, pause menu, map, and HUD.

## Legal and asset policy

Grand Theft Auto and related marks are owned by their respective rights holders. This is an independent fan project. Use original or properly licensed code and assets; do not distribute proprietary game files.
