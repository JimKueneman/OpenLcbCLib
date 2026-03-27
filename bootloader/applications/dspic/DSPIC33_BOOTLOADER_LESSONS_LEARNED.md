# dsPIC33 Bootloader and Application Demo -- Tutorial and Reference

This document explains how the BasicNodeBootloader.X (bootloader) and
BasicNode.X (application) projects work together on the dsPIC33EP512MC506.
It covers the architecture, the build and flash workflow, the MCC hand-edits
that must be reapplied after code generation, and the pitfalls that were
discovered during development.

---

## 1. Flash Layout

The dsPIC33EP512MC506 has 512 KB of program flash.  The bootloader and
application share it as follows:

```
Address        Contents
-----------    ------------------------------------------
0x00000        Reset vector (GOTO __reset) -- bootloader
0x00004        Hardware IVT (126 vectors)  -- bootloader
0x00200        Bootloader code begins
0x07FFC        Bootloader code ends
0x08000        Application entry (GOTO __reset via reset_vector.s)
  ...          Application code
0x547FC        Application code ends
0x54800        Checksum / Node ID page (reserved by bootloader)
0x55000        Configuration bits page (hardware protected)
```

Both linker scripts pin data RAM at the same region:

```
data  ORIGIN = 0x1000,  LENGTH = 0xC000   (48 KB)
```

### Linker scripts

- Bootloader: `p33EP512MC506_BasicNodeBootloader.gld`
  - `program ORIGIN = 0x200, LENGTH = 0x8000 - 0x200 - 0x4`
- Application: `p33EP512MC506_BasicNode.gld`
  - `program ORIGIN = 0x8000, LENGTH = 0x55800 - 0x8000 - 0x800 - 0x800 - 0x4`

### Application `reset_vector.s`

The standard XC16 linker emits `GOTO __reset` at address 0x0000, which the
bootloader owns.  The application must place its own `GOTO __reset` at its
entry point so the bootloader can jump there:

```asm
    .section resetvector, address(0x8000), code
    goto __reset
```

Without this file the bootloader jumps to 0x8000 and lands on whatever code
the linker happened to place first -- NOT the C runtime.  The application
hangs silently.

### `__CORESIDENT` linker flag

Add `--defsym __CORESIDENT=1` to the application's XC16 linker options.
This suppresses the default reset vector at 0x0000 so it does not collide
with the bootloader's.

---

## 2. Shared RAM (Bootloader <-> Application Handshake)

Both projects compile the same header: `shared/bootloader_shared_ram.h`.
It declares three persistent variables pinned to fixed SRAM addresses using
`__attribute__((persistent, address(0xNNNN)))`:

```
Address   Variable                     Purpose
-------   -------------------------    ----------------------------------------
0x1000    bootloader_cached_alias      CAN alias passed from app to bootloader
0x1002    bootloader_request_flag      Magic value 0xB00710AD triggers bootloader mode
0x1006    bootloader_vivt_jumptable    7 function pointers for interrupt redirect
```

`persistent` means the C startup code does NOT zero these variables.  They
survive a software reset (`asm("RESET")`) because SRAM content is preserved
when neither POR nor BOR occurs.

**Pitfall -- stale cached alias:** After programming with PICkit/ICD, SRAM
contains garbage.  If `bootloader_cached_alias` happens to be non-zero the
CAN state machine skips alias negotiation and uses a garbage value -- often
0xFFFF, which overflows the 12-bit alias field and corrupts the CAN
identifier.  The fix: `initialize_hardware()` clears `bootloader_cached_alias`
unconditionally on every reset EXCEPT `BOOTLOADER_REQUESTED_BY_APP`.

---

## 3. Interrupt Architecture (VIVT)

### The problem

The dsPIC33 hardware Interrupt Vector Table (IVT) is at a fixed flash
address (0x0004) owned by the bootloader.  After `jump_to_application()` the
application runs but ALL interrupts still route through the bootloader's ISR
stubs.

### The solution -- Virtual Interrupt Vector Table

The bootloader's ISR stubs check function pointers in
`bootloader_vivt_jumptable` (shared persistent SRAM).  If non-NULL, the stub
calls through the pointer to the application's handler.  If NULL, the stub
is a safe no-op (or halts for traps).

The application registers its handlers during startup before enabling GIE:

```c
bootloader_vivt_jumptable.timer_2_handler = Dspic33OlcbDrivers_t2_interrupt_handler;
bootloader_vivt_jumptable.can1_handler    = Dspic33CanDriver_c1_interrupt_handler;
// ... trap handlers ...
_GIE = 1;
```

### Why not use MCC's TMR2_InterruptHandler pointer?

MCC's `TMR2_InterruptHandler` is a plain `.data` global.  When the
bootloader jumps to the application, the app's CRT0 startup re-initializes
`.data` and `.bss`, clobbering this pointer back to NULL.  The VIVT slots
survive because they are `persistent` -- they are NOT touched by CRT0.

### Two call paths for each handler

Each interrupt handler in the application has two call paths:

**Standalone (no bootloader):** The MCC ISR in the application project calls
the handler directly.  For TMR2 this uses the weak `TMR2_CallBack()`
override.  For CAN1 this uses a hand-edit in `can1.c` since MCC has no
weak callback for CAN.

**Bootloaded:** The bootloader's ISR stub reads the VIVT function pointer
and calls the handler indirectly.  The bootloader's `_T2Interrupt` calls
`bootloader_vivt_jumptable.timer_2_handler()` and the bootloader's
`_C1Interrupt` calls `bootloader_vivt_jumptable.can1_handler()`.

Both paths call the same function.  The VIVT handler must clear its own
interrupt flag (e.g. `IFS0bits.T2IF = 0`) because the MCC ISR is not in
the call path when bootloaded.  The standalone `TMR2_CallBack()` does not
need to clear T2IF because MCC's `_T2Interrupt` handles that.

---

## 4. GIE Ownership

Global Interrupt Enable (_GIE) must be carefully managed.  The rule is:
**whoever creates a safe state enables GIE last.**

```
SYSTEM_Initialize()           -- does NOT touch GIE (hand-edited in both projects)
Bootloader: initialize_hardware()  -- clears VIVT, sets up CAN, enables GIE last
Bootloader: cleanup_before_handoff() -- disables GIE before jumping to app
Application: main()           -- registers VIVT, inits peripherals, enables GIE last
Application: freeze()         -- disables GIE, clears VIVT, resets into bootloader
```

MCC's `SYSTEM_Initialize()` normally enables GIE as one of its last steps.
This is hand-edited out in BOTH projects.  If MCC regenerates `system.c`,
re-comment the `INTERRUPT_GlobalEnable()` call.

**Why this matters:** If GIE is enabled before the VIVT is cleared (bootloader)
or populated (application), a pending interrupt will vector through a NULL
or stale function pointer and crash.

---

## 5. MCC Hand-Edits Summary

Both projects use MCC-generated driver files with specific hand-edits.  If
MCC regenerates any of these files, the edits must be reapplied.  All edits
are marked with `/* HAND-EDIT */` comments.

### Bootloader project (BasicNodeBootloader.X)

| File | Edit | Purpose |
|------|------|---------|
| `system.c` | Comment out `INTERRUPT_GlobalEnable()` | Bootloader owns GIE lifecycle |
| `tmr2.c` | Add `#include "../../shared/bootloader_shared_ram.h"` and VIVT check in `_T2Interrupt` | Forward TMR2 interrupt to application |
| `can1.c` | Add `#include "../../shared/bootloader_shared_ram.h"` and VIVT check in `_C1Interrupt` | Forward CAN1 interrupt to application |
| `traps.c` | Add VIVT check in each trap handler | Forward traps to application |

### Application project (BasicNode.X)

| File | Edit | Purpose |
|------|------|---------|
| `system.c` | Comment out `INTERRUPT_GlobalEnable()` | main() owns GIE lifecycle |
| `can1.c` | Add `#include "../src/application_drivers/dspic33_can_drivers.h"` and direct call to `Dspic33CanDriver_c1_interrupt_handler()` in `_C1Interrupt` | MCC has no weak callback for CAN; this is the only hook point |

The application's `tmr2.c` does NOT need a hand-edit.  MCC provides a weak
`TMR2_CallBack()` that the application overrides in `dspic33_olcb_drivers.c`.

### MCC regeneration behavior

MCC does NOT silently overwrite modified files.  When a conflict is detected
it prompts you to keep your changes or accept the new version.  The
`HAND-EDIT` comments make it clear what to reapply if you accept a
regeneration.

---

## 6. Application-to-Bootloader Drop-Back (Freeze)

When the OpenLCB Config Tool sends a Freeze command targeting the firmware
address space (0xEF), the application's `CallbacksConfigMem_freeze()`
executes the drop-back sequence:

```
Step 1:  _GIE = 0              Disable all interrupts including traps
         C1INTEbits.RBIE = 0   Disable CAN RX interrupt source
         IEC2bits.C1IE = 0     Disable CAN module interrupt

Step 2:  Clear all VIVT slots  Prevent bootloader ISR stubs from calling
                               dead application code

Step 3:  Cache CAN alias       bootloader_cached_alias = node->alias
                               Bootloader reuses alias (no 200ms negotiation)

Step 4:  Set magic flag        bootloader_request_flag = 0xB00710AD
                               Tells bootloader: this is a deliberate drop-back

Step 5:  asm("RESET")          Software reset, SRAM preserved
```

**Order matters:** VIVT must be cleared (step 2) BEFORE the magic flag is
written (step 4).  If a trap fired between those two writes, the bootloader
could see the flag but find stale VIVT pointers pointing into dead app code.

**Why `_GIE = 0` not `DISI`:** The `DISI` instruction only blocks priority
0-6 interrupts for a cycle count.  Traps (priority 7) can still fire.
`_GIE = 0` is the hard global disable that blocks everything.

---

## 7. Bootloader Startup Sequence

After any reset, the bootloader runs:

```
1. _GIE = 0
2. SYSTEM_Initialize()       -- clock, pins, TMR2, DMA, CAN1 (GIE stays off)
3. is_bootloader_requested() -- reads and consumes magic flag
4. initialize_hardware(request):
     a. Clear VIVT unconditionally
     b. Clear bootloader_request_flag unconditionally
     c. If NOT app drop-back: clear bootloader_cached_alias (stale garbage)
     d. If app drop-back: disable C1IE, clear C1IF, keep cached alias
     e. Enable _GIE = 1 last
5. Bootloader_init(request):
     - If valid app exists and NOT requested by app: jump to application
     - Otherwise: enter firmware update mode, start polling CAN
```

**Pitfall -- double call bug:** `is_bootloader_requested()` consumes (clears)
the magic flag.  If called twice, the second call always returns
`BOOTLOADER_NOT_REQUESTED`.  The fix: call it once in `Bootloader_entry()`,
pass the result as a parameter to `initialize_hardware()` and
`Bootloader_init()`.

---

## 8. CAN Configuration

Both projects use the same MCC CAN configuration:

- 125 kbps at Fcy = 40 MHz
- Extended frames (29-bit identifiers)
- DMA0 = CAN1 TX, DMA1 = CAN1 RX

### MCC ECAN filter trick

To accept extended frames, append a lowercase `x` to the filter/mask value
in the MCC ECAN GUI:

```
0x0x
```

Without the trailing `x`, MCC treats it as a standard-frame filter and
generates `EXIDE = 0`, rejecting all extended-frame (OpenLCB) traffic.

### CAN interrupt enable

MCC is configured with CAN interrupt DISABLED (the bootloader uses polling).
The application enables CAN interrupts in `Dspic33CanDriver_initialize()`:

```c
C1INTEbits.RBIE = 1;   // RX buffer interrupt sub-source
IEC2bits.C1IE = 1;     // CAN1 module interrupt enable
```

There is no MCC API for toggling these bits.  Direct register writes are
required.

---

## 9. Clock Configuration

- 16 MHz external crystal
- PLL: PLLPRE /4, PLLFBD x40, PLLPOST /2
- Fosc = 80 MHz, Fcy = 40 MHz

### Derived peripheral settings

- TMR2: PR2 = 15624, prescaler 1:256, gives 100ms tick at 40 MHz Fcy
- CAN: 125 kbps baud rate calculated against 40 MHz Fcy
- UART (temporary debug): U1BRG = 29, BRGH = 1, gives 333333 baud at 40 MHz Fcy

---

## 10. Harvard Architecture Considerations

The dsPIC33 is a Harvard architecture processor.  Program flash and data
SRAM are separate address spaces.  This has two major consequences:

### Flash cannot be read via data pointers

`memcpy` from a flash address, or dereferencing a flash pointer, does not
work.  Flash must be read using table-read instructions (TBLRDL/TBLRDH),
exposed by MCC as `FLASH_ReadWord24()`.

The bootloader library uses a `read_flash_bytes()` DI function pointer.
The dsPIC33 implementation uses `FLASH_ReadWord24()` to extract 3 data bytes
per 24-bit instruction word:

```
FLASH_ReadWord24(addr) returns uint32_t:
  bits[7:0]   = instruction byte 0
  bits[15:8]  = instruction byte 1
  bits[23:16] = instruction byte 2
  bits[31:24] = phantom byte (always 0, ignore)
PC address advances by 2 per instruction word.
```

On ARM platforms (STM32, MSPM0) flash is memory-mapped, so
`read_flash_bytes()` is simply `memcpy`.

### XC16 uintptr_t is 16 bits

`sizeof(void*) == 2` on dsPIC33.  Flash addresses above 0xFFFF are silently
truncated when cast through `uintptr_t`.  The bootloader library uses
`uintptr_t` internally, which caps the addressable application region to
about 54 KB.  `APP_FLASH_END` in `bootloader_drivers_openlcb.h` is capped
at `0x00FFFE` as a workaround.

The `-mlarge-data` flag does NOT fix this.  It widens data pointers but does
not allow reading program flash via a data pointer (Harvard restriction).

---

## 11. Building and Flashing

### First-time setup (both binaries)

1. Open both projects in MPLAB X
2. Build BasicNodeBootloader.X first
3. Program the bootloader via PICkit/ICD -- this writes 0x0000-0x7FFF
4. Build BasicNode.X
5. Program the application via PICkit/ICD -- this writes 0x8000+

After both are programmed, power-cycle the board.  The bootloader will
validate the application checksum and jump to it.

### Firmware update via OpenLCB Config Tool

1. Build the application and convert the hex to a binary file
2. In the Config Tool, connect to the node
3. Send a Freeze command to the firmware address space (0xEF)
4. The application drops back to the bootloader (see section 6)
5. The Config Tool sends the binary data
6. The bootloader writes it to flash, verifies the checksum, and reboots
7. The bootloader validates the new application and jumps to it

---

## 12. File Reference

### Bootloader project (BasicNodeBootloader.X)

| File | Purpose |
|------|---------|
| `bootloader_main.c` | Entry point; wires DI structs, calls `Bootloader_entry()` |
| `p33EP512MC506_BasicNodeBootloader.gld` | Linker script: program 0x200-0x7FFC |
| `application_drivers/bootloader_drivers_can.c/h` | CAN physical layer DI |
| `application_drivers/bootloader_drivers_openlcb.c/h` | OpenLCB/hardware DI |
| `mcc_generated_files/system.c` | MCC init (hand-edited: GIE disabled) |
| `mcc_generated_files/tmr2.c` | TMR2 ISR (hand-edited: VIVT redirect) |
| `mcc_generated_files/can1.c` | CAN1 ISR (hand-edited: VIVT redirect) |
| `mcc_generated_files/traps.c` | Trap ISRs (hand-edited: VIVT redirect) |

### Application project (BasicNode.X)

| File | Purpose |
|------|---------|
| `main.c` | Entry point; VIVT registration, library init, GIE enable |
| `p33EP512MC506_BasicNode.gld` | Linker script: program 0x8000+ |
| `reset_vector.s` | Places `GOTO __reset` at 0x8000 |
| `src/application_drivers/dspic33_olcb_drivers.c/h` | Platform drivers (init, timer, config mem) |
| `src/application_drivers/dspic33_can_drivers.c/h` | CAN driver (MCC API wrappers) |
| `src/application_callbacks/callbacks_config_mem.c` | Freeze handler (drop-back to bootloader) |
| `mcc_generated_files/system.c` | MCC init (hand-edited: GIE disabled) |
| `mcc_generated_files/can1.c` | CAN1 ISR (hand-edited: direct call to app handler) |

### Shared

| File | Purpose |
|------|---------|
| `shared/bootloader_shared_ram.h` | VIVT struct, magic flag, cached alias declarations |
| `shared/bootloader_shared_ram.c` | Variable definitions with persistent/address attributes |
