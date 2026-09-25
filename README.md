# OpenLcbCLib

A portable, production-ready C library for building OpenLCB/LCC nodes on any processor — microcontrollers, desktop PCs, or anything in between.

OpenLCB (Open Layout Control Bus) is an open standard for connecting model railroad accessories. The NMRA adopted it as LCC (Layout Command Control). OpenLcbCLib implements the full protocol stack in plain C — no dynamic memory, no OS, no external dependencies — so it can run on anything with a C compiler.

## Features

- Full OpenLCB/LCC protocol stack in C: events, datagrams, configuration memory, SNIP, streams, broadcast time, train control and search, DCC detector, DCC CV programming space, firmware upgrade
- No dynamic memory allocation — all buffers are statically defined at compile time
- No OS or RTOS required
- Dependency injection pattern — wire in your own hardware drivers
- CAN bus or TCP/IP GridConnect transport, selected at compile time
- Optional feature flags compile out every protocol you do not use
- [Node Wizard](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/node_wizard.html) — browser-based project generator, runs entirely offline
- Standalone firmware-upgrade bootloader with platform examples
- Working example projects for multiple platforms and IDEs

## Platform-agnostic by design

The library core has no hardware dependencies. If your platform has a C compiler and a CAN (or network) peripheral, it can run OpenLcbCLib. The platforms listed below are the ones that ship with working example projects. Adapting the library to a new platform means writing a small set of driver callbacks — the rest of the code is unchanged.

## Platforms with working examples

| Platform | Transport | IDE / Toolchain |
|---|---|---|
| ESP32 | CAN (TWAI) | Arduino IDE, PlatformIO |
| ESP32 | WiFi GridConnect | PlatformIO |
| Raspberry Pi Pico (RP2040) | MCP2517FD (SPI) | Arduino IDE (Earle Philhower core) |
| STM32F4xx | CAN | STM32CubeIDE |
| TI MSPM0 | MCAN | Code Composer Studio Theia |
| dsPIC | CAN | MPLAB X |
| macOS | GridConnect over TCP | Xcode, PlatformIO |

## Getting started

The fastest path is the ESP32 BasicNode demo. Copy the example project, open it in Arduino IDE, and upload.

See the [Quick Start Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/QuickStartGuide.pdf) for step-by-step instructions.

For all other platforms, the [Developer Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/DeveloperGuide.pdf) covers the Node Wizard, driver callbacks, CDI configuration, and project structure in detail.

For a deep dive into the library architecture, state machines, and protocol internals, see the [Implementation Guide](https://jimkueneman.github.io/OpenLcbCLib/documentation/help/overviews/index.html).

API reference: [https://jimkueneman.github.io/OpenLcbCLib/documentation/help/html/](https://jimkueneman.github.io/OpenLcbCLib/documentation/help/html/)

## Repository layout

```
src/                              main library source
  openlcb/                        protocol engine — state machines and protocol handlers
  drivers/
    canbus/                       CAN bus transport (alias negotiation, RX/TX, login)
    tcp_ip/                       TCP/IP GridConnect transport
  utilities/                      endian and string helpers
  utilities_pc/                   PC-only (POSIX) helpers, not copied to embedded targets
  test/                           shared GoogleTest main used by every unit test

applications/                     ready-to-run example projects (full node apps)
  arduino/                        Arduino IDE examples (ESP32, Raspberry Pi Pico)
  dspic/                          MPLAB X dsPIC example
  platformio/                     PlatformIO examples (ESP32 CAN, ESP32 WiFi, macOS native)
  stm32_cubeide/                  STM32CubeIDE STM32F4xx example
  ti_theia/                       Code Composer Studio Theia TI MSPM0 example
  xcode/                          macOS GridConnect-over-TCP example
  reference_material/             vendor reference documents

bootloader/                       standalone OpenLCB firmware-upgrade bootloader
  src/                            bootloader library source
    crc/                          CRC-16/IBM with triple-checksum validation
    drivers/
      canbus/                     CAN state machine, RX, TX engines
    openlcb/                      OpenLCB protocol layer, boot-decision logic
  templates/                      driver templates for a new bootloader port
  applications/                   platform-specific bootloader projects
    dspic/                        MPLAB X (dsPIC33): bootloader, matching app, shared RAM
    esp32/                        ESP32: bootloader, matching app, shared RAM
    stm32_cubeide/                STM32CubeIDE (STM32F407 Discovery): bootloader, app, shared RAM
    ti_theia/                     Theia (MSPM0G3507): bootloader, app, shared RAM
  test/                           bootloader unit tests (CMake / GoogleTest)
  tools/
    hex2bin/                      hex-to-binary image tool and boot image script
    update_bootloaders/           script to sync bootloader sources across platforms
  documentation/                  architecture, porting guide, checksum design,
                                  state-machine diagrams

test/                             unit, compliance and integration tests
  CMakeLists.txt, Makefile        GoogleTest build for src/ (see test/README.md)
  user_config/                    test configurations (typical, minimal, tcp, bootloader, ...)
  guard_audit.sh                  compiles every feature-flag configuration and checks guards
  check_symbol_leakage.sh         verifies a disabled module exports no code
  compliance_node/                macOS compliance-test node (Xcode)
  olcbchecker_bridge/             bridge and runner for the OpenLCB OlcbChecker suite

templates/                        user config templates for new projects
  typical/                        standard node openlcb_user_config.h/.c
  bootloader/                     bootloader openlcb_user_config.h/.c
  canbus/                         can_user_config.h
  tcp_ip/                         tcp_user_config.h
  sampleCDI.xml, sampleFDI.xml    example CDI and FDI documents

tools/
  node_wizard/                    browser-based project generator (runs offline)
  xml_to_array/                   convert CDI/FDI XML to a C byte array
  update_applications/            script to sync library files across platform apps
  release_update_wizard/          release helper for the Node Wizard

wasm/                             WebAssembly build of the library with a JS test harness

documentation/                    guides (PDF), Doxygen API reference, design notes,
                                  style guides
```

## Node Wizard

The [Node Wizard](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/node_wizard.html) is a browser-based project generator included in the repository. It can be opened directly from GitHub Pages using the link above, or locally from `tools/node_wizard/node_wizard.html` in a cloned copy — no internet connection needed either way. The Wizard walks you through node type and add-ons, CDI/FDI configuration, target platform, and callback options, then generates a complete project ZIP ready to open in your IDE.

If you are building your own custom library and only need to author CDI or FDI XML, the [CDI / FDI Editor](https://jimkueneman.github.io/OpenLcbCLib/tools/node_wizard/cdi_fdi_wizard.html) is a lighter standalone tool (also available locally at `tools/node_wizard/cdi_fdi_wizard.html`) that exposes just the two XML editors without the full project-generation workflow.

## Building and running the tests

```
cd test && rm -rf build && mkdir build && cd build && cmake .. && make
```

Each test executable runs as part of the build. `test/guard_audit.sh` compiles the library under every feature-flag configuration and fails on any unguarded cross-module call.

## License

BSD 2-Clause. See individual source file headers for the full license text.

## Author

Jim Kueneman
