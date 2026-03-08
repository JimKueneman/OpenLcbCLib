# Node Builder Tool — Specification

**Project:** OpenLcbCLib
**Tool:** node_builder
**Location:** `tools/node_builder/`
**Status:** Specification / Design Phase
**Date:** 2026-03-01

---

## 1. Problem Statement

Every OpenLcbCLib application requires the user to define and maintain a set of files that
describe their node's identity, capabilities, and configuration. Currently those files are:

| File | Purpose |
|---|---|
| `openlcb_user_config.h` | Compile-time feature flags and buffer/size constants |
| `node_parameters.c/.h` | Node identity, protocol support, address space descriptors, embedded CDI/FDI byte arrays |
| `callbacks.c/.h` | Application event handler implementations |

These files are **interdependent** — changes to one often require coordinated changes in
the others. The dependencies are currently implicit, undocumented, and enforced only at
runtime (if at all). Examples:

- `OPENLCB_COMPILE_DATAGRAMS` in `openlcb_user_config.h` requires `PSI_DATAGRAM` to be
  set in `node_parameters.c` and the datagram address space to be present.
- The CDI byte array length in `node_parameters.c` must equal `highest_address + 1` in
  the same file, and `USER_DEFINED_CDI_LENGTH` in `openlcb_user_config.h` must be >= that
  length.
- The Configuration space `highest_address` must cover what the CDI actually describes.
- If `OPENLCB_COMPILE_BROADCAST_TIME` is enabled, the broadcast time callback must be
  implemented in `callbacks.c`.

This creates friction for all users and is a significant barrier for beginners.

---

## 2. Goals

1. **Single source of truth.** All node configuration lives in one user-edited file:
   `node_config.h`. All other generated artifacts are derived from it.

2. **Compile-time enforcement.** Inconsistencies between settings are caught by the
   compiler via `#error` directives, not discovered at runtime.

3. **Tool-assisted editing.** A browser-based or script-based tool reads and writes
   `node_config.h` so beginners never need to edit it directly.

4. **Round-trip capability.** The tool can open an existing `node_config.h`, display its
   current values, allow modification, and write the result back. It is not a one-way
   code generator.

5. **CDI/FDI automation.** The tool accepts CDI and FDI XML, validates them, generates
   the byte arrays, computes lengths, and writes everything into `node_config.h`
   automatically. This replaces the manual workflow of `cdi_to_array` /
   `cdi_to_array_webbrowser` + cut-and-paste.

6. **No new runtime dependencies.** The tool runs on the developer's machine. The
   generated `node_config.h` and any derived headers must compile with a standard C99
   compiler and no additional libraries.

---

## 3. Non-Goals

- The tool generates `callbacks.c/.h` stubs for new projects but does not overwrite
  existing ones. Once the user has customized their callbacks file, it is their
  responsibility to maintain it.
- The tool does not replace the IDE or build system.
- The tool does not validate CDI/FDI XML against the full OpenLCB schema (initially —
  this may be added later). It validates structure sufficient to generate a correct byte
  array.
- The tool does not manage Node IDs. Node ID assignment is out of scope.

---

## 4. The `node_config.h` File

### 4.1 Design Principles

- One file, human-readable, heavily commented.
- All settings are `#define` macros so the compiler can process them directly.
- Intermediate and expert users can edit it by hand. Comments explain every setting and
  its dependencies.
- The file is self-documenting: reading it top to bottom tells you everything about the
  node.

### 4.2 Structure

The file is organized into clearly labelled sections. The order mirrors the tool's panel
layout:

```
Section 1:  Feature Flags
Section 2:  Node Identity (SNIP strings)
Section 3:  Protocol Support Flags
Section 4:  Buffer Pool Sizing
Section 5:  Event System (if Events enabled)
Section 6:  Memory Configuration Layout (if Memory Configuration enabled)
Section 7:  CDI
Section 8:  FDI (if Train enabled)
Section 9:  Address Space Descriptors
Section 10: Configuration Options (if Memory Configuration enabled)
Section 11: Train Protocol (if Train enabled)
Section BC: Backward Compatibility Aliases
```

### 4.3 Feature Flags (Section 1)

These drive everything else. Enabling a flag creates obligations in later sections that
the `#error` guards enforce. The tool enforces a dependency cascade: checking a flag
auto-checks its prerequisites, and unchecking a flag auto-unchecks its dependents.

**Dependency cascade rules:**
- Check `OPENLCB_COMPILE_FIRMWARE` -> auto-check `OPENLCB_COMPILE_MEMORY_CONFIGURATION` + `OPENLCB_COMPILE_DATAGRAMS`
- Check `OPENLCB_COMPILE_MEMORY_CONFIGURATION` -> auto-check `OPENLCB_COMPILE_DATAGRAMS`
- Check `OPENLCB_COMPILE_BROADCAST_TIME` -> auto-check `OPENLCB_COMPILE_EVENTS`
- Unchecking reverses dependents (e.g. uncheck Datagrams -> uncheck Memory Configuration -> uncheck Firmware)

```c
#define OPENLCB_COMPILE_EVENTS
#define OPENLCB_COMPILE_DATAGRAMS
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION
#define OPENLCB_COMPILE_FIRMWARE
#define OPENLCB_COMPILE_BROADCAST_TIME
// #define OPENLCB_COMPILE_TRAIN
// #define OPENLCB_COMPILE_STREAM              // unreleased — disabled in tool
```

`OPENLCB_COMPILE_STREAM` is present in the tool UI but disabled and tagged "unreleased".
It cannot be checked by the user. When eventually released, it will require Datagrams and
Memory Configuration.

`OPENLCB_COMPILE_FIRMWARE` controls whether the firmware upgrade address space (0xEF) is
advertised and the `PSI_FIRMWARE_UPGRADE` protocol bit is set.

The Feature Flags panel uses **beginner-friendly labels** (e.g. "Event Exchange",
"Datagrams", "Memory Configuration", "Firmware Upgrade", "Broadcast Time",
"Train Control (Traction)", "Streaming") instead of raw `#define` names. Each checkbox
has a short **help description** explaining what it enables. Common flags are tagged
"common". The `OPENLCB_COMPILE_*` define names are not shown in the UI.

The Feature Flags panel also contains:
- **CDI file-picker** — a compact row with "Load CDI XML" button, "Generate Starter"
  button, and file name/byte count display. Visible when Memory Configuration is checked.
  The row is a drag-and-drop target for XML files. A clear (x) button appears when a
  file is loaded.
- **FDI file-picker** — same layout as CDI. Visible when Train is checked. Also a
  drag-and-drop target with clear button.
- **Protocol Support** — an auto-derived read-only indicator list showing which PSI bits
  are active based on the current feature flag state.

### 4.4 Node Identity (Section 2 — SNIP)

The SNIP panel is **always visible** in the sidebar. The fields have three states
depending on the Memory Configuration flag and CDI status:

- **Config Mem OFF** — fields are **editable**. The user types values directly.
- **Config Mem ON, no CDI loaded** — fields are **disabled** (greyed out). A note
  directs the user to Feature Flags to load a CDI XML file.
- **Config Mem ON, CDI loaded** — fields are **read-only** with values extracted from
  the CDI `<identification>` block. A note explains where the values came from and
  that the user should edit their CDI XML to change them.

When the CDI is cleared, the fields return to the disabled state (if Config Mem is
still enabled) or editable (if Config Mem was disabled). When Config Mem is toggled,
the SNIP field state updates accordingly.

```c
#define NODE_SNIP_MFG_VERSION        4
#define NODE_SNIP_NAME               "Basic OpenLcb Node"
#define NODE_SNIP_MODEL              "Test Application"
#define NODE_SNIP_HARDWARE_VERSION   "0.0.1"
#define NODE_SNIP_SOFTWARE_VERSION   "0.0.1"
#define NODE_SNIP_USER_VERSION       2
```

### 4.5 Protocol Support Flags

Protocol Support is no longer a standalone section or panel. It is auto-derived from
feature flags and displayed as a read-only indicator list inside the Feature Flags panel.
The `NODE_PROTOCOL_SUPPORT` macro is still emitted in `node_config.h` for compile-time
use.

**`PSI_SIMPLE` is intentionally excluded.** The Simple Protocol bit was envisioned 15
years ago when microprocessors were much less capable. This library shall never set that
bit. `PSI_SIMPLE_NODE_INFORMATION` is always included (every node supports SNIP).

```c
#define NODE_PROTOCOL_SUPPORT  ( \
    PSI_DATAGRAM                 | \
    PSI_MEMORY_CONFIGURATION     | \
    PSI_EVENT_EXCHANGE           | \
    PSI_SIMPLE_NODE_INFORMATION  | \
    PSI_CONFIGURATION_DESCRIPTION_INFO | \
    PSI_ABBREVIATED_DEFAULT_CDI  | \
    PSI_FIRMWARE_UPGRADE           \
)
```

### 4.6 Buffer Pool Sizing (Section 4)

Each value sets the maximum buffer depth for that pool. Labels spell out the full byte
size (e.g. "16 Bytes each" not "16 B each"). Help text emphasizes that each value is
the maximum number of buffers that can be allocated.

```c
#define NODE_BASIC_BUFFER_DEPTH       32
#define NODE_DATAGRAM_BUFFER_DEPTH    4
#define NODE_SNIP_BUFFER_DEPTH        4
#define NODE_STREAM_BUFFER_DEPTH      1
#define NODE_BUFFER_DEPTH             4
```

### 4.7 Event System Buffers (Section 5)

Named "Event System Buffers" in the UI (sidebar and panel title). The panel is **always
visible** in the sidebar. When the Events feature flag is unchecked, all fields are
**disabled** (greyed out) and an info message explains that library defaults will be used.
The disabled defaults are: 0 producers, 0 consumers, 1 producer range, 1 consumer range.

Help text emphasizes that each value is the maximum number of event IDs that can be
created.

```c
#define NODE_PRODUCER_COUNT           64
#define NODE_PRODUCER_RANGE_COUNT     5
#define NODE_CONSUMER_COUNT           32
#define NODE_CONSUMER_RANGE_COUNT     5
```

### 4.8 Memory Configuration Layout

Memory Configuration fields (User Name Address, User Description Address, Highest Address)
are in the Address Spaces **0xFD tab**, under a "Configuration Memory Layout" section
divider that appears after the common address space fields.

```c
#define NODE_CONFIG_MEM_USER_NAME_ADDRESS         0x00000000
#define NODE_CONFIG_MEM_USER_DESCRIPTION_ADDRESS  62
#define NODE_CONFIG_MEM_HIGHEST_ADDRESS           0x0200
```

### 4.9 CDI (Section 7)

The CDI XML is maintained as a separate file edited by the user in their preferred XML
editor. `node_config.h` stores only the path to that file and the generated byte array
derived from it. When the tool is run it reads the XML from the stored path,
regenerates the byte array and all derived values, and rewrites the generated block.
The user never edits the generated block by hand.

The CDI file-picker and "Generate Starter" button are located in the Feature Flags panel.
The CDI byte array preview is displayed in the Address Spaces 0xFF tab.

```c
// Path to CDI XML source — edit this file in your XML editor, then run node_builder
#define NODE_CDI_XML_PATH  "node_cdi.xml"

// Generated block — do not edit below, regenerated by node_builder from NODE_CDI_XML_PATH
#define NODE_CDI_LENGTH  1098
#define NODE_CDI_BYTES   { \
    0x3C, 0x3F, 0x78, ...  \
}
```

`NODE_CDI_LENGTH` is the single source of truth for CDI size. It is referenced by:
- `address_space_configuration_definition.highest_address` = `NODE_CDI_LENGTH - 1`
- `USER_DEFINED_CDI_LENGTH` (kept for backward compatibility, defined as `NODE_CDI_LENGTH`)

### 4.10 FDI (Section 8)

Same pattern as CDI. Only present when `OPENLCB_COMPILE_TRAIN` is defined. The FDI
file-picker and "Generate Starter" button are located in the Feature Flags panel.

```c
// Path to FDI XML source — edit this file in your XML editor, then run node_builder
#define NODE_FDI_XML_PATH  "node_fdi.xml"

// Generated block — do not edit below, regenerated by node_builder from NODE_FDI_XML_PATH
#define NODE_FDI_LENGTH  500
#define NODE_FDI_BYTES   { \
    0x3C, 0x3F, 0x78, ...  \
}
```

### 4.11 Address Space Descriptors (Section 9)

Each address space is defined by a set of macros. The tool computes values that depend
on CDI/FDI length automatically. The user sets policy (read-only, present, etc.).

Address spaces are presented as a **tabbed interface** in the tool UI. All tabs have the
**same consistent layout** with exactly 6 rows in the same order: Present (checkbox),
Read Only (checkbox), Low Address Valid (checkbox), Low Address (text input),
Highest Address (text input), Description (text input). Every row has inline help text.

Boolean fields use **checkboxes**, address/string fields use **text inputs**. Fields
that are constants for a given space use **disabled** checkboxes and inputs (greyed out
with reduced opacity). Fields that are user-configurable are enabled and editable.
Low Address is always visible but only enabled when Low Address Valid is checked.
This consistent structure means the view looks identical as the user switches between
tabs — only 0xFD has additional rows after the common block.

Tabs show and hide based on the current feature flag state:
- **0xFF (CDI)** — visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled.
  All common fields are disabled checkboxes/inputs.
  Includes CDI byte array preview below the common fields.
- **0xFE (All Memory)** — visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled.
  Present, Read Only, Low Address Valid are enabled checkboxes. Low Address is enabled
  only when Low Address Valid is checked. Highest Address and Description are enabled
  inputs. When Present is unchecked, all other fields are **disabled** (not hidden).
- **0xFD (Configuration)** — visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is
  enabled. Common fields first: Present (disabled, checked), Read Only (disabled,
  unchecked), Low Address Valid (disabled, auto-set), Low Address (disabled, auto-set),
  Highest Address (editable), Description (editable). Then a "Configuration Memory
  Layout" section with: User Name Address (editable), User Description Address
  (auto-calculated readonly), First User-Defined Address (auto-calculated readonly).
- **0xFC (ACDI Manufacturer)** — visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is
  enabled. All common fields are disabled. Description is editable.
- **0xFB (ACDI User)** — visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled.
  All common fields are disabled. Description is editable.
- **0xFA (Train FDI)** — visible when `OPENLCB_COMPILE_TRAIN` is enabled. All common
  fields are disabled. Description is editable.
- **0xF9 (Train Function Config)** — visible when `OPENLCB_COMPILE_TRAIN` is enabled.
  All common fields are disabled except Highest Address and Description (editable).
- **0xEF (Firmware)** — visible when `OPENLCB_COMPILE_FIRMWARE` is enabled. All common
  fields are disabled. Description is editable.

If the user's currently active tab becomes hidden due to a flag change, the view
automatically switches to the **first visible tab** (not necessarily 0xFF, since it
may also be hidden).

```c
// Space 0xFF — Configuration Definition Info (CDI)
#define NODE_ADDRSPACE_CDI_DESCRIPTION    ""
/* highest_address = NODE_CDI_LENGTH - 1 (computed) */

// Space 0xFE — All Memory
#define NODE_ADDRSPACE_ALL_PRESENT        true
#define NODE_ADDRSPACE_ALL_READONLY       true
#define NODE_ADDRSPACE_ALL_LOW_VALID      false
#define NODE_ADDRSPACE_ALL_HIGH           0xFFFF
#define NODE_ADDRSPACE_ALL_DESCRIPTION    ""

// Space 0xFD — Configuration
#define NODE_ADDRSPACE_CONFIG_LOW_VALID   false
#define NODE_ADDRSPACE_CONFIG_HIGH        NODE_CONFIG_MEM_HIGHEST_ADDRESS
#define NODE_ADDRSPACE_CONFIG_DESCRIPTION ""

// Space 0xFC — ACDI Manufacturer
#define NODE_ADDRSPACE_ACDI_MFG_DESCRIPTION ""

// Space 0xFB — ACDI User
#define NODE_ADDRSPACE_ACDI_USER_DESCRIPTION ""

// Space 0xEF — Firmware (present when OPENLCB_COMPILE_FIRMWARE enabled)
#define NODE_ADDRSPACE_FIRMWARE_DESCRIPTION ""

// Space 0xFA — Train FDI (train nodes only)
#define NODE_ADDRSPACE_FDI_DESCRIPTION    ""

// Space 0xF9 — Train Function Configuration (train nodes only)
#define NODE_ADDRSPACE_TRAIN_CONFIG_HIGH          0x0100
#define NODE_ADDRSPACE_TRAIN_CONFIG_DESCRIPTION   ""
```

---

## 5. `node_parameters_t` Struct — Member Mapping Rules

This section documents the `node_parameters_t` struct that `node_parameters.c`
initializes, and specifies exactly where each member's value comes from in `node_config.h`.

The struct is defined in `src/openlcb/openlcb_types.h` as:

```c
typedef struct {

    user_snip_struct_t snip;
    uint64_t protocol_support;
    uint8_t cdi[USER_DEFINED_CDI_LENGTH];
    uint8_t fdi[USER_DEFINED_FDI_LENGTH];
    user_address_space_info_t address_space_configuration_definition; // Space 0xFF
    user_address_space_info_t address_space_all;                      // Space 0xFE
    user_address_space_info_t address_space_config_memory;            // Space 0xFD
    user_address_space_info_t address_space_acdi_manufacturer;        // Space 0xFC
    user_address_space_info_t address_space_acdi_user;                // Space 0xFB
    user_address_space_info_t address_space_train_function_definition_info; // Space 0xFA
    user_address_space_info_t address_space_train_function_config_memory;   // Space 0xF9
    user_configuration_options configuration_options;
    user_address_space_info_t address_space_firmware;                 // Space 0xEF

} node_parameters_t;
```

Supporting sub-structs referenced above:

```c
typedef struct {
    uint8_t mfg_version;
    char name[LEN_SNIP_NAME_BUFFER];                         // max 41 chars incl. null
    char model[LEN_SNIP_MODEL_BUFFER];                       // max 41 chars incl. null
    char hardware_version[LEN_SNIP_HARDWARE_VERSION_BUFFER]; // max 21 chars incl. null
    char software_version[LEN_SNIP_SOFTWARE_VERSION_BUFFER]; // max 21 chars incl. null
    uint8_t user_version;
} user_snip_struct_t;

typedef struct {
    bool write_under_mask_supported : 1;
    bool unaligned_reads_supported : 1;
    bool unaligned_writes_supported : 1;
    bool read_from_manufacturer_space_0xfc_supported : 1;
    bool read_from_user_space_0xfb_supported : 1;
    bool write_to_user_space_0xfb_supported : 1;
    bool stream_read_write_supported : 1;
    uint8_t high_address_space;
    uint8_t low_address_space;
    char description[LEN_CONFIG_MEM_OPTIONS_DESCRIPTION]; // max 63 chars incl. null
} user_configuration_options;

typedef struct {
    bool present : 1;
    bool read_only : 1;
    bool low_address_valid : 1;
    uint8_t address_space;
    uint32_t highest_address;
    uint32_t low_address;
    char description[LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION]; // max 59 chars incl. null
} user_address_space_info_t;
```

### 5.1 Member-by-Member Mapping

The tables below list every field in `node_parameters_t` and its sub-structs, the
`node_config.h` macro that supplies its value, how it is derived, any validation rule
that must hold, and whether the field is shown in the tool UI.

**Source legend:**
- **User-set** — the user provides this value directly
- **Extracted from CDI** — parsed automatically from the CDI XML when loaded
- **Computed** — derived by the tool or a `#define` expression from other values
- **Fixed** — constant across all projects; never changes

**Visibility legend:**
- **Yes** — always shown as an editable field
- **No** — never shown; set silently by tool or hardcoded in `node_parameters.c`
- **Read-only** — shown for informational purposes but not editable
- **If no CDI** — shown as editable; becomes read-only once CDI `<identification>` is loaded
- **If `<flag>`** — shown only when the named feature flag is enabled

---

#### 5.1.1 `snip` — `user_snip_struct_t`

When a CDI XML is loaded the tool extracts `name`, `model`, `hardware_version`, and
`software_version` from the `<identification>` section and populates these fields
automatically. They become read-only in the UI with a note explaining the values came
from the CDI — the user edits the CDI XML to change them. If no CDI is present (or no
`<identification>` block exists) they are manually editable.

The SNIP panel ("Node Identity (SNIP)") is **always visible** in the sidebar. When
Memory Configuration is enabled but no CDI is loaded, the fields are **disabled** with a
note directing the user to load a CDI in Feature Flags. When Memory Configuration is
disabled, the fields are **editable**.

`mfg_version` and `user_version` are fixed by the OpenLCB spec and are never editable.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `snip.mfg_version` | `NODE_SNIP_MFG_VERSION` | Fixed = `4` | No | Always 4 per spec |
| `snip.name` | `NODE_SNIP_NAME` | Extracted from CDI `<manufacturer>` | If no CDI | Max 40 chars + null |
| `snip.model` | `NODE_SNIP_MODEL` | Extracted from CDI `<model>` | If no CDI | Max 40 chars + null |
| `snip.hardware_version` | `NODE_SNIP_HARDWARE_VERSION` | Extracted from CDI `<hardwareVersion>` | If no CDI | Max 20 chars + null |
| `snip.software_version` | `NODE_SNIP_SOFTWARE_VERSION` | Extracted from CDI `<softwareVersion>` | If no CDI | Max 20 chars + null |
| `snip.user_version` | `NODE_SNIP_USER_VERSION` | Fixed = `2` | No | Always 2 per spec |

---

#### 5.1.2 `protocol_support` — `uint64_t`

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `protocol_support` | `NODE_PROTOCOL_SUPPORT` | Computed from feature flags | Read-only | Each enabled feature flag requires its corresponding PSI bit. See Section 6.1. Displayed as an auto-derived indicator list inside the Feature Flags panel. |

---

#### 5.1.3 `cdi[]` — CDI byte array

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `cdi[USER_DEFINED_CDI_LENGTH]` | `NODE_CDI_BYTES` | Computed — generated by tool from CDI XML | No (byte array preview shown in Address Spaces 0xFF tab) | Array size = `USER_DEFINED_CDI_LENGTH` = `NODE_CDI_LENGTH`. Byte count must equal `NODE_CDI_LENGTH`. User never edits this directly. |

---

#### 5.1.4 `fdi[]` — FDI byte array

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `fdi[USER_DEFINED_FDI_LENGTH]` | `NODE_FDI_BYTES` | Computed — generated by tool from FDI XML | No | Conditional on `OPENLCB_COMPILE_TRAIN`. Array size = `USER_DEFINED_FDI_LENGTH` = `NODE_FDI_LENGTH`. If train is not enabled a minimal placeholder is emitted. |

---

#### 5.1.5 `address_space_configuration_definition` — Space 0xFF (CDI)

`present` is driven by whether a CDI XML has been loaded — it is not a user decision.
All other policy fields are fixed by the spec. The only thing the user can provide is an
optional description string. The CDI byte array preview is displayed in this tab.

All common fields use **disabled** checkboxes and inputs (greyed out) to maintain
consistent layout across all address space tabs.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if CDI XML loaded, else `false` | Disabled checkbox (checked) | Auto-set; no user input |
| `read_only` | *(fixed)* | Fixed = `true` | Disabled checkbox (checked) | CDI space is always read-only per spec |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | CDI always starts at 0 |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | *(computed)* | `NODE_CDI_LENGTH - 1` | Disabled input (auto) | Auto-computed from CDI length |
| `address_space` | *(fixed)* | Fixed = `0xFF` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_CDI_DESCRIPTION` | User-set | Yes | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.6 `address_space_all` — Space 0xFE (All Memory)

Optional. Whether to advertise this space and how to configure it is entirely up to
the application and platform. All 6 common fields are always visible. When `present`
is unchecked, the other 5 fields are **disabled** (greyed out), not hidden. Low Address
is enabled only when both `present` and `low_address_valid` are checked.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | `NODE_ADDRSPACE_ALL_PRESENT` | User-set | Yes (checkbox, enabled) | |
| `read_only` | `NODE_ADDRSPACE_ALL_READONLY` | User-set | Checkbox — disabled when `present` unchecked | Typically `true` |
| `low_address_valid` | `NODE_ADDRSPACE_ALL_LOW_VALID` | User-set | Checkbox — disabled when `present` unchecked | Set `true` if memory does not start at 0 |
| `low_address` | `NODE_ADDRSPACE_ALL_LOW` | User-set | Input — enabled only when `present` AND `low_address_valid` both checked | Only meaningful when `low_address_valid` is true |
| `highest_address` | `NODE_ADDRSPACE_ALL_HIGH` | User-set | Input — disabled when `present` unchecked | Platform-dependent upper memory limit |
| `address_space` | *(fixed)* | Fixed = `0xFE` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_ALL_DESCRIPTION` | User-set | Input — disabled when `present` unchecked | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.7 `address_space_config_memory` — Space 0xFD (Configuration)

This tab shows common address space fields first (all consistent with other tabs), then
a "Configuration Memory Layout" section divider, followed by the Memory Configuration
layout fields (User Name Address, User Description Address, First User-Defined Address).

Low Address Valid and Low Address are **auto-calculated**: Low Address Valid checkbox is
checked when User Name Address is not zero; Low Address shows the User Name Address value.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if `OPENLCB_COMPILE_MEMORY_CONFIGURATION` enabled | Disabled checkbox (checked) | Auto-set from feature flag |
| `read_only` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Configuration space is always writable |
| `low_address_valid` | `NODE_ADDRSPACE_CONFIG_LOW_VALID` | Computed from User Name Address | Disabled checkbox (auto) | `true` when User Name Address != 0 |
| `low_address` | `NODE_ADDRSPACE_CONFIG_LOW` | Computed = User Name Address | Disabled input (auto) | Equals User Name Address when valid |
| `highest_address` | `NODE_ADDRSPACE_CONFIG_HIGH` | User-set = `NODE_CONFIG_MEM_HIGHEST_ADDRESS` | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Must be large enough to hold all variables described in the CDI |
| `address_space` | *(fixed)* | Fixed = `0xFD` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_CONFIG_DESCRIPTION` | User-set | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.8 `address_space_acdi_manufacturer` — Space 0xFC

The library fully handles Space 0xFC — it is always present with no user options.
All fields are fixed by the spec or the library. All common fields are shown as
**disabled** checkboxes and inputs for consistency with other tabs.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(fixed)* | Fixed = `true` | Disabled checkbox (checked) | Always present — library-managed |
| `read_only` | *(fixed)* | Fixed = `true` | Disabled checkbox (checked) | ACDI manufacturer data is always read-only per spec |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Always starts at 0 |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | *(fixed)* | Fixed = `124` | Disabled input (124) | 1 + 41 + 41 + 21 + 21 = 125 bytes, zero-indexed = 124 |
| `address_space` | *(fixed)* | Fixed = `0xFC` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_ACDI_MFG_DESCRIPTION` | User-set | Yes | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.9 `address_space_acdi_user` — Space 0xFB

The library handles Space 0xFB but it has a dependency: it mirrors the user name and
description from Space 0xFD (Configuration), so it is only valid when
`OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled and Space 0xFD is present with a valid
layout. When those conditions are met, `present` is `true` automatically — there are
no user options. All common fields are shown as **disabled** checkboxes and inputs.

`NODE_CONFIG_MEM_USER_NAME_ADDRESS` and `NODE_CONFIG_MEM_USER_DESCRIPTION_ADDRESS` must
align with the first 128 bytes of Space 0xFD so that ACDI writes (which target 0xFB)
are correctly mirrored into the Configuration space. These addresses are validated by the
tool — a warning is shown if they would fall outside the Configuration space range.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if `OPENLCB_COMPILE_MEMORY_CONFIGURATION` enabled AND Space 0xFD `highest_address` >= 127 | Disabled checkbox (checked) | Auto-set; no user input |
| `read_only` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | User name/description are writable per spec |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Always starts at 0 |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | *(fixed)* | Fixed = `127` | Disabled input (127) | 1 + 63 + 64 = 128 bytes, zero-indexed = 127 |
| `address_space` | *(fixed)* | Fixed = `0xFB` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_ACDI_USER_DESCRIPTION` | User-set | Yes | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.10 `address_space_train_function_definition_info` — Space 0xFA

Entire section is conditional on `OPENLCB_COMPILE_TRAIN`. `present` and `highest_address`
are computed from FDI presence, all other fields are fixed. All common fields are shown
as **disabled** checkboxes and inputs except Description.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if FDI XML loaded | Disabled checkbox (checked) | Auto-set; conditional on `OPENLCB_COMPILE_TRAIN` |
| `read_only` | *(fixed)* | Fixed = `true` | Disabled checkbox (checked) | FDI is always read-only per spec |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Always starts at 0 |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | *(computed)* | `NODE_FDI_LENGTH - 1` | Disabled input (auto) | If `OPENLCB_COMPILE_TRAIN` — auto-computed from FDI length |
| `address_space` | *(fixed)* | Fixed = `0xFA` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_FDI_DESCRIPTION` | User-set | If `OPENLCB_COMPILE_TRAIN` | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.11 `address_space_train_function_config_memory` — Space 0xF9

Entire section conditional on `OPENLCB_COMPILE_TRAIN`. `present` is computed; all other
policy fields are user-set. Common fields shown as **disabled** checkboxes and inputs
except Highest Address and Description (editable).

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if `OPENLCB_COMPILE_TRAIN` enabled | Disabled checkbox (checked) | Auto-set from feature flag |
| `read_only` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Train function configuration space is writable |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Always starts at 0 |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | `NODE_ADDRSPACE_TRAIN_CONFIG_HIGH` | User-set | If `OPENLCB_COMPILE_TRAIN` | Size of train function configuration space |
| `address_space` | *(fixed)* | Fixed = `0xF9` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_TRAIN_CONFIG_DESCRIPTION` | User-set | If `OPENLCB_COMPILE_TRAIN` | Optional; max 59 chars; defaults to empty with placeholder text |

---

#### 5.1.12 `configuration_options` — `user_configuration_options`

The Configuration Options panel is only visible when `OPENLCB_COMPILE_MEMORY_CONFIGURATION`
is checked.

`write_under_mask_supported` is always checked and disabled in the UI — it is
library-managed and cannot be changed by the user.

`stream_read_write_supported` is driven by the `OPENLCB_COMPILE_STREAM` feature flag.
When the Stream flag is disabled (currently always, since it is unreleased), the
Stream Read/Write checkbox is disabled and unchecked. When the Stream flag is enabled,
the checkbox becomes editable.

`high_address_space` is always `0xFF` — the CDI space is always the highest.

`low_address_space` is auto-derived from the active feature flags:
- `0xEF` if `OPENLCB_COMPILE_FIRMWARE` is enabled
- `0xF9` if `OPENLCB_COMPILE_TRAIN` is enabled (and firmware is not)
- `0xFB` if `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled (and neither firmware nor train)
- `0xFC` otherwise

The three ACDI-related capability flags are directly linked to the ACDI space states
and are never independent user decisions:

- `read_from_manufacturer_space_0xfc_supported` mirrors Space 0xFC `present`, which is
  always `true` — so this flag is always `true`.
- `read_from_user_space_0xfb_supported` mirrors Space 0xFB `present`, which is `true`
  only when `OPENLCB_COMPILE_MEMORY_CONFIGURATION` is enabled and Space 0xFD `highest_address`
  >= 127. If Space 0xFB is not present this flag must be `false`.
- `write_to_user_space_0xfb_supported` follows the same condition as
  `read_from_user_space_0xfb_supported` — Space 0xFB is always writable when present
  (its `read_only` is fixed `false`), so this flag equals Space 0xFB `present`.

In other words: **if Space 0xFB is present both read and write flags are `true`; if
Space 0xFB is absent both are `false`.** There is no configuration where one is true
and the other is false. The tool enforces this automatically with no user input.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `write_under_mask_supported` | `NODE_CFGOPT_WRITE_UNDER_MASK` | Fixed = `true` | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` (checked, disabled — library-managed) | Always `true`; library-managed |
| `unaligned_reads_supported` | `NODE_CFGOPT_UNALIGNED_READS` | User-set | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Typically `true` |
| `unaligned_writes_supported` | `NODE_CFGOPT_UNALIGNED_WRITES` | User-set | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Typically `true` |
| `read_from_manufacturer_space_0xfc_supported` | *(fixed)* | Fixed = `true` | No | Space 0xFC is always present — library-managed |
| `read_from_user_space_0xfb_supported` | *(computed)* | Computed — `true` if Space 0xFB present (i.e. `OPENLCB_COMPILE_MEMORY_CONFIGURATION` enabled and Configuration space >= 128 bytes) | No | Auto-set |
| `write_to_user_space_0xfb_supported` | *(computed)* | Computed — same condition as `read_from_0xfb` | No | Auto-set |
| `stream_read_write_supported` | `NODE_CFGOPT_STREAM_RW` | Driven by `OPENLCB_COMPILE_STREAM` flag | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` (disabled when Stream flag is off) | Enabled only when `OPENLCB_COMPILE_STREAM` is checked |
| `high_address_space` | *(computed)* | Fixed = `0xFF` | No | Always 0xFF — CDI is always the highest space |
| `low_address_space` | *(computed)* | Computed — lowest present space ID based on active flags | No | Auto-derived: 0xEF if firmware, 0xF9 if train, 0xFB if config mem, 0xFC otherwise |
| `description` | `NODE_CFGOPT_DESCRIPTION` | User-set | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | Optional; max 63 chars; defaults to empty with placeholder text |

---

#### 5.1.13 `address_space_firmware` — Space 0xEF

`present` is driven by the `OPENLCB_COMPILE_FIRMWARE` feature flag. When the flag is
enabled the firmware address space is automatically present. When the flag is disabled
the space is not advertised and its tab is hidden. All common fields are shown as
**disabled** checkboxes and inputs except Description.

| Field | `node_config.h` Macro | Source | Visibility | Validation Rule |
|---|---|---|---|---|
| `present` | *(computed)* | Computed — `true` if `OPENLCB_COMPILE_FIRMWARE` enabled | Disabled checkbox (checked) | Auto-set from feature flag |
| `read_only` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Firmware space must be writable for upgrade per spec |
| `low_address_valid` | *(fixed)* | Fixed = `false` | Disabled checkbox (unchecked) | Always starts at 0 per spec |
| `low_address` | *(fixed)* | Fixed = `0` | Disabled input (0x00000000) | Always zero |
| `highest_address` | *(fixed)* | Fixed = `0xFFFFFFFF` | Disabled input (0xFFFFFFFF) | Defined by spec |
| `address_space` | *(fixed)* | Fixed = `0xEF` | No | Fixed library constant |
| `description` | `NODE_ADDRSPACE_FIRMWARE_DESCRIPTION` | User-set | If `OPENLCB_COMPILE_FIRMWARE` | Optional; max 59 chars; defaults to empty with placeholder text |

---

### 5.2 Visibility Summary

The table below consolidates all fields by visibility category to give a complete picture
of what the tool UI shows.

| Visibility | Fields |
|---|---|
| **Always shown (editable or state-dependent)** | SNIP fields: `snip.name`, `snip.model`, `snip.hardware_version`, `snip.software_version` — editable when Config Mem OFF, disabled when Config Mem ON without CDI, read-only when CDI loaded. |
| **If `OPENLCB_COMPILE_MEMORY_CONFIGURATION`** | Space 0xFF: all common fields (disabled checkboxes/inputs); Space 0xFE: all 6 common fields; Space 0xFD: common fields + `highest_address`, `description` (editable) + Configuration Memory Layout section; Space 0xFC: all common fields (disabled), description editable; Space 0xFB: all common fields (disabled), description editable; Config options panel visible |
| **If `OPENLCB_COMPILE_TRAIN`** | Space 0xFA: common fields (disabled) + `description` (editable); Space 0xF9: common fields (disabled) + `highest_address`, `description` (editable) |
| **If `OPENLCB_COMPILE_FIRMWARE`** | Space 0xEF: common fields (disabled) + `description` (editable) |
| **0xFE conditional (disable/enable, not hide/show)** | When `present` unchecked: `read_only`, `low_address_valid`, `low_address`, `highest_address`, `description` all **disabled**. When `low_address_valid` unchecked: `low_address` **disabled**. |
| **Never shown** | All `address_space` IDs; `snip.mfg_version`; `snip.user_version`; `high_address_space`; `low_address_space`; `cdi[]`; `fdi[]` |

---

### 5.3 Summary: User-Set vs. Computed vs. Fixed

| Category | Who sets it | Examples |
|---|---|---|
| **User-set** | User via tool or hand-edit | Buffer depths, event counts, Configuration space size, Space 0xFE policy, config options capability flags (unaligned reads/writes) |
| **Extracted from CDI** | Tool parses CDI XML | `snip.name`, `snip.model`, `snip.hardware_version`, `snip.software_version` |
| **Computed by tool** | node_builder generates | CDI/FDI byte arrays, `NODE_CDI_LENGTH`, `NODE_FDI_LENGTH`, `present` flags driven by feature flags or file presence |
| **Computed by expression** | `#define` arithmetic | `highest_address` for 0xFF = `NODE_CDI_LENGTH - 1`, `highest_address` for 0xFA = `NODE_FDI_LENGTH - 1`, `low_address_space` |
| **Fixed by spec or library** | Hardcoded in `node_parameters.c` | Space IDs, ACDI space sizes (124, 127), firmware `highest_address` (0xFFFFFFFF), CDI/FDI `read_only = true`, `mfg_version = 4`, `user_version = 2`, `write_under_mask = true`, `high_address_space = 0xFF` |

---

## 6. Compile-Time Validation

A header `openlcb_node_config_validate.h` (part of the library, not user-edited) contains
`#error` guards that enforce consistency. It is included by the library's core headers so
validation is automatic on every build.

### 6.1 Feature Flag vs. Protocol Support Guards

| Feature Flag | Required PSI Bit |
|---|---|
| `OPENLCB_COMPILE_DATAGRAMS` | `PSI_DATAGRAM` |
| `OPENLCB_COMPILE_MEMORY_CONFIGURATION` | `PSI_MEMORY_CONFIGURATION` |
| `OPENLCB_COMPILE_EVENTS` | `PSI_EVENT_EXCHANGE` |
| `OPENLCB_COMPILE_BROADCAST_TIME` | `PSI_BROADCAST_TIME` |
| `OPENLCB_COMPILE_TRAIN` | `PSI_TRACTION_CONTROL` |
| `OPENLCB_COMPILE_FIRMWARE` | `PSI_FIRMWARE_UPGRADE` |

### 6.2 Cross-Value Guards

| Condition | Error Message |
|---|---|
| `NODE_CDI_LENGTH > USER_DEFINED_CDI_LENGTH` | CDI byte array larger than buffer — increase `NODE_CDI_LENGTH` or regenerate |
| `NODE_FDI_LENGTH > USER_DEFINED_FDI_LENGTH` | FDI byte array larger than buffer |
| `OPENLCB_COMPILE_TRAIN` defined but `NODE_FDI_LENGTH` not defined | Train enabled but no FDI loaded |
| `NODE_ADDRSPACE_CDI_PRESENT = false` and `OPENLCB_COMPILE_MEMORY_CONFIGURATION` defined | Memory Configuration requires CDI space 0xFF to be present |
| `NODE_ADDRSPACE_CONFIG_PRESENT = false` and `OPENLCB_COMPILE_MEMORY_CONFIGURATION` defined | Memory Configuration requires space 0xFD to be present |
| `NODE_CONFIG_MEM_USER_NAME_ADDRESS` + 63 > `NODE_ADDRSPACE_CONFIG_HIGH` | ACDI user name field extends beyond Configuration space — increase Configuration space size or reduce `NODE_CONFIG_MEM_USER_NAME_ADDRESS` |
| `NODE_CONFIG_MEM_USER_DESCRIPTION_ADDRESS` + 64 > `NODE_ADDRSPACE_CONFIG_HIGH` | ACDI user description field extends beyond Configuration space |
| `NODE_ADDRSPACE_CONFIG_HIGH` < 127 and `OPENLCB_COMPILE_MEMORY_CONFIGURATION` defined | Configuration space too small to support Space 0xFB — must be at least 128 bytes (0x7F) |
| `OPENLCB_COMPILE_FIRMWARE` defined but `OPENLCB_COMPILE_MEMORY_CONFIGURATION` not defined | Firmware requires Memory Configuration |

---

## 7. The node_builder Tool

### 7.1 Delivery Format

A self-contained HTML file (`node_builder.html`) with accompanying CSS and JavaScript
modules that runs in any modern browser with no installation, no server, no internet
connection required. This follows the pattern already established by
`cdi_to_array_webbrowser/cdi2node.html`.

A companion Python script (`node_builder.py`) provides a command-line interface for
users who prefer scripting or want to integrate into a build pipeline.

### 7.2 Browser Tool — Workflow

**New Project:**
1. User opens `node_builder.html` in a browser.
2. Tool presents a sidebar navigation with all panels.
3. User checks feature flags. Dependencies are auto-cascaded. CDI/FDI file-pickers
   appear inline when their prerequisite flags are checked.
4. User loads CDI XML via the file-picker in the Feature Flags panel (or drags the file
   onto the file-picker row). Tool parses it, generates byte array, computes
   `NODE_CDI_LENGTH`, extracts SNIP fields from `<identification>`, locks those fields
   to read-only, and shows a note in the SNIP panel.
5. If train is enabled, user loads FDI XML. Same process.
6. Tool shows a summary of all computed values and any detected inconsistencies.
7. User clicks "Generate & Download" — tool produces `node_config.h` and offers it as a
   download.
8. Optionally, user selects which callbacks to include and downloads `callbacks.c/.h`.

**Existing Project (round-trip):**
1. User opens `node_builder.html`, loads their existing `node_config.h`.
2. Tool parses the file and populates all form fields with current values.
3. User modifies what they need (e.g. loads a new CDI XML).
4. Tool regenerates and offers the updated `node_config.h` as a download.

### 7.3 Browser Tool — UI Layout

The tool uses a **sidebar navigation layout** with a fixed sidebar on the left and a
content area on the right. One panel is visible at a time, selected by clicking the
corresponding sidebar nav item. The sidebar is organized into three groups:

**Group 1: OpenLCB/LCC Node Configuration**
1. **Feature Flags** — Checkboxes with beginner-friendly labels and help descriptions.
   Auto-dependency cascade. Contains CDI/FDI file-pickers and Protocol Support indicator.
2. **Configuration Options** — visible only when Memory Configuration is checked.
   Contains Write Under Mask (disabled), Unaligned Reads/Writes, Stream Read/Write.
3. **Node Identity (SNIP)** — always visible. Shows manufacturer name, model, hardware
   version, software version. Three states: editable (Config Mem OFF), disabled with
   note to load CDI (Config Mem ON, no CDI), read-only with CDI source note (CDI loaded).
4. **Train Protocol** — visible only when Train is checked.
5. **Address Spaces** — tabbed interface with 8 address space tabs. All tabs have
   identical 6-row layout: Present (checkbox), Read Only (checkbox), Low Address Valid
   (checkbox), Low Address (input), Highest Address (input), Description (input).
   Constant values use disabled checkboxes/inputs (greyed out). 0xFD tab has additional
   Configuration Memory Layout fields after the common block.

**Group 2: OpenLcbCLib Buffer Management**
6. **Buffer Pool Sizing** — always visible. Buffer depth for Basic (16 Bytes each),
   Datagram (72 Bytes each), SNIP (256 Bytes each), Stream (512 Bytes each), and
   Virtual Nodes.
7. **Event System Buffers** — always visible. Fields disabled when Events flag is
   unchecked. Maximum producer/consumer counts and range counts.

**Group 3: OpenLcbCLib Callback Defines**
8. **Callbacks** — checkbox list of available callbacks. Mandatory callbacks locked.
   Optional callbacks shown when their feature flag is enabled.

**Summary & Validation** — fixed at the bottom of the page (outside the sidebar layout).
Run Validation button checks configuration consistency.

### 7.4 Python CLI Tool — Usage

```
# Generate new node_config.h interactively
python node_builder.py --new

# Generate from a saved answers file (for scripting/CI)
python node_builder.py --from answers.json --cdi node_cdi.xml --fdi node_fdi.xml

# Validate an existing node_config.h without modifying it
python node_builder.py --validate node_config.h

# Update CDI in existing node_config.h
python node_builder.py --update node_config.h --cdi new_cdi.xml

# Update FDI in existing node_config.h
python node_builder.py --update node_config.h --fdi new_fdi.xml
```

### 7.5 Form State Persistence

The browser tool saves the entire form state to `localStorage` so that the user's
configuration survives browser refreshes. This is implemented in `js/persistence.js`.

**What is persisted:**
- All feature flag checkboxes and configuration option checkboxes
- All text/number input values (SNIP fields, buffer sizes, event counts, address space
  fields, train settings)
- CDI and FDI raw XML text strings and filenames (not the byte arrays — those are
  regenerated on restore via `processCDI()` / `processFDI()`)

**What is NOT persisted:**
- Read-only computed fields (they are re-derived from persisted inputs on restore)
- File input elements (the browser prohibits setting these for security)

**Mechanism:**
- A single delegated `input`/`change` event listener on `<main>` triggers a debounced
  `saveState()` (500ms delay after last change).
- Explicit `saveState()` calls are also made after CDI/FDI load, CDI/FDI clear, and
  roundtrip config import — these are async operations that bypass normal input events.
- On page load, `restoreState()` runs **before** the cascade functions
  (`onFeatureFlagChange`, `onAddrSpaceChange`, `onConfigSpaceChange`) so that the
  cascade correctly derives UI state from the restored values.

**Storage key:** `nodeBuilderState` in `localStorage`.

### 7.6 CDI/FDI Processing

The tool replaces the existing `cdi_to_array` and `cdi_to_array_webbrowser` tools.

**New project:** the user points the tool at their CDI XML file via the file-picker in
the Feature Flags panel (or drags the file onto the file-picker row). The tool writes
`NODE_CDI_XML_PATH` into `node_config.h` and generates the byte array. The byte array
preview is displayed in the Address Spaces 0xFF tab.

**Round-trip (existing project):** when `node_config.h` is loaded, the tool reads
`NODE_CDI_XML_PATH`, locates that file relative to `node_config.h`, and automatically
reloads and reprocesses the XML. The user does not need to re-select the file unless
the path has changed.

**Processing steps (same for CDI and FDI):**

1. Read the XML file from the path stored in `NODE_CDI_XML_PATH` / `NODE_FDI_XML_PATH`.
2. Validate it is well-formed XML.
3. Convert every character to its hex byte value.
4. Append a null terminator (`0x00`).
5. Compute `NODE_CDI_LENGTH` / `NODE_FDI_LENGTH` = byte count including null.
6. For CDI: extract `<manufacturer>`, `<model>`, `<hardwareVersion>`, `<softwareVersion>`
   from the `<identification>` section and populate the SNIP fields. Show a note in the
   SNIP panel indicating the source.
7. Format the output as a C initializer list with inline ASCII comments (same style as
   current `node_parameters.c`).
8. Write the generated block back into `node_config.h` between the clearly marked
   `// Generated block` delimiters, replacing any previous content.

### 7.7 Callbacks Generation

The tool generates `callbacks.c` and `callbacks.h` based on a checkbox selection in the
UI. Mandatory callbacks are always included with their checkboxes locked. Optional
callbacks appear only when their associated feature flag is enabled.

| Callback | Mandatory | Condition |
|---|---|---|
| `Callbacks_initialize(void)` | Yes | Always |
| `Callbacks_on_100ms_timer_callback(void)` | Yes | Always |
| `Callbacks_on_can_rx_callback(can_msg_t *)` | Yes | Always — fires for every inbound message on any transport (CAN, WiFi, TCP/IP future). Name is historical; not CAN-only. |
| `Callbacks_on_can_tx_callback(can_msg_t *)` | Yes | Always — fires for every outbound message on any transport. Name is historical; not CAN-only. |
| `Callbacks_alias_change_callback(uint16_t, node_id_t)` | Yes | Always |
| `Callbacks_on_login_complete(openlcb_node_t *)` | Yes | Always — stub must `return true` |
| `Callbacks_operations_request_factory_reset(...)` | No | If `OPENLCB_COMPILE_MEMORY_CONFIGURATION` |
| `Callbacks_on_consumed_event_identified(...)` | No | If `OPENLCB_COMPILE_EVENTS` |
| `Callbacks_on_consumed_event_pcer(...)` | No | If `OPENLCB_COMPILE_EVENTS` |
| `Callbacks_on_event_learn(...)` | No | If `OPENLCB_COMPILE_EVENTS` |
| `Callbacks_on_broadcast_time_changed(broadcast_clock_t *)` | No | If `OPENLCB_COMPILE_BROADCAST_TIME` |
| Train callbacks | No | If `OPENLCB_COMPILE_TRAIN` — TBD |

Each generated stub includes a comment block explaining what the callback is for and what
the user is expected to implement inside it. The tool will not overwrite an existing
`callbacks.c/.h` — it warns the user and offers a diff view instead.

---

## 8. Migration Path

Existing demos are not broken immediately. The migration path is:

1. `node_config.h` is introduced alongside the existing `node_parameters.c` structure.
2. `node_parameters.c` is refactored to pull its values from `node_config.h` macros
   rather than hardcoded literals.
3. `openlcb_user_config.h` defines its values as aliases of `node_config.h` macros for
   backward compatibility (`#define USER_DEFINED_CDI_LENGTH NODE_CDI_LENGTH`).
4. Demos are updated one by one to use the new structure.
5. Old `cdi_to_array` and `cdi_to_array_webbrowser` tools are marked deprecated but
   left in place.

---

## 9. File Layout After Implementation

```
tools/
  node_builder/
    docs/
      SPEC.md                  (this file)
      TODO.md
    node_builder.html          (browser tool)
    node_builder.css           (stylesheet)
    js/                        (JavaScript modules)
      utils.js
      protocol_support.js
      feature_flags.js
      cdi_processing.js
      fdi_processing.js
      callbacks.js
      address_spaces.js
      validation.js
      codegen.js
      roundtrip.js
      persistence.js
      app.js
    node_builder.py            (CLI tool)
    README.md

src/applications/<platform>/BasicNode/
    node_config.h              (user-edited or tool-generated — single source of truth)
    node_cdi.xml               (user's CDI source XML)
    node_fdi.xml               (user's FDI source XML, train nodes only)
    node_parameters.c/.h       (refactored to consume node_config.h macros)
    callbacks.c/.h             (generated stub or user-maintained application code)
    openlcb_user_config.h      (backward compat aliases only)
    main.c

src/openlcb/
    openlcb_node_config_validate.h   (library-owned compile-time guards)
```

---

## 10. Open Questions

1. **`node_config.h` location** — resolved. Top-level application directory alongside
   `node_parameters.c`. All XML paths in `NODE_CDI_XML_PATH` / `NODE_FDI_XML_PATH`
   are relative to that directory.

2. **CDI/FDI XML storage** — resolved. Stored as separate files. Paths recorded as
   `NODE_CDI_XML_PATH` / `NODE_FDI_XML_PATH` macros in `node_config.h`.

3. **`NODE_*` macro naming** — resolved. No conflicts found in core library headers.

4. **Space 0xFE (All Memory)** — resolved. Optional; up to the application.

5. **CLI tool without Python dependency — OPEN.** The browser tool cannot write files
   to disk directly. Options under consideration: (a) browser generates a sh/bat script
   for local execution, (b) browser handles all processing at generate-time requiring
   the user to re-open and re-load for updates, (c) a self-contained native binary.
   CLI approach deferred; browser tool is the primary v1 delivery.

6. **Train callbacks** — OPEN. Full set of train protocol callbacks not yet defined.
   Callbacks generation section will be completed once train demos exist.

7. **`can_rx`/`can_tx` callback mandatory status** — resolved. Both are mandatory on
   all transports including WiFi. The library uses `can_msg_t` as its internal message
   format regardless of physical transport, so these callbacks fire for every message
   regardless of whether the hardware is CAN or WiFi. A TCP/IP transport layer is
   planned for the future — the callback names will likely need revisiting at that
   point. The generated stub comment must not imply these are CAN-only.

---

## 11. Out of Scope for v1

- Schema validation of CDI/FDI XML against the official OpenLCB XSD.
- GUI for editing CDI/FDI XML content (the tool processes XML, it does not create it).
- Node ID management.
- Multi-node configurations (more than one virtual node per device).
- Integration with specific IDEs (Xcode, STM32CubeIDE, PlatformIO, etc.).
