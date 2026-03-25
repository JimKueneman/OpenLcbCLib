# MSPM0G3507 Launchpad — Bootloader Instrumentation Guide

The production bootloader image contains **no debug output** to keep the flash
footprint as small as possible.  This guide describes how to re-add visibility
when you need to debug a problem.

---

## Option 1 — XDS110 Backchannel UART (recommended for bring-up)

The LaunchPad's XDS110 debug probe exposes a virtual COM port over USB.
UART_0 on the MSPM0G3507 is wired to the XDS110 backchannel (PA10 TX / PA11 RX
by default on the LP-MSPM0G3507 board).

### Step 1 — Add UART_0 to the syscfg

Open `mspm03507_bootloader/bootloader.syscfg` in SysConfig and add a
**UART Main** instance:

| Setting | Value |
|---|---|
| Instance name | `UART_0` |
| Baud rate | 115200 |
| TX pin | PA10 (UART0_TX, backchannel) |
| RX pin | PA11 (UART0_RX, backchannel) — leave disabled if RX not needed |
| Hardware flow control | None |

SysConfig will regenerate `Debug/ti_msp_dl_config.h` and
`Debug/ti_msp_dl_config.c` with the `UART_0_INST` macro and the
`SYSCFG_DL_UART_0_init()` call wired into `SYSCFG_DL_init()`.

### Step 2 — Restore uart_debug.c / uart_debug.h

Create `application_drivers/uart_debug.h`:

```c
#ifndef __UART_DEBUG__
#define __UART_DEBUG__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern void UartDebug_putchar(char c);
extern void UartDebug_print(const char *s);
#ifdef __cplusplus
}
#endif
#endif /* __UART_DEBUG__ */
```

Create `application_drivers/uart_debug.c`:

```c
#include "../Debug/ti_msp_dl_config.h"
#include "uart_debug.h"

void UartDebug_putchar(char c) {
    DL_UART_Main_transmitDataBlocking(UART_0_INST, (uint8_t) c);
}

void UartDebug_print(const char *s) {
    while (*s)
        DL_UART_Main_transmitDataBlocking(UART_0_INST, (uint8_t) *s++);
}
```

### Step 3 — Add the include to the files you want to instrument

```c
#include "uart_debug.h"          /* from files in application_drivers/ */
#include "../../application_drivers/uart_debug.h"   /* from src/openlcb/ */
```

### Step 4 — Add print calls

Typical boot-path instrumentation points:

```c
/* bootloader_drivers_openlcb.c — end of initialize_hardware() */
UartDebug_print("\r\n--- BOOTLOADER START ---\r\n");

/* bootloader_drivers_openlcb.c — is_bootloader_requested() */
UartDebug_print("MAGIC=0x");
/* ... hex-format bootloader_request_flag ... */
UartDebug_print(" BTN=");
UartDebug_print(DL_GPIO_readPins(BOOTLOADER_BUTTON_PORT, BOOTLOADER_BUTTON_PIN) == 0 ? "DOWN" : "UP");
UartDebug_print("\r\n");

/* bootloader_drivers_openlcb.c — jump_to_application() */
UartDebug_print("JUMP\r\n");

/* bootloader.c — Bootloader_entry() boot-path decisions */
UartDebug_print("BOOT: normal cold boot\r\n");
UartDebug_print("BOOT: app present - jumping to app\r\n");
UartDebug_print("BOOT: app dropped back - magic matched\r\n");
UartDebug_print("BOOT: button held - waiting for Freeze\r\n");
UartDebug_print("BOOT: no app (flash erased) - staying in bootloader\r\n");
```

### Step 5 — Connect a terminal

Any 115200-8-N-1 terminal works.  On macOS/Linux:

```
screen /dev/tty.usbmodem<XDS110-port> 115200
```

On Windows use PuTTY or TeraTerm on the COM port assigned to the XDS110.

---

## Option 2 — GPIO Toggle (no UART, minimal overhead)

Useful for timing critical sections or confirming execution reached a point
without pulling in the UART driver.

```c
/* Quick pulse on any spare GPIO — configure it as output in syscfg first */
DL_GPIO_setPins(DEBUG_PORT, DEBUG_PIN);
DL_GPIO_clearPins(DEBUG_PORT, DEBUG_PIN);
```

Observe with a logic analyser or oscilloscope.

---

## Option 3 — SEGGER RTT (lowest overhead, no UART pins needed)

If you have a SEGGER J-Link (or a J-Link OB firmware flashed to the XDS110),
SEGGER RTT works over the existing SWD connection with no extra pins.

1. Add the SEGGER RTT source files (`SEGGER_RTT.c`, `SEGGER_RTT.h`) to the
   project.
2. Replace `UartDebug_print(s)` with `SEGGER_RTT_WriteString(0, s)`.
3. View output in **J-Link RTT Viewer** or via `JLinkRTTLogger`.

RTT adds ~1 KB of flash.  It is the preferred approach once JTAG bring-up is
stable.

---

## Removing instrumentation before a production build

1. Delete (or do not add) `uart_debug.c` and `uart_debug.h`.
2. Remove all `#include "uart_debug.h"` lines.
3. Remove all `UartDebug_*` call sites.
4. Remove `UART_0` from `bootloader.syscfg` and regenerate.
5. Rebuild — the TI DL UART driver code is no longer compiled in.

The linker will discard any unreferenced DL library functions, so only the
functions actually called from the bootloader contribute to flash usage.
