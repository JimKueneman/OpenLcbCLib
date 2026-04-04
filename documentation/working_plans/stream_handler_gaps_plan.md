# Stream Handler Gaps -- Implementation Plan

## Status: COMPLETE

All 6 gaps closed. Source-side outbound API fully implemented and tested.
Additionally, send_early_proceed() was added for double-buffering support
(not in original gap inventory but needed per StreamTransportS Section 7.3).
The content_uid parameter was added to initiate_outbound() and
_load_initiate_request() to support both announced and unannounced streams.

## Goal

Close all gaps in protocol_stream_handler.c/h so that both the Source and
Destination roles are fully functional at the transport layer (Layer 1).
This is prerequisite work for the config-mem stream Layer 2 module.


## Spec References

- StreamTransportS.pdf (Feb 1, 2026 Preliminary) -- Sections 5, 6, 7
- StreamTransportTN.pdf (Feb 1, 2026 Preliminary)


## Gap Inventory

### Source Role (we send data)

| # | Gap | Description |
|---|-----|-------------|
| S1 | Send Stream Initiate Request | No function to build and send MTI_STREAM_INIT_REQUEST (0x0CC8). Payload: proposed_buffer_size (2), flags (2), SID (1), suggested DID (1), optional content_uid (6). |
| S2 | Allocate stream as source | _allocate_stream() exists but no code path populates a slot with is_source=true, assigns a SID, and sets state to INITIATED. |
| S3 | Send Stream Data Send | No function to build MTI_STREAM_SEND (0x1F88). Payload: DID (1 byte prefix) + data bytes. Must respect bytes_remaining flow control. |
| S4 | Send Stream Data Complete | No function to build MTI_STREAM_COMPLETE (0x08A8). Payload: SID (1), DID (1). Source sends this when transfer is done. |

### Destination Role (we receive data)

| # | Gap | Description |
|---|-----|-------------|
| D1 | Send Stream Data Complete | Same message as S4 but sent by destination. Destination may close the stream (e.g., received all expected data, or aborting). |
| D2 | Send TDE for stream abort | No function to build MTI_TERMINATE_DUE_TO_ERROR (0x00A8) with rejected_mti = MTI_STREAM_SEND. Destination sends this to abort a stream in error. |

Note: S4 and D1 are the same message builder -- one function covers both.


## Feature Flag

All new code gated under `#ifdef OPENLCB_COMPILE_STREAM` (existing guard).


## Implementation

### Step 1: SID Assignment Helper

Add a static `_next_source_stream_id` counter (mirrors `_next_dest_stream_id`)
and a `_assign_source_stream_id()` helper.

```c
static uint8_t _next_source_stream_id = 0;

static uint8_t _assign_source_stream_id(void) {

    uint8_t id = _next_source_stream_id;

    _next_source_stream_id++;

    if (_next_source_stream_id >= STREAM_ID_RESERVED) {

        _next_source_stream_id = 0;

    }

    return id;

}
```

Reset in ProtocolStreamHandler_initialize() alongside the DID counter.

Files: protocol_stream_handler.c


### Step 2: _load_initiate_request() (closes S1)

New static helper in protocol_stream_handler.c. Builds outgoing
MTI_STREAM_INIT_REQUEST (0x0CC8).

Payload per spec Section 5.3:
- Bytes 0-1: proposed max buffer size (big-endian)
- Bytes 2-3: flags (0x0000, no type included)
- Byte 4: Source Stream ID
- Byte 5: suggested Destination Stream ID (0xFF if no preference)
- Bytes 6-11: optional Stream Content UID (omit for now)

```c
static void _load_initiate_request(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t proposed_buffer_size,
            uint8_t source_stream_id,
            uint8_t suggested_dest_stream_id,
            uint16_t dest_alias,
            node_id_t dest_id) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            dest_alias,
            dest_id,
            MTI_STREAM_INIT_REQUEST);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, proposed_buffer_size, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x0000, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, source_stream_id, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, suggested_dest_stream_id, 5);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 6;
    statemachine_info->outgoing_msg_info.valid = true;

}
```

NOTE: This helper takes dest_alias and dest_id as explicit parameters rather
than reading them from incoming_msg_info, because when we are the source we
are INITIATING -- there is no incoming message to reply to.

Files: protocol_stream_handler.c


### Step 3: ProtocolStreamHandler_initiate_outbound() (closes S1 + S2)

New PUBLIC function. This is the entry point for Layer 2 modules (like
config-mem-stream) to open a stream where we are the source.

```c
stream_state_t *ProtocolStreamHandler_initiate_outbound(
            openlcb_statemachine_info_t *statemachine_info,
            uint16_t dest_alias,
            node_id_t dest_id,
            uint16_t proposed_buffer_size,
            uint8_t suggested_dest_stream_id) {

    stream_state_t *stream = _allocate_stream();

    if (!stream) { return NULL; }

    stream->state = STREAM_STATE_INITIATED;
    stream->source_stream_id = _assign_source_stream_id();
    stream->dest_stream_id = 0;  // unknown until reply
    stream->remote_node_id = dest_id;
    stream->is_source = true;
    stream->max_buffer_size = proposed_buffer_size;
    stream->bytes_transferred = 0;
    stream->bytes_remaining = 0;  // no window until accept

    _load_initiate_request(
            statemachine_info,
            proposed_buffer_size,
            stream->source_stream_id,
            suggested_dest_stream_id,
            dest_alias,
            dest_id);

    return stream;

}
```

Returns the stream_state_t pointer so the caller can track it. Returns NULL
if the stream table is full (caller should handle the error).

The existing ProtocolStreamHandler_initiate_reply() already handles the
incoming reply -- it finds the stream by SID, sets dest_stream_id, negotiates
buffer, transitions to OPEN, and calls on_initiate_reply. No changes needed
there.

Files: protocol_stream_handler.c, protocol_stream_handler.h (declaration)


### Step 4: _load_data_send() (closes S3)

New static helper. Builds MTI_STREAM_SEND (0x1F88).

Per spec Section 5.5:
- Byte 0: Destination Stream ID
- Bytes 1+: data payload

This helper loads the message frame. The caller is responsible for putting
the actual data bytes into the payload starting at offset 1.

```c
static void _load_data_send(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            const uint8_t *data,
            uint16_t data_len) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            stream->remote_node_id,
            MTI_STREAM_SEND);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 0);

    for (uint16_t i = 0; i < data_len; i++) {

        OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, data[i], 1 + i);

    }

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 1 + data_len;
    statemachine_info->outgoing_msg_info.valid = true;

}
```

OPEN QUESTION: The destination alias. When we are the source and sending data,
the destination's alias may not be in incoming_msg_info (there may be no
incoming message in context). We may need to store the remote alias in
stream_state_t. See Step 7.


### Step 5: ProtocolStreamHandler_send_data() (public wrapper, closes S3)

New PUBLIC function for Layer 2 modules to send data on an open stream.

```c
bool ProtocolStreamHandler_send_data(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            const uint8_t *data,
            uint16_t data_len) {

    if (!stream) { return false; }

    if (stream->state != STREAM_STATE_OPEN) { return false; }

    if (!stream->is_source) { return false; }

    if (data_len > stream->bytes_remaining) { return false; }

    _load_data_send(statemachine_info, stream, data, data_len);

    stream->bytes_transferred += data_len;
    stream->bytes_remaining -= data_len;

    return true;

}
```

Returns false if the stream is not open, we are not the source, or data_len
exceeds the current window. Caller should wait for on_data_proceed before
sending more when bytes_remaining hits 0.

Files: protocol_stream_handler.c, protocol_stream_handler.h (declaration)


### Step 6: _load_data_complete() + public wrapper (closes S4 + D1)

New static helper and public function. Builds MTI_STREAM_COMPLETE (0x08A8).

Per spec Section 5.7:
- Byte 0: Source Stream ID
- Byte 1: Destination Stream ID

Either side can send this.

```c
static void _load_data_complete(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_STREAM_COMPLETE);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->source_stream_id, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, stream->dest_stream_id, 1);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 2;
    statemachine_info->outgoing_msg_info.valid = true;

}
```

Public wrapper:

```c
void ProtocolStreamHandler_send_complete(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream) {

    if (!stream) { return; }

    _load_data_complete(statemachine_info, stream);

    _free_stream(stream);

}
```

Files: protocol_stream_handler.c, protocol_stream_handler.h (declaration)


### Step 7: Add remote_alias to stream_state_t

The _load_data_send() and _load_data_complete() helpers need the remote
node's CAN alias to address outgoing messages. When we are the destination
receiving data, the alias comes from incoming_msg_info. But when we are the
source sending data asynchronously (e.g., pumping data on a timer or in
on_data_proceed), there may be no incoming message in context.

Add to stream_state_t:

```c
    /** @brief CAN alias of the remote end (needed for outgoing messages). */
uint16_t remote_alias;
```

Populate it in:
- ProtocolStreamHandler_initiate_request() from incoming->source_alias
- ProtocolStreamHandler_initiate_outbound() from the dest_alias parameter

Files: protocol_stream_handler.h (struct), protocol_stream_handler.c
(two assignment lines)


### Step 8: ProtocolStreamHandler_send_terminate() (closes D2)

New PUBLIC function. Sends MTI_TERMINATE_DUE_TO_ERROR (0x00A8) to abort a
stream.

Per Message Network Standard, TDE payload:
- Bytes 0-1: error code
- Bytes 2-3: rejected MTI

For a destination aborting a stream, rejected_mti = MTI_STREAM_SEND.
For a source aborting, rejected_mti = MTI_STREAM_PROCEED.

```c
void ProtocolStreamHandler_send_terminate(
            openlcb_statemachine_info_t *statemachine_info,
            stream_state_t *stream,
            uint16_t error_code) {

    if (!stream) { return; }

    uint16_t rejected_mti = stream->is_source ? MTI_STREAM_PROCEED : MTI_STREAM_SEND;

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            stream->remote_alias,
            stream->remote_node_id,
            MTI_TERMINATE_DUE_TO_ERROR);

    OpenLcbUtilities_clear_openlcb_message_payload(statemachine_info->outgoing_msg_info.msg_ptr);

    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, error_code, 0);
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, rejected_mti, 2);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 4;
    statemachine_info->outgoing_msg_info.valid = true;

    _free_stream(stream);

}
```

Files: protocol_stream_handler.c, protocol_stream_handler.h (declaration)


## Implementation Order

| Order | Step | Gap(s) Closed | Risk |
|-------|------|---------------|------|
| 1 | Step 7: remote_alias in stream_state_t | prerequisite | Low -- additive struct change, zero-init safe |
| 2 | Step 1: SID assignment helper | prerequisite | Low -- mirrors existing DID helper |
| 3 | Step 2 + 3: _load_initiate_request + initiate_outbound | S1, S2 | Medium -- new public entry point |
| 4 | Step 4 + 5: _load_data_send + send_data | S3 | Medium -- data path, flow control |
| 5 | Step 6: _load_data_complete + send_complete | S4, D1 | Low -- simple 2-byte message |
| 6 | Step 8: send_terminate | D2 | Low -- simple 4-byte message |
| 7 | Tests | all | -- |

Steps 1-2 are prerequisites with no dependencies on each other -- can be done
together.

Steps 3-6 each depend on Step 7 (remote_alias) being in place.

Step 8 depends on Step 7 only.


## Test Plan

### New gTests in protocol_stream_handler_Test.cxx

**Section 7: Outbound stream initiation (Steps 1-3)**

1. `initiate_outbound_allocates_as_source` -- call initiate_outbound, verify
   stream_state_t has is_source=true, state=INITIATED, SID assigned,
   bytes_remaining=0

2. `initiate_outbound_sends_initiate_request` -- verify outgoing message has
   MTI_STREAM_INIT_REQUEST, correct payload (buffer size, SID, suggested DID)

3. `initiate_outbound_table_full_returns_null` -- fill table first, verify
   NULL return and no outgoing message

4. `initiate_outbound_then_reply_accept` -- call initiate_outbound, then
   feed in an accept Stream Initiate Reply; verify state transitions to
   OPEN, dest_stream_id populated, bytes_remaining = negotiated buffer

5. `initiate_outbound_then_reply_reject` -- call initiate_outbound, feed
   reject reply; verify slot freed

**Section 8: Send data (Steps 4-5)**

6. `send_data_basic` -- open stream as source (via outbound + accept reply),
   call send_data with small payload; verify outgoing MTI_STREAM_SEND with
   DID prefix + data, bytes_remaining decremented

7. `send_data_respects_window` -- send data up to bytes_remaining; verify
   next send_data returns false when window exhausted

8. `send_data_wrong_role_rejected` -- open stream as destination, call
   send_data; verify returns false

9. `send_data_not_open_rejected` -- call send_data on INITIATED stream;
   verify returns false

**Section 9: Send complete (Step 6)**

10. `send_complete_source` -- open stream as source, call send_complete;
    verify outgoing MTI_STREAM_COMPLETE with SID+DID, slot freed

11. `send_complete_destination` -- open stream as destination, call
    send_complete; verify same message, slot freed

**Section 10: Send terminate (Step 8)**

12. `send_terminate_as_destination` -- open as destination, call
    send_terminate; verify outgoing MTI_TERMINATE_DUE_TO_ERROR with
    rejected_mti = MTI_STREAM_SEND, slot freed

13. `send_terminate_as_source` -- open as source, call send_terminate;
    verify rejected_mti = MTI_STREAM_PROCEED, slot freed

**Section 11: Full outbound lifecycle**

14. `outbound_full_lifecycle` -- initiate_outbound, accept reply, send_data
    x N with proceed in between, send_complete; verify bytes_transferred
    matches total sent, slot freed

**Section 12: remote_alias (Step 7)**

15. `remote_alias_stored_on_inbound` -- receive Stream Initiate Request,
    verify stream->remote_alias matches incoming source_alias

16. `remote_alias_stored_on_outbound` -- call initiate_outbound with
    dest_alias, verify stream->remote_alias matches


## Files Modified

| File | Changes |
|------|---------|
| protocol_stream_handler.h | Add remote_alias to stream_state_t; declare 4 new public functions |
| protocol_stream_handler.c | Add _next_source_stream_id, _assign_source_stream_id, _load_initiate_request, _load_data_send, _load_data_complete; add 4 public functions; store remote_alias in 2 existing functions |
| protocol_stream_handler_Test.cxx | ~16 new tests in sections 7-12 |

No changes to openlcb_config.c, openlcb_main_statemachine.c/h, or the
datagram handler. This is purely Layer 1 transport completion.


## After This Plan

With all gaps closed, protocol_stream_handler provides a complete transport
API:

| Operation | Function |
|-----------|----------|
| Open stream as source | ProtocolStreamHandler_initiate_outbound() |
| Open stream as destination | ProtocolStreamHandler_initiate_request() (existing, auto) |
| Send data (source) | ProtocolStreamHandler_send_data() |
| Close stream (either side) | ProtocolStreamHandler_send_complete() |
| Abort stream (either side) | ProtocolStreamHandler_send_terminate() |
| Receive data (destination) | on_data_received callback (existing) |
| Flow control (destination) | auto Stream Data Proceed (existing) |
| Flow control (source) | on_data_proceed callback (existing) |

The config-mem-stream Layer 2 module (config_mem_stream_plan.md) can then
be built entirely on top of this API without touching Layer 1 internals.
