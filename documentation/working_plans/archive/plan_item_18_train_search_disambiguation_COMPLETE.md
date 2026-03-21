<!--
  ============================================================
  STATUS: IMPLEMENTED
  `_does_train_match()` in `protocol_train_search_handler.c` uses the corrected nested `if/else if` structure that rejects short-address trains when the search address >= 128, with ALLOCATE flag bypass applied correctly.
  ============================================================
-->

# Implementation Plan: Item 18 -- Train Search Short/Long Address Disambiguation

## 1. Summary

When a DCC train search arrives **without** the `TRAIN_SEARCH_FLAG_LONG_ADDR` flag and
the search address is >= 128, the current code in `_does_train_match()` allows the
search to match both short-address and long-address trains. Per TrainSearchS Section 4.4,
addresses >= 128 without the long flag should only match **long-address** trains, because
short DCC addresses are restricted to the range 0-127. The fix is a single additional
`else if` clause in the DCC disambiguation block.

The Allocate flag continues to bypass this filtering, because an Allocate search is asking
the system to create a new train node -- it should not reject existing matches based on
address-type heuristics.

## 2. Files to Modify

### Source (1 file)

| File | Change |
|------|--------|
| `src/openlcb/protocol_train_search_handler.c` | Fix the `else` branch in `_does_train_match()` (lines 258-266) |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests (1 file)

| File | Change |
|------|--------|
| `src/openlcb/protocol_train_search_handler_Test.cxx` | Add new Section 11 with comprehensive disambiguation tests |

## 3. Implementation Steps

### 3.1 Understand the Current Logic

In `_does_train_match()` at lines 252-268, the DCC protocol check is:

```c
if (flags & TRAIN_SEARCH_FLAG_DCC) {

    if (flags & TRAIN_SEARCH_FLAG_LONG_ADDR) {

        if (!train_state->is_long_address) { return false; }

    } else {

        if (search_address < 128 && train_state->is_long_address && !(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

            return false;

        }

    }

}
```

**Current behavior truth table** (LONG_ADDR flag NOT set):

| search_address | train is_long_address | ALLOCATE | Result  | Correct? |
|---------------|-----------------------|----------|---------|----------|
| < 128         | false (short)         | no       | allow   | YES      |
| < 128         | true (long)           | no       | reject  | YES      |
| < 128         | true (long)           | yes      | allow   | YES      |
| >= 128        | true (long)           | no       | allow   | YES      |
| >= 128        | false (short)         | no       | allow   | **NO -- should reject** |
| >= 128        | false (short)         | yes      | allow   | YES (Allocate bypass) |

### 3.2 The Fix

Replace the `else` branch with logic that handles both directions of the mismatch.
The corrected code rejects when:
- `search_address < 128` AND train is long-address AND NOT Allocate (existing rule), OR
- `search_address >= 128` AND train is NOT long-address AND NOT Allocate (**new rule**)

Both conditions share the same structure: the address range implies a specific address
type, and the train's actual type does not match.

### 3.3 Exact Code Change

**File:** `src/openlcb/protocol_train_search_handler.c`

**Replace** lines 258-266 (the `else` block):

```c
    } else {

        if (search_address < 128 && train_state->is_long_address && !(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

            return false;

        }

    }
```

**With:**

```c
    } else {

        if (!(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

            if (search_address < 128 && train_state->is_long_address) {

                return false;

            } else if (search_address >= 128 && !train_state->is_long_address) {

                return false;

            }

        }

    }
```

### 3.4 Corrected Truth Table

| search_address | train is_long_address | ALLOCATE | Result  | Correct? |
|---------------|-----------------------|----------|---------|----------|
| < 128         | false (short)         | no       | allow   | YES      |
| < 128         | true (long)           | no       | reject  | YES      |
| < 128         | true (long)           | yes      | allow   | YES      |
| >= 128        | true (long)           | no       | allow   | YES      |
| >= 128        | false (short)         | no       | reject  | YES      |
| >= 128        | false (short)         | yes      | allow   | YES      |

### 3.5 Full Function After Fix

For clarity, the complete `_does_train_match()` DCC block after the fix:

```c
    // Check DCC protocol match
    if (flags & TRAIN_SEARCH_FLAG_DCC) {

        if (flags & TRAIN_SEARCH_FLAG_LONG_ADDR) {

            if (!train_state->is_long_address) { return false; }

        } else {

            if (!(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

                if (search_address < 128 && train_state->is_long_address) {

                    return false;

                } else if (search_address >= 128 && !train_state->is_long_address) {

                    return false;

                }

            }

        }

    }
```

## 4. Test Changes

### 4.1 New Test Section

Add **Section 11** to `src/openlcb/protocol_train_search_handler_Test.cxx` after the
existing Section 10 (line 1056). The new section header:

```
// ============================================================================
// Section 11: Handler -- Short/Long Address Disambiguation Tests (Item 18)
// ============================================================================
```

### 4.2 Test Cases

Each test follows the established pattern: `_global_initialize()`, create a train node
with a specific address and is_long_address setting, build a search event with specific
flags, call `ProtocolTrainSearch_handle_search_event()`, and assert on
`sm.outgoing_msg_info.valid`.

All tests use `TRAIN_SEARCH_FLAG_DCC` in the search (without `TRAIN_SEARCH_FLAG_LONG_ADDR`
unless testing the LONG_ADDR path explicitly).

#### Test 1: Short address search (< 128) matches short-address train

```cpp
TEST(TrainSearch, handler_disambig_short_search_matches_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC, no LONG_ADDR flag
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Short search, short train -- should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 2: Short address search (< 128) rejects long-address train

```cpp
TEST(TrainSearch, handler_disambig_short_search_rejects_long_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, true); // long-address train at addr 42

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 42, DCC, no LONG_ADDR flag
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            42, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Short search, long train -- should reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}
```

#### Test 3: High address search (>= 128) matches long-address train

```cpp
TEST(TrainSearch, handler_disambig_high_search_matches_long_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, DCC, no LONG_ADDR flag
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            200, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // High search, long train -- should match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 4: High address search (>= 128) rejects short-address train -- THE BUG FIX

```cpp
TEST(TrainSearch, handler_disambig_high_search_rejects_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false); // short-address at 200

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for address 200, DCC, no LONG_ADDR flag
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            200, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // High search, short train -- should REJECT (this is the bug)
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}
```

#### Test 5: Boundary -- address exactly 128, short train, rejected

```cpp
TEST(TrainSearch, handler_disambig_boundary_128_rejects_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 128, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            128, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 128 without LONG_ADDR -- short train rejected
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}
```

#### Test 6: Boundary -- address exactly 127, short train, allowed

```cpp
TEST(TrainSearch, handler_disambig_boundary_127_allows_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 127, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            127, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 127 without LONG_ADDR -- short train allowed
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 7: Boundary -- address exactly 128, long train, allowed

```cpp
TEST(TrainSearch, handler_disambig_boundary_128_allows_long_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 128, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            128, TRAIN_SEARCH_FLAG_DCC);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Address 128 without LONG_ADDR -- long train allowed
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 8: Allocate flag bypasses disambiguation for high address + short train

```cpp
TEST(TrainSearch, handler_disambig_allocate_bypasses_high_search_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false); // short-address

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for 200, DCC, ALLOCATE flag set -- should bypass disambiguation
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            200, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // ALLOCATE bypasses -- match allowed
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 9: Allocate flag bypasses disambiguation for low address + long train

```cpp
TEST(TrainSearch, handler_disambig_allocate_bypasses_short_search_long_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, true); // long-address

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for 42, DCC, ALLOCATE flag set -- should bypass disambiguation
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_ALLOCATE);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // ALLOCATE bypasses -- match allowed
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 10: Explicit LONG_ADDR flag still works -- long train matched

```cpp
TEST(TrainSearch, handler_disambig_explicit_long_flag_matches_long_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 5000, true);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            5000, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Explicit LONG_ADDR + long train -- match
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

#### Test 11: Explicit LONG_ADDR flag rejects short train

```cpp
TEST(TrainSearch, handler_disambig_explicit_long_flag_rejects_short_train)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 42, false);

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(
            42, TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // Explicit LONG_ADDR + short train -- reject
    EXPECT_FALSE(sm.outgoing_msg_info.valid);

}
```

#### Test 12: Non-DCC search (flags = 0x00) ignores disambiguation entirely

```cpp
TEST(TrainSearch, handler_disambig_non_dcc_search_ignores_address_type)
{

    _global_initialize();

    openlcb_node_t *node = _create_train_node();
    OpenLcbApplicationTrain_set_dcc_address(node, 200, false); // short-address at high addr

    openlcb_msg_t *incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing = OpenLcbBufferStore_allocate_buffer(BASIC);

    openlcb_statemachine_info_t sm;
    _setup_statemachine(&sm, node, incoming, outgoing);

    // Search for 200 with protocol=any (no DCC flag)
    event_id_t search_event = OpenLcbUtilities_create_train_search_event_id(200, 0x00);

    ProtocolTrainSearch_handle_search_event(&sm, search_event);

    // No DCC flag -- disambiguation not applied, address match only
    EXPECT_TRUE(sm.outgoing_msg_info.valid);

}
```

### 4.3 Test Verification

Before the fix, Test 4 (`handler_disambig_high_search_rejects_short_train`) and
Test 5 (`handler_disambig_boundary_128_rejects_short_train`) will **fail**, confirming
the bug exists. After applying the fix, all 12 tests should pass.

Build and run:
```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

### Risk Level: LOW

**Behavioral change scope:** Only affects DCC train search matching when:
- The DCC flag is set
- The LONG_ADDR flag is NOT set
- The search address is >= 128
- The train has `is_long_address == false`

This is a narrow, well-defined correction. No existing tests fail (because no existing
tests cover this case). The only affected scenario is one that the standard says should
not match.

**Regression risk:** The fix adds a new rejection path that could theoretically prevent a
previously-matching train from responding. However, this is exactly the intended behavior
-- a short-address DCC train at address >= 128 is an unusual configuration (short DCC
addresses are 1-127), and the standard says such a search should not match it.

**Allocate path:** The Allocate flag bypass is preserved, so dynamic train node creation
is not affected.

**Non-DCC path:** The entire change is inside the `if (flags & TRAIN_SEARCH_FLAG_DCC)`
block, so non-DCC searches are completely unaffected.

## 6. Estimated Effort

| Task | Time |
|------|------|
| Modify `_does_train_match()` in source file | 5 minutes |
| Write 12 test cases | 30 minutes |
| Build and verify all tests pass | 5 minutes |
| **Total** | **~40 minutes** |
