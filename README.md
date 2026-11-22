# Grand Theft Auto 6 Xbox 360

Independent native Xbox 360 demake prototype inspired by the broad setting and gameplay ideas of Grand Theft Auto VI. This is not a port of Rockstar's original game and contains no proprietary Rockstar code or assets.

**Target:** Xbox 360 RGH/JTAG  
**Build deliverable:** `default.xex`  
**Build system:** XSDK (PowerPC Xbox 360 homebrew SDK)  
**Current milestone:** v0.1b — native XEX bootstrap

## Build policy

The project's public deliverable is `default.xex`. The build system may create intermediate object files and an ELF internally because the XEX2 packer consumes a linked PowerPC ELF. ELF is not published as the project's target artifact.

## Current implementation

- `src/main.c`: minimal XSDK-native title entry point using the XSDK logging API.
- `CMakeLists.txt`: defines the title and calls `xsdk_add_xex`, which links and packages the executable as XEX2.
- `Makefile`: configures the XSDK cross-compilation and copies the packaged title to `default.xex`.
- `.github/workflows/xenon-toolchain.yml`: cloud workflow to build XSDK, compile the title, check the XEX2 magic, and upload `default.xex`.

## CI pipeline

1. Install the documented Linux build dependencies.
2. Clone XSDK and build its PowerPC cross-toolchain.
3. Build the XSDK runtime and libraries.
4. Configure this project with XSDK's CMake toolchain.
5. Build and package the title with `elf2xex.py` through `xsdk_add_xex`.
6. Verify that the packaged file starts with the XEX2 magic and publish it as the `default.xex` artifact.

A successful header check establishes that the output is an XEX2 container; it does not establish that the title has been tested on a physical Xbox 360. Console runtime validation remains a separate milestone.

## Status

- Project scope and initial documentation: committed.
- Earlier LibXenon object smoke test: passed.
- Earlier direct LibXenon ELF link: failed due to PowerPC ABI/architecture mismatch; that route is superseded.
- XSDK-based direct XEX packaging workflow: committed, awaiting CI execution.
- `default.xex`: not yet generated/verified by CI.
- Xbox 360 RGH runtime test: not performed.

## Roadmap

### v0.1b — bootstrap
- [x] Establish project scope and evidence-based progress policy.
- [x] Add minimal native title source.
- [x] Configure XSDK-based XEX2 packaging.
- [ ] Obtain a successful cloud build and `default.xex` artifact.
- [ ] Test launch on an Xbox 360 RGH/JTAG console.

### Later milestones
- Native render loop and input.
- Original low-poly Vice City-inspired test area.
- Player controller, collision, and vehicle prototype.
- Mission scripting, pedestrians, traffic, and police AI.
- Audio, save data, settings, and menus.

## Legal and asset policy

Grand Theft Auto and related marks are owned by their respective rights holders. This is an independent fan project. Use original or properly licensed assets and do not distribute proprietary game files.
