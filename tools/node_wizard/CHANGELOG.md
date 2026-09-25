# Changelog — Node Wizard

All notable changes to the Node Wizard tool will be documented in this file.

For library changes, see the root `CHANGELOG.md`.

---

## [1.0.2] - 2026-09-25

### Added

- **DCC CV Programming add-on** (memory space 0xF8, library flag
  `OPENLCB_COMPILE_DCC_CV`). New checkbox in the Node Configuration add-ons,
  available on node types with Config Memory (disabled for Basic and
  Bootloader, hidden for the JS target). When enabled the generated
  `openlcb_user_config.h` defines the flag and the three
  `USER_DEFINED_DCC_CV_*` settings, `openlcb_user_config.c` marks
  `address_space_dcc_cv` present (highest address 1023) and lowers
  `configuration_options.low_address_space` to 0xF8 when no firmware space is
  below it, and a new **DCC CV Programming Callbacks** group offers
  `dcc_cv_read` and `dcc_cv_write`.

### Fixed

- **Generated `openlcb_user_config.h` now selects the transport.** The header
  never defined `OPENLCB_COMPILE_CAN`, so every generated project relied on the
  library's default-to-CAN fallback and compiled with its warning. A Transport
  Selection section (CAN on, TCP commented out) is emitted ahead of the
  feature flags, matching the templates.
- **Generated `openlcb_user_config.c` used a macro the library does not define.**
  The 0xF9 space and the derived `low_address_space` were emitted as
  `CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG`; the library name is
  `CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY`. Every generated
  node parameters file failed to compile until the name was corrected by hand.
- **Reply-time callback descriptions.** The `config_mem_read/write_delayed_reply_time`
  entries said to return `(0x80 | N)`; the callbacks return seconds and the
  library encodes the exponent.

---

## [1.0.1] — 2026-03-22

### Housekeeping

- **Consolidated duplicate `codegen.js`** — moved the active version from
  `node_config/js/codegen.js` to `js/codegen.js` and deleted the stale copy.
  Updated script references in `node_wizard.html`, `node_config.html`,
  `file_preview.html`, and `MAINTENANCE.md`.

---

## [1.0.0] — 2026-03-16

Initial production release of the Node Wizard toolchain.

### CDI Editor

- **Schema-aware autocomplete** — typing `<` or Ctrl+Space offers only elements and
  attributes valid at the cursor position, driven by the CDI 1.4 XSD via
  `generate_cdi_schema.py`.
- **Inline validation** — custom Emscripten/asm.js build of libxml2 2.16.0 validates
  against the official CDI XSD with accurate line numbers. Red gutter dots appear
  600 ms after each keystroke. A footer panel shows the full error list.
- **Semantic checks** — DOMParser-based Layer 3 catches domain-specific issues the XSD
  cannot express: missing ACDI, space 253 misconfiguration, min>max, duplicate
  singletons (`<identification>`, `<acdi>`), stray text in container and variable
  elements, and more.
- **Format button** — pretty-prints XML with 2-space indentation, preserving comments.
- **Generate Basic XML** — scaffolds a starter CDI document with guidance comments.
- **C byte array view** — generates a byte array initializer for manual
  `openlcb_user_config.c` builds.

### FDI Editor

- **Schema-aware autocomplete** — mirrors CDI editor, driven by the FDI XSD via
  `generate_fdi_schema.py`.
- **Inline validation** — same xmllint engine as CDI, validating against the FDI XSD.
- **Semantic checks** — Layer 3 checks for FDI-specific rules: valid function `kind`
  values (`binary`, `momentary`, `analog`), singleton `<segment>`, known FDI tags,
  stray text detection.
- **Format button** — matches CDI editor.
- **Function Map view** — visual grid of FDI functions with kind/size breakdown.

### Node Config

- **Hover help** — all sidebar elements including collapsible section headers show
  contextual help in the right-hand detail panel on hover; clears on mouse-leave.

### Callbacks

- **Tile-level hover help** — hovering a callback group tile shows a group-level
  explanation; hovering a specific checkbox shows function-level detail.
- **`groupDetail` field** — all 7 callback groups carry detailed descriptions
  referencing the relevant OpenLCB standards.

### Documentation

- **`MAINTENANCE.md`** — top-level maintenance index organized by update frequency.
- **`documentation/maintenance_instructions.md`** — full architecture reference with
  postMessage protocol, tile rules, descriptor badge logic, and step-by-step guide
  for adding new sidebar sections.
- **`vendor/xmllint/build_history/BUILD_HISTORY.md`** — complete rebuild instructions
  for the custom libxml2 Emscripten build.
- **`vendor/codemirror/build_history/BUILD_HISTORY.md`** — complete rebuild instructions
  for the CodeMirror 6 IIFE bundle.

### Vendor Libraries

- **xmllint** — custom Emscripten/asm.js compilation of libxml2 2.16.0 (1.5 MB) with
  6 post-processing patches for browser compatibility. Replaces the older kripken/xml.js
  (4.1 MB, inaccurate line numbers).
- **CodeMirror 6** — esbuild IIFE bundle with XML, C++, oneDark theme, and lint support.
