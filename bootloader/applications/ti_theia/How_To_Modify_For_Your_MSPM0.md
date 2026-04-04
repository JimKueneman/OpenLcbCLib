# How To Modify This Demo For Your MSPM0

This guide walks through every change needed to adapt the MSPM0G3507
LaunchPad bootloader demo to a different MSPM0 part or board. It covers both
the BasicNodeBootloader project (the bootloader itself) and the BasicNode
project (the application it boots).

The demo is built around two CCS Theia projects that share a common RAM region
for handshake data. Both projects need coordinated changes when you retarget.

**Important:** The MSPM0 is an ARM Cortex-M0+ with a Von Neumann memory
architecture -- flash and SRAM share one address bus. Flash can be read with
ordinary data pointers (memcpy). This is simpler than the dsPIC Harvard
architecture but the flash controller has its own quirks: 8-byte write
alignment, mandatory sector unprotection, and a requirement to disable
interrupts during erase/write operations.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Understand the Flash Layout](#2-understand-the-flash-layout)
3. [Create the CCS Theia Projects](#3-create-the-ccs-theia-projects)
4. [Bootloader Linker Script](#4-bootloader-linker-script)
5. [Application Linker Script](#5-application-linker-script)
6. [Flash Constants](#6-flash-constants)
7. [App Header Offset](#7-app-header-offset)
8. [Flash Operations](#8-flash-operations)
9. [CAN Configuration (MCAN)](#9-can-configuration-mcan)
10. [GPIO and LEDs](#10-gpio-and-leds)
11. [Boot Button](#11-boot-button)
12. [Clock and SysConfig](#12-clock-and-sysconfig)
13. [SysTick as 100 ms Timer](#13-systick-as-100-ms-timer)
14. [Cleanup Before Handoff](#14-cleanup-before-handoff)
15. [Shared RAM (Handshake)](#15-shared-ram-handshake)
16. [Application app_header.c](#16-application-app_headerc)
17. [Node ID](#17-node-id)
18. [Post-Link Checksum Tool](#18-post-link-checksum-tool)
19. [Application Drop-Back (Freeze)](#19-application-drop-back-freeze)
20. [Building and Flashing](#20-building-and-flashing)
21. [Verification Checklist](#21-verification-checklist)

---

## 1. Prerequisites

- TI CCS Theia IDE installed
- The datasheet and technical reference manual for your target MSPM0 part
- The flash memory map (sector size, total size, address range)
- Know which MCAN module and pins your board uses
- SysConfig for peripheral initialization

---

## 2. Understand the Flash Layout

Before changing any code, draw out the flash layout for your part. You need
to decide three things:

1. **How much flash for the bootloader?** It must fit in contiguous 1KB
   sectors starting at flash base. The MSPM0G3507 demo uses 15KB (15
   sectors, 0x0000-0x3BFF). Typical bootloader size is 12-16KB.

2. **Where does the application start?** Immediately after the bootloader
   sectors. This becomes APP_FLASH_START.

3. **Do you need a reserved region?** The demo reserves the last 1KB sector
   for node ID storage. You may not need this, or you may want it somewhere
   else.

**MSPM0G3507 layout (for reference):**

```
Address        Contents
-----------    ------------------------------------------
0x00000000     Bootloader vector table + code (15KB)
0x00003C00     Application vector table (48 entries, 192 bytes)
0x00003CC0     Application app_header (36 bytes)
  ...          Application code
0x0001FBFF     Application ends (113KB total)
0x0001FC00     Node ID sector (1KB, production-programmed)
0x0001FFFF     End of 128KB flash
```

**Key constraint:** All MSPM0G3507 flash sectors are uniform 1KB (1024 bytes).
This makes the sector table trivial -- any address can be sector-aligned with
a bitmask. If your MSPM0 variant has a different sector size, update
FLASH_SECTOR_SIZE accordingly.

---

## 3. Create the CCS Theia Projects

Start with two CCS Theia projects generated from your MSPM0 part:

- **BasicNodeBootloader** -- configure MCAN, GPIO for LEDs and boot button,
  SysTick. No other peripherals needed.
- **BasicNode** -- your application project.

Both projects use SysConfig (`.syscfg`) for peripheral initialization. SysConfig
generates `ti_msp_dl_config.c` and `ti_msp_dl_config.h` in the `Debug/`
folder. The bootloader's `bootloader_main.c` includes these generated files
via `#include "../Debug/ti_msp_dl_config.h"`.

**Lesson learned -- CCS hex output drops zero-filled regions:** TI CCS Theia's
default hex output omits zero-filled regions from the hex file, which corrupts
the app_header (all zeros become absent). The post-link checksum tool then
fails because the header region is missing from the hex. The fix is to use
`tiarmobjcopy` to generate the Intel HEX from the ELF:

```bash
tiarmobjcopy -O ihex BasicNode.out BasicNode.objcopy.hex
```

The `create_boot_images.sh` script does this automatically. See Step 18.

---

## 4. Bootloader Linker Script

**File:** `BasicNodeBootloader/device_linker.cmd`

CCS Theia uses `.cmd` linker scripts. Create yours by starting from the
TI-generated device linker script and making these changes.

### Required directives at the top

The first two lines of every `.cmd` file must be:

```
-uinterruptVectors
--stack_size=512
```

`-uinterruptVectors` forces the linker to pull in the vector table even if
no code references it directly. `--stack_size` sets the minimum stack
allocation. Adjust the stack size if your bootloader uses deep call chains
or large stack buffers.

### MEMORY section

```
MEMORY
{
    BOOT_FLASH      (RX)  : origin = 0x00000000, length = <bootloader_size>
    APP_FLASH       (RX)  : origin = <app_start>,  length = <app_size>
    NODEID_FLASH    (R)   : origin = <nodeid_addr>, length = 0x00000400
    SRAM            (RWX) : origin = 0x20200000, length = <ram_size - 16>
    SHARED_NOINIT   (RW)  : origin = <ram_end - 16>, length = 0x00000010
    BCR_CONFIG      (R)   : origin = 0x41C00000, length = 0x000000FF
    BSL_CONFIG      (R)   : origin = 0x41C00100, length = 0x00000080
}
```

- **BOOT_FLASH LENGTH** = bootloader region only (e.g. 15KB).
- **APP_FLASH** = application region. Defined but not used by the bootloader
  linker -- it is here for documentation and so the linker symbols resolve.
- **SRAM LENGTH** = total SRAM minus 16 bytes for the shared region.
- **SHARED_NOINIT** = the last 16 bytes of SRAM. Must be identical in both
  linker scripts.

### SECTIONS

All bootloader code goes into BOOT_FLASH:

```
SECTIONS
{
    .intvecs:   > 0x00000000
    .text   : palign(8) {} > BOOT_FLASH
    .const  : palign(8) {} > BOOT_FLASH
    .cinit  : palign(8) {} > BOOT_FLASH
    .pinit  : palign(8) {} > BOOT_FLASH
    .rodata : palign(8) {} > BOOT_FLASH
    .ARM.exidx    : palign(8) {} > BOOT_FLASH
    .init_array   : palign(8) {} > BOOT_FLASH
    .binit        : palign(8) {} > BOOT_FLASH
    .TI.ramfunc   : load = BOOT_FLASH, palign(8), run=SRAM, table(BINIT)

    .vtable :   > SRAM
    .args   :   > SRAM
    .data   :   > SRAM
    .bss    :   > SRAM
    .noinit : type = NOINIT {} > SHARED_NOINIT
    .sysmem :   > SRAM
    .stack  :   > SRAM (HIGH)

    .BCRConfig  : {} > BCR_CONFIG
    .BSLConfig  : {} > BSL_CONFIG
}
```

### .TI.ramfunc -- flash operations run from RAM

The MSPM0 flash controller cannot be read while an erase or program operation
is in progress. TI's DriverLib flash functions are annotated with
`__attribute__((section(".TI.ramfunc")))` so they execute from SRAM. The
`table(BINIT)` directive tells the C startup code to copy them from flash to
SRAM before `main()` runs.

**Lesson learned:** If you remove the `.TI.ramfunc` line or change the load
region, flash erase/write calls will hard fault because the CPU tries to fetch
instructions from flash while the flash controller is busy.

### Linker symbols

Add these at the end for code that needs to locate flash regions:

```
__app_start        = 0x00003C00;
__app_end          = 0x0001FBFF;
__app_header_addr  = 0x00003CC0;
__nodeid_addr      = 0x0001FC00;
```

---

## 5. Application Linker Script

**File:** `BasicNode/device_linker.cmd`

Start from the TI-generated device linker script and make these changes:

### MEMORY section

```
MEMORY
{
    APP_FLASH       (RX)  : origin = <app_start>,    length = <app_size>
    NODEID_FLASH    (R)   : origin = <nodeid_addr>,  length = 0x00000400
    SRAM            (RWX) : origin = 0x20200000, length = <ram_size - 16>
    SHARED_NOINIT   (RW)  : origin = <ram_end - 16>, length = 0x00000010
}
```

- **APP_FLASH ORIGIN** = where the application starts (right after the
  bootloader sectors).
- **SHARED_NOINIT** = same address and size as the bootloader script.
- **No BOOT_FLASH** -- the application must not write to bootloader flash.

### SECTIONS -- add .app_header

The vector table goes at the application start address, and the app_header
goes immediately after it:

```
SECTIONS
{
    .intvecs:   > <app_start>

    .app_header : fill = 0x00000000 {} > <app_header_address>

    .text   : palign(8) {} > APP_FLASH
    /* ... remaining sections same as bootloader ... */
    .noinit : type = NOINIT {} > SHARED_NOINIT
}
```

The `fill = 0x00000000` ensures the header region is zero-filled even if the
hex output omits it. The `type = NOINIT` on `.noinit` prevents the C startup
from zeroing the shared RAM.

The application also needs `.TI.ramfunc` with load/run split if it performs
any flash operations (e.g. writing configuration data). The syntax is
identical to the bootloader linker script:

```
.TI.ramfunc   : load = APP_FLASH, palign(8), run=SRAM, table(BINIT)
```

**Lesson learned:** The `.app_header` section must appear BEFORE `.text` in
the SECTIONS block. CCS Theia's linker places sections in the order they
appear. If `.text` comes first, the linker may place code at the app_header
address and the header ends up somewhere else.

---

## 6. Flash Constants

**File:** `bootloader_drivers_openlcb.h`

Update every `#define` to match your flash map:

```c
#define BOOTLOADER_FLASH_START     0x00000000U
#define BOOTLOADER_FLASH_SIZE      0x00003C00U       /* your bootloader size */
#define APP_FLASH_START            0x00003C00U       /* first byte after bootloader */
#define APP_FLASH_END              0x0001FBFFU       /* last byte of app region */
#define NODEID_FLASH_ADDRESS       0x0001FC00U       /* or remove if not used */
#define FLASH_SECTOR_SIZE          1024U             /* uniform on MSPM0G3507 */
#define FLASH_WRITE_ALIGN          8U                /* 64-bit word with ECC */
```

### How to find your values

- **APP_FLASH_START** = `BOOTLOADER_FLASH_START + BOOTLOADER_FLASH_SIZE`.
- **APP_FLASH_END** = last byte before any reserved sectors (node ID, etc.).
- **FLASH_SECTOR_SIZE** = check the device datasheet. All MSPM0G35xx parts
  use 1KB sectors. MSPM0L parts may differ.
- **FLASH_WRITE_ALIGN** = 8 for all MSPM0 parts (64-bit write with ECC).

---

## 7. App Header Offset

**File:** `bootloader_drivers_openlcb.h`

The app header (bootloader_app_header_t, 36 bytes) must be placed after the
vector table so they do not overlap.

```c
#define APP_HEADER_OFFSET     0x000000C0U
#define APP_HEADER_ADDRESS    (APP_FLASH_START + APP_HEADER_OFFSET)
```

**How to calculate the offset:**

1. Count the interrupt vectors for your part. The Cortex-M0+ vector table
   has 16 system entries plus device-specific IRQs. MSPM0G3507 has 48
   total entries.
2. Multiply by 4 bytes per entry: 48 x 4 = 192 bytes = 0xC0.
3. That gives APP_HEADER_OFFSET = 0xC0.

Other MSPM0 parts may have fewer or more interrupt vectors. Check the
startup .s file or the `IRQn_Type` enum in the device header.

---

## 8. Flash Operations

**File:** `bootloader_drivers_openlcb.c`

### Write alignment: 8 bytes with ECC

The MSPM0 flash controller requires writes to be aligned to 8-byte (64-bit)
boundaries. The `DL_FlashCTL_programMemoryFromRAM64WithECCGenerated()` API
writes exactly 8 bytes and generates ECC in hardware.

**Lesson learned -- Cortex-M0+ unaligned access:** The Cortex-M0+ core does
NOT support unaligned memory access. Casting a `uint8_t*` to `uint32_t*` will
hard fault if the pointer is not naturally 4-byte aligned. The driver copies
incoming data through a stack-local `uint32_t aligned_data[2]` buffer using
`memcpy()` to guarantee alignment:

```c
uint32_t aligned_data[2];
memcpy(aligned_data, source_data, FLASH_WRITE_ALIGN);
DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(FLASHCTL, dest, aligned_data);
```

Trailing data fewer than 8 bytes is padded with 0xFF (erased flash value).

### Interrupts must be disabled during erase/write

**Lesson learned:** The MSPM0 flash controller runs erase/program operations
from a RAM trampoline (`.TI.ramfunc`). If an interrupt fires during the
operation and the ISR tries to execute code from flash, the CPU will hard
fault because the flash controller is busy and cannot service read requests.

Both `erase_flash_page()` and `write_flash_bytes()` bracket the entire
operation with `__disable_irq()` / `__enable_irq()`. Every early-return
error path must call `__enable_irq()` before returning.

**Contrast with other platforms:**
- **STM32F407:** The flash controller handles concurrent reads (the CPU
  stalls until the operation completes), so interrupt disable is not
  strictly required.
- **ESP32:** Flash operations go through the partition API which handles
  synchronisation internally.
- **dsPIC33:** Flash operations use the NVM controller with WREN bit;
  MCC's `FLASH_WriteRow24()` handles the unlock sequence internally.

### Sector unprotection before every operation

**Lesson learned:** The MSPM0 flash controller requires explicit sector
unprotection before any erase or program operation:

```c
DL_FlashCTL_unprotectSector(FLASHCTL, address, DL_FLASHCTL_REGION_SELECT_MAIN);
```

Forgetting this call results in a silent failure -- the erase/write command
completes but flash contents are unchanged.

### Clear status before every operation

Call `DL_FlashCTL_executeClearStatus(FLASHCTL)` before each operation to
reset the flash controller's status register. Stale error flags from a
previous operation can cause the next operation to report failure even if
it would have succeeded.

### Read-back verification

Every 8-byte write is read back and compared. The flash controller returning
success only means the command was accepted, not that the bits landed
correctly. Flash can fail silently due to wear, voltage droop, or marginal
cells.

### Flash write deduplication (CT retry safety)

**Lesson learned -- MSPM0 flash hangs on double-write:** Writing to an
already-programmed flash address causes undefined behavior on the MSPM0 flash
controller. In testing, it hangs permanently. This matters because the Config
Tool retries datagram writes -- a retry re-sends the same data for an address
that was already programmed.

The statemachine reads current flash content before writing. If the data
already matches, the write is skipped. This check happens BEFORE any page
erase -- if the address is page-aligned, erasing first would destroy the data
we need to compare against.

### Erase

```c
DL_FlashCTL_executeClearStatus(FLASHCTL);
DL_FlashCTL_unprotectSector(FLASHCTL, address, DL_FLASHCTL_REGION_SELECT_MAIN);
DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, address, DL_FLASHCTL_COMMAND_SIZE_SECTOR);
DL_FlashCTL_waitForCmdDone(FLASHCTL);
```

### Sector table

MSPM0G3507 has uniform 1KB sectors. The page info function is a simple
bitmask -- no lookup table needed:

```c
*page_start        = address & ~((uint32_t)(FLASH_SECTOR_SIZE - 1));
*page_length_bytes = FLASH_SECTOR_SIZE;
```

If your MSPM0 variant has non-uniform sectors (unlikely but check the
datasheet), you will need a lookup table like the STM32 demo uses.

---

## 9. CAN Configuration (MCAN)

### MCAN peripheral

MSPM0G3507 uses the MCAN (M_CAN / Bosch M_CAN) peripheral, configured
through SysConfig. The bootloader uses polling mode -- no MCAN interrupts
are enabled.

### SysConfig MCAN settings

In the `.syscfg` file, configure MCAN with:

- **Nominal baud rate:** 125 kbps (required by OpenLCB)
- **RX FIFO 1** enabled with at least 1 element
- **TX buffer** with 2 elements
- **Standard filter** accepting all extended frames
- **Interrupts disabled** in SysConfig (bootloader polls)

The demo uses MCAN0 on the default LaunchPad pins (PA13 RX / PA12 TX).

### TX buffer ordering

The MCAN peripheral has dedicated TX buffers (not a priority-based mailbox
like bxCAN on STM32). The demo uses 2 TX buffers. The `try_send_frame()`
function checks `DL_MCAN_getTxBufReqPend()` to find a free buffer:

```c
uint32_t tx_req_pending = DL_MCAN_getTxBufReqPend(CANFD0);
if (tx_req_pending & (1U << 0)) {
    if (tx_req_pending & (1U << 1)) {
        return false;   /* both buffers busy */
    }
    buf_idx = 1;
}
```

### Baud rate

OpenLCB requires 125 kbps. The prescaler and time segments depend on your
MCAN source clock. SysConfig calculates these automatically from the target
baud rate and clock frequency.

**If you change the clock tree**, regenerate SysConfig and verify the baud
rate with a scope or CAN analyzer.

---

## 10. GPIO and LEDs

**File:** `bootloader_drivers_openlcb.h` and `bootloader_drivers_openlcb.c`

The MSPM0G3507 LaunchPad has user LEDs on GPIOB. The demo uses PB22
(negative logic -- clearPins turns it ON, setPins turns it OFF):

```c
#define BOOTLOADER_LED_PORT        GPIOB
#define BOOTLOADER_LED_PIN         DL_GPIO_PIN_22
```

All logical LED functions (ACTIVE, WRITING, CSUM_ERROR, REQUEST, FRAME_LOST)
map to this single physical LED. On boards with multiple LEDs, add a switch
on the `led` enum parameter.

Configure GPIO outputs in SysConfig. The generated code in
`ti_msp_dl_config.c` handles clock enables and pin mux.

---

## 11. Boot Button

**File:** `bootloader_drivers_openlcb.h` and `bootloader_drivers_openlcb.c`

The demo reads PB21 (active-low with pull-up) to force bootloader mode at
power-on:

```c
#define BOOTLOADER_BUTTON_PORT     GPIOB
#define BOOTLOADER_BUTTON_PIN      DL_GPIO_PIN_21

uint32_t pin_state = DL_GPIO_readPins(BOOTLOADER_BUTTON_PORT, BOOTLOADER_BUTTON_PIN);
if (pin_state == 0) {
    return BOOTLOADER_REQUESTED_BY_BUTTON;
}
```

Change the GPIO pin and polarity for your board. Configure the pin as a
digital input with pull-up in SysConfig.

If your board has no dedicated button, you can remove this check entirely.
The app-to-bootloader magic RAM handshake still works without it.

---

## 12. Clock and SysConfig

Both projects configure peripherals through SysConfig (`.syscfg` files).
SysConfig generates `SYSCFG_DL_init()` which configures clocks, GPIO, MCAN,
and SysTick in one call.

The MSPM0G3507 LaunchPad runs from the internal oscillators. If your board
uses an external crystal, update the SysConfig clock tree accordingly.

**Derived settings that change with the clock:**
- **MCAN baud rate** -- must remain 125 kbps. SysConfig recalculates
  prescaler/segments automatically.
- **SysTick period** -- must remain 100 ms. Adjust the reload value for
  your system clock frequency.

### SysConfig regeneration

When you modify the `.syscfg` file, SysConfig regenerates `ti_msp_dl_config.c`
and `ti_msp_dl_config.h` in the `Debug/` folder. This is safe -- the
bootloader code includes the generated files and does not hand-edit them.

---

## 13. SysTick as 100 ms Timer

The bootloader uses SysTick for the 100 ms tick needed by CAN alias
negotiation and protocol timeouts:

```c
static volatile uint8_t _tick_counter = 0;

void SysTick_Handler(void) {
    _tick_counter++;
}

uint8_t BootloaderDriversOpenlcb_get_100ms_timer_tick(void) {
    return _tick_counter;
}
```

SysTick is configured by `SYSCFG_DL_init()` to fire every 100 ms.

**Lesson learned:** SysTick must be stopped in `cleanup_before_handoff()`
before jumping to the application. If left running, the SysTick interrupt
fires into the application's vector table before its own `SysTick_Handler`
is ready -- potentially calling into uninitialised code or faulting if the
handler address is still erased flash (0xFFFFFFFF).

---

## 14. Cleanup Before Handoff

**File:** `bootloader_drivers_openlcb.c`, `cleanup_before_handoff()`

This function tears down everything the bootloader touched so the application
starts from a clean state:

```c
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    /* Stop SysTick completely. */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Disable all NVIC interrupts and clear pending flags.
     * MSPM0G3507 has fewer than 32 IRQs -- one register pair is enough.
     * If porting to a chip with more than 32 IRQs, loop over ICER[0..N]. */
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;

    /* Reset the MCAN peripheral to power-on defaults. */
    DL_MCAN_reset(MCAN0_INST);

}
```

### What to change for your part

- **NVIC register count:** If your MSPM0 variant has more than 32 IRQs,
  loop over multiple ICER/ICPR registers. Count = ceil(max_IRQn / 32).
- **Additional peripherals:** If your bootloader enables UART, SPI, or
  other peripherals for debug, de-init them here.
- **MCAN instance:** Change `MCAN0_INST` if your board uses a different
  MCAN instance.

### Jump to application

After cleanup, the bootloader jumps using the standard ARM Cortex-M pattern:

```c
uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;
SCB->VTOR = APP_FLASH_START;
__DSB();
__ISB();
__set_MSP(app_vectors[0]);
void (*app_reset)(void) = (void (*)(void)) app_vectors[1];
app_reset();
```

The DSB/ISB after the VTOR write ensures the pipeline fetches vectors from
the new table before the jump.

---

## 15. Shared RAM (Handshake)

**Files:** `shared/bootloader_shared_ram.h`, `shared/bootloader_shared_ram.c`

The handshake uses a struct in a `.noinit` section at a fixed SRAM address:

```c
typedef struct {
    volatile uint32_t request_flag;    /* magic value 0xB00710AD */
    volatile uint16_t cached_alias;    /* app's CAN alias */
} bootloader_shared_ram_t;

/* In .c file: */
bootloader_shared_ram_t bootloader_shared_ram __attribute__((section(".noinit")));
```

### What to change

- The SHARED_NOINIT address in both linker scripts must match and must be at
  the very end of SRAM so it does not collide with heap/stack.
- MSPM0G3507 SRAM starts at 0x20200000, 32KB total. The demo puts
  SHARED_NOINIT at 0x20207FF0 (last 16 bytes).
- If your part has a different SRAM size or base address, adjust both
  linker scripts.

**The struct must be identical in both projects.** Both projects compile the
same `shared/bootloader_shared_ram.c`. The `__attribute__((section(".noinit")))`
places it in the NOLOAD region.

**Lesson learned -- use a struct, not individual variables:** Individual
variables can be reordered by the linker between Debug and Release builds,
breaking the handshake. A struct guarantees identical layout in both binaries.

**Lesson learned -- stale cached alias:** After a power-on or programmer
reset, `.noinit` RAM contains garbage. If `bootloader_cached_alias` happens
to be non-zero, the CAN state machine skips alias negotiation and uses a
garbage value -- often overflowing the 12-bit alias field and corrupting the
CAN identifier. The fix: `initialize_hardware()` clears
`bootloader_shared_ram.cached_alias` unconditionally on every reset EXCEPT
`BOOTLOADER_REQUESTED_BY_APP`.

**Lesson learned -- use the `request` parameter, not `DL_SYSCTL_getResetCause()`:**
The original implementation checked `DL_SYSCTL_getResetCause()` to decide
whether to clear shared RAM. This is fragile -- TI's reset-cause register
bits may be sticky across resets. `initialize_hardware(request)` now bases
all cleanup on the `request` parameter from `is_bootloader_requested()`,
which is unambiguous. This pattern is consistent across all platforms.

### Bootloader_init() takes a `request` parameter (library fix)

The old `Bootloader_init()` called `is_bootloader_requested()` a second time
internally. Since the first call (in `Bootloader_entry()`) had already
consumed the magic flag, the second call always returned
`BOOTLOADER_NOT_REQUESTED`. On app drop-back, the bootloader would bounce
back to the application instead of staying in bootloader mode.

**Fix (library-level):** `Bootloader_init()` now takes a
`bootloader_request_t request` parameter. `Bootloader_entry()` calls
`is_bootloader_requested()` once and passes the result through. If you are
implementing a new driver, make sure `is_bootloader_requested()` is only
called once -- consuming the magic flag is a one-shot operation.

---

## 16. Application app_header.c

**File:** `BasicNode/app_header.c`

This file places a 36-byte zero-initialized struct in the `.app_header`
linker section. Copy it as-is to your application project. The only things
that matter are:

1. The struct size matches `bootloader_app_header_t` (36 bytes).
2. It has `__attribute__((section(".app_header")))`.
3. It has `__attribute__((used))` so the linker does not discard it.
4. All fields are zero-initialized (the post-link tool patches them).

The `_Static_assert` guard catches size mismatches at compile time:

```c
_Static_assert(sizeof(app_header_image_t) == 36U,
               "app_header_image_t size mismatch vs bootloader_app_header_t -- "
               "update APP_HEADER_CHECKSUM_COUNT or field list");
```

---

## 17. Node ID

**File:** `bootloader_drivers_openlcb.c`, `get_persistent_node_id()`

The demo returns a hardcoded node ID for bringup:

```c
return 0x050101012200ULL;
```

For production, read the node ID from the reserved flash sector at
`NODEID_FLASH_ADDRESS` (0x0001FC00). Every OpenLCB node must have a globally
unique 48-bit node ID.

---

## 18. Post-Link Checksum Tool

After building the application, run `hex2bin.py` to produce a
checksum-patched binary. On MSPM0 there is an extra step: convert the ELF
to Intel HEX first.

### Step 1: ELF to Intel HEX

CCS Theia outputs `.out` (ELF) files, not Intel HEX. Use `tiarmobjcopy`:

```bash
tiarmobjcopy -O ihex BasicNode.out BasicNode.objcopy.hex
```

**Lesson learned -- CCS hex drops zeros:** TI CCS's built-in hex converter
omits zero-filled regions from the output. The app_header (all zeros before
patching) is silently dropped. This causes hex2bin to report the header
region as "not all zeros" or produce incorrect checksums. Always use
`tiarmobjcopy` from the ELF instead.

### Step 2: hex2bin

```bash
python3 hex2bin.py \
    --arch flat \
    --start 0x00003C00 \
    --app-header-offset 0xC0 \
    --trim \
    BasicNode.objcopy.hex \
    BasicNode.mspm0.debug.boot.bin
```

### Arguments explained

- **`--arch flat`** -- Von Neumann architecture (binary bytes = flash bytes).
  No phantom byte correction needed (that is dsPIC-only).
- **`--start 0x00003C00`** -- APP_FLASH_START.
- **`--app-header-offset 0xC0`** -- APP_HEADER_OFFSET (192 bytes after app
  start, right after the vector table).
- **`--trim`** -- strips trailing 0xFF fill bytes (rounded to 8-byte
  boundary) to reduce upload time.

### create_boot_images.sh

Add your target to `bootloader/tools/hex2bin/create_boot_images.sh`. The
script handles the ELF-to-hex conversion and hex2bin invocation for all
platforms.

### Development bypass (NO_CHECKSUM)

During bringup you can skip CRC validation entirely by uncommenting
`#define NO_CHECKSUM` in `bootloader_drivers_openlcb.h` (or the shared
`bootloader_types.h`). This lets you program the application via the
debugger without running hex2bin.py first. The bootloader will jump to
the application regardless of the header contents.

Comment it out once `hex2bin.py` is producing patched binaries for your
target.

---

## 19. Application Drop-Back (Freeze)

When the OpenLCB Config Tool sends a Freeze command targeting the firmware
address space (0xEF), the application executes this sequence:

```
Step 1:  __disable_irq()          Disable all interrupts
Step 2:  Cache CAN alias          bootloader_shared_ram.cached_alias = node->alias
Step 3:  Set magic flag           bootloader_shared_ram.request_flag = 0xB00710AD
Step 4:  DL_SYSCTL_resetDevice()  System reset, SRAM preserved
```

**Lesson learned -- no peripheral teardown needed:** Unlike the
jump_to_application path, the Freeze/drop-back path uses
`DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_SYSRST)` which resets all peripherals
to their power-on state. No explicit SysTick stop, NVIC disable, or MCAN
reset is needed. `__disable_irq()` is sufficient to prevent any ISR from
running in the window between the shared RAM writes and the reset.

**Why __disable_irq() is critical:** Without it, a CAN RX interrupt could
fire between the alias write and the reset, corrupting state or processing a
stale message while shared RAM is half-written.

---

## 20. Building and Flashing

### First-time setup (both binaries)

1. Build BasicNodeBootloader (Debug configuration)
2. Flash the bootloader via the LaunchPad's on-board XDS110 debugger
3. Build BasicNode (Debug configuration)
4. Run `create_boot_images.sh` (or the manual steps in Step 18) to produce
   the `.boot.bin`
5. Upload the `.bin` via the OpenLCB Config Tool, or flash via the debugger

### Firmware update via OpenLCB Config Tool

1. Build the application and run hex2bin.py to produce the `.boot.bin`
2. In the Config Tool, connect to the node
3. Send Freeze to firmware space (0xEF)
4. The application drops back to the bootloader (see Step 19)
5. Upload the binary data
6. The bootloader writes to flash, verifies checksums, and reboots
7. The bootloader validates the application and jumps to it

**Lesson learned:** Make sure you load the correct `.bin` file for the target
platform. Loading an STM32 binary onto an MSPM0 board will write garbage to
flash and the checksum will fail. The filenames include the platform:
`BasicNode.mspm0.debug.boot.bin`.

---

## 21. Verification Checklist

Work through this list after making all changes:

### Linker scripts
- [ ] Both .cmd files start with `-uinterruptVectors` and `--stack_size=512`
- [ ] Bootloader BOOT_FLASH origin = 0x00000000, length = bootloader sectors only
- [ ] Application APP_FLASH origin = APP_FLASH_START, length = app sectors only
- [ ] SHARED_NOINIT address and size identical in both scripts
- [ ] .noinit section with type = NOINIT present in both scripts
- [ ] .app_header section at correct fixed address in application script
- [ ] .TI.ramfunc section present with load/run split (flash ops run from RAM)
- [ ] BCR_CONFIG and BSL_CONFIG sections present (required by MSPM0)

### Flash constants
- [ ] APP_FLASH_START matches application linker script APP_FLASH origin
- [ ] APP_FLASH_END matches last byte of application region
- [ ] APP_HEADER_OFFSET places header after vector table (no overlap)
- [ ] FLASH_SECTOR_SIZE matches device datasheet
- [ ] FLASH_WRITE_ALIGN = 8 (64-bit write with ECC)

### Flash operations
- [ ] __disable_irq() / __enable_irq() around every erase and write
- [ ] DL_FlashCTL_executeClearStatus() before every operation
- [ ] DL_FlashCTL_unprotectSector() before every erase and write
- [ ] memcpy through aligned uint32_t buffer (never cast uint8_t* to uint32_t*)
- [ ] Read-back verification after every 8-byte write
- [ ] Trailing bytes padded with 0xFF to 8-byte boundary
- [ ] Write deduplication check (read before write, skip if matching)

### CAN
- [ ] MCAN configured in SysConfig with correct pins
- [ ] Baud rate = 125 kbps (verify with scope or analyzer)
- [ ] MCAN interrupts NOT enabled in SysConfig (bootloader polls)
- [ ] TX function checks both buffer slots before returning false

### GPIO
- [ ] LED pin and polarity match your board
- [ ] Boot button pin and polarity correct
- [ ] GPIO configured in SysConfig (not hand-coded)

### Timer
- [ ] SysTick period = 100 ms at your system clock frequency
- [ ] SysTick_Handler increments _tick_counter
- [ ] SysTick stopped in cleanup_before_handoff()

### Handshake
- [ ] Both projects compile shared/bootloader_shared_ram.c
- [ ] .noinit region is type = NOINIT in both linker scripts
- [ ] SHARED_NOINIT does not overlap heap or stack
- [ ] initialize_hardware() clears stale cached_alias on non-app resets
- [ ] Shared RAM uses a struct (not individual variables)
- [ ] is_bootloader_requested() called only once (double-call bug)

### Checksum
- [ ] app_header.c in application project, placed in .app_header section
- [ ] tiarmobjcopy used to generate hex (not CCS built-in hex converter)
- [ ] hex2bin --arch flat --start and --app-header-offset match your constants
- [ ] hex2bin produces non-zero checksums (not all 0x0000)
- [ ] Bootloader boots the patched image without checksum error LED

### Cleanup before handoff
- [ ] SysTick stopped (CTRL, LOAD, VAL all zeroed)
- [ ] NVIC ICER/ICPR registers cleared (all interrupts disabled)
- [ ] MCAN peripheral reset via DL_MCAN_reset()
- [ ] DSB + ISB after VTOR write in jump_to_application()

### Integration
- [ ] Bootloader fits in allocated flash (check .map file)
- [ ] Application .map shows .app_header at expected address
- [ ] Boot button forces bootloader mode
- [ ] Application can drop back to bootloader (magic RAM handshake)
- [ ] After bootload + reboot, bootloader jumps to application
- [ ] Checksum error LED lights when image is corrupt or unpatched

### File reference

| File | Purpose |
|------|---------|
| `bootloader_main.c` | Entry point; wires DI structs, calls Bootloader_entry() |
| `device_linker.cmd` (bootloader) | Bootloader linker script |
| `device_linker.cmd` (application) | Application linker script |
| `application_drivers/bootloader_drivers_can.c/h` | CAN physical layer DI |
| `application_drivers/bootloader_drivers_openlcb.c/h` | OpenLCB/hardware DI |
| `app_header.c` | 36-byte zero-init struct for CRC patching |
| `shared/bootloader_shared_ram.h/c` | Magic flag, cached alias |
| `bootloader.syscfg` | SysConfig peripheral configuration |
