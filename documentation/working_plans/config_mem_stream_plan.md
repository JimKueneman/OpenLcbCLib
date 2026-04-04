# Config Memory Stream Read/Write Implementation Plan

## Status

- Phase 1 (stream transport layer): COMPLETE
- Stream handler gaps (source-side API): COMPLETE
- Phase 2 (this plan -- config-mem stream Layer 2 module): NOT STARTED


## Spec References

- MemoryConfigurationS.pdf (July 22, 2024 Adopted) -- Sections 4.6, 4.7, 4.11, 4.12
- StreamTransportS.pdf (Feb 1, 2026 Preliminary) -- Sections 5, 6, 7
- MemoryConfigurationTN.pdf -- Sections 2.4.6, 2.4.7, 2.4.11, 2.4.12 (all say
  "stream support is considered experimental")


## Current State of Infrastructure

### Stream Transport Layer (protocol_stream_handler.c/h) -- COMPLETE

Layer 1 transport fully implemented with both source and destination roles.

Destination-side (incoming message handlers, wired via main statemachine):

| Function | MTI | Role |
|----------|-----|------|
| ProtocolStreamHandler_initiate_request() | 0x0CC8 | Destination receives initiation |
| ProtocolStreamHandler_initiate_reply() | 0x0868 | Source receives accept/reject |
| ProtocolStreamHandler_data_send() | 0x1F88 | Destination receives data |
| ProtocolStreamHandler_data_proceed() | 0x0888 | Source receives flow control ack |
| ProtocolStreamHandler_data_complete() | 0x08A8 | Either side receives completion |
| ProtocolStreamHandler_handle_terminate_due_to_error() | 0x00A8 | Either side receives abort |

Source-side (outbound API for Layer 2 modules):

| Function | Purpose |
|----------|---------|
| ProtocolStreamHandler_initiate_outbound(statemachine_info, dest_alias, dest_id, proposed_buffer_size, suggested_dest_stream_id, content_uid) | Allocates stream slot as source, sends Stream Initiate Request. Returns stream_state_t* or NULL. |
| ProtocolStreamHandler_send_data(statemachine_info, stream, data, data_len) | Sends data on open stream. Returns false if window exhausted or precondition fail. |
| ProtocolStreamHandler_send_complete(statemachine_info, stream) | Sends Stream Data Complete, frees slot. |
| ProtocolStreamHandler_send_terminate(statemachine_info, stream, error_code) | Sends TDE with role-appropriate rejected_mti, frees slot. |
| ProtocolStreamHandler_send_early_proceed(statemachine_info, stream) | Destination sends advance Proceed for double-buffering. |

Stream state entry (stream_state_t):

| Field | Type | Description |
|-------|------|-------------|
| state | stream_state_enum | CLOSED, INITIATED, or OPEN |
| source_stream_id | uint8_t | SID assigned by source node |
| dest_stream_id | uint8_t | DID assigned by destination node |
| remote_node_id | node_id_t | Node ID of the remote end |
| remote_alias | uint16_t | CAN alias of the remote end |
| max_buffer_size | uint16_t | Negotiated bytes per window |
| bytes_transferred | uint32_t | Running total of payload bytes |
| bytes_remaining | uint16_t | Bytes left in current send window |
| is_source | bool | True if this node is the source |
| content_uid | uint8_t[6] | Optional Stream Content UID |
| context | void* | Application context for Layer 2 dispatch |

DI interface (interface_protocol_stream_handler_t, 5 optional callbacks):

| Callback | When | Role |
|----------|------|------|
| on_initiate_request(statemachine_info, stream) -> bool | Incoming Stream Initiate Request | Destination accepts/rejects |
| on_initiate_reply(statemachine_info, stream) | Incoming Stream Initiate Reply | Source learns accept/reject |
| on_data_received(statemachine_info, stream) | Incoming Stream Data Send | Destination receives data |
| on_data_proceed(statemachine_info, stream) | Incoming Stream Data Proceed | Source window extended |
| on_complete(statemachine_info, stream) | Incoming Stream Data Complete | Either side, stream closed |

### Datagram Handler Routing -- COMPLETE

protocol_datagram_handler.c/h already dispatches Read Stream and Write Stream
command bytes to per-space callback function pointers. All currently NULL
(auto-rejected via _handle_subcommand).

Read Stream command dispatch (byte 1 values):

| Byte 1 | Constant | Space | Dispatch target |
|--------|----------|-------|-----------------|
| 0x60 | CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6 | payload[6] | _handle_read_stream_address_space_at_offset_6() |
| 0x61 | CONFIG_MEM_READ_STREAM_SPACE_FD | 0xFD | memory_read_stream_space_configuration_memory |
| 0x62 | CONFIG_MEM_READ_STREAM_SPACE_FE | 0xFE | memory_read_stream_space_all |
| 0x63 | CONFIG_MEM_READ_STREAM_SPACE_FF | 0xFF | memory_read_stream_space_config_description_info |

Read Stream Reply dispatch (byte 1 values):

| Byte 1 | OK/Fail | Space | Dispatch target pattern |
|--------|---------|-------|------------------------|
| 0x70-0x73 | OK | per encoding | memory_read_stream_space_*_reply_ok |
| 0x78-0x7B | Fail | per encoding | memory_read_stream_space_*_reply_fail |

Write Stream command dispatch (byte 1 values):

| Byte 1 | Constant | Space | Dispatch target |
|--------|----------|-------|-----------------|
| 0x20 | CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6 | payload[6] | _handle_write_stream_address_space_at_offset_6() |
| 0x21 | CONFIG_MEM_WRITE_STREAM_SPACE_FD | 0xFD | memory_write_stream_space_configuration_memory |
| 0x22 | CONFIG_MEM_WRITE_STREAM_SPACE_FE | 0xFE | memory_write_stream_space_all |
| 0x23 | CONFIG_MEM_WRITE_STREAM_SPACE_FF | 0xFF | memory_write_stream_space_config_description_info |

Write Stream Reply dispatch (byte 1 values):

| Byte 1 | OK/Fail | Space | Dispatch target pattern |
|--------|---------|-------|------------------------|
| 0x30-0x33 | OK | per encoding | memory_write_stream_space_*_reply_ok |
| 0x38-0x3B | Fail | per encoding | memory_write_stream_space_*_reply_fail |

Additional per-space slots in the datagram handler interface for spaces
0xFC, 0xFB, 0xFA, 0xF9, and 0xEF (firmware) are routed via the
_at_offset_6 dispatchers.

### Configuration Options -- COMPLETE

Get Options reply already advertises CONFIG_OPTIONS_WRITE_LENGTH_STREAM_READ_WRITE
(0x01) when user_configuration_options.stream_read_write_supported == true.

### Wiring in openlcb_config.c -- COMPLETE (for Layer 1)

- _build_stream_handler() wires user callbacks from openlcb_config_t into
  interface_protocol_stream_handler_t
- ProtocolStreamHandler_initialize(&_stream_handler) called during init
- Main statemachine function pointers wired to all 6 inbound handlers
- Datagram handler stream callback slots remain NULL (Layer 2 not yet wired)


## Protocol Flow (from Spec)

### Read Stream Command Datagram Format (MemoryConfigurationS Section 4.6)

```
Byte 0:    0x20 (config mem datagram type)
Byte 1:    0x60 space in byte 6
           0x61 space 0xFD
           0x62 space 0xFE
           0x63 space 0xFF
Bytes 2-5: Starting address (big-endian)
Byte 6:    Address space (only when byte 1 = 0x60)
Next byte: 0xFF (reserved, send as 0xFF, ignore on receipt)
Next byte: Destination Stream ID (suggested by requester)
Next 4:    Read Count (0 = read to end of space)
```

Total datagram payload: 11 bytes (space in byte 1) or 12 bytes (space in byte 6).

### Read Stream Reply Datagram Format (MemoryConfigurationS Section 4.7)

OK reply:
```
Byte 0:    0x20
Byte 1:    0x70 space in byte 6 | 0x71 FD | 0x72 FE | 0x73 FF
Bytes 2-5: Starting address
Byte 6:    Address space (only when byte 1 = 0x70)
Next byte: Source Stream ID
Next byte: Destination Stream ID
Next 4:    Read Count
```

Fail reply:
```
Byte 0:    0x20
Byte 1:    0x78 space in byte 6 | 0x79 FD | 0x7A FE | 0x7B FF
Bytes 2-5: Starting address
Byte 6:    Address space (only when byte 1 = 0x78)
Next 2:    Error code
Next 0-63: Optional zero-terminated error string
```

### Read Stream Sequence (server side -- this node is being read)

1. Remote node sends Read Stream Command datagram
2. This node (server/source) must:
   a. Send Datagram Received OK (Reply Pending set)
   b. Parse address, space, dest_stream_id, read_count from datagram
   c. Validate address space exists and is readable
   d. Allocate stream slot via ProtocolStreamHandler_initiate_outbound()
      with the suggested Destination Stream ID from the datagram
   e. Wait for Stream Initiate Reply (accept/reject + negotiated buffer)
   f. On accept: send Read Stream Reply datagram (OK) with SID and DID,
      then begin streaming data via ProtocolStreamHandler_send_data()
   g. On reject or error: send Read Stream Reply datagram (Fail)
   h. When all data sent: call ProtocolStreamHandler_send_complete()

   Key: per spec, the stream must be established BEFORE the Read Stream Reply
   datagram is sent, so errors during stream setup can be reported in the reply.

### Write Stream Command Datagram Format (MemoryConfigurationS Section 4.11)

```
Byte 0:    0x20 (config mem datagram type)
Byte 1:    0x20 space in byte 6
           0x21 space 0xFD
           0x22 space 0xFE
           0x23 space 0xFF
Bytes 2-5: Starting address (big-endian)
Byte 6:    Address space (only when byte 1 = 0x20)
Last byte: Source Stream ID (from the remote node)
```

Total datagram payload: 7 bytes (space in byte 1) or 8 bytes (space in byte 6).

### Write Stream Reply Datagram Format (MemoryConfigurationS Section 4.12)

OK reply:
```
Byte 0:    0x20
Byte 1:    0x30 space in byte 6 | 0x31 FD | 0x32 FE | 0x33 FF
Bytes 2-5: Starting address
Byte 6:    Address space (only when byte 1 = 0x30)
Next byte: Source Stream ID
Next byte: Destination Stream ID
```

Fail reply:
```
Byte 0:    0x20
Byte 1:    0x38 space in byte 6 | 0x39 FD | 0x3A FE | 0x3B FF
Bytes 2-5: Starting address
Byte 6:    Address space (only when byte 1 = 0x38)
Next 2:    Error code
Next 0-63: Optional zero-terminated error string
```

### Write Stream Sequence (server side -- this node is being written)

1. Remote node sends Write Stream Command datagram
2. This node (server/destination) must:
   a. Send Datagram Received OK (Reply Pending set)
   b. Parse address, space, source_stream_id from datagram
   c. Validate address space exists and is writable
   d. If invalid: send Datagram Rejected with error code, return
   e. Record source_stream_id, wait for Stream Initiate Request from remote
   f. on_initiate_request callback fires:
      - Match by remote_node_id and source_stream_id
      - Accept (send Stream Initiate Reply with negotiated buffer)
      - Set stream->context to point to the write context
   g. on_data_received callback fires repeatedly:
      - Write received data to config memory at current_offset
      - Track bytes; when window exhausted, auto Stream Data Proceed fires
   h. on_complete callback fires:
      - Send Write Stream Reply (OK) datagram
   i. If error during write: send ProtocolStreamHandler_send_terminate(),
      then send Write Stream Reply (Fail) datagram


## Architecture Decision: Layer 2 Module

This is NOT a modification to protocol_stream_handler.c (Layer 1). This is a
new Layer 2 module that wires into both the datagram handler's stream callbacks
AND the stream handler's application callbacks.

New file: protocol_config_mem_stream_handler.c / .h

This module:
- Receives Read Stream / Write Stream datagram commands from the datagram
  handler (via function pointer wiring in the datagram handler interface)
- Manages the lifecycle: parses the datagram, allocates stream state, drives
  the stream transport handshake, feeds/consumes data
- Uses the existing stream handler public API for the actual transport
- Calls existing config_memory_read() / config_memory_write() for data access
- Sends Read/Write Stream Reply datagrams when the operation completes

Feature flags: #ifdef OPENLCB_COMPILE_STREAM AND #ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION


## Phased Implementation

### Phase 2A: Read Stream (Server Side)

This node responds to a remote node's request to stream-read an address space.

#### Context Type

A struct to track the in-progress stream-read operation. Allocated statically
(one per concurrent stream, tied to stream_state_t via context pointer).

```
typedef struct {

    uint8_t  address_space;
    uint32_t start_address;
    uint32_t read_count;        // 0 = read to end
    uint32_t current_offset;    // bytes already sent
    uint8_t  dest_stream_id;    // suggested DID from the requester
    uint8_t  command_byte;      // original byte 1 for building reply
    bool     active;            // slot in use

} config_mem_stream_read_context_t;
```

#### DI Interface

```
typedef struct {

    // Required -- read config memory (same signature as datagram read handler)
    uint16_t (*config_memory_read)(openlcb_node_t *node, uint32_t address,
                                   uint16_t count,
                                   configuration_memory_buffer_t *buffer);

    // Required -- write config memory (for Phase 2B)
    uint16_t (*config_memory_write)(openlcb_node_t *node, uint32_t address,
                                    uint16_t count,
                                    configuration_memory_buffer_t *buffer);

    // Required -- send a datagram (for Read/Write Stream Reply)
    void (*load_datagram_reply)(openlcb_statemachine_info_t *statemachine_info);

    // Required -- send Datagram Received OK
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info);

    // Required -- send Datagram Rejected
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info,
                                                     uint16_t error_code);

    // Required -- stream transport API (wired to ProtocolStreamHandler functions
    // via openlcb_config.c, not direct #include)
    stream_state_t *(*initiate_outbound)(openlcb_statemachine_info_t *statemachine_info,
                                          uint16_t dest_alias, node_id_t dest_id,
                                          uint16_t proposed_buffer_size,
                                          uint8_t suggested_dest_stream_id,
                                          const uint8_t *content_uid);

    bool (*send_data)(openlcb_statemachine_info_t *statemachine_info,
                      stream_state_t *stream, const uint8_t *data,
                      uint16_t data_len);

    void (*send_complete)(openlcb_statemachine_info_t *statemachine_info,
                          stream_state_t *stream);

    void (*send_terminate)(openlcb_statemachine_info_t *statemachine_info,
                           stream_state_t *stream, uint16_t error_code);

} interface_protocol_config_mem_stream_handler_t;
```

#### Callback Routing

The stream handler has ONE callback interface for ALL streams. Config-mem
streams need different behavior than generic application streams. The context
pointer in stream_state_t solves this:

- When this module allocates a stream (via initiate_outbound or by accepting
  an incoming initiate_request), it sets stream->context to point to its
  own static context struct (config_mem_stream_read_context_t or
  config_mem_stream_write_context_t).
- The stream handler callbacks registered in openlcb_config.c act as routers:
  they check stream->context. If non-NULL and pointing to a config-mem context,
  dispatch to this module. Otherwise, dispatch to the application callback.

This keeps the dispatch clean without requiring multiple stream handler
instances or a purpose enum.

#### Data Pump

The data pump runs in the on_initiate_reply and on_data_proceed callbacks.
When the stream transitions to OPEN (or when the window is extended by a
Proceed), the module reads config memory in chunks up to max_buffer_size and
sends via send_data(). When bytes_remaining hits 0, the pump stops and waits
for the next on_data_proceed.

When read_count bytes have been sent (or end of address space reached), the
module calls send_complete().


### Phase 2B: Write Stream (Server Side)

This node receives a stream of data from a remote node writing to an address
space. The roles are reversed from Read Stream:

- This node is the DESTINATION (data receiver)
- The remote node is the SOURCE (it sends Stream Initiate Request after we
  ACK the Write Stream Command datagram)

#### Context Type

```
typedef struct {

    uint8_t  address_space;
    uint32_t start_address;
    uint32_t current_offset;    // bytes already written
    uint8_t  source_stream_id;  // SID from the remote (announced in datagram)
    uint8_t  command_byte;      // original byte 1 for building reply
    bool     active;            // slot in use

} config_mem_stream_write_context_t;
```

#### Write Data Path

In on_data_received: extract payload from statemachine_info, write to config
memory at start_address + current_offset, advance current_offset. The stream
handler automatically sends Stream Data Proceed when the window is consumed.

In on_complete: send Write Stream Reply (OK) datagram.

If config_memory_write fails mid-stream: call send_terminate() with appropriate
error code, then send Write Stream Reply (Fail) datagram.


### Phase 2C: Client Side (Optional, Lower Priority)

If this node wants to READ or WRITE another node's config memory via stream.
This is the mirror of 2A/2B -- this node sends the Read/Write Stream Command
datagram and handles the stream from the other end. Useful for configuration
tools running on this node.

Lower priority because most embedded nodes are servers (being configured), not
clients (doing the configuring). A PC-based config tool would typically use
the python-openlcb library instead.


## Files to Create/Modify

### New Files

- src/openlcb/protocol_config_mem_stream_handler.h
- src/openlcb/protocol_config_mem_stream_handler.c
- src/openlcb/protocol_config_mem_stream_handler_Test.cxx

### Modified Files

- src/openlcb/openlcb_config.c -- wire new module's functions into
  datagram handler's stream callback slots; update stream handler callbacks
  to route through config-mem-stream module when stream->context is set
- src/openlcb/openlcb_config.h -- add config_mem_stream callbacks to
  openlcb_config_t if needed; add verbose flag
- test/CMakeLists.txt -- add new test file
- Copy new files to all application directories (via update_applications.sh)


## Open Questions

1. **Async data pump**: Recommendation is to pump in the on_initiate_reply /
   on_data_proceed callbacks, sending as many frames as the window allows in
   one call. The stream handler already tracks bytes_remaining per window.
   However, the outgoing message buffer is a single slot -- each send_data()
   call puts one message in the outgoing buffer. If the callback is only
   invoked once per Proceed, the pump may need to be driven externally (e.g.,
   from the 100ms timer or from the main loop checking for active streams
   with bytes_remaining > 0). This needs investigation during implementation.

2. **Stream table slot contention**: With USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS
   defaulting to 1, a config-mem stream transfer blocks all other stream
   usage. Acceptable for typical embedded nodes. Nodes that need concurrent
   streams should increase the count.

3. **Spec says "experimental"**: The Memory Configuration TN explicitly says
   stream support is "considered experimental." We implement to the current
   Standard letter (which IS normative and adopted) and accept that the TN
   guidance may change.

4. **content_uid usage**: The spec does not mention content_uid for config-mem
   streams. Config-mem streams use the "opening with announcement" pattern
   (Section 7.1) where the datagram announces the stream before the Stream
   Initiate Request. Per spec, content_uid is optional for announced streams.
   Pass NULL to initiate_outbound (omit content_uid from the request).

5. **Write Stream -- when to send Reply?**: Spec says after stream closes
   successfully. If write fails mid-stream (e.g., flash error), abort the
   stream via send_terminate() and then send Write Stream Reply (Fail).

6. **Firmware space (0xEF) for Write Stream**: The datagram handler has a
   memory_write_stream_space_firmware_upgrade slot. This is relevant for
   firmware upgrade via stream. Include it in the routing but it can remain
   NULL initially (firmware upgrade is a separate feature).


## Implementation Order

| Step | Description | Dependencies |
|------|-------------|--------------|
| 1 | Create protocol_config_mem_stream_handler.h -- types, interface struct, public function declarations | None |
| 2 | Create protocol_config_mem_stream_handler.c -- Read Stream server (Phase 2A) | Step 1 |
| 3 | Wire in openlcb_config.c -- connect datagram handler stream slots + stream handler callback routing | Steps 1-2 |
| 4 | Write tests for Read Stream server flow | Steps 1-3 |
| 5 | Add Write Stream server flow to the .c (Phase 2B) | Steps 1-3 |
| 6 | Write tests for Write Stream server flow | Step 5 |
| 7 | Copy new files to application directories (update_applications.sh) | Steps 1-6 |
| 8 | Add OlcbChecker tests if applicable | Steps 1-6 |


## Compliance Mapping: Tests to Spec Requirements

Spec abbreviations:
- **MemCfgS** = MemoryConfigurationS.pdf (July 22, 2024 Adopted)
- **StreamS** = StreamTransportS.pdf (Feb 1, 2026 Preliminary)
- **CanFTS** = CanFrameTransferS.pdf

### CAN TX Stream Tests (can_tx_message_handler_Test.cxx)

| Test | Spec | Section | Requirement |
|------|------|---------|-------------|
| stream_single_frame_small_payload | StreamS | 8.1 | DID in byte 0, payload in bytes 1+, up to 7 data bytes per frame |
| stream_single_frame_full | StreamS | 8.1 | Full 8-byte CAN frame: DID + 7 payload bytes |
| stream_multi_frame_did_in_every_frame | StreamS | 8.1 | DID must appear in byte 0 of every CAN frame (not just the first) |
| stream_multi_frame_short_last_frame | StreamS | 8.1 | Final frame may be shorter than 8 bytes |
| stream_transmit_failure_index_unchanged | StreamS | 8.1 | On CAN TX failure, payload index must not advance (retry safety) |
| stream_did_only_frame | StreamS | 8.1 | DID-only frame (zero payload bytes) -- keepalive per StreamS 7.3 |
| stream_three_frames | StreamS | 8.1 | CAN header: Frame Type 7 (0x1F), dest alias bits [23:12], source alias bits [11:0] |

### Stream Transport Layer Tests (protocol_stream_handler_Test.cxx)

| Test | Spec | Section | Requirement |
|------|------|---------|-------------|
| initialize | StreamS | -- | Module initializes without crash |
| initiate_request_accept | StreamS | 7.1 | Destination accepts Stream Initiate Request, allocates DID, replies with accept (0x8000) |
| initiate_request_buffer_negotiation | StreamS | 7.1 | Destination may reduce Max Buffer Size to <= proposed |
| initiate_request_reject_no_callback | StreamS | 7.1 | No callback -> reject with Permanent Error (0x1040 Unimplemented) |
| initiate_request_reject_table_full | StreamS | 7.1 | No free stream slot -> reject with Temporary Error (0x2020 Buffer Unavailable) |
| initiate_request_with_content_uid | StreamS | 5.1, 7.2 | Optional 6-byte Stream Content UID parsed from bytes 6-11 |
| initiate_request_veto_rejects | StreamS | 7.1 | Application callback returns false -> reject stream |
| initiate_request_accept_via_callback | StreamS | 7.1 | Application callback returns true -> accept stream |
| initiate_reply_accept | StreamS | 7.1 | Source receives accept reply, stream transitions to OPEN |
| initiate_reply_alternate_accept | StreamS | 5.2 | 0x0000 with non-zero buffer is also accept |
| initiate_reply_reject | StreamS | 7.1 | Source receives reject reply, stream freed |
| initiate_reply_0x8000_zero_buffer_rejected | StreamS | 5.2 | Accept flag set but buffer=0 -> treated as reject |
| initiate_reply_0x0000_zero_buffer_rejected | StreamS | 5.2 | 0x0000 with buffer=0 -> treated as reject |
| initiate_reply_0x0000_nonzero_buffer_accepted | StreamS | 5.2 | 0x0000 with non-zero buffer -> accepted |
| initiate_reply_unknown_stream | StreamS | 7.1 | Reply for non-existent SID -> ignored |
| initiate_reply_null_callback | StreamS | 7.1 | No on_initiate_reply callback -> stream still transitions |
| data_send_basic | StreamS | 5.3, 7.3 | Destination receives data, bytes_remaining decremented |
| data_send_unknown_stream | StreamS | 7.3 | Data for unknown DID -> ignored |
| data_send_non_open_stream | StreamS | 6 | Data on non-OPEN stream -> ignored |
| data_send_bytes_remaining_underflow | StreamS | 7.3 | Data exceeding window -> clamped, no underflow |
| data_send_zero_payload_flush | StreamS | 7.3 | Zero-payload Stream Data Send (keepalive) per Section 7.3 |
| data_send_null_callback | StreamS | 7.3 | No on_data_received callback -> data consumed silently |
| data_proceed_extends_window | StreamS | 7.3 | Each Proceed extends window by Max Buffer Size (additive) |
| data_proceed_unknown_stream | StreamS | 7.3 | Proceed for unknown stream -> ignored |
| data_proceed_non_open_stream | StreamS | 6 | Proceed on non-OPEN stream -> ignored |
| data_proceed_null_callback | StreamS | 7.3 | No on_data_proceed callback -> window still extended |
| data_complete_frees_stream | StreamS | 7.4 | Stream Data Complete transitions to CLOSED, slot freed |
| data_complete_unknown_stream | StreamS | 7.4 | Complete for unknown stream -> ignored |
| data_complete_null_callback | StreamS | 7.4 | No on_complete callback -> stream still freed |
| dest_stream_id_wraps_at_reserved | StreamS | 5.1 | DID 0xFF is reserved -- allocator wraps past it |
| tde_stream_send_mti_closes_stream | StreamS | 7.4 | Terminate Due To Error with MTI=Stream Data Send closes stream (destination sent) |
| tde_stream_proceed_mti_closes_stream | StreamS | 7.4 | Terminate Due To Error with MTI=Stream Data Proceed closes stream (source sent) |
| tde_unrelated_mti_ignored | StreamS | 7.4 | TDE with unrelated MTI -> no stream affected |
| tde_no_stream_ignored | StreamS | 7.4 | TDE with no matching stream -> ignored |
| tde_payload_too_short_ignored | StreamS | 7.4 | Malformed TDE (too few bytes) -> ignored |
| tde_wrong_node_ignored | StreamS | 7.4 | TDE from non-matching node -> ignored |
| full_lifecycle | StreamS | 7.1, 7.3, 7.4 | End-to-end: initiate -> accept -> data -> proceed -> complete -> freed |
| remote_alias_stored_on_inbound | StreamS | 7.1 | Destination stores source alias for future replies |
| remote_alias_stored_on_outbound | StreamS | 7.1 | Source stores dest alias from initiate_outbound |
| context_pointer | StreamS | -- | Application context pointer stored/retrieved on stream_state_t |
| initiate_outbound_allocates_as_source | StreamS | 7.1, 7.2 | Source allocates stream slot, assigns SID, is_source=true |
| initiate_outbound_sends_initiate_request | StreamS | 5.1 | Outbound sends Stream Initiate Request with correct payload layout |
| initiate_outbound_table_full_returns_null | StreamS | 7.1 | No free slot -> returns NULL |
| initiate_outbound_then_reply_accept | StreamS | 7.1 | Outbound followed by accept reply -> OPEN |
| initiate_outbound_then_reply_reject | StreamS | 7.1 | Outbound followed by reject reply -> CLOSED |
| initiate_outbound_with_content_uid | StreamS | 5.1, 7.2 | Content UID included in Stream Initiate Request bytes 6-11 |
| initiate_outbound_without_content_uid | StreamS | 7.1 | Announced streams (Section 7.1) may omit content UID |
| send_data_basic | StreamS | 5.3, 7.3 | Source sends data, bytes_remaining decremented |
| send_data_respects_window | StreamS | 7.3 | Source blocked when bytes_remaining = 0 |
| send_data_wrong_role_rejected | StreamS | 7.3 | Destination cannot call send_data (is_source must be true) |
| send_data_not_open_rejected | StreamS | 6 | send_data on non-OPEN stream -> rejected |
| send_complete_source | StreamS | 7.4 | Source sends Stream Data Complete |
| send_complete_destination | StreamS | 7.4 | Destination sends Stream Data Complete (error close) |
| send_complete_includes_total_bytes | StreamS | 5.5 | Optional total bytes field in Stream Data Complete |
| send_terminate_as_destination | StreamS | 7.4 | Destination terminates with TDE (rejected_mti = Stream Data Send) |
| send_terminate_as_source | StreamS | 7.4 | Source terminates with TDE (rejected_mti = Stream Data Proceed) |
| early_proceed_as_destination | StreamS | 7.3 | Destination may send one Proceed in advance |
| early_proceed_rejected_as_source | StreamS | 7.3 | Source cannot send Proceed (role enforcement) |
| outbound_full_lifecycle | StreamS | 7.1-7.4 | End-to-end outbound: initiate -> accept -> data -> proceed -> complete |

### Config-Mem Stream Handler Tests (protocol_config_mem_stream_handler_Test.cxx)

| Test | Spec | Section | Requirement |
|------|------|---------|-------------|
| initialize_does_not_crash | -- | -- | Module startup sanity |
| initialize_with_null_user_callbacks | -- | -- | Null user callbacks accepted (config-mem-only usage) |
| phase1_sends_datagram_ack | MemCfgS | 4.6 | Read Stream Command -> Datagram Received OK with Reply Pending |
| phase2_initiates_outbound_stream | MemCfgS | 4.6, StreamS 7.1 | After ACK, initiates outbound stream with suggested DID from datagram |
| phase2_stream_table_full_resets | MemCfgS | 4.7, StreamS 7.1 | Stream table full -> send Read Stream Reply (Fail) |
| reject_when_busy | MemCfgS | 4.3 | Second Read Stream while first active -> Datagram Rejected |
| reject_when_no_cdi | MemCfgS | 4.6, 4.7 | Read space 0xFF with NULL CDI callback -> Datagram Rejected |
| parses_request_parameters | MemCfgS | 4.6 | Correct parsing of address, space, DID, read_count from datagram |
| initiate_request_forwards_to_user | StreamS | 7.1 | Non-config-mem stream initiate request forwarded to user callback |
| initiate_request_no_user_callback_returns_false | StreamS | 7.1 | No user callback for non-config stream -> reject via return false |
| initiate_reply_accepted_transitions_to_send_reply | MemCfgS | 4.7, StreamS 7.1 | Stream accept -> load Read Stream Reply (OK) datagram |
| initiate_reply_rejected_resets_to_idle | MemCfgS | 4.7, StreamS 7.1 | Stream reject -> send Read Stream Reply (Fail), reset to IDLE |
| initiate_reply_forwards_non_config_stream_to_user | StreamS | 7.1 | Non-config-mem stream reply forwarded to user callback |
| data_received_always_forwards_to_user | StreamS | 7.3 | Inbound data forwarded to user (this node is source for reads, not destination) |
| data_proceed_for_our_stream_does_not_forward | StreamS | 7.3 | Config-mem stream Proceed handled internally (resumes pump) |
| data_proceed_forwards_other_stream_to_user | StreamS | 7.3 | Non-config-mem stream Proceed forwarded to user callback |
| complete_for_our_stream_resets_to_idle | StreamS | 7.4 | Remote closes our config-mem stream -> reset to IDLE |
| complete_forwards_other_stream_to_user | StreamS | 7.4 | Non-config-mem stream Complete forwarded to user callback |
| pump_idle_does_nothing | -- | -- | Pump called when IDLE -> no action |
| pump_sends_reply_datagram_then_pumps_data | MemCfgS | 4.7, StreamS 7.3 | Reply datagram sent, then data pumped via send_data |
| pump_sends_complete_after_all_data | MemCfgS | 4.7, StreamS 7.4 | All bytes sent -> Stream Data Complete |
| pump_respects_zero_bytes_remaining | StreamS | 7.3 | Window exhausted -> pump stops, waits for Proceed |
| pump_with_specific_read_count | MemCfgS | 4.6 | Non-zero read_count limits total bytes sent |
| pump_with_start_address_offset | MemCfgS | 4.6 | Starting address offsets into address space data |
| pump_send_fails_retries_next_run | StreamS | 7.3 | send_data returns false -> pump retries next cycle |
| pump_address_past_end_sends_fail_reply | MemCfgS | 4.7 | Start address beyond space -> Read Stream Reply (Fail) |
| remote_close_during_pump_resets | StreamS | 7.4 | Remote sends Complete mid-transfer -> handler resets |
| config_memory_read_stream_pumps_data | MemCfgS | 4.6, 4.7, StreamS 7.1-7.4 | Full end-to-end: datagram -> ACK -> stream -> data -> complete |
| reject_space_with_null_read_callback | MemCfgS | 4.3 | Address space with no read callback -> Datagram Rejected |
| reply_datagram_uses_correct_cmd_byte_for_config_memory | MemCfgS | 4.7 | Reply byte 1 matches request encoding (0x71 for 0xFD, 0x72 for 0xFE, etc.) |
| reject_space_not_present | MemCfgS | 4.3, 4.7 | Space not present in node -> Datagram Rejected |
| out_of_bounds_cdi_sends_fail_reply | MemCfgS | 4.7 | CDI start address past end -> Read Stream Reply (Fail, 0x1081) |
| out_of_bounds_config_memory_sends_fail_reply | MemCfgS | 4.7 | Config memory start address past end -> Read Stream Reply (Fail) |
| out_of_bounds_module_accepts_new_request | MemCfgS | 4.6 | After fail reply, module returns to IDLE and accepts new requests |

### OlcbChecker Compliance Tests (External)

| Test Script | Spec | Section | Requirement |
|-------------|------|---------|-------------|
| check_cd20_read (--stream) | MemCfgS + StreamS | 4.6, 4.7, 7.1-7.4, 8.1 | Full CDI Read Stream: datagram -> stream initiate -> data transfer -> complete; validates CDI content |
| check_mc30_read | MemCfgS | 4.4, 4.5 | Non-stream config memory read (datagram-based) |
| check_mc60_out_of_range | MemCfgS | 4.3, 4.5 | Out-of-range address returns error reply |
| check_st* (future) | StreamS | 5-8 | Dedicated stream transport compliance tests (not yet in OlcbChecker) |
