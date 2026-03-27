# TI MSPM0G3507 Launchpad Bootloader TODO

Issues found by swimlane analysis against the dsPIC reference implementation.

## MEDIUM

### Freeze callback does not disable interrupts before writing shared RAM

**File:** `BasicNode/application_callbacks/callbacks_config_mem.c:52-67`

The freeze callback writes `bootloader_cached_alias` and `bootloader_request_flag` then
calls `DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_SYSRST)` without disabling interrupts first.
If a CAN (MCAN) interrupt fires between the alias write and the reset, it could corrupt
state or cause the library to process a stale message while shared RAM is half-written.

The dsPIC reference does `_GIE = 0` and disables CAN interrupts before touching shared
RAM (see `bootloader/applications/dspic/BasicNode.X/src/application_callbacks/callbacks_config_mem.c:139-148`).

Suggested fix -- add before the shared RAM writes:

```c
__disable_irq();
```

### Misleading comment about cleanup

**File:** `BasicNode/application_callbacks/callbacks_config_mem.c:58`

Comment says "TI_DriverLibDrivers_cleanup_before_handoff will get called by the library"
but the `DL_SYSCTL_resetDevice()` call fires immediately -- the library never gets a chance
to call cleanup. The comment should be removed or the cleanup should be called explicitly
before reset if peripheral teardown matters.

### `NO_CHECKSUM` is globally enabled

**File:** Shared `bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot and
after firmware write. Any corrupted image will be jumped to. Acceptable for development, must
be addressed before production.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c`

Returns `0x050101012200ULL` for all boards. Two MSPM0 boards on the same CAN bus will have
identical node IDs. TODO comment already present about reading from protected flash sector
at `NODEID_FLASH_ADDRESS`.

### No button debouncing

**File:** `BasicNodeBootloader/openlcb_c_lib_bootloader/drivers/bootloader_drivers_openlcb.c:116-122`

Single read of GPIO PB21 with no debounce logic. Electrical noise could theoretically
trigger false bootloader entry. Low risk since the button is only checked once at startup,
but worth noting for noisy environments.

### `finalize_flash()` checksum validation is stubbed

Returns 0 unconditionally without validating the written image. Gated by `NO_CHECKSUM`.
Once the post-link checksum tool is integrated, implement the validation logic here.
