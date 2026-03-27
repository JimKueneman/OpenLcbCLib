# dsPIC33 Bootloader TODO

The dsPIC implementation is the reference pattern. All three swimlanes are fully implemented
and working. The items below are shared issues that also apply to all other platforms.

## MEDIUM

### `NO_CHECKSUM` is globally enabled

**File:** Shared `bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot and
after firmware write. Any corrupted image will be jumped to. Acceptable for development, must
be addressed before production.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader.X/application_drivers/bootloader_drivers_openlcb.c`

Returns a fixed node ID for all boards. Two dsPIC boards on the same CAN bus will have
identical node IDs. TODO comment already present about reading from protected flash.

### `finalize_flash()` checksum validation is stubbed

Returns 0 unconditionally without validating the written image. Gated by `NO_CHECKSUM`.
Once the post-link checksum tool is integrated, implement the validation logic here.
