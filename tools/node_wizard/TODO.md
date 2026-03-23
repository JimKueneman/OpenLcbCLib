# Node Wizard — TODO

### 1. Update firmware_write Prototype

**Issue:** The `firmware_write` callback signature changed to include a `write_result_t` completion callback parameter. The Node Wizard code generator still emits the old two-parameter signature.

**Scope:** Update the Node Wizard template for `firmware_write` to generate:
```c
void CallbacksConfigMem_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result)
```

**Files:**
- `drivers/js/driver_defs.js:143` — update params
- `callbacks/js/callback_defs.js:378` — update params
- `js/codegen.js:1342` — update struct wiring

### 2. Fix write_firmware → firmware_write Name Mismatch

**Issue:** `driver_defs.js:141` uses `name: 'write_firmware'` which generates `Drivers_write_firmware()`, but the struct field is `.firmware_write` and the correct convention is `Drivers_firmware_write()`.

**Scope:** Rename `name: 'write_firmware'` to `name: 'firmware_write'` in `driver_defs.js`.

### 3. Rename _setup to _initialize in Driver Defs

**Issue:** Driver function names should use `initialize` not `setup` to match the standardized convention across all application demos.

**Scope:** Update `driver_defs.js` function names from `setup` to `initialize` for both CAN and OLCB driver groups.

### 4. Add #include <stddef.h> to Generated openlcb_user_config.c

**Issue:** Generated `openlcb_user_config.c` uses `NULL` (e.g. `.fdi = NULL`) but does not include `<stddef.h>`, causing a compile error on some toolchains.

**Scope:** Add `#include <stddef.h>` to the generated includes in `js/codegen.js` `generateC()`.

### 5. ~~Disable Platform Cards with Under Construction Notice~~

**Status:** COMPLETE

**Scope:** All platform cards except None/Custom are hidden (`disabled: true` in `platform_defs.js`). An "Additional target platforms are under construction." notice is shown in the Target Platform panel (`platform/js/platform.js`).
