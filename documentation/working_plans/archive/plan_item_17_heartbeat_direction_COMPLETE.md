<!--
  ============================================================
  STATUS: IMPLEMENTED
  `openlcb_application_train.c` preserves direction via `OpenLcbFloat16_get_direction()` and emits either `FLOAT16_POSITIVE_ZERO` or `FLOAT16_NEGATIVE_ZERO` on heartbeat timeout, matching the plan.
  ============================================================
-->

# Plan: Item 17 -- Heartbeat Timeout Zeroes set_speed Without Preserving Direction

## 1. Summary

When the heartbeat timer expires in `OpenLcbApplicationTrain_100ms_timer_tick()`, the code
sets `state->set_speed = 0`, which always produces forward-stopped (`0x0000`). If the train
was moving in reverse, the direction bit is silently discarded. This violates TrainControl
Standard Section 4.4, which requires the node to stop the train while retaining its last
commanded direction.

The fix mirrors the existing correct pattern in `_handle_emergency_stop()` inside
`protocol_train_handler.c`, which reads the direction bit before zeroing the speed magnitude.

## 2. Files to Modify

| File | Change |
|------|--------|
| `src/openlcb/openlcb_application_train.c` | Add `#include "openlcb_float16.h"`, replace bare `state->set_speed = 0` with direction-preserving logic |
| `src/openlcb/openlcb_application_train_Test.cxx` | Add `#include "openlcb_float16.h"`, update existing forward test to use `FLOAT16_POSITIVE_ZERO`, add new reverse-direction test |

No other files require changes. The `openlcb_float16.c` object is already compiled into the
`openlcb` library via `src/openlcb/CMakeLists.txt` (line 26), so no build system changes are
needed.

## 3. Implementation Steps

### 3a. Add include to `openlcb_application_train.c`

Add `#include "openlcb_float16.h"` to the include block. Insert it in alphabetical order
among the project includes.

**Current** (lines 42-45):
```c
#include "openlcb_application.h"
#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
```

**After**:
```c
#include "openlcb_application.h"
#include "openlcb_defines.h"
#include "openlcb_float16.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
```

### 3b. Replace the heartbeat timeout speed assignment

**Current** (lines 241-244):
```c
        if (state->heartbeat_counter_100ms == 0) {

            state->estop_active = true;
            state->set_speed = 0;
```

**After**:
```c
        if (state->heartbeat_counter_100ms == 0) {

            state->estop_active = true;

            // Preserve direction, set speed magnitude to zero
            bool reverse = OpenLcbFloat16_get_direction(state->set_speed);
            state->set_speed = reverse ? FLOAT16_NEGATIVE_ZERO : FLOAT16_POSITIVE_ZERO;
```

This is identical to the pattern used in `_handle_emergency_stop()` at
`protocol_train_handler.c` lines 412-414.

### 3c. Update the Doxygen algorithm comment

The comment block above `OpenLcbApplicationTrain_100ms_timer_tick()` at line 213 currently
reads:

```
 *    - At zero, set estop_active = true, set_speed = 0, and fire on_heartbeat_timeout.
```

Change to:

```
 *    - At zero, set estop_active = true, zero set_speed preserving direction, and fire on_heartbeat_timeout.
```

## 4. Test Changes

### 4a. Add include to `openlcb_application_train_Test.cxx`

Add `#include "openlcb_float16.h"` after the existing `#include "protocol_train_handler.h"`
line (line 53). This provides `FLOAT16_POSITIVE_ZERO`, `FLOAT16_NEGATIVE_ZERO`, and
`OpenLcbFloat16_get_direction()` to the test file.

**After line 53**:
```cpp
#include "openlcb_float16.h"
```

### 4b. Update existing `heartbeat_timer_countdown` test

The existing test at line 593 checks:
```cpp
    EXPECT_EQ(state->set_speed, 0);
```

The test sets up speed implicitly as zero (forward) because `train_state_t` is zero-initialized
by the pool allocator. The behavior is unchanged but the assertion should be updated to use
the named constant for clarity and consistency with the new reverse test:

```cpp
    EXPECT_EQ(state->set_speed, FLOAT16_POSITIVE_ZERO);
```

### 4c. Add new test: `heartbeat_timer_countdown_reverse`

Insert immediately after `heartbeat_timer_countdown` (after line 595) and before
`heartbeat_disabled` (line 597). This test verifies that a train moving in reverse retains
its direction after heartbeat timeout.

```cpp
TEST(ApplicationTrain, heartbeat_timer_countdown_reverse)
{

    _reset_tracking();
    _global_initialize();

    openlcb_node_t *node = OpenLcbNode_allocate(TEST_DEST_ID, &_test_node_parameters);
    node->train_state = NULL;
    train_state_t *state = OpenLcbApplicationTrain_setup(node);

    EXPECT_NE(state, nullptr);

    // Set heartbeat: 3 seconds = 30 ticks of 100ms
    state->heartbeat_timeout_s = 3;
    state->heartbeat_counter_100ms = 30;

    // Simulate reverse movement: set_speed has the sign bit set
    state->set_speed = FLOAT16_NEGATIVE_ZERO | 0x3C00;  // Reverse, ~1.0 mph

    // Tick 30 times -- counter reaches 0, estop fires
    for (int i = 0; i < 30; i++) {

        OpenLcbApplicationTrain_100ms_timer_tick();

    }

    EXPECT_EQ(state->heartbeat_counter_100ms, (uint32_t) 0);
    EXPECT_TRUE(mock_heartbeat_timeout_called);
    EXPECT_EQ(mock_heartbeat_timeout_node, node);
    EXPECT_TRUE(state->estop_active);

    // Direction must be preserved: reverse, stopped
    EXPECT_EQ(state->set_speed, FLOAT16_NEGATIVE_ZERO);

}
```

### 4d. Test matrix summary

| Test | Initial direction | Expected set_speed after timeout |
|------|------------------|----------------------------------|
| `heartbeat_timer_countdown` | Forward (default) | `FLOAT16_POSITIVE_ZERO` (0x0000) |
| `heartbeat_timer_countdown_reverse` | Reverse | `FLOAT16_NEGATIVE_ZERO` (0x8000) |

## 5. Risk Assessment

**Risk: LOW**

- The change is a two-line replacement in one function, using the same pattern already
  validated in `_handle_emergency_stop()`.
- `OpenLcbFloat16_get_direction()` is a simple bit-mask check (`(half & 0x8000) != 0`),
  already thoroughly tested in `openlcb_float16_Test.cxx`.
- The added `#include "openlcb_float16.h"` has no side effects. The header is already used
  throughout the codebase and is guarded.
- No changes to public API signatures, struct layouts, or build configuration.
- The existing forward-direction test still passes because `FLOAT16_POSITIVE_ZERO == 0x0000 == 0`.
- No platform-specific concerns -- the fix is pure C99 with no hardware dependencies.

## 6. Estimated Effort

- Implementation: ~5 minutes (add include, replace one line, update comment)
- Test update: ~10 minutes (update assertion, write new reverse test)
- Build and verify: ~5 minutes (`cd test && rm -rf build && mkdir build && cd build && cmake .. && make`)
- **Total: ~20 minutes**
