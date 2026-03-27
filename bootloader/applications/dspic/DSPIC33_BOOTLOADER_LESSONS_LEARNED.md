# dsPIC33 Bootloader -- Lessons Learned

## 1. MCC-Generated ISR Files and Interrupt Redirection

**Context:** The bootloader occupies the start of flash (0x0200+) and therefore owns the hardware Interrupt Vector Table (IVT). After `jump_to_application()`, the application code runs but ALL interrupts still route through the bootloader's IVT. To let the application receive interrupts normally, the bootloader must redirect each fired interrupt to a function pointer registered by the application -- the Virtual Interrupt Vector Table (VIVT) pattern.

**Problem:** MCC generates `_T2Interrupt` (tmr2.c) and `_C1Interrupt` (can1.c) as strong (non-weak) symbols. Traps (`_OscillatorFail`, `_AddressError`, etc.) are generated as weak symbols. All of these ISRs need VIVT redirect logic.

**Solution -- hand-edit ALL MCC ISR files consistently:** For consistency and reliability, ALL VIVT redirects are hand-edited directly into the MCC source files where the ISR is defined. Each hand-edit is marked with a `/* HAND-EDIT: VIVT redirect */` comment and includes the `bootloader_shared_ram.h` header. No separate `bootloader_isr_redirect.c` file is used.

The three hand-edited MCC files:

- **`tmr2.c`** -- VIVT check added inside `_T2Interrupt`:
  ```c
  #include "../shared/bootloader_shared_ram.h"  /* HAND-EDIT */
  // ...
  void _T2Interrupt(void) {
      if (bootloader_vivt_jumptable.timer_2_handler) {
          bootloader_vivt_jumptable.timer_2_handler();
      }
      tmr2_obj.count++;
      tmr2_obj.timerElapsed = true;
      IFS0bits.T2IF = false;
  }
  ```

- **`can1.c`** -- VIVT check added inside `_C1Interrupt`:
  ```c
  #include "../shared/bootloader_shared_ram.h"  /* HAND-EDIT */
  // ...
  if (bootloader_vivt_jumptable.can1_handler) {
      bootloader_vivt_jumptable.can1_handler();
  }
  IFS2bits.C1IF = 0;
  ```

- **`traps.c`** -- VIVT check added to each trap handler (traps are weak so could be overridden in a separate file, but hand-editing keeps all VIVT logic in one consistent pattern):
  ```c
  #include "../shared/bootloader_shared_ram.h"  /* HAND-EDIT */
  // ...
  void ERROR_HANDLER _OscillatorFail(void) {
      INTCON1bits.OSCFAIL = 0;
      if (bootloader_vivt_jumptable.oscillatorfail_handler) {
          bootloader_vivt_jumptable.oscillatorfail_handler();
          return;
      }
      TRAPS_halt_on_error(TRAPS_OSC_FAIL);
  }
  // same pattern for _AddressError, _StackError, _MathError, _DMACError
  ```

**CRITICAL: Do NOT use MCC's TMR2_InterruptHandler function pointer for VIVT.** MCC's `TMR2_InterruptHandler` is a plain RAM global (`void (*TMR2_InterruptHandler)(void) = NULL;` in tmr2.c). When the bootloader jumps to the application, the app's CRT0 startup code re-initializes `.data` and `.bss` in RAM, clobbering this pointer back to NULL. The VIVT function pointers in `bootloader_vivt_jumptable` survive because they are declared with `__attribute__((persistent, address(...)))`. Always check the persistent VIVT directly -- never go through an MCC function pointer for interrupt redirect.

**MCC merge behaviour:** MCC does NOT blindly overwrite modified generated files. When you regenerate and a conflict is detected, MCC prompts you to keep your changes or accept the new generated version. Leave clear `HAND-EDIT` comments marking each change so you know what to re-apply if you ever accept a regeneration.

---

## 2. The VIVT Drop-Back Problem (Application --> Bootloader)

**Context:** When the application receives a Freeze command from the OpenLCB Config Tool it resets into the bootloader so the bootloader can receive new firmware. On dsPIC33 the hardware IVT is fixed in flash and always owned by the bootloader. After `jump_to_application()` the application runs but the bootloader's ISR stubs remain active. Each stub checks a function pointer in `bootloader_vivt_jumptable` (shared persistent SRAM) and calls through to the application's registered handler.

**The application's `CallbacksConfigMem_freeze()` performs the drop-back:**

```c
_GIE = 0;   /* hard-disable all interrupts including traps */

/* 1. Clear every VIVT slot FIRST */
bootloader_vivt_jumptable.oscillatorfail_handler = 0;
/* ... all slots ... */
bootloader_vivt_jumptable.can1_handler           = 0;

/* 2. Cache the CAN alias */
bootloader_cached_alias = statemachine_info->openlcb_node->alias;

/* 3. Write the magic flag */
bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC;

/* 4. Software reset -- SRAM survives */
asm("RESET");
```

**Why `_GIE = 0` instead of `__builtin_disi(0x3FFF)`:** DISI only blocks priority 0-6 interrupts for a cycle count. Traps (priority 7) can still fire. `_GIE = 0` is the hard global disable that blocks everything. Since we never need to re-enable (the reset fires next), `_GIE = 0` is correct.

**Why VIVT clear before magic flag:** If a trap fires between clearing VIVT and writing the magic flag, the bootloader stub sees NULL and halts safely rather than calling dead code.

**Bootloader `initialize_hardware(request)` now clears VIVT unconditionally:**

The old design only cleared VIVT on POR/BOR and relied on the application to clear it before drop-back. This had a race condition if an interrupt fired between `SYSTEM_Initialize()` enabling C1IE and `initialize_hardware()` clearing the VIVT. The new design:

1. Clears VIVT unconditionally on every reset (not just POR/BOR)
2. On app drop-back: disables C1IE and clears C1IF (prevents CAN interrupt from vectoring into NULL VIVT)
3. Enables `_GIE = 1` at the end after everything is safe
4. `SYSTEM_Initialize()` is hand-edited to NOT enable GIE

**Application startup responsibility:**

After `jump_to_application()` the application must REGISTER its handlers in the VIVT during startup with global interrupts disabled, then re-enable GIE:

```c
/* VIVT registration (GIE is off -- bootloader disabled it in cleanup_before_handoff) */
bootloader_vivt_jumptable.timer_2_handler = Dspic33Drivers_t2_interrupt_handler;
bootloader_vivt_jumptable.can1_handler    = Dspic33CanDriver_c1_interrupt_handler;
/* ... all handlers ... */

/* Initialize peripherals, library, etc. */

/* Re-enable global interrupts */
_GIE = 1;
```

**Complete VIVT lifecycle:**

```
Any reset (POR, BOR, software, programmer)
    initialize_hardware(request) zeros VIVT unconditionally
    initialize_hardware(request) clears bootloader_request_flag unconditionally
    If request != BOOTLOADER_REQUESTED_BY_APP:
        clears bootloader_cached_alias (stale garbage)
    If request == BOOTLOADER_REQUESTED_BY_APP:
        disables C1IE, clears C1IF
        leaves bootloader_cached_alias for CAN state machine
    Enables GIE after VIVT is safe and CAN interrupt is clean

jump_to_application()
    cleanup_before_handoff() disables GIE
    Application starts with GIE off

Application startup
    Application registers handler function addresses into VIVT slots
    Application initializes peripherals
    Application enables GIE

Freeze command received by application
    _GIE = 0 (hard disable all interrupts)
    Application zeros all VIVT slots
    Application caches CAN alias in shared RAM
    Application sets magic flag in shared RAM
    asm("RESET")

Bootloader starts again (software reset, SRAM intact)
    is_bootloader_requested() reads magic flag -- returns BOOTLOADER_REQUESTED_BY_APP
    initialize_hardware(BOOTLOADER_REQUESTED_BY_APP):
        clears VIVT, clears flag, disables C1IE, leaves cached alias, enables GIE
    Bootloader_init(request) sees REQUESTED_BY_APP -- skips app jump
    CAN state machine picks up cached alias -- no negotiation delay
```

---

## 3. XC16 16-bit uintptr_t and Flash Address Truncation

**Problem:** XC16 defines `uintptr_t` as 16-bit (sizeof(void*) == 2 on dsPIC33). Flash addresses above 0xFFFF are silently truncated when cast through `uintptr_t`. The bootloader library uses `uintptr_t` internally for pointer arithmetic. This caps the addressable application flash region to 0x0000--0xFFFE (about 54 KB of image space across 31 flash pages).

**Workaround:** `APP_FLASH_END` in `bootloader_drivers_openlcb.h` is capped at `0x00FFFE`. The full 512 KB flash is not accessible through the current library. Fixing this requires changing the library to use `uint32_t` instead of `uintptr_t` for flash addresses throughout.

**The `-mlarge-data` flag does NOT fix this:** The large-data memory model widens data pointers but does NOT allow reading program flash via a data pointer. dsPIC33 is a Harvard architecture -- flash and SRAM are separate address spaces. Even with 32-bit data pointers, dereferencing a flash address as a data pointer is undefined behavior. The correct way to read dsPIC33 flash is via table-read instructions (TBLRDL/TBLRDH), exposed by MCC as `FLASH_ReadWord24()`.

---

## 5. MCC ECAN Acceptance Filter: Entering an Extended-Frame Filter Value

To configure an acceptance filter or mask for **extended frames** in the MCC ECAN
GUI, append a lowercase `x` to the hex value in the filter/mask field:

```
0x0x
```

Without the trailing `x`, MCC treats the value as a standard-frame (11-bit SID)
filter and generates `EXIDE = 0` with `SID = 0x7FF` (full mask), which rejects
all extended-frame traffic including every OpenLCB frame.

With the trailing `x`, MCC generates `EXIDE = 1` and the correct mask/filter
register values for 29-bit extended IDs.

---

## 4. Harvard Architecture Flash Access (TBLRDL/TBLRDH)

**Problem:** dsPIC33 program flash cannot be read by dereferencing a data pointer (`*ptr`, `memcpy` from flash address, etc.). This is a fundamental Harvard architecture restriction -- program memory and data memory are separate address spaces. The bootloader library's `_check_application_checksum()` originally used `memcpy` to copy the app header from flash and dereferenced a pointer to check whether flash was blank -- both fail silently on dsPIC33.

**Solution:** Added a `read_flash_bytes()` function pointer to `bootloader_openlcb_driver_t`. The dsPIC33 implementation uses `FLASH_ReadWord24()` (MCC table-read wrapper) to extract 3 data bytes per 24-bit instruction word. The bootloader library was updated to call `read_flash_bytes()` instead of `memcpy` / direct dereference for all flash reads.

**Instruction word layout:**
```
FLASH_ReadWord24(addr) returns uint32_t:
  bits[7:0]   = instruction bits [7:0]    --> dest[0]
  bits[15:8]  = instruction bits [15:8]   --> dest[1]
  bits[23:16] = instruction bits [23:16]  --> dest[2]
  bits[31:24] = phantom (always 0, do not use)
PC address advances by 2 per instruction.
```

**Contrast with Von Neumann (ARM):** On STM32 and MSPM0 flash is memory-mapped and readable via a normal data pointer. The `read_flash_bytes()` implementation on those platforms is simply `memcpy(dest, (const void *)flash_addr, size_bytes)`. The DI function pointer lets both architectures share the same library code with different implementations injected at startup.

**CRC/checksum for large flash regions:** A streaming CRC API (`bootloader_crc3_state_t`) was added to `bootloader_crc.c` so the `compute_checksum` driver function can process flash one instruction at a time without needing a large RAM buffer. This is necessary on dsPIC33 because flash cannot be bulk-copied to RAM for checksumming.

---

## 6. Application Reset Vector (`reset_vector.s`) Required for Bootloaded Apps

**Problem:** The bootloader jumps to the application entry point (e.g. PC 0x8000) via a function pointer call. Without an explicit `GOTO __reset` instruction at that address, the PC lands on whatever code the linker happened to place first in the `program` section — NOT the C runtime startup (`__reset` / `crt0`). The application hangs or crashes because the stack pointer, data sections, and C runtime are never initialised.

**Why it's not automatic:** The standard XC16 linker script emits a `GOTO __reset` in the `.reset` section at address 0x0000. But address 0x0000 is owned by the bootloader. Even with `__CORESIDENT` defined (which suppresses the 0x0000 reset vector), the linker does NOT automatically place a `GOTO __reset` at the application's `program` origin.

**Solution:** Add a `reset_vector.s` assembly file to the application project that forces a `GOTO __reset` at the application start address:

```asm
; reset_vector.s
;
; Places a GOTO __reset at the application entry point so the
; bootloader can jump here and reach the C runtime startup.

    .section resetvector, address(0x8000), code
    goto __reset
```

The address must match `APP_FLASH_START` in the bootloader's `bootloader_drivers_openlcb.h` and the `ORIGIN` of the `program` region in the application's linker script.

**Symptom without this file:** The bootloader UART debug log shows a successful jump (`JUMP TO APP at PC=0x00008000`) but the application never runs. A hard power cycle produces the same result — the data in flash is verified correct but execution hangs immediately after the jump.

**TurnoutBoss reference:** The working TurnoutBoss bootloader uses this exact pattern — both the bootloader and application projects have their own `reset_vector.s` placing `GOTO __reset` at their respective entry points (0x0200 for bootloader, 0xB000 for application).

---

## 7. Stale `bootloader_cached_alias` After Programmer Reset

**Problem:** After programming the dsPIC via PICkit/ICD and the device resets, `bootloader_cached_alias` (in `.noinit` / persistent RAM) contains garbage -- typically 0xFFFF. The CAN alias negotiation state machine checks this variable and, if non-zero, skips CID negotiation and uses it directly as the node's alias. A 16-bit value like 0xFFFF overflows the 12-bit alias field, corrupting the MTI bits of the CAN identifier. The first frame sent looks like `[1910FFFF] 05 01 01 01 07 AA` -- a malformed CAN frame that no node on the bus recognises.

**Symptom:** After programming, the bootloader sends one junk CAN frame and then goes silent. The PC configuration tool never sees the node. A hard power cycle produces the same result because SRAM is not guaranteed to be zeroed on POR either -- `.noinit` variables contain whatever was in RAM.

**Root cause:** The old `initialize_hardware()` only cleared `.noinit` variables on POR/BOR resets (checking `RCONbits.POR || RCONbits.BOR`). Programmer resets, watchdog resets, and software resets did not clear the cached alias.

**Solution:** `initialize_hardware(request)` now clears `bootloader_cached_alias` unconditionally on every reset EXCEPT when `request == BOOTLOADER_REQUESTED_BY_APP` (where the alias is intentionally preserved for fast CAN re-login). The cleanup decision is based on the `request` parameter, not hardware reset-cause registers.

```c
bootloader_request_flag = 0;  /* always clear */

if (request != BOOTLOADER_REQUESTED_BY_APP) {
    bootloader_cached_alias = 0;  /* stale garbage on POR/programmer/button */
}
```

**Why not use RCON bits:** Hardware reset-cause registers (`RCONbits.POR`, `RCONbits.BOR`, `RCC->CSR` on STM32, `esp_reset_reason()` on ESP32) are unreliable for this purpose. They may have stale flags from prior resets, require manual clearing, and have platform-specific edge cases. The `request` parameter from `is_bootloader_requested()` is unambiguous: if the app wrote the magic flag, it's a drop-back; otherwise, clear everything.

---

## 8. Double-Call Bug in `Bootloader_init()` / `Bootloader_entry()`

**Problem:** `Bootloader_entry()` calls `is_bootloader_requested()` to check the magic flag, then passes execution to `Bootloader_init()`. The old `Bootloader_init()` called `is_bootloader_requested()` a second time. But `is_bootloader_requested()` consumed (cleared) the magic flag on the first call. The second call always returned `BOOTLOADER_NOT_REQUESTED`, causing the bootloader to attempt a jump back to the application instead of staying in bootloader mode.

**Symptom:** On app drop-back (Freeze command), the CAN trace shows the bootloader briefly starting, then the application restarting with full CID negotiation. The bootloader bounces back to the app because it thinks no bootloader request was made.

**Solution:** `Bootloader_init()` now takes a `bootloader_request_t request` parameter. `Bootloader_entry()` calls `is_bootloader_requested()` once, passes the result to `initialize_hardware(request)` and then to `Bootloader_init(can_driver, openlcb_driver, request)`. The magic flag is consumed once and flows through as a parameter.

```c
void Bootloader_entry(...) {
    bootloader_request_t request = openlcb_driver->is_bootloader_requested();
    openlcb_driver->initialize_hardware(request);
    if (Bootloader_init(can_driver, openlcb_driver, request)) { return; }
    while (true) {
        if (Bootloader_loop()) { return; }
    }
}
```

**This is a library-level fix** in `bootloader/src/openlcb/bootloader.c` that affects all platforms.

---

## 9. GIE Ownership and `SYSTEM_Initialize()` Hand-Edit

**Context:** On dsPIC33, MCC's `SYSTEM_Initialize()` enables global interrupts (`_GIE = 1`) as one of its last steps. In the bootloader, this is dangerous: if GIE is enabled before the VIVT is cleared, a pending interrupt (e.g. CAN C1IF left set from the application) will vector through the bootloader's ISR stub, find a NULL function pointer in the VIVT, and crash.

**Solution:** `SYSTEM_Initialize()` (in MCC-generated `system.c`) is hand-edited to NOT enable GIE. The `initialize_hardware()` function owns GIE: it clears the VIVT, optionally disables C1IE/clears C1IF on app drop-back, and enables `_GIE = 1` at the very end when everything is safe.

```
Ownership chain:
  SYSTEM_Initialize()           -- does NOT touch GIE (hand-edited)
  initialize_hardware(request)  -- clears VIVT, sets up peripherals, enables GIE last
  cleanup_before_handoff()      -- disables GIE before jump_to_application()
  Application startup           -- registers VIVT handlers, enables GIE when ready
```

Mark the hand-edit in `system.c` with a `/* HAND-EDIT: GIE disabled -- bootloader owns GIE lifecycle */` comment so it's preserved across MCC regenerations.
