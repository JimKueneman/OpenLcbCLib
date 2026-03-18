# Bootloader Build Configuration Plan

## Objective

Add a `OPENLCB_COMPILE_BOOTLOADER` convenience preset to `openlcb_user_config.h`
and verify that `openlcb_config.h/c` can compile a minimal bootloader image that
supports only what the Firmware Upgrade Standard requires:

- Node login / Initialization Complete
- SNIP (Simple Node Information Protocol)
- Datagrams (required transport for Memory Config)
- Memory Configuration — read, write, and ops (Freeze / Unfreeze)
- Firmware upgrade write to address space 0xEF
- Protocol Support Inquiry reply with the correct PSI bits

---

## What the Standard Requires

From `FirmwareUpgradeS.pdf` and `FirmwareUpgradeTN.pdf`:

| Requirement | Protocol |
|---|---|
| Emit Node Initialization Complete on state change | Message Network (always present) |
| Answer Protocol Support Inquiry | Message Network (always present) |
| Answer SNIP request | SNIP (always present) |
| Receive Freeze datagram for space 0xEF | Datagrams + Memory Config Ops |
| Accept firmware writes to space 0xEF | Datagrams + Memory Config Write |
| Emit Datagram Received OK | Datagrams |
| Send/receive stream for fast transfer (optional) | Stream (not yet implemented — skip) |
| Receive Unfreeze datagram | Datagrams + Memory Config Ops |

---

## Feature Flags — What to Include vs. Exclude

### Include (minimum required)

| Flag | Reason |
|---|---|
| `OPENLCB_COMPILE_DATAGRAMS` | Transport for Memory Config Freeze / Write / Unfreeze |
| `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Enables Config Mem Read, Write, and Ops handlers |
| `OPENLCB_COMPILE_FIRMWARE` | Enables Freeze / Unfreeze callbacks and space 0xEF write |

### Exclude (strip from image)

| Flag | Modules Removed | Reason |
|---|---|---|
| `OPENLCB_COMPILE_EVENTS` | `protocol_event_transport.h`, all producer/consumer state, ~15 DI slots | Bootloader never produces or consumes events |
| `OPENLCB_COMPILE_BROADCAST_TIME` | `protocol_broadcast_time_handler.h`, `openlcb_application_broadcast_time.h`, 8 DI slots | No clock needed |
| `OPENLCB_COMPILE_TRAIN` | `protocol_train_handler.h`, `openlcb_application_train.h`, ~20 DI slots | No train control |
| `OPENLCB_COMPILE_TRAIN_SEARCH` | `protocol_train_search_handler.h`, 2 DI slots | No train search |

The `#warning "No optional features enabled"` in `openlcb_config.h` will fire for a
BOOTLOADER build. **That warning guard needs to be updated** — a BOOTLOADER build
with DATAGRAMS + MEMORY_CONFIGURATION + FIRMWARE is a valid (non-empty) configuration.

---

## PSI Bits — Minimal Bootloader Set

In the bootloader's `openlcb_user_config.c`, set only:

| Bit | Hex | Reason |
|---|---|---|
| `PSI_SIMPLE_NODE_INFORMATION` | `0x001000` | Always required — SNIP always present |
| `PSI_IDENTIFICATION` | `0x020000` | Always required — Identification always present |
| `PSI_DATAGRAM` | `0x400000` | Datagrams enabled |
| `PSI_MEMORY_CONFIGURATION` | `0x100000` | Memory Config enabled |
| `PSI_FIRMWARE_UPGRADE` | `0x000020` | Node supports firmware upgrade (Operating state) |

Set at runtime when in Firmware Upgrade state:

| Bit | Hex | How |
|---|---|---|
| `PSI_FIRMWARE_UPGRADE_ACTIVE` | `0x000010` | Already tracked in `openlcb_node_state_t.firmware_upgrade_active`; PIP reply must reflect this |

Bits to **NOT** set: `PSI_EVENT_EXCHANGE`, `PSI_TRAIN_CONTROL`, `PSI_FUNCTION_DESCRIPTION`,
`PSI_FUNCTION_CONFIGURATION`, `PSI_CONFIGURATION_DESCRIPTION_INFO` (no CDI in bootloader),
`PSI_STREAM` (not yet implemented).

---

## Buffer Pool Minimization

Normal application buffers can be reduced dramatically in bootloader context.
These are all `#define` values in `openlcb_user_config.h`.

| Parameter | Normal App Typical | Bootloader Minimum | Notes |
|---|---|---|---|
| `USER_DEFINED_BASIC_BUFFER_DEPTH` | 32 | 8 | Only login + SNIP + PIP traffic expected |
| `USER_DEFINED_DATAGRAM_BUFFER_DEPTH` | 4 | 2 | One Freeze in, one Write sequence at a time |
| `USER_DEFINED_SNIP_BUFFER_DEPTH` | 4 | 1 | One SNIP reply at a time |
| `USER_DEFINED_STREAM_BUFFER_DEPTH` | 1 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_STREAM_BUFFER_LEN` | 256 | 256 | Keep default (used in struct sizing) |
| `USER_DEFINED_NODE_BUFFER_DEPTH` | 4 | 1 | Bootloader is a single-node device |
| `USER_DEFINED_PRODUCER_COUNT` | 64 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_PRODUCER_RANGE_COUNT` | 5 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_CONSUMER_COUNT` | 32 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_CONSUMER_RANGE_COUNT` | 5 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_TRAIN_NODE_COUNT` | 4 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_MAX_LISTENERS_PER_TRAIN` | 4 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_MAX_TRAIN_FUNCTIONS` | 29 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_CDI_LENGTH` | 1024 | 1 | 0 creates zero-length array (see todo #6) |
| `USER_DEFINED_FDI_LENGTH` | 1024 | 1 | 0 creates zero-length array (see todo #6) |

---

## DI Chain — Callbacks Required in openlcb_config_t

### openlcb_config_t slots (user-facing config struct)

With the bootloader flag set, the user **must** provide:

| Callback | Type | Why Required |
|---|---|---|
| `lock_shared_resources` | `void (*)(void)` | Always required |
| `unlock_shared_resources` | `void (*)(void)` | Always required |
| `freeze` | ops callback signature | `OPENLCB_COMPILE_FIRMWARE` — called on Freeze 0xEF |
| `unfreeze` | ops callback signature | `OPENLCB_COMPILE_FIRMWARE` — called on Unfreeze 0xEF |
| `firmware_write` | write callback signature | `OPENLCB_COMPILE_FIRMWARE` — receives raw firmware bytes |

The following are **NULL / omit** in a bootloader build:

| Callback | Normal Use | Bootloader |
|---|---|---|
| `config_mem_read` | Read config space 0xFD | NULL — bootloader has no config memory to read; space descriptor queries go through the ops handler struct, not this callback |
| `config_mem_write` | Write config space 0xFD | NULL — the only writes are to 0xEF, routed via `firmware_write` |
| `reboot` | Memory Config Reset/Reboot command (0xA9) | **WIRED** — some CTs send Reset/Reboot after Unfreeze; must trigger hardware reset |
| `factory_reset` | Wipe config mem | NULL — safe; ops handler sends NACK/NOT_IMPLEMENTED when NULL |
| `on_100ms_timer` | App timer tick | NULL in template — available if application needs periodic work (e.g., watchdog kick) |
| `on_login_complete` | Application startup | NULL in template — available if application needs startup action (e.g., status LED) |
| `on_optional_interaction_rejected` | App error handling | NULL in template — available if application needs error handling |
| `on_terminate_due_to_error` | App error handling | NULL in template — available if application needs fatal error handling |
| All 15 event callbacks | Event exchange | Not compiled — EVENTS off |
| All 8 broadcast time callbacks | Clock | Not compiled — BROADCAST_TIME off |
| All 20 train callbacks | Train control | Not compiled — TRAIN off |

---

### interface_protocol_datagram_handler_t — Complete Slot Table

`interface_protocol_datagram_handler.h` has ~100 function pointer slots. The CT
operating in firmware upgrade mode sends only five message types to the node:
Freeze, Write to 0xEF, Unfreeze, Get Config Options (optional), and Get Address
Space Info for 0xEF (optional). All other slots auto-reject with
SUBCOMMAND\_UNKNOWN when NULL — no crash, no special handling needed.

#### Resource Locking (REQUIRED)

| Slot | Bootloader |
|---|---|
| `lock_shared_resources` | **WIRED** — always required |
| `unlock_shared_resources` | **WIRED** — always required |

#### Datagram READ server handlers (7 spaces: 0xFF CDI, 0xFD All, 0xFE Config, 0xFC ACDI-Mfg, 0xFB ACDI-User, 0xFA Train FDI, 0xF9 Train Fn Config)

All 7 → **NULL** — bootloader has no readable spaces; CT never sends read requests during firmware upgrade.

#### Datagram READ client reply handlers (14 slots: OK + FAIL × 7 spaces)

All 14 → **NULL** — bootloader never initiates reads.

#### Stream READ server and client handlers (28 slots)

All 28 → **NULL** — stream not used for firmware upgrade datagram path.

#### Datagram WRITE server handlers (8 spaces: above 7 + 0xEF Firmware)

| Slot | Bootloader |
|---|---|
| `memory_write_space_config_description_info` (0xFF) | NULL |
| `memory_write_space_all` (0xFD) | NULL |
| `memory_write_space_configuration_memory` (0xFE) | NULL |
| `memory_write_space_acdi_manufacturer` (0xFC) | NULL |
| `memory_write_space_acdi_user` (0xFB) | NULL |
| `memory_write_space_train_function_definition_info` (0xFA) | NULL |
| `memory_write_space_train_function_config_memory` (0xF9) | NULL |
| `memory_write_space_firmware_upgrade` (0xEF) | **WIRED** — receives each firmware data datagram; calls `firmware_write` callback |

#### Datagram WRITE client reply handlers (14 slots: OK + FAIL × 7 non-firmware spaces)

All 14 → **NULL** — bootloader never initiates writes to other spaces.

#### Write-under-mask handlers (8 spaces: same set as WRITE)

All 8 → **NULL** — CT never sends write-under-mask during firmware upgrade.

#### Stream WRITE server handlers (8 spaces)

All 8 → **NULL** — stream not used (not yet implemented in library).

#### Stream WRITE client reply handlers (14 slots)

All 14 → **NULL** — stream not used.

#### Configuration Memory Command handlers

| Slot | Bootloader | Reason |
|---|---|---|
| `memory_options_cmd` | **WIRED** | CT may query node capabilities before starting upgrade |
| `memory_options_reply` | NULL | Bootloader never queries another node's options |
| `memory_get_address_space_info` | **WIRED** | CT queries whether 0xEF is present and its size |
| `memory_get_address_space_info_reply_not_present` | NULL | Bootloader never queries another node |
| `memory_get_address_space_info_reply_present` | NULL | Bootloader never queries another node |
| `memory_reserve_lock` | NULL | Advisory only; bootloader ignores Lock/Reserve |
| `memory_reserve_lock_reply` | NULL | Bootloader never issues Lock/Reserve |
| `memory_get_unique_id` | NULL | Not needed for firmware upgrade |
| `memory_get_unique_id_reply` | NULL | Not needed for firmware upgrade |
| `memory_unfreeze` | **WIRED** | CT sends Unfreeze 0xEF to end upgrade; node decides what to do next (typically hardware reset, but not required by spec) |
| `memory_freeze` | **WIRED** | CT sends Freeze 0xEF to begin upgrade; sets firmware_upgrade_active, emits Init Complete |
| `memory_update_complete` | NULL | Update Complete is for config memory (0xFE) modifications, not firmware |
| `memory_reset_reboot` | **WIRED** | Wired defensively — not required by spec or sent by JMRI, but TN acknowledges Reboot as a valid state-transition mechanism; costs nothing to support |
| `memory_factory_reset` | NULL | Safe — NULL causes NACK/NOT_IMPLEMENTED via phase-1 NULL gate in ops handler |

**Summary: 7 slots wired, ~94 slots NULL.**

---

### can_config_t — CAN Driver Interface

The CAN driver config struct has 4 REQUIRED slots and 3 optional debug/notification
hooks. The optional hooks are NULL by default in all existing applications and remain
so in a bootloader build. Available if the application needs them.

| Slot | Bootloader | Notes |
|---|---|---|
| `transmit_raw_can_frame` | **REQUIRED** | Platform CAN transmit function |
| `is_tx_buffer_clear` | **REQUIRED** | Platform CAN TX-ready check |
| `lock_shared_resources` | **REQUIRED** | Same function as openlcb_config_t lock |
| `unlock_shared_resources` | **REQUIRED** | Same function as openlcb_config_t unlock |
| `on_rx` | NULL by default — available for debug/diagnostics (e.g., logging received frames) |
| `on_tx` | NULL by default — available for debug/diagnostics (e.g., logging transmitted frames) |
| `on_alias_change` | NULL by default — available for notification when alias mappings change |

---

### Other DI Interfaces — No Bootloader Action Needed

| Interface | Location | Bootloader Status |
|---|---|---|
| `interface_openlcb_application_t` | `openlcb_application.h` | 3 REQUIRED slots, no NULLs — always fully wired |
| `interface_openlcb_protocol_snip_t` | `protocol_snip.h` | 1 REQUIRED slot (`config_memory_read`) — always wired |
| `interface_openlcb_protocol_message_network_t` | `protocol_message_network.h` | 2 optional slots (`on_optional_interaction_rejected`, `on_terminate_due_to_error`) — mirrors `openlcb_config_t`, wired internally by `openlcb_config.c` |
| `interface_openlcb_protocol_event_transport_t` | `protocol_event_transport.h` | 15 slots — entire module compiled out (`OPENLCB_COMPILE_EVENTS` off) |
| `interface_protocol_train_handler_t` | `protocol_train_handler.h` | 20 slots — entire module compiled out (`OPENLCB_COMPILE_TRAIN` off) |
| `interface_openlcb_protocol_broadcast_time_handler_t` | `protocol_broadcast_time_handler.h` | 7 slots — entire module compiled out (`OPENLCB_COMPILE_BROADCAST_TIME` off) |
| `interface_can_rx_message_handler_t` | `can_rx_message_handler.h` | Internal wiring by `openlcb_config.c` — 5 optional listener slots inactive (virtual node support off in bootloader) |
| `interface_can_tx_message_handler_t` | `can_tx_message_handler.h` | Internal wiring by `openlcb_config.c` — 1 optional `on_transmit` slot |

---

## Proposed openlcb_config.h Changes

### 1. Update the "no features" warning guard

Current:
```c
#if !defined(OPENLCB_COMPILE_EVENTS) && !defined(OPENLCB_COMPILE_DATAGRAMS) && \
    !defined(OPENLCB_COMPILE_TRAIN)
#warning "No optional features enabled..."
#endif
```

Proposed: exclude warning when FIRMWARE is enabled (bootloader case):
```c
#if !defined(OPENLCB_COMPILE_EVENTS) && !defined(OPENLCB_COMPILE_DATAGRAMS) && \
    !defined(OPENLCB_COMPILE_TRAIN) && !defined(OPENLCB_COMPILE_FIRMWARE)
#warning "No optional features enabled. This node will only support SNIP identification."
#endif
```

### 2. No new feature flags in openlcb_config.h itself

`OPENLCB_COMPILE_BOOTLOADER` is defined in `openlcb_user_config.h` and propagated
through `openlcb_config.h` (which includes the user config). It serves two roles:

1. **Convenience preset** — automatically sets DATAGRAMS + MEMORY\_CONFIGURATION +
   FIRMWARE and undefs EVENTS, BROADCAST\_TIME, TRAIN, TRAIN\_SEARCH.
2. **Internal compile-time guard** — used inside `protocol_config_mem_*.c` files
   to strip code the bootloader does not need (see Compile-Time Module Exclusion
   section below).

No new feature-flag logic is needed in `openlcb_config.h` beyond the warning fix
above.

---

## Proposed openlcb_user_config.h Template Changes

Add a new `OPENLCB_COMPILE_BOOTLOADER` preset block. When this define is set, it
automatically defines the minimum required set and undefines everything else:

```c
// =============================================================================
// Bootloader preset — define this to get a minimal firmware-upgrade-only build.
// Automatically sets DATAGRAMS + MEMORY_CONFIGURATION + FIRMWARE.
// All other features are disabled.
// =============================================================================
// #define OPENLCB_COMPILE_BOOTLOADER

#ifdef OPENLCB_COMPILE_BOOTLOADER

    #define OPENLCB_COMPILE_DATAGRAMS
    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
    #define OPENLCB_COMPILE_FIRMWARE

    // Suppress features that must not be present in a bootloader image
    #undef OPENLCB_COMPILE_EVENTS
    #undef OPENLCB_COMPILE_BROADCAST_TIME
    #undef OPENLCB_COMPILE_TRAIN
    #undef OPENLCB_COMPILE_TRAIN_SEARCH

#endif
```

---

## Proposed openlcb_config.c Changes

No changes required. The existing `#ifdef` guards already exclude all non-bootloader
modules when the flags are absent. `_build_application()` wraps all event callbacks
in `#ifdef OPENLCB_COMPILE_EVENTS` — confirmed safe with NULL event callbacks.

---

## Callback Contract — What the Library Requires

The library delivers three platform-agnostic callback slots that the application
must implement. What those callbacks do internally is entirely platform-specific
and is outside the library's scope. The contract from the library's perspective:

| Callback | When called | Required library actions inside callback |
|---|---|---|
| `freeze` | CT sends Freeze 0xEF datagram | Set `statemachine_info->openlcb_node->state.firmware_upgrade_active = true`; call `OpenLcbApplication_send_initialization_event()` in a loop until it returns `true` (buffers may be full on first call) |
| `firmware_write` | Each datagram write to space 0xEF | Call `OpenLcbUtilities_load_config_mem_reply_write_ok_message_header()` on success or `_write_fail_message_header()` with an error code on failure; set `statemachine_info->outgoing_msg_info.valid = true` |
| `unfreeze` | CT sends Unfreeze 0xEF datagram | Set `firmware_upgrade_active = false`; the node decides what to do next — most implementations trigger a hardware reset so the new firmware boots cleanly, but the spec does not require it; a node could in principle resume normal operation without restarting |

`OpenLcbApplication_send_initialization_event()` is the only library call the
application must make explicitly, and only in the `freeze` callback. Everything
else (Datagram OK, Write reply, Initialization Complete on exit) is handled by
the library or by the platform. If the application resets on Unfreeze, the login
state machine emits Node Initialization Complete naturally on restart with no
explicit call needed.

---

## Open Questions Before Implementation

1. ~~**Zero-count arrays**~~ — **DEFERRED TO todo.md #6.** Setting these counts to 0
   creates zero-length struct array members (GCC extension, not C99). All existing
   application files use non-zero values so there is no current breakage. The bootloader
   template will use 1 as the minimum for all affected counts. A proper fix (compile-time
   `#error` guards in `openlcb_types.h`) is tracked separately.

2. ~~**CDI length of 0**~~ — **DEFERRED TO todo.md #6.** Same zero-length array issue
   as #1. Bootloader template will use 1 as minimum.

3. ~~**`factory_reset` NULL safety**~~ — **RESOLVED SAFE.** `_handle_operations_request()`
   gates on `operations_func != NULL` in phase 1. A NULL `factory_reset` sends
   `NACK / NOT_IMPLEMENTED` and never reaches the phase-2 call. No crash.

4. ~~**`config_mem_read` in a bootloader**~~ — **RESOLVED: NOT NEEDED.** Address space
   descriptor queries (Get Address Space Info) are handled entirely by the ops handler
   reading `node->parameters->address_space_firmware` struct fields. The `config_mem_read`
   callback is only called for space 0xFD reads, which a bootloader never receives.
   Space 0xEF has no read dispatcher at all — the standard only writes to it.

5. ~~**`reboot` semantics on Unfreeze**~~ — **RESOLVED.** JMRI sends only Unfreeze —
   it does not send Reset/Reboot (0xA9) after firmware upgrade. What happens next
   is entirely the node's decision: most implementations trigger a hardware reset
   so the new firmware boots cleanly, but the spec does not require this. A node
   could resume normal operation without restarting. The `reboot` field in
   `openlcb_config_t` handles the separate Memory Config Reset/Reboot command
   (0xA9). This slot is wired defensively (not required by spec) because the TN
   acknowledges Reboot as a valid alternative state-transition mechanism and it
   costs nothing to support. The RPiPico reference wires both `unfreeze` and
   `reboot` to `rp2040.reboot()` as a reasonable default.

6. ~~**Buffer count total ≤ 126 check**~~ — **RESOLVED: GUARD WAS OUT OF DATE.**
   All buffer indices and counters are `uint16_t`. The guard and its comment in
   `openlcb_types.h` have been updated to 65535. Bootloader sum of 11 is trivially
   safe.

---

## Compile-Time Module Exclusion

### Whole-module guards (DONE)

Each protocol `.c` file now wraps its entire implementation body in the
corresponding feature-flag `#ifdef`.  When a flag is undefined the compiler
sees an empty translation unit — zero Flash.

| File | Guard | Approx. lines excluded |
|---|---|---|
| `protocol_event_transport.c` | `OPENLCB_COMPILE_EVENTS` | ~1 232 |
| `protocol_broadcast_time_handler.c` | `OPENLCB_COMPILE_BROADCAST_TIME` | ~342 |
| `protocol_train_handler.c` | `OPENLCB_COMPILE_TRAIN` | ~1 476 |
| `protocol_train_search_handler.c` | `OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH` | ~520 |
| `protocol_datagram_handler.c` | `OPENLCB_COMPILE_DATAGRAMS` | ~1 618 |
| `protocol_config_mem_read_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~646 |
| `protocol_config_mem_write_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~1 048 |
| `protocol_config_mem_operations_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~710 |

Each `.c` file adds `#include "openlcb_config.h"` (after its own header) to make
the flags visible, then `#ifdef GUARD` before the rest of the includes/code, and
`#endif` at the end.  All 40 test suites (1 549 tests) pass after this change.

---

### Internal bootloader guards (TODO — requires code reorganization)

The three config-mem handler files compile in a bootloader build because
`OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled.  However, a bootloader does not
need all of their code.  Each file will be reorganized so that the unneeded code
lives in **one continuous block at the bottom**, wrapped in
`#ifndef OPENLCB_COMPILE_BOOTLOADER`.

#### 1. `protocol_config_mem_read_handler.c` — exclude entire body

The CT never reads from any address space during firmware upgrade.  No functions in
this file are called in bootloader mode, and the file has zero cross-dependencies
with the write or operations handlers.

```c
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#ifndef OPENLCB_COMPILE_BOOTLOADER

// ... entire existing implementation ...

#endif /* OPENLCB_COMPILE_BOOTLOADER */
#endif /* OPENLCB_COMPILE_MEMORY_CONFIGURATION */
```

**Savings: ~646 lines (100%).**

#### 2. `protocol_config_mem_write_handler.c` — reorganize, exclude bottom block

The bootloader only uses `write_space_firmware()` (space 0xEF) and the shared
infrastructure it depends on (parameter extraction, validation, overrun checks,
`_write_data()`, `_dispatch_write_request()`).

**Keep at top (bootloader needs):**
- `_extract_write_command_parameters()`
- `_is_valid_write_parameters()`
- `_check_for_write_overrun()`
- `_dispatch_write_request()`
- `_write_data()` (shared write helper used by firmware write path)
- `ProtocolConfigMemWriteHandler_write_space_firmware()`
- `ProtocolConfigMemWriteHandler_initialize()`

**Move to bottom, wrap in `#ifndef OPENLCB_COMPILE_BOOTLOADER`:**
- 7 non-firmware write space dispatchers (0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9)
- Entire write-under-mask subsection (infrastructure + 8 space dispatchers)
- Client-side write request functions (config\_mem, acdi\_user, train\_fn\_config)
- Message stubs (write\_message, write\_reply\_ok, write\_reply\_fail)

**Savings: ~632 lines (~60%).**

#### 3. `protocol_config_mem_operations_handler.c` — reorganize, exclude bottom block

The bootloader needs: freeze, unfreeze, get\_address\_space\_info, options\_cmd,
reset\_reboot, and the generic two-phase dispatch infrastructure.

**Keep at top (bootloader needs):**
- `_handle_operations_request()` (generic two-phase ACK-then-execute)
- Helper functions (flags builders, message loaders)
- `options_cmd()` / `options_reply()`
- `get_address_space_info()` / reply handlers
- `freeze()` / `unfreeze()`
- `reset_reboot()`
- `ProtocolConfigMemOperationsHandler_initialize()`

**Move to bottom, wrap in `#ifndef OPENLCB_COMPILE_BOOTLOADER`:**
- `reserve_lock()` / `reserve_lock_reply()`
- `get_unique_id()` / `get_unique_id_reply()`
- `update_complete()`
- `factory_reset()`
- Client-side request builders (request\_options\_cmd, request\_get\_address\_space\_info, request\_reserve\_lock)

**Savings: ~245 lines (~34%).**

---

### Total Flash savings summary (bootloader vs. full node)

| Source | Lines excluded |
|---|---|
| Whole-module exclusion (events, broadcast time, train, train search) | ~3 570 |
| Config mem read handler (entire body) | ~646 |
| Config mem write handler (non-firmware block) | ~632 |
| Config mem operations handler (unused ops block) | ~245 |
| **Total** | **~5 093 lines** |

---

## Files to Modify (Pending Approval)

### Already done

| File | Change | Status |
|---|---|---|
| `src/openlcb/openlcb_config.h` | Updated buffer-count guard from 126 to 65535 | DONE |
| `src/openlcb/openlcb_types.h` | Updated buffer-count comment from 126 to 65535 | DONE |
| `src/openlcb/protocol_event_transport.c` | `#ifdef OPENLCB_COMPILE_EVENTS` whole-module guard | DONE |
| `src/openlcb/protocol_broadcast_time_handler.c` | `#ifdef OPENLCB_COMPILE_BROADCAST_TIME` whole-module guard | DONE |
| `src/openlcb/protocol_train_handler.c` | `#ifdef OPENLCB_COMPILE_TRAIN` whole-module guard | DONE |
| `src/openlcb/protocol_train_search_handler.c` | `#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)` whole-module guard | DONE |
| `src/openlcb/protocol_datagram_handler.c` | `#ifdef OPENLCB_COMPILE_DATAGRAMS` whole-module guard | DONE |
| `src/openlcb/protocol_config_mem_read_handler.c` | `#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION` whole-module guard | DONE |
| `src/openlcb/protocol_config_mem_write_handler.c` | `#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION` whole-module guard | DONE |
| `src/openlcb/protocol_config_mem_operations_handler.c` | `#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION` whole-module guard | DONE |

### Remaining changes

| File | Change |
|---|---|
| `src/openlcb/openlcb_config.h` | Update "no features" warning guard (1-line change) |
| `templates/openlcb_user_config.h` | Add `OPENLCB_COMPILE_BOOTLOADER` preset block |
| `src/openlcb/openlcb_config.c` | Likely none — audit `_build_application()` first |
| `src/openlcb/protocol_config_mem_read_handler.c` | Add `#ifndef OPENLCB_COMPILE_BOOTLOADER` to exclude entire body |
| `src/openlcb/protocol_config_mem_write_handler.c` | Reorganize: bootloader-needed code at top, `#ifndef OPENLCB_COMPILE_BOOTLOADER` block at bottom |
| `src/openlcb/protocol_config_mem_operations_handler.c` | Reorganize: bootloader-needed ops at top, `#ifndef OPENLCB_COMPILE_BOOTLOADER` block at bottom |

### New files

| File | Purpose |
|---|---|
| `src/applications/bootloader/openlcb_user_config.h` | Bootloader-specific user config with all counts minimized |
| `src/applications/bootloader/openlcb_user_config.c` | Minimal PSI bits + address space 0xEF descriptor |
| `test/openlcb_bootloader_Test.cxx` | Bootloader configuration test suite (see Test Plan below) |
| `test/openlcb_bootloader_user_config.h` | Test-specific bootloader user config with `OPENLCB_COMPILE_BOOTLOADER` defined |
| `test/openlcb_bootloader_user_config.c` | Test-specific bootloader PSI bits + 0xEF descriptor |

---

## Test Plan — `openlcb_bootloader_Test.cxx`

A dedicated test file that compiles with `OPENLCB_COMPILE_BOOTLOADER` defined.  It
uses its own `openlcb_bootloader_user_config.h/c` pair (minimal buffer counts,
bootloader PSI bits, space 0xEF descriptor only).  This is a separate CMake target
— it does not share the full-node user config used by the existing test suite.

### 1. Compilation Validation

These tests verify that the bootloader configuration compiles and initializes
without errors.  They catch missing symbols, broken `#ifdef` chains, and
unresolved dependencies.

| # | Test | What it validates |
|---|---|---|
| 1.1 | `Bootloader_compile_and_initialize` | `OpenLcbConfig_initialize()` succeeds with bootloader config — all DI wiring resolves, no NULL dereference on startup |
| 1.2 | `Bootloader_node_login_complete` | Node reaches Initialized state — login statemachine runs, emits Node Initialization Complete |
| 1.3 | `Bootloader_psi_bits_correct` | Protocol Support Reply contains exactly: SNIP, Identification, Datagram, Memory Configuration, Firmware Upgrade — no Event Exchange, no Train Control |

### 2. Firmware Upgrade Sequence — Happy Path

These tests walk through the normative firmware upgrade sequence from
`FirmwareUpgradeS.pdf` Section 5.5.

| # | Test | What it validates |
|---|---|---|
| 2.1 | `Bootloader_freeze_sets_firmware_active` | Freeze 0xEF datagram → `freeze` callback called, `firmware_upgrade_active` set to `true` |
| 2.2 | `Bootloader_freeze_emits_init_complete` | After Freeze, node emits Node Initialization Complete (state transition acknowledgement) |
| 2.3 | `Bootloader_psi_active_after_freeze` | After Freeze, PSI reply includes `PSI_FIRMWARE_UPGRADE_ACTIVE` bit |
| 2.4 | `Bootloader_write_firmware_data` | Write datagram to space 0xEF → `firmware_write` callback called with correct address, count, and data bytes |
| 2.5 | `Bootloader_write_firmware_reply_ok` | Firmware write success → Datagram Received OK + Write Reply OK sent back to CT |
| 2.6 | `Bootloader_write_firmware_sequential` | Multiple sequential writes (offset 0, 64, 128, ...) → each calls `firmware_write` with correct offset |
| 2.7 | `Bootloader_unfreeze_calls_callback` | Unfreeze 0xEF datagram → `unfreeze` callback called |
| 2.8 | `Bootloader_reset_reboot_calls_callback` | Reset/Reboot (0xA9) datagram → `reboot` callback called |

### 3. Config Memory Commands — Bootloader Subset

These tests verify the Memory Config operations that a CT may send before or
during firmware upgrade.

| # | Test | What it validates |
|---|---|---|
| 3.1 | `Bootloader_options_cmd_reply` | Get Configuration Options → valid reply with supported capabilities |
| 3.2 | `Bootloader_address_space_info_0xEF` | Get Address Space Info for 0xEF → reply with Present, correct size, writable |
| 3.3 | `Bootloader_address_space_info_unknown` | Get Address Space Info for non-0xEF space → reply with Not Present |

### 4. Negative Tests — NULL Slot Rejection

These tests verify that commands not supported in bootloader mode are properly
rejected rather than crashing.

| # | Test | What it validates |
|---|---|---|
| 4.1 | `Bootloader_read_request_rejected` | Read datagram for any space → Datagram Rejected (SUBCOMMAND_UNKNOWN) |
| 4.2 | `Bootloader_write_non_firmware_rejected` | Write to space 0xFE (config mem) → Datagram Rejected |
| 4.3 | `Bootloader_write_under_mask_rejected` | Write-under-mask for any space → Datagram Rejected |
| 4.4 | `Bootloader_factory_reset_rejected` | Factory Reset command → NACK / NOT_IMPLEMENTED |
| 4.5 | `Bootloader_lock_reserve_rejected` | Lock/Reserve command → Datagram Rejected |
| 4.6 | `Bootloader_get_unique_id_rejected` | Get Unique ID command → Datagram Rejected |

### 5. Error Handling

These tests verify the firmware upgrade error paths defined in
`FirmwareUpgradeS.pdf` Section 5.4.3.

| # | Test | What it validates |
|---|---|---|
| 5.1 | `Bootloader_write_firmware_fail_reply` | `firmware_write` callback signals failure → Write Reply Fail datagram sent with error code |
| 5.2 | `Bootloader_write_fail_stays_in_upgrade_state` | After write failure, node remains in Firmware Upgrade state (`firmware_upgrade_active` still true) |
| 5.3 | `Bootloader_retry_after_fail` | After write failure, CT can restart from offset 0 — new Freeze + writes accepted |

### 6. SNIP and PIP in Bootloader Mode

| # | Test | What it validates |
|---|---|---|
| 6.1 | `Bootloader_snip_reply` | SNIP request → valid SNIP reply with bootloader manufacturer/model strings |
| 6.2 | `Bootloader_pip_reply_operating` | PIP request in Operating state → correct bits (Firmware Upgrade, no Firmware Upgrade Active) |
| 6.3 | `Bootloader_pip_reply_firmware_active` | PIP request in Firmware Upgrade state → correct bits (Firmware Upgrade Active set) |

### Test Infrastructure Notes

- The test file needs its own mock callbacks for `freeze`, `unfreeze`,
  `firmware_write`, and `reboot` that record whether they were called and with
  what arguments.
- The test `openlcb_bootloader_user_config.h` defines `OPENLCB_COMPILE_BOOTLOADER`
  and uses the minimum buffer counts from the Buffer Pool Minimization table above.
- CMake adds this as a separate target so it compiles with a different preprocessor
  configuration than the existing tests.

---

## Out of Scope for This Plan

- Stream transport (not yet implemented in the library)
- Actual flash write implementation (`firmware_write` callback body)
- Hardware-specific bootloader startup / primary firmware validation
- Dual-firmware architecture (Bootloader + Primary) — that is a platform concern
