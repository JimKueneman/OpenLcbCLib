# Node Wizard — Maintenance Guide

Central reference for everything in the Node Wizard that needs to be updated over
time as the OpenLCB library evolves. Sections are ordered from **most likely to need
updating** to **least likely**.

---

## Table of Contents

**Routine Updates (when the OpenLCB spec changes)**
1. [Updating CDI / FDI Schemas](#1-updating-cdi--fdi-schemas) — A new CDI or FDI XSD is released.
2. [Adding Semantic Validation Rules](#2-adding-semantic-validation-rules) — New domain-specific checks beyond what the XSD enforces.
3. [Adding Test Cases](#3-adding-test-cases) — New validator rule, bug fix, or edge case to cover.

**Editor Feature Work (adding or changing functionality)**
4. [Feature Flags](#4-feature-flags) — Enabling or disabling features for release.
5. [Callbacks](#5-callbacks) — New callback functions in `openlcb_config_t`.
6. [Hardware Interface Drivers](#6-hardware-interface-drivers) — New or changed driver functions.
7. [Platform Drivers](#7-platform-drivers) — New target board or updated driver templates.
8. [Node Config — Well Known Events](#8-node-config--well-known-events) — New Well Known Events in the spec.

**Vendor Package Rebuilds (rare — only for bugs or new features)**
9. [Rebuilding the CodeMirror Bundle](#9-rebuilding-the-codemirror-bundle) — CM6 bug fix or new extension needed.
10. [Rebuilding the xmllint Library](#10-rebuilding-the-xmllint-library) — libxml2 bug or validation issue.

**Reference**
11. [Architecture Overview](#11-architecture-overview) — How the wizard is structured.

---

## 1. Updating CDI / FDI Schemas

**When:** OpenLCB releases a new version of the CDI or FDI XSD (`.xsd` files).

**What gets updated:** Two things need to stay in sync with the XSD:
- The **autocomplete schema** (what the editor suggests when you type `<`)
- The **embedded XSD** used by the validator (what xmllint validates against)

### Step 1 — Update the autocomplete schema

Two Python 3 scripts read the XSD and patch the schema block in `cm-adapter.js`:

```bash
python3 vendor/codemirror/build_history/generate_cdi_schema.py \
    --xsd "/path/to/cdi.xsd" --patch

python3 vendor/codemirror/build_history/generate_fdi_schema.py \
    --xsd "/path/to/fdi.xsd" --patch
```

These replace the blocks between marker comments in `cm-adapter.js`:
```
/* <<CDI_SCHEMA_START>> */ ... /* <<CDI_SCHEMA_END>> */
/* <<FDI_SCHEMA_START>> */ ... /* <<FDI_SCHEMA_END>> */
```

### Step 2 — Update the embedded XSD for validation

```bash
python3 update_xsd.py --cdi-xsd /path/to/cdi.xsd --fdi-xsd /path/to/fdi.xsd
```

This patches the XSD strings in the validator JS files between `<<CDI_XSD_START>>`
/ `<<CDI_XSD_END>>` (and FDI equivalents). It also copies the XSD files into
`schema/` for the test runner.

### Files involved

| File | Role |
|------|------|
| `vendor/codemirror/build_history/generate_cdi_schema.py` | Patches autocomplete schema in `cm-adapter.js` |
| `vendor/codemirror/build_history/generate_fdi_schema.py` | Patches autocomplete schema in `cm-adapter.js` |
| `vendor/codemirror/cm-adapter.js` | Contains `CDI_SCHEMA` and `FDI_SCHEMA` blocks |
| `update_xsd.py` | Patches embedded XSD in validator JS files |
| `cdi_editor/js/cdi_validator.js` | CDI validator — embedded XSD + semantic checks |
| `fdi_editor/js/fdi_validator.js` | FDI validator — embedded XSD + semantic checks |

---

## 2. Adding Semantic Validation Rules

**When:** The library grows new conventions that should be enforced, or a user reports
a common mistake the editor should catch.

**How validation works:** Each validator has two layers:

- **Layer 1+2** — Well-formedness and structural rules are handled by a custom
  Emscripten/asm.js compilation of libxml2 2.16.0 validating against the official XSD.
  This gives accurate line numbers. See `vendor/xmllint/build_history/BUILD_HISTORY.md`
  for the full story.
- **Layer 3** — Semantic hints use DOMParser to check domain-specific best practices
  that the XSD cannot express (missing ACDI, space 253, min>max, etc.).

Validation is async — `validate(xmlString)` returns a `Promise` that resolves to an
array of `{ severity, line, col, message }` objects. Results are cached so the two
callers (validation panel and CodeMirror linter) do not re-run xmllint for the same
XML. The CodeMirror linter runs automatically on every edit (600 ms debounce) and
shows red dots in the gutter for lines with errors.

**To add a new semantic check:** Append to the `_runSemanticChecks()` function in
`cdi_validator.js` or `fdi_validator.js`. Follow the existing pattern — use
DOMParser to query the XML tree and push `{ severity, line, col, message }` objects.

### Files involved

| File | Role |
|------|------|
| `cdi_editor/js/cdi_validator.js` | CDI validator — `_runSemanticChecks()` at the bottom |
| `fdi_editor/js/fdi_validator.js` | FDI validator — `_runSemanticChecks()` at the bottom |
| `vendor/xmllint/xmllint_patched.js` | Patched Emscripten/asm.js build of libxml2 (1.5 MB) |
| `vendor/xmllint/xmllint_api.js` | Async browser API wrapper for xmllint |

---

## 3. Adding Test Cases

**When:** A new validator rule is added, a bug is fixed, or an edge case is discovered.

**How to run:**

```bash
cd tools/node_wizard/test
npm install jsdom    # first time only
node run_tests.js
```

The test runner uses the same xmllint engine as the browser validators, so results
are identical.

**How to add a test:** Copy an existing XML file from `test/cdi/` (or `test/fdi/`),
modify it for the scenario being tested, and embed a `<!-- TEST_CHECKS -->` block
describing the expected outcomes. See `test/TEST_CASES.md` for the format.

### Files involved

| Location | Role |
|----------|------|
| `test/cdi/` | CDI XML test files, each self-describing expected results |
| `test/fdi/` | FDI XML test files |
| `test/run_tests.js` | Headless Node.js test runner |
| `test/TEST_CASES.md` | Format reference and test case inventory |

---

## 4. Feature Flags

**When:** A feature is being enabled or disabled for release.

**Current flags** — all in `js/app.js`:

| Flag | Default | Effect |
|------|---------|--------|
| `ENABLE_PLATFORM` | `false` | Shows/hides the Platform selection panel in the sidebar |

---

## 5. Callbacks

**When:** The library adds new callback functions to `openlcb_config_t`, a function
signature changes, or a new logical group of callbacks is needed.

**Detailed guide:** `callbacks/maintenance_guide.md`

### Files involved

| File | Role |
|------|------|
| `callbacks/js/callback_defs.js` | Callback group and function definitions |
| `callbacks/js/callback_codegen.js` | Generates `.c` / `.h` stub files |
| `callbacks/callbacks.html` | Node-type visibility rules |

---

## 6. Hardware Interface Drivers

**When:** The library adds a new driver function to a group, a function signature
changes, or a new driver group is needed.

The structure mirrors the Callbacks system — see the callbacks guide and the comments
in `driver_defs.js` for the data format.

### Files involved

| File | Role |
|------|------|
| `drivers/js/driver_defs.js` | Driver group and function definitions |
| `drivers/js/driver_codegen.js` | Generates `.c` / `.h` stub files |

---

## 7. Platform Drivers

**When:** A new supported board is added to the library, or existing driver templates
need to be updated to match the latest demo application code in `applications/`.

**Detailed guide:** `platform/maintenance_guide.md`

### Files involved

| File | Role |
|------|------|
| `platform/js/platform_defs.js` | All platform definitions and code templates |
| `platform/images/` | SVG icons (one per platform) |

---

## 8. Node Config — Well Known Events

**When:** The OpenLCB spec defines new Well Known Events or the library adds new
`#define` constants.

**Detailed guide:** `node_config/maintenance_guide.md`

### Files involved

| File | Role |
|------|------|
| `node_config/js/node_config.js` | `WELL_KNOWN_EVENT_GROUPS` data and UI builder |
| `js/codegen.js` | Generates `_register_app_producers/consumers()` calls |
| `src/openlcb/openlcb_defines.h` | Source of truth for `#define` constant names |

---

## 9. Rebuilding the CodeMirror Bundle

**When:** A CodeMirror package has a bug fix or new feature worth adopting, or a new
CM6 extension is needed (e.g. a new language or extension).

**How to rebuild:**

```bash
cd tools/node_wizard/vendor/codemirror/build_history
npm install          # first time, or after package.json changes
npm run build        # produces ../dist/codemirror.min.js
```

The entry point `entry.mjs` controls which CodeMirror symbols are exported onto
`window.CM`. If a new package is added, import it there and add it to the `window.CM`
assignment before rebuilding.

> **Note:** `npm install` creates a `node_modules/` folder with platform-specific
> esbuild binaries. This folder is NOT committed to the repo.

**Full documentation:** `vendor/codemirror/build_history/BUILD_HISTORY.md`

### Files involved

| File | Role |
|------|------|
| `vendor/codemirror/build_history/package.json` | npm dependencies |
| `vendor/codemirror/build_history/entry.mjs` | Bundle entry point |
| `vendor/codemirror/build_history/build.mjs` | esbuild script |
| `vendor/codemirror/dist/codemirror.min.js` | The built bundle (committed to repo) |
| `vendor/codemirror/cm-adapter.js` | Wrapper that uses `window.CM` |

---

## 10. Rebuilding the xmllint Library

**When:** A bug is found in libxml2's schema validation, or the Emscripten output
format changes and the patches need updating. This should be very rare.

The xmllint library is a custom Emscripten/asm.js compilation of libxml2 2.16.0
with 6 post-processing patches applied to the Emscripten output. It replaced the
older kripken/xml.js library (4.1 MB, inaccurate line numbers) with a 1.5 MB build
that gives accurate line numbers.

**Full documentation:** `vendor/xmllint/build_history/BUILD_HISTORY.md`
— includes the complete story of what was tried, what failed, how it was fixed,
and step-by-step rebuild instructions.

### Files involved

| File | Role |
|------|------|
| `vendor/xmllint/xmllint_patched.js` | The compiled+patched library (1.5 MB) |
| `vendor/xmllint/xmllint_api.js` | Async browser API wrapper |
| `vendor/xmllint/build_history/BUILD_HISTORY.md` | Complete build documentation |
| `vendor/xmllint/build_history/build_asmjs.sh` | Emscripten compile/link script |
| `vendor/xmllint/build_history/build_wrapper.js` | 6-patch post-processing script |

---

## 11. Architecture Overview

The wizard is an **iframe-based single-page app**. The parent shell
(`node_wizard.html` + `js/app.js`) manages the sidebar and state persistence.
Each panel (Node Config, CDI Editor, FDI Editor, Drivers, Callbacks, Platform) is
a self-contained HTML page that loads in an iframe and communicates via `postMessage`.

All state is persisted to `localStorage` under `node_wizard_state` with a 300 ms
debounce. The **Clear Storage** button in the header resets everything.

The full architecture document — including the postMessage protocol, sidebar tile
focus/selection behaviour, descriptor warning badge logic, node type differences
(Train vs Train Controller), and the step-by-step process for adding a new sidebar
section — is in:

[`documentation/maintenance_instructions.md`](documentation/maintenance_instructions.md)
