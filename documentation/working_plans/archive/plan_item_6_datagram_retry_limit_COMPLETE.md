<!--
  ============================================================
  STATUS: REJECTED — SUPERSEDED BY DESIGN DECISION

  The plan proposed adding `datagram_retry_count` as a per-node field on
  openlcb_node_t.  The plan itself was later annotated: "STATUS: SUPERSEDED —
  Fully covered by Item 2 (plan_item_2_datagram_timeout.md)."

  The GOAL of this plan (enforce a maximum retry limit) IS fully implemented:
  - DATAGRAM_MAX_RETRIES = 3 is defined in protocol_datagram_handler.c (line 64).
  - The retry counter is stored as a 3-bit field (retry_count : 3) in the
    openlcb_msg_timer_t.datagram union on the message buffer itself, not as a
    per-node field. This was a conscious design improvement — the counter travels
    with the datagram rather than with the node.
  - ProtocolDatagramHandler_datagram_rejected() reads, increments, and enforces
    the limit correctly (lines ~1504-1513 of protocol_datagram_handler.c).

  This plan is closed as superseded, not as a defect.
  ============================================================
-->

# Plan: Item 6 -- No Datagram Retry Limit Enforcement

**STATUS: SUPERSEDED — Fully covered by Item 2 (plan_item_2_datagram_timeout.md).**
**The retry count is packed into the upper 3 bits of `openlcb_msg_t.timerticks`.**

## 1. Summary (Original — see Item 2 plan for current design)

When a datagram is rejected with a temporary error (0x2xxx), `resend_datagram` is set to true
with no retry counter, no maximum limit, and no backoff. An infinite retry loop is possible if
the remote node keeps rejecting temporarily.

The fix adds a retry counter per node and a maximum limit (3 retries, per DatagramTransportTN
Section 4.3). After the limit is reached, the pending datagram is abandoned.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_types.h` | Add `uint8_t datagram_retry_count` field to `openlcb_node_t` |
| `src/openlcb/protocol_datagram_handler.c` | Increment retry counter on temporary rejection; abandon after max retries; reset counter on OK or clear |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_datagram_handler_Test.cxx` | Add tests for retry counter increment, max retry abandonment, and counter reset |

## 3. Implementation Steps

### Step 1. Add retry counter to openlcb_node_t

In `openlcb_types.h`, add after `last_received_datagram`:

```c
        uint8_t datagram_retry_count;       /**< Number of temporary-error retries attempted */
```

### Step 2. Define maximum retry constant

In `protocol_datagram_handler.c` (or `openlcb_defines.h`):

```c
    /** @brief Maximum datagram retry attempts before abandoning. */
#define DATAGRAM_MAX_RETRIES 3
```

### Step 3. Modify the rejection handler

In `ProtocolDatagramHandler_datagram_rejected()` (lines 1480-1498), change:

```c
void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if ((OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) & ERROR_TEMPORARY) == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            statemachine_info->openlcb_node->state.resend_datagram = true;

        }

    } else {

        ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
```

to:

```c
void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if ((OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) & ERROR_TEMPORARY) == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            statemachine_info->openlcb_node->datagram_retry_count++;

            if (statemachine_info->openlcb_node->datagram_retry_count <= DATAGRAM_MAX_RETRIES) {

                statemachine_info->openlcb_node->state.resend_datagram = true;

            } else {

                // Max retries exceeded — abandon
                ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

            }

        }

    } else {

        ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}
```

### Step 4. Reset counter on success and on clear

In `ProtocolDatagramHandler_clear_resend_datagram_message()`, add:

```c
    openlcb_node->datagram_retry_count = 0;
```

This ensures the counter resets when:
- Datagram OK is received (calls `clear_resend_datagram_message`)
- Timeout fires (calls `clear_resend_datagram_message`)
- Max retries exceeded (calls `clear_resend_datagram_message`)
- Permanent error received (calls `clear_resend_datagram_message`)

### Step 5. Reset counter when datagram is first saved

Wherever `last_received_datagram` is assigned for a new outgoing datagram, reset the counter:

```c
    openlcb_node->datagram_retry_count = 0;
```

## 4. Test Changes

### Test 1: Retry counter increments on temporary error

```cpp
TEST(ProtocolDatagramHandler, retry_counter_increments)
{
    // Setup: node with pending datagram
    // Receive temporary rejection 3 times
    // Verify retry_count == 3 and resend_datagram still true after 3rd
}
```

### Test 2: Max retries reached — datagram abandoned

```cpp
TEST(ProtocolDatagramHandler, max_retries_abandons_datagram)
{
    // Setup: node with pending datagram, retry_count = 3
    // Receive 4th temporary rejection
    // Verify last_received_datagram == NULL and resend_datagram == false
}
```

### Test 3: Counter resets on OK reply

```cpp
TEST(ProtocolDatagramHandler, retry_counter_resets_on_ok)
{
    // Setup: node with retry_count > 0
    // Receive Datagram OK
    // Verify retry_count == 0
}
```

### Test 4: Permanent error clears immediately (no retry)

```cpp
TEST(ProtocolDatagramHandler, permanent_error_no_retry)
{
    // Setup: node with pending datagram
    // Receive permanent rejection (0x1xxx)
    // Verify last_received_datagram == NULL, retry_count == 0
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Field addition to openlcb_node_t | LOW | `OpenLcbNode_allocate()` zeroes the node structure, so the new field starts at 0. |
| Existing retry behavior | LOW | Current code retries forever. After fix, retries max 3 times. This is more correct per standard. |
| Counter not reset properly | LOW | Counter resets in `clear_resend_datagram_message()` which is the single cleanup path. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Add field to openlcb_types.h | 5 min |
| Define max retry constant | 2 min |
| Modify rejection handler | 10 min |
| Reset counter in clear function | 5 min |
| Add 4 tests | 20 min |
| Build and run full test suite | 5 min |
| **Total** | **~47 min** |
