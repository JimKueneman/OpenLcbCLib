# STM32F407 Bootloader and Application Demo -- Tutorial and Reference

This document explains how the BasicNodeBootloader (bootloader) and
BasicNode (application) projects work together on the STM32F407 Discovery
board.  It covers the architecture, the build and flash workflow, the
CubeMX considerations, and the pitfalls that were discovered during
development.

---

## 1. Flash Layout

The STM32F407VGT6 has 1 MB of flash organised in a single bank with
NON-UNIFORM sector sizes.  Unlike many microcontrollers where every page
is the same size, the STM32F4 uses a mixed layout:

```
Sector   Size     Address Range             Contents
------   ------   -----------------------   ----------------------
0        16 KB    0x08000000 - 0x08003FFF   Bootloader
1        16 KB    0x08004000 - 0x08007FFF   Bootloader
2        16 KB    0x08008000 - 0x0800BFFF   Application start
3        16 KB    0x0800C000 - 0x0800FFFF   Application
4        64 KB    0x08010000 - 0x0801FFFF   Application
5        128 KB   0x08020000 - 0x0803FFFF   Application
6        128 KB   0x08040000 - 0x0805FFFF   Application
7        128 KB   0x08060000 - 0x0807FFFF   Application
8        128 KB   0x08080000 - 0x0809FFFF   Application
9        128 KB   0x080A0000 - 0x080BFFFF   Application
10       128 KB   0x080C0000 - 0x080DFFFF   Application
11       128 KB   0x080E0000 - 0x080FFFFF   Reserved (node ID)
```

The non-uniform layout matters for two reasons:

1. **Erase granularity** -- the smallest erase unit is a full sector.
   Erasing sector 5 wipes 128 KB even if only one byte needs to change.
   The bootloader library calls `get_flash_page_info()` to learn the
   erase size so it can manage write boundaries correctly.

2. **Memory partitioning** -- the small 16 KB sectors at the bottom fit
   the bootloader (sectors 0-1, 32 KB total).  The application gets
   sectors 2-10 (864 KB).  Sector 11 is reserved for persistent data
   (node ID) and is never erased during firmware updates.

The HAL erase function requires a SECTOR NUMBER, not an address.  The
driver maintains a lookup table (`_flash_sectors[]`) that maps byte
addresses to sector numbers.

### Linker scripts

Both projects have custom linker scripts that split the memory regions:

- Bootloader: `STM32F407VGTX_BOOTLOADER.ld`
  - `FLASH ORIGIN = 0x08000000, LENGTH = 32K`
- Application: `STM32F407VGTX_APPLICATION.ld`
  - `FLASH ORIGIN = 0x08008000, LENGTH = 864K`

Both share a `SHARED_NOINIT` region at the top of RAM (see section 2).

### Application vector table and header

On Cortex-M the vector table is the first thing in the application's flash
region.  The STM32F407 has 98 vectors (reset + 81 IRQs + NMI + faults),
occupying 392 bytes.  The application header (checksums, app size) is placed
at a fixed offset of 512 bytes after the application start:

```
0x08008000    Application vector table (98 x 4 = 392 bytes)
0x08008188    Gap (filled with 0xFF, included in checksum)
0x08008200    Application header (checksums, app size)
```

The bootloader reads the application header to validate the firmware
image before jumping to it.

---

## 2. Shared RAM (Bootloader <-> Application Handshake)

Both projects compile the same source file: `shared/bootloader_shared_ram.c`.
It defines a single struct placed in a `.noinit` section:

```c
typedef struct {
    volatile uint32_t request_flag;    // Magic value 0xB00710AD triggers bootloader mode
    volatile uint16_t cached_alias;    // CAN alias passed from app to bootloader
} bootloader_shared_ram_t;

bootloader_shared_ram_t bootloader_shared_ram __attribute__((section(".noinit")));
```

```
Address        Field                     Size    Purpose
-----------    ----------------------    ----    ---------------------------------
0x2001FFF0     .request_flag             4 B     Magic value 0xB00710AD triggers
                                                 bootloader mode
0x2001FFF4     .cached_alias             2 B     CAN alias passed from app to
                                                 bootloader
0x2001FFF6     (padding)                 10 B    Unused
```

The struct uses `__attribute__((section(".noinit")))` which tells the GCC
startup code NOT to zero it.  The values survive a software reset
(`HAL_NVIC_SystemReset()`) because SRAM content is preserved when neither
a power-on reset nor a brownout occurs.

Both linker scripts map the `.noinit` section to the same physical SRAM
region (`SHARED_NOINIT: ORIGIN = 0x2001FFF0, LENGTH = 16`).  This ensures
both binaries read and write the same physical addresses.

The `NOLOAD` directive in the linker script's `.noinit` output section
prevents the C startup from clearing this region:

```
.noinit (NOLOAD) :
{
    *(.noinit*)
} > SHARED_NOINIT
```

### CRITICAL PITFALL -- Why the variables MUST be in a struct

An earlier version defined `request_flag` and `cached_alias` as two
separate global variables, both with `__attribute__((section(".noinit")))`.
This worked in initial testing but failed unpredictably.

The root cause: GCC's linker is free to reorder individual variables
within a section.  The order depends on factors like:

- Debug vs Release build configuration
- Object file link order (which can change when you add/remove files)
- Optimisation level (-O0 vs -Os vs -O2)

Because the bootloader and application are SEPARATELY LINKED binaries,
the linker can (and does) place the variables at different offsets in
each binary.  The .map files showed:

```
BasicNode (application) -- Release build:
  0x2001FFF0   cached_alias
  0x2001FFF4   request_flag

BasicNodeBootloader -- Debug build:
  0x2001FFF0   request_flag
  0x2001FFF4   cached_alias
```

The application wrote the CAN alias to 0x2001FFF0, but the bootloader
read the alias from 0x2001FFF4.  The handshake silently failed --
the bootloader saw a garbage alias and ran a full CID negotiation,
which changed the alias mid-session and confused the Config Tool.

**The fix:** Wrap both variables in a struct.  C guarantees that struct
members are laid out in declaration order.  Both binaries compile the
same header, so both see `request_flag` at offset 0 and `cached_alias`
at offset 4 -- regardless of build configuration, link order, or
optimisation level.

This is a general rule for any two-binary shared RAM scheme on GCC:
**never rely on the linker to order individual variables within a section.
Always use a struct.**

Some compilers offer alternatives (e.g. dsPIC XC16 supports
`__attribute__((address(0xNNNN)))` to pin each variable to a fixed
address), but GCC ARM does not have an equivalent.  The struct approach
is portable and works everywhere.

### Other shared RAM pitfalls

**Stale cached alias:** After programming with ST-Link, SRAM contains
random garbage.  If `cached_alias` happens to be non-zero, the CAN state
machine skips alias negotiation and uses a garbage value -- often something
that overflows the 12-bit alias field and corrupts the CAN identifier.
The fix: `initialize_hardware()` clears `cached_alias` on every reset path
EXCEPT `BOOTLOADER_REQUESTED_BY_APP`, where the application wrote a valid
alias before resetting.

**Why a 32-bit magic value?** A single boolean flag in uninitialised RAM
has a 50% chance of being "true" after power-on.  The 32-bit magic value
`0xB00710AD` has a 1-in-4-billion chance of matching random garbage,
making false-positive bootloader entry effectively impossible.

---

## 3. Interrupt Architecture

### How it differs from dsPIC

On Cortex-M the vector table is relocatable via the `SCB->VTOR` register.
When the bootloader hands off to the application, it writes
`SCB->VTOR = APP_FLASH_START` and all interrupts and faults immediately
route to the application's own vector table in flash.  No software jump
table (VIVT) or ISR forwarding is needed.

This means:

- The bootloader does NOT need to patch or forward interrupts
- The application does NOT need to register interrupt handlers at startup
- There are no MCC/CubeMX hand-edits required for interrupt forwarding

### What DOES matter: cleanup before handoff

Even though the vector table switch is simple, the bootloader must still
put all peripherals back to their reset state before jumping.  If the
bootloader leaves SysTick running, for example, the SysTick interrupt will
fire immediately after the VTOR switch -- before the application has
initialised its HAL.  The handler address in the application's vector table
might point to uninitialised code or erased flash (0xFFFFFFFF), causing a
hard fault.

`cleanup_before_handoff()` handles this:

```
1. HAL_CAN_DeInit(&hcan1)        -- return CAN1 to reset state
2. SysTick->CTRL = 0             -- stop the timer
   SysTick->LOAD = 0             -- clear reload value
   SysTick->VAL  = 0             -- clear current count
3. Loop over NVIC->ICER[0..2]    -- disable all 82 interrupt sources
   Loop over NVIC->ICPR[0..2]    -- clear all pending flags
```

**Pitfall -- only clearing ICER[0]:** The STM32F407 has up to 82
interrupts spanning 3 NVIC registers (IRQ 0-31, 32-63, 64-81).  An early
version only cleared register [0], which worked because the bootloader
only uses CAN1 (IRQ 19-22) and SysTick.  But any future peripheral
addition above IRQ 31 would silently break the handoff.  Always loop over
all registers.

### VTOR write ordering

After writing `SCB->VTOR`, a `__DSB()` + `__ISB()` sequence is required
before jumping to the application:

```c
SCB->VTOR = APP_FLASH_START;
__DSB();    // Ensure the VTOR write completes
__ISB();    // Flush the pipeline so vectors are fetched from the new table
```

Without the barrier instructions, the CPU's pipeline could fetch a vector
from the old (bootloader) table during the jump sequence, causing a hard
fault or executing the wrong handler.

---

## 4. Application-to-Bootloader Drop-Back (Freeze)

When the OpenLCB Config Tool sends a Freeze command targeting the firmware
address space (0xEF), the application's `CallbacksConfigMem_freeze()`
executes the drop-back sequence:

```
Step 1:  __disable_irq()             Disable all interrupts globally.
                                     Prevents any ISR from firing between
                                     the shared RAM writes and the reset.

Step 2:  Cache CAN alias             bootloader_shared_ram.cached_alias = node->alias
                                     Bootloader reuses alias (no 200ms
                                     negotiation delay).

Step 3:  Set magic flag              bootloader_shared_ram.request_flag = 0xB00710AD
                                     Tells bootloader: this is a deliberate
                                     drop-back, start with firmware_active=1.

Step 4:  HAL_NVIC_SystemReset()      Reset the entire CPU and all peripherals.
                                     The HAL wrapper includes a __DSB() that
                                     flushes the write buffer so both shared
                                     RAM values are reliably visible afterward.
```

**Why no peripheral teardown before reset?**  On Cortex-M,
`NVIC_SystemReset()` resets ALL peripherals to their power-on state.
There is no need to call `HAL_CAN_DeInit()`, stop SysTick, or clear NVIC
registers.  The `__disable_irq()` call is sufficient to prevent any ISR
from running in the window between the shared RAM writes and the reset
completing.

**Why `__disable_irq()` is required:** Without it, a CAN RX interrupt
could fire between writing the cached alias (step 2) and the reset
(step 4).  The ISR would process a stale CAN message while the shared RAM
handshake is half-written, potentially corrupting state.

---

## 5. Bootloader Startup Sequence

After any reset, the bootloader runs:

```
1. HAL_Init()                    -- reset peripherals, init flash, start SysTick
2. SystemClock_Config()          -- configure PLL to 168 MHz
3. MX_GPIO_Init()                -- configure LED and button pins
4. MX_CAN1_Init()               -- configure CAN1 (does NOT start it)
5. Wire DI structs               -- populate function pointer structs
6. Bootloader_entry():
     a. is_bootloader_requested() -- reads magic flag and button
     b. initialize_hardware(request):
          - Clear bootloader_shared_ram.request_flag unconditionally
          - If NOT app drop-back: clear bootloader_shared_ram.cached_alias (stale)
          - If app drop-back: keep cached alias for reuse
     c. Bootloader_init(request):
          - If valid app and NOT requested: jump to application
          - Otherwise: enter firmware update mode, start polling CAN
```

### CAN initialisation is deferred

`MX_CAN1_Init()` only configures the bxCAN peripheral (bit timing,
mode).  It does NOT start it -- on STM32 the HAL requires an explicit
`HAL_CAN_Start()` call before the peripheral can receive or transmit.
The bootloader's CAN driver calls `HAL_CAN_Start()` on the first call to
`read_received_frame()`, after configuring the acceptance filter.  This
means no CAN interrupts can fire during the startup sequence.

**Why this matters:** On some other platforms, the CAN peripheral starts
generating interrupts as soon as it is initialised.  If the startup
sequence has not finished setting up the handler (or has cleared the
dispatch table), a pending interrupt would call into uninitialised code.
The STM32 HAL's two-step init/start pattern avoids this problem entirely.

---

## 6. Jump to Application

When the bootloader decides to hand off to the application (either because
the application is valid and no bootloader entry was requested, or because
a firmware update completed successfully):

```
1. cleanup_before_handoff()      -- de-init CAN, stop SysTick, clear NVIC
2. jump_to_application():
     a. Read app vector table     -- app_vectors = (uint32_t*)APP_FLASH_START
     b. SCB->VTOR = APP_FLASH_START  -- relocate vector table
        __DSB() + __ISB()         -- ensure pipeline sees new table
     c. __set_MSP(app_vectors[0]) -- load app's stack pointer
     d. app_vectors[1]()          -- jump to app's reset handler
```

The application's reset handler runs the standard GCC startup
(`SystemInit`, `__libc_init_array`, `main`).  From the application's
perspective, it is starting fresh -- all peripherals are in their reset
state and interrupts are disabled.

**Pitfall -- not setting MSP:** The bootloader's stack pointer may be
anywhere in RAM depending on call depth.  The application expects MSP to
point to the top of its own stack (defined in its linker script).  If you
skip `__set_MSP()`, the application will use the bootloader's stale stack
pointer and corrupt memory.

---

## 7. Flash Write and Read-Back Verification

The STM32F407 flash is written in 32-bit words (FLASH_TYPEPROGRAM_WORD)
at the 2.7V-3.6V voltage range (FLASH_VOLTAGE_RANGE_3).  The write
function performs these steps:

```
1. HAL_FLASH_Unlock()
2. For each 4-byte aligned word:
     a. HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest, word)
     b. Read back: *(volatile uint32_t*)dest
     c. Compare read-back to source word
     d. If mismatch: lock flash, return ERROR_PERMANENT
3. Handle partial tail (< 4 bytes): pad with 0xFF, write, verify
4. HAL_FLASH_Lock()
```

**Why read-back verification matters:** `HAL_FLASH_Program()` returning
`HAL_OK` means the flash controller accepted the command.  It does NOT
guarantee the bits landed correctly in the flash cells.  Flash can fail
silently due to wear, voltage droop, or marginal cells.  Reading back each
word immediately after writing catches these failures before the
bootloader declares success and reboots into a corrupted image.

**Why pad with 0xFF:** Erased flash reads as 0xFF on STM32.  When the
last chunk of data is smaller than the 4-byte write alignment, the
remaining bytes are padded with 0xFF so the written word matches what
erased flash would contain in those positions.  This makes the read-back
comparison work correctly for partial writes.

---

## 8. CAN Configuration

The bootloader uses polling (no interrupts) for CAN communication:

- 125 kbps bit rate
- CAN1 on PD0 (RX) / PD1 (TX)
- Extended frames (29-bit identifiers) for OpenLCB
- Single acceptance filter: bank 0, mask mode, accept all IDs

### CubeMX CAN settings

```
Prescaler         = 42
Time Segment 1    = 3 TQ (CAN_BS1_3TQ)
Time Segment 2    = 4 TQ (CAN_BS2_4TQ)
Sync Jump Width   = 1 TQ (CAN_SJW_1TQ)
Mode              = Normal
Auto Retransmit   = Disabled
Auto Bus Off      = Disabled
```

These settings produce 125 kbps at the 42 MHz APB1 clock (168 MHz SYSCLK
with AHB /2 and APB1 /2).

### TX mailbox check

The TX function checks `HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3`
before sending.  This means it only transmits when ALL 3 mailboxes are
free.  This is intentionally conservative -- the bootloader sends one
frame at a time and waits for it to complete rather than queuing multiple
frames.

---

## 9. Clock Configuration

- 8 MHz external crystal (HSE)
- PLL: PLLM /4, PLLN x168, PLLP /2
- SYSCLK = 168 MHz
- AHB = 84 MHz (HCLK_DIV2)
- APB1 = 42 MHz (HCLK_DIV2) -- CAN1 clock domain
- APB2 = 42 MHz (HCLK_DIV2)

### Timer

The bootloader does not use a dedicated hardware timer.  Instead it
derives the 100ms tick from `HAL_GetTick()`:

```c
return (uint8_t)(HAL_GetTick() / 100U);
```

`HAL_GetTick()` is incremented by SysTick_Handler every 1 ms.  Dividing
by 100 and casting to `uint8_t` gives a value that increments every 100ms
and wraps at 256, matching the bootloader library's expected interface.

---

## 10. CubeMX Considerations

Unlike the dsPIC MCC workflow, STM32 CubeMX does NOT require hand-edits
to generated files for interrupt forwarding.  The Cortex-M VTOR mechanism
handles vector table switching automatically (see section 3).

### What CubeMX generates

The bootloader project uses CubeMX to generate:

- `main.c` -- startup sequence (HAL_Init, clocks, GPIO, CAN1)
- `stm32f4xx_it.c` -- interrupt handlers (SysTick_Handler)
- `stm32f4xx_hal_msp.c` -- low-level peripheral init (GPIO alternate
  functions for CAN pins)

The bootloader-specific code (DI struct wiring, `Bootloader_entry()` call)
is placed in `USER CODE` sections that CubeMX preserves during
regeneration.

### Safe to regenerate

CubeMX preserves code between `USER CODE BEGIN` / `USER CODE END` markers.
Regenerating the project will NOT lose the bootloader integration code.
The linker script (`STM32F407VGTX_BOOTLOADER.ld`) is a custom file that
CubeMX does not touch.

---

## 11. Building and Flashing

### Why mass erase matters

The BasicNodeBootloader debug configuration has `monitor flash mass_erase`
in the Startup tab's Initialization Commands.  This tells the ST-Link
debugger to perform a FULL CHIP ERASE every time you start a debug session
for the bootloader.  That wipes all 12 sectors -- including the
application in sectors 2-10.

This is there for a reason.  If you flash a new bootloader WITHOUT mass
erase, the old application is still sitting in the application sectors.
On the next reset the new bootloader sees a valid application image, no
magic flag, no button pressed -- and jumps straight to the old app.  You
never enter bootloader mode and never get to test your new bootloader
code:

```
You flash a new bootloader -->  only sectors 0-1 are rewritten
  without mass erase
You power cycle            -->  bootloader finds the OLD app still valid
                                in sectors 2-10
                           -->  jumps to the old app immediately
You wonder why your        -->  it never ran -- the old app took over
  bootloader changes
  have no effect
```

The fix is to either mass erase (so there is no application to jump to)
or hold the B1 (blue) button during reset to force bootloader mode
regardless of what is in the application sectors.

**When to use mass erase:**

- **Development (default):** Leave `monitor flash mass_erase` in the
  BasicNodeBootloader debug configuration.  After flashing the bootloader,
  flash the application.  Both binaries start from a clean slate.

- **Quick bootloader testing:** Mass erase, flash only the bootloader.
  With no application present the bootloader stays in bootloader mode
  and waits for firmware via the Config Tool.

**When to skip mass erase:**

- **Bootloader-only update with button entry:** Remove
  `monitor flash mass_erase`, flash the bootloader, then hold B1 during
  reset to force bootloader mode.  The old application is still present
  but the button bypasses the "jump to app" logic.

- **Application-only update:** The BasicNode debug configuration should
  NOT have `monitor flash mass_erase`.  It only rewrites the application
  sectors, leaving the bootloader in sectors 0-1 intact.  Alternatively,
  use the OpenLCB Config Tool for over-the-network firmware updates (see
  below).

**Where to find this setting in STM32CubeIDE:**

1. Run > Debug Configurations
2. Select the BasicNodeBootloader launch configuration
3. Click the Startup tab
4. Look in the Initialization Commands text box
5. Add or remove `monitor flash mass_erase` as needed

### Reset behaviour -- use "Hardware reset"

After ST-Link programs the flash, the CPU must be reset so the new code
starts executing.  The "Reset behaviour" dropdown on the Debugger tab
controls how this happens.  There are several options but only one works
reliably:

- **Software system reset** -- resets the CPU core but may NOT fully
  reset all peripherals.  The PLL and clock tree can be left in an
  undefined state, which means `HAL_Init()` configures SysTick against
  the wrong clock frequency.  Symptom: SysTick does not run after
  programming, but a hard reboot (power cycle or reset button) fixes it.

- **Hardware reset** -- pulses the physical NRST pin via ST-Link.  This
  is electrically identical to pressing the reset button.  All
  peripherals, the clock tree, and the CPU core are fully reset to their
  power-on state.  SysTick works correctly every time.

**Use "Hardware reset" for both the BasicNodeBootloader and BasicNode
debug configurations.**  It produces the same result as a power cycle
and avoids subtle peripheral initialisation failures that are difficult
to diagnose.

**Where to set it:**

1. Run > Debug Configurations
2. Select the launch configuration (BasicNodeBootloader or BasicNode)
3. Click the Debugger tab
4. Find the "Reset behaviour" dropdown
5. Set it to "Hardware reset"

### First-time setup (both binaries)

1. Open both projects in STM32CubeIDE
2. Build BasicNodeBootloader first (Release configuration)
3. Flash the bootloader via ST-Link -- mass erase wipes the chip clean
4. Build BasicNode (Release configuration)
5. Flash the application via ST-Link -- this writes sectors 2+

After both are programmed, power-cycle the board.  The bootloader will
check for the magic flag (not set), check the button (not pressed), and
jump to the application.

### Firmware update via OpenLCB Config Tool

1. Build the application and prepare the binary image
2. In the Config Tool, connect to the node
3. Send a Freeze command to the firmware address space (0xEF)
4. The application drops back to the bootloader (see section 4)
5. The Config Tool sends the binary data via Memory Configuration
   datagrams
6. The bootloader erases each sector as needed, writes the data with
   read-back verification, and reboots
7. The bootloader validates the new application and jumps to it

### Bootloader button

Hold the B1 (blue) button on PA0 during power-on or reset to force
bootloader mode.  This is useful when the application is corrupted or
missing.  The bootloader starts as a normal OpenLCB node
(`firmware_active = 0`) and waits for the Config Tool to send a Freeze
command before accepting firmware data.

---

## 12. File Reference

### Bootloader project (BasicNodeBootloader)

| File | Purpose |
|------|---------|
| `Core/Src/bootloader_main.c` | Entry point; wires DI structs, calls `Bootloader_entry()` |
| `STM32F407VGTX_BOOTLOADER.ld` | Linker script: FLASH 0x08000000 (32K), SHARED_NOINIT at 0x2001FFF0 |
| `Core/Src/application_drivers/bootloader_drivers_can.c/h` | CAN physical layer DI (polling RX/TX, alias cache) |
| `Core/Src/application_drivers/bootloader_drivers_openlcb.c/h` | OpenLCB/hardware DI (flash, lifecycle, LED, timer, identity) |
| `Core/Src/stm32f4xx_it.c` | SysTick_Handler (calls HAL_IncTick for 1ms tick) |
| `Core/Src/stm32f4xx_hal_msp.c` | Low-level CAN1 GPIO alternate function init |

### Application project (BasicNode)

| File | Purpose |
|------|---------|
| `Core/Src/main.c` | Entry point; library init, main loop |
| `STM32F407VGTX_APPLICATION.ld` | Linker script: FLASH 0x08008000 (864K), SHARED_NOINIT at 0x2001FFF0 |
| `Core/Src/application_callbacks/callbacks_config_mem.c` | Freeze handler (drop-back to bootloader) |
| `Core/Src/application_callbacks/callbacks_can.c` | CAN activity LED callbacks |
| `Core/Src/application_drivers/stm32_driverlib_can_driver.h` | Platform CAN driver |

### Shared

| File | Purpose |
|------|---------|
| `shared/bootloader_shared_ram.h` | Magic flag and cached alias declarations |
| `shared/bootloader_shared_ram.c` | Variable definitions with `.noinit` section attribute |
