<!--
  ============================================================
  STATUS: IMPLEMENTED
  `_load_controller_assign_reply()` in `protocol_train_handler.c` accepts `current_controller` and appends the 6-byte Node ID to rejection replies when `result != 0x00`, matching the plan spec.
  ============================================================
-->

# Plan: Item 5 -- Controller Assign Reject Reply Missing Current Controller Node ID

## 1. Summary

When Controller Assign is rejected (another controller already owns the train), the reject reply
is only 3 bytes: `[TRAIN_CONTROLLER_CONFIG, TRAIN_CONTROLLER_ASSIGN, 0xFF]`. The standard
(TrainControlTN Section 2.8) says the reply should include the 6-byte Node ID of the current
controller so the requester can negotiate a handoff.

The fix appends the current controller's Node ID (6 bytes) to the reject reply, making it
9 bytes total.

### Out of Scope: Controller Changed Notify to Previous Controller

The spec says the train node *may* send a Controller Changed Notify to the previous controller
when accepting a new one. This is explicitly optional ("may, but is not required to"). Most
implementations simply accept the handoff without notifying the old controller. Our code
already supports this: the `on_controller_assign_request` decision callback defaults to accept
(NULL = accept), and the application can reject if it wants. Adding the Controller Changed
Notify to the old controller is a separate optional enhancement, not part of this fix.

### No Alias Resolution Dependency

This fix does NOT need the listener alias table from Plan #3. The reject reply goes back to the
*requesting* throttle, whose alias is already known — `_load_reply_header()` uses the incoming
message's `source_alias` as the reply's `dest_alias`. No alias resolution needed.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/protocol_train_handler.c` | Modify `_load_controller_assign_reply()` to accept and append current controller Node ID on rejection |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_train_handler_Test.cxx` | Add assertions checking controller Node ID in reject reply payload; add new regression test |

## 3. Implementation Steps

### Step 1. Modify the reply builder

Change `_load_controller_assign_reply()` (line 223) from:

```c
static void _load_controller_assign_reply(openlcb_statemachine_info_t *statemachine_info, uint8_t result) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 2);

    statemachine_info->outgoing_msg_info.valid = true;

}
```

to:

```c
    /**
     * @brief Build a Controller Assign reply.
     *
     * @details Algorithm:
     * -# Build reply header (MTI_TRAIN_REPLY)
     * -# Write [TRAIN_CONTROLLER_CONFIG, TRAIN_CONTROLLER_ASSIGN, result]
     * -# If rejected (result != 0), append current controller Node ID (6 bytes)
     * -# Mark outgoing message valid
     *
     * @verbatim
     * @param statemachine_info  Context.
     * @param result             0x00 = accepted, 0xFF = rejected.
     * @param current_controller Node ID of current controller (only used on reject).
     * @endverbatim
     */
static void _load_controller_assign_reply(
            openlcb_statemachine_info_t *statemachine_info,
            uint8_t result,
            node_id_t current_controller) {

    _load_reply_header(statemachine_info);

    openlcb_msg_t *msg = statemachine_info->outgoing_msg_info.msg_ptr;

    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_CONFIG, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, TRAIN_CONTROLLER_ASSIGN, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, result, 2);

    if (result != 0x00) {

        OpenLcbUtilities_copy_node_id_to_openlcb_payload(msg, current_controller, 3);

    }

    statemachine_info->outgoing_msg_info.valid = true;

}
```

### Step 2. Update the call site in `_handle_controller_config()`

Currently (line 510):

```c
    _load_controller_assign_reply(statemachine_info, accepted ? 0x00 : 0xFF);
```

Change to:

```c
    if (accepted) {

        _load_controller_assign_reply(statemachine_info, 0x00, 0);

    } else {

        _load_controller_assign_reply(statemachine_info, 0xFF, state->controller_node_id);

    }
```

Note: The current controller's Node ID must be captured BEFORE the `accepted` check potentially
overwrites it. Looking at the code flow (lines 479-518), the `controller_node_id` is only
updated when `accepted == true`, so on rejection it still holds the original controller's ID.
This is correct.

## 4. Test Changes

All changes are in `src/openlcb/protocol_train_handler_Test.cxx`.

### 4a. Update existing reject tests

Find existing tests that verify Controller Assign rejection. Add assertions:

```cpp
    // After existing EXPECT_EQ for result byte:
    EXPECT_EQ(outgoing_msg->payload_count, 9);  // 3 header + 6 node ID
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 3),
              EXPECTED_CURRENT_CONTROLLER_ID);
```

### 4b. Add regression test

```cpp
// ============================================================================
// TEST: Controller Assign Reject includes current controller Node ID
// @details Regression for TODO Item 5: reject reply must include 6-byte
//          Node ID of the current controller per TrainControlTN §2.8.
// @coverage _load_controller_assign_reply(), _handle_controller_config()
// ============================================================================

TEST(ProtocolTrainHandler, controller_assign_reject_includes_node_id)
{
    _reset_variables();
    _global_initialize();

    // Setup: train with controller already assigned
    // ... allocate node, set controller_node_id to CONTROLLER_A ...

    // Send Controller Assign from CONTROLLER_B
    // ... build incoming message ...

    ProtocolTrainHandler_handle_train_command(&statemachine_info);

    // Verify rejection with current controller's Node ID
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    openlcb_msg_t *reply = statemachine_info.outgoing_msg_info.msg_ptr;
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(reply, 0), TRAIN_CONTROLLER_CONFIG);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(reply, 1), TRAIN_CONTROLLER_ASSIGN);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(reply, 2), 0xFF);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(reply, 3), CONTROLLER_A_NODE_ID);
}
```

### 4c. Add test for accept case (no Node ID appended)

```cpp
TEST(ProtocolTrainHandler, controller_assign_accept_no_extra_bytes)
{
    // Setup: train with no controller
    // Send Controller Assign
    // Verify reply is 3 bytes (no Node ID appended)
    EXPECT_EQ(reply->payload_count, 3);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(reply, 2), 0x00);
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Payload buffer overflow | VERY LOW | Reply uses BASIC buffer (16 bytes). 9 bytes fits easily. |
| payload_count accuracy | LOW | `_load_reply_header()` and `OpenLcbUtilities_copy_*` functions update payload_count automatically. Verify in tests. |
| Accepted case regression | LOW | Accepted path passes `current_controller = 0` but the `if (result != 0x00)` guard prevents it from being appended. Existing accept tests catch regressions. |
| Existing tests expecting 3-byte reject | MEDIUM | Any existing test checking reject reply length will need updating. |
