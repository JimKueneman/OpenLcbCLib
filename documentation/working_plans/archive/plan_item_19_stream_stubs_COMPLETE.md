<!--
  ============================================================
  STATUS: IMPLEMENTED
  `openlcb_main_statemachine.c` sends OIR for `MTI_STREAM_INIT_REQUEST`, `MTI_STREAM_SEND`, and `MTI_STREAM_COMPLETE` when handlers are NULL, leaving reply MTIs unchanged as planned.
  ============================================================
-->

# Plan: Item 19 -- Stream Support Stubs Send OIR

## Summary

Stream MTIs (`MTI_STREAM_INIT_REQUEST`, `MTI_STREAM_INIT_REPLY`, `MTI_STREAM_SEND`,
`MTI_STREAM_PROCEED`, `MTI_STREAM_COMPLETE`) are dispatched by the main statemachine
but the interface function pointers are always NULL. When a remote node sends a stream
request to this node, the message is silently dropped. The OpenLCB MessageNetworkS
standard requires that an addressed message with no handler must be answered with an
Optional Interaction Rejected (OIR) reply.

This plan covers the **minimum viable fix**: add `else` branches to the stream
`case` statements so that request-type stream MTIs send OIR when the handler is NULL.
Reply/indication-type stream MTIs remain silently ignored (consistent with how all
other reply MTIs are handled in the statemachine).

Full stream protocol implementation is out of scope and noted as future work.

---

## Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine.c` | Add `else { _interface->load_interaction_rejected(statemachine_info); }` to the three request-type stream cases |

### Test

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine_Test.cxx` | Update five existing null-handler stream tests; three flip from `EXPECT_FALSE` to `EXPECT_TRUE`, two remain `EXPECT_FALSE` |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

---

## Implementation Steps

### Phase A: Immediate Fix -- Send OIR for Unhandled Stream Requests

#### Step 1: Classify Stream MTIs as Request vs Reply

All five stream MTIs are addressed messages. They must be classified to determine
which ones require an OIR when unhandled:

| MTI | Name | Type | OIR on NULL? |
|-----|------|------|--------------|
| `0x0CC8` | `MTI_STREAM_INIT_REQUEST` | Request | Yes |
| `0x0868` | `MTI_STREAM_INIT_REPLY` | Reply | No |
| `0x1F88` | `MTI_STREAM_SEND` | Request (data push) | Yes |
| `0x0888` | `MTI_STREAM_PROCEED` | Reply (flow control) | No |
| `0x08A8` | `MTI_STREAM_COMPLETE` | Request (sender closes) | Yes |

Rationale:

- `STREAM_INIT_REQUEST` -- A remote node is asking us to open a stream. If we cannot
  handle streams, we must reject.
- `STREAM_SEND` -- A remote node is pushing data into a stream. If we have no handler,
  the sender needs to know we do not support this. Sending OIR stops the sender from
  continuing to push data that will never be processed.
- `STREAM_COMPLETE` -- The sender is telling us the stream is done. If we have no
  handler, rejecting is the safe response so the sender knows the data was not
  consumed.
- `STREAM_INIT_REPLY` and `STREAM_PROCEED` -- These are replies to messages we
  initiated. If we have no stream handler, we would never have initiated a stream, so
  receiving these is a protocol error on the remote side. Silently ignoring is correct
  (consistent with how `MTI_SIMPLE_NODE_INFO_REPLY`, `MTI_TRAIN_REPLY`, etc. are
  handled).

This matches the established pattern in the statemachine: request-type MTIs with NULL
handlers call `_interface->load_interaction_rejected()` (see `MTI_SIMPLE_NODE_INFO_REQUEST`
at line 275, `MTI_TRAIN_PROTOCOL` at line 618, `MTI_SIMPLE_TRAIN_INFO_REQUEST` at
line 642), while reply-type MTIs with NULL handlers are silently ignored.

#### Step 2: Edit `openlcb_main_statemachine.c`

Add `else` branches to three of the five stream cases. The existing code for
`MTI_STREAM_INIT_REQUEST` (lines 694-702) currently reads:

```c
        case MTI_STREAM_INIT_REQUEST:

            if (_interface->stream_initiate_request) {

                _interface->stream_initiate_request(statemachine_info);

            }

            break;
```

Change to:

```c
        case MTI_STREAM_INIT_REQUEST:

            if (_interface->stream_initiate_request) {

                _interface->stream_initiate_request(statemachine_info);

            } else {

                _interface->load_interaction_rejected(statemachine_info);

            }

            break;
```

Apply the same pattern to `MTI_STREAM_SEND` (handler: `stream_send_data`) and
`MTI_STREAM_COMPLETE` (handler: `stream_data_complete`).

Leave `MTI_STREAM_INIT_REPLY` and `MTI_STREAM_PROCEED` unchanged (no `else` branch).

#### Step 3: Update Tests in `openlcb_main_statemachine_Test.cxx`

The test file already has five null-handler stream tests (lines 3474-3576). Three of
them currently assert `EXPECT_FALSE(load_interaction_rejected_called)` and must be
flipped to `EXPECT_TRUE`:

1. **`null_handler_stream_init_request`** (line 3474) -- Change `EXPECT_FALSE` to
   `EXPECT_TRUE`. This is a request MTI; OIR is now expected.

2. **`null_handler_stream_init_reply`** (line 3496) -- Keep `EXPECT_FALSE`. This is a
   reply MTI; no OIR.

3. **`null_handler_stream_send`** (line 3516) -- Change `EXPECT_FALSE` to
   `EXPECT_TRUE`. This is a request/data-push MTI; OIR is now expected.

4. **`null_handler_stream_proceed`** (line 3538) -- Keep `EXPECT_FALSE`. This is a
   reply/flow-control MTI; no OIR.

5. **`null_handler_stream_complete`** (line 3558) -- Change `EXPECT_FALSE` to
   `EXPECT_TRUE`. This is a close-request MTI; OIR is now expected.

#### Step 4: Build and Run Tests

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

Verify:
- All existing tests still pass.
- The three updated null-handler stream tests now pass with `EXPECT_TRUE`.
- The two unchanged null-handler stream tests still pass with `EXPECT_FALSE`.
- No regressions in any other test suite.

---

### Phase B: Future Work -- Full Stream Protocol (Out of Scope)

Full stream implementation would require:

- New files: `protocol_stream_handler.c` / `.h` (Layer 1 dispatcher)
- Optional: `openlcb_application_stream.c` / `.h` (Layer 2 application helper)
- Stream state tracking: buffer IDs, negotiated window sizes, in-progress transfers
- CAN-level fragmentation: `MTI_FRAME_TYPE_CAN_STREAM_SEND` (`0xF000`) handling in
  `can_tx_message_handler.c` and `can_rx_message_handler.c`
- Interface wiring in `openlcb_config.c` (`_build_*()` functions)
- Feature flag gating: `OPENLCB_COMPILE_STREAM` / `PSI_STREAM`
- Timer integration for stream timeouts
- Comprehensive test suite

This is a large separate effort and is explicitly not part of this plan.

---

## Test Changes

| Test Name | File | Current Assertion | New Assertion | Reason |
|-----------|------|-------------------|---------------|--------|
| `null_handler_stream_init_request` | `openlcb_main_statemachine_Test.cxx:3493` | `EXPECT_FALSE` | `EXPECT_TRUE` | Request MTI now sends OIR |
| `null_handler_stream_init_reply` | `openlcb_main_statemachine_Test.cxx:3513` | `EXPECT_FALSE` | `EXPECT_FALSE` (no change) | Reply MTI still silently ignored |
| `null_handler_stream_send` | `openlcb_main_statemachine_Test.cxx:3535` | `EXPECT_FALSE` | `EXPECT_TRUE` | Data-push MTI now sends OIR |
| `null_handler_stream_proceed` | `openlcb_main_statemachine_Test.cxx:3555` | `EXPECT_FALSE` | `EXPECT_FALSE` (no change) | Flow-control reply still silently ignored |
| `null_handler_stream_complete` | `openlcb_main_statemachine_Test.cxx:3575` | `EXPECT_FALSE` | `EXPECT_TRUE` | Close-request MTI now sends OIR |

No new test functions are needed. The existing tests already set up the correct
statemachine context with null handlers and check the `load_interaction_rejected_called`
flag.

---

## Risk Assessment

**Risk: LOW**

- The change is small and mechanical: three `else` branches added, following an
  established pattern already used for `MTI_SIMPLE_NODE_INFO_REQUEST`,
  `MTI_TRAIN_PROTOCOL`, and `MTI_SIMPLE_TRAIN_INFO_REQUEST`.
- No stream handlers are wired in any existing application configuration, so the
  `else` branch will always execute for stream messages today. This is the desired
  behavior: nodes that do not support streams now correctly reject stream requests
  instead of silently dropping them.
- The OIR payload uses `ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL`
  (`0x1043`), which is the same error code used by the default case and by
  `OpenLcbMainStatemachine_load_interaction_rejected()`. This tells the remote node
  that the MTI is recognized at the dispatch level but not implemented.
- Reply-type MTIs (`STREAM_INIT_REPLY`, `STREAM_PROCEED`) are intentionally left
  without OIR. If a future full stream implementation wires up handlers, the `else`
  branches become dead code and can be removed or will simply never execute.
- No risk of breaking existing functionality since stream handlers are universally
  NULL across all application configurations.

---

## Estimated Effort

| Task | Time |
|------|------|
| Edit `openlcb_main_statemachine.c` (3 else branches) | 10 minutes |
| Update 3 test assertions | 5 minutes |
| Build and run tests | 5 minutes |
| **Total** | **~20 minutes** |
