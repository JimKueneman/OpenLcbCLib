# dsPIC33 Bootloader TODO

The dsPIC implementation is the reference pattern. All three swimlanes are fully
implemented, tested on real hardware, and working end-to-end (PICkit program,
boot-time checksum validation, OTA firmware upload via OpenLCB Config Tool,
application drop-back via Freeze, and reboot into new firmware).

## DONE (kept for history)

### `NO_CHECKSUM` -- RESOLVED

`NO_CHECKSUM` in `bootloader_types.h` is now commented out (disabled).
Checksum validation is active at both boot time and after firmware write.
The post-link tool (`hex2bin.py --arch dspic`) patches the app header with
correct triple-CRC-16-IBM checksums including phantom byte correction.

### `finalize_flash()` checksum validation -- RESOLVED

The `#ifndef NO_CHECKSUM` branch reads the app header via Harvard-safe
`_read_flash_image_bytes()`, recomputes both pre and post triple-CRC
checksums, and returns `ERROR_PERMANENT` on mismatch. Active and verified
on hardware.

### BasicNode `app_header.c` and linker sections -- RESOLVED

- `BasicNode.X/app_header.c` -- 27-byte zero-init struct with `space(prog)`
- `BasicNode.X/p33EP512MC506_BasicNode.gld` -- `.app_reset` at `__CODE_BASE`,
  `.app_header` at PC 0x4004
- `BasicNode.X/reset_vector.s` -- `GOTO __reset` via linker-controlled section

All checksum fields are patched by hex2bin.py before upload.

### firmware_active not cleared on Unfreeze failure -- RESOLVED

**File:** Shared `bootloader_openlcb_statemachine.c`

The Unfreeze error path now clears `bootloader_state.firmware_active = 0`
before sending datagram_rejected, so the node does not stay stuck with the
Firmware Upgrade Active PIP bit set.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader.X/application_drivers/bootloader_drivers_openlcb.c`

Returns a fixed node ID (`0x0501010107AA`) for all boards. Two dsPIC boards
on the same CAN bus will have identical node IDs. TODO comment already present
about reading from protected flash or device unique ID.

### DSPIC33_BOOTLOADER_LESSONS_LEARNED.md -- DELETED

Content was integrated into `How_To_Modify_For_Your_dsPIC.md`. The old file
has been removed.
