<!--
  ============================================================
  STATUS: IMPLEMENTED
  The else-branch in `protocol_snip.c` correctly appends a null terminator when string content exceeds requested bytes, using `(byte_count - 1)` for content and reserving one byte for `0x00`.
  ============================================================
-->

# Plan: Fix TODO Item 15 -- SNIP _process_snip_string Can Omit Null Terminator

**Standard:** SimpleNodeInformationS Section 5.3
**Severity:** LOW
**Date:** 2 March 2026

---

## 1. Summary

The static helper `_process_snip_string()` in `protocol_snip.c` has an `else` branch
(lines 115-121) that copies a partial string without appending a null terminator.
This violates the SNIP standard, which requires every string field to be
null-terminated. The bug is triggered when `string_length > byte_count` while
`string_length <= max_str_len - 1` -- that is, the string fits within its
maximum field size but the caller requests fewer bytes than the string is long.

**Example:** A 9-character string `"ShortName"` with `byte_count = 5` copies
`"Short"` into the payload with no trailing `0x00`.

In the normal full-reply path (`ProtocolSnip_handle_simple_node_info_request`),
`byte_count` is always passed as `LEN_SNIP_*_BUFFER - 1`, which equals the
field's maximum content length. Therefore `string_length <= byte_count` is
always true for any string that fits, and the bug is never reached. However,
the same `ProtocolSnip_load_*` functions are also called from
`protocol_config_mem_read_handler.c` for ACDI address-space reads, where
`byte_count` comes from the remote node's read request
(`config_mem_read_request_info->bytes`). A remote node could request fewer
bytes than the string length, triggering the buggy path and producing a
non-conformant response missing a null terminator.

The fix is straightforward: always append a null terminator in the `else`
branch, truncating the copied string content to `byte_count - 1` characters
to make room for it.

---

## 2. Files to Modify

| File | Change |
|------|--------|
| `src/openlcb/protocol_snip.c` (lines 91-123) | Fix `_process_snip_string()` else branch to always null-terminate |
| `src/openlcb/protocol_snip_Test.cxx` | Update two tests that document the buggy no-null behavior; add one new edge-case test |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

---

## 3. Implementation Steps

### Step 1: Fix the else branch in `_process_snip_string()`

**Current code** (lines 115-121):

```c
    } else {

        memcpy(&outgoing_msg->payload[*payload_offset], str, byte_count);
        *payload_offset = *payload_offset + byte_count;
        outgoing_msg->payload_count += byte_count;

    }
```

**Proposed replacement:**

```c
    } else {

        uint16_t copy_len = (byte_count > 0) ? byte_count - 1 : 0;
        memcpy(&outgoing_msg->payload[*payload_offset], str, copy_len);
        *payload_offset = *payload_offset + copy_len;
        *outgoing_msg->payload[*payload_offset] = 0x00;
        (*payload_offset)++;
        outgoing_msg->payload_count += (copy_len + 1);

    }
```

**Why this works:**

- The `else` branch executes when `string_length > byte_count` (the string is
  longer than the requested byte budget). We must fit both content AND a null
  terminator within `byte_count` bytes total.
- We copy `byte_count - 1` characters of the string, then append one `0x00`
  byte, for a total of `byte_count` bytes written. This matches the caller's
  budget exactly.
- The `byte_count > 0` guard handles the degenerate case where zero bytes are
  requested. In that case no bytes are written and no null is appended (the
  caller asked for nothing). Note: `byte_count == 0` with a non-empty string
  is not currently reachable via any call site, but defensive coding is
  warranted.
- The resulting payload now always contains a null terminator for every string
  field, satisfying the SNIP standard requirement and ensuring
  `ProtocolSnip_validate_snip_reply()` (which expects exactly 6 nulls) will
  pass for any well-formed SNIP reply.

### Step 2: Update the Doxygen comment for `_process_snip_string()`

Update the `@details` block (lines 74-82) to reflect the new behavior:

**Current** (line 80):
```
 * -# Otherwise copy only byte_count bytes (no terminator)
```

**Proposed:**
```
 * -# Otherwise truncate to byte_count - 1 chars + null terminator
```

---

## 4. Test Changes

### 4a: Update `process_string_exceeds_requested_bytes` (line 1077)

This test currently documents the buggy behavior. It calls
`ProtocolSnip_load_name()` with `byte_count = 5` on a 9-character string
and expects 5 bytes with NO null terminator.

**Current assertions (lines 1093-1103):**

```cpp
    EXPECT_EQ(offset, 5);  // Only 5 bytes copied, NO null
    EXPECT_EQ(msg->payload_count, 5);

    uint8_t *payload_ptr = (uint8_t *)msg->payload;
    // Should have "Short" (5 chars) with NO null terminator
    EXPECT_EQ(payload_ptr[0], 'S');
    EXPECT_EQ(payload_ptr[1], 'h');
    EXPECT_EQ(payload_ptr[2], 'o');
    EXPECT_EQ(payload_ptr[3], 'r');
    EXPECT_EQ(payload_ptr[4], 't');
    // payload_ptr[5] should NOT be 0x00 (not written)
```

**Proposed assertions:**

```cpp
    // After fix: 4 chars of string content + null terminator = 5 bytes total
    EXPECT_EQ(offset, 5);
    EXPECT_EQ(msg->payload_count, 5);

    uint8_t *payload_ptr = (uint8_t *)msg->payload;
    // Should have "Shor" (4 chars) + null terminator
    EXPECT_EQ(payload_ptr[0], 'S');
    EXPECT_EQ(payload_ptr[1], 'h');
    EXPECT_EQ(payload_ptr[2], 'o');
    EXPECT_EQ(payload_ptr[3], 'r');
    EXPECT_EQ(payload_ptr[4], 0x00);  // Null terminator
```

Also update the test name, comments, and section header to reflect the
corrected behavior:

```
// TEST: String Fits Max But Exceeds Requested - Truncated with Null
// @details Tests string <= max_str_len - 1 BUT string_length > byte_count
// @coverage _process_snip_string() - Path: fits max but not requested, truncated + null
```

### 4b: Update `process_string_minimal_byte_request` (line 1204)

This test calls `ProtocolSnip_load_name()` with `byte_count = 1` on a
9-character string and expects 1 byte (`'S'`) with no null.

**Current assertions (lines 1220-1224):**

```cpp
    EXPECT_EQ(offset, 1);  // Only 1 byte
    EXPECT_EQ(msg->payload_count, 1);

    uint8_t *payload_ptr = (uint8_t *)msg->payload;
    EXPECT_EQ(payload_ptr[0], 'S');  // First character only, no null
```

**Proposed assertions:**

```cpp
    // After fix: byte_count=1 means 0 chars + null terminator = 1 byte total
    EXPECT_EQ(offset, 1);
    EXPECT_EQ(msg->payload_count, 1);

    uint8_t *payload_ptr = (uint8_t *)msg->payload;
    EXPECT_EQ(payload_ptr[0], 0x00);  // Only a null terminator (empty string)
```

Also update the test name, comments, and section header:

```
// TEST: Requested Bytes = 1 with Multi-Char String - Just Null Terminator
// @details Tests minimal byte request with longer string (truncated to empty + null)
// @coverage _process_snip_string() - Path: byte_count = 1, string > 1, just null
```

### 4c: Add new test `process_string_exceeds_requested_boundary`

Add a new test that verifies the boundary condition where `byte_count` equals
`string_length` (the string just fits within the budget, including the null).
This confirms the existing `if (result_is_null_terminated)` path still works
correctly and that the boundary between the two branches is correct:

```cpp
// ============================================================================
// TEST: String Length Equals Byte Count - Null Terminated via Main Path
// @details Tests string_length == byte_count (boundary between branches)
// @coverage _process_snip_string() - Path: boundary, string_length <= byte_count
// ============================================================================

TEST(ProtocolSnip, process_string_length_equals_byte_count)
{
    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_short_name);
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(node1, nullptr);
    ASSERT_NE(msg, nullptr);

    // "ShortName" = 9 chars, max_str_len = 41, requesting exactly 9 bytes
    // string_length (9) <= byte_count (9) => takes the null-terminated path
    uint16_t offset = ProtocolSnip_load_name(node1, msg, 0, 9);

    EXPECT_EQ(offset, 10);  // 9 chars + null
    EXPECT_EQ(msg->payload_count, 10);

    uint8_t *payload_ptr = (uint8_t *)msg->payload;
    EXPECT_EQ(payload_ptr[9], 0x00);  // Null terminator
    EXPECT_STREQ((char *)payload_ptr, "ShortName");
}
```

### 4d: Update test summary comment block

Update the summary comment block near the end of the test file (around line
1256) to reflect that the "no null" paths are now fixed:

- Change `"String exceeds requested bytes (no null)"` to
  `"String exceeds requested bytes (truncated + null)"`
- Change `"Minimal byte request (1 byte, no null)"` to
  `"Minimal byte request (1 byte, just null)"`
- Change `"Length <= max BUT length > requested (no null)"` to
  `"Length <= max BUT length > requested (truncated + null)"`
- Change `"Minimal byte request (partial copy, no null)"` to
  `"Minimal byte request (just null terminator)"`

---

## 5. Risk Assessment

### Could this affect multi-frame SNIP assembly?

**No.** The multi-frame transport layer (`can_tx_message_handler.c`) operates
on the completed `outgoing_msg` payload after `_process_snip_string()` has
finished populating it. The transport layer fragments the payload by byte
offset and length; it does not interpret string content or null terminators.
The fix changes only the content of the bytes written and the byte counts,
not the message framing or fragmentation logic.

### Could this affect the ACDI config-memory read path?

**Yes, intentionally and positively.** The `ProtocolSnip_load_*` functions are
called from `protocol_config_mem_read_handler.c` for ACDI address space reads
(spaces 0xFC and 0xFB), where `byte_count` is set by the remote node's read
request (`config_mem_read_request_info->bytes`). This is the path most likely
to trigger the bug in practice -- a configuration tool reading an ACDI field
with a byte count smaller than the string length would receive a non-null-
terminated response. After the fix, all ACDI reads will produce properly
terminated strings.

### Could the total payload size change?

**No.** The fix preserves `byte_count` as the total number of bytes written
(content + null terminator). The caller's byte budget is respected exactly.
Before the fix, `byte_count` bytes of content were written with no null. After
the fix, `byte_count - 1` bytes of content plus one null byte are written.
The total is still `byte_count`. This means `payload_count` and
`payload_offset` will have the same values as before, so downstream code
(multi-frame assembly, buffer sizing) is unaffected.

### Does the fix break the full SNIP reply path?

**No.** In `ProtocolSnip_handle_simple_node_info_request()`, every string
loader is called with `byte_count = LEN_SNIP_*_BUFFER - 1`, which is the
maximum content length. For any string that fits within its buffer
(`string_length <= max_str_len - 1`), the condition `string_length <= byte_count`
is also satisfied, so the code takes the `if (result_is_null_terminated)` branch
-- the same branch it always took. The `else` branch is not reached in
full-reply usage, so the fix has zero impact on that path.

### Does the `byte_count == 0` guard matter?

In current usage, `byte_count == 0` should not be reachable for string fields
(the config-mem read handler requires `bytes > 0` for the request to reach the
SNIP loaders, and the full SNIP reply path always passes `>= 20`). The guard
is purely defensive. If somehow `byte_count == 0` were reached, the old code
would call `memcpy` with length 0 (harmless but writes no null), and the new
code would also write 0 bytes (no content, no null). Both produce an empty
contribution, which is acceptable for a zero-byte request.

---

## 6. Estimated Effort

| Task | Time |
|------|------|
| Fix `_process_snip_string()` in source | 10 min |
| Update Doxygen comment | 5 min |
| Update 2 existing tests, add 1 new test | 20 min |
| Update test summary comments | 5 min |
| Run test suite, verify all pass | 5 min |
| **Total** | **~45 min** |
