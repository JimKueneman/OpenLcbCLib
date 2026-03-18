# Changelog — OpenLcbCLib

All notable changes to the OpenLcbCLib C library will be documented in this file.

For Node Wizard changes, see `tools/node_wizard/CHANGELOG.md`.

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
