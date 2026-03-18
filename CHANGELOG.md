# Changelog — OpenLcbCLib

All notable changes to the OpenLcbCLib C library will be documented in this file.

For Node Wizard changes, see `tools/node_wizard/CHANGELOG.md`.

---

## [Unreleased]

### Fixed
- **CDI/FDI read failure when stream not compiled in.** The main statemachine outgoing
  message payload was backed by `payload_stream_t`, which collapsed to 1 byte when
  `OPENLCB_COMPILE_STREAM` was not defined. All datagram-based reads (CDI, FDI, config
  memory) returned zero data. Introduced `payload_worker_t` / `openlcb_worker_message_t`
  sized to `LEN_MESSAGE_BYTES_WORKER` (max of SNIP and STREAM), guaranteeing at least
  256 bytes regardless of stream compilation state.
- Added `default: break;` to `free_buffer()` switch in `openlcb_buffer_store.c` to
  suppress compiler warnings from the new `WORKER` enum value.

### Changed
- **Replaced dispatcher types with worker types.** `LEN_MESSAGE_BYTES_SIBLING_DISPATCH`,
  `payload_dispatcher_t`, and `openlcb_dispatcher_message_t` replaced by
  `LEN_MESSAGE_BYTES_WORKER`, `payload_worker_t`, and `openlcb_worker_message_t`.
- Renamed `openlcb_outgoing_stream_msg_info_t` → `openlcb_outgoing_msg_info_t`.
- Added `WORKER` value to `payload_type_enum` with corresponding case in
  `OpenLcbUtilities_payload_type_to_len()`.
- Main and login statemachine outgoing messages now set `payload_type = WORKER`.
- Sibling response queue and Path B pending buffer now use `openlcb_worker_message_t`.
- **Bootloader single-define setup.** `openlcb_config.h` now auto-defines
  `OPENLCB_COMPILE_DATAGRAMS`, `OPENLCB_COMPILE_MEMORY_CONFIGURATION`, and
  `OPENLCB_COMPILE_FIRMWARE` (and undefines EVENTS, BROADCAST_TIME, TRAIN,
  TRAIN_SEARCH) when `OPENLCB_COMPILE_BOOTLOADER` is defined. Bootloader user configs
  only need `#define OPENLCB_COMPILE_BOOTLOADER`.
- **FDI length auto-collapse.** `USER_DEFINED_FDI_LENGTH` is overridden to 1 in
  `openlcb_types.h` when `OPENLCB_COMPILE_TRAIN` is not defined, saving RAM in
  non-train nodes without requiring users to set it manually.
### Added
- **Compile-time zero-length array guards.** `#if DEFINE < 1 #error` checks in
  `openlcb_types.h` for all buffer depth and count defines: `USER_DEFINED_BASIC_BUFFER_DEPTH`,
  `USER_DEFINED_DATAGRAM_BUFFER_DEPTH`, `USER_DEFINED_SNIP_BUFFER_DEPTH`,
  `USER_DEFINED_STREAM_BUFFER_DEPTH`, `USER_DEFINED_NODE_BUFFER_DEPTH`,
  `USER_DEFINED_CDI_LENGTH`, `USER_DEFINED_FDI_LENGTH`, `USER_DEFINED_PRODUCER_COUNT`,
  `USER_DEFINED_PRODUCER_RANGE_COUNT`, `USER_DEFINED_CONSUMER_COUNT`,
  `USER_DEFINED_CONSUMER_RANGE_COUNT`, `USER_DEFINED_MAX_LISTENERS_PER_TRAIN`,
  `USER_DEFINED_MAX_TRAIN_FUNCTIONS`.

### Removed
- Dead code: `openlcb_statemachine_worker_t` (from `openlcb_types.h`) and
  `can_main_statemachine_t` (from `can_types.h`) — both were defined but never
  instantiated in any `.c` file.

### Memory
- `LEN_MESSAGE_BYTES_STREAM` in `openlcb_types.h` now derives from `USER_DEFINED_STREAM_BUFFER_LEN`
  rather than being hardcoded to 512, saving 1,280 bytes of static RAM at the default setting.

---

## [1.0.0] — 2026-03-17

Initial production release.

### Protocol support
- Full OpenLCB/LCC protocol stack in C
- Frame layer: alias negotiation, alias conflict detection, CAN framing
- Message layer: Verified Node ID, Protocol Identification (PIP), SNIP, Optional Interaction Rejected (OIR)
- Event protocol: producer/consumer, identify events, learn/teach
- Datagram protocol: send, receive, error handling
- Configuration memory protocol: read, write, lock, ACDI manufacturer and user spaces, CDI/FDI XML delivery
- Broadcast Time protocol: query, set, immediate time, rollover, producer and consumer roles
- Train Control protocol: speed, functions, emergency stop, global e-stop, global off, memory spaces,
  listener management, controller assignment, reserve/release, heartbeat
- Consist forwarding: listener alias table initialization and TX state machine wiring

### Architecture
- No dynamic memory — all buffers statically defined at compile time
- No OS or RTOS dependency
- Dependency injection pattern for all hardware and platform callbacks
- Periodic listener alias verification with configurable timing

### Virtual node system
- Multiple logical OpenLCB nodes running on a single physical device
- Incoming CAN messages fanned out to all sibling nodes via the main statemachine
- Outgoing messages from any node dispatched to all siblings before going to the wire
  (zero-copy — no extra buffer allocation per sibling)
- Chain dispatch: sibling responses that themselves trigger further sibling notifications
  handled via a static response queue, not recursion
- Path B: application messages generated while sibling dispatch is active queued in a
  single-slot pending buffer and delivered on the next run loop tick
- Login statemachine notifies all sibling nodes when any node completes alias negotiation
- Single-node fast path: entire sibling loop bypassed when only one node is configured,
  zero overhead for the common case
- 6-priority run loop with dedicated sibling dispatch sub-steps
- New required DI function pointer: `openlcb_node_get_count`
- New public API: `OpenLcbMainStatemachine_send_with_sibling_dispatch()`

### Testing
- `can_multinode_e2e_Test.cxx`: CAN-layer multi-node end-to-end tests — multiple virtual nodes
  sharing a single CAN bus, alias negotiation, message routing across siblings
- `openlcb_multinode_e2e_Test.cxx`: OpenLCB-layer multi-node end-to-end tests — event, datagram,
  and protocol interactions between sibling nodes within a single device
- `threadsafe_stringlist_Test.cxx`: 9 unit tests for the PC utilities string FIFO

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
- OlcbChecker compliance test suite: frame, message, event, datagram, config memory,
  SNIP, CDI, FDI, broadcast time, train control (TR010–TR110)

### Utilities
- `src/utilities_pc/`: PC-only (POSIX) utilities, not copied to embedded targets
  - `threadsafe_stringlist`: mutex-protected string FIFO for GridConnect output queuing
