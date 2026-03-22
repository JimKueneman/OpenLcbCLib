# Changelog — Node Wizard

All notable changes to the Node Wizard tool will be documented in this file.

For library changes, see the root `CHANGELOG.md`.

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
