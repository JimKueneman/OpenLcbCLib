# Test Dependency Report

**Date**: 2026-03-21

## Executive Summary

The test suite contains **42 built test executables**:

- **41 tests** in the CMake `foreach` loop (25 openlcb + 13 canbus + 2 utilities + 1 utilities_pc)
- **1 bootloader test** built separately with its own library

Additionally, **6 compile-time validation `.cxx` files** exist on disk but are **not registered in any CMakeLists.txt** and are not currently built.

Each `.cxx` test file compiles into its **own separate executable**. There is no linking of multiple test files into a single binary. There are **zero cross-test `extern` declarations** and all test-local globals are `static`.

---

## Build Architecture

Each `_Test.cxx` file produces a separate executable that links against four precompiled libraries:

| Library | Source Directory | Source Files |
|---------|-----------------|-------------|
| `utilities` | `src/utilities/` | 2 |
| `utilities_pc` | `src/utilities_pc/` | 1 |
| `canbus` | `src/drivers/canbus/` | 12 |
| `openlcb` | `src/openlcb/` | 24 |

Special cases:
- **Bootloader test**: builds its own `openlcb_bootloader` static library from an explicit source list (excludes `protocol_config_mem_read_handler.c`), using `test/user_config/bootloader/openlcb_user_config.h`

---

## Shared Test Infrastructure

### main_Test.hxx

- **Location**: `src/test/main_Test.hxx`
- **Used by**: 40 of 42 built tests
- **Provides**: GTest/GMock setup, `main()` function, `TEST_get_argument()` helper
- **Nature**: header-only, no global state

### Tests NOT using main_Test.hxx (2 of 42 built tests)

- `openlcb_bootloader_Test.cxx`
- `threadsafe_stringlist_Test.cxx`

### Unbuilt compile-time validation files (6, not in CMakeLists.txt)

- `openlcb_minimal_compile_Test.cxx`
- `openlcb_broadcast_time_compile_Test.cxx`
- `openlcb_datagrams_only_compile_Test.cxx`
- `openlcb_events_only_compile_Test.cxx`
- `openlcb_memory_config_compile_Test.cxx`
- `openlcb_train_compile_Test.cxx`

---

## Cross-Test Dependencies: Summary

| Dependency Type | Count | Details |
|---|---|---|
| Cross-test `extern` refs | **0** | No test references symbols from another test |
| Shared test header | **1** | `main_Test.hxx` used by 40 of 42 built tests |
| Shared global state | **0** | All globals are `static` to each test |
| Shared precompiled libraries | **4** | utilities, utilities_pc, canbus, openlcb |
| Shared user config | **1** | `test/user_config/typical/` (all tests except bootloader) |

---

## The Real Coupling: Monolithic Library Linking

Every test in the `foreach` loop links the full `openlcb` and `canbus` libraries regardless of what it actually needs:

- A change to **any** `.c` source requires recompiling its library, then **re-linking all 41 foreach executables**
- `openlcb_float16_Test` links the entire `openlcb` library (24 files) but only needs `openlcb_float16.c`
- CAN-only tests like `can_buffer_fifo_Test` link against `openlcb` too

---

## Per-Test Include Dependencies

### CAN Bus Driver Tests (13 tests)

```
can_buffer_fifo_Test        -> can_buffer_fifo.h, can_buffer_store.h
can_buffer_store_Test       -> can_buffer_store.h, can_utilities.h, can_types.h
can_utilities_Test          -> can_utilities.h, openlcb_buffer_store.h, openlcb_utilities.h
can_rx_statemachine_Test    -> can_rx_statemachine.h, can_types.h, can_utilities.h, openlcb_defines.h, alias_mappings.h
can_tx_statemachine_Test    -> can_tx_statemachine.h, can_types.h, can_utilities.h, openlcb_types.h, openlcb_buffer_store.h
can_login_statemachine_Test -> can_login_statemachine.h, can_types.h, can_utilities.h, can_rx_message_handler.h, openlcb_types.h, openlcb_buffer_store.h, openlcb_node.h
can_main_statemachine_Test  -> can_main_statemachine.h, can_utilities.h, can_buffer_store.h, alias_mappings.h, alias_mapping_listener.h, openlcb_node.h
can_login_message_handler_Test -> can_login_message_handler.h, can_types.h, can_utilities.h, can_rx_message_handler.h, alias_mappings.h, openlcb_node.h
can_rx_message_handler_Test -> can_types.h, can_utilities.h, can_rx_message_handler.h, can_buffer_store.h, openlcb_types.h, openlcb_buffer_store.h
can_tx_message_handler_Test -> can_tx_message_handler.h, can_types.h, can_utilities.h, can_buffer_store.h, openlcb_types.h, openlcb_buffer_store.h
alias_mappings_Test         -> can_types.h, alias_mappings.h, openlcb_types.h
alias_mapping_listener_Test -> can_types.h, alias_mapping_listener.h, openlcb_types.h
can_multinode_e2e_Test      -> can_main_statemachine.h, can_login_*.h, can_rx_message_handler.h, can_buffer_store.h, can_buffer_fifo.h, alias_mappings.h, alias_mapping_listener.h, can_utilities.h, [openlcb headers]
```

### OpenLCB Core Tests (25 tests)

```
openlcb_buffer_store_Test   -> openlcb_buffer_store.h, openlcb_types.h
openlcb_buffer_fifo_Test    -> openlcb_buffer_store.h, openlcb_buffer_fifo.h, openlcb_types.h
openlcb_buffer_list_Test    -> openlcb_buffer_list.h, openlcb_buffer_store.h, openlcb_types.h
openlcb_float16_Test        -> openlcb_float16.h, cmath
openlcb_gridconnect_Test    -> openlcb_gridconnect.h, openlcb_types.h
openlcb_node_Test           -> openlcb_node.h, openlcb_types.h, openlcb_defines.h
openlcb_utilities_Test      -> openlcb_utilities.h, openlcb_types.h, openlcb_buffer_store.h, openlcb_node.h, protocol_broadcast_time_handler.h, protocol_train_search_handler.h, protocol_train_handler.h
openlcb_application_Test    -> openlcb_application.h, protocol_message_network.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
openlcb_login_statemachine_Test -> openlcb_login_statemachine.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_buffer_store.h, openlcb_utilities.h
openlcb_login_statemachine_handler_Test -> openlcb_login_statemachine_handler.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
openlcb_main_statemachine_Test -> openlcb_main_statemachine.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_broadcast_time_handler.h, protocol_train_search_handler.h, protocol_train_handler.h
openlcb_config_Test         -> openlcb_config.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_buffer_store.h, can_main_statemachine.h, can_types.h
openlcb_multinode_e2e_Test  -> openlcb_main_statemachine.h, openlcb_login_statemachine.h, openlcb_login_statemachine_handler.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_datagram_handler.h
openlcb_application_broadcast_time_Test -> openlcb_application_broadcast_time.h, openlcb_application.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_broadcast_time_handler.h
openlcb_application_train_Test -> openlcb_application_train.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_train_handler.h, openlcb_float16.h
protocol_datagram_handler_Test -> protocol_datagram_handler.h, protocol_message_network.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
protocol_event_transport_Test -> protocol_event_transport.h, protocol_message_network.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
protocol_message_network_Test -> protocol_message_network.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
protocol_snip_Test          -> protocol_snip.h, protocol_message_network.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
protocol_broadcast_time_handler_Test -> openlcb_utilities.h, openlcb_types.h, openlcb_defines.h, openlcb_buffer_store.h, openlcb_node.h, protocol_broadcast_time_handler.h, openlcb_application_broadcast_time.h
protocol_train_handler_Test -> protocol_train_handler.h, openlcb_application_train.h, openlcb_float16.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h
protocol_train_search_handler_Test -> openlcb_utilities.h, openlcb_types.h, openlcb_defines.h, openlcb_buffer_store.h, openlcb_buffer_list.h, openlcb_buffer_fifo.h, openlcb_node.h, protocol_train_search_handler.h, protocol_train_handler.h, openlcb_application_train.h
protocol_config_mem_read_handler_Test -> protocol_config_mem_read_handler.h, openlcb_application.h, openlcb_types.h, protocol_message_network.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_datagram_handler.h, protocol_snip.h, openlcb_application_train.h
protocol_config_mem_write_handler_Test -> protocol_config_mem_write_handler.h, openlcb_application.h, openlcb_types.h, protocol_message_network.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_datagram_handler.h, protocol_snip.h, openlcb_application_train.h
protocol_config_mem_operations_handler_Test -> protocol_config_mem_operations_handler.h, openlcb_application.h, openlcb_types.h, protocol_message_network.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_datagram_handler.h
```

### Utility Tests (3 tests)

```
mustangpeak_endian_helper_Test  -> mustangpeak_endian_helper.h
mustangpeak_string_helper_Test  -> mustangpeak_string_helper.h, cstring, string
threadsafe_stringlist_Test      -> threadsafe_stringlist.h, cstdlib
```

### Bootloader Test (1 test, separate build)

```
openlcb_bootloader_Test -> openlcb_user_config.h, openlcb_types.h, openlcb_defines.h, openlcb_node.h, openlcb_utilities.h, openlcb_buffer_store.h, protocol_datagram_handler.h, protocol_config_mem_write_handler.h, protocol_config_mem_operations_handler.h, protocol_snip.h
```

---

## Plan to Remove Library-Level Coupling

### Phase 1: Split main_Test.hxx (low effort, low risk)

Extract `main()` and `TEST_get_argument()` into separate headers so tests include only what they need. Minimal impact since it is already header-only.

### Phase 2: Granular OpenLCB library targets (medium effort, high value)

Replace the single `openlcb` library (24 source files) with granular CMake targets:

| Target | Source Files |
|--------|-------------|
| `openlcb_buffers` | `openlcb_buffer_store.c`, `openlcb_buffer_fifo.c`, `openlcb_buffer_list.c` |
| `openlcb_core` | `openlcb_node.c`, `openlcb_utilities.c`, `openlcb_config.c` |
| `openlcb_protocols` | `protocol_datagram_handler.c`, `protocol_event_transport.c`, `protocol_message_network.c`, `protocol_snip.c` |
| `openlcb_config_mem` | `protocol_config_mem_read_handler.c`, `protocol_config_mem_write_handler.c`, `protocol_config_mem_operations_handler.c` |
| `openlcb_train` | `protocol_train_handler.c`, `protocol_train_search_handler.c`, `openlcb_application_train.c`, `openlcb_float16.c` |
| `openlcb_broadcast_time` | `protocol_broadcast_time_handler.c`, `openlcb_application_broadcast_time.c` |
| `openlcb_transport` | `openlcb_gridconnect.c`, `openlcb_login_statemachine.c`, `openlcb_login_statemachine_handler.c`, `openlcb_main_statemachine.c` |
| `openlcb_application` | `openlcb_application.c` |

### Phase 3: Granular CAN library targets (same approach)

Split `canbus` (12 source files) into:

| Target | Source Files |
|--------|-------------|
| `canbus_buffers` | `can_buffer_store.c`, `can_buffer_fifo.c` |
| `canbus_alias` | `alias_mappings.c`, `alias_mapping_listener.c` |
| `canbus_login` | `can_login_statemachine.c`, `can_login_message_handler.c` |
| `canbus_main` | `can_main_statemachine.c` |
| `canbus_transport` | `can_rx_statemachine.c`, `can_tx_statemachine.c`, `can_rx_message_handler.c`, `can_tx_message_handler.c`, `can_utilities.c` |

### Phase 4: Per-test link lists (medium effort, completes isolation)

Replace the blanket `LINK_LIBS` in the `foreach` loop (line 51-56, 159-187 of `test/CMakeLists.txt`) with per-test link specifications. Currently every test gets `utilities + utilities_pc + canbus + openlcb`. Change to a mapping so each test links only what it needs.

**Example**: `openlcb_float16_Test` would link only `openlcb_train` instead of all four libraries.

### Expected Result

- Changing a `.c` source file only re-links tests that actually depend on it
- CAN-only tests stop linking against openlcb
- Leaf tests like `openlcb_float16_Test` link a single small target
- Incremental rebuild time drops significantly
