<!--
  ============================================================
  STATUS: IMPLEMENTED
  All eight per-space Write Under Mask dispatch functions are present in `protocol_config_mem_write_handler.c` with supporting read-modify-write helpers.
  ============================================================
-->

# Implementation Plan -- Item 20: Write Under Mask

**TODO Reference:** Item 20 in `documentation/todo.md`
**Status:** COMPLETE — fully implemented
**Date:** 2026-03-02

---

## 1. Summary

Write Under Mask is fully implemented. The handler performs a read-modify-write
operation using the standard formula:

```
new_value[i] = (old_value[i] & ~mask[i]) | (data[i] & mask[i])
```

All 8 per-space dispatch functions are present in
`protocol_config_mem_write_handler.c`, wired through `openlcb_config.c`, and
the `write_under_mask_supported` flag is correctly set to `true` in all
application `node_parameters` structs.

---

## 2. Implementation

### Core Functions (`protocol_config_mem_write_handler.c`)

| Function | Purpose |
|----------|---------|
| `_extract_write_under_mask_command_parameters()` | Parse address, byte count, encoding, data/mask pointers from incoming datagram |
| `_is_valid_write_under_mask_parameters()` | Validate space present, not read-only, bounds, 1-64 bytes, even data+mask length |
| `_write_data_under_mask()` | Read current values, apply mask, write back merged values |
| `_dispatch_write_under_mask_request()` | Two-phase dispatcher: phase 1 validates + ACKs, phase 2 reads-modifies-writes |

### Per-Space Dispatch Functions (`protocol_config_mem_write_handler.c`)

| Function | Space |
|----------|-------|
| `write_under_mask_space_config_description_info()` | CDI (0xFF) |
| `write_under_mask_space_all()` | All (0xFE) |
| `write_under_mask_space_config_memory()` | Config (0xFD) |
| `write_under_mask_space_acdi_manufacturer()` | ACDI-Mfg (0xFC) |
| `write_under_mask_space_acdi_user()` | ACDI-User (0xFB) |
| `write_under_mask_space_train_function_definition_info()` | Train FDI (0xFA) |
| `write_under_mask_space_train_function_config_memory()` | Train Fn Config (0xF9) |
| `write_under_mask_space_firmware()` | Firmware (0xEF) |

### Wiring (`openlcb_config.c`)

All 8 `memory_write_under_mask_space_*` pointers are wired in
`_build_datagram_handler()` under `#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION`.

### Configuration Flag

`write_under_mask_supported = true` in all application `node_parameters` structs
and in the Node Builder code generator (`codegen.js`). This correctly reflects
the implemented capability.

---

## 3. Payload Format

Sub-command 0x08 (space in byte 6):
```
Byte 0:    0x20 (CONFIG_MEM_CONFIGURATION)
Byte 1:    0x08 (CONFIG_MEM_WRITE_UNDER_MASK_SPACE_IN_BYTE_6)
Bytes 2-5: Starting address (big-endian 32-bit)
Byte 6:    Address space number
Bytes 7 .. 7+N-1: Data bytes (N bytes)
Bytes 7+N .. 7+2N-1: Mask bytes (N bytes)
```

Shorthand encodings (sub-commands 0x09, 0x0A, 0x0B with implicit space):
```
Byte 0:    0x20
Byte 1:    0x09/0x0A/0x0B
Bytes 2-5: Starting address
Bytes 6 .. 6+N-1: Data bytes (N bytes)
Bytes 6+N .. 6+2N-1: Mask bytes (N bytes)
```
