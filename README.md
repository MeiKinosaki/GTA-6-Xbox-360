# Grand Theft Auto 6 Xbox 360

**Codename:** GTA6-X360  
**Repository:** [MeiKinosaki/GTA-6-Xbox-360](https://github.com/MeiKinosaki/GTA-6-Xbox-360)  
**Target:** Xbox 360 RGH / JTAG (PowerPC Xenon)  
**Project version:** v0.1b — initial project definition

## Project scope

This is an independent, native Xbox 360 demake inspired by the setting and broad gameplay ideas of Grand Theft Auto VI. It is **not** a conversion of Rockstar's original game and does not contain Rockstar source code, game assets, executable files, or other proprietary materials.

The goal is to build an original, technically feasible open-world action prototype for Xbox 360, using original or properly licensed code and assets.

## Development principles

- Track planned, implemented, compiled, and tested work separately.
- Never mark a feature complete without implementation evidence.
- Never claim a successful build or a working `default.xex` without a validated CI artifact.
- Develop in small, sequential versions; do not skip unfinished milestones.
- Prefer a cloud build pipeline (GitHub Actions) so contributors do not need to install the Xenon toolchain locally.

## Target build path

1. Validate an Xbox 360 PowerPC Xenon cross-toolchain in CI.
2. Build and test a minimal native executable.
3. Validate ELF output and the conversion path to `default.xex`.
4. Add engine subsystems incrementally.

The expected deliverables are `Xenon.elf` first and `default.xex` after the conversion pipeline is verified. Neither artifact is claimed to exist in this initial version.

## Roadmap

### v0.1b — project foundation
- [x] Define project identity, target platform, scope, and evidence policy.
- [ ] Select and validate the Xenon/libxenon toolchain in GitHub Actions.
- [ ] Add a minimal native entry point.
- [ ] Produce and validate the first `Xenon.elf`.
- [ ] Validate ELF-to-XEX conversion and produce `default.xex`.

### Future milestones
- Rendering and frame loop.
- Input and camera.
- Original low-poly Vice City-inspired test district.
- Player controller, collision, and vehicle prototype.
- Mission scripting, pedestrians, traffic, and police AI.
- Audio, save data, settings, and game menus.

## Legal and asset policy

Grand Theft Auto and related names and materials are the property of their respective owners. This repository is an independent fan project. Use original assets or assets with clear redistribution and modification permissions. Do not distribute proprietary game files.

## Current status

**Version:** v0.1b  
**Code implementation:** not started  
**Toolchain validation:** not performed  
**Build/CI:** not performed  
**Xenon.elf:** not generated  
**default.xex:** not generated  
**Verified runtime tests:** none
