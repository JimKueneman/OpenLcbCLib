# OpenLcbCLib — WebAssembly build

Emscripten build of OpenLcbCLib that `OpenLcbJSLib` consumes as a thin wrapper.
Transport is CAN with gridconnect text frames; the JS side relays those frames
over a WebSocket or equivalent socket.  Native binary TCP is planned for the
future.

## Build

```
cd wasm
./build.sh
```

Produces `wasm/dist/openlcb-core.{js,wasm}`.  Requires Emscripten on `PATH`
(`brew install emscripten`).

Current artifact size: ~104 KB `.wasm` + ~27 KB `.js` glue.

## Test

```
node wasm/test.mjs
```

Runs the integration suite: lifecycle, error codes, event send round-trip,
event registration, config-memory callback wiring, broadcast time.  CI
(GitHub Actions) runs the same script on every push that touches `src/` or
`wasm/`.

## Everything is keyed off node IDs

There are no opaque handles.  Every callback carries a 48-bit node ID as a JS
`BigInt`; every node-scoped export takes the same node ID as input.  Node
lifetimes extend for the life of the WASM instance — OpenLCB has no protocol-
level node removal, so there is no `wasm_destroy_node`.

## Compile-time feature flags

Set in [`openlcb_user_config.h`](openlcb_user_config.h):

- `OPENLCB_COMPILE_CAN`
- `OPENLCB_COMPILE_EVENTS`
- `OPENLCB_COMPILE_DATAGRAMS`
- `OPENLCB_COMPILE_MEMORY_CONFIGURATION`
- `OPENLCB_COMPILE_BROADCAST_TIME`
- `OPENLCB_COMPILE_TRAIN`
- `OPENLCB_COMPILE_TRAIN_SEARCH`
- `OPENLCB_COMPILE_STREAM`
- `OPENLCB_COMPILE_DCC_DETECTOR`

Firmware upgrade is deliberately excluded — a WASM node reloads from source
each page load, there is nothing to upgrade.

## Compile-time pool ceilings

Also set in [`openlcb_user_config.h`](openlcb_user_config.h).  Per-node
parameters pick within these ceilings; exceeding them requires rebuilding the
WASM.

| Pool | Default | Library max |
|------|---------|-------------|
| `NODE_BUFFER_DEPTH` | 255 | 255 (`uint8_t` node index) |
| `PRODUCER_COUNT` | 256 | 65535 |
| `CONSUMER_COUNT` | 256 | 65535 |
| `PRODUCER_RANGE_COUNT` | 256 | 65535 |
| `CONSUMER_RANGE_COUNT` | 256 | 65535 |
| `BASIC_BUFFER_DEPTH` | 255 | total across all four pools <= 65535 |
| `DATAGRAM_BUFFER_DEPTH` | 64 | same |
| `SNIP_BUFFER_DEPTH` | 64 | same |
| `STREAM_BUFFER_DEPTH` | 64 | same |
| `STREAM_BUFFER_LEN` | 1024 | 65535 |
| `MAX_CONCURRENT_ACTIVE_STREAMS` | 64 | — |
| `CAN_MSG_BUFFER_DEPTH` | 20 | 254 |
| `TRAIN_NODE_COUNT` | 255 | — |
| `MAX_LISTENERS_PER_TRAIN` | 16 | — |
| `MAX_TRAIN_FUNCTIONS` | 29 | — |

## Error codes (int32 returns)

| Code | Name | Meaning |
|-----:|------|---------|
|  0 | `WASM_OK` | success |
| -1 | `WASM_ERR_INVALID_ARG` | bad argument (e.g. unknown address-space id) |
| -2 | `WASM_ERR_CEILING_EXCEEDED` | pool full or allocation failed |
| -3 | `WASM_ERR_UNKNOWN_NODE` | no node with that ID exists |
| -4 | `WASM_ERR_TX_BUSY` | transport TX path could not accept the message |
| -5 | `WASM_ERR_NOT_INITIALIZED` | `wasm_initialize()` was not called first |

## ABI — exports (JS -> C)

Call via `Module.cwrap` or `Module.ccall`.

### Lifecycle

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_initialize` | `() -> void` | One-time transport + OpenLCB stack bring-up.  Idempotent. |
| `wasm_run` | `() -> void` | Run one pass of the main state machine.  Call in a loop. |
| `wasm_100ms_tick` | `() -> void` | Advance the global 100 ms counter.  Call every 100 ms. |
| `wasm_rx_gridconnect` | `(cstr: string) -> void` | Feed one or more concatenated `:X...;` gridconnect frames.  Completed frames are pushed into the CAN RX path. |

### Node builder

Build a `node_parameters_t` in a scratch buffer, then commit with
`wasm_create_node(node_id)`.  All setters overwrite the corresponding field
in the scratch; strings and byte arrays are copied on the C side so JS is
free to release its originals immediately.

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_node_builder_reset` | `() -> void` | Zero the scratch and free any staged CDI/FDI bytes. |
| `wasm_node_set_snip` | `(mfg_version: u8, name: string, model: string, hw_version: string, sw_version: string, user_version: u8) -> void` | Strings are clamped to their buffer sizes. |
| `wasm_node_set_protocol_support` | `(low: u32, high: u32) -> void` | 64-bit protocol support bitfield, split low/high. |
| `wasm_node_set_event_autocreate` | `(producer_count: u8, consumer_count: u8) -> void` | |
| `wasm_node_set_configuration_options` | `(flags: u32, high_space: u8, low_space: u8, description: string) -> void` | See flag table below. |
| `wasm_node_set_address_space` | `(space_id: u8, flags: u32, low_address: u32, highest_address: u32, description: string) -> int32` | Returns `-1` for unknown `space_id`.  Supported IDs: `0xFF`, `0xFE`, `0xFD`, `0xFC`, `0xFB`, `0xFA`, `0xF9`, `0xEF`. |
| `wasm_node_set_cdi` | `(ptr: u32, len: u32) -> int32` | `ptr` is a byte offset into `Module.HEAPU8`.  C copies the bytes. |
| `wasm_node_set_fdi` | `(ptr: u32, len: u32) -> int32` | Same conventions as `wasm_node_set_cdi`. |
| `wasm_create_node` | `(node_id: bigint) -> int32` | Duplicates the scratch and creates the node.  Resets the scratch on success. |

#### `wasm_node_set_configuration_options` flag bits

| Bit | Field |
|----:|-------|
| 0 | `write_under_mask_supported` |
| 1 | `unaligned_reads_supported` |
| 2 | `unaligned_writes_supported` |
| 3 | `read_from_manufacturer_space_0xfc_supported` |
| 4 | `read_from_user_space_0xfb_supported` |
| 5 | `write_to_user_space_0xfb_supported` |
| 6 | `stream_read_write_supported` |

#### `wasm_node_set_address_space` flag bits

| Bit | Field |
|----:|-------|
| 0 | `present` |
| 1 | `read_only` |
| 2 | `low_address_valid` |

### Events

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_send_event_pc_report` | `(node_id: bigint, event_id: bigint) -> int32` | Publishes a PC Event Report from `node_id`.  Returns `-3` if the node doesn't exist, `-4` if the TX path is busy. |
| `wasm_send_event_with_mti` | `(node_id: bigint, event_id: bigint, mti: u32) -> int32` | Sends an event message with the given MTI (producer/consumer identified set/clear, identified range, etc.). |
| `wasm_send_teach_event` | `(node_id: bigint, event_id: bigint) -> int32` | Sends a Learn Event. |
| `wasm_send_initialization_event` | `(node_id: bigint) -> int32` | Sends the node's initialization complete (re-announce). |
| `wasm_register_consumer_eventid` | `(node_id: bigint, event_id: bigint, status: u32) -> int32` | Returns the event list index on success, `-2` if the consumer list is full. `status` is the `event_status_enum` value (0 unknown / 1 valid / 2 invalid / 3 unknown). |
| `wasm_register_producer_eventid` | `(node_id: bigint, event_id: bigint, status: u32) -> int32` | Same pattern for producers. |
| `wasm_clear_consumer_eventids` | `(node_id: bigint) -> int32` | |
| `wasm_clear_producer_eventids` | `(node_id: bigint) -> int32` | |
| `wasm_register_consumer_range` | `(node_id: bigint, event_id_base: bigint, range_size: u32) -> int32` | `range_size` is an `event_range_count_enum` (power of 2 span code). |
| `wasm_register_producer_range` | `(node_id: bigint, event_id_base: bigint, range_size: u32) -> int32` | |
| `wasm_clear_consumer_ranges` | `(node_id: bigint) -> int32` | |
| `wasm_clear_producer_ranges` | `(node_id: bigint) -> int32` | |

### Broadcast Time

All exports below take a 64-bit `clock_id` (the well-known clock event prefix).

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_bt_is_consumer` | `(clock_id: bigint) -> int32` | Returns 1 if a consumer slot is allocated for this clock. |
| `wasm_bt_is_producer` | `(clock_id: bigint) -> int32` | |
| `wasm_bt_start` / `wasm_bt_stop` | `(clock_id: bigint) -> void` | Start / stop a producer slot locally. |
| `wasm_bt_send_report_time` | `(node_id, clock_id, hour, minute) -> int32` | |
| `wasm_bt_send_report_date` | `(node_id, clock_id, month, day) -> int32` | |
| `wasm_bt_send_report_year` | `(node_id, clock_id, year) -> int32` | |
| `wasm_bt_send_report_rate` | `(node_id, clock_id, rate) -> int32` | `rate` is signed fixed-point (int16). |
| `wasm_bt_send_start` / `wasm_bt_send_stop` / `wasm_bt_send_date_rollover` | `(node_id, clock_id) -> int32` | |
| `wasm_bt_send_query` / `wasm_bt_send_query_reply` | (same signatures) | |
| `wasm_bt_send_set_time` / `_set_date` | `(node_id, clock_id, a, b) -> int32` | |
| `wasm_bt_send_set_year` / `_set_rate` | `(node_id, clock_id, v) -> int32` | |
| `wasm_bt_send_command_start` / `_command_stop` | `(node_id, clock_id) -> int32` | |

### DCC detector helpers

Pure functions for encoding / decoding DCC detector event IDs (no node
lookup, no state).

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_dcc_encode_event_id` | `(detector_node_id: bigint, direction: u32, raw_address_14: u32) -> bigint` | |
| `wasm_dcc_make_short_address` | `(short_address: u32) -> u32` | Returns the 14-bit raw address with the 0x38 prefix. |
| `wasm_dcc_make_consist_address` | `(consist_address: u32) -> u32` | Returns the 14-bit raw address with the 0x39 prefix. |
| `wasm_dcc_extract_direction` | `(event_id: bigint) -> u32` | `dcc_detector_direction_enum`. |
| `wasm_dcc_extract_address_type` | `(event_id: bigint) -> u32` | `dcc_detector_address_type_enum` (short / consist / track-empty / long). |
| `wasm_dcc_extract_raw_address` | `(event_id: bigint) -> u32` | Full 14-bit field (includes prefix). |
| `wasm_dcc_extract_dcc_address` | `(event_id: bigint) -> u32` | 8-bit DCC address for short/consist; full 14-bit for long; 0 for track-empty. |
| `wasm_dcc_extract_detector_id` | `(event_id: bigint) -> bigint` | Upper 48 bits as a node ID. |
| `wasm_dcc_is_track_empty` | `(event_id: bigint) -> int32` | 1 when the raw address field is 0x3800. |

### Trains

Exports for the throttle side (controlling a train) plus per-node property getters/setters.

| Export | Signature | Notes |
|--------|-----------|-------|
| `wasm_train_send_set_speed` | `(throttle_node_id, train_alias, train_node_id, speed_f16) -> int32` | `speed_f16` is a 16-bit half-float speed. |
| `wasm_train_send_set_function` | `(throttle_node_id, train_alias, train_node_id, fn_address, fn_value) -> int32` | |
| `wasm_train_send_emergency_stop` | `(throttle_node_id, train_alias, train_node_id) -> int32` | |
| `wasm_train_send_query_speeds` | `(...) -> int32` | |
| `wasm_train_send_query_function` | `(..., fn_address) -> int32` | |
| `wasm_train_send_assign_controller` | `(...) -> int32` | |
| `wasm_train_send_release_controller` | `(...) -> int32` | |
| `wasm_train_send_noop` | `(...) -> int32` | |
| `wasm_train_set_dcc_address` / `wasm_train_get_dcc_address` / `wasm_train_is_long_address` | `(train_node_id, ...) -> int32` | Configure this node as a train. |
| `wasm_train_set_speed_steps` / `wasm_train_get_speed_steps` | `(train_node_id, ...) -> int32` | |

Also exported for JS interop: `_malloc`, `_free`.  Runtime methods: `ccall`,
`cwrap`, `HEAPU8`.

## ABI — callbacks (C -> JS)

Set these on the Emscripten `Module` before the factory resolves (or at
least before `wasm_initialize()` runs — the first three only fire after a
node exists).

| Hook | Signature | Wraps | Notes |
|------|-----------|-------|-------|
| `Module.onGridconnectTx` | `(frame: string) -> void` | `can_config_t.transmit_raw_can_frame` | One `:X...;` frame per call.  No trailing newline. |
| `Module.onLoginComplete` | `(node_id: bigint) -> void` | `openlcb_config_t.on_login_complete` | Fires once per node after alias allocation + Initialization Complete. |
| `Module.onPcEventReport` | `(node_id: bigint, event_id: bigint) -> void` | `on_pc_event_report` | Unfiltered — fires for every PC Event Report seen on the wire. |
| `Module.onConsumedEventPcer` | `(node_id: bigint, index: u16, event_id: bigint) -> void` | `on_consumed_event_pcer` | Filtered — fires only when the PCER matches a registered consumer on that node.  `index` is the consumer list index. |
| `Module.onConsumedEventIdentified` | `(node_id: bigint, index: u16, event_id: bigint, status: u8) -> void` | `on_consumed_event_identified` | Consumer Identified (valid / invalid / unknown) for a registered consumer. |
| `Module.onPcEventReportWithPayload` | `(node_id: bigint, event_id: bigint, count: u16, heap_ptr: u32) -> void` | `on_pc_event_report_with_payload` | Payload bytes live at `Module.HEAPU8.subarray(heap_ptr, heap_ptr + count)` for the duration of the callback. Copy if retained. |
| `Module.onEventLearn` / `onConsumerRangeIdentified` / `onProducerRangeIdentified` | `(node_id: bigint, event_id: bigint) -> void` | `on_event_learn` / `on_{consumer,producer}_range_identified` | |
| `Module.onConsumerIdentified{Unknown,Set,Clear,Reserved}` | `(node_id: bigint, event_id: bigint) -> void` | matching C hooks | |
| `Module.onProducerIdentified{Unknown,Set,Clear,Reserved}` | `(node_id: bigint, event_id: bigint) -> void` | matching C hooks | |
| `Module.onConfigMemRead` | `(node_id: bigint, address: u32, count: u32, heap_ptr: u32) -> number` | `config_mem_read` | Fill `HEAPU8.set(bytes, heap_ptr)`; return bytes-written (or `0` to signal "no backing store"). |
| `Module.onConfigMemWrite` | `(node_id: bigint, address: u32, count: u32, heap_ptr: u32) -> number` | `config_mem_write` | Read bytes from `HEAPU8.subarray(heap_ptr, heap_ptr + count)`; return bytes-consumed. |
| `Module.onOptionalInteractionRejected` | `(node_id, source_node_id, error_code: u16, rejected_mti: u16) -> void` | `on_optional_interaction_rejected` | |
| `Module.onTerminateDueToError` | `(node_id, source_node_id, error_code: u16, rejected_mti: u16) -> void` | `on_terminate_due_to_error` | |
| `Module.on100msTimer` | `() -> void` | `on_100ms_timer` | Fires after every `wasm_100ms_tick()` once OpenLCB has been initialized. |
| `Module.onBroadcastTimeChanged` | `(clock_id: bigint, hour: u8, minute: u8) -> void` | `on_broadcast_time_changed` | |
| `Module.onBroadcastTimeReceived` / `DateReceived` | `(node_id, clock_id, a, b) -> void` | | `a`,`b` = hour/minute or month/day. |
| `Module.onBroadcastYearReceived` / `RateReceived` | `(node_id, clock_id, v) -> void` | | |
| `Module.onBroadcastClockStarted` / `ClockStopped` / `DateRollover` | `(node_id, clock_id) -> void` | | |
| `Module.onTrainSpeedChanged` | `(node_id, speed_f16) -> void` | `on_train_speed_changed` | |
| `Module.onTrainFunctionChanged` | `(node_id, fn_address, fn_value) -> void` | | |
| `Module.onTrainEmergencyEntered` / `EmergencyExited` | `(node_id, type) -> void` | | `type` is `train_emergency_type_enum`. |
| `Module.onTrainControllerAssigned` | `(node_id, controller_node_id) -> void` | | |
| `Module.onTrainControllerReleased` / `HeartbeatTimeout` / `ListenerChanged` | `(node_id) -> void` | | |
| `Module.onTrainControllerAssignRequest` | `(node_id, current_controller, requesting_controller) -> bool` | | Return `true` to accept a takeover, `false` to reject. |
| `Module.onTrainControllerChangedRequest` | `(node_id, new_controller) -> bool` | | |
| `Module.onTrainQuerySpeedsReply` | `(node_id, set_speed, status, commanded, actual) -> void` | | |
| `Module.onTrainQueryFunctionReply` | `(node_id, fn_address, fn_value) -> void` | | |
| `Module.onTrainControllerAssignReply` / `ControllerQueryReply` / `ControllerChangedNotifyReply` / `ListenerAttachReply` / `ListenerDetachReply` / `ListenerQueryReply` / `ReserveReply` / `HeartbeatRequest` | (see bindings.c) | | |
| `Module.onTrainSearchMatched` | `(node_id, search_address, flags) -> void` | `on_train_search_matched` | |
| `Module.onStreamInitiateRequest` | `(stream_state_ptr: u32) -> bool` | `on_stream_initiate_request` | Return `true` to accept the stream. |
| `Module.onStreamInitiateReply` / `DataReceived` / `DataProceed` / `Complete` | `(stream_state_ptr: u32) -> void` | matching C hooks | Read `stream_state_t` fields via `HEAP*` if needed. |

### Re-entrancy

Callbacks run synchronously from within the C state machine.  Do **not**
re-enter the library (`wasm_rx_gridconnect`, `wasm_send_event_pc_report`,
etc.) from inside a callback.  Queue the work in JS and handle it after the
callback returns.

## Minimum JS usage

```js
import factory from "./openlcb-core.js";

const Module = await factory({
    onGridconnectTx:   (frame) => socket.send(frame + "\n"),
    onLoginComplete:   (nid)   => console.log("login:", nid.toString(16)),
    onPcEventReport:   (nid, eid) => console.log("PCER:", eid.toString(16)),
});

const init     = Module.cwrap("wasm_initialize",                   null,     []);
const reset    = Module.cwrap("wasm_node_builder_reset",           null,     []);
const setSnip  = Module.cwrap("wasm_node_set_snip",                null,     ["number","string","string","string","string","number"]);
const setPS    = Module.cwrap("wasm_node_set_protocol_support",    null,     ["number","number"]);
const setAuto  = Module.cwrap("wasm_node_set_event_autocreate",    null,     ["number","number"]);
const create   = Module.cwrap("wasm_create_node",                  "number", ["bigint"]);
const run      = Module.cwrap("wasm_run",                          null,     []);
const tick     = Module.cwrap("wasm_100ms_tick",                   null,     []);
const rx       = Module.cwrap("wasm_rx_gridconnect",               null,     ["string"]);
const sendPcer = Module.cwrap("wasm_send_event_pc_report",         "number", ["bigint","bigint"]);

init();
reset();
setSnip(4, "My Node", "Model X", "hw0.1", "sw0.1", 2);
setPS(0x0000DE00, 0);  // datagrams, mem config, events, ACDI, SNIP, CDI, firmware
setAuto(2, 2);
const rc = create(0x050101010707n);
if (rc !== 0) { throw new Error("create_node failed: " + rc); }

setInterval(run,  10);
setInterval(tick, 100);

socket.onmessage = (ev) => rx(ev.data);
```

Passing CDI / FDI bytes:

```js
const cdiBytes = new Uint8Array(/* your CDI XML, null-terminated */);
const heapPtr  = Module._malloc(cdiBytes.length);
Module.HEAPU8.set(cdiBytes, heapPtr);
Module.cwrap("wasm_node_set_cdi", "number", ["number","number"])(heapPtr, cdiBytes.length);
Module._free(heapPtr);  // C copied the bytes; JS can release immediately
```

## Memory and threading

- Static pools only — no `malloc` in the protocol core.  Pool depths are set
  in [`openlcb_user_config.h`](openlcb_user_config.h).
- `malloc` is used in `bindings.c` to retain per-node `node_parameters_t`
  and CDI/FDI blobs for the lifetime of the node.
- Single-threaded.  The `lock_shared_resources` / `unlock_shared_resources`
  callbacks are no-ops in this build.
- `ALLOW_MEMORY_GROWTH` is enabled; initial stack is 64 KB.

## Layout

```
wasm/
  CMakeLists.txt          Emscripten build config
  build.sh                one-command build
  bindings.c              C entry points + JS trampolines
  openlcb_user_config.h   feature flags, buffer sizes
  can_user_config.h       CAN driver pool size
  dist/                   build output (gitignored)
```

## By design

These are deliberate choices, not pending work:

- **Streams and datagrams are protocol-internal.**  The main library
  intentionally has no user-facing `send_datagram` / `send_stream` API —
  these transports exist to move data for config memory, firmware, etc.,
  and are handled entirely inside the library.  The WASM wrapper follows
  the same contract: no stream / datagram origination from JS.  Stream
  observe-only callbacks remain available for debugging.
- **Firmware upgrade is excluded** (`OPENLCB_COMPILE_FIRMWARE` unset) — a
  WASM node reloads from source on every page load, there is nothing to
  upgrade.
- **No `wasm_destroy_node`.**  OpenLCB has no protocol-level node removal;
  nodes live for the lifetime of the WASM instance.

## Consumer caveats

Things the JS wrapper needs to handle; none of these are C-side gaps:

- **Config-memory persistence lives in JS.**  `Module.onConfigMemRead` and
  `onConfigMemWrite` are wired; if the wrapper does not set them, reads
  return zero bytes and writes are silently dropped.  Back the `0xFD`
  space (and `0xFB` ACDI-user, for SNIP user name / description) with
  IndexedDB, localStorage, or a file.  Persistence across page reloads is
  the wrapper's responsibility.
- **Enum values cross the ABI as plain integers.**
  `train_emergency_type_enum`, `event_range_count_enum`,
  `event_status_enum`, `dcc_detector_direction_enum`, and
  `dcc_detector_address_type_enum` are passed as `u32` over `cwrap` / the
  JS hooks.  The wrapper must keep its enum ↔ integer mapping in sync with
  `src/openlcb/openlcb_types.h`.
- **`wasm_send_event_pc_report` does not validate** that `event_id` is in
  the node's producer list.  The C call always succeeds; downstream nodes
  may ignore an un-produced event.  Wrappers that want stricter semantics
  should track produced IDs on the JS side and reject locally.
