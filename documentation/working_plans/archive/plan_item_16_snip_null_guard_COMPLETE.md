<!--
  ============================================================
  STATUS: IMPLEMENTED
  Both `ProtocolSnip_load_user_name()` and `ProtocolSnip_load_user_description()` guard the `config_memory_read` callback with a NULL check, falling back to an empty string as planned.
  ============================================================
-->

# Plan: Item 16 -- SNIP No NULL Guard on config_memory_read Callback

## 1. Summary

Two call sites in `protocol_snip.c` invoke `_interface->config_memory_read()` without
first checking whether the function pointer is NULL.  If `ProtocolSnip_initialize()` was
called with an interface struct whose `config_memory_read` member is NULL, calls to
`ProtocolSnip_load_user_name()` or `ProtocolSnip_load_user_description()` will
dereference a NULL function pointer and crash.

The fix adds a NULL guard before each call, following the exact pattern already
established in `openlcb_application.c` lines 514-525.  When the callback is NULL the
functions emit an empty string (a single null terminator) into the SNIP payload so the
reply remains structurally valid.

The two existing EXPECT_DEATH tests are converted to assert graceful behavior instead of
documenting a crash.

**Severity:** LOW
**TODO Reference:** `documentation/todo.md` Item 16


## 2. Files to Modify

### Source (canonical)

| File | Lines | Change |
|------|-------|--------|
| `src/openlcb/protocol_snip.c` | 240, 276 | Add NULL guard around each `_interface->config_memory_read()` call |

### Test

| File | Lines | Change |
|------|-------|--------|
| `src/openlcb/protocol_snip_Test.cxx` | 718-721, 742-745 | Replace EXPECT_DEATH with assertions for graceful empty-string behavior |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.


## 3. Implementation Steps

### Step 1: Add NULL guard in ProtocolSnip_load_user_name()

**Current code (line 240):**
```c
_interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer);

_process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_NAME_BUFFER, requested_bytes);
```

**Replacement:**
```c
if (_interface->config_memory_read) {

    _interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer);

    _process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_NAME_BUFFER, requested_bytes);

} else {

    _process_snip_string(outgoing_msg, &offset, "", LEN_SNIP_USER_NAME_BUFFER, requested_bytes);

}
```

**Rationale:**
- The `if (_interface->callback)` pattern matches `openlcb_application.c` lines 514 and 549.
- When the callback is NULL the function must still contribute a null-terminated empty
  string to the SNIP payload so the reply contains the required 6 null separators and
  passes `ProtocolSnip_validate_snip_reply()`.
- Passing `""` (empty string) to `_process_snip_string` with the same `max_str_len` and
  `requested_bytes` produces a single 0x00 byte in the payload, which is a valid
  zero-length SNIP user name.

### Step 2: Add NULL guard in ProtocolSnip_load_user_description()

**Current code (line 276):**
```c
_interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer); // grab string from config memory

_process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_DESCRIPTION_BUFFER, requested_bytes);
```

**Replacement:**
```c
if (_interface->config_memory_read) {

    _interface->config_memory_read(openlcb_node, data_address, requested_bytes, &configuration_memory_buffer); // grab string from config memory

    _process_snip_string(outgoing_msg, &offset, (char*) (&configuration_memory_buffer[0]), LEN_SNIP_USER_DESCRIPTION_BUFFER, requested_bytes);

} else {

    _process_snip_string(outgoing_msg, &offset, "", LEN_SNIP_USER_DESCRIPTION_BUFFER, requested_bytes);

}
```

Same rationale as Step 1.

### Step 3: Update tests

**Test: `load_user_name_null_callback` (current lines 703-722)**

Replace the EXPECT_DEATH block with assertions that the function returns gracefully and
produces a single null byte (empty user name):

```cpp
TEST(ProtocolSnip, load_user_name_null_callback)
{
    _reset_variables();
    _global_initialize_null_callbacks();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(msg, nullptr);

    // NULL callback should produce an empty string (single null terminator), not crash
    uint16_t offset = ProtocolSnip_load_user_name(node1, msg, 0, LEN_SNIP_USER_NAME_BUFFER - 1);

    // Empty string = one null byte written
    EXPECT_EQ(offset, 1);
    EXPECT_EQ(msg->payload_count, 1);
    EXPECT_EQ(*msg->payload[0], 0x00);
}
```

**Test: `load_user_description_null_callback` (current lines 730-746)**

Same pattern:

```cpp
TEST(ProtocolSnip, load_user_description_null_callback)
{
    _reset_variables();
    _global_initialize_null_callbacks();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(msg, nullptr);

    // NULL callback should produce an empty string (single null terminator), not crash
    uint16_t offset = ProtocolSnip_load_user_description(node1, msg, 0, LEN_SNIP_USER_DESCRIPTION_BUFFER - 1);

    // Empty string = one null byte written
    EXPECT_EQ(offset, 1);
    EXPECT_EQ(msg->payload_count, 1);
    EXPECT_EQ(*msg->payload[0], 0x00);
}
```

### Step 4: Build and run tests

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

Verify:
- All tests pass (no EXPECT_DEATH tests remain for these two cases).
- The two renamed tests (`load_user_name_null_callback`, `load_user_description_null_callback`)
  pass with the new graceful-behavior assertions.
- No regressions in the rest of the ProtocolSnip test suite.


## 4. Test Changes Summary

| Test Name | Before | After |
|-----------|--------|-------|
| `load_user_name_null_callback` | EXPECT_DEATH (documents crash) | Asserts graceful return: offset=1, payload_count=1, payload[0]=0x00 |
| `load_user_description_null_callback` | EXPECT_DEATH (documents crash) | Asserts graceful return: offset=1, payload_count=1, payload[0]=0x00 |

No new test cases are needed.  The existing two tests fully cover the NULL-callback path
for both functions once converted from crash-documenting to behavior-asserting.


## 5. Risk Assessment

**Risk: Very Low**

- The change adds a conditional branch that only fires when `config_memory_read` is NULL.
  Normal operation (non-NULL callback) follows the exact same code path as before.
- The empty-string fallback uses `_process_snip_string`, which is already well-tested by
  the existing suite.  No new helper code is introduced.
- The pattern is already proven in `openlcb_application.c` for the same callback type.
- The SNIP reply remains structurally valid (6 null separators preserved) because
  `_process_snip_string("")` emits exactly one 0x00 byte, matching the null terminator
  that a real empty user name/description would produce.
- No changes to public API signatures or interface structs.

**Potential concern:** If a future caller relies on detecting whether config memory was
actually read (e.g., by checking the return value of `config_memory_read`), this change
silently substitutes an empty string.  However, the current function signatures return
only the payload offset (not a status code), so there is no mechanism for the caller to
distinguish "empty because NULL callback" from "empty because config memory contained an
empty string."  This is acceptable for a LOW-severity defensive guard.


## 6. Estimated Effort

| Task | Time |
|------|------|
| Edit canonical `protocol_snip.c` (2 guards) | 5 min |
| Update 2 test cases in `protocol_snip_Test.cxx` | 5 min |
| Build and run tests, verify | 5 min |
| **Total** | **~15 min** |
