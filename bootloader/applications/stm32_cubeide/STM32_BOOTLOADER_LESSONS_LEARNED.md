# STM32 Bootloader Implementation — Lessons Learned

This document records the non-obvious problems, surprises, and design decisions
encountered while porting the OpenLCB standalone bootloader to the
STM32F407G-DISC1 (STM32F407VGTx) Discovery board using STM32CubeIDE.  It is
written for a developer who is familiar with embedded C but may not have
worked with the STM32 HAL, the bxCAN peripheral, or multi-image flash
layouts before.

---

## 1. Use `HAL_NVIC_SystemReset()`, Not `NVIC_SystemReset()`

### What the code needs to do
When the running application receives an OpenLCB *Firmware Upgrade* (Freeze)
command it must write a magic handshake value and the current CAN alias into
a shared RAM region, then immediately reset the CPU so the bootloader starts
and sees those values.

### The problem with the bare CMSIS call
`NVIC_SystemReset()` is a raw CMSIS function.  It sets the SYSRESETREQ bit in
the ARM SCB AIRCR register to request a reset, then enters an infinite loop
waiting for the reset to occur.  What it does **not** do is flush the CPU
write buffer before requesting the reset.

Modern ARM Cortex-M cores have a write buffer.  A store instruction
(`*ptr = value`) can return before the value has reached SRAM.  If the reset
fires while the write is still buffered, SRAM may never be updated and the
bootloader will read stale (or zero) values.

### The correct call
```c
HAL_NVIC_SystemReset();   // correct
```

The STM32 HAL wrapper inserts a `__DSB()` (Data Synchronisation Barrier)
instruction before requesting the reset.  `__DSB()` stalls the CPU until
every pending write has completed and reached its destination memory.  Only
then is the reset requested.  This guarantees that both
`bootloader_request_flag` and `bootloader_cached_alias` are safely written to
SRAM before the CPU resets.

### Where to apply it
- Application freeze callback (`callbacks_config_mem.c`)
- Bootloader reboot function (`bootloader_drivers_openlcb.c`)

---

## 2. STM32 bxCAN TX Mailboxes Are a Priority Queue, Not a FIFO

### Background
The STM32F4 bxCAN peripheral has **three independent transmit mailboxes**
(mailbox 0, 1, 2).  When multiple mailboxes contain pending frames the
hardware decides which one to put on the bus first.  The decision rule is:

1. The frame with the **lowest CAN identifier** wins (standard CAN
   arbitration priority).
2. If two frames have **equal identifiers**, the frame in the
   **lowest-numbered mailbox** wins.

### Why this breaks multiframe OpenLCB messages
OpenLCB multiframe messages (SNIP reply, PIP reply, datagram) send several
CAN frames in sequence.  All frames in a sequence share the same CAN
identifier.

The original `try_send_frame` function returned `true` (success) as long as
**at least one** mailbox was free:
```c
// WRONG — allows up to 3 frames in flight simultaneously
if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    return false;
}
```

What happened in practice:
- Frame 1 was loaded into mailbox 0 and started transmitting.
- While frame 1 was still on the wire, frames 2 and 3 were loaded into
  mailboxes 1 and 2.
- When frame 1 finished, mailbox 0 became free.  Frame 4 was loaded into
  mailbox 0.
- The hardware now had frames 4, 2, 3 pending.  Equal identifiers →
  mailbox 0 wins → **frame 4 transmitted before frames 2 and 3**.
- Frames 5, 6, 7 continued to win over frames 2 and 3 for the same reason.
- Frames 2 and 3 were transmitted last, after the "FINAL" flag had already
  been sent with frame 5.

The receiving node saw the frames completely out of order and could not
reassemble the message correctly.

### The fix
Only accept a new frame when **all three mailboxes are free**:
```c
// CORRECT — only one frame in flight at a time
if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {
    return false;
}
```

Because `_block_send()` spins calling `try_send_frame()` until it returns
`true`, this enforces a strictly sequential, one-frame-at-a-time policy.
Every frame always goes into mailbox 0 (the first free mailbox the HAL
finds), so ordering is guaranteed.

### The application driver already had this right
`stm32_driverlib_can_driver.c` in the BasicNode application uses a boolean
`_is_transmitting` flag cleared by the TX-mailbox-complete interrupt
callback.  A new frame is only submitted when `_is_transmitting` is false.
There is even a comment in that file: *"We only send one at a time so only
MailBox 0 will ever be used."*  The bootloader driver was written
independently and missed this lesson.

---

## 3. Shared RAM Requires a `.noinit` Linker Section

### What "shared RAM" means
The application and the bootloader are two separate firmware images.  They
need to exchange two values across a software reset:

| Variable | Type | Purpose |
|---|---|---|
| `bootloader_request_flag` | `uint32_t` | Magic value `0xB00710AD` written by the app to tell the bootloader "I asked for this reset — enter upgrade mode immediately" |
| `bootloader_cached_alias` | `uint16_t` | The CAN alias the app was using, passed to the bootloader so it can rejoin the network quickly |

A software reset (`HAL_NVIC_SystemReset()`) resets the CPU and peripherals
but does **not** clear SRAM.  The values survive — provided the C startup
code does not zero-initialise them.

### The problem with normal variables
Every C variable that is not explicitly initialised (and many that are) lives
in the `.bss` section.  The startup code (`Reset_Handler` in
`startup_stm32f407vgtx.s`) zeroes the entire `.bss` section before calling
`main()`.  If the shared variables are in `.bss` they will be zeroed on every
reset, including the soft reset the app just triggered.  The bootloader will
read zeros and think no upgrade was requested.

### The solution — `.noinit`
The variables are placed in a special linker section that the startup code
skips:
```c
// bootloader_shared_ram.c
__attribute__((section(".noinit")))
volatile uint32_t bootloader_request_flag;

__attribute__((section(".noinit")))
volatile uint16_t bootloader_cached_alias;
```

The linker script must define a matching memory region and section for both
the bootloader and the application — they **must map to the same physical
SRAM addresses** or the two images will read and write different locations.

```ld
/* In both STM32F407VGTX_BOOTLOADER.ld and STM32F407VGTX_APPLICATION.ld */
MEMORY
{
  RAM           (xrw) : ORIGIN = 0x20000000, LENGTH = 0x1FFF0   /* 128KB - 16B */
  SHARED_NOINIT (xrw) : ORIGIN = 0x2001FFF0, LENGTH = 0x10      /* 16B at top of RAM */
  ...
}

SECTIONS
{
  ...
  .noinit (NOLOAD) :
  {
    KEEP(*(.noinit))
  } >SHARED_NOINIT
}
```

The `NOLOAD` keyword tells the linker this section is never loaded from flash
— it only exists in RAM.  The startup code zeros `.bss` (which maps to the
`RAM` region) but never touches `SHARED_NOINIT`.

---

## 4. CubeIDE Regenerates the Linker Script — Rename Your Custom One

### What CubeIDE does
STM32CubeIDE generates a linker script named `STM32F407VGTX_FLASH.ld` (and
`STM32F407VGTX_RAM.ld`).  Every time you press *Project → Generate Code*
from the `.ioc` file, CubeIDE **overwrites** that file with a freshly
generated version.  Any custom modifications you made are silently lost.

### The solution
Create custom linker scripts with different names:
- `STM32F407VGTX_BOOTLOADER.ld` for the bootloader project
- `STM32F407VGTX_APPLICATION.ld` for the application project

Then update each project's `.cproject` XML file to reference the new name.
The relevant setting is the *Linker Script (-T)* option in the MCU GCC Linker
tool.  The original `STM32F407VGTX_FLASH.ld` can remain in the project
directory — it will still be regenerated but it is no longer referenced by
the build, so it has no effect.

The custom scripts differ from the generated default in the following ways:

| Setting | Bootloader | Application |
|---|---|---|
| `FLASH ORIGIN` | `0x08000000` (sector 0) | `0x08008000` (sector 2) |
| `FLASH LENGTH` | `32K` (sectors 0-1) | `864K` (sectors 2-10) |
| `RAM LENGTH` | `128K - 16B = 0x1FFF0` | same |
| `SHARED_NOINIT` region | added | added |
| `.noinit (NOLOAD)` section | added | added |
| `.app_header` section | not needed | added at `0x08008200` |

---

## 5. Flashing Does Not Erase the Whole Chip

### What actually happens
When CubeIDE flashes a binary via OpenOCD and ST-LINK it performs a
*sector-by-sector erase* of only the flash sectors that the new binary
occupies.  It does **not** perform a full chip erase.

The STM32F407G-DISC1 Discovery board ships from ST with a demonstration
application pre-programmed across the **entire** 1 MB flash.

Flashing the 32 KB bootloader image erases and reprograms sectors 0 and 1
only.  The ST demo application remains intact in sectors 2-11
(`0x08008000` – `0x080FFFFF`).

### Why this matters
On power-up the bootloader checks whether the application region is blank:
```c
if (((const uint32_t *) flash_min)[0] != 0xFFFFFFFF) {
    jump_to_application();
}
```

The first word at `0x08008000` is the ST demo's initial stack pointer — not
`0xFFFFFFFF`.  The bootloader jumps to the ST demo rather than staying in
bootloader mode.  The only way to prevent this (before the real application
is flashed) is to hold the user button during power-on, which forces
`BOOTLOADER_REQUESTED_BY_BUTTON` and bypasses the jump.

### How to perform a full chip erase in CubeIDE (later versions)
The *Erase Full Chip* checkbox that older CubeIDE versions had in the
Download Options tab has been removed in later releases.  Use a GDB
initialisation command instead — this runs automatically every time you
start a debug or flash session:

1. Click the dropdown arrow next to the debug icon (🐛) and select
   **Debug Configurations…**
2. Select your active configuration from the
   *STM32 Cortex-M C/C++ Application* list on the left.
3. Click the **Startup** tab.
4. In the **Initialization Commands** text box add:
   ```
   monitor flash mass_erase
   ```
5. Click **Apply**, then **Debug** (or **Run**).

CubeIDE will execute the mass erase before downloading the new firmware on
every session.  Remove the command once the real application is flashed and
the ST demo is no longer a concern.

Alternatively, use **STM32CubeProgrammer** for a one-time manual erase:
connect to the device, select *Full chip erase*, then reflash.

---

## 6. The Bootloader Jumps via the Vector Table, Not the First Byte of Flash

### A common misconception
It might seem like the bootloader should jump to the start of the application
flash (`APP_FLASH_START = 0x08008000`).  If it did, it would try to execute
the Cortex-M4 exception vector table as if it were code — which would crash
immediately.

### How ARM Cortex-M actually boots
The ARM Cortex-M architecture boots using an **exception vector table**.
This table lives at the start of the image and is a simple array of 32-bit
addresses:

| Offset | Content |
|---|---|
| `+0x00` | Initial stack pointer value |
| `+0x04` | Address of `Reset_Handler` (the actual first instruction) |
| `+0x08` | Address of `NMI_Handler` |
| ... | ... |

The bootloader uses this table directly:
```c
uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;

SCB->VTOR = APP_FLASH_START;          // tell the CPU where the new table is
__set_MSP(app_vectors[0]);            // load the app's initial stack pointer
void (*reset)(void) = (void (*)(void)) app_vectors[1];
reset();                              // jump to Reset_Handler
```

`Reset_Handler` lives in `.text`, which the linker places after both the
vector table and the `app_header` struct.  The bootloader simply follows the
pointer and arrives at the correct place regardless of how much space the
vector table and header occupy.

### Why this also means the `.app_header` location is not a jump problem
The `bootloader_app_header_t` struct is placed at `APP_FLASH_START + 0x200`
(= `0x08008200`) — between the end of the vector table and the start of
`.text`.  This region is never fetched as instructions.  No special
"jump over it" logic is needed.

---

## 7. Peripherals Must Be Shut Down Before Handing Off to the Application

### Why
When the bootloader finishes and jumps to the application, both firmware
images share the same hardware.  If the bootloader leaves peripherals running
the application's `HAL_Init()` and `MX_*_Init()` calls will try to
re-initialise them from scratch.  At best this causes redundant
configuration; at worst it causes hard faults or incorrect peripheral state.

More importantly, the CAN peripheral must be de-initialised before the jump.
The application sets up its own CAN configuration, baud rate, filters, and
interrupt handlers.  If the bootloader's CAN is still active the application
initialisation will fail or produce incorrect behaviour.

### What `cleanup_before_handoff` does
```c
void STM32_DriverLibDrivers_cleanup_before_handoff(void) {

    extern CAN_HandleTypeDef hcan1;
    HAL_CAN_DeInit(&hcan1);        // stop CAN, disable CAN clock

    SysTick->CTRL = 0;             // stop SysTick timer
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    NVIC->ICER[0] = 0xFFFFFFFF;   // disable all interrupts
    NVIC->ICPR[0] = 0xFFFFFFFF;   // clear all pending interrupt flags
}
```

The application's `Reset_Handler` and `HAL_Init()` will then start from a
clean state.

---

## 8. `finalize_flash` Is Intentionally Left as a TODO Stub

### What it is
`finalize_flash` is called by the bootloader library after it has received
and written all firmware packets to flash.  Its job is to validate the
newly-written image before the bootloader declares the upgrade complete.

### Why it cannot validate *before* writing
The incoming firmware image may be up to 864 KB.  The STM32F407 has only
128 KB of RAM.  It is impossible to buffer the entire image in RAM before
writing it.  The image must be written to flash incrementally as packets
arrive.

### What `finalize_flash` should do (when implemented)
After all writing is complete the full image is in flash and can be read
back.  `finalize_flash` should:

1. Read the entire image back from flash.
2. Compute the CRC-16-IBM (triple variant) over the pre-header and
   post-header regions.
3. Compare the computed checksums against the values stored in the
   `bootloader_app_header_t` struct that was part of the downloaded image.
4. If the checksums match — return success.  The bootloader will reboot
   into the new application.
5. If the checksums do not match — erase the entire application region so
   the first word reads `0xFFFFFFFF`.  The bootloader will stay resident on
   the next power-up and wait for a retry upload.

### The hard reality
Once the bootloader starts erasing and writing new firmware the old
application is gone.  A corrupted download cannot restore it.  The best
outcome of a failed `finalize_flash` is an orderly "no valid app" state
that allows a retry — not a recovery of the previous image.  This is the
fundamental trade-off of a single-bank, no-staging-area flash layout.

---

## 9. `NO_CHECKSUM` Must Be Enabled During Development

### What the checksum system does
On every cold boot (no magic flag, no button held) the bootloader validates
the application image by computing CRC-16-IBM checksums over two regions of
flash and comparing them to values stored in the `bootloader_app_header_t`
struct embedded in the image.  If either checksum fails the bootloader stays
resident — the application never runs.

### Why development images always fail the checksum
The `bootloader_app_header_t` struct (placed at `APP_FLASH_START + 0x200`)
must be **patched by a post-link tool** after the linker produces the final
binary.  The tool computes the actual checksums of the linked image and writes
them into the struct in the binary file.  Without the tool the struct contains
all zeros, and the CRC check will always fail.

### The `NO_CHECKSUM` flag
Defining `NO_CHECKSUM` in
`openlcb_bootloader_src/openlcb/bootloader_types.h` disables the checksum
validation entirely.  The bootloader simply checks that the first word at
`APP_FLASH_START` is not `0xFFFFFFFF` (i.e., the flash is not blank) and
jumps to the application.

```c
/* Uncomment to bypass CRC validation — development use only */
#define NO_CHECKSUM
```

The TI reference implementation ships with `NO_CHECKSUM` enabled.  The STM32
version should match this for development.  Comment it back out (disable it)
when the post-link CRC patching tool is integrated into the build system.

---

## 10. Freeze Belongs in `callbacks_config_mem`, Not `callbacks_olcb`

### Why it matters for code organisation
The OpenLCB *Freeze* command is part of the **Configuration Memory** protocol
— it is a datagram sent to the configuration memory space `0xEF` (firmware
space).  The handler therefore belongs alongside the other configuration
memory callbacks (`factory_reset`, address space queries, etc.) in
`callbacks_config_mem.c`.

Placing it in `callbacks_olcb.c` (the general OpenLCB event/node callbacks)
is technically functional but misleading and inconsistent with the TI
reference implementation.

### What the callback does
```c
void CallbacksConfigMem_freeze(
        openlcb_statemachine_info_t *statemachine_info,
        config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space
            == CONFIG_MEM_SPACE_FIRMWARE) {

        // cleanup_before_handoff() is called automatically by the library

        bootloader_cached_alias = statemachine_info->openlcb_node->alias;
        bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC;

        HAL_NVIC_SystemReset();   // DSB barrier flushes writes before reset
    }
}
```

`unfreeze` and `firmware_write` are both set to `NULL` in the application's
`openlcb_config` struct — the bootloader owns those operations, not the
application.

---

## 11. STM32F407 Flash Is Non-Uniform — There Is No Single `FLASH_SECTOR_SIZE`

### The sector layout
Unlike many microcontrollers that have uniform flash sectors, the STM32F407
single-bank 1 MB device has sectors of three different sizes:

| Sectors | Count | Size each | Address range |
|---|---|---|---|
| 0 – 3 | 4 | 16 KB | `0x08000000` – `0x0800FFFF` |
| 4 | 1 | 64 KB | `0x08010000` – `0x0801FFFF` |
| 5 – 11 | 7 | 128 KB | `0x08020000` – `0x080FFFFF` |

**Total: 1 MB**

### Flash partitioning for the bootloader project

| Region | Sectors | Size | Start address |
|---|---|---|---|
| Bootloader | 0 – 1 | 32 KB | `0x08000000` |
| Application | 2 – 10 | 864 KB | `0x08008000` |
| Node ID storage | 11 | 128 KB | `0x080E0000` |

### Implications for erasing
Flash can only be erased one **whole sector** at a time.  The bootloader must
look up the correct sector number and size from a table before erasing.  A
constant `FLASH_SECTOR_SIZE` is defined in `bootloader_drivers_openlcb.h` as
`16384U` (the smallest sector size) but this is only a reference value — the
actual erase code uses a full sector table:

```c
static const flash_sector_entry_t _flash_sectors[] = {
    { 0x08000000U,  16384U, FLASH_SECTOR_0 },
    { 0x08004000U,  16384U, FLASH_SECTOR_1 },
    { 0x08008000U,  16384U, FLASH_SECTOR_2 },
    { 0x0800C000U,  16384U, FLASH_SECTOR_3 },
    { 0x08010000U,  65536U, FLASH_SECTOR_4 },
    { 0x08020000U, 131072U, FLASH_SECTOR_5 },
    ...
};
```

If you attempt to erase using the wrong size you will either fail to erase a
complete sector (leaving stale data) or erase more than intended (destroying
adjacent code or data).

---

## 12. The Firmware File to Upload Is a `.bin`, Not an `.elf` or `.hex`

### What the bootloader expects
The OpenLCB firmware upgrade protocol transfers the new application as a
stream of raw bytes written via configuration memory write datagrams to the
firmware address space (`0xEF`).  The bootloader writes those bytes directly
to flash starting at `APP_FLASH_START` (`0x08008000`).  It has no parser for
any file format — it just stores raw bytes.

The file to upload is therefore a **flat binary `.bin` file**:

| File | Suitable? | Reason |
|---|---|---|
| `.bin` | ✅ Yes | Flat binary image, byte 0 = first byte at `APP_FLASH_START` |
| `.elf` | ❌ No | ELF container with section headers, symbol tables, debug info |
| `.hex` | ❌ No | Intel HEX text format with address records — no HEX parser in bootloader |

### Layout of the `.bin` file
```
Offset 0x000  →  APP_FLASH_START (0x08008000)  : vector table (392 bytes)
Offset 0x188  →  0x08008188                    : padding to 0x200 (0xFF)
Offset 0x200  →  0x08008200                    : bootloader_app_header_t (36 bytes, zeros until CRC tool patches them)
Offset 0x224  →  0x08008224                    : .text (Reset_Handler and all code)
...
```

### Generating the `.bin` in CubeIDE
CubeIDE does not produce a `.bin` by default.  Enable it once in
*Project Properties → C/C++ Build → Settings → MCU Post build outputs* by
ticking **Convert to binary file (-O binary)**.  After that the `.bin` file
appears in the `Debug/` (or `Release/`) folder alongside the `.elf` after
every build.

### Note on the `app_header` while `NO_CHECKSUM` is active
While `NO_CHECKSUM` is defined in the bootloader, the `app_header` fields
(all zeros) are never checked.  The `.bin` can be uploaded and will run
correctly.  When `NO_CHECKSUM` is eventually removed a post-link tool must
patch the CRC fields in the `.bin` before it is uploaded, otherwise the
bootloader will reject the image on the next cold boot.

---

## Summary Checklist for a New STM32 Bootloader Port

- [ ] Use `HAL_NVIC_SystemReset()` everywhere a soft reset is needed
- [ ] `try_send_frame` must require all TX mailboxes free (`FreeLevel == 3`)
- [ ] Add `SHARED_NOINIT` memory region and `.noinit (NOLOAD)` section to **both** linker scripts at the **same physical address**
- [ ] Name custom linker scripts differently from the CubeIDE-generated defaults
- [ ] Do a full chip erase on first board bring-up (ST demo occupies the whole flash)
- [ ] Enable `NO_CHECKSUM` in the bootloader until the post-link CRC tool is ready
- [ ] Place the `bootloader_app_header_t` placeholder instance (all zeros) in `.app_header` section in the application
- [ ] Implement `cleanup_before_handoff` — at minimum: de-init CAN, stop SysTick, disable/clear all NVIC
- [ ] Implement `finalize_flash` — read back, verify CRC, erase on failure
- [ ] Build and verify the sector lookup table matches your device's actual flash geometry

---

## 11. `.noinit` Cleanup Must Use the `request` Parameter, Not Reset-Cause Registers

**Problem:** The original `initialize_hardware()` checked STM32 reset-cause flags (`RCC->CSR & RCC_CSR_SFTRSTF`, `RCC_CSR_PORRSTF`, etc.) to decide whether to clear shared `.noinit` variables (`bootloader_request_flag`, `bootloader_cached_alias`). This approach has several failure modes:

1. **Stale flags:** `RCC->CSR` reset-cause bits are sticky -- they accumulate across resets until manually cleared with `__HAL_RCC_CLEAR_RESET_FLAGS()`. After a POR followed by a soft reset, BOTH `PORRSTF` and `SFTRSTF` are set, making it ambiguous which reset just occurred.

2. **Stale `bootloader_cached_alias`:** On POR or programmer reset, `bootloader_cached_alias` in `.noinit` RAM contains garbage (whatever was in SRAM). If the CAN state machine picks up a non-zero garbage value, it uses it as the 12-bit alias without CID negotiation. A 16-bit garbage value overflows the alias field and corrupts the MTI bits of the CAN identifier, producing a malformed CAN frame that no node recognises.

3. **Flag-clearing order:** If the flag was consumed (cleared) in `is_bootloader_requested()` and then `initialize_hardware()` checked it again, the second check always saw zero. This was part of the double-call bug fixed in the library.

**Solution:** `initialize_hardware(request)` now takes the `bootloader_request_t` parameter and bases all cleanup decisions on it:

```c
void BootloaderDriversOpenlcb_initialize_hardware(bootloader_request_t request) {
    /* Always clear the magic flag */
    bootloader_request_flag = 0;

    /* Clear cached alias on every path except app drop-back */
    if (request != BOOTLOADER_REQUESTED_BY_APP) {
        bootloader_cached_alias = 0;
    }

    /* ... peripheral init ... */
}
```

No `RCC->CSR` inspection is needed. No `__HAL_RCC_CLEAR_RESET_FLAGS()` call is needed. The `request` parameter from `is_bootloader_requested()` is unambiguous: `BOOTLOADER_REQUESTED_BY_APP` means the application wrote the magic flag and the cached alias is valid; anything else means the alias is stale garbage.

**This pattern is consistent across all platforms** (dsPIC, ESP32, TI MSPM0, Template).
