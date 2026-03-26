# dsPIC33 Bootloader -- Lessons Learned

## 1. MCC-Generated ISR Files and Interrupt Redirection

**Context:** The bootloader occupies the start of flash (0x0200+) and therefore owns the hardware Interrupt Vector Table (IVT). After `jump_to_application()`, the application code runs but ALL interrupts still route through the bootloader's IVT. To let the application receive interrupts normally, the bootloader must redirect each fired interrupt to a function pointer registered by the application -- the Virtual Interrupt Vector Table (VIVT) pattern.

**Problem:** MCC generates `_T2Interrupt` (tmr2.c) and `_C1Interrupt` (can1.c) as strong (non-weak) symbols. You cannot define your own versions in a separate file without a linker conflict. This means adding VIVT redirect logic normally requires editing MCC-generated files, which get overwritten on regeneration.

**Workaround for `_T2Interrupt`:** MCC's `_T2Interrupt` already calls through a function pointer (`TMR2_InterruptHandler`). Call `TMR2_SetInterruptHandler(vivt_t2_proxy)` from application code before jumping. The proxy checks `bootloader_state.interrupt_redirect` and calls `vivt_jumptable.timer_2_handler`. No MCC file modification needed.

**No clean workaround for `_C1Interrupt`:** MCC's `_C1Interrupt` only handles the WAKIF (bus wakeup) flag and exposes only a `CAN1_BusWakeUpActivityInterruptHandler` hook inside that branch. There is no hook for general CAN RX/TX interrupts. Options:
- Modify `can1.c` (small -- add ~3 lines before `IFS2bits.C1IF = 0`)
- Replace MCC's CAN driver entirely with a hand-written `_C1Interrupt` that has VIVT support built in (cleaner long-term, same approach used in TurnoutBossBootloader.X)

**Traps are fine:** MCC marks all trap handlers with `__attribute__((weak))`, so they can be fully overridden in a separate project file with no modification to MCC files.

**Why MCC only put WAKIF inside `_C1Interrupt`:** MCC always emits the `_C1Interrupt` wrapper function regardless of what is checked -- it just fills the body with only the interrupt sources that are enabled. Unchecking all CAN interrupt sources results in an ISR shell that only contains the WAKIF (bus wakeup) handler, because that is the one source MCC still enabled. The symbol `_C1Interrupt` is still claimed by MCC; only the body is minimal.

**Solution -- disable the interrupt in MCC:** The bootloader polls CAN; it does not need `_C1Interrupt` at all. Uncheck the CAN interrupt in MCC Easy Setup so MCC stops generating `_C1Interrupt` in `can1.c`. With that symbol now unclaimed, define your own `_C1Interrupt` in a project-owned file (e.g. `bootloader_isr_redirect.c`) with full VIVT redirect logic. No MCC file modification required.

**Takeaway:** If the bootloader does not need a peripheral's ISR (because it polls that peripheral), disable the interrupt in MCC. This frees the ISR symbol so you can define it yourself with VIVT support -- without ever touching a generated file.

**MCC merge behaviour:** MCC does NOT blindly overwrite modified generated files. When you regenerate and a conflict is detected, MCC prompts you to keep your changes or accept the new generated version. It is therefore safe to hand-edit generated files (e.g. adding VIVT redirect lines to `_C1Interrupt` in `can1.c`). Leave a clear comment marking the hand-edit so you know what to re-apply if you ever accept a regeneration.

---

## 2. The VIVT Drop-Back Problem (Application --> Bootloader)

**Context:** When the application receives a Freeze command from the OpenLCB Config Tool it resets into the bootloader so the bootloader can receive new firmware. On dsPIC33 the hardware IVT is fixed in flash and always owned by the bootloader. After `jump_to_application()` the application runs but the bootloader's ISR stubs remain active. Each stub checks a function pointer in `bootloader_vivt_jumptable` (shared persistent SRAM) and calls through to the application's registered handler.

**The Gap:** A software reset (`asm("RESET")`) preserves SRAM. The VIVT function pointers still hold the application's handler addresses after the reset. The bootloader's `initialize_hardware()` only clears the VIVT on POR or BOR (power-on/brownout) -- it deliberately leaves SRAM intact on a software reset so it can read the magic handshake flag and cached CAN alias that the application wrote just before the reset.

**What goes wrong without the fix:** The first interrupt that fires in the bootloader after drop-back (typically the TMR2 100ms tick) will see a non-NULL VIVT slot and call into dead application code. The application's code is still in flash but its stack, data, and peripherals are no longer properly set up. The result is a hard fault, a stack overflow, or silent state corruption.

**The Fix -- Application clears VIVT before reset:**

The application's `CallbacksConfigMem_freeze()` (in `callbacks_config_mem.c`) performs the drop-back in this exact order, with global interrupts disabled throughout:

```c
__builtin_disi(0x3FFF);   /* disable interrupts for the critical section */

/* 1. Clear every VIVT slot FIRST */
bootloader_vivt_jumptable.oscillatorfail_handler = 0;
bootloader_vivt_jumptable.addresserror_handler   = 0;
bootloader_vivt_jumptable.stackerror_handler     = 0;
bootloader_vivt_jumptable.matherror_handler      = 0;
bootloader_vivt_jumptable.dmacerror_handler      = 0;
bootloader_vivt_jumptable.timer_2_handler        = 0;
bootloader_vivt_jumptable.can1_handler           = 0;

/* 2. Cache the CAN alias so the bootloader can skip negotiation */
bootloader_cached_alias = statemachine_info->openlcb_node->alias;

/* 3. Write the magic flag so the bootloader knows this was intentional */
bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC;

/* 4. Reset -- SRAM survives, POR/BOR bits stay clear */
asm("RESET");
```

**Why this order matters:**
- VIVT clear FIRST: ensures that if a trap fires in the narrow window between clearing VIVT and writing the magic flag, the bootloader stub sees NULL and halts safely rather than calling dead code.
- Magic flag LAST: `initialize_hardware()` does not clear shared RAM on software reset so the bootloader can read the flag and alias that the application just wrote.

**Responsibility boundary:**

| Event | Who clears VIVT |
|---|---|
| Power-on or brownout reset | `initialize_hardware()` -- random SRAM must be zeroed |
| Application drop-back (software reset) | The APPLICATION -- in the Freeze callback |

This division is intentional. `initialize_hardware()` cannot clear VIVT on software reset because doing so would also wipe the magic flag and cached alias the application wrote.

**Application startup responsibility:**

After `jump_to_application()` the application must REGISTER its handlers in the VIVT during startup with global interrupts disabled:

```c
/* With global interrupts disabled (e.g. in BasicNodeDrivers_initialize()): */
bootloader_vivt_jumptable.timer_2_handler = &my_t2_handler;
bootloader_vivt_jumptable.can1_handler    = &Dspic33CanDriver_c1_interrupt_handler;
/* ... register any other handlers needed ... */
```

Without this registration the application's interrupt-driven code will silently not fire -- the bootloader ISR stubs see NULL and skip the call.

Note: The application's `_T2Interrupt` and `_C1Interrupt` symbols are compiled into the application binary but they are NOT in the hardware IVT (which is owned by the bootloader). They are effectively unreachable unless called explicitly through the VIVT. The handler logic must be extracted into plain functions and registered in the VIVT table.

**Complete VIVT lifecycle:**

```
Power-on / BOR
    initialize_hardware() zeros VIVT (random SRAM is dangerous)
    Bootloader runs -- all VIVT slots NULL -- ISR stubs are no-ops / spin on traps

jump_to_application()
    Application code runs -- VIVT still all NULL

Application startup (global interrupts disabled)
    Application writes handler function addresses into VIVT slots
    Application enables global interrupts
    Interrupts now flow: hardware IVT --> bootloader stub --> VIVT --> app handler

Freeze command received by application
    (global interrupts disabled via __builtin_disi(0x3FFF))
    Application zeros all VIVT slots           <-- CRITICAL STEP
    Application caches CAN alias in shared RAM
    Application sets magic flag in shared RAM
    asm("RESET")

Bootloader starts again (software reset, SRAM intact)
    initialize_hardware() sees POR=0, BOR=0 -- leaves SRAM untouched
    is_bootloader_requested() reads magic flag -- returns BOOTLOADER_REQUESTED_BY_APP
    Bootloader enters firmware-update mode with VIVT all NULL (safe)
    Bootloader reuses cached alias -- no alias negotiation delay
```

---

## 3. XC16 16-bit uintptr_t and Flash Address Truncation

**Problem:** XC16 defines `uintptr_t` as 16-bit (sizeof(void*) == 2 on dsPIC33). Flash addresses above 0xFFFF are silently truncated when cast through `uintptr_t`. The bootloader library uses `uintptr_t` internally for pointer arithmetic. This caps the addressable application flash region to 0x0000--0xFFFE (about 54 KB of image space across 31 flash pages).

**Workaround:** `APP_FLASH_END` in `bootloader_drivers_openlcb.h` is capped at `0x00FFFE`. The full 512 KB flash is not accessible through the current library. Fixing this requires changing the library to use `uint32_t` instead of `uintptr_t` for flash addresses throughout.

**The `-mlarge-data` flag does NOT fix this:** The large-data memory model widens data pointers but does NOT allow reading program flash via a data pointer. dsPIC33 is a Harvard architecture -- flash and SRAM are separate address spaces. Even with 32-bit data pointers, dereferencing a flash address as a data pointer is undefined behavior. The correct way to read dsPIC33 flash is via table-read instructions (TBLRDL/TBLRDH), exposed by MCC as `FLASH_ReadWord24()`.

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
