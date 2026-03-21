<!--
  ============================================================
  STATUS: REJECTED — SUPERSEDED BY INTENTIONAL DESIGN DECISION

  The original plan proposed populating non-zero destination fields when the triggering
  Verify Node ID was addressed.  During implementation the design was deliberately changed:
  destination fields are always zeroed (unaddressed) regardless of how the request arrived.
  The developer documented this choice in the Doxygen comment at line 96-98 of
  protocol_message_network.c:
  "always unaddressed (dest fields zeroed) regardless of whether the triggering Verify
  Node ID was global or addressed."  This cites MessageNetworkS §3.4.2.

  Rationale (v1.0.0 decision): The OpenLCB Message Network Standard §3.4.2 specifies that
  Verified Node ID is a broadcast message and must be unaddressed on all transports.
  The earlier plan pre-dated a closer reading of the spec.  Zeroing the dest fields
  unconditionally is correct per the standard.  This plan is closed as superseded by
  the spec, not as a defect.
  ============================================================
-->

# Plan: Item 14 -- Verified Node ID Response Has Non-Zero Dest Fields for Global Message

## 1. Summary

Clear the destination alias and destination ID fields to zero in the outgoing Verified Node ID
message when replying to a global Verify Node ID request, so the response is a proper global
(unaddressed) message on all transports.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network.c` | Split `_load_verified_node_id()` into two paths or add a parameter so the global path passes 0/0 for dest fields |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network_Test.cxx` | Add assertions for `dest_alias == 0` and `dest_id == 0` in global tests; add assertions for correct dest values in addressed tests |

## 3. Implementation Steps

There are two viable approaches. **Approach A (recommended)** is the simplest and most
localized change. **Approach B** is included as an alternative if future callers need finer
control.

### Approach A: Add a `bool is_addressed` parameter to `_load_verified_node_id()`

This approach keeps a single helper function and selects between zero and non-zero destination
fields based on a boolean flag.

**Step 1.** Change the signature of the static helper from:

```c
static void _load_verified_node_id(openlcb_statemachine_info_t *statemachine_info)
```

to:

```c
static void _load_verified_node_id(
            openlcb_statemachine_info_t *statemachine_info,
            bool is_addressed)
```

**Step 2.** Inside `_load_verified_node_id()`, conditionally choose the dest fields:

```c
    /** @brief Build Verified Node ID (or _SIMPLE) reply with this node's ID. */
static void _load_verified_node_id(
            openlcb_statemachine_info_t *statemachine_info,
            bool is_addressed) {

    uint16_t mti = MTI_VERIFIED_NODE_ID;

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    }

    uint16_t dest_alias = 0;
    uint64_t dest_id = 0;

    if (is_addressed) {

        dest_alias = statemachine_info->incoming_msg_info.msg_ptr->source_alias;
        dest_id = statemachine_info->incoming_msg_info.msg_ptr->source_id;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            dest_alias,
            dest_id,
            mti);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->id,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}
```

**Step 3.** Update the two call sites:

In `ProtocolMessageNetwork_handle_verify_node_id_global()` (two places -- once for the
matching-payload path, once for the empty-payload path), pass `false`:

```c
    _load_verified_node_id(statemachine_info, false);
```

In `ProtocolMessageNetwork_handle_verify_node_id_addressed()`, pass `true`:

```c
    _load_verified_node_id(statemachine_info, true);
```

---

### Approach B: Split into two separate static helpers

This approach eliminates the boolean parameter in favor of two distinct functions.

**Step 1.** Rename the existing helper and create a second one:

```c
    /** @brief Build Verified Node ID reply for a global (unaddressed) response. */
static void _load_verified_node_id_global(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t mti = MTI_VERIFIED_NODE_ID;

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            mti);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->id,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /** @brief Build Verified Node ID reply for an addressed response. */
static void _load_verified_node_id_addressed(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t mti = MTI_VERIFIED_NODE_ID;

    if (statemachine_info->openlcb_node->parameters->protocol_support & PSI_SIMPLE) {

        mti = MTI_VERIFIED_NODE_ID_SIMPLE;

    }

    OpenLcbUtilities_load_openlcb_message(statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            mti);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->id,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}
```

**Step 2.** Update the call sites:

- `handle_verify_node_id_global()` calls `_load_verified_node_id_global()`
- `handle_verify_node_id_addressed()` calls `_load_verified_node_id_addressed()`

**Tradeoff:** Approach B duplicates the MTI selection logic but makes each call site
self-documenting. Approach A avoids duplication with a single boolean parameter.

---

### Recommendation

**Use Approach A.** It is the smallest diff, avoids code duplication, and the boolean
parameter name (`is_addressed`) is self-documenting. The function is `static` so the parameter
is not part of any public API.

## 4. Test Changes

All changes are in `src/openlcb/protocol_message_network_Test.cxx`.

### 4a. Add dest-field assertions to existing global tests

Add two assertions to each of the two global tests that produce a response
(`handle_verify_node_id_global_empty_payload` and
`handle_verify_node_id_global_matching_payload`):

```cpp
    // After the existing EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
```

These assertions will **fail before the fix** (dest_alias would be SOURCE_ALIAS = 0x222,
dest_id would be SOURCE_ID = 0x010203040506) and **pass after the fix**.

### 4b. Add dest-field assertions to existing addressed tests

Add two assertions to each addressed test (`handle_verify_node_id_addressed_full` and
`handle_verify_node_id_addressed_simple`):

```cpp
    // After the existing EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, SOURCE_ALIAS);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) SOURCE_ID);
```

These assertions confirm that the addressed path still correctly populates destination fields.

### 4c. (Optional) Add a dedicated regression test

A new test that explicitly verifies the fix is in place, named clearly for the regression:

```cpp
// ============================================================================
// TEST: Verified Node ID Global - Destination Fields Must Be Zero
// @details Regression test for TODO Item 14: global Verified Node ID must
//          have dest_alias = 0 and dest_id = 0 for transport independence.
// @coverage ProtocolMessageNetwork_handle_verify_node_id_global()
// ============================================================================

TEST(ProtocolMessageNetwork, handle_verify_node_id_global_dest_fields_zero)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(openlcb_msg, nullptr);
    ASSERT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;

    // Set up incoming global verify with a non-zero source (the requester)
    OpenLcbUtilities_load_openlcb_message(openlcb_msg,
            SOURCE_ALIAS, SOURCE_ID, 0, 0, MTI_VERIFY_NODE_ID_GLOBAL);
    openlcb_msg->payload_count = 0;

    ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

    // Response must be valid and global (dest fields zeroed)
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
    EXPECT_EQ(outgoing_msg->dest_alias, 0);
    EXPECT_EQ(outgoing_msg->dest_id, (uint64_t) 0);
    EXPECT_EQ(outgoing_msg->source_alias, DEST_ALIAS);
    EXPECT_EQ(outgoing_msg->source_id, (uint64_t) DEST_ID);
    EXPECT_EQ(outgoing_msg->payload_count, 6);
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0), DEST_ID);

}
```

### 4d. Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

All tests must pass. Specifically the new/modified assertions in:
- `handle_verify_node_id_global_empty_payload`
- `handle_verify_node_id_global_matching_payload`
- `handle_verify_node_id_addressed_full`
- `handle_verify_node_id_addressed_simple`
- `handle_verify_node_id_global_dest_fields_zero` (new test)

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| CAN transport regression | VERY LOW | CAN framing ignores dest fields for global MTIs (0x0170 has address-present bit clear). Zeroing fields that were already ignored on CAN changes nothing. |
| Addressed path regression | LOW | The addressed call site explicitly passes `true`, preserving identical behavior to the original code. Existing addressed tests with new dest assertions confirm this. |
| `_load_duplicate_node_id()` same pattern | INFO | The same source-to-dest copy pattern exists in `_load_duplicate_node_id()` for PC Event Report (MTI 0x05B4, also global). That is a separate item and is NOT addressed in this plan. |
| PSI_SIMPLE path | VERY LOW | Both approaches preserve the existing `PSI_SIMPLE` check for MTI selection. The `handle_verify_node_id_addressed_simple` test confirms the simple-node path is unaffected. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Modify `_load_verified_node_id()` in canonical source | 10 min |
| Add test assertions + new regression test | 15 min |
| Build and run full test suite | 5 min |
| **Total** | **~30 min** |
