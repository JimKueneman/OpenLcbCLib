# Stream Transport Protocol -- Working Implementation Plan

**Status:** COMPLETE -- All phases done. Stream handler, CAN drivers, config-mem
stream handler, and full unit test suites implemented and passing.

**Source spec:** OpenLCB Stream Transport Standard, Feb 1 2026 (Preliminary)
and OpenLCB Stream Transport Technical Note, Feb 1 2026 (Preliminary).
Retrieved from bobjacobsen/documents branch bobjacobsen-update-stream-docs.

---

## Context

The Stream Transport Protocol enables unidirectional bulk data transfer between
two OpenLCB nodes with flow control.  The primary consumer is Memory
Configuration Protocol streaming reads/writes for large address spaces (CDI,
firmware).

All dispatch infrastructure already existed in the library:
- 5 MTI defines: MTI_STREAM_INIT_REQUEST (0x0CC8), MTI_STREAM_INIT_REPLY
  (0x0868), MTI_STREAM_SEND (0x1F88), MTI_STREAM_PROCEED (0x0888),
  MTI_STREAM_COMPLETE (0x08A8)
- CAN frame type 7 routing in RX/TX statemachines
- 5 DI function pointers in openlcb_main_statemachine.h (were NULL ->
  Interaction Rejected)
- STREAM buffer pool with configurable size/depth
- OPENLCB_COMPILE_STREAM feature flag

All phases added: protocol_stream_handler.c/.h, CAN TX/RX handlers, wiring in
openlcb_config.c, stream callbacks in openlcb_config_t, error code defines,
config-mem stream handler, and comprehensive unit tests.

---

## Spec Summary (Feb 2026 Standard)

### States
- **Closed/Non-existent** -- no active stream between the node pair
- **Announced** -- source announced SID to destination via higher-level
  protocol (e.g. datagram)
- **Initiated** -- source sent Stream Initiate Request, waiting for reply
- **Open** -- destination accepted with Stream Initiate Reply

### Message Payload Layouts (Feb 2026 Standard)

**Stream Initiate Request** (MTI 0x0CC8, source -> dest, 5/6/12 bytes):

| Bytes 0-1 | Bytes 2-3 | Byte 4 | Byte 5 | Bytes 6-11 |
|-----------|-----------|--------|--------|------------|
| Max Buffer Size | Flags (bit 0 deprecated, rest reserved) | Source Stream ID | Optional: Proposed DID | Optional: Stream Content UID |

- Bit 0 of flags is deprecated (send 0, ignore on receipt)
- Proposed DID only if previously negotiated via higher-level protocol
- Stream Content UID is optional 6-byte globally unique content type ID

**Stream Initiate Reply** (MTI 0x0868, dest -> source, 6+ bytes):

| Bytes 0-1 | Bytes 2-3 | Byte 4 | Byte 5 | Bytes 6+ |
|-----------|-----------|--------|--------|----------|
| Max Buffer Size (0=reject) | Flags/Error Code | Source Stream ID | Dest Stream ID | Optional: error string |

Flags/Error Code values (standard OpenLCB error codes):
- 0x8000 Accept, no errors
- 0x0000 Accept, no errors (alternate)
- 0x1000 Permanent error
- 0x1010 Streams not supported
- 0x1020 Source not permitted
- 0x1040 Unimplemented
- 0x2000 Temporary error
- 0x2020 Buffer unavailable
- 0x2040 Out of order

**Stream Data Send** (MTI 0x1F88, source -> dest):

| Byte 0 | Bytes 1+ |
|--------|----------|
| Dest Stream ID | Payload bytes |

**Stream Data Proceed** (MTI 0x0888, dest -> source, 2 or 4 bytes):

| Byte 0 | Byte 1 | Bytes 2-3 |
|--------|--------|-----------|
| Source Stream ID | Dest Stream ID | Optional Flags |

**Stream Data Complete** (MTI 0x08A8, source -> dest, 2 or 6 bytes):

| Byte 0 | Byte 1 | Bytes 2-3 | Bytes 4-9 |
|--------|--------|-----------|-----------|
| Source Stream ID | Dest Stream ID | Optional Flags | Optional: total bytes % 2^24 |

### Flow Control (Standard Section 7.3)
1. Source sends up to MaxBufSize bytes in Stream Data Send messages
2. Source pauses
3. Destination sends Stream Data Proceed when ready for another MaxBufSize
4. Source may resume sending MaxBufSize more bytes
5. Each Proceed extends the window by MaxBufSize (additive)
6. Destination MAY send one Proceed in advance (before full buffer received)
7. Source sends Stream Data Complete when done

### Buffer Size
- Max 2^16 - 1 = 65535 bytes (driven by 2-byte field in initiate messages)

---

## CAN Adaptation (Standard Section 8)

Stream Data Send on CAN uses Frame Type 7.  CAN header carries source/dest
aliases.

**RESOLVED:** CAN data byte 0 = Destination Stream ID (DID).  Remaining bytes
(up to 7) are payload.  This matches OpenMRN and is explicitly stated in
Standard Section 8.1.

All other stream messages (Initiate Request/Reply, Proceed, Complete) use
standard addressed message framing -- they are NOT Frame Type 7.  The existing
addressed message TX/RX handlers already support them.

---

## Changes from Apr 2021 Working Note to Feb 2026 Standard

1. **Initiate Request flags**: "Type Included" bit 0 is now deprecated (was
   used to signal content type in payload)
2. **Stream Content UID**: Now optional bytes 6-11 in Initiate Request itself
   (was previously signaled by flag and placed in stream data)
3. **Proposed DID**: New optional byte 5 in Initiate Request
4. **Initiate Reply error codes**: Changed from custom bit-field flags to
   standard OpenLCB error codes (0x8000, 0x1xxx, 0x2xxx)
5. **Optional error string**: Initiate Reply can now carry zero-terminated
   human-readable error string after byte 5
6. **Complete total bytes**: Now 6-byte optional field (was 4 bytes), and the
   spec says "% 2^24" (modular)
7. **CAN byte 0**: Confirmed as DID (was ambiguous in Working Note Section 3.3
   which said SID)

---

## Architecture

Follows the existing two-layer pattern:
- **Layer 1: protocol_stream_handler.c/.h** -- stateless MTI dispatcher at the
  OpenLCB level.  Receives the 5 stream MTIs from main statemachine, parses
  payload fields, manages stream state table, and dispatches to application
  callbacks via DI.
- **Layer 2: Application helpers** -- higher-level protocols (e.g. firmware
  upload, config memory streaming read) would use the stream handler's API.
  These are NOT part of this plan -- they are separate modules that wire into
  the stream handler's callbacks.

### Stream State Table

A fixed-size array of stream_state_t entries (configurable via
USER_DEFINED_MAX_STREAM_COUNT, default 1).  Each entry tracks:

```c
typedef struct {

    stream_state_enum state;
    uint8_t source_stream_id;
    uint8_t dest_stream_id;
    node_id_t remote_node_id;
    uint16_t max_buffer_size;
    uint32_t bytes_transferred;
    uint16_t bytes_remaining;
    bool is_source;
    uint8_t content_uid[6];

} stream_state_t;
```

Gated under #ifdef OPENLCB_COMPILE_STREAM.

### DI Interface (interface_protocol_stream_handler_t)

```c
typedef struct {

    void (*on_initiate_request)(openlcb_statemachine_info_t *info, stream_state_t *stream);
    void (*on_initiate_reply)(openlcb_statemachine_info_t *info, stream_state_t *stream);
    void (*on_data_received)(openlcb_statemachine_info_t *info, stream_state_t *stream);
    void (*on_data_proceed)(openlcb_statemachine_info_t *info, stream_state_t *stream);
    void (*on_complete)(openlcb_statemachine_info_t *info, stream_state_t *stream);

} interface_protocol_stream_handler_t;
```

---

## Style Guides

All code follows:
- documentation/style_guides/StyleGuide_v4.md
- documentation/style_guides/Doxygen_StyleGuide_v2_0.md

---

## Files Created / Modified

### New Files (Phase 1 -- DONE)

1. **src/openlcb/protocol_stream_handler.h** -- types, DI struct, public API
2. **src/openlcb/protocol_stream_handler.c** -- stream state table, 5 handlers

### Modified Files (Phase 1 -- DONE)

3. **src/openlcb/openlcb_defines.h** -- Added ERROR_PERMANENT_STREAMS_NOT_SUPPORTED
   (0x1010), STREAM_REPLY_ACCEPT (0x8000), STREAM_ID_RESERVED (0xFF)
4. **src/openlcb/openlcb_config.h** -- Added stream callbacks to openlcb_config_t,
   OPENLCB_COMPILE_STREAM verbose message, include for protocol_stream_handler.h
5. **src/openlcb/openlcb_config.c** -- _build_stream_handler(), initialize call,
   5 function pointers wired into main statemachine
6. **src/openlcb/CMakeLists.txt** -- Added protocol_stream_handler.c to SOURCES

### CAN Drivers (Phase 2 -- DONE)

7. **src/drivers/canbus/can_tx_message_handler.c** -- Implemented
   CanTxMessageHandler_stream_frame(): builds CAN frames with Frame Type 7,
   DID in data byte 0, up to 7 payload bytes per frame
8. **src/drivers/canbus/can_rx_message_handler.c** -- Implemented
   CanRxMessageHandler_stream_frame(): parses stream data from CAN frames,
   extracts DID from byte 0, passes to OpenLCB layer

### Unit Tests (Phase 2 -- DONE)

9. **src/openlcb/protocol_stream_handler_Test.cxx** -- Comprehensive unit tests
   (2,731 lines) covering all handlers, state transitions, flow control, buffer
   management
10. **test/CMakeLists.txt** -- Stream handler test target added and building

### Config-Mem Stream Layer (Phase 3 -- DONE, beyond original plan)

11. **src/openlcb/protocol_config_mem_stream_handler.h** -- Types and API for
    Memory Configuration streaming reads/writes
12. **src/openlcb/protocol_config_mem_stream_handler.c** -- Bridges config-mem
    and stream layers for large address space transfers (CDI, firmware)
13. **src/openlcb/protocol_config_mem_stream_handler_Test.cxx** -- Unit tests
    for config-mem streaming

---

## Feature Flag Gating

- All new code gated under #ifdef OPENLCB_COMPILE_STREAM
- Stream state table and handler module only compiled when enabled
- CAN TX/RX frame handlers: fully implemented, wired via can_config.c
- Wiring in openlcb_config.c under stream guard
- When flag is off: zero stream handler code compiled, stream buffers collapse
  to 1 byte per slot, all 5 statemachine function pointers stay NULL ->
  Interaction Rejected automatically

---

## Timeout Handling

**CAN layer (frame reassembly):** Handled by
OpenLcbBufferList_check_timeouts().  Stream control messages use standard
addressed multi-frame sequencing -- covered.  Stream Data Send uses Frame
Type 7 (single CAN frame per message) -- no reassembly needed.

**OpenLCB protocol layer (stream flow):** A stream can stall when:
- Initiate Request sent, no reply (stuck INITIATED)
- Stream OPEN, no Proceed arrives (source stalled)
- Source disappears mid-transfer (no Complete)

TODO: add last_activity_tick to stream_state_t and periodic scan to abort
stale streams.

---

## Verification

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

All phases verified: all tests pass with -Wall -Werror, stream handler compiles
to nothing when flag is off.  Config-mem stream handler tests also passing.

Stream_state_t includes two fields beyond the original plan: remote_alias
(needed for sending messages) and void *context (optional application context
for Layer 2 dispatch).
