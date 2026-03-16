<!--
  ============================================================
  STATUS: REJECTED
  No `protocol_stream_handler.h/c` files exist in the source tree. Stream MTI constants are defined in `openlcb_defines.h`, but the full handler module was never written.
  ============================================================
-->

# Implementation Plan: Stream Transport Protocol Handler

## Context

The OpenLCB Stream Transport protocol enables reliable unidirectional bulk data transfer
with flow control between two nodes. The primary use case is Memory Configuration Protocol
stream reads/writes (e.g. reading an entire CDI via stream rather than 64-byte datagrams).

The library already has **all dispatch infrastructure** in place:
- 5 MTI cases in `openlcb_main_statemachine.c` (with OIR for unhandled requests)
- 5 function pointers in the main statemachine interface (all currently NULL)
- CAN RX statemachine routes frame type 7 to a stream handler (empty body)
- CAN TX statemachine routes stream MTIs to a stream handler (returns true, no-op)
- STREAM buffer type (512 bytes), buffer pool, and payload types all defined
- All stream MTI constants defined in `openlcb_defines.h`

What's missing is the **actual handler module** that implements the stream protocol logic.

**Spec reference:** StreamTransportS.pdf and StreamTransportTN.pdf (drafts, Apr 30 2021).

---

## Protocol Summary (from Spec)

### Messages (5 MTIs)

| MTI | Name | Direction | Payload |
|-----|------|-----------|---------|
| 0x0CC8 | Stream Initiate Request | Source→Dest | DID suggestion, SID, max buffer size, flags, optional content type UID |
| 0x0868 | Stream Initiate Reply | Dest→Source | DID, SID, negotiated buffer size, error code |
| 0x1F88 | Stream Data Send | Source→Dest | DID + payload bytes |
| 0x0888 | Stream Data Proceed | Dest→Source | DID, SID (flow control — send another window) |
| 0x08A8 | Stream Data Complete | Source→Dest | DID, SID, type/flags, optional total byte count |

### Stream IDs
- **SID** (Source Stream ID): 0–254, assigned by source (0xFF reserved)
- **DID** (Destination Stream ID): 0–254, assigned by destination (0xFF reserved)
- Stream uniquely identified by (Source Node, Dest Node, SID) or (Source Node, Dest Node, DID)

### States
- **Closed/Non-existent** — no active stream between these nodes with these IDs
- **Initiated** — Init Request sent, awaiting reply
- **Open** — accepted, data flowing with flow control

### Flow Control
- Source proposes max buffer size in Init Request
- Destination may reduce (never increase) in Init Reply
- Source sends up to negotiated buffer size bytes, then pauses
- Destination sends Proceed when ready → extends window by buffer size
- Source sends Complete when done

### CAN Adaptation (Spec Section 7)
- **Stream Data Send** uses CAN **Frame Type 7** (0x07000000): header = `0x1Fdd,dsss`,
  data byte 0 = Destination Stream ID, bytes 1–7 = payload (up to 7 bytes/frame)
- **All other stream messages** (Init Request/Reply, Proceed, Complete) use
  **normal addressed message framing** (first/middle/last frame bits per Message Network Std)

---

## Step 1: Add Stream Connection Types to `openlcb_types.h`

**File:** `src/openlcb/openlcb_types.h`

Add configurable depth and stream connection tracking structure:

```c
#ifndef USER_DEFINED_STREAM_CONNECTION_DEPTH
#define USER_DEFINED_STREAM_CONNECTION_DEPTH 1
#endif

typedef enum {
    STREAM_STATE_CLOSED,
    STREAM_STATE_INITIATED,
    STREAM_STATE_OPEN
} stream_state_enum;

typedef struct {
    stream_state_enum state;
    uint8_t source_stream_id;       /**< SID assigned by source (0xFF = unused) */
    uint8_t dest_stream_id;         /**< DID assigned by destination (0xFF = unused) */
    uint16_t source_alias;          /**< CAN alias of source node */
    uint16_t dest_alias;            /**< CAN alias of destination node */
    node_id_t source_id;            /**< Node ID of source */
    node_id_t dest_id;              /**< Node ID of destination */
    uint16_t negotiated_buffer_size; /**< Agreed max bytes per window */
    uint32_t total_bytes_transferred; /**< Running byte count */
    int32_t send_window_remaining;  /**< Bytes source can still send before pause */
    uint8_t is_source : 1;          /**< 1 = we are source, 0 = we are destination */
} stream_connection_t;

typedef stream_connection_t stream_connection_table_t[USER_DEFINED_STREAM_CONNECTION_DEPTH];
```

---

## Step 2: New Module `protocol_stream_handler.h`

**File:** `src/openlcb/protocol_stream_handler.h`

Follows `protocol_datagram_handler.h` pattern — dependency-injection interface struct
with application callbacks.

### Interface Struct

```c
typedef struct {

    /* Resource locking (REQUIRED) */
    void (*lock_shared_resources)(void);
    void (*unlock_shared_resources)(void);

    /* Buffer allocation (REQUIRED) */
    openlcb_msg_t *(*allocate_buffer)(payload_type_enum type);

    /* Application callbacks (all OPTIONAL — NULL = reject/ignore) */

    /** Called when we (destination) receive Stream Initiate Request.
     *  Return true to accept. Set *dest_stream_id_out and *negotiated_buf_size_out.
     *  Return false to reject (handler sends reject reply automatically). */
    bool (*on_stream_initiate_request)(
            openlcb_node_t *node,
            node_id_t source_node_id,
            uint8_t source_stream_id,
            uint16_t proposed_buffer_size,
            uint8_t *dest_stream_id_out,
            uint16_t *negotiated_buf_size_out);

    /** Called when we (destination) receive stream payload data. */
    void (*on_stream_data_received)(
            openlcb_node_t *node,
            uint8_t dest_stream_id,
            uint8_t *data,
            uint16_t length);

    /** Called when we (source) get accepted — stream is open. */
    void (*on_stream_accepted)(
            openlcb_node_t *node,
            uint8_t source_stream_id,
            uint8_t dest_stream_id,
            uint16_t negotiated_buffer_size);

    /** Called when we (source) get rejected. */
    void (*on_stream_rejected)(
            openlcb_node_t *node,
            uint8_t source_stream_id,
            uint16_t error_code);

    /** Called when we (source) may send another window. */
    void (*on_stream_proceed)(
            openlcb_node_t *node,
            uint8_t source_stream_id);

    /** Called when stream is complete (both roles). */
    void (*on_stream_complete)(
            openlcb_node_t *node,
            uint8_t stream_id,
            uint32_t total_bytes);

} interface_protocol_stream_handler_t;
```

### Public API

| Function | Purpose |
|----------|---------|
| `ProtocolStreamHandler_initialize(interface)` | Register DI interface, zero connection table |
| `ProtocolStreamHandler_handle_stream_initiate_request(sm_info)` | **Dest side:** accept/reject incoming stream |
| `ProtocolStreamHandler_handle_stream_initiate_reply(sm_info)` | **Source side:** process accept/reject |
| `ProtocolStreamHandler_handle_stream_data_send(sm_info)` | **Dest side:** receive data, manage flow control |
| `ProtocolStreamHandler_handle_stream_data_proceed(sm_info)` | **Source side:** window extended |
| `ProtocolStreamHandler_handle_stream_data_complete(sm_info)` | **Dest side:** stream finished |

---

## Step 3: Implement `protocol_stream_handler.c`

**File:** `src/openlcb/protocol_stream_handler.c`

### Internal State

```c
static interface_protocol_stream_handler_t *_interface;
static stream_connection_table_t _connections;
```

### Handler Logic

**`handle_stream_initiate_request`** (we are destination):
1. Extract fields from incoming message payload:
   - Byte 0: suggested DID (or 0xFF)
   - Byte 1: SID
   - Bytes 2–3: proposed max buffer size
   - Byte 4: flags (deprecated, check as 0)
   - Byte 5: additional flags (reserved)
   - Bytes 6–11: optional Stream Content Type UID
2. Call `_interface->on_stream_initiate_request()` callback
3. If callback returns true (accept):
   - Allocate a connection slot
   - Store SID, DID, aliases, node IDs, buffer size, state = OPEN
   - Build Stream Initiate Reply with: DID, SID, negotiated buffer size, error = 0x8000 (accept)
   - Send reply
4. If callback returns false or is NULL:
   - Build Stream Initiate Reply with: SID, buffer size = 0, error = 0x1040 (streams not supported) or 0x2020 (buffer unavailable)
   - Send reply

**`handle_stream_initiate_reply`** (we are source):
1. Extract: DID, SID, buffer size, error code
2. Find connection by SID + source/dest aliases
3. If error code has Accept bit (0x8000):
   - Store DID, update buffer size, set state = OPEN
   - Initialize send_window_remaining = negotiated_buffer_size
   - Call `_interface->on_stream_accepted()`
4. Else (rejected):
   - Free connection slot
   - Call `_interface->on_stream_rejected()`

**`handle_stream_data_send`** (we are destination):
1. Extract DID from payload byte 0
2. Find connection by DID + source/dest aliases
3. Pass payload bytes (after DID) to `_interface->on_stream_data_received()`
4. Track total_bytes_transferred
5. When total received reaches negotiated_buffer_size:
   - Build and send Stream Data Proceed (DID, SID)
   - Reset byte counter for next window

**`handle_stream_data_proceed`** (we are source):
1. Extract DID, SID from payload
2. Find connection by SID + aliases
3. Add negotiated_buffer_size to send_window_remaining
4. Call `_interface->on_stream_proceed()`

**`handle_stream_data_complete`** (we are destination):
1. Extract DID, SID, type byte, optional total byte count (bytes 3–5)
2. Find connection, verify
3. Call `_interface->on_stream_complete()`
4. Free connection slot, set state = CLOSED

### Source-side Send API (for higher-level protocols to call)

```c
/** Initiate a stream (we are source). Returns SID or 0xFF on failure. */
uint8_t ProtocolStreamHandler_open_stream(
        openlcb_node_t *node,
        node_id_t dest_node_id, uint16_t dest_alias,
        uint16_t proposed_buffer_size);

/** Send stream data (we are source). Returns bytes accepted. */
uint16_t ProtocolStreamHandler_send_data(
        openlcb_node_t *node,
        uint8_t source_stream_id,
        uint8_t *data, uint16_t length);

/** Close the stream (we are source). */
void ProtocolStreamHandler_close_stream(
        openlcb_node_t *node,
        uint8_t source_stream_id);
```

---

## Step 4: Fix CAN TX Dispatch + Implement Stream Frame Handlers

### Fix Dispatch in `can_tx_statemachine.c`

**File:** `src/drivers/canbus/can_tx_statemachine.c`

Current dispatch is **wrong**: Init/Reply/Proceed/Complete go to `handle_stream_frame`
(placeholder). Per spec section 7.2, these use normal addressed framing. Only Stream Data
Send uses frame type 7.

Change the switch in `_transmit_openlcb_message()` (~line 86):

```c
    /* BEFORE (wrong): */
    case MTI_STREAM_COMPLETE:
    case MTI_STREAM_INIT_REPLY:
    case MTI_STREAM_INIT_REQUEST:
    case MTI_STREAM_PROCEED:
        return _interface->handle_stream_frame(...);

    /* AFTER (correct): */
    case MTI_STREAM_SEND:
        return _interface->handle_stream_frame(...);
```

Stream Init Request/Reply/Proceed/Complete now fall through to the `default` case
(`handle_addressed_msg_frame`), which is correct — they use normal addressed multi-frame
CAN encoding.

### Implement `CanTxMessageHandler_stream_frame()`

**File:** `src/drivers/canbus/can_tx_message_handler.c`

Replace the placeholder with a real implementation. For MTI_STREAM_SEND:

1. Build CAN identifier: `RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_STREAM | (dest_alias << 12) | source_alias`
2. Set CAN data byte 0 = Destination Stream ID (from `openlcb_msg->payload[0]`)
3. Copy up to 7 payload bytes from the OpenLCB message into CAN data bytes 1–7
4. Set `can_msg->payload_count` = 1 + bytes copied
5. Transmit frame
6. Advance `*openlcb_start_index`
7. Return true when all payload sent

The DID is in `openlcb_msg->payload[0]`. The actual stream data starts at
`openlcb_msg->payload[1]`. Each CAN frame carries byte 0 = DID + up to 7 data bytes.

Add a static identifier builder:
```c
static const uint32_t _OPENLCB_MESSAGE_STREAM_FRAME =
    RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_STREAM;
```

---

## Step 5: Implement CAN RX Stream Frame Handler

**File:** `src/drivers/canbus/can_rx_message_handler.c`

Replace the empty `CanRxMessageHandler_stream_frame()` body.

For each CAN frame type 7 received:
1. Extract source alias from CAN identifier
2. Extract dest alias from CAN identifier
3. Extract DID from `can_msg->payload[0]`
4. Extract payload bytes from `can_msg->payload[1..7]`
5. Allocate an OpenLCB BASIC buffer (small — just one CAN frame's worth of data)
6. Set `mti = MTI_STREAM_SEND`, source/dest aliases and IDs, copy payload
7. Push to OpenLCB FIFO

The protocol layer handler accumulates these individual messages and manages flow control.
Using BASIC buffers (16 bytes) is sufficient since each CAN frame carries at most 8 bytes.

**Note:** The existing call signature passes `data_type = STREAM` (512-byte buffer), but we
only need BASIC for individual CAN frames. The handler can ignore the passed data_type and
allocate BASIC instead, or we could use the STREAM buffer for accumulation. For simplicity,
use BASIC per frame and let the protocol layer accumulate.

---

## Step 6: Wire in `openlcb_config.c`

**File:** `src/openlcb/openlcb_config.c`

### Add Feature Flag

Gate everything under `OPENLCB_COMPILE_STREAM`.

### Add to `_build_main_statemachine()`:

```c
#ifdef OPENLCB_COMPILE_STREAM
    _main_sm.stream_initiate_request = &ProtocolStreamHandler_handle_stream_initiate_request;
    _main_sm.stream_initiate_reply   = &ProtocolStreamHandler_handle_stream_initiate_reply;
    _main_sm.stream_send_data        = &ProtocolStreamHandler_handle_stream_data_send;
    _main_sm.stream_data_proceed     = &ProtocolStreamHandler_handle_stream_data_proceed;
    _main_sm.stream_data_complete    = &ProtocolStreamHandler_handle_stream_data_complete;
#endif
```

### Add `_build_stream()` and call from `OpenlcbConfig_initialize()`:

Build the `interface_protocol_stream_handler_t` struct with lock/unlock, buffer allocator,
and application callbacks from `openlcb_config_t`. Call `ProtocolStreamHandler_initialize()`.

### Add to `openlcb_config.h`:

Add stream configuration callbacks to `openlcb_config_t` struct, gated on
`OPENLCB_COMPILE_STREAM`.

Add to feature flag validation section:
```c
#if defined(OPENLCB_COMPILE_STREAM) && !defined(OPENLCB_COMPILE_DATAGRAMS)
#error "OPENLCB_COMPILE_STREAM requires OPENLCB_COMPILE_DATAGRAMS"
#endif
```

(Stream transport is typically triggered by datagram-level commands like Memory Config
stream read/write.)

---

## Step 7: Add Stream Defines to `openlcb_defines.h`

**File:** `src/openlcb/openlcb_defines.h`

Add error code constants for Stream Initiate Reply (near existing error codes):

```c
/* Stream Initiate Reply error codes */
#define STREAM_ERROR_ACCEPT                 0x8000
#define STREAM_ERROR_PERMANENT              0x1000
#define STREAM_ERROR_STREAMS_NOT_SUPPORTED  0x1040
#define STREAM_ERROR_SOURCE_NOT_PERMITTED   0x1020
#define STREAM_ERROR_UNIMPLEMENTED          0x1010
#define STREAM_ERROR_TEMPORARY              0x2000
#define STREAM_ERROR_UNEXPECTED             0x2040
#define STREAM_ERROR_BUFFER_UNAVAILABLE     0x2020
#define STREAM_ERROR_LOGGED                 0x0001

/* Stream Data Complete type codes (byte 2 bits 0-3) */
#define STREAM_COMPLETE_TYPE_RESERVED       0x00
#define STREAM_COMPLETE_TYPE_PING           0x01
#define STREAM_COMPLETE_TYPE_PONG           0x02
#define STREAM_COMPLETE_TYPE_COMPLETED      0x03
#define STREAM_COMPLETE_TYPE_ABORT          0x04

/* Stream ID reserved value */
#define STREAM_ID_RESERVED                  0xFF
```

---

## Step 8: Tests

**File:** `src/openlcb/protocol_stream_handler_Test.cxx`

Following `protocol_datagram_handler_Test.cxx` pattern:

### Destination-side tests:
- **Init request accepted:** Receive Init Request → callback accepts → verify Init Reply sent with accept code, DID, SID, buffer size
- **Init request rejected (no callback):** NULL callback → verify reject reply (0x1040)
- **Init request rejected (callback declines):** callback returns false → verify reject reply
- **Data receive:** Open stream → receive Stream Data Send → verify callback gets payload
- **Flow control:** Receive `negotiated_buffer_size` bytes → verify Proceed sent automatically
- **Complete:** Receive Stream Data Complete → verify callback, connection freed

### Source-side tests:
- **Open stream:** Call `open_stream()` → verify Init Request sent with SID, proposed buffer size
- **Accepted reply:** Send Init Request → receive accept reply → verify callback, state = OPEN
- **Rejected reply:** Send Init Request → receive reject reply → verify callback, slot freed
- **Proceed received:** Open stream → receive Proceed → verify window extended, callback called
- **Send data:** Open + accepted → send data → verify Stream Data Send message built correctly
- **Close stream:** Send Complete → verify message sent, connection freed

### Edge cases:
- **Connection table full:** All slots in use → new Init Request → reject with buffer unavailable
- **Unknown stream ID:** Data for non-existent DID → ignored/rejected
- **Duplicate Init Request:** Same SID from same source → reject

### CAN layer tests:

**File:** `src/drivers/canbus/can_tx_message_handler_Test.cxx` (add to existing)
- **Stream Data Send TX:** Verify frame type 7 identifier, DID in byte 0, payload fragmentation
- **Multi-frame stream TX:** Large payload → multiple CAN frames, each with DID prefix

**File:** `src/drivers/canbus/can_rx_message_handler_Test.cxx` (add to existing)
- **Stream frame RX:** Frame type 7 → OpenLCB message with MTI_STREAM_SEND, correct DID, payload

---

## Step 9: CMakeLists.txt

**File:** `test/CMakeLists.txt` (or relevant location)

- Add `protocol_stream_handler.c` to SOURCES
- Add `protocol_stream_handler_Test.cxx` to TESTS

---

## File Change Summary

| File | Change |
|------|--------|
| `src/openlcb/openlcb_types.h` | Add `stream_state_enum`, `stream_connection_t`, `stream_connection_table_t`, `USER_DEFINED_STREAM_CONNECTION_DEPTH` |
| `src/openlcb/openlcb_defines.h` | Add stream error codes, complete type codes, `STREAM_ID_RESERVED` |
| `src/openlcb/protocol_stream_handler.h` | **New** — interface struct + public API |
| `src/openlcb/protocol_stream_handler.c` | **New** — full implementation |
| `src/openlcb/protocol_stream_handler_Test.cxx` | **New** — test suite |
| `src/openlcb/openlcb_config.h` | Add stream callbacks to `openlcb_config_t`, feature flag validation |
| `src/openlcb/openlcb_config.c` | Add `_build_stream()`, wire 5 handlers in `_build_main_statemachine()` |
| `src/drivers/canbus/can_tx_statemachine.c` | Fix dispatch: move Init/Reply/Proceed/Complete to default, add MTI_STREAM_SEND to stream case |
| `src/drivers/canbus/can_tx_message_handler.c` | Implement `CanTxMessageHandler_stream_frame()` with frame type 7 encoding |
| `src/drivers/canbus/can_rx_message_handler.c` | Implement `CanRxMessageHandler_stream_frame()` — CAN type 7 → OpenLCB FIFO |
| `test/CMakeLists.txt` | Add new source + test |

---

## Spec Inconsistency Note

The draft standard has a contradiction in section 8.3 (says "Source Stream ID is carried"
in CAN data frames) vs. section 7.1 normative table (says "Destination Stream ID" in byte
0). The TN section 2.4.1 confirms Destination Stream ID is the one included in Stream Data
Send to maximize efficiency. We follow the **normative standard table (Section 7.1):**
**byte 0 = Destination Stream ID.**

---

## Build & Verify

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```
