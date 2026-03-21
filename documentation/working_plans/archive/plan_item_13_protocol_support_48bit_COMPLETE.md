<!--
  ============================================================
  STATUS: IMPLEMENTED
  `protocol_message_network.c` encodes all six bytes of `support_flags` using shifts 40, 32, 24, 16, 8, and 0, matching Option B from the plan.
  ============================================================
-->

# Plan: Item 13 -- Protocol Support Reply Only Encodes 24 of 48 Bits

## 1. Summary

Replace the three hardcoded `0x00` bytes (payload positions 3-5) in the Protocol Support
Inquiry reply with shifts from the full `uint64_t support_flags`, so all 48 bits defined by
MessageNetworkS Section 3.4 are encoded on the wire.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network.c` (lines 170-175) | Replace hardcoded bytes 3-5 with shifts from `support_flags` bits 24-47 |

### Test file

| File | Change |
|------|--------|
| `src/openlcb/protocol_message_network_Test.cxx` | Update expectations for bytes 3-5 in existing tests; add new 48-bit test |

### No changes required

| File | Reason |
|------|--------|
| `src/openlcb/openlcb_types.h` | `protocol_support` is already `uint64_t` (line 537) -- no change needed |
| `src/openlcb/openlcb_defines.h` | All `PSI_*` defines are plain integer literals -- they already fit in `uint64_t`. No new defines needed for this fix. |
| `tools/node_builder/js/codegen.js` | Generates `.protocol_support = (PSI_X | PSI_Y | ...)` which is already `uint64_t`-compatible. No change needed. |

## 3. Implementation Steps

### Step 1: Fix the source encoding (canonical file)

**File:** `src/openlcb/protocol_message_network.c`

**Current code (lines 170-175):**

```c
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 16) & 0xFF, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 8) & 0xFF, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) (support_flags >> 0) & 0xFF, 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0x00, 5);
```

**Replacement code:**

```c
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 40) & 0xFF), 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 32) & 0xFF), 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 24) & 0xFF), 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 16) & 0xFF), 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 8) & 0xFF), 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 0) & 0xFF), 5);
```

**IMPORTANT -- Bit layout discussion:**

The OpenLCB MessageNetworkS standard defines the Protocol Support Reply as a 48-bit
big-endian field in bytes 0-5 of the payload. The existing `PSI_*` defines use values
like `0x800000` (PSI_SIMPLE) through `0x000010` (PSI_FIRMWARE_UPGRADE_ACTIVE), which
occupy bits 4-23 (the low 24 bits of the `uint64_t`).

There are **two valid interpretations** of the bit-to-byte mapping:

**Option A -- Shift existing defines up by 24 bits (align to MSB of 48-bit field):**
This would place the current defines at bits 28-47 of the `uint64_t`, requiring
all `PSI_*` values in `openlcb_defines.h` to be shifted left by 24 bits (e.g.,
`PSI_SIMPLE` from `0x800000` to `0x800000000000`). The code above (shifts 40, 32,
24, 16, 8, 0) would then be correct. Every `openlcb_user_config.h` and application
copy would also need updating.

**Option B -- Keep existing defines, shift encoding to match (align to LSB of 48-bit field):**
Keep the current defines as-is. The existing encoding already puts the low 24 bits
into bytes 0-2. This fix simply extends it to also encode bits 24-47 into bytes 0-2
while the current bits 0-23 remain in bytes 3-5. But this reverses the natural
big-endian byte order relative to the bit numbering.

**Recommended approach -- Option B (minimal change, backward compatible):**

Since every existing `PSI_*` define, every `openlcb_user_config.h` (including the
Node Builder code generator), and every application already uses the current bit
positions (0-23), the lowest-risk fix is to keep the existing byte-0/1/2 encoding
unchanged and simply extract bits 24-47 into bytes 0-2 while moving the existing
extraction up:

```c
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 16) & 0xFF), 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 8) & 0xFF), 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 0) & 0xFF), 2);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 40) & 0xFF), 3);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 32) & 0xFF), 4);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, (uint8_t) ((support_flags >> 24) & 0xFF), 5);
```

This keeps bytes 0-2 identical to today (all existing PSI flags land in the same
wire positions), and bytes 3-5 now correctly encode any future flags defined in bits
24-47 of the `uint64_t`. No existing define, user config, or generated code changes.

**HOWEVER** -- before implementing, the OpenLCB standard document (MessageNetworkS
Section 3.4) must be consulted to confirm whether the bit numbering places "bit 0"
at the MSB or LSB of the 48-bit on-wire field. The correct byte mapping depends on
this. If the standard numbers bit 0 as the MSB of byte 0 (which is common in
OpenLCB), then the current defines may already be positioned for bytes 0-2 and
Option B is correct. If bit 0 is the LSB of byte 5, then Option A is needed.

**Decision required:** Verify the standard's bit-to-byte mapping before coding.

### Step 2: Update test expectations

See Section 4 below.

### Step 3: Build and run tests

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

Run the test binary for protocol_message_network and verify all tests pass.

## 4. Test Changes

### 4.1 Existing tests that need updating

All three PSI tests currently assert `payload_ptr[3] == 0x00`, `[4] == 0x00`,
`[5] == 0x00`. These must change to verify the shifted bits.

#### Test: `handle_protocol_support_inquiry_full` (lines 326-332)

**Current assertions (bytes 3-5):**
```cpp
EXPECT_EQ(payload_ptr[3], 0x00);
EXPECT_EQ(payload_ptr[4], 0x00);
EXPECT_EQ(payload_ptr[5], 0x00);
```

**Updated assertions (Option B mapping):**
```cpp
EXPECT_EQ(payload_ptr[3], (uint8_t)((support_flags >> 40) & 0xFF));
EXPECT_EQ(payload_ptr[4], (uint8_t)((support_flags >> 32) & 0xFF));
EXPECT_EQ(payload_ptr[5], (uint8_t)((support_flags >> 24) & 0xFF));
```

Since all current `PSI_*` flags fit in bits 0-23, and the test's `_node_parameters_main_node`
only sets flags in that range, bytes 3-5 will still be `0x00` in this test.
The assertions change form but not expected value -- this is intentional to make the
test future-proof.

#### Test: `handle_protocol_support_inquiry_firmware_upgrade_active` (lines 424-430)

Same pattern -- replace hardcoded `0x00` with shift expressions from `expected_flags`.
Values will still be `0x00` since all flags are in bits 0-23.

#### Test: `protocol_support_all_flags` (lines 1098-1103)

**Current assertions:**
```cpp
EXPECT_EQ(payload_ptr[0], 0xFF);
EXPECT_EQ(payload_ptr[1], 0xFF);
EXPECT_EQ(payload_ptr[2], 0xFF);
```
(bytes 3-5 not explicitly checked but implied 0x00 by the hardcoded encoding)

**Updated assertions:**
```cpp
EXPECT_EQ(payload_ptr[0], 0xFF);
EXPECT_EQ(payload_ptr[1], 0xFF);
EXPECT_EQ(payload_ptr[2], 0xFF);
EXPECT_EQ(payload_ptr[3], 0xFF);
EXPECT_EQ(payload_ptr[4], 0xFF);
EXPECT_EQ(payload_ptr[5], 0xFF);
```

Since `params_all_flags.protocol_support = 0xFFFFFFFFFFFFFFFF`, all 6 bytes must
now be `0xFF`.

#### Test: `protocol_support_no_flags` (lines 1144-1148)

Add explicit checks for bytes 3-5:
```cpp
EXPECT_EQ(payload_ptr[3], 0x00);
EXPECT_EQ(payload_ptr[4], 0x00);
EXPECT_EQ(payload_ptr[5], 0x00);
```

### 4.2 New test: flags in upper 24 bits

Add a new test that sets bits only in the upper 24 bits (bits 24-47) and verifies
they appear correctly in bytes 3-5 while bytes 0-2 remain `0x00`.

```cpp
TEST(ProtocolMessageNetwork, protocol_support_upper_24_bits)
{
    _reset_variables();
    _global_initialize();

    node_parameters_t params_upper = _node_parameters_main_node;
    params_upper.protocol_support = 0x0000AABBCC000000;  // bits 24-47 only

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_upper);
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

    OpenLcbUtilities_load_openlcb_message(
            openlcb_msg, SOURCE_ALIAS, SOURCE_ID,
            DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);

    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);

    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;

    // Bytes 0-2: bits 16-23, 8-15, 0-7 -- all zero (no lower flags set)
    EXPECT_EQ(payload_ptr[0], 0x00);
    EXPECT_EQ(payload_ptr[1], 0x00);
    EXPECT_EQ(payload_ptr[2], 0x00);

    // Bytes 3-5: bits 40-47, 32-39, 24-31 -- should contain AA, BB, CC
    EXPECT_EQ(payload_ptr[3], 0xAA);
    EXPECT_EQ(payload_ptr[4], 0xBB);
    EXPECT_EQ(payload_ptr[5], 0xCC);
}
```

### 4.3 New test: mixed lower and upper bits

Add a test with flags in both the lower and upper 24-bit halves:

```cpp
TEST(ProtocolMessageNetwork, protocol_support_mixed_48_bits)
{
    _reset_variables();
    _global_initialize();

    node_parameters_t params_mixed = _node_parameters_main_node;
    params_mixed.protocol_support = 0x0000112233445566;  // spans all 6 bytes

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &params_mixed);
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

    OpenLcbUtilities_load_openlcb_message(
            openlcb_msg, SOURCE_ALIAS, SOURCE_ID,
            DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);

    ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(outgoing_msg->payload_count, 6);

    uint8_t *payload_ptr = (uint8_t *)outgoing_msg->payload;

    // Bytes 0-2: lower 24 bits (bits 16-23=0x44, 8-15=0x55, 0-7=0x66)
    EXPECT_EQ(payload_ptr[0], 0x44);
    EXPECT_EQ(payload_ptr[1], 0x55);
    EXPECT_EQ(payload_ptr[2], 0x66);

    // Bytes 3-5: upper 24 bits (bits 40-47=0x11, 32-39=0x22, 24-31=0x33)
    EXPECT_EQ(payload_ptr[3], 0x11);
    EXPECT_EQ(payload_ptr[4], 0x22);
    EXPECT_EQ(payload_ptr[5], 0x33);
}
```

## 5. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Bit-to-byte mapping wrong** -- standard may number bits MSB-first, making the current encoding already wrong for all flags | Medium | HIGH | Read MessageNetworkS Section 3.4 carefully before coding. Compare with other OpenLCB implementations (e.g., OpenMRN). |
| **Operator precedence bug** -- `(uint8_t) (support_flags >> 40) & 0xFF` without inner parens casts before masking | Low | Medium | Use explicit parentheses: `(uint8_t) ((support_flags >> 40) & 0xFF)`. Note the existing code on lines 170-172 has this same subtle issue (`(uint8_t) (support_flags >> 16) & 0xFF` casts first, then masks); fix all 6 lines with proper parens. |
| **No behavioral change for current nodes** -- since all current PSI flags fit in bits 0-23, bytes 3-5 will still be 0x00 for all existing deployments | None | None | This is expected. The fix is forward-looking to prevent future silent data loss. |
| **Firmware upgrade active bit manipulation** -- the `~((uint64_t) PSI_FIRMWARE_UPGRADE)` mask already handles the full 64 bits correctly | None | None | No change needed to the firmware upgrade logic. |

### Operator Precedence Note

The existing lines 170-172 have a subtle precedence issue:

```c
(uint8_t) (support_flags >> 16) & 0xFF
```

This is parsed as `((uint8_t)(support_flags >> 16)) & 0xFF`. The cast to `uint8_t`
already truncates to 8 bits, so the `& 0xFF` is redundant and the result happens
to be correct. However, for clarity and to prevent future confusion, all 6 lines
should use explicit parentheses:

```c
(uint8_t) ((support_flags >> 16) & 0xFF)
```

## 6. Estimated Effort

| Task | Time |
|------|------|
| Verify standard's bit-to-byte mapping | 15 minutes |
| Edit `protocol_message_network.c` (canonical) | 5 minutes |
| Update 4 existing tests + add 2 new tests | 30 minutes |
| Build and run test suite | 5 minutes |
| Fix any test failures | 10 minutes |
| **Total** | **~65 minutes** |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.
