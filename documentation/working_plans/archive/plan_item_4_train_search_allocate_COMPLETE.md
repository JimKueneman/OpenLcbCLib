<!--
  ============================================================
  STATUS: IMPLEMENTED
  `OpenLcbNode_is_last()` and `ProtocolTrainSearch_handle_search_no_match()` are both implemented, correctly invoking the `on_search_no_match` allocate callback when the ALLOCATE flag is set and no matching train is found.
  ============================================================
-->

# Plan: Item 4 -- Train Search Allocate Callback Never Called

## Context

The `on_search_no_match` callback in `interface_protocol_train_search_handler_t` is declared
(line 66 of `protocol_train_search_handler.h`) and wired in `openlcb_config.c` (line 243), but
it is **never invoked** anywhere in the codebase. When a train search event with the ALLOCATE
flag set arrives and no existing train node matches, the application should be notified so it can
create a new virtual train node. Without this callback, the command station silently ignores
allocation requests.

The specified approach: add an `is_last` function to the `openlcb_node` module, then use
it in the main statemachine's MTI_PRODUCER_IDENTIFY switch case to detect when all nodes have
been checked without a match.

## Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_node.c` | Add `OpenLcbNode_is_last(uint8_t key)` function |
| `src/openlcb/openlcb_node.h` | Add `extern bool OpenLcbNode_is_last(uint8_t key)` declaration |
| `src/openlcb/protocol_train_search_handler.c` | Add `ProtocolTrainSearch_handle_search_no_match()` function |
| `src/openlcb/protocol_train_search_handler.h` | Add `extern` declaration for the new function |
| `src/openlcb/openlcb_main_statemachine.h` | Add `openlcb_node_is_last` and `train_search_no_match_handler` to interface struct |
| `src/openlcb/openlcb_main_statemachine.c` | Add static match flag; modify MTI_PRODUCER_IDENTIFY case; reset flag in enumerate_first_node |
| `src/openlcb/openlcb_config.c` | Wire the two new interface pointers |

### Tests

| File | Change |
|------|--------|
| `src/openlcb/openlcb_node_Test.cxx` | Add test for `OpenLcbNode_is_last()` |
| `src/openlcb/protocol_train_search_handler_Test.cxx` | Add tests for `ProtocolTrainSearch_handle_search_no_match()` |

**Note:** Application copies in `applications/` are NOT edited -- they are updated via the
project's copy shell script.

## Implementation Steps

### Step 1. Add `OpenLcbNode_is_last()` to the node module

**`openlcb_node.c`** -- insert after `OpenLcbNode_get_next()` (after line 231):

```c
    /**
     * @brief Returns true if the current enumeration position is the last node.
     *
     * @details Algorithm:
     * -# Validate key is within range
     * -# Return false if no nodes allocated
     * -# Return true if current index equals count minus one
     *
     * @verbatim
     * @param key Same enumerator index used in the corresponding get_first/get_next calls
     * @endverbatim
     *
     * @return true if current enumeration position is the last allocated node
     */
bool OpenLcbNode_is_last(uint8_t key)
{

    if (key >= MAX_NODE_ENUM_KEY_VALUES) { return false; }

    if (_openlcb_nodes.count == 0) { return false; }

    return (_node_enum_index_array[key] >= _openlcb_nodes.count - 1);

}
```

**`openlcb_node.h`** -- insert after the `OpenLcbNode_get_next()` declaration (after line 114):

```c
        /**
         * @brief Returns true if the current enumeration position is the last node.
         *
         * @param key  Same enumerator index used in the corresponding get_first/get_next calls.
         *
         * @return true if current enumeration position is the last allocated node.
         */
    extern bool OpenLcbNode_is_last(uint8_t key);
```

### Step 2. Add `ProtocolTrainSearch_handle_search_no_match()` to the train search handler

**`protocol_train_search_handler.c`** -- insert after `ProtocolTrainSearch_handle_search_event()`
(after line 366):

```c
    /**
     * @brief Handles the no-match case after all train nodes have been checked.
     *
     * @details Called by the main statemachine when the last node in the
     * enumeration has been reached and no train node matched the search query.
     * If the ALLOCATE flag is set in the search event and the on_search_no_match
     * callback is registered, invokes it so the application can create a new
     * virtual train node.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit event_id_t containing encoded search query.
     * @endverbatim
     */
void ProtocolTrainSearch_handle_search_no_match(
        openlcb_statemachine_info_t *statemachine_info,
        event_id_t event_id) {

    if (!statemachine_info) { return; }

    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    if (!(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) { return; }

    if (!_interface || !_interface->on_search_no_match) { return; }

    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);
    uint16_t search_address = OpenLcbUtilities_train_search_digits_to_address(digits);

    openlcb_node_t *new_node = _interface->on_search_no_match(search_address, flags);

    if (new_node && new_node->train_state) {

        // Build Producer Identified reply from the newly allocated node
        uint8_t reply_flags = 0;
        if (new_node->train_state->is_long_address) {

            reply_flags |= TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR;

        } else {

            reply_flags |= TRAIN_SEARCH_FLAG_DCC;

        }
        reply_flags |= (new_node->train_state->speed_steps & TRAIN_SEARCH_SPEED_STEP_MASK);

        event_id_t reply_event = OpenLcbUtilities_create_train_search_event_id(
                new_node->train_state->dcc_address, reply_flags);

        OpenLcbUtilities_load_openlcb_message(
                statemachine_info->outgoing_msg_info.msg_ptr,
                new_node->alias,
                new_node->id,
                0,
                0,
                MTI_PRODUCER_IDENTIFIED_SET);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr, reply_event);

        statemachine_info->outgoing_msg_info.valid = true;

    }

}
```

**`protocol_train_search_handler.h`** -- insert after the existing `handle_search_event`
declaration (after line 91):

```c
    /**
     * @brief Handles the no-match case after full train search enumeration.
     *
     * @details If the ALLOCATE flag is set and on_search_no_match is registered,
     * invokes the callback to create a new virtual train node.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit @ref event_id_t containing encoded search query.
     */
    extern void ProtocolTrainSearch_handle_search_no_match(
            openlcb_statemachine_info_t *statemachine_info,
            event_id_t event_id);
```

### Step 3. Add interface pointers to the main statemachine

**`openlcb_main_statemachine.h`** -- add to the interface struct:

In the **Node Enumeration** section (after the `openlcb_node_get_next` pointer, after line 71):

```c
        /** @brief Return true if current enumeration position is the last node.  REQUIRED. */
    bool (*openlcb_node_is_last)(uint8_t key);
```

In the **Train Search Handler** section (after `train_search_event_handler`, after line 269):

```c
        /** @brief Called when train search enumeration completes with no match.  Optional. */
    void (*train_search_no_match_handler)(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);
```

### Step 4. Modify the main statemachine dispatch

**`openlcb_main_statemachine.c`**:

4a. Add a static flag after the existing static declarations (after line 90):

```c
    /** @brief Tracks whether any train node matched during the current enumeration. */
static bool _train_search_match_found;
```

4b. Reset the flag in `handle_try_enumerate_first_node()` (line 862). Insert at
the start of the `if (!_statemachine_info.openlcb_node)` block, after line 864:

```c
        _train_search_match_found = false;
```

4c. Replace the MTI_PRODUCER_IDENTIFY case (lines 448-473) with:

```c
        case MTI_PRODUCER_IDENTIFY: {

            event_id_t producer_event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr);

            bool is_train_search = _interface->train_search_event_handler &&
                                   OpenLcbUtilities_is_train_search_event(producer_event_id);

            if (is_train_search) {

                // Dispatch to train search handler for train nodes only
                if (statemachine_info->openlcb_node->train_state) {

                    _interface->train_search_event_handler(statemachine_info, producer_event_id);

                    if (statemachine_info->outgoing_msg_info.valid) {

                        _train_search_match_found = true;

                    }

                }

                // On last node with no match, invoke no-match handler
                if (_interface->openlcb_node_is_last(OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX) &&
                    !_train_search_match_found &&
                    _interface->train_search_no_match_handler) {

                    _interface->train_search_no_match_handler(statemachine_info, producer_event_id);

                }

                break;

            }

            if (_interface->event_transport_producer_identify) {

                _interface->event_transport_producer_identify(statemachine_info);

            }

            break;

        }
```

Key design decisions in this change:
- `is_train_search` is checked BEFORE the `train_state` guard so the is-last check works
  even if the last node in the pool is not a train node
- The match flag is set immediately after the handler runs (before `run()` returns and clears
  `outgoing_msg_info.valid`)
- The no-match handler is only called on the very last node when no match was found across
  the entire enumeration

### Step 5. Wire the new interface pointers in openlcb_config.c

In `_build_main_statemachine()`, add to the Node Enumeration section:

```c
    _main_sm.openlcb_node_is_last = &OpenLcbNode_is_last;
```

In the `#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)` block
(after line 543):

```c
    _main_sm.train_search_no_match_handler = &ProtocolTrainSearch_handle_search_no_match;
```

Also add `#include "openlcb_node.h"` to the includes if not already present.

## Test Changes

### Test 1: `openlcb_node_Test.cxx` -- Add `is_last` tests

Insert after the `get_next_end_of_list` test (after line 434):

```cpp
// ============================================================================
// TEST: Is Last - Single Node
// @details Verifies is_last returns true for a single allocated node
// @coverage OpenLcbNode_is_last()
// ============================================================================

TEST(OpenLcbNode, is_last_single_node)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    ASSERT_NE(node1, nullptr);

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    EXPECT_TRUE(OpenLcbNode_is_last(USER_ENUM_KEYS_VALUES_1));
}

// ============================================================================
// TEST: Is Last - Multiple Nodes
// @details Verifies is_last returns false for non-last, true for last
// @coverage OpenLcbNode_is_last()
// ============================================================================

TEST(OpenLcbNode, is_last_multiple_nodes)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    openlcb_node_t *node3 = OpenLcbNode_allocate(0x010203040508, &_node_parameters_main_node);

    EXPECT_EQ(OpenLcbNode_get_first(USER_ENUM_KEYS_VALUES_1), node1);
    EXPECT_FALSE(OpenLcbNode_is_last(USER_ENUM_KEYS_VALUES_1));

    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node2);
    EXPECT_FALSE(OpenLcbNode_is_last(USER_ENUM_KEYS_VALUES_1));

    EXPECT_EQ(OpenLcbNode_get_next(USER_ENUM_KEYS_VALUES_1), node3);
    EXPECT_TRUE(OpenLcbNode_is_last(USER_ENUM_KEYS_VALUES_1));
}

// ============================================================================
// TEST: Is Last - Invalid Key
// @details Verifies is_last returns false for out-of-range key
// @coverage OpenLcbNode_is_last()
// ============================================================================

TEST(OpenLcbNode, is_last_invalid_key)
{
    _global_initialize();
    _reset_variables();

    OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);

    EXPECT_FALSE(OpenLcbNode_is_last(MAX_NODE_ENUM_KEY_VALUES));
    EXPECT_FALSE(OpenLcbNode_is_last(MAX_NODE_ENUM_KEY_VALUES + 1));
}

// ============================================================================
// TEST: Is Last - Empty Pool
// @details Verifies is_last returns false when no nodes allocated
// @coverage OpenLcbNode_is_last()
// ============================================================================

TEST(OpenLcbNode, is_last_empty_pool)
{
    _global_initialize();
    _reset_variables();

    EXPECT_FALSE(OpenLcbNode_is_last(USER_ENUM_KEYS_VALUES_1));
}
```

### Test 2: `protocol_train_search_handler_Test.cxx` -- Add no-match tests

Add mock tracking variables and a mock callback, then add test cases. The exact
placement depends on the current test file structure, but the tests should:

1. **no_match_allocate_flag_set**: Verify that when ALLOCATE flag is set and
   `on_search_no_match` is registered, the callback IS invoked.

2. **no_match_allocate_flag_clear**: Verify that when ALLOCATE flag is NOT set,
   the callback is NOT invoked.

3. **no_match_null_interface**: Verify graceful handling when interface or
   callback is NULL.

4. **no_match_returns_node**: Verify that when `on_search_no_match` returns a
   valid node with `train_state`, a Producer Identified reply is loaded in
   `outgoing_msg_info`.

5. **no_match_returns_null**: Verify that when `on_search_no_match` returns NULL,
   no outgoing message is loaded.

## Verification

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

All test suites must pass. Key tests to verify:
- `OpenLcbNode` suite: 4 new `is_last_*` tests
- `ProtocolTrainSearchHandler` suite: 5 new `no_match_*` tests
- `OpenLcbMainStatemachine` suite: all existing tests still pass (interface struct grew)

## Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Interface struct size change | LOW | Adding two new pointers to `interface_openlcb_main_statemachine_t`. All existing call sites use designated initializers or memset+assign, so new fields default to NULL/0. |
| Non-train last node | LOW | The `is_train_search` check is done outside the `train_state` guard, so no-match detection works even if the last pool node is not a train. |
| Match flag false positive | VERY LOW | Flag is reset in `handle_try_enumerate_first_node()` for every message, ensuring no stale state from a previous message. |
| Existing producer identify tests | LOW | Non-train-search Producer Identify messages follow the same path as before (the `is_train_search` false path falls through to the existing handler). |
| on_search_no_match returns pre-logged-in node | INFO | The reply is built from the returned node's alias/id. If the node hasn't logged in, alias=0 and the CAN layer will not transmit. This is expected -- the node will respond organically after login. |

## Estimated Effort

| Task | Time |
|------|------|
| Add `OpenLcbNode_is_last()` (source + header) | 5 min |
| Add `ProtocolTrainSearch_handle_search_no_match()` (source + header) | 10 min |
| Modify main statemachine interface + dispatch + flag | 15 min |
| Wire in openlcb_config.c | 5 min |
| Add node tests (4 tests) | 10 min |
| Add train search no-match tests (5 tests) | 15 min |
| Update statemachine test mock interface (add new fields) | 5 min |
| Build and run full test suite | 5 min |
| **Total** | **~70 min** |
