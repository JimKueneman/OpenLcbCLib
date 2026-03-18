# OpenLcbCLib

A portable, production-ready C library for building OpenLCB/LCC nodes on any processor — microcontrollers, desktop PCs, or anything in between.

OpenLCB (Open Layout Control Bus) is an open standard for connecting model railroad accessories. The NMRA adopted it as LCC (Layout Command Control). OpenLcbCLib implements the full protocol stack in plain C — no dynamic memory, no OS, no external dependencies — so it can run on anything with a C compiler.

## Features

- Full OpenLCB/LCC protocol stack in C
- No dynamic memory allocation — all buffers are statically defined at compile time
- No OS or RTOS required
- Dependency injection pattern — wire in your own hardware drivers
- [Node Wizard](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/node_wizard.html) — browser-based project generator, runs entirely offline
- Working example projects for multiple platforms and IDEs

## Platform-agnostic by design

The library core has no hardware dependencies. If your platform has a C compiler and a CAN (or WiFi) peripheral, it can run OpenLcbCLib. The platforms listed below are the ones that ship with working example projects. Adapting the library to a new platform means writing a small set of driver callbacks — the rest of the code is unchanged.

## Platforms with working examples

| Platform | Transport | IDE / Toolchain |
|---|---|---|
| ESP32 | CAN (TWAI) | Arduino IDE, PlatformIO |
| ESP32 | WiFi GridConnect | Arduino IDE, PlatformIO |
| Raspberry Pi Pico (RP2040) | MCP2517FD | Arduino IDE (Earle Philhower core) |
| STM32F4xx | CAN | STM32CubeIDE |
| TI MSPM0 | MCAN | Code Composer Theia |
| dsPIC | CAN | MPLAB X |
| macOS | Virtual CAN | Xcode |

## Getting started

The fastest path is the ESP32 BasicNode demo. Copy the example project, open it in Arduino IDE, and upload.

See the [Quick Start Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/QuickStartGuide.pdf) for step-by-step instructions.

For all other platforms, the [Developer Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/DeveloperGuide.pdf) covers the Node Wizard, driver callbacks, CDI configuration, and project structure in detail.

For a deep dive into the library architecture, state machines, and protocol internals, see the [Implementation Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/help/overviews/index.html).

API reference: [https://jimkueneman.github.io/OpenLcbCLib/documentation/help/html/](https://jimkueneman.github.io/OpenLcbCLib/documentation/help/html/)

## Repository layout

```
src/                        library source and platform examples
  openlcb/                  protocol engine
  drivers/                  CAN and platform driver implementations
  applications/             ready-to-run example projects
    arduino/esp32/BasicNode/    ESP32 starting point
  utilities/                endian and string helpers
  test/                     unit tests
templates/                  CDI/FDI XML templates and user config templates
  typical/                  standard node openlcb_user_config.h/.c template
  bootloader/               bootloader-only openlcb_user_config.h/.c template
tools/
  node_wizard/              browser-based project generator
  cdi_to_array/             Python script — convert CDI XML to a C byte array
  cdi_to_array_webbrowser/  browser-based CDI to C array converter
  update_applications/      script to sync platform application files
documentation/              guides, design notes, style guides
test/                       compliance and integration tests
```

## Node Wizard

The [Node Wizard](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/node_wizard.html) is a browser-based project generator included in the repository. It can be opened directly from GitHub Pages using the link above, or locally from `tools/node_wizard/node_wizard.html` in a cloned copy — no internet connection needed either way. The Wizard walks you through node type, CDI/FDI configuration, platform selection, and callback options, then generates a complete project ZIP ready to open in your IDE.

If you are building your own custom library and only need to author CDI or FDI XML, the [CDI / FDI Editor](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/cdi_fdi_wizard.html) is a lighter standalone tool (also available locally at `tools/node_wizard/cdi_fdi_wizard.html`) that exposes just the two XML editors without the full project-generation workflow.

## License

BSD 2-Clause. See individual source file headers for the full license text.

## Author

Jim Kueneman
