# Changelog — OpenLcbCLib

All notable changes to the OpenLcbCLib C library will be documented in this file.

For Node Wizard changes, see `tools/node_wizard/CHANGELOG.md`.

---

## [Unreleased]

### Added
- **Listener alias table registration on attach/detach.** The CAN TX path now sniffs
  outgoing Listener Config Reply messages (MTI 0x01E9, byte 0 = 0x30). On a successful
  attach reply, it registers the listener's node_id in the alias table and immediately
  sends a targeted AME to resolve the alias. On a successful detach reply, it
  unregisters the node_id. This fixes consist forwarding, which previously always
  failed because the listener alias table was never populated.
  (`can_tx_statemachine.c`, `can_tx_statemachine.h`, `can_config.c`)
- **10 new tests for listener config reply sniffing** in `can_tx_statemachine_Test.cxx`:
  attach/detach success and failure, AME generation, lock/unlock verification, null
  pointer safety, short payload and non-matching MTI guards.

### Fixed
- **Compliance node FDI data.** Replaced single-byte placeholder with valid FDI XML
  byte array. The `<function>` element now uses `<number>` as a child element instead
  of an attribute, matching the FDI XSD schema.
- **Address space highest_address now computed from data.** CDI and FDI address space
  `highest_address` fields in the compliance node use `sizeof(_xxx_data) - 1` instead
  of hardcoded values, preventing mismatches when the XML changes.
- **CDI/FDI read failure when stream not compiled in.** The main statemachine outgoing
  message payload was backed by `payload_stream_t`, which collapsed to 1 byte when
  `OPENLCB_COMPILE_STREAM` was not defined. All datagram-based reads (CDI, FDI, config
  memory) returned zero data. Introduced `payload_worker_t` / `openlcb_worker_message_t`
  sized to `LEN_MESSAGE_BYTES_WORKER` (max of SNIP and STREAM), guaranteeing at least
  256 bytes regardless of stream compilation state.
- Added `default: break;` to `free_buffer()` switch in `openlcb_buffer_store.c` to
  suppress compiler warnings from the new `WORKER` enum value.
- **Uninitialized variables across much of the library.** Fixed multiple uninitialized
  variable bugs caught during cross-platform testing.
- **Linux test compile fixes.** Multiple rounds of fixes for C++ designated initializer
  ordering, sign-compare warnings (`-Werror`), and other portability issues to get the
  full test suite compiling on Linux with GCC.
- **Fixed sample CDI.** Corrected the sample CDI XML and the byte array generation in
  both the Python tool and Node Wizard.

### Changed
- **CDI/FDI arrays moved to pointers.** `node_parameters_t` now holds `const uint8_t *`
  pointers to CDI and FDI byte arrays instead of embedding fixed-size arrays in the
  struct. Allows auto-generation of array-only files when new XMLs are created.
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
- **`firmware_write` prototype simplified.** Changed the function signature to make it
  easier for users to implement.
- **Repository layout.** Moved `applications/` from `src/` to the project root so
  `src/` contains only the library source.
- **`node_parameters_t` struct reordered.** `configuration_options` moved after
  `consumer/producer_count_autocreate`; `cdi` and `fdi` changed from arrays to pointers
  at the end of the struct. All templates, demos, and compliance node initializers
  updated to match.

### Added
- **Custom clock ID helper.** Added `OpenLcbApplicationBroadcastTime_make_clock_id()`
  that constructs a properly formatted broadcast time clock ID from a raw 48-bit
  unique identifier, for use with custom clocks beyond the four well-known ones.
- **Compile-time zero-length array guards.** `#if DEFINE < 1 #error` checks in
  `openlcb_types.h` for all buffer depth and count defines.
- **`can_user_config.h` required.** `can_types.h` now searches for `can_user_config.h`
  using the same `__has_include` pattern as `openlcb_types.h`. CAN buffer depth is
  user-configurable via `USER_DEFINED_CAN_MSG_BUFFER_DEPTH` (default 20). Template
  added at `templates/canbus/can_user_config.h`.
- **`can_config.c` added to CMake** test build.
- **XmlToArray.py tool** (renamed from CdiToArray.py). Generates CDI/FDI byte array
  `.c`/`.h` files with options for licence headers, author/company metadata, and
  whitespace stripping.
- **Compile flag coverage tests.** Tests verifying each `OPENLCB_COMPILE_*` flag
  compiles correctly, plus tests validating dead code is stripped for lazy linkers
  (e.g. XC16).
- **CAN driver DI cleanup.** Added dependency injection points in the CAN driver to
  remove some hard dependencies.

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
