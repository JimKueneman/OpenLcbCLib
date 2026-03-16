# Node Wizard — Maintenance Guide

Central reference for everything in the Node Wizard that needs to be updated over
time as the OpenLCB library evolves. Each section explains *why* the thing exists,
*what triggers an update*, and *where to go* — with links to deeper guides where
they exist.

---

## Table of Contents

1. [CDI / FDI XML Schemas (autocomplete)](#1-cdi--fdi-xml-schemas-autocomplete) — OpenLCB releases a new CDI or FDI XSD version.
2. [CDI / FDI Validators](#2-cdi--fdi-validators) — The spec adds or changes element rules, required attributes, or valid enum values.
3. [CDI Test Cases](#3-cdi-test-cases) — A new validator rule is added, a bug is fixed, or an edge case is discovered.
4. [Platform Drivers](#4-platform-drivers) — A new target board is added or existing driver templates drift from the demo source.
5. [Hardware Interface Drivers](#5-hardware-interface-drivers) — The library adds, removes, or changes a CAN or OLCB driver function signature.
6. [Callbacks](#6-callbacks) — The library adds, removes, or changes a callback function in `openlcb_config_t`.
7. [Node Config — Well Known Events](#7-node-config--well-known-events) — The OpenLCB spec defines new Well Known Events or the library adds new `#define` constants.
8. [CodeMirror Bundle](#8-codemirror-bundle) — A CodeMirror package has a needed bug fix or a new extension is required.
9. [Feature Flags](#9-feature-flags) — A feature is being enabled or disabled for release.
10. [Architecture Overview](#10-architecture-overview) — Reference only; no data to update.

---

## 1. CDI / FDI XML Schemas (autocomplete)

**Why it exists:** The CDI and FDI editors use CodeMirror 6's XML language with a
schema object that describes valid elements and attributes. This powers context-aware
autocomplete — pressing `<` or Ctrl+Space inside any element only offers children
and attributes that are legal at that position according to the OpenLCB spec.

**When to update:** Whenever OpenLCB releases a new version of the CDI or FDI XSD
(the `.xsd` files that define the official schema). The current XSD files live in the
OpenLcb Documents folder alongside this project.

**How to update:** Two Python 3 scripts automate the process. They read the XSD,
derive element/attribute/enum rules from it, and patch the schema block directly into
`cm-adapter.js` between its marker comments. No manual editing of `cm-adapter.js` is
needed.

```bash
# Update CDI schema
python3 tools/node_wizard/vendor/codemirror/generate_cdi_schema.py \
    --xsd "/path/to/schema/cdi/1/4/cdi.xsd" \
    --patch

# Update FDI schema
python3 tools/node_wizard/vendor/codemirror/generate_fdi_schema.py \
    --xsd "/path/to/schema/fdi/1/1/fdi.xsd" \
    --patch
```

The scripts replace the blocks between these marker comments in `cm-adapter.js`:

```
/* <<CDI_SCHEMA_START>> */ ... /* <<CDI_SCHEMA_END>> */
/* <<FDI_SCHEMA_START>> */ ... /* <<FDI_SCHEMA_END>> */
```

**Files involved:**

| File | Role |
|------|------|
| `vendor/codemirror/generate_cdi_schema.py` | Reads CDI XSD, patches `cm-adapter.js` |
| `vendor/codemirror/generate_fdi_schema.py` | Reads FDI XSD, patches `cm-adapter.js` |
| `vendor/codemirror/cm-adapter.js` | Contains `CDI_SCHEMA` and `FDI_SCHEMA` blocks |

---

## 2. CDI / FDI Validators

**Why they exist:** Beyond what the schema can enforce in autocomplete, the validators
catch structural and semantic problems — missing required elements, invalid attribute
values, duplicate singletons, conflicting numbers — and display them as inline
squiggles in the editor (via CodeMirror lint) and as a list in the footer panel.

**When to update:** When the CDI or FDI spec adds new required elements or attributes,
changes valid enum values (e.g. a new `kind` for `<function>`), or changes cardinality
rules (e.g. a new singleton element). Also update when the library grows new
conventions that should be enforced.

**Files involved:**

| File | Role |
|------|------|
| `cdi_editor/js/cdi_validator.js` | CDI structural and semantic checks |
| `fdi_editor/js/fdi_validator.js` | FDI structural and semantic checks |

Each validator is self-contained and well-commented with three layers:

- **Layer 1** — XML well-formedness (handled by DOMParser)
- **Layer 2** — Structural rules (required attributes, singleton counts, known tags)
- **Layer 3** — Semantic hints (duplicate numbers, missing names, out-of-range values)

Adding a new check is a matter of appending to the appropriate layer inside the
`validate()` function in the relevant file.

---

## 3. CDI Test Cases

**Why they exist:** The test cases are a suite of XML files that each embed their
own expected validator/renderer results. They protect against regressions when the
validator or renderer is changed.

**When to update:** Add a new test case whenever a new validator rule is added, a
bug is fixed, or an edge case is discovered. The test runner works entirely offline —
no server needed.

**How to run:** Open `test/test_runner.html` in a browser and drag all `.xml` files
from `test/cdi/` onto the drop zone.

**How to add a test:** Copy an existing XML file from `test/cdi/`, modify it for the
scenario being tested, and embed a `<!-- TEST_CHECKS -->` block describing the
expected outcomes. See [`test/TEST_CASES.md`](test/TEST_CASES.md) for the format.

**Files involved:**

| Location | Role |
|----------|------|
| `test/cdi/` | CDI XML test files, each self-describing expected results |
| `test/test_runner.html` | In-browser test runner |
| `test/TEST_CASES.md` | Format reference and test case inventory |

---

## 4. Platform Drivers

**Why it exists:** The Platform panel lets users select a target board (ESP32, RP2040,
STM32, etc.) and automatically fills in working CAN and OLCB driver code. The
definitions are data-driven so adding a new platform requires no HTML or UI changes.

**When to update:** When a new supported board is added to the library, or when
existing driver templates need to be updated to match the latest demo application
code in `src/applications/`.

**Detailed guide:** [`platform/maintenance_guide.md`](platform/maintenance_guide.md)

**Files involved:**

| File | Role |
|------|------|
| `platform/js/platform_defs.js` | All platform definitions and code templates |
| `platform/images/` | SVG icons (one per platform) |

---

## 5. Hardware Interface Drivers

**Why it exists:** The Drivers panel generates the CAN driver and OLCB/LCC driver
stub files. Driver groups and their function signatures are defined in a data file
so the UI and code generation stay in sync automatically.

**When to update:** When the library adds a new driver function to a group, when a
function signature changes, or when an entirely new driver group is needed.

The structure mirrors the Callbacks system — see the callbacks guide below and the
comments in `driver_defs.js` for the data format. There is no separate maintenance
guide for drivers; the data structure is equivalent to the callbacks one.

**Files involved:**

| File | Role |
|------|------|
| `drivers/js/driver_defs.js` | Driver group and function definitions |
| `drivers/js/driver_codegen.js` | Generates `.c` / `.h` stub files |

---

## 6. Callbacks

**Why it exists:** The Callbacks panel generates stub `.c` / `.h` files for every
OpenLCB callback the application wants to handle. Which callbacks are shown depends
on node type (train, throttle, basic, typical) and which add-ons are enabled.

**When to update:** When the library adds new callback functions to `openlcb_config_t`,
when a function signature changes, or when a new logical group of callbacks is needed.

**Detailed guide:** [`callbacks/maintenance_guide.md`](callbacks/maintenance_guide.md)

**Files involved:**

| File | Role |
|------|------|
| `callbacks/js/callback_defs.js` | Callback group and function definitions |
| `callbacks/js/callback_codegen.js` | Generates `.c` / `.h` stub files |
| `callbacks/callbacks.html` | Node-type visibility rules (`_baseGroups`, `_addonGroupMap`) |

---

## 7. Node Config — Well Known Events

**Why it exists:** The Node Config panel has a section for pre-defined OpenLCB Well
Known Events (emergency stop, power off, etc.) that the user can opt into as
producers or consumers. These appear as checkboxes and the selections drive code
generation of the registration calls in `main.c`.

**When to update:** When the OpenLCB spec defines new Well Known Events, or when the
library adds new `#define` constants for them in `openlcb_defines.h`.

**Detailed guide:** [`node_config/maintenance_guide.md`](node_config/maintenance_guide.md)

**Files involved:**

| File | Role |
|------|------|
| `node_config/js/node_config.js` | `WELL_KNOWN_EVENT_GROUPS` data and UI builder |
| `node_config/js/codegen.js` | Generates `_register_app_producers/consumers()` calls |
| `src/openlcb/openlcb_defines.h` | Source of truth for `#define` constant names |

---

## 8. CodeMirror Bundle

**Why it exists:** The CDI and FDI editors use CodeMirror 6 for syntax highlighting,
autocomplete, and inline validation squiggles. Because the wizard runs as a local
HTML file (no server, no module bundler at runtime), all CodeMirror packages are
pre-bundled into a single IIFE file at `vendor/codemirror/dist/codemirror.min.js`.

**When to update:** When a CodeMirror package has a bug fix or new feature worth
adopting, or when a new CodeMirror package is needed (e.g. a new language or
extension).

**How to rebuild:**

```bash
cd tools/node_wizard/vendor/codemirror
npm install          # only needed first time, or after package.json changes
node build.mjs       # produces dist/codemirror.min.js
```

The entry point `entry.mjs` controls which CodeMirror symbols are exported onto
`window.CM`. If a new package is added, import it there and add it to the `window.CM`
assignment before rebuilding.

> **Note:** `npm install` must be run on the same OS/architecture as the machine that
> will run the build. The `node_modules` folder contains native binaries for esbuild.

**Files involved:**

| File | Role |
|------|------|
| `vendor/codemirror/package.json` | npm dependencies (CodeMirror packages + esbuild) |
| `vendor/codemirror/entry.mjs` | Bundle entry point — controls what goes into `window.CM` |
| `vendor/codemirror/build.mjs` | esbuild script that produces the IIFE bundle |
| `vendor/codemirror/dist/codemirror.min.js` | The built bundle (committed to the repo) |
| `vendor/codemirror/cm-adapter.js` | Wrapper that uses `window.CM` to create editor instances |

---

## 9. Feature Flags

**Why they exist:** Some features are complete but not enabled by default while they
are being evaluated. Flags let the feature code stay in the tree without appearing
in the UI.

**Current flags** — all in `js/app.js`:

| Flag | Default | Effect |
|------|---------|--------|
| `ENABLE_PLATFORM` | `false` | Shows/hides the Platform selection panel in the sidebar |

See [`documentation/maintenance_instructions.md`](documentation/maintenance_instructions.md)
for the full architecture description, postMessage state flow, sidebar tile system,
descriptor badge logic, and how to add a new sidebar section.

---

## 10. Architecture Overview

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
