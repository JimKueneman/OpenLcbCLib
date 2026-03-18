<!-- STATUS: IMPLEMENTED — 2026-03-18
     All whole-module #ifdef guards in place and tested.
     Internal #ifndef OPENLCB_COMPILE_BOOTLOADER guards in config_mem_read/write/operations handlers.
     Buffer-count guard updated (126 → 65535).
     30 bootloader tests passing (openlcb_bootloader_Test.cxx).
     1 544 total tests across 41 suites, 0 failures.
     Remaining items: template openlcb_user_config.h preset block, "no features" warning guard.
     Zero-length array guards deferred to todo.md #6. -->

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

---

## PSI Bits — Minimal Bootloader Set

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

---

## Buffer Pool Minimization

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
| `config_mem_read` | Read config space 0xFD | NULL — bootloader has no config memory to read |
| `config_mem_write` | Write config space 0xFD | NULL — the only writes are to 0xEF, routed via `firmware_write` |
| `reboot` | Memory Config Reset/Reboot command (0xA9) | **WIRED** — some CTs send Reset/Reboot after Unfreeze |
| `factory_reset` | Wipe config mem | NULL — safe; ops handler sends NACK/NOT_IMPLEMENTED when NULL |
| `on_100ms_timer` | App timer tick | NULL in template — available if needed |
| `on_login_complete` | Application startup | NULL in template — available if needed |
| `on_optional_interaction_rejected` | App error handling | NULL in template — available if needed |
| `on_terminate_due_to_error` | App error handling | NULL in template — available if needed |
| All event/broadcast/train callbacks | Various | Not compiled — feature flags off |

---

### interface_protocol_datagram_handler_t — Complete Slot Table

~100 function pointer slots. CT in firmware upgrade mode sends only: Freeze,
Write to 0xEF, Unfreeze, Get Config Options (optional), Get Address Space Info
(optional). All other slots auto-reject with SUBCOMMAND_UNKNOWN when NULL.

**Summary: 7 slots wired, ~94 slots NULL.**

Wired: `lock_shared_resources`, `unlock_shared_resources`,
`memory_write_space_firmware_upgrade`, `memory_freeze`, `memory_unfreeze`,
`memory_options_cmd`, `memory_get_address_space_info`, `memory_reset_reboot`.

---

## Callback Contract

| Callback | When called | Required actions |
|---|---|---|
| `freeze` | CT sends Freeze 0xEF | Set `firmware_upgrade_active = true`; call `OpenLcbApplication_send_initialization_event()` in a loop until it returns `true` |
| `firmware_write` | Each datagram write to 0xEF | Call write OK/fail reply helper; set `outgoing_msg_info.valid = true` |
| `unfreeze` | CT sends Unfreeze 0xEF | Set `firmware_upgrade_active = false`; typically trigger hardware reset |

---

## Compile-Time Module Exclusion

### Whole-module guards (DONE)

| File | Guard | Lines excluded |
|---|---|---|
| `protocol_event_transport.c` | `OPENLCB_COMPILE_EVENTS` | ~1 232 |
| `protocol_broadcast_time_handler.c` | `OPENLCB_COMPILE_BROADCAST_TIME` | ~342 |
| `protocol_train_handler.c` | `OPENLCB_COMPILE_TRAIN` | ~1 476 |
| `protocol_train_search_handler.c` | `OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH` | ~520 |
| `protocol_datagram_handler.c` | `OPENLCB_COMPILE_DATAGRAMS` | ~1 618 |
| `protocol_config_mem_read_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~646 |
| `protocol_config_mem_write_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~1 048 |
| `protocol_config_mem_operations_handler.c` | `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | ~710 |

### Internal bootloader guards (DONE)

| File | Guard | Savings |
|---|---|---|
| `protocol_config_mem_read_handler.c` | `#ifndef OPENLCB_COMPILE_BOOTLOADER` — entire body excluded | ~646 (100%) |
| `protocol_config_mem_write_handler.c` | `#ifndef OPENLCB_COMPILE_BOOTLOADER` — non-firmware code in bottom block | ~632 (60%) |
| `protocol_config_mem_operations_handler.c` | `#ifndef OPENLCB_COMPILE_BOOTLOADER` — unused ops in bottom block | ~245 (34%) |

**Total: ~5 093 lines excluded in bootloader build.**

---

## Resolved Open Questions

1. **Zero-count arrays** — DEFERRED to todo.md #6. Bootloader uses 1 as minimum.
2. **CDI length of 0** — DEFERRED to todo.md #6. Same issue, same fix.
3. **`factory_reset` NULL safety** — RESOLVED SAFE. Phase-1 NULL gate sends NACK.
4. **`config_mem_read` in bootloader** — RESOLVED: NOT NEEDED. Ops handler serves descriptors.
5. **`reboot` semantics on Unfreeze** — RESOLVED. Wired defensively; spec allows either path.
6. **Buffer count ≤ 126 check** — RESOLVED. Updated to 65535 (uint16_t indices).

---

## Changes Made

| File | Change |
|---|---|
| `src/openlcb/openlcb_config.h` | Buffer-count guard updated 126 → 65535 |
| `src/openlcb/openlcb_types.h` | Buffer-count comment updated 126 → 65535 |
| `src/openlcb/protocol_event_transport.c` | `#ifdef OPENLCB_COMPILE_EVENTS` whole-module guard |
| `src/openlcb/protocol_broadcast_time_handler.c` | `#ifdef OPENLCB_COMPILE_BROADCAST_TIME` whole-module guard |
| `src/openlcb/protocol_train_handler.c` | `#ifdef OPENLCB_COMPILE_TRAIN` whole-module guard |
| `src/openlcb/protocol_train_search_handler.c` | `#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)` whole-module guard |
| `src/openlcb/protocol_datagram_handler.c` | `#ifdef OPENLCB_COMPILE_DATAGRAMS` whole-module guard |
| `src/openlcb/protocol_config_mem_read_handler.c` | Whole-module guard + `#ifndef OPENLCB_COMPILE_BOOTLOADER` excludes entire body |
| `src/openlcb/protocol_config_mem_write_handler.c` | Whole-module guard + `#ifndef OPENLCB_COMPILE_BOOTLOADER` bottom block |
| `src/openlcb/protocol_config_mem_operations_handler.c` | Whole-module guard + `#ifndef OPENLCB_COMPILE_BOOTLOADER` bottom block |
| `src/openlcb/openlcb_bootloader_Test.cxx` | 30-test bootloader test suite |
| `test/CMakeLists.txt` | Bootloader build target (separate static library with bootloader config) |
| `test/user_config/typical/openlcb_user_config.h` | Moved from `test/` (normal test config) |
| `test/user_config/bootloader/openlcb_user_config.h` | Bootloader test compile config |

## Remaining Work

| Item | Status |
|---|---|
| `templates/openlcb_user_config.h` — add `OPENLCB_COMPILE_BOOTLOADER` preset block | TODO |
| `src/openlcb/openlcb_config.h` — update "no features" warning guard to include FIRMWARE | TODO |
| Zero-length array guards in `openlcb_types.h` | Deferred — todo.md #6 |

---

## Out of Scope

- Stream transport (not yet implemented in the library)
- Actual flash write implementation (`firmware_write` callback body)
- Hardware-specific bootloader startup / primary firmware validation
- Dual-firmware architecture (Bootloader + Primary) — platform concern
