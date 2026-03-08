# OpenLCB C Library - Documentation Outline

**Purpose:** Guide for maintainers and new contributors to understand how this library is implemented, how data flows through it, and where to look when debugging.

**Existing docs** (already in this folder): StyleGuide_v4.md, C_Documentation_Guide_v1_7.md, Doxygen_StyleGuide.md, Doxygen HTML reference, MainStatemachineFlowCharts.pdf

---

## Part I: Orientation

### Chapter 1 - Project Overview
- What is OpenLCB and what problem does this library solve
- Relationship to the OpenLCB/LCC specification
- Design philosophy: zero dynamic allocation, non-blocking state machines, static buffer pools, callback-based dependency injection
- **Architecture boundary: transport-independent core vs. transport drivers**
  - `openlcb/` — transport-independent core: types, nodes, buffers, main statemachine, protocol handlers all operate on full 48-bit Node IDs and `openlcb_msg_t` with no knowledge of any transport
  - `drivers/canbus/` — CAN transport driver: aliases, CAN frames, `can_msg_t`, CAN-specific buffers/FIFO, login with CID/RID/AMD, fragmentation — all CAN-specific
  - Note: the `alias` field rides along in `openlcb_node_t` for convenience but is not required by the OpenLCB core to function; it is populated and consumed only by the CAN transport layer
  - TCP/IP transport (in development) — works with pure OpenLCB messages directly, no alias mapping needed
- Target platforms (bare-metal embedded, RTOS, POSIX for testing)
- Directory structure walkthrough (`openlcb/`, `drivers/canbus/`, `templates/`, `gTests/`)

### Chapter 2 - Building and Running
- Brief overview only; a separate dedicated document/website covers this topic in full
- Where to find compile-time configuration constants (`USER_DEFINED_*` macros)
- Link to the dedicated build/integration guide

---

## Part II: Core Infrastructure

### Chapter 3 - Types and Constants (Transport-Independent)
- `openlcb_types.h` master tour: Node IDs, aliases, MTI codes, payload types
- `openlcb_defines.h`: MTI values, protocol support bits, run-states
- Note: the alias field in `openlcb_node_t` and alias fields in `openlcb_msg_t` exist for CAN transport convenience but are not used by protocol-layer logic
- Naming conventions and how to find the constant you need

### Chapter 3b - CAN-Specific Types and Constants
- `can_types.h`: CAN frame structure (`can_msg_t`), 29-bit identifier layout
- `can_defines.h` (if applicable): CAN frame type constants, control frame identifiers
- CAN-specific state machine structs: `can_statemachine_info_t`, `alias_mapping_t`, `listener_alias_entry_t`

### Chapter 4 - OpenLCB Buffer and Message System (Transport-Independent)
- The `message_buffer_t` master struct and its four segregated payload pools (BASIC 16B, DATAGRAM 72B, SNIP 256B, STREAM 512B)
- `openlcb_msg_t` structure: MTI, Node IDs, payload pointer, reference count, timer union
- Note: alias fields in `openlcb_msg_t` are populated by the CAN transport but not used by protocol handlers
- Allocation pattern: `OpenLcbBufferStore_allocate_buffer()` / `_free_buffer()` / `_inc_reference_count()`
- How payload pointers are lazily linked at initialization
- Reference counting for shared ownership across FIFO and processing contexts
- Pool exhaustion: what happens, how to diagnose, how to tune pool sizes
- Debugging tip: peak allocation tracking
- _Thread safety: buffer allocation requires lock/unlock protection (see Chapter 6)_

### Chapter 4b - CAN Buffer System (CAN-Specific)
- `can_buffer_store.h`: CAN frame buffer pool (`can_msg_t` array, `USER_DEFINED_CAN_MSG_BUFFER_DEPTH`)
- `can_buffer_fifo.h`: circular FIFO for incoming CAN frames, thread-safety model
- How CAN buffers feed into OpenLCB buffers via the RX pipeline
- _Thread safety: FIFO is the ISR/main-loop boundary — see Chapter 6_

### Chapter 5 - Node Management
- `openlcb_node_t` structure walkthrough: state bits, ID, alias, seed, event lists, parameters pointer, train_state pointer
- `openlcb_node_state_t` bit-fields: runstate, allocated, permitted, initialized, duplicate_id_detected, datagram flags
- Static node pool: `USER_DEFINED_NODE_BUFFER_DEPTH`, nodes never deallocated
- `node_parameters_t`: per-node const configuration (SNIP data, CDI, FDI, address space definitions)
- Multi-key enumerator system: `openlcb_node_get_first(key)` / `_get_next(key)` / `_is_last(key)` with 8 concurrent enumeration keys
- How to add a new virtual node at runtime

### Chapter 6 - Thread Safety and Shared Resource Protection
- Why this matters: the library runs across interrupt context (CAN RX), main loop, and potentially RTOS tasks
- The `lock_shared_resources()` / `unlock_shared_resources()` contract: what the application must provide
- What must be protected and why:
  - Alias mapping table: ISR/RX writes `is_duplicate`, main loop reads and resets nodes
  - CAN RX FIFO: ISR pushes frames, main loop pops — producer/consumer boundary
  - OpenLCB message buffer pool: allocation from RX path vs. main loop
  - Node state flags: RX path may set flags that main loop acts on
- Bare-metal implementation: disable/enable interrupts
- RTOS implementation: mutex or critical section
- Common pitfalls: holding locks too long, forgetting to protect buffer allocation, nested lock calls
- _This topic is reinforced in context throughout the document — see Chapters 4, 4b, 8, 10, 11, 19, 23_

### Chapter 6b - Callback Interface Pattern
- The dependency-injection pattern used throughout: application creates callback struct, calls `*_initialize()`, module stores pointer
- Required vs. optional callbacks: NULL optional callback = auto-reject with Optional Interaction Rejected
- `send_openlcb_msg()` and `send_can_message()`: transmission queuing contracts
- Node enumeration callbacks: how the main loop iterates all nodes
- How to wire up a new protocol handler

### Chapter 7 - Utilities
- `openlcb_utilities.h` function families:
  - Message loading/clearing helpers
  - Big-endian payload insert/extract (8, 16, 32, 48, 64 bit)
  - Event range generation and testing (power-of-2 ranges)
  - Broadcast time event encoding/decoding
  - Train search event encoding/decoding
  - Configuration memory buffer operations
  - Address filtering logic (global vs addressed messages)

---

## Part III: State Machines — Transport-Independent (the Engine Room)

### Chapter 8 - Main Dispatcher State Machine (Transport-Independent)
- `openlcb_main_statemachine.h`: the central message router — operates entirely on `openlcb_msg_t`, no CAN knowledge
- Execution model: one step per call, no blocking
- _Thread safety: lock/unlock around FIFO pop and buffer operations (see Chapter 6)_
- Priority order: send pending outgoing -> re-enumerate -> pop incoming from FIFO -> enumerate first node -> enumerate next node
- How an incoming message fans out to all nodes (enumeration loop)
- MTI-based dispatch table: which callback handles which MTI
- The `openlcb_statemachine_info_t` context struct: current node, incoming msg, outgoing worker buffer, tick
- Worker buffer: STREAM-sized temporary assembly area for building responses
- The `enumerate` flag: how one incoming message generates multiple outgoing responses (e.g. multi-event identify)
- Flowchart reference: MainStatemachineFlowCharts.pdf

### Chapter 9 - Login State Machine (Transport-Independent)
- `openlcb_login_statemachine.h`: the node startup sequence — handles the OpenLCB-level login (Initialization Complete, event announcements) independent of transport
- Full state progression: INIT -> GENERATE_SEED -> GENERATE_ALIAS -> CID7/6/5/4 -> WAIT_200ms -> RID -> AMD -> INITIALIZATION_COMPLETE -> CONSUMER_EVENTS -> PRODUCER_EVENTS -> LOGIN_COMPLETE -> RUN
- Note: the early states (GENERATE_SEED through AMD) are driven by the CAN transport; the later states (INITIALIZATION_COMPLETE onward) are pure OpenLCB
- Alias generation: LFSR algorithm, 12-bit alias from 48-bit seed (CAN-specific concern)
- Collision handling: what triggers re-seed, how duplicate aliases are resolved
- The 200ms wait window and why it matters
- Event announcement: how all consumer/producer events are broadcast at login
- Multi-node login: round-robin through all nodes not yet in RUN state

---

## Part IIIb: State Machines — CAN Transport

### Chapter 10 - CAN Transport State Machine (CAN-Specific)
- `can_main_statemachine.h`: the CAN-layer orchestrator — a self-contained subsystem with its own buffer pool, FIFO, state machines, and spec-level interactions that runs autonomously before handing off `openlcb_msg_t` to the core
- Priority order: duplicate alias resolution -> outgoing CAN frame -> login CAN frame -> node enumeration
- How OpenLCB messages become CAN frames (fragmentation for datagrams, streams)
- How CAN frames become OpenLCB messages (assembly, multi-frame tracking)
- CAN identifier layout: 29-bit extended ID bit-field map
- Control frames vs. OpenLCB frames (bit 27 selector)
- Frame types: CID 7-4, RID, AMD, AME, AMR, Standard, Datagram Only/First/Middle/Final, Stream

### Chapter 11 - CAN RX and TX Pipelines (CAN-Specific)
- `can_rx_statemachine.h` / `can_rx_message_handler.h`: CAN frame-to-OpenLCB message conversion
  - Multi-frame datagram assembly with timeout tracking
  - How `state.inprocess` and `timer.assembly_ticks` manage in-flight datagrams
  - Output: transport-independent `openlcb_msg_t` pushed to the OpenLCB FIFO
  - _Thread safety: runs in ISR/RX context, writes to shared structures (see Chapter 6)_
- `can_tx_statemachine.h` / `can_tx_message_handler.h`: OpenLCB message-to-CAN frame conversion
  - Fragmentation logic for datagrams (Only/First/Middle/Final)
  - CAN header construction from MTI + aliases

---

## Part IV: Protocol Handlers

### Chapter 12 - Message Network Protocol
- `protocol_message_network.h`
- Initialization Complete (full vs simple)
- Verify Node ID (addressed and global) and Verified Node ID responses
- Protocol Support Inquiry/Reply: the 48-bit capability bitmap
- Optional Interaction Rejected: when and how it's generated
- Terminate Due to Error

### Chapter 13 - Event Transport Protocol
- `protocol_event_transport.h`
- Consumer/Producer Identified messages (Unknown, Set, Clear, Reserved, Range)
- Identify Consumer/Producer: automatic response from event lists
- Learn Event message
- PC Event Report and PC Event Report with Payload
- Event ranges: power-of-2 encoding, how ranges are matched
- How event state (set/clear) is tracked and reported
- Application callbacks for event handling

### Chapter 14 - Datagram Protocol
- `protocol_datagram_handler.h`
- Multi-frame CAN assembly (Only/First/Middle/Final frame types)
- Two-phase acknowledgment: Datagram Received OK (with Reply Pending flag + timeout) then data reply
- Retry logic: `timer.datagram` retry counter, up to 7 retries
- Per-address-space routing: how the first payload byte selects the handler
- Address space overview: CDI (0xFF), All (0xFE), Config (0xFD), ACDI Mfg (0xFC), ACDI User (0xFB), Train FDI (0xFA), Train Fn Config (0xF9)
- Read/Write/Write-Under-Mask command structure
- How to add a custom address space handler

### Chapter 15 - Simple Node Information Protocol (SNIP)
- `protocol_snip.h`
- Response format: 8 fields (mfg version, name, model, hw version, sw version, user version, user name, user description)
- How manufacturer fields come from `node.parameters.snip` and user fields from config memory
- Individual field loader functions
- Validation: `ProtocolSnip_validate_snip_reply()`

### Chapter 16 - Configuration Description Information (CDI)
- What CDI is: XML device self-description
- How CDI is served via the datagram address space 0xFF
- CDI structure: identification, segments, groups, integer/string/eventid entries
- Relationship between CDI definition and configuration memory layout

### Chapter 17 - Train Control Protocol
- `protocol_train_handler.h`
- `train_state_t` structure: speeds (set/commanded/actual as float16), E-stop flags, controller assignment, heartbeat, functions, DCC address
- Control commands: speed, functions (individual and range), emergency stop
- Controller assignment and reservation model (owner_node, reserved_node_count)
- Consist (listener) system: attach/detach, forwarding flags (reverse, link F0, link Fn, hide)
- Listener enumeration for command forwarding
- Heartbeat timeout mechanism
- Global emergency events (E-Stop 0x01000000FFFFFFFF, Emergency Off 0x01000000FFFFFFFE)
- Train search protocol: event ID encoding with 6 nibbles + flags

### Chapter 18 - Broadcast Time Protocol
- Event-based time distribution
- Clock ID, event type encoding in 8-byte Event IDs
- Time/Date/Year/Rate field encoding (12-bit packed values)
- `broadcast_clock_state_t` tracking structure
- Commands: Query, Start, Stop, Date Rollover
- Utility functions for encoding/decoding

---

## Part V: CAN Driver Layer (All CAN-Specific)

### Chapter 19 - Alias Mapping: Global Table
- `alias_mappings.h` / `alias_mappings.c`: the authoritative alias-to-Node-ID registry for all nodes on the CAN network
- `alias_mapping_t` structure: `node_id`, `alias`, `is_duplicate`, `is_permitted` flags
- `alias_mapping_info_t`: the table array plus `has_duplicate_alias` fast-check flag
- Registration (`AliasMappings_register`), bidirectional lookup by alias or Node ID, unregistration
- How duplicate aliases are detected (ISR sets `is_duplicate`) and resolved (main loop resets node, re-allocates)
- The alias lifecycle: generate -> check (CID) -> reserve (RID) -> define (AMD) -> release (AMR)
- _Thread safety: shared between ISR/RX and main loop — a key lock/unlock scenario (see Chapter 6)_

### Chapter 20 - Alias Mapping: Listener Table for Consist Support
- `alias_mapping_listener.h` / `alias_mapping_listener.c`: on-demand alias table specifically for consist listener nodes
- `listener_alias_entry_t` structure: `node_id` + `alias` (0 = not yet resolved)
- Table size: `USER_DEFINED_MAX_LISTENERS_PER_TRAIN * USER_DEFINED_TRAIN_NODE_COUNT`
- **Purpose differs from global table:** maps remote listener Node IDs to their CAN aliases so consist commands can be forwarded
- Population flow: Train Listener Attach arrives -> register Node ID with alias=0 -> send AME query -> AMD arrives -> alias resolved -> held messages released
- `ListenerAliasTable_flush_aliases()`: clears all aliases but preserves Node IDs (on global AME)
- `ListenerAliasTable_clear_alias_by_alias()`: clears one alias (on AMR)
- How the TX path uses this table to resolve destination aliases for consist command forwarding
- Can be NULL in the callback interface if consist feature is not needed

### Chapter 21 - Multi-Node Message Routing (In Development)
- How multiple virtual nodes share a single CAN interface
- Per-node alias allocation via round-robin login
- Current gap: outgoing messages from one virtual node are not automatically delivered to other local virtual nodes
- The loopback problem: when virtual node A sends a message that virtual node B should also receive, the CAN driver does not push it back to the RX path
- Approaches under consideration for local message routing
- Impact on event transport, datagrams, and train control between co-hosted nodes

### Chapter 22 - CAN Login Sequence
- `can_login_statemachine.h` / `can_login_message_handler.h`
- CID frame construction: how 48-bit Node ID is split across CID 7/6/5/4 frames
- LFSR-based seed generation (per OpenLCB CAN Frame Transfer Standard)
- RID and AMD frame construction
- Integration with the login state machine (Chapter 9)
- Timing requirements and collision detection
- `on_alias_change` optional callback for alias resolution notifications

---

## Part VI: Integration and Debugging

### Chapter 23 - Wiring It All Together
- Template application walkthrough: what the application must provide
- Initialization sequence: buffer stores -> nodes -> alias mappings -> login SM -> CAN SM -> main SM -> protocols
- The main loop: calling `CanMainStateMachine_run()` and `OpenLcbMainStatemachine_run()` and feeding CAN frames
- How a TCP/IP transport would wire differently (no CAN SM, no alias mappings, direct OpenLCB message exchange)
- 100ms timer tick: who provides it, who consumes it, what it drives
- Implementing `lock_shared_resources()` / `unlock_shared_resources()` for your platform (see Chapter 6 for full details)

### Chapter 24 - Data Flow Walkthrough
- Trace (CAN): incoming CAN frame -> CAN FIFO -> RX handler -> `openlcb_msg_t` -> main dispatcher -> protocol handler -> response `openlcb_msg_t` -> TX handler -> CAN frame(s)
- Trace (TCP/IP, future): incoming TCP message -> `openlcb_msg_t` -> main dispatcher -> protocol handler -> response `openlcb_msg_t` -> TCP send
- Trace: application sends event -> buffer allocate -> main dispatcher outgoing -> transport TX
- Trace: node login from power-on to RUN state (CAN-specific alias steps vs. transport-independent OpenLCB steps)

### Chapter 25 - Debugging Guide
- Buffer pool exhaustion: symptoms, diagnosis, tuning
- Alias collisions (CAN-specific): how they manifest, log points
- Datagram assembly timeout: causes and fixes
- State machine stuck: how to identify which SM is blocked and why
- Using peak allocation counters
- Common mistakes when adding new protocols or nodes
- Google Test patterns: how tests mock the callback interfaces

---

## Part VII: Extending the Library

### Chapter 26 - Adding a New Protocol
- Step-by-step: define MTI, create callback interface, register with main dispatcher, implement handler, write tests
- Protocol handlers live in `openlcb/` and must remain transport-independent

### Chapter 27 - Adding a New Transport
- How to create a new driver folder alongside `canbus/` (e.g. `drivers/tcpip/`)
- What a transport must provide: convert between its wire format and `openlcb_msg_t`
- CAN transport requirements: alias management, frame fragmentation/assembly, login sequence
- TCP/IP transport requirements: pure OpenLCB messages, no aliases, no fragmentation, simpler login
- The boundary: transport drivers produce and consume `openlcb_msg_t`; everything above that is shared

### Chapter 28 - Porting to a New Platform
- What the application must provide: transport driver, timer, persistent storage, lock/unlock
- Bare-metal vs. RTOS considerations
- Memory tuning for constrained targets

---

## Appendices

### Appendix A - MTI Quick Reference Table
- All MTI values, names, addressing mode, and which module handles them

### Appendix B - CAN Frame Format Reference
- 29-bit identifier bit-field diagram
- All control frame types with field layouts
- Datagram/Stream frame sequencing

### Appendix C - Configuration Memory Address Space Map
- All address spaces (0xF9-0xFF) with purpose, read/write permissions, typical sizes

### Appendix D - Event ID Encoding Reference
- Well-known event IDs (global E-stop, emergency off)
- Broadcast time event encoding
- Train search event encoding
- Range encoding

### Appendix E - Compile-Time Configuration Reference
- All `USER_DEFINED_*` macros with purpose, default values, and tuning guidance

### Appendix F - OpenLCB Specification Traceability

**Purpose:** Verify spec coverage, identify unimplemented requirements, aid compliance auditing.

**Spec documents** (Standards "S" + Technical Notes "TN") located in `OpenLcb Documents/standards/`. Unreleased standards (Stream Transport, TCP/IP Transport) are in `drafts/` — not yet implemented.

**Columns:** Spec Section | Requirement Summary | Implementing Source File(s) | Doc Chapter

---

#### F.1 — Message Network Standard (`MessageNetworkS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 3.1 | All messages shall contain source Node ID and MTI | `openlcb_types.h` (openlcb_msg_t) | 3 |
| 3.1.2 | Message fully decodable from MTI flag bits | `openlcb_defines.h` | 3 |
| 3.2 | Nodes start in Uninitialized state | `openlcb_node.h` (openlcb_node_state_t) | 5 |
| 3.2 | Uninitialized node shall not transmit except Init Complete | `openlcb_login_statemachine.c` | 9 |
| 3.3 | Node ID in data content shall be full 48-bit on all transports | `openlcb_utilities.c` (payload insert helpers) | 7 |
| 3.3.4 | Nodes shall process OIR even if not all contents provided | `protocol_message_network.c` | 12 |
| 3.3.5 | Nodes shall process Terminate Due to Error even if incomplete | `protocol_message_network.c` | 12 |
| 3.3.7 | Missing Protocol Support Reply bits interpreted as zero | `protocol_message_network.c` | 12 |
| 3.3.7 | Reserved protocol bits sent as 0, ignored on receipt | `protocol_message_network.c` | 12 |
| 3.4 | All nodes shall take part in all standard interactions | `openlcb_main_statemachine.c` (dispatch) | 8 |
| 3.4.1 | Newly functional nodes shall send Init Complete, enter Initialized | `openlcb_login_statemachine.c` | 9 |
| 3.4.1 | No OpenLCB messages before Init Complete | `openlcb_login_statemachine.c` | 9 |
| 3.4.2 | Addressed Verify Node ID → reply Verified Node ID | `protocol_message_network.c` | 12 |
| 3.4.2 | Global Verify Node ID (no optional ID) → reply Verified Node ID | `protocol_message_network.c` | 12 |
| 3.5.1 | Unknown addressed MTI → send OIR | `openlcb_main_statemachine.c` (NULL handler) | 8 |
| 3.5.1 | OIR shall contain fields per section 3.3.4 | `protocol_message_network.c` | 12 |
| 3.5.3 | Error during addressed interaction → send Terminate Due to Error | `protocol_message_network.c` | 12 |
| 3.5.3 | Node shall reset state after sending/receiving Terminate Due to Error | `protocol_message_network.c` | 12 |
| 3.5.4 | Detect incoming message with own Source Node ID → indicate error | `protocol_message_network.c` | 12 |
| 3.5.5 | Error type flags: zero or one bit selected | `openlcb_defines.h` | 3 |
| 3.6 | Addressed messages routed to addressed node | `openlcb_main_statemachine.c`, `openlcb_utilities.c` | 8 |
| 3.6 | Global messages forwarded to all nodes | `openlcb_main_statemachine.c` (enumeration) | 8 |
| 3.7 | Protocol messages sent within 750ms | Application timer tick | 23 |
| 3.7 | Timeout mechanism shall not be shorter than 3 seconds | `openlcb_defines.h` (timer constants) | 3 |
| 4.1.4 | Simple nodes use Simple Set Sufficient Init Complete (0x0101) | `openlcb_login_statemachine_handler.c` | 9 |
| 7.3.1.3 | First/Middle CAN frames carry 8 data bytes | `can_tx_message_handler.c` | 11 |
| 7.3.1.3 | Last/Only CAN frames have 2-8 data bytes | `can_tx_message_handler.c` | 11 |
| 7.3.7 | CAN message frames sent consecutively, no lower-priority interspersed | `can_tx_statemachine.c` | 11 |

---

#### F.2 — Event Transport Standard (`EventTransportS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 5 | Messages only sent when node is in Initialized state | `openlcb_login_statemachine.c` (state check) | 9 |
| 6 | Before sending PCER, node shall emit Producer Identified | `openlcb_login_statemachine.c` (event announcement) | 9 |
| 6 | Consumer shall emit Consumer Identified to ensure PCER receipt | `openlcb_login_statemachine.c` (event announcement) | 9 |
| 6.1 | To produce event, emit PCER with specific Event ID | `protocol_event_transport.c` | 13 |
| 6.1 | Consumers check for match, perform local ops if matched | `protocol_event_transport.c` | 13 |
| 6.1 | No local ops if no match | `protocol_event_transport.c` | 13 |
| 6.2 | On Identify Events (global or addressed), reply with all Producer/Consumer Identified | `protocol_event_transport.c` | 13 |
| 6.3 | On Identify Producer, reply with Producer Identified if produced | `protocol_event_transport.c` | 13 |
| 6.4 | On Identify Consumer, reply with Consumer Identified if consumed | `protocol_event_transport.c` | 13 |
| 6.6 | After reset to defaults, every producer/consumer has valid unique Event ID | Application responsibility | 13 |
| 7 | PCER with payload CAN frames sent together, no delays | `can_tx_statemachine.c` | 11 |
| 7 | Nodes consuming PCER with payload shall handle at least 2 overlapping | `can_rx_message_handler.c` | 11 |

---

#### F.3 — Datagram Transport Standard (`DatagramTransportS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4.2 | Reserved Datagram OK flag bits sent as zero, ignored on receipt | `protocol_datagram_handler.c` | 14 |
| 4.2 | Datagram OK without Flags byte treated as zero flags | `protocol_datagram_handler.c` | 14 |
| 4.3 | Nodes accept/process Datagram Rejected with incomplete error code | `protocol_datagram_handler.c` | 14 |
| 6 | Valid Datagram received → send Datagram OK or Datagram Rejected | `protocol_datagram_handler.c` | 14 |
| 6.1 | Shall not send second Datagram to same node before reply/timeout | `protocol_datagram_handler.c` (retry logic) | 14 |
| 6.2 | Permanent Error → abandon, do not resend | `protocol_datagram_handler.c` | 14 |
| 7.2 | Maintain Datagram-started state for each receiving datagram | `can_rx_message_handler.c` (inprocess flag) | 11 |
| 7.2 | Overlapping datagrams from different sources → independent states | `can_rx_message_handler.c` | 11 |
| 7.3.1 | No lower-priority frames between datagram frames | `can_tx_statemachine.c` | 11 |
| 7.3.1 | Valid datagram frame sequence → send Datagram OK or Rejected | `protocol_datagram_handler.c` | 14 |
| 7.3.2 | Invalid datagram frame sequence → Datagram Rejected, Temporary Error | `can_rx_message_handler.c` | 11 |

---

#### F.4 — CAN Frame Transfer Standard (`CanFrameTransferS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 3 | Each node shall have unique identifier used as NodeID | `openlcb_node.h` | 5 |
| 4 | CAN extended format only (29-bit header) | `can_types.h`, `can_utilities.c` | 3b |
| 4 | Operate properly with standard-format (11-bit) frames on segment | `can_rx_statemachine.c` | 11 |
| 4 | Shall not transmit extended-format remote frames (RTR) | `can_tx_message_handler.c` | 11 |
| 4 | MSB of CAN header transmitted as 1, ignored on receipt | `can_utilities.c` | 3b |
| 4 | Frame contains 0-8 bytes of data after header | `can_types.h` | 3b |
| 5 | Nodes start in Inhibited state | `openlcb_node.h` (runstate), `can_login_statemachine.c` | 22 |
| 5 | Inhibited node transmits only CID, RID, AMD frames | `can_login_statemachine.c` | 22 |
| 6 | Control frames carried with 0 in Frame Type field | `can_types.h`, `can_utilities.c` | 3b |
| 6.1 | Reserved control frame types not sent, ignored on receipt | `can_rx_statemachine.c` | 11 |
| 6.2.1 | Alias reservation: CID7/6/5/4 → wait 200ms → RID | `can_login_statemachine.c`, `can_login_message_handler.c` | 22 |
| 6.2.1 | Restart reservation if matching alias received before completion | `can_rx_message_handler.c` (collision) | 22 |
| 6.2.1 | Restart reservation on TX error | `can_login_statemachine.c` | 22 |
| 6.2.2 | Inhibited→Permitted: reserved alias + transmit AMD | `can_login_message_handler.c` | 22 |
| 6.2.3 | AME with matching Node ID → reply AMD | `can_rx_message_handler.c` | 11 |
| 6.2.3 | AME with no data → reply AMD (if Permitted) | `can_rx_message_handler.c` | 11 |
| 6.2.3 | Inhibited node shall not reply to AME | `can_rx_message_handler.c` | 11 |
| 6.2.3 | AME with no data → discard cached alias mappings | `alias_mappings.c`, `alias_mapping_listener.c` | 19, 20 |
| 6.2.4 | Permitted→Inhibited: transmit AMR | `can_main_statemachine.c` | 10 |
| 6.2.4 | On receiving AMR, stop using that alias within 100ms | `alias_mappings.c` | 19 |
| 6.2.5 | Compare source alias in each received frame against reserved aliases | `can_rx_statemachine.c` | 11 |
| 6.2.5 | Alias collision on CID → send RID | `can_rx_message_handler.c` | 11 |
| 6.2.5 | Alias collision on non-CID (Permitted) → AMR, transition Inhibited | `can_rx_message_handler.c`, `can_main_statemachine.c` | 10, 11 |
| 6.2.5 | Alias collision on non-CID (Inhibited) → stop using alias | `can_rx_message_handler.c` | 11 |
| 6.2.6 | Compare Node ID in AMD against own → duplicate detection | `can_rx_message_handler.c`, `alias_mappings.c` | 11, 19 |
| 6.3 | Alias values shall not be zero | `can_login_message_handler.c` (LFSR) | 22 |
| 6.3 | Same-type nodes with close Node IDs → different first aliases | `can_login_message_handler.c` (LFSR) | 22 |
| 6.3 | >99% probability next alias differs after collision | `can_login_message_handler.c` (LFSR) | 22 |

---

#### F.5 — Simple Node Information Protocol Standard (`SimpleNodeInformationS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 5.1 | Manufacturer info version 0x01 interpreted as 0x04 | `protocol_snip.c` | 15 |
| 5.1 | Mfg name ≤41 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.1 | Model name ≤41 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.1 | HW version ≤21 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.1 | SW version ≤21 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.1 | User info version 0x01 interpreted as 0x02 | `protocol_snip.c` | 15 |
| 5.1 | User name ≤63 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.1 | User description ≤64 bytes null-terminated | `protocol_snip.c` | 15 |
| 5.2 | Accept data with any version number; ignore extra strings for higher | `protocol_snip.c` | 15 |
| 6.1 | SNIP support indicated in Protocol Support Reply | `protocol_message_network.c` | 12 |
| 6.2 | Requesting node handles reply in one or more messages | `protocol_snip.c` | 15 |
| 6.3 | SNIP Reply constant; change requires re-initialization | `protocol_snip.c`, application | 15 |

---

#### F.6 — Memory Configuration Standard (`MemoryConfigurationS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4.1 | 32-bit addresses, byte-addressable | `openlcb_defines.h`, `protocol_config_mem_read_handler.c` | 14 |
| 4.2 | Address space not partially read-only/write-only | Application config | 14 |
| 4.3 | Error codes per Message Network Standard | `openlcb_defines.h` | 3 |
| 4.3 | Unknown command → datagram rejected | `protocol_datagram_handler.c` | 14 |
| 4.4 | Read command → Reply Pending in Datagram OK | `protocol_config_mem_read_handler.c` | 14 |
| 4.5 | Read reply may return less data but at least 1 byte | `protocol_config_mem_read_handler.c` | 14 |
| 4.5 | Read failure → Fail bit + error code | `protocol_config_mem_read_handler.c` | 14 |
| 4.8 | Write with delay/failure → Reply Pending in Datagram OK | `protocol_config_mem_write_handler.c` | 14 |
| 4.9 | Write Reply failure → Fail bit + error code | `protocol_config_mem_write_handler.c` | 14 |
| 4.10 | Write Under Mask response consistent with Write | `protocol_config_mem_write_handler.c` | 14 |
| 4.14 | Reserved Get Config Options bits sent as zero, ignored on receipt | `protocol_config_mem_operations_handler.c` | 14 |
| 4.19 | Get Unique ID: each request provides different Event ID | Application callback | 14 |
| 4.24 | Reset/Reboot → power-on-reset state | `protocol_config_mem_operations_handler.c` | 14 |
| 4.25 | Factory Reset: Node ID in payload for redundancy | `protocol_config_mem_operations_handler.c` | 14 |

---

#### F.7 — Train Control Protocol Standard (`TrainControlS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 3 | Train Control node shall implement Event Transport | `protocol_event_transport.c` | 13 |
| 3 | Train Control node shall implement SNIP | `protocol_snip.c` | 15 |
| 4.3 | P bit = 0 for Throttle→Train, P bit = 1 for Train→Listener | `protocol_train_handler.c` | 17 |
| 4.4 | Unknown speed in Query Speed reply → float16 NaN (0xFFFF) | `openlcb_float16.c` | 7 |
| 5 | Updating listener flags shall not change listener order | `protocol_train_handler.c` | 17 |
| 5 | E-Stop state entered on E-Stop command, exited on Set Speed | `protocol_train_handler.c`, `openlcb_application_train.c` | 17 |
| 5 | Global E-Stop entered on well-known event | `protocol_train_handler.c` | 17 |
| 5 | Global E-Off entered on well-known event | `protocol_train_handler.c` | 17 |
| 6.1 | Throttle shall assign itself as Controller before operations | `openlcb_application_train.c` | 17 |
| 6.1 | Non-Controller commands → Terminate Due to Error | `protocol_train_handler.c` | 17 |
| 6.1 | Listener messages (P=1) shall never be rejected | `protocol_train_handler.c` | 17 |
| 6.1 | Assign Controller → reply with Result=0 | `protocol_train_handler.c` | 17 |
| 6.1 | Assign Controller reply within 3 seconds | `protocol_train_handler.c` | 17 |
| 6.2 | E-Stop → Set Speed 0 (preserve direction), Commanded Speed 0 | `protocol_train_handler.c` | 17 |
| 6.2 | Train stopped while in any E-Stop state | `protocol_train_handler.c` | 17 |
| 6.3 | Binary function: 0=off, 1=on | `protocol_train_handler.c` | 17 |
| 6.4 | Train nodes produce "Is Train" well-known event | `protocol_event_transport.c`, application | 13 |
| 6.5 | Forwarded message to Listeners → P bit = 1 | `protocol_train_handler.c` | 17 |
| 6.5 | Set speed always forwarded; Reverse flag flips direction | `protocol_train_handler.c` | 17 |
| 6.5 | Function 0 forwarded only if "Link F0" flag set | `protocol_train_handler.c` | 17 |
| 6.5 | Other functions forwarded only if "Link Fn" flag set | `protocol_train_handler.c` | 17 |
| 6.5 | Attach/Detach response contains same Node ID + error code | `protocol_train_handler.c` | 17 |
| 6.6 | No heartbeat if Set Speed is zero | `openlcb_application_train.c` | 17 |
| 6.6 | Any Controller command clears heartbeat timer | `protocol_train_handler.c` | 17 |
| 6.6 | No command within deadline → Set Speed 0 | `protocol_train_handler.c` | 17 |
| 6.6 | Heartbeat Set Speed 0 forwarded to all Listeners | `protocol_train_handler.c` | 17 |

---

#### F.8 — Train Search Protocol Standard (`TrainSearchS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 5.1 | Event ID range 09.00.99.FF.xx reserved for Train Search | `openlcb_defines.h` | 3 |
| 5.2 | Search query: 6 nibbles MSB-first in bytes 5-7 | `openlcb_utilities.c` (train search encoding) | 7 |
| 5.2 | Short queries padded with 0xF nibbles | `openlcb_utilities.c` | 7 |
| 6.1 | Throttle sends Identify Producer with search Event ID | `openlcb_application_train.c` | 17 |
| 6.1 | Train node compares properties to search criteria | `protocol_train_search_handler.c` | 17 |
| 6.1 | Match → emit Producer Identified | `protocol_train_search_handler.c` | 17 |
| 6.1 | No match → no reply | `protocol_train_search_handler.c` | 17 |
| 6.2 | Allocate bit set → Command Station creates Train Node if no reply in 200ms | `protocol_train_search_handler.c` | 17 |

---

#### F.9 — Broadcast Time Protocol Standard (`BroadcastTimeS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4 | Upper 6 bytes of event ID from specified clock range | `openlcb_defines.h`, `openlcb_utilities.c` | 3, 7 |
| 4.12 | Undefined byte 6/7 values reserved; not sent, ignored on receipt | `protocol_broadcast_time_handler.c` | 18 |
| 6.1 | Clock generator sends Producer/Consumer Range Identified at startup | `openlcb_application_broadcast_time.c` | 18 |
| 6.2 | Running clock sends Report Time ≤ once/min, ≥ once/hour | `openlcb_application_broadcast_time.c` | 18 |
| 6.2 | Date rollover: send Date Rollover Event, then Year+Date events after 3s | `openlcb_application_broadcast_time.c` | 18 |
| 6.3 | Synchronization sequence: Start/Stop, Rate, Year, Date, Time, next-minute | `openlcb_application_broadcast_time.c` | 18 |
| 6.4 | Query Event → respond with sync sequence | `protocol_broadcast_time_handler.c` | 18 |
| 6.5 | Set commands → change effective immediately, produce Report event | `protocol_broadcast_time_handler.c` | 18 |
| 6.5 | 3 seconds after last Set command → full sync sequence | `openlcb_application_broadcast_time.c` | 18 |

---

#### F.10 — Configuration Description Information Standard (`ConfigurationDescriptionInformationS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4 | CDI invariant while node has Initialized connections | Application, `protocol_config_mem_read_handler.c` | 16 |
| 5 | CDI shall validate against schema v1.4 | Application (CDI XML content) | 16 |
| 5.1.1 | SNIP Reply matches CDI identification element | `protocol_snip.c`, application | 15, 16 |
| 5.1.2 | ACDI space version values match acdi element attributes | Application config | 16 |
| 5.1.2 | acdi specified iff Protocol Support Reply ACDI bit set | `protocol_message_network.c` | 12 |
| 5.1.3 | Segment 'space' attribute applies to all data elements within | Application (CDI structure) | 16 |
| 5.1.4.2 | Integer stored unsigned unless min < 0, big-endian | Application config memory | 16 |
| 5.1.4.3 | String null-terminated in config memory | Application config memory | 16 |
| 5.1.4.4 | Event ID stored big-endian | `openlcb_utilities.c` | 7 |
| 6 | Config tools process any earlier CDI version | Application (CT responsibility) | 16 |

---

#### F.11 — Function Description Information Standard (`FunctionDescriptionInformationS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4 | FDI constant while node Initialized | Application, `protocol_config_mem_read_handler.c` | 16 |
| 5 | FDI validates against schema v1.0 | Application (FDI XML content) | 16 |
| 5.1.1 | fdi element contains exactly one segment | Application FDI content | 16 |
| 5.1.4 | Function number 0-16777215 | `protocol_train_handler.c` | 17 |

---

#### F.12 — Firmware Upgrade Standard (`FirmwareUpgradeS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| States | In Firmware Upgrade state, node still standards compliant | Application responsibility | 14 |
| States | Operating state shall not export writable Firmware Space | Application config | 14 |
| 5.2 | No direct Firmware-Operating transition without Uninitialized | Application, `openlcb_login_statemachine.c` | 9 |
| 5.2 | Power-up default to Operating state | Application | 23 |
| 5.2 | Freeze command → transition to Firmware Upgrade, send Init Complete | `protocol_config_mem_operations_handler.c` | 14 |
| 5.2 | Unfreeze command → transition to Operating, send Init Complete | `protocol_config_mem_operations_handler.c` | 14 |
| 5.3 | Firmware Upgrade Protocol bit in Protocol Support Reply | `protocol_message_network.c`, `openlcb_defines.h` | 12 |
| 5.4.2 | Datagram transfer: 64-byte writes starting at offset 0 | `protocol_config_mem_write_handler.c` | 14 |

---

#### F.13 — Unique Identifiers Standard (`UniqueIdentifiersS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4 | Unique Identifier is 6 bytes, MSB transmitted first | `openlcb_types.h` (uint64_t node_id) | 3 |
| 4 | Shall include one or more 1 bits (not all zeros) | Application (ID assignment) | 23 |
| 4 | Every node shall have a Unique Identifier as Node ID | `openlcb_node.h` | 5 |
| 5 | Allocated using mechanisms in this section | Application (ID assignment) | 23 |
| 5.4 | Manufacturers ensure uniqueness | Application (ID assignment) | 23 |

---

#### F.14 — Event Identifiers Standard (`EventIdentifiersS.pdf`)

| Spec Section | Requirement | Source File(s) | Ch |
|---|---|---|---|
| 4 | Event identifier is 8 bytes, MSB first | `openlcb_types.h` (event_id_t) | 3 |
| 4 | Byte order significant, MSB transmitted first | `openlcb_utilities.c` (big-endian helpers) | 7 |
| 5 | Well-known events allocated per table | `openlcb_defines.h` | 3 |

---

#### F.15 — CAN Physical Layer Standard (`CanPhysicalS.pdf`)

Hardware-level requirements (bus termination, cable, connector, bit rate). Implemented by CAN transceiver hardware, not by this library. Application wiring guide references these (Ch 23, 28).

---

#### F.16 — Unreleased Standards (in `drafts/`)

| Standard | Status |
|---|---|
| Stream Transport | Draft — not yet released, not implemented |
| TCP/IP Transport | Draft — not yet released, not implemented |

### Appendix G - Glossary
- OpenLCB/LCC terminology: Node ID, alias, MTI, CDI, SNIP, PCER, datagram, etc.
