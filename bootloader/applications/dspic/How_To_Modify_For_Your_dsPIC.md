# How To Modify This Demo For Your dsPIC

This guide walks through every change needed to adapt the dsPIC33EP512MC506
bootloader demo to a different dsPIC33 or PIC24 part or board.  It covers both
the BasicNodeBootloader.X project (the bootloader itself) and the BasicNode.X
project (the application it boots).

The demo is built around two MPLAB X projects that share persistent SRAM
variables for handshake data.  Both projects need coordinated changes when
you retarget.

**Important:** The dsPIC is a Harvard architecture processor.  Program flash
and data SRAM are separate address spaces.  You cannot read flash with a data
pointer or use memcpy on flash addresses.  All flash reads go through
FLASH_ReadWord24() (table-read instructions).  This affects nearly every
flash-related function in the driver.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Understand the Flash Layout](#2-understand-the-flash-layout)
3. [Harvard Architecture Primer](#3-harvard-architecture-primer)
4. [Bootloader Linker Script](#4-bootloader-linker-script)
5. [Application Linker Script](#5-application-linker-script)
6. [Application Reset Vector](#6-application-reset-vector)
7. [Bootloader Startup Sequence](#7-bootloader-startup-sequence)
8. [Flash Constants](#8-flash-constants)
9. [App Header Offset](#9-app-header-offset)
10. [Flash Operations](#10-flash-operations)
11. [CAN Configuration](#11-can-configuration)
12. [GPIO and LEDs](#12-gpio-and-leds)
13. [Boot Button](#13-boot-button)
14. [Clock Configuration](#14-clock-configuration)
15. [Interrupt Architecture (VIVT)](#15-interrupt-architecture-vivt)
16. [MCC Hand-Edits](#16-mcc-hand-edits)
17. [GIE Ownership](#17-gie-ownership)
18. [Cleanup Before Handoff](#18-cleanup-before-handoff)
19. [Shared RAM (Handshake)](#19-shared-ram-handshake)
20. [Application app_header.c](#20-application-app_headerc)
21. [Node ID](#21-node-id)
22. [Post-Link Checksum Tool](#22-post-link-checksum-tool)
23. [Application Drop-Back (Freeze)](#23-application-drop-back-freeze)
24. [Building and Flashing](#24-building-and-flashing)
25. [Verification Checklist](#25-verification-checklist)

---

## 1. Prerequisites

- MPLAB X IDE and XC16 compiler installed
- The datasheet and family reference manual for your target dsPIC/PIC24 part
- The program flash memory map (page sizes, address range, config word locations)
- Know which ECAN module and pins your board uses
- MCC (MPLAB Code Configurator) for peripheral initialization

---

## 2. Understand the Flash Layout

Before changing any code, draw out the flash layout for your part.  You need
to decide four things:

1. **How much flash for the bootloader?**  It must fit in contiguous pages
   starting after the IVT.  The dsPIC33EP512MC506 demo uses 8 pages
   (PC 0x0000-0x3FFF).  Typical bootloader size is 4-8 pages.

2. **Where does the application start?**  Immediately after the bootloader
   pages.  This becomes APP_FLASH_START.

3. **Where does the application end?**  Leave room for reserved pages
   (node ID storage, config bits).  This becomes APP_FLASH_END.

4. **Where are the config words?**  dsPIC config words are at high flash
   addresses and must never be erased by the bootloader.

**dsPIC33EP512MC506 layout (for reference):**

```
PC Address     Contents
-----------    ------------------------------------------
0x000000       Reset vector (GOTO __reset) -- bootloader
0x000004       Hardware IVT (126 vectors)  -- bootloader
0x000200       Bootloader code begins
0x003FFC       Bootloader code ends (8 pages)
0x004000       Application entry (GOTO __reset via reset_vector.s)
0x004004       Application app_header (9 instructions, 36 binary bytes)
  ...          Application code
0x0547FC       Application code ends
0x054800       Checksum / Node ID page (reserved)
0x055000       Configuration bits page (hardware protected)
```

**Key constraint:** All addresses are PC (Program Counter) addresses.  Each
instruction occupies 2 PC addresses.  Flash erase is per-page (1024
instructions = 2048 PC units = 0x800).

---

## 3. Harvard Architecture Primer

This is the single biggest difference from ARM-based bootloaders (STM32,
MSPM0).  Every decision in the dsPIC bootloader is shaped by it.

### Separate address spaces

Program flash and data SRAM have independent address buses.  A data pointer
(`uint8_t *`) can only address SRAM.  Casting a flash PC address to a pointer
and dereferencing it reads garbage from SRAM, not flash.

### Table-read instructions

To read program flash, the CPU uses TBLRDL/TBLRDH instructions, exposed by
MCC as `FLASH_ReadWord24(pc_address)`.  This returns one 24-bit instruction
word as a uint32_t:

```
bits[7:0]   = instruction byte 0
bits[15:8]  = instruction byte 1
bits[23:16] = instruction byte 2
bits[31:24] = undefined (phantom byte -- always treat as 0x00)
```

### Phantom bytes

Each instruction is 24 bits (3 data bytes), but occupies 4 bytes in the
Intel HEX file and binary image (3 data + 1 phantom at 0x00).  The hardware
ignores the phantom byte on write and always reads it back as 0x00.  This
matters critically for CRC checksums -- see Step 21.

### PC addressing

Each instruction occupies 2 consecutive PC addresses (even addresses only).
The Intel HEX file uses byte addresses that are 2x the PC address:

```
hex_byte_address = PC_address * 2
```

The driver uses `BINARY_TO_PC_DIVISOR = 2` to convert between binary-image
byte offsets and PC addresses.

### XC16 uintptr_t is 16 bits

`sizeof(void*) == 2` on dsPIC33.  The bootloader library uses `uint32_t` for
all flash addresses to avoid truncation.  Never cast a flash address through
`uintptr_t` or `void*`.

---

## 4. Bootloader Linker Script

Create your linker script by copying the XC16 device linker support file
for your part and making these changes:

### MEMORY section

```
program (xr) : ORIGIN = 0x200, LENGTH = <bootloader_size - 0x200 - 0x4>
data    (a!xr): ORIGIN = 0x1000, LENGTH = <your_data_ram_size>
```

- **program ORIGIN** = 0x200 (after the IVT at 0x004-0x1FE).
- **program LENGTH** = bootloader pages only.  Subtract the IVT region and
  leave 4 bytes margin.
- **data** = your part's data RAM origin and length.

### Shared RAM variables

The handshake variables use `__attribute__((persistent, address(0xNNNN)))` to
pin them at fixed SRAM addresses.  Both projects must use the same addresses.
See Step 18.

---

## 5. Application Linker Script

Start from the XC16 device linker support file and make these changes:

### MEMORY section

```
program (xr) : ORIGIN = <APP_FLASH_START>, LENGTH = <app_region_size>
data    (a!xr): ORIGIN = 0x1000, LENGTH = <your_data_ram_size>
```

- **program ORIGIN** = APP_FLASH_START (e.g. 0x4000).
- **program LENGTH** = from APP_FLASH_START to the last app page, minus
  reserved pages.

### __CORESIDENT linker flag

Add `--defsym __CORESIDENT=1` to the application's XC16 linker options
(Project Properties -> xc16-ld -> Additional Options).  This suppresses the
default `.reset` section at PC 0x0000 so it does not collide with the
bootloader's reset vector.

### SECTIONS -- add .app_reset and .app_header

Before any `.text` sections, add:

```
  .app_reset __CODE_BASE :
  {
        KEEP (*(.resetvector));
  } >program

  .app_header __APP_HEADER_ADDR :
  {
        KEEP (*(.app_header));
  } >program
```

**Lesson learned:** The XC16 assembler's `address()` attribute on `.section`
directives does NOT reliably emit data into the hex file when the target
address falls inside a linker-script-defined memory region.  The
`reset_vector.s` file used `.section resetvector, address(0x4000), code`
but the linker silently dropped it -- the hex file had no data at PC 0x4000.
The bootloader read 0xFFFFFF (erased flash) and concluded no application was
present.  Mapping the section explicitly in the linker script is the only
reliable approach.

---

## 6. Application Reset Vector

**File:** `reset_vector.s`

The bootloader jumps to APP_FLASH_START to start the application.  A
`GOTO __reset` instruction must be there so execution reaches the C runtime
startup code.

```asm
    .section .resetvector, code
    goto __reset
```

The linker script maps `.resetvector` to the `.app_reset` output section at
`__CODE_BASE` (see Step 5).

**Without this file** the bootloader jumps to APP_FLASH_START and lands on
whatever the linker placed first -- which could be the app_header (all zeros)
or random code.  The application hangs or crashes silently.

---

## 7. Bootloader Startup Sequence

After any reset the bootloader runs this exact sequence:

```
1. _GIE = 0
2. SYSTEM_Initialize()            -- clock, pins, TMR2, DMA, CAN1 (GIE stays off)
3. is_bootloader_requested()      -- reads and consumes magic flag
4. initialize_hardware(request):
     a. Clear VIVT unconditionally
     b. Clear bootloader_request_flag unconditionally
     c. If NOT app drop-back: clear bootloader_cached_alias (stale garbage)
     d. If app drop-back: disable C1IE, clear C1IF, keep cached alias
     e. Enable _GIE = 1 last
5. Bootloader_init(request):
     - If valid app exists and NOT requested by app/button: jump to application
     - Otherwise: enter firmware update mode, start polling CAN
```

**Pitfall -- double call bug:** `is_bootloader_requested()` consumes (clears)
the magic flag on first read.  If called a second time it always returns
`BOOTLOADER_NOT_REQUESTED`.  The library calls it once in `Bootloader_entry()`
and passes the result as a parameter to `initialize_hardware()` and
`Bootloader_init()`.  If you add any pre-init logic, do not call
`is_bootloader_requested()` yourself -- use the value the library provides.

---

## 8. Flash Constants

**File:** `bootloader_drivers_openlcb.h`

Update every `#define` to match your flash map:

```c
#define BOOTLOADER_FLASH_START     0x000000UL
#define BOOTLOADER_FLASH_PAGES     8U          /* pages reserved for bootloader */
#define APP_FLASH_START            0x004000UL  /* PC addr: first app page */
#define APP_FLASH_END              0x0547FCUL  /* PC addr: last app page */
#define BINARY_TO_PC_DIVISOR       2U          /* hex addr = PC * 2 */
#define FLASH_WRITE_ALIGN          8U          /* bytes per WriteDoubleWord24 */
```

### How to find your values

- **APP_FLASH_START** = first page after the bootloader.  If the bootloader
  uses N pages and each page is P PC units, then
  `APP_FLASH_START = N * P`.
- **APP_FLASH_END** = last PC address of the last application page, before
  any reserved pages (node ID, config bits).
- **FLASH_WRITE_ALIGN** = 8 for all dsPIC33E/dsPIC33C parts (WriteDoubleWord24
  writes 2 instructions = 8 binary bytes per call).

### Page size

Check MCC's `flash.h` for:
```c
#define FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS  1024
#define FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS      2048  /* 0x800 */
```

These values vary by family.  dsPIC33EP uses 1024 instructions/page.
dsPIC33CK uses 512 or 1024 depending on the part.

---

## 9. App Header Offset

**File:** `bootloader_drivers_openlcb.h`

The app header is placed immediately after the `GOTO __reset` instruction at
APP_FLASH_START.  The GOTO is 1 instruction = 2 PC addresses = 4 binary bytes.

```c
#define APP_HEADER_OFFSET     0x00000004UL   /* PC units after APP_FLASH_START */
#define APP_HEADER_ADDRESS    ((APP_FLASH_START + APP_HEADER_OFFSET) * BINARY_TO_PC_DIVISOR)
```

APP_HEADER_ADDRESS is in binary-byte units (the coordinate system the library
uses for all flash addresses).  The driver functions convert back to PC
internally with `/ BINARY_TO_PC_DIVISOR`.

**The app header occupies 9 instructions (18 PC addresses, 36 binary bytes).**
Code placement starts after the header at `APP_FLASH_START + 0x4 + 0x12 =
APP_FLASH_START + 0x16` (PC) which is typically where `.text` begins.

---

## 10. Flash Operations

**File:** `bootloader_drivers_openlcb.c`

### Write

`FLASH_WriteDoubleWord24(pc_addr, instr0, instr1)` writes 2 instructions per
call.  Each instruction is assembled from 3 data bytes; the 4th (phantom) byte
in the binary image is discarded:

```c
uint32_t instr0 = byte[0] | (byte[1] << 8) | (byte[2] << 16);
uint32_t instr1 = byte[4] | (byte[5] << 8) | (byte[6] << 16);
FLASH_WriteDoubleWord24(dest_pc, instr0, instr1);
```

PC address advances by 4 per call (2 instructions x 2 PC units each).
Binary pointer advances by 8.

### Read

Two different read functions serve different purposes:

- **`read_flash_bytes`** (public, used by blank-flash check): 3 data bytes per
  instruction, no phantom byte.
- **`_read_flash_image_bytes`** (private, used by finalize_flash to read the
  header): 4 bytes per instruction (3 data + phantom 0x00), matching the
  binary image layout.

### Erase

```c
FLASH_Unlock(FLASH_UNLOCK_KEY);
FLASH_ErasePage(pc_address);
FLASH_Lock();
```

The unlock key is defined in MCC's `flash.h`.  Always lock after erase/write.

### Read-back verify

The demo verifies every write by reading back with `FLASH_ReadWord24` and
comparing.  This catches silent flash failures that can occur on parts with
marginal voltage or temperature conditions.

---

## 11. CAN Configuration

### ECAN module

dsPIC33 parts use the ECAN (Enhanced CAN) peripheral, configured through MCC.
The bootloader uses polling mode (no interrupts).

### MCC ECAN filter trick

To accept extended frames (29-bit identifiers, required for OpenLCB), append
a lowercase `x` to the filter/mask value in the MCC ECAN GUI:

```
0x0x
```

Without the trailing `x`, MCC generates `EXIDE = 0`, rejecting all
extended-frame traffic.

### DMA channels

ECAN requires DMA for both TX and RX.  The demo uses:
- DMA0 = CAN1 TX
- DMA1 = CAN1 RX

Configure these in MCC when setting up ECAN.

### Baud rate

OpenLCB uses 125 kbps.  The prescaler and time segments depend on your Fcy:

```
Baud = Fcy / (Prescaler * (1 + PropSeg + Seg1 + Seg2))
```

Demo: Fcy = 40 MHz, giving 125 kbps with MCC's default timing.

### CAN interrupt

The bootloader uses CAN in polling mode -- CAN interrupts are DISABLED in MCC.
The application enables CAN interrupts in its own driver init:

```c
C1INTEbits.RBIE = 1;   /* RX buffer interrupt sub-source */
IEC2bits.C1IE = 1;     /* CAN1 module interrupt enable */
```

There is no MCC API for toggling these bits.  Direct register writes are
required.

---

## 12. GPIO and LEDs

**File:** `bootloader_drivers_openlcb.c`, `set_status_led()`

Map the bootloader LED functions to whatever your board provides:

| Function | What it signals |
|----------|----------------|
| BOOTLOADER_LED_ACTIVE | Blinking while bootloader is running |
| BOOTLOADER_LED_WRITING | Flash write in progress |
| BOOTLOADER_LED_CSUM_ERROR | Checksum validation failed |
| BOOTLOADER_LED_REQUEST | Bootloader was explicitly requested |
| BOOTLOADER_LED_FRAME_LOST | CAN frame dropped |

If you only have one LED, drive it for the most important states (ACTIVE and
CSUM_ERROR).  If you have no LEDs, make `set_status_led()` a no-op.

Use MCC Pin Manager to configure GPIO outputs and generate the SetHigh/SetLow
macros in `pin_manager.h`.

---

## 13. Boot Button

**File:** `bootloader_drivers_openlcb.c`, `is_bootloader_requested()`

The demo reads IO_BOOT (RA10, active-low with external pull-up) to force
bootloader mode at power-on:

```c
if (!IO_BOOT_GetValue()) {
    return BOOTLOADER_REQUESTED_BY_BUTTON;
}
```

Change the GPIO pin and polarity for your board.  Configure the pin as a
digital input in MCC Pin Manager.

If your board has no dedicated button, you can remove this check entirely.
The app-to-bootloader magic RAM handshake still works without it.

---

## 14. Clock Configuration

Both projects share the same clock configuration through MCC:

- External crystal frequency (demo: 16 MHz)
- PLL settings: PLLPRE, PLLFBD, PLLPOST
- Target Fosc and Fcy

**Demo values:**
```
16 MHz crystal -> PLL -> Fosc = 80 MHz, Fcy = 40 MHz
PLL registers: PLLPRE /4, PLLFBD x40, PLLPOST /2
```

### Derived peripheral settings

When you change the clock, recalculate:

- **TMR2 period register (PR2):** For a 100 ms tick at your Fcy.
  Demo: PR2 = 15624, prescaler 1:256.
- **CAN baud rate:** Must remain 125 kbps.  Recalculate prescaler/segments.
- **UART baud (if used for debug):** Recalculate U1BRG.
  Demo: U1BRG = 29, BRGH = 1, gives ~333333 baud at 40 MHz Fcy.
  Useful during bringup; disable the UART in `cleanup_before_handoff()`
  when done.

---

## 15. Interrupt Architecture (VIVT)

### The problem

The dsPIC33 hardware IVT is at a fixed flash address (0x0004) owned by the
bootloader.  After `jump_to_application()` the application runs but ALL
interrupts still route through the bootloader's ISR stubs.

### The solution -- Virtual Interrupt Vector Table

The bootloader's ISR stubs check function pointers in
`bootloader_vivt_jumptable` (persistent SRAM).  If non-NULL, the stub calls
through the pointer to the application's handler.  If NULL, the stub is a safe
no-op (or halts for traps).

The application registers its handlers during startup before enabling GIE:

```c
bootloader_vivt_jumptable.timer_2_handler = MyApp_t2_handler;
bootloader_vivt_jumptable.can1_handler    = MyApp_c1_handler;
/* ... trap handlers ... */
_GIE = 1;
```

### What to change for your part

The VIVT struct in `bootloader_shared_ram.h` must have a slot for every
interrupt the application uses.  If your application needs additional
interrupts (UART, SPI, ADC, etc.), add corresponding function pointer slots
to the VIVT struct and corresponding redirect stubs in the bootloader's MCC
ISR files.

### Two call paths

Each interrupt handler in the application has two call paths:

1. **Standalone (no bootloader):** The MCC ISR calls the handler directly.
2. **Bootloaded:** The bootloader's ISR stub reads the VIVT pointer and calls
   the handler indirectly.

Both paths call the same function.  The VIVT handler must clear its own
interrupt flag (e.g. `IFS0bits.T2IF = 0`) because the MCC ISR is not in the
call path when bootloaded.

### Why not use MCC's TMR2_InterruptHandler pointer?

MCC's `TMR2_InterruptHandler` is a plain `.data` global.  When the bootloader
jumps to the application, the app's CRT0 startup re-initializes `.data` and
`.bss`, clobbering this pointer back to NULL before `main()` runs.  The VIVT
slots survive because they use `__attribute__((persistent))` -- the CRT0
startup code does not zero persistent variables.

---

## 16. MCC Hand-Edits

Both projects use MCC-generated driver files with specific hand-edits.  If
MCC regenerates any of these files, the edits must be reapplied.  All edits
are marked with `/* HAND-EDIT */` comments.

### Bootloader project (BasicNodeBootloader.X)

| File | Edit | Purpose |
|------|------|---------|
| `system.c` | Comment out `INTERRUPT_GlobalEnable()` | Bootloader owns GIE lifecycle |
| `tmr2.c` | Add VIVT redirect in `_T2Interrupt` | Forward TMR2 to application |
| `can1.c` | Add VIVT redirect in `_C1Interrupt` | Forward CAN1 to application |
| `traps.c` | Add VIVT redirect in each trap handler | Forward traps to application |

### Application project (BasicNode.X)

| File | Edit | Purpose |
|------|------|---------|
| `system.c` | Comment out `INTERRUPT_GlobalEnable()` | main() owns GIE lifecycle |
| `can1.c` | Direct call to app handler in `_C1Interrupt` | MCC has no weak callback for CAN |

The application's `tmr2.c` does NOT need a hand-edit.  MCC provides a weak
`TMR2_CallBack()` that the application overrides.

### MCC regeneration behavior

MCC does NOT silently overwrite modified files.  When a conflict is detected
it prompts you to keep your changes or accept the new version.

**Lesson learned:** The `HAND-EDIT` comments make it clear what to reapply if
you accept a regeneration.  Keep them short and searchable.

---

## 17. GIE Ownership

Global Interrupt Enable (_GIE) must be carefully managed.  The rule is:
**whoever creates a safe state enables GIE last.**

```
SYSTEM_Initialize()                -- does NOT touch GIE (hand-edited)
Bootloader: initialize_hardware()  -- clears VIVT, sets up CAN, enables GIE last
Bootloader: cleanup_before_handoff() -- disables GIE before jumping to app
Application: main()                -- registers VIVT, inits peripherals, enables GIE last
Application: freeze()              -- disables GIE, clears VIVT, resets into bootloader
```

**Why this matters:** If GIE is enabled before the VIVT is cleared (bootloader)
or populated (application), a pending interrupt will vector through a NULL or
stale function pointer and crash.

**Lesson learned:** Use `_GIE = 0` not `DISI`.  The `DISI` instruction only
blocks priority 0-6 interrupts for a cycle count.  Traps (priority 7) can
still fire.  `_GIE = 0` is the hard global disable that blocks everything.

---

## 18. Cleanup Before Handoff

**File:** `bootloader_drivers_openlcb.c`, `cleanup_before_handoff()`

This function tears down the bootloader's state before jumping to the
application.  On dsPIC the minimum is:

```c
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {
    _GIE = 0;
}
```

The VIVT is already cleared by `initialize_hardware()`.  The application's
`SYSTEM_Initialize()` will reconfigure all peripherals from scratch.

If your bootloader enables additional peripherals (UART for debug, etc.),
disable them here.

---

## 19. Shared RAM (Handshake)

**Files:** `shared/bootloader_shared_ram.h`, `shared/bootloader_shared_ram.c`

The handshake uses persistent variables pinned to fixed SRAM addresses:

```c
volatile uint16_t __attribute__((persistent, address(0x1000)))
    bootloader_cached_alias;

volatile uint32_t __attribute__((persistent, address(0x1002)))
    bootloader_request_flag;

volatile bootloader_vivt_jumptable_t __attribute__((persistent, address(0x1006)))
    bootloader_vivt_jumptable;
```

### What to change

- **Addresses:** Must be within your part's data RAM and must not collide with
  `.bss`, `.data`, heap, or stack.  The demo uses the very beginning of data
  RAM (0x1000) which the linker respects because of the `address()` attribute.
- **VIVT size:** Add or remove function pointer slots depending on which
  interrupts your application uses.
- **Both projects must compile the same files** so the addresses and struct
  layout match exactly.

**Lesson learned -- stale cached alias:** After programming with PICkit/ICD,
SRAM contains garbage.  If `bootloader_cached_alias` happens to be non-zero,
the CAN state machine skips alias negotiation and uses a garbage value --
often 0xFFFF, which overflows the 12-bit alias field and corrupts the CAN
identifier.  The fix: `initialize_hardware()` clears `bootloader_cached_alias`
unconditionally on every reset EXCEPT `BOOTLOADER_REQUESTED_BY_APP`.

---

## 20. Application app_header.c

**File:** `BasicNode.X/app_header.c`

This file places a zero-initialized struct in the `.app_header` linker
section.  On dsPIC, it uses `space(prog)` to place const data in program
flash rather than the PSV window:

```c
typedef struct {
    uint8_t raw[27];
} app_header_image_t;

__attribute__((section(".app_header"), space(prog)))
__attribute__((used))
const app_header_image_t app_header = {{ 0U }};
```

### Why 27 bytes?

On dsPIC with `space(prog)`, XC16 packs 3 data bytes per instruction word.
The bootloader_app_header_t is 36 binary bytes = 9 instructions.  9
instructions x 3 data bytes = 27 bytes.  The phantom byte per instruction
is implicit in the `space(prog)` encoding.

### Size guard

```c
typedef char _check_size[(sizeof(app_header_image_t) == 27U) ? 1 : -1];
```

If the library's `bootloader_app_header_t` changes size, update the array
size to `ceil(new_binary_size / 4) * 3`.

---

## 21. Node ID

**File:** `bootloader_drivers_openlcb.c`, `get_persistent_node_id()`

The demo returns a hardcoded node ID for bringup:

```c
return 0x0501010107AAULL;
```

For production, read the node ID from a reserved flash page or use the
device unique ID.  Every OpenLCB node must have a globally unique 48-bit
node ID.

---

## 22. Post-Link Checksum Tool

After building the application hex, run `hex2bin.py` to produce a
checksum-patched binary:

```bash
python3 hex2bin.py \
    --arch dspic \
    --start 0x4000 \
    --end 0x54FFC \
    --app-header-offset 8 \
    --trim \
    BasicNode.X.production.hex \
    BasicNode.dspic.production.boot.bin
```

### Arguments explained

- **`--arch dspic`** -- enables PC-to-hex address conversion (x2) and
  phantom byte correction for CRC computation.
- **`--start 0x4000`** -- APP_FLASH_START in PC addresses.
- **`--end 0x54FFC`** -- caps the binary at the application flash boundary
  to exclude config words at high addresses.  Without this the binary
  inflates to the full flash size (667KB vs 108KB).
- **`--app-header-offset 8`** -- binary byte offset of the app header.
  This is `APP_HEADER_OFFSET * BINARY_TO_PC_DIVISOR = 0x4 * 2 = 8`.
- **`--trim`** -- strips trailing 0xFF fill bytes (rounded to 8-byte
  boundary) to reduce upload time.

### Phantom byte CRC correction

**This is the most critical dsPIC-specific behavior.**

Regions not present in the hex file are filled with 0xFF, including the
phantom byte positions (indices 3, 7, 11, ...).  But the dsPIC hardware
always reads the phantom byte back as 0x00.  If the tool computes CRC over
raw 0xFF phantom bytes, it will not match what the on-target CRC sees.

hex2bin.py automatically corrects this when `--arch dspic` is specified:
every 4th byte is forced to 0x00 before CRC computation.  The output binary
is NOT modified -- only the CRC computation uses the corrected view.

**Lesson learned:** This mismatch only manifests in fill regions (erased flash
or gaps in the hex file).  Programmed instructions always have phantom = 0x00
in the hex file.  The first 2 instructions at APP_FLASH_START (the GOTO and
padding) were the most common failure point because the hex file originally
had no data there and fill put 0xFF in the phantom positions.

Add your target to `create_boot_images.sh` in `bootloader/tools/hex2bin/`.

### Development bypass (NO_CHECKSUM)

During bringup you can skip CRC validation entirely by uncommenting
`#define NO_CHECKSUM` in `bootloader_types.h`.  This lets you program the
application via PICkit/ICD without running hex2bin.py first.  The bootloader
will jump to the application regardless of the header contents.

Remove the define before production use.

---

## 23. Application Drop-Back (Freeze)

When the OpenLCB Config Tool sends a Freeze command targeting the firmware
address space (0xEF), the application executes this sequence:

```
Step 1:  _GIE = 0              Disable all interrupts
         C1INTEbits.RBIE = 0   Disable CAN RX interrupt source
         IEC2bits.C1IE = 0     Disable CAN module interrupt

Step 2:  Clear all VIVT slots  Prevent stale calls into dead app code

Step 3:  Cache CAN alias       bootloader_cached_alias = node->alias
                               (bootloader reuses it, skips 200ms negotiation)

Step 4:  Set magic flag        bootloader_request_flag = 0xB00710AD
                               (tells bootloader this is a deliberate drop-back)

Step 5:  asm("RESET")          Software reset, SRAM preserved
```

**Order matters:** VIVT must be cleared (step 2) BEFORE the magic flag is
written (step 4).  If a trap fired between those two writes, the bootloader
could see the flag but find stale VIVT pointers.

---

## 24. Building and Flashing

### First-time setup (both binaries)

1. Open both projects in MPLAB X
2. Build BasicNodeBootloader.X first
3. Program the bootloader via PICkit/ICD -- this writes 0x0000-0x3FFF
4. Build BasicNode.X
5. Run `create_boot_images.sh` (or hex2bin.py manually) to produce the .bin
6. Upload the .bin via the OpenLCB Config Tool, or program via PICkit/ICD

### Firmware update via OpenLCB Config Tool

1. Build the application and run hex2bin.py to produce the .boot.bin
2. In the Config Tool, connect to the node
3. Send Freeze to firmware space (0xEF)
4. The application drops back to the bootloader (see Step 22)
5. Upload the binary data
6. The bootloader writes to flash, verifies checksums, and reboots
7. The bootloader validates the application and jumps to it

**Lesson learned:** Make sure you load the correct .bin file for the target
platform.  Loading an STM32 binary onto a dsPIC board will write garbage to
flash and the checksum will fail.  The filenames include the platform:
`BasicNode.dspic.production.boot.bin`.

---

## 25. Verification Checklist

Work through this list after making all changes:

### Linker scripts
- [ ] Bootloader program region covers bootloader pages only
- [ ] Application program ORIGIN = APP_FLASH_START
- [ ] Application has `--defsym __CORESIDENT=1` in linker options
- [ ] `.app_reset` section at `__CODE_BASE` in application linker script
- [ ] `.app_header` section at correct PC address after `.app_reset`
- [ ] Neither linker script was auto-generated by MCC without edits

### Reset vector
- [ ] `reset_vector.s` uses `.section .resetvector, code` (not `address()`)
- [ ] Linker map shows `.app_reset` section at APP_FLASH_START
- [ ] Hex file has non-0xFF data at hex address APP_FLASH_START * 2

### Flash constants
- [ ] APP_FLASH_START matches application linker script program ORIGIN
- [ ] APP_FLASH_END excludes reserved pages (node ID, config bits)
- [ ] APP_HEADER_OFFSET = 0x4 (after the 1-instruction GOTO)
- [ ] BINARY_TO_PC_DIVISOR = 2
- [ ] FLASH_WRITE_ALIGN = 8 (WriteDoubleWord24)
- [ ] Page size constants match MCC's flash.h defines

### Flash operations
- [ ] write_flash_bytes extracts 3 data bytes per instruction, skips phantom
- [ ] Read-back verify after each WriteDoubleWord24
- [ ] FLASH_Unlock/FLASH_Lock around every erase and write

### CAN
- [ ] ECAN configured with MCC, DMA channels assigned
- [ ] Filter uses `0x0x` suffix for extended frame acceptance
- [ ] Baud rate = 125 kbps (verify with scope or analyzer)
- [ ] CAN interrupt disabled in MCC (bootloader uses polling)

### VIVT
- [ ] VIVT struct has slots for all interrupts the application uses
- [ ] Bootloader ISR stubs check VIVT and redirect (hand-edits in place)
- [ ] Application registers VIVT handlers before enabling GIE
- [ ] `system.c` hand-edited in BOTH projects (GIE disabled)

### Shared RAM
- [ ] Both projects compile shared/bootloader_shared_ram.c
- [ ] Variable addresses match between bootloader and application
- [ ] `initialize_hardware()` clears stale cached_alias on non-app resets
- [ ] Persistent attribute prevents CRT0 from zeroing handshake variables

### Checksum
- [ ] app_header.c in application project with `space(prog)` attribute
- [ ] app_header size = 27 bytes (9 instructions x 3 data bytes)
- [ ] hex2bin uses `--arch dspic` (enables phantom byte CRC correction)
- [ ] hex2bin `--start` and `--end` match your flash constants
- [ ] hex2bin `--app-header-offset` = APP_HEADER_OFFSET * BINARY_TO_PC_DIVISOR
- [ ] hex2bin produces non-zero checksums (not all 0x0000)
- [ ] Bootloader boots the patched image without checksum error LED

### Integration
- [ ] Bootloader fits in allocated pages (check .map file)
- [ ] Application .map shows `.app_reset` and `.app_header` at expected addresses
- [ ] Boot button forces bootloader mode
- [ ] Application can drop back to bootloader (magic RAM handshake)
- [ ] After bootload + reboot, bootloader jumps to application (not back to bootloader)
- [ ] Checksum error LED lights when image is corrupt or unpatched

### File reference

| File | Purpose |
|------|---------|
| `bootloader_main.c` | Entry point; wires DI structs, calls Bootloader_entry() |
| `p33EPxxx_BasicNodeBootloader.gld` | Bootloader linker script |
| `p33EPxxx_BasicNode.gld` | Application linker script |
| `application_drivers/bootloader_drivers_can.c/h` | CAN physical layer DI |
| `application_drivers/bootloader_drivers_openlcb.c/h` | OpenLCB/hardware DI |
| `reset_vector.s` | GOTO __reset at APP_FLASH_START |
| `app_header.c` | 27-byte zero-init struct for CRC patching |
| `shared/bootloader_shared_ram.h/c` | VIVT, magic flag, cached alias |
