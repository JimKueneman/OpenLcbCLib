<!--
  ============================================================
  STATUS: REJECTED — SUPERSEDED BY INTENTIONAL DESIGN DECISION

  The original plan proposed making DATAGRAM_OK_REPLY_PENDING conditional on
  reply_pending_time_in_seconds > 0.  During implementation the design was deliberately
  changed: the bit is now ALWAYS set, regardless of the timeout value.  The developer
  documented this choice in a comment at line 1335 of protocol_datagram_handler.c:
  "The Reply Pending bit (DATAGRAM_OK_REPLY_PENDING, 0x80) is always set."

  Rationale (v1.0.0 decision): Setting the bit unconditionally is safe and simpler —
  a remote node that receives an unexpected "reply pending" flag simply waits briefly
  before timing out, which causes no protocol violation.  Removing the flag when there
  is no pending reply adds conditional complexity for negligible benefit.  This plan is
  therefore closed as superseded rather than as a defect.
  ============================================================
-->

# Plan: Item 1 -- Datagram OK Reply Always Sets Reply Pending Flag

## 1. Summary

The `ProtocolDatagramHandler_load_datagram_received_ok_message()` function always ORs
`DATAGRAM_OK_REPLY_PENDING` (0x80) with the exponent, even when `reply_pending_time_in_seconds`
is 0. This means every Datagram Received OK message incorrectly signals "reply pending" to the
remote node, even for simple acknowledgements that carry no pending reply.

The fix is a one-line change: only OR in the `DATAGRAM_OK_REPLY_PENDING` flag when the
`reply_pending_time_in_seconds` parameter is greater than zero.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/protocol_datagram_handler.c` | Conditionally apply `DATAGRAM_OK_REPLY_PENDING` flag only when `reply_pending_time_in_seconds > 0` |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_datagram_handler_Test.cxx` | Fix existing test at line ~3504 that expects buggy behavior; add new tests for zero and non-zero reply pending |

## 3. Implementation Steps

### Step 1. Fix the reply-pending flag logic

In `protocol_datagram_handler.c`, lines 1406-1409, change:

```c
    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_OK_REPLY_PENDING | exponent,
            0);
```

to:

```c
    uint8_t flags = exponent;

    if (reply_pending_time_in_seconds > 0) {

        flags |= DATAGRAM_OK_REPLY_PENDING;

    }

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            flags,
            0);
```

### Step 2. Update the Doxygen algorithm

Update the Doxygen comment block (lines 1306-1321) to document the conditional flag:

```
 * -# Compute exponent from reply_pending_time_in_seconds (power-of-2 ceiling)
 * -# Build MTI_DATAGRAM_OK_REPLY addressed back to the sender
 * -# If reply_pending_time_in_seconds > 0, set DATAGRAM_OK_REPLY_PENDING flag
 * -# Copy flags byte (reply pending flag | exponent) to payload[0]
 * -# Mark outgoing message valid
```

## 4. Test Changes

All changes are in `src/openlcb/protocol_datagram_handler_Test.cxx`.

### 4a. Fix existing test for zero-timeout case

Find the existing test that calls `ProtocolDatagramHandler_load_datagram_received_ok_message()`
with `reply_pending_time_in_seconds = 0` and expects `DATAGRAM_OK_REPLY_PENDING | 0` (0x80)
in the payload. Change the expected value to `0x00`:

```cpp
    // Was:  EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_msg, 0), 0x80);
    // Now:
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_msg, 0), 0x00);
```

### 4b. Add new test: simple OK with no reply pending

```cpp
// ============================================================================
// TEST: Datagram OK Reply - No Reply Pending
// @details Regression for TODO Item 1: when reply_pending_time_in_seconds == 0,
//          the reply pending flag (0x80) must NOT be set.
// @coverage ProtocolDatagramHandler_load_datagram_received_ok_message()
// ============================================================================

TEST(ProtocolDatagramHandler, datagram_ok_reply_no_pending)
{
    _reset_variables();
    _global_initialize();

    // ... setup statemachine_info with allocated nodes and buffers ...

    ProtocolDatagramHandler_load_datagram_received_ok_message(&statemachine_info, 0);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_OK_REPLY);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_msg, 0), 0x00);
}
```

### 4c. Add new test: reply pending with timeout

```cpp
// ============================================================================
// TEST: Datagram OK Reply - With Reply Pending
// @details Verifies that when reply_pending_time_in_seconds > 0, the
//          DATAGRAM_OK_REPLY_PENDING flag (0x80) IS set along with exponent.
// @coverage ProtocolDatagramHandler_load_datagram_received_ok_message()
// ============================================================================

TEST(ProtocolDatagramHandler, datagram_ok_reply_with_pending)
{
    _reset_variables();
    _global_initialize();

    // ... setup statemachine_info ...

    ProtocolDatagramHandler_load_datagram_received_ok_message(&statemachine_info, 4);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_OK_REPLY);
    // 4 seconds -> exponent 2, reply pending flag = 0x80 | 0x02 = 0x82
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(outgoing_msg, 0), 0x82);
}
```

### 4d. Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Existing callers passing 0 | VERY LOW | All callers passing 0 currently get incorrect 0x80 flag. After fix they correctly get 0x00. |
| Callers passing > 0 | NONE | Behavior is identical — flag was already set, exponent already computed. |
| Test regression | LOW | One existing test expects the buggy 0x80 byte. Must update that assertion. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Fix flag logic in source | 5 min |
| Update Doxygen | 5 min |
| Fix existing test + add 2 new tests | 15 min |
| Build and run full test suite | 5 min |
| **Total** | **~30 min** |
