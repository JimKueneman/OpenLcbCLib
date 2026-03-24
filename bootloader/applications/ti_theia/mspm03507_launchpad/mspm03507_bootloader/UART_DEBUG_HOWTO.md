# UART Debug Output on LP-MSPM0G3507

Quick reference for getting `printf`-style debug output over the XDS110 USB
backchannel UART on the MSPM0G3507 LaunchPad.

---

## Hardware

- Backchannel UART is **UART0** on **PA10** (TX) and **PA11** (RX).
- Jumpers **J21** (TX) and **J22** (RX) on the board must be in position 1-2
  (route to XDS110). This is the factory default.
- The signal path is: UART0 TX (PA10) -> J21 -> XDS110 -> USB -> host PC.

## SysConfig

Add a UART instance in SysConfig with these settings:

| Setting | Value |
|---------|-------|
| Instance name | `UART_0` |
| Peripheral | `UART0` |
| TX pin | `PA10` |
| RX pin | `PA11` |
| Baud rate | `115200` |

SysConfig generates `SYSCFG_DL_UART_0_init()` which is called automatically
from `SYSCFG_DL_init()` inside `initialize_hardware()`.

## Host Side (macOS)

The LaunchPad enumerates two serial ports:

| Port | Purpose |
|------|---------|
| `/dev/cu.usbmodemMG3500011` (lower number) | UART backchannel |
| `/dev/cu.usbmodemMG3500014` (higher number) | XDS110 auxiliary data |

Connect to the **lower-numbered** port at **115200 8N1**.

### TI Theia Port Conflict

TI Theia grabs the UART port on launch and holds it. If your terminal app
shows no output, Theia is likely holding the port.

**Fix:** Open your terminal app (CoolTerm, minicom, screen, etc.) **before**
launching Theia, or make sure Theia's serial monitor is not connected to the
same port. The terminal app that opens the port first wins.

## Code

`printf` does not work with the TI Arm Clang toolchain out of the box on this
target (the weak `fputc` override is not pulled in reliably). Use the
`uart_debug` module in `application_drivers/` instead:

### Module Files

| File | Purpose |
|------|---------|
| `application_drivers/uart_debug.h` | Public API |
| `application_drivers/uart_debug.c` | Implementation (calls `DL_UART_Main_transmitDataBlocking`) |

### API

```c
#include "application_drivers/uart_debug.h"

UartDebug_putchar('F');           /* single character */
UartDebug_print("boot\r\n");     /* null-terminated string */
```

### Usage From Library Source Files

Library files under `src/` can include via relative path:

```c
#include "../../../application_drivers/uart_debug.h"
```

### Timing Impact

Each character at 115200 baud takes ~87 us. A 6-character message
(`"boot\r\n"`) takes ~0.5 ms. This is fast enough for most CAN bus debugging
but will change timing if placed inside tight loops. Use single-character
markers in hot paths.

### Cleanup

Remove all `UartDebug_putchar` / `UartDebug_print` calls and the
`uart_debug.h/c` include before release builds. Also remove the UART0
SysConfig instance — the UART peripheral and pin mux consume flash and RAM
that the production bootloader does not need.
