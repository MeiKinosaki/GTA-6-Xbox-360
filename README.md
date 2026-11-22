# Grand Theft Auto 6 Xbox 360

**Codename:** GTA6-X360  
**Repository:** [MeiKinosaki/GTA-6-Xbox-360](https://github.com/MeiKinosaki/GTA-6-Xbox-360)  
**Target:** Xbox 360 RGH / JTAG (PowerPC Xenon)  
**Project version:** v0.1b — foundation and compiler smoke-test setup

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
2. Compile a minimal native object and then link a valid Xenon ELF.
3. Validate the ELF-to-XEX conversion path.
4. Add engine subsystems incrementally.

The expected deliverables are `Xenon.elf` first and `default.xex` after the conversion pipeline is verified. Neither artifact is currently claimed to exist.

## Current implementation

- `src/main.c`: minimal compiler smoke-test translation unit; not yet a game entry point.
- `Makefile`: uses `xenon-gcc` to compile `src/main.c` into `build/main.o`.
- `.github/workflows/xenon-toolchain.yml`: GitHub Actions workflow using the public `free60/libxenon:latest` container image to run the smoke test and upload the object file.
- `.gitignore`: excludes generated build outputs.

The workflow has been committed, but no workflow run or successful artifact has been observed yet. The Free60 LibXenon repository documents the toolchain image and its `DEVKITXENON` environment setup; this project still needs a successful CI run to validate the selected image and compiler invocation.

## Roadmap

### v0.1b — project foundation
- [x] Define project identity, target platform, scope, and evidence policy.
- [x] Add minimal C translation unit for compiler smoke testing.
- [x] Add Makefile for Xenon object compilation.
- [x] Add GitHub Actions workflow for cloud toolchain validation.
- [ ] Obtain a successful CI run and `build/main.o` artifact.
- [ ] Add a real platform entry point and link the first `Xenon.elf`.
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

**Version:** v0.1b (in progress)  
**Project structure and documentation:** implemented  
**Compiler smoke-test source and Makefile:** committed, not yet CI-validated  
**GitHub Actions workflow:** committed, run not yet observed  
**Toolchain validation:** pending  
**Successful object compilation:** not verified  
**Xenon.elf:** not generated  
**default.xex:** not generated  
**Verified runtime tests:** none
