<!--
  ============================================================
  STATUS: IMPLEMENTED
  Both `on_optional_interaction_rejected` and `on_terminate_due_to_error` callbacks are defined in `protocol_message_network.h` and wired with proper bounds-checking handlers in `protocol_message_network.c`.
  ============================================================
-->

# Plan: Item 8 -- OIR and TDE Receipt Handlers Do Nothing

## 1. Summary

The handlers for Optional Interaction Rejected (OIR, MTI 0x0068) and Terminate Due To Error
(TDE, MTI 0x00A8) in `protocol_message_network.c` simply set `valid = false`. No error code
parsing, no MTI extraction, no cancellation of pending transactions, no application notification.
Remote rejections cause silent failure.

The fix adds basic error parsing and application notification through callbacks in the
`interface_openlcb_protocol_message_network_t` struct.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network.c` | Parse error code and rejected MTI from OIR/TDE payload; invoke callbacks |
| `src/openlcb/protocol_message_network.h` | Add `on_optional_interaction_rejected` and `on_terminate_due_to_error` callbacks to interface struct |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network_Test.cxx` | Add tests for error parsing and callback invocation |

## 3. Implementation Steps

### Step 1. Add callbacks to the interface struct

In `protocol_message_network.h`, expand the empty interface struct:

```c
    /** @brief Callback interface for message network protocol events. */
typedef struct {

        /**
         * @brief Called when an Optional Interaction Rejected message is received.
         *
         * @param openlcb_node    The node that received the OIR.
         * @param source_node_id  Node ID of the rejecting node.
         * @param error_code      16-bit error code from the rejection.
         * @param rejected_mti    MTI of the message that was rejected (0 if not present).
         */
    void (*on_optional_interaction_rejected)(
            openlcb_node_t *openlcb_node,
            node_id_t source_node_id,
            uint16_t error_code,
            uint16_t rejected_mti);

        /**
         * @brief Called when a Terminate Due To Error message is received.
         *
         * @param openlcb_node    The node that received the TDE.
         * @param source_node_id  Node ID of the terminating node.
         * @param error_code      16-bit error code.
         * @param rejected_mti    MTI of the terminated interaction (0 if not present).
         */
    void (*on_terminate_due_to_error)(
            openlcb_node_t *openlcb_node,
            node_id_t source_node_id,
            uint16_t error_code,
            uint16_t rejected_mti);

} interface_openlcb_protocol_message_network_t;
```

### Step 2. Implement OIR handler

Replace the current handler (lines 257-261):

```c
    /**
     * @brief Handle Optional Interaction Rejected.
     *
     * @details Algorithm:
     * -# Extract 16-bit error code from payload[0..1]
     * -# Extract rejected MTI from payload[2..3] if present
     * -# Invoke on_optional_interaction_rejected callback if registered
     * -# Set outgoing valid = false (no automatic reply to OIR)
     */
void ProtocolMessageNetwork_handle_optional_interaction_rejected(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t error_code = 0;
    uint16_t rejected_mti = 0;

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count >= 2) {

        error_code = OpenLcbUtilities_extract_word_from_openlcb_payload(
                statemachine_info->incoming_msg_info.msg_ptr, 0);

    }

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count >= 4) {

        rejected_mti = OpenLcbUtilities_extract_word_from_openlcb_payload(
                statemachine_info->incoming_msg_info.msg_ptr, 2);

    }

    if (_interface && _interface->on_optional_interaction_rejected) {

        _interface->on_optional_interaction_rejected(
                statemachine_info->openlcb_node,
                statemachine_info->incoming_msg_info.msg_ptr->source_id,
                error_code,
                rejected_mti);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
```

### Step 3. Implement TDE handler

Replace the current handler (lines 264-268):

```c
    /**
     * @brief Handle Terminate Due To Error.
     *
     * @details Algorithm:
     * -# Extract 16-bit error code from payload[0..1]
     * -# Extract terminated MTI from payload[2..3] if present
     * -# Invoke on_terminate_due_to_error callback if registered
     * -# Set outgoing valid = false (no automatic reply to TDE)
     */
void ProtocolMessageNetwork_handle_terminate_due_to_error(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t error_code = 0;
    uint16_t rejected_mti = 0;

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count >= 2) {

        error_code = OpenLcbUtilities_extract_word_from_openlcb_payload(
                statemachine_info->incoming_msg_info.msg_ptr, 0);

    }

    if (statemachine_info->incoming_msg_info.msg_ptr->payload_count >= 4) {

        rejected_mti = OpenLcbUtilities_extract_word_from_openlcb_payload(
                statemachine_info->incoming_msg_info.msg_ptr, 2);

    }

    if (_interface && _interface->on_terminate_due_to_error) {

        _interface->on_terminate_due_to_error(
                statemachine_info->openlcb_node,
                statemachine_info->incoming_msg_info.msg_ptr->source_id,
                error_code,
                rejected_mti);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
```

### Step 4. Wire callbacks in openlcb_config.c (optional for now)

The callbacks can be wired to NULL initially. Application-level handling of OIR/TDE is
optional — the key improvement is that the error information is now parsed and available.

## 4. Test Changes

All changes are in `src/openlcb/protocol_message_network_Test.cxx`.

### Test 1: OIR with error code and rejected MTI

```cpp
TEST(ProtocolMessageNetwork, oir_parses_error_code_and_mti)
{
    // Setup: incoming OIR with 4-byte payload [error_hi, error_lo, mti_hi, mti_lo]
    // Call handler
    // Verify callback received correct error_code and rejected_mti
    // Verify outgoing valid == false
}
```

### Test 2: OIR with only error code (2 bytes)

```cpp
TEST(ProtocolMessageNetwork, oir_parses_error_code_only)
{
    // Setup: incoming OIR with 2-byte payload
    // Verify callback received error_code, rejected_mti == 0
}
```

### Test 3: OIR with empty payload

```cpp
TEST(ProtocolMessageNetwork, oir_empty_payload)
{
    // Setup: incoming OIR with 0-byte payload
    // Verify callback received error_code == 0, rejected_mti == 0
}
```

### Test 4: TDE with error code and MTI

```cpp
TEST(ProtocolMessageNetwork, tde_parses_error_code_and_mti)
{
    // Similar to OIR test but for TDE handler
}
```

### Test 5: OIR with NULL callback

```cpp
TEST(ProtocolMessageNetwork, oir_null_callback_no_crash)
{
    // Setup: interface with NULL on_optional_interaction_rejected
    // Call handler
    // Verify no crash, outgoing valid == false
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Interface struct growth | LOW | Was empty (0 bytes). Adding 2 function pointers. All existing code that initializes it must be updated (add NULL for new fields). |
| Existing test initialization | MEDIUM | Test mock must include new fields. Empty initializer `{}` may need explicit NULLs for the new callbacks. |
| No automatic reply | NONE | Standard does not require a reply to OIR or TDE. The handler correctly sets valid = false. |
| Payload bounds checking | LOW | Guards check `payload_count >= 2` and `>= 4` before extracting. Short payloads handled gracefully. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Add callbacks to interface struct | 10 min |
| Implement OIR handler | 10 min |
| Implement TDE handler | 10 min |
| Update test mock initialization | 5 min |
| Add 5 tests | 20 min |
| Build and run full test suite | 5 min |
| **Total** | **~60 min** |
