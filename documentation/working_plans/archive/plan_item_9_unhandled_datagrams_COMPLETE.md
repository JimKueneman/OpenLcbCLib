<!--
  ============================================================
  STATUS: IMPLEMENTED
  The `load_datagram_rejected` callback is wired in `openlcb_main_statemachine.c`; the MTI_DATAGRAM else-branch calls it with `ERROR_PERMANENT_NOT_IMPLEMENTED` when no datagram handler is registered.
  ============================================================
-->

# Plan: Item 9 -- Unhandled Datagrams Silently Dropped (No OIR/Reject)

## 1. Summary

When `_interface->datagram` is NULL (datagrams not compiled in), incoming datagrams at
`openlcb_main_statemachine.c` lines 685-693 are silently ignored. The standard requires a
Datagram Rejected reply so the sending node does not time out waiting for a response it will
never receive.

The fix adds an `else` branch that sends a Datagram Rejected reply via a new
`load_datagram_rejected` interface callback, following the same dependency-injection pattern
used throughout the library.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine.h` | Add `load_datagram_rejected` callback to interface struct |
| `src/openlcb/openlcb_main_statemachine.c` | Add `else` branch to `MTI_DATAGRAM` case calling `load_datagram_rejected` |
| `src/openlcb/openlcb_config.c` | Wire `load_datagram_rejected` to the datagram handler's reject builder |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine_Test.cxx` | Add test for datagram rejection when handler is NULL; update mock interface for new field |

## 3. Implementation Steps

### Step 1. Add callback to interface struct

In `openlcb_main_statemachine.h`, add to the interface struct in the Datagram section:

```c
        /** @brief Build Datagram Rejected reply.  Optional — called when datagram handler is NULL. */
    void (*load_datagram_rejected)(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code);
```

### Step 2. Add else branch in the main statemachine

In `openlcb_main_statemachine.c`, change the `MTI_DATAGRAM` case from:

```c
        case MTI_DATAGRAM:

            if (_interface->datagram) {

                _interface->datagram(statemachine_info);

            }

            break;
```

to:

```c
        case MTI_DATAGRAM:

            if (_interface->datagram) {

                _interface->datagram(statemachine_info);

            } else {

                if (_interface->load_datagram_rejected) {

                    _interface->load_datagram_rejected(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED);

                }

            }

            break;
```

### Step 3. Wire in openlcb_config.c

In `_build_main_statemachine()`, wire the callback. The implementation function needs to
build a Datagram Rejected reply (MTI 0x0A48) with the given error code. Either:

- Reuse `ProtocolDatagramHandler_load_datagram_rejected_message()` if its signature matches
  or can be adapted, or
- Provide a small wrapper function that matches the interface signature.

Check the existing reject function signature:

```c
    _main_sm.load_datagram_rejected = &ProtocolDatagramHandler_load_datagram_rejected_message;
```

If the existing function signature is `(openlcb_statemachine_info_t *, uint16_t)`, it wires
directly. If not, a thin wrapper is needed.

**Note:** If datagram support is not compiled in (`#ifndef OPENLCB_COMPILE_DATAGRAMS`), the
reject function won't be available. In that case, provide a standalone implementation in
`openlcb_config.c` or conditionally compile a minimal reject builder that does not depend on
the full datagram handler.

## 4. Test Changes

### Test 1: Datagram rejected when handler is NULL

```cpp
// ============================================================================
// TEST: Incoming datagram rejected when datagram handler is NULL
// @details Regression for TODO Item 9: when _interface->datagram is NULL,
//          incoming datagrams must receive a Datagram Rejected reply.
// @coverage OpenLcbMainStatemachine MTI_DATAGRAM case
// ============================================================================

TEST(OpenLcbMainStatemachine, datagram_rejected_when_handler_null)
{
    // Setup: interface with datagram = NULL, load_datagram_rejected = mock
    // Send an incoming MTI_DATAGRAM
    // Verify outgoing message is MTI_DATAGRAM_REJECTED_REPLY
    // Verify error code is ERROR_PERMANENT_NOT_IMPLEMENTED
    // Verify dest fields point back to the sender
}
```

### Test 2: Datagram dispatched normally when handler exists

```cpp
TEST(OpenLcbMainStatemachine, datagram_dispatched_when_handler_exists)
{
    // Existing behavior confirmation: handler is not NULL
    // Send datagram, verify handler was called
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Datagram handler not compiled in | NONE | This is exactly the scenario we're fixing. The rejection tells the sender their datagram is not supported. |
| MTI_DATAGRAM_REJECTED_REPLY undefined | VERY LOW | Already defined in `openlcb_defines.h` at line 319. |
| Outgoing buffer availability | LOW | The statemachine always has an outgoing buffer allocated. |
| Interface struct size change | LOW | Adding one new pointer. All existing call sites use memset+assign, so new field defaults to NULL. When NULL, no reject is sent — same as current behavior. |
| Reject function unavailable without OPENLCB_COMPILE_DATAGRAMS | LOW | Provide a standalone minimal reject builder that does not depend on the full datagram handler. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Add callback to interface struct | 5 min |
| Add else branch in statemachine | 5 min |
| Wire callback in openlcb_config.c | 5 min |
| Add 2 tests | 15 min |
| Update test mock interface for new field | 5 min |
| Build and run full test suite | 5 min |
| **Total** | **~40 min** |
