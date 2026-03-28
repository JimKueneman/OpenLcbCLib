# How To Modify This Demo For Your STM32

This guide walks through every change needed to adapt the STM32F407 Discovery
bootloader demo to a different STM32 part or board. It covers both the
BasicNodeBootloader project (the bootloader itself) and the BasicNode project
(the application it boots).

The demo is built around two CubeIDE projects that share a common RAM region
for handshake data. Both projects need coordinated changes when you retarget.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Understand the Flash Layout](#2-understand-the-flash-layout)
3. [Create the CubeIDE Projects](#3-create-the-cubeide-projects)
4. [Bootloader Linker Script](#4-bootloader-linker-script)
5. [Application Linker Script](#5-application-linker-script)
6. [Flash Sector Table](#6-flash-sector-table)
7. [Flash Constants](#7-flash-constants)
8. [App Header Offset](#8-app-header-offset)
9. [Flash Operations](#9-flash-operations)
10. [CAN Configuration](#10-can-configuration)
11. [GPIO and LEDs](#11-gpio-and-leds)
12. [Boot Button](#12-boot-button)
13. [Clock Configuration](#13-clock-configuration)
14. [Cleanup Before Handoff](#14-cleanup-before-handoff)
15. [Shared RAM (Handshake)](#15-shared-ram-handshake)
16. [Application app_header.c](#16-application-app_headerc)
17. [Node ID](#17-node-id)
18. [Post-Link Checksum Tool](#18-post-link-checksum-tool)
19. [Building and Flashing](#19-building-and-flashing)
20. [Verification Checklist](#20-verification-checklist)

---

## 1. Prerequisites

- STM32CubeIDE installed
- The reference manual and datasheet for your target STM32 part
- The flash memory map from the reference manual (sector layout, sizes, base
  addresses)
- Know which CAN peripheral and pins your board uses

---

## 2. Understand the Flash Layout

Before changing any code, draw out the flash layout for your part. You need
to decide three things:

1. **How much flash for the bootloader?** It must fit in contiguous sectors
   starting at the flash base. The F407 demo uses 32KB (two 16KB sectors).
   Typical bootloader size is 16-32KB.

2. **Where does the application start?** Immediately after the bootloader
   sectors. This becomes APP_FLASH_START.

3. **Do you need a reserved region?** The demo reserves the last 128KB sector
   for node ID storage. You may not need this, or you may want it somewhere
   else.

**F407 Discovery layout (for reference):**

```
0x08000000 +---------------------------+
           | Bootloader (32KB)         |  Sectors 0-1
0x08008000 +---------------------------+
           | Application (864KB)       |  Sectors 2-10
           |   +-- app_header at +0x200|
0x080E0000 +---------------------------+
           | Node ID (128KB)           |  Sector 11
0x08100000 +---------------------------+
```

**Key constraint:** The bootloader must occupy complete sectors because flash
erase granularity is per-sector. If your part has 4KB sectors, a 16KB
bootloader uses 4 sectors. If it has 128KB sectors, you may be forced to
waste space or restructure.

---

## 3. Create the CubeIDE Projects

Start with two CubeIDE projects generated from your STM32 part:

- **BasicNodeBootloader** -- configure CAN, GPIO for LEDs and boot button,
  SysTick. No other peripherals needed.
- **BasicNode** -- your application project.

**Lesson learned:** CubeIDE regenerates `STM32xxxx_FLASH.ld` every time you
run code generation. Do NOT use that file. Create custom linker scripts with
different names (e.g. `STM32xxxx_BOOTLOADER.ld` and
`STM32xxxx_APPLICATION.ld`) and point the project settings to them. The
auto-generated file is harmless but not used.

To change the linker script: Project Properties -> C/C++ Build -> Settings ->
Tool Settings -> MCU GCC Linker -> General -> Linker Script.

---

## 4. Bootloader Linker Script

Create `STM32xxxx_BOOTLOADER.ld` by copying the auto-generated FLASH.ld and
making these changes:

### MEMORY section

```
MEMORY
{
  RAM           (xrw) : ORIGIN = 0x20000000, LENGTH = <total_ram - 16>
  SHARED_NOINIT (xrw) : ORIGIN = <ram_end - 16>,     LENGTH = 0x10
  FLASH          (rx) : ORIGIN = 0x08000000, LENGTH = <bootloader_size>
}
```

- **FLASH LENGTH** = bootloader region only (e.g. 32K, not the whole chip).
- **RAM LENGTH** = total RAM minus 16 bytes. The last 16 bytes are reserved
  for the handshake.
- **SHARED_NOINIT** = the last 16 bytes of SRAM. Must be identical in both
  linker scripts.

**CCMRAM:** The F407 has 64KB of core-coupled memory (CCMRAM at 0x10000000).
The auto-generated linker script includes it. If your part does not have
CCMRAM (most STM32G0, STM32L0, and some STM32G4 parts), remove the CCMRAM
line from the MEMORY block. If your part does have it, keep it -- the linker
will use it for `.ccmram` sections if any code requests it.

### SECTIONS -- add .noinit

Add this section at the end (before /DISCARD/):

```
  .noinit (NOLOAD) :
  {
    KEEP(*(.noinit))
  } >SHARED_NOINIT
```

The `NOLOAD` attribute prevents the C startup code from zeroing this region.
That is how the handshake data survives a software reset.

---

## 5. Application Linker Script

Create `STM32xxxx_APPLICATION.ld` with these changes from the auto-generated
file:

### MEMORY section

```
MEMORY
{
  RAM           (xrw) : ORIGIN = 0x20000000, LENGTH = <total_ram - 16>
  SHARED_NOINIT (xrw) : ORIGIN = <ram_end - 16>,     LENGTH = 0x10
  FLASH          (rx) : ORIGIN = <app_flash_start>,   LENGTH = <app_flash_size>
}
```

- **FLASH ORIGIN** = where the application starts (right after the bootloader
  sectors).
- **FLASH LENGTH** = application region only. Exclude the bootloader sectors
  and any reserved sectors (node ID, etc.).
- **SHARED_NOINIT** = same address and size as the bootloader script.

### SECTIONS -- add .app_header and .noinit

After `.isr_vector`, add the app header section at a fixed address:

```
  .app_header <app_header_absolute_address> :
  {
    KEEP(*(.app_header))
    . = ALIGN(4);
  } >FLASH
```

And add `.noinit` at the end, same as the bootloader script.

---

## 6. Flash Sector Table

**File:** `bootloader_drivers_openlcb.c`

The F407 has non-uniform sectors (16KB/64KB/128KB). Most other STM32 families
have uniform sectors. Either way, you need a lookup table that maps any flash
address to its sector start, size, and HAL sector number.

**F407 (non-uniform):**

```c
static const flash_sector_entry_t _flash_sectors[] = {
    { 0x08000000U,  16384U, FLASH_SECTOR_0  },
    { 0x08004000U,  16384U, FLASH_SECTOR_1  },
    { 0x08008000U,  16384U, FLASH_SECTOR_2  },
    { 0x0800C000U,  16384U, FLASH_SECTOR_3  },
    { 0x08010000U,  65536U, FLASH_SECTOR_4  },
    { 0x08020000U, 131072U, FLASH_SECTOR_5  },
    /* ... */
};
```

**If your part has uniform sectors** (e.g. STM32G4 with 2KB pages), you can
simplify `_find_sector()` to a calculation instead of a table lookup, but the
table approach works everywhere.

**Where to find sector info:** Reference manual, "Embedded Flash memory"
chapter. Look for the sector/page map table.

**STM32 families with uniform pages** (STM32L4, STM32G0, STM32G4, STM32H5,
STM32WB, etc.) use `FLASH_TYPEERASE_PAGES` and `Page` instead of
`FLASH_TYPEERASE_SECTORS` and `Sector`. The erase function in
`bootloader_drivers_openlcb.c` must match. See Step 9.

---

## 7. Flash Constants

**File:** `bootloader_drivers_openlcb.h`

Update every `#define` to match your flash map:

```c
#define BOOTLOADER_FLASH_START     0x08000000U       /* always 0x08000000 on STM32 */
#define BOOTLOADER_FLASH_SIZE      0x00008000U       /* your bootloader size */
#define APP_FLASH_START            0x08008000U       /* first byte after bootloader */
#define APP_FLASH_END              0x080DFFFFU       /* last byte of app region */
#define NODEID_FLASH_ADDRESS       0x080E0000U       /* or remove if not used */
#define FLASH_WRITE_ALIGN          4U                /* see Step 9 */
```

These values must agree with the linker scripts.

---

## 8. App Header Offset

**File:** `bootloader_drivers_openlcb.h`

The app header (bootloader_app_header_t, 36 bytes) must be placed after the
vector table so they do not overlap.

```c
#define APP_HEADER_OFFSET     0x00000200U
#define APP_HEADER_ADDRESS    (APP_FLASH_START + APP_HEADER_OFFSET)
```

**How to calculate the offset:**

1. Count the interrupt vectors for your part. The Cortex-M vector table has
   16 system entries plus the device-specific IRQs. Check the startup .s file
   or the IRQn_Type enum in your device header.
2. Multiply by 4 bytes per entry. For example: 98 vectors x 4 = 392 bytes.
3. Round up to a comfortable power-of-2 boundary. The F407 uses 0x200 (512)
   which gives headroom above the 392-byte table.

**Cortex-M0/M0+ parts** (like STM32G0, STM32L0) have fewer vectors and a
smaller table. 0xC0 (192 bytes, matching 48 vectors) may be sufficient. The
MSPM0 demo uses this value.

---

## 9. Flash Operations

**File:** `bootloader_drivers_openlcb.c`

### Write granularity

Different STM32 families have different flash write sizes:

| Family | Write unit | HAL constant | FLASH_WRITE_ALIGN |
|--------|-----------|--------------|-------------------|
| F4     | 32-bit word | FLASH_TYPEPROGRAM_WORD | 4 |
| L4, G4, WB | 64-bit doubleword | FLASH_TYPEPROGRAM_DOUBLEWORD | 8 |
| H7     | 256-bit (32 bytes) | FLASH_TYPEPROGRAM_FLASHWORD | 32 |
| G0     | 64-bit doubleword | FLASH_TYPEPROGRAM_DOUBLEWORD | 8 |

Update `FLASH_WRITE_ALIGN` and the `HAL_FLASH_Program()` call to match.

**For doubleword parts (8-byte write):** The write_buffer in
bootloader_types.h is already 8-byte aligned, so no changes there. But
`write_flash_bytes()` must accumulate 8 bytes before calling
`HAL_FLASH_Program()`:

```c
/* Example for 64-bit doubleword programming */
uint64_t dword;
memcpy(&dword, source_data, 8);
HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, dest, dword);
```

### Erase

Parts with uniform pages use a different erase struct:

```c
/* F4 (non-uniform sectors): */
erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
erase.Sector       = s->sector_number;
erase.NbSectors    = 1;
erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

/* G4, L4, WB (uniform pages): */
erase.TypeErase = FLASH_TYPEERASE_PAGES;
erase.Page      = page_number;
erase.NbPages   = 1;
/* No VoltageRange field */
```

### Voltage range

The F407 uses `FLASH_VOLTAGE_RANGE_3` (2.7-3.6V) which enables 32-bit
parallelism. If your board runs at a lower voltage, use the appropriate range
constant. The voltage range determines the write parallelism and affects
programming speed.

---

## 10. CAN Configuration

### Which CAN peripheral?

STM32 families use different CAN IP blocks:

| IP block | Families | HAL prefix |
|----------|----------|------------|
| bxCAN | F1, F2, F4, L4 | HAL_CAN_* |
| FDCAN | G0, G4, H7, L5, U5, WB | HAL_FDCAN_* |

**If your part uses FDCAN:** The `bootloader_drivers_can.c` file needs
significant rework. The filter setup, RX/TX API, and handle types are all
different. The logic (poll FIFO, check mailbox free, map extended ID) is the
same but every HAL call changes. Use the CubeIDE-generated FDCAN init as a
starting point.

### Pin mapping

**File:** `stm32f4xx_hal_msp.c` (CubeMX generates this)

The F407 demo uses CAN1 on PD0 (RX) / PD1 (TX) with GPIO_AF9_CAN1. Your
part will have different pin options and alternate function numbers. Use
CubeMX to configure CAN pins and regenerate.

### Baud rate

**File:** `bootloader_main.c`, `MX_CAN1_Init()`

OpenLCB uses 125 kbps. The prescaler and time segments depend on your APB1
clock frequency:

```
Baud = APB1_Clock / (Prescaler * (1 + TimeSeg1 + TimeSeg2))
```

F407 demo: 42 MHz / (42 * (1 + 3 + 4)) = 125 kbps

Use the CubeMX CAN bit timing calculator or compute manually. The total
number of time quanta per bit (1 + BS1 + BS2) should be 8-25 for good
signal quality. The sample point should be near 87.5%.

### TX frame ordering

**File:** `bootloader_drivers_can.c`, `try_send_frame()`

The bxCAN peripheral has 3 TX mailboxes that transmit by CAN priority (lowest
ID first), not by insertion order. OpenLCB multiframe messages share the same
CAN ID, so frames loaded into different mailboxes can be reordered by the
hardware -- the receiving node sees scrambled data.

The fix: only send when ALL 3 mailboxes are free, so only one frame is ever
in flight:

```c
if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {
    return false;   /* wait until the previous frame completes */
}
```

**If you are writing a new FDCAN driver**, apply the same principle. FDCAN has
a TX FIFO that preserves insertion order, so you may not need this workaround
-- but verify with your part's reference manual.

---

## 11. GPIO and LEDs

**File:** `bootloader_drivers_openlcb.c`, `set_status_led()`

The F407 Discovery has 4 LEDs on PD12-PD15. Map the bootloader LED functions
to whatever your board provides:

| Function | What it signals |
|----------|----------------|
| BOOTLOADER_LED_ACTIVE | Blinking while bootloader is running |
| BOOTLOADER_LED_WRITING | Flash write in progress |
| BOOTLOADER_LED_CSUM_ERROR | Checksum validation failed |
| BOOTLOADER_LED_REQUEST | Bootloader was explicitly requested |
| BOOTLOADER_LED_FRAME_LOST | CAN frame dropped |

If you only have one LED, you can multiplex or just drive it for the most
important states (ACTIVE and CSUM_ERROR). If you have no LEDs, make
`set_status_led()` a no-op.

Update `MX_GPIO_Init()` and the `main.h` pin defines to match your board.

---

## 12. Boot Button

**File:** `bootloader_drivers_openlcb.c`, `is_bootloader_requested()`

The F407 demo reads PA0 (B1 button, active-high) to force bootloader mode at
power-on. Change the GPIO pin and polarity for your board:

```c
/* Active-high button (F407 Discovery B1): */
if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET)

/* Active-low button (most other boards): */
if (HAL_GPIO_ReadPin(BOOT_GPIO_Port, BOOT_Pin) == GPIO_PIN_RESET)
```

If your board has no dedicated button, you can remove this check entirely.
The app-to-bootloader magic RAM handshake still works without it.

---

## 13. Clock Configuration

**File:** `bootloader_main.c`, `SystemClock_Config()`

CubeMX generates this function. Regenerate it for your part. The bootloader
does not need USB, I2S, or other peripheral clocks -- just the system clock
and the APB1 clock (which feeds the CAN peripheral).

**Lesson learned:** The CAN prescaler depends on the APB1 clock frequency.
If you change the clock tree, recalculate the CAN baud rate settings.

---

## 14. Cleanup Before Handoff

**File:** `bootloader_drivers_openlcb.c`, `cleanup_before_handoff()`

This function tears down everything the bootloader touched so the application
starts from a clean state. Update it for your peripherals:

```c
void BootloaderDriversOpenlcb_cleanup_before_handoff(void) {

    /* De-init CAN */
    HAL_CAN_DeInit(&hcan1);    /* or HAL_FDCAN_DeInit() */

    /* Stop SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Clear all NVIC interrupts.
     * Count = ceil(total_IRQs / 32).
     * F407 has 82 IRQs -> 3 registers.
     * Check your device header for the max IRQn. */
    for (int i = 0; i < 3; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

}
```

The NVIC register count depends on how many IRQs your part has. Check the
last entry in your device's IRQn_Type enum and divide by 32, rounding up.

---

## 15. Shared RAM (Handshake)

**Files:** `shared/bootloader_shared_ram.h`, `shared/bootloader_shared_ram.c`

The handshake uses a struct in a `.noinit` section at a fixed SRAM address:

```c
typedef struct {
    volatile uint32_t request_flag;    /* magic value 0xB00710AD */
    volatile uint16_t cached_alias;    /* app's CAN alias */
} bootloader_shared_ram_t;
```

**What to change:**

- The SHARED_NOINIT address in both linker scripts must match and must be at
  the very end of SRAM so it does not collide with heap/stack.
- If your part has multiple SRAM regions (e.g. SRAM1 + SRAM2 on some STM32L4),
  pick one contiguous region and carve 16 bytes from the end of it.

**The struct must be identical in both projects.** Both projects compile
`bootloader_shared_ram.c`. The `__attribute__((section(".noinit")))` places
it in the NOLOAD region.

**Lesson learned:** Use a struct (not individual variables) for the shared
RAM. Individual variables can be reordered by the linker between Debug and
Release builds, breaking the handshake.

**Use `HAL_NVIC_SystemReset()`, not `NVIC_SystemReset()`:** The application's
freeze callback and the bootloader's reboot function both write to shared RAM
and then reset the CPU. The bare CMSIS `NVIC_SystemReset()` does not flush the
CPU write buffer before requesting the reset -- if the store is still buffered
when the reset fires, SRAM may never be updated. `HAL_NVIC_SystemReset()`
inserts a `__DSB()` (Data Synchronisation Barrier) that stalls the CPU until
all pending writes have reached SRAM, then requests the reset. Always use the
HAL wrapper.

---

## 16. Application app_header.c

**File:** `BasicNode/Core/Src/app_header.c`

This file places a 36-byte zero-initialized struct in the `.app_header`
linker section. Copy it as-is to your application project. The only thing
that matters is:

1. The struct size matches `bootloader_app_header_t` (36 bytes).
2. It has `__attribute__((section(".app_header")))`.
3. It has `__attribute__((used))` so the linker does not discard it.
4. All fields are zero-initialized (the post-link tool patches them).

The `_Static_assert` guard catches size mismatches at compile time.

---

## 17. Node ID

**File:** `bootloader_drivers_openlcb.c`, `get_persistent_node_id()`

The F407 demo returns a hardcoded node ID for bringup:

```c
return 0x050101012201ULL;
```

For production, read the node ID from a reserved flash region or OTP memory.
Every OpenLCB node must have a globally unique 48-bit node ID.

---

## 18. Post-Link Checksum Tool

After building the application hex, run `hex2bin.py` to produce a
checksum-patched binary:

```bash
python3 hex2bin.py \
    --arch flat \
    --start <APP_FLASH_START> \
    --app-header-offset <APP_HEADER_OFFSET> \
    --trim \
    BasicNode.hex \
    BasicNode.<platform>.<config>.boot.bin
```

The `--start` value matches `APP_FLASH_START` and `--app-header-offset`
matches `APP_HEADER_OFFSET` from `bootloader_drivers_openlcb.h`.

Add your target to `create_boot_images.sh` in
`bootloader/tools/hex2bin/`.

**Lesson learned:** CubeIDE's GCC toolchain generates correct Intel HEX
files that include all zero-initialized data. Some other toolchains (notably
TI CCS) omit zero-filled regions from the hex output, which corrupts the
app_header. If you see hex2bin complain that the header region is "not all
zeros", the toolchain's hex conversion is dropping zero bytes. The fix is to
generate the hex from the ELF using objcopy: `arm-none-eabi-objcopy -O ihex
BasicNode.elf BasicNode.hex`

### NO_CHECKSUM during development

The shared `bootloader_types.h` has a `#define NO_CHECKSUM` that disables all
checksum validation -- both at boot and after firmware write. With this flag
enabled the bootloader only checks that flash is not blank before jumping.

**Leave `NO_CHECKSUM` enabled** until your post-link checksum tool is
integrated into the build. Without it, every application image will fail the
CRC check and the bootloader will refuse to jump. Comment it out once
`hex2bin.py` is producing patched binaries for your target.

### Generating a .bin for firmware upload

The OpenLCB firmware upgrade protocol sends raw bytes. The bootloader writes
them directly to flash with no file format parsing. The upload file must be a
flat binary (`.bin`), not an `.elf` or `.hex`.

CubeIDE does not produce a `.bin` by default. Enable it once: Project
Properties -> C/C++ Build -> Settings -> MCU Post build outputs -> check
"Convert to binary file (-O binary)". After that the `.bin` appears in the
`Debug/` or `Release/` folder alongside the `.elf` after every build.

---

## 19. Building and Flashing

### Mass erase

When CubeIDE flashes via ST-Link it performs a sector-by-sector erase of only
the sectors the new binary occupies. It does NOT erase the whole chip. If you
flash a new bootloader without erasing the application sectors, the old
application is still there. On reset the bootloader sees a valid app, no
magic flag, no button pressed -- and jumps straight to the old app. You never
enter bootloader mode.

**Fix:** Add `monitor flash mass_erase` to the bootloader's debug
configuration so the entire chip is wiped before the bootloader is programmed:

1. Run -> Debug Configurations
2. Select the BasicNodeBootloader launch configuration
3. Click the Startup tab
4. In the Initialization Commands text box, add: `monitor flash mass_erase`
5. Click Apply

The BasicNode (application) debug configuration should NOT have mass erase --
it only rewrites the application sectors, leaving the bootloader intact.

### Reset behaviour -- use "Hardware reset"

After ST-Link programs the flash, the CPU must be reset. The "Reset
behaviour" dropdown on the Debugger tab controls how:

- **Software system reset** -- resets the CPU core but may NOT fully reset all
  peripherals. The PLL and clock tree can be left in an undefined state, which
  means `HAL_Init()` configures SysTick against the wrong clock frequency.
  Symptom: SysTick does not run after programming, but a power cycle fixes it.

- **Hardware reset** -- pulses the physical NRST pin via ST-Link. All
  peripherals, clock tree, and CPU core are fully reset to power-on state.

**Use "Hardware reset" for both projects.** Set it at: Run -> Debug
Configurations -> Debugger tab -> Reset behaviour dropdown.

### First-time setup

1. Build BasicNodeBootloader (Release configuration)
2. Flash the bootloader via ST-Link (mass erase wipes the chip)
3. Build BasicNode (Release configuration)
4. Flash the application via ST-Link (application sectors only)
5. Power-cycle the board

The bootloader checks for the magic flag (not set), checks the button (not
pressed), validates the application checksum, and jumps to the application.

### Firmware update via OpenLCB Config Tool

After the initial setup, firmware updates can be done over the network:

1. Build the application and run `hex2bin.py` to produce the patched `.bin`
2. In the Config Tool, connect to the node
3. Send a Freeze command to the firmware address space (0xEF)
4. The application drops back to the bootloader via the shared RAM handshake
5. The Config Tool sends the binary data via Memory Configuration datagrams
6. The bootloader erases, writes with read-back verification, and reboots
7. The bootloader validates the new application and jumps to it

---

## 20. Verification Checklist

Work through this list after making all changes:

### Linker scripts
- [ ] Bootloader FLASH origin = 0x08000000, length = bootloader sectors only
- [ ] Application FLASH origin = APP_FLASH_START, length = app sectors only
- [ ] SHARED_NOINIT address and size identical in both scripts
- [ ] .noinit (NOLOAD) section present in both scripts
- [ ] .app_header section at correct fixed address in application script
- [ ] Neither script is the auto-generated FLASH.ld

### Flash constants
- [ ] APP_FLASH_START matches application linker script FLASH origin
- [ ] APP_FLASH_END matches last byte of application region
- [ ] APP_HEADER_OFFSET places header after vector table (no overlap)
- [ ] FLASH_WRITE_ALIGN matches HAL programming granularity
- [ ] Sector table covers all flash sectors (or page calculation is correct)

### Flash operations
- [ ] erase_flash_page uses correct TypeErase (SECTORS vs PAGES)
- [ ] write_flash_bytes uses correct TYPEPROGRAM and alignment
- [ ] VoltageRange set correctly for your supply voltage (F4 only)

### CAN
- [ ] Correct CAN/FDCAN peripheral instance
- [ ] Pin mapping matches your board schematic
- [ ] Baud rate = 125 kbps (verify with scope or analyzer)
- [ ] HAL API matches your CAN IP block (bxCAN vs FDCAN)
- [ ] TX function enforces one-frame-at-a-time (all mailboxes free before send)

### GPIO
- [ ] LED pins match your board
- [ ] Boot button pin and polarity correct
- [ ] GPIO clock enables cover all used ports

### Handshake
- [ ] Both projects compile bootloader_shared_ram.c
- [ ] .noinit region is NOLOAD in both linker scripts
- [ ] SHARED_NOINIT does not overlap heap or stack
- [ ] Soft resets use HAL_NVIC_SystemReset() (not bare NVIC_SystemReset())

### Checksum
- [ ] app_header.c in application project, placed in .app_header section
- [ ] hex2bin --start and --app-header-offset match your constants
- [ ] hex2bin produces non-zero checksums (not all 0x0000)
- [ ] Bootloader boots the patched image without checksum error LED

### Build and flash
- [ ] Bootloader debug config has `monitor flash mass_erase`
- [ ] Application debug config does NOT have mass erase
- [ ] Both debug configs use "Hardware reset" (not software reset)
- [ ] Application project has "Convert to binary file" enabled
- [ ] NO_CHECKSUM enabled in bootloader_types.h during development
- [ ] CCMRAM line removed from linker scripts if part has no CCMRAM

### Integration
- [ ] Bootloader fits in allocated flash (check .map file)
- [ ] Application .map shows .app_header at expected address
- [ ] Boot button forces bootloader mode
- [ ] Application can drop back to bootloader (magic RAM handshake)
- [ ] Bootloader jumps to application after successful checksum
- [ ] Checksum error LED lights when image is corrupt or unpatched
