# ESP32 Bootloader TODO

Issues found by swimlane analysis against the dsPIC reference implementation.

## CRITICAL

### Freeze callback is a stub -- Swimlane 3 (app drop-back) does not work

**File:** `BasicNode/src/application_callbacks/callbacks_config_mem.c:54-62`

The freeze callback only prints a message. It does NOT:
- Write the current CAN alias to `bootloader_cached_alias`
- Write `BOOTLOADER_REQUEST_MAGIC` to `bootloader_request_flag`
- Call `esp_restart()`

The full implementation should follow the STM32/MSPM0 pattern:

```c
void CallbacksConfigMem_freeze(...) {
    if (...address_space == CONFIG_MEM_SPACE_FIRMWARE) {
        bootloader_cached_alias = statemachine_info->openlcb_node->alias;
        bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC;
        esp_restart();
    }
}
```

Compare to:
- dsPIC: `bootloader/applications/dspic/BasicNode.X/src/application_callbacks/callbacks_config_mem.c:135-184`
- STM32: `bootloader/applications/stm32_cubeide/.../callbacks_config_mem.c:67-80`
- MSPM0: `bootloader/applications/ti_theia/.../callbacks_config_mem.c:52-67`

## HIGH

### `jump_to_application()` broken on cold boot path

**File:** `BasicNodeBootloader/src/application_drivers/bootloader_drivers_openlcb.c:194-214`

On cold boot (valid app in flash, no firmware update requested), `jump_to_application()`
calls `esp_restart()`. But `esp_ota_set_boot_partition()` is only called inside
`finalize_flash()` (after a firmware write completes). On a plain cold boot the ROM
bootloader may not know which OTA partition holds the valid app.

The dsPIC/STM32/MSPM0 all do a direct vector-table jump that always works. ESP32 needs
to resolve the valid application partition and call `esp_ota_set_boot_partition()` before
`esp_restart()` on the cold boot path.

There is already a TODO comment at line 205 acknowledging this.

## MEDIUM

### Verify that inactive OTA partition is memory-mapped for reads

**File:** `BasicNodeBootloader/src/application_drivers/bootloader_drivers_openlcb.c:228-241`

`get_flash_boundaries()` returns addresses based on `ESP32_FLASH_MMAP_BASE + partition->address`.
`read_flash_bytes()` does `memcpy(dest, (void*)flash_addr, size)` using those mapped addresses.

ESP-IDF automatically maps the running partition, but the inactive OTA partition (the write
target) may NOT be mapped. The checksum validation at boot reads `flash_min` which points at
the write target. If not mapped, this will segfault.

Verify whether `esp_partition_read()` should be used instead of `memcpy()` for reads to the
inactive partition.

### `NO_CHECKSUM` is globally enabled

**File:** `BasicNodeBootloader/src/openlcb_c_lib_bootloader/openlcb/bootloader_types.h:218`

`#define NO_CHECKSUM` is uncommented. All checksum validation is skipped -- both at boot and
after firmware write. Any corrupted image will be jumped to. Acceptable for development, must
be addressed before production.

## LOW

### Node ID is hardcoded

**File:** `BasicNodeBootloader/src/application_drivers/bootloader_drivers_openlcb.c:391-396`

Returns `0x050101012201ULL` for all boards. Two ESP32 boards on the same CAN bus will have
identical node IDs, causing alias collisions and protocol violations.

TODO comment already present about reading from NVS.

### `finalize_flash()` checksum validation is stubbed

**File:** `BasicNodeBootloader/src/application_drivers/bootloader_drivers_openlcb.c:308-330`

Returns success unconditionally without validating the written image (gated by `NO_CHECKSUM`).
The `esp_ota_set_boot_partition()` call is present, which is correct. Once `NO_CHECKSUM` is
removed, the checksum verification logic needs to be implemented.
