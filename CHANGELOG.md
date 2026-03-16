# Changelog

All notable changes to OpenLcbCLib will be documented in this file.

---

## [1.1.0] — 2026-03-15

### Fixed
- **Consist forwarding silently dropped** — `ListenerAliasTable_initialize()` was not called
  at startup and `listener_find_by_node_id` was not wired into the TX state machine in
  `can_config.c`. Both DI pointer assignments are now guarded by `#ifdef OPENLCB_COMPILE_TRAIN`
  and added to `CanConfig_initialize()` and `_build_tx_statemachine()` respectively.
  Without this fix, all consist-forwarded Train Control messages were silently dropped because
  the alias-resolution callback was always NULL after the `memset()` initialisation.

### Changed
- macOS platform transport corrected from "Virtual CAN" to GridConnect in all documentation.

---

## [1.0.0] — 2026-03-15

Initial production release.

### Protocol support
- Full OpenLCB/LCC protocol stack in C
- Frame layer: alias negotiation, alias conflict detection, CAN framing
- Message layer: Verified Node ID, Protocol Identification (PIP), SNIP, Optional Interaction Rejected (OIR)
- Event protocol: producer/consumer, identify events, learn/teach
- Datagram protocol: send, receive, error handling
- Configuration memory protocol: read, write, lock, ACDI manufacturer and user spaces, CDI/FDI XML delivery
- Broadcast Time protocol: query, set, immediate time, rollover, producer and consumer roles
- Train Control protocol: speed, functions, emergency stop, global e-stop, global off, memory spaces, listener management, controller assignment, reserve/release, heartbeat

### Architecture
- No dynamic memory — all buffers statically defined at compile time
- No OS or RTOS dependency
- Dependency injection pattern for all hardware and platform callbacks
- Virtual node support with sibling loopback dispatch
- Periodic listener alias verification with configurable timing

### Platforms with working examples
- ESP32 + TWAI (Arduino IDE, PlatformIO)
- ESP32 + WiFi GridConnect (Arduino IDE, PlatformIO)
- Raspberry Pi Pico / RP2040 + MCP2517FD (Arduino IDE)
- STM32F4xx + CAN (STM32CubeIDE)
- TI MSPM0 + MCAN (Code Composer Theia)
- dsPIC + CAN (MPLAB X)
- macOS GridConnect (Xcode)

### Tools
- Node Wizard: browser-based project generator for all supported platforms
- OlcbChecker compliance test suite: frame, message, event, datagram, config memory, SNIP, CDI, FDI, broadcast time, train control (TR010–TR110)
