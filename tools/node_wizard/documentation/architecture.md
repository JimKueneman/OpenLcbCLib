# Node Wizard — Architecture & Maintenance Instructions

This document covers the internal architecture of the Node Wizard shell — how the
views are wired together, how state flows between them, how persistence works, and
what to touch when adding new sections or changing behaviour. For data-level
maintenance (adding platforms, callbacks, well-known events, etc.) see the
specialised guides linked from `../MAINTENANCE.md`.

---

## File Structure Overview

```
tools/node_wizard/
    MAINTENANCE.md              Central maintenance index (start here)
    node_wizard.html            Shell page — header, sidebar, single iframe workspace
    node_wizard.css             Shell styles (sidebar, tiles, header)
    js/
        app.js                  Sidebar controller, state management, postMessage relay
        codegen.js              Code generation for user config, CAN config, and main.c
        zip_export.js           Builds and downloads the project ZIP via JSZip
    node_config/                Node configuration view (generates openlcb_user_config.c/h + main.c)
        node_config.html
        node_config.css
        js/
            node_config.js      UI logic, form state, Well Known Events
            wke_defs.js         Well Known Event group definitions
    cdi_editor/                 CDI XML editor view
        cdi_view.html
        js/
            cdi_validator.js    3-layer CDI XML validator
            cdi_renderer.js     Builds Flat/Tree/Defines/Struct/Array/Preview views
    fdi_editor/                 FDI XML editor view
        fdi_editor.html
        js/
            fdi_validator.js    3-layer FDI XML validator
            fdi_function_grid.js  Visual function grid + popover editor
    drivers/                    Platform drivers view (CAN + OLCB/LCC driver stubs)
        drivers.html
        js/
            driver_defs.js      Driver group and function definitions
            driver_codegen.js   Generates driver .c/.h stub files
    callbacks/                  Callbacks view (application hook stubs)
        callbacks.html
        callbacks.css
        js/
            callback_defs.js    Callback group and function definitions
            callback_codegen.js Generates callback .c/.h stub files
    platform/                   Target Platform selection view
        platform.html
        platform.css
        js/
            platform_defs.js    Platform definitions and code templates
            platform.js         Card/form rendering, parameter collection
    file_preview/               Generated Files view — file tree + CodeMirror preview
        file_preview.html
        file_preview.css
        js/
            file_preview.js     File tree builder, CodeMirror read-only viewer
    documentation/              This file and supporting docs
        maintenance_instructions.md
        wizard_advanced_validation_rules/
        wizard_display_matrix/
    test/                       CDI validator/renderer test suite
        test_runner.html
        cdi/                    25 self-describing XML test cases
        TEST_CASES.md
    vendor/
        codemirror/             CodeMirror 6 IIFE bundle + adapter
        jszip/                  JSZip library (ZIP file generation)
```

---

## Architecture

The wizard is an **iframe-based single-page application**. The outer shell
(`node_wizard.html` + `js/app.js`) renders the header and sidebar. The entire
right-hand workspace is a single `<iframe id="workspace-iframe">` whose `src`
changes as the user navigates. Each view is a fully self-contained HTML page.

Communication between the shell and active view uses `window.postMessage`. The
shell is always the parent; views are always children. Neither side directly reads
the other's DOM.

### Views and their URLs

| Sidebar section | `currentView` key | URL loaded in iframe |
|----------------|-------------------|----------------------|
| Node Type / Config | `'config'` | `node_config/node_config.html` |
| CDI | `'cdi'` | `cdi_editor/cdi_view.html` |
| FDI | `'fdi'` | `fdi_editor/fdi_editor.html` |
| Platform Drivers | `'platform-drivers'` | `platform/platform.html` |
| Callbacks | `'callbacks'` | `callbacks/callbacks.html` |
| Generated Files | `'file-preview'` | `file_preview/file_preview.html` |

---

## State Management

All wizard state lives in variables at the top of `app.js`. The full set:

| Variable | Type | Description |
|----------|------|-------------|
| `selectedNodeType` | string \| null | `'basic'`, `'typical'`, `'train'`, `'train-controller'`, `'custom'` |
| `currentView` | string \| null | Active view key (see table above) |
| `cdiUserXml` | string \| null | Full CDI XML text from the CDI editor |
| `fdiUserXml` | string \| null | Full FDI XML text from the FDI editor |
| `cdiFilename` | string \| null | Filename shown on the CDI sidebar tile |
| `fdiFilename` | string \| null | Filename shown on the FDI sidebar tile |
| `configMemHighest` | number | Highest config memory address (from CDI map) — default `0x200` |
| `configFormState` | object \| null | Full form state snapshot from `node_config` |
| `driverState` | object | Per-group driver checkbox state, keyed by group ID |
| `callbackState` | object | Per-group callback checkbox state, keyed by group ID |
| `platformState` | object \| null | `{ platform, params, isArduino, framework, libraries, notes }` |
| `cdiValidation` | object \| null | `{ errors: N, warnings: N }` from CDI editor lint |
| `fdiValidation` | object \| null | `{ errors: N, warnings: N }` from FDI editor lint |
| `filePreviewSelection` | string \| null | Last selected file path in the file preview |

### Persistence

All state (except `cdiValidation`/`fdiValidation`, which are always recomputed)
is saved to `localStorage` under the key `node_wizard_state` with a 300 ms
debounce. On page load, `_restoreState()` reads it back and the last active view
is reloaded automatically.

The **Clear Storage** button in the header wipes `localStorage` and reloads the page.

### Save / Load Project

The header also has **Save Project** and **Load Project** buttons that work
independently of `localStorage`. Saving serialises the full `_buildWizardState()`
object (which is identical to what gets persisted) to a `.json` file and downloads
it. Loading reads that JSON back, applies it exactly as if it came from
`_restoreState()`, and navigates to the saved view. The project format is identified
by `_format: 'node_wizard_project'` and `_version: 1`.

---

## postMessage Protocol

Every view signals it is ready with a `{ type: 'ready' }` message before the shell
sends any init messages. The shell queues pending messages in `_pendingMsgs` and
flushes them once `ready` arrives.

### Shell → View (init messages)

| Message type | Sent to | Payload | Purpose |
|---|---|---|---|
| `setNodeType` | config | `{ nodeType }` | Set selected node type |
| `setCdiBytes` | config | `{ xml }` | Forward CDI XML to config code-gen |
| `setFdiBytes` | config | `{ xml }` | Forward FDI XML to config code-gen |
| `restoreFormState` | config | `{ formState }` | Restore full config form |
| `setDriverCallbackState` | config | `{ driverState, callbackState, platformState }` | Wire driver/callback/platform into main.c |
| `setArduinoMode` | config | `{ arduino: bool }` | Switch preview between `main.c` and `main.ino` |
| `setConfigMemSize` | cdi | `{ value }` | Provide config memory size for map rendering |
| `loadXml` | cdi, fdi | `{ xml, filename }` | Restore XML content in editor |
| `restoreState` | platform-drivers | `{ state }` | Restore platform selection |
| `restoreState` | callbacks | `{ state, nodeType, addons }` | Restore callback selections + node type context |
| `setWizardState` | file-preview | `{ state }` | Full wizard state for file tree generation |

### View → Shell (event messages)

| Message type | From | Payload | Shell action |
|---|---|---|---|
| `ready` | any | — | Flush `_pendingMsgs` |
| `stateChanged` | config | `{ state }` | Store `configFormState`, update `configMemHighest`, re-gate tiles |
| `xmlChanged` | cdi, fdi | `{ xml, filename }` | Store `cdiUserXml`/`fdiUserXml`, update badges |
| `updateConfigMemSize` | cdi | `{ value }` | Update `configMemHighest`, forward to config if loaded |
| `driverStateChanged` | platform-drivers | `{ group, state }` | Store into `driverState[group]` |
| `callbackStateChanged` | callbacks (legacy) | `{ group, state }` | Store into `callbackState[group]` |
| `allCallbackStateChanged` | callbacks | `{ state }` | Replace entire `callbackState` |
| `nodeTypeChanged` | config | `{ nodeType }` | Call `selectNodeType()`, update sidebar |
| `platformStateChanged` | platform-drivers | `{ state }` | Store `platformState`, update tile description |
| `filePreviewSelection` | file-preview | `{ selectedFile }` | Store `filePreviewSelection` |
| `requestDownload` | file-preview | — | Call `ZipExport.generateZip()` |
| `switchView` | any | `{ view }` | Call `selectView(view)` |
| `validationStatus` | cdi, fdi | `{ view, errors, warnings }` | Update `cdiValidation`/`fdiValidation`, refresh badges |

---

## Tile Enable/Disable Rules

`_updateTileStates(nodeType)` is called whenever the node type changes or the config
form state changes. Current rules:

| Tile | Enabled when |
|------|-------------|
| CDI | Node type is anything except `'basic'` |
| FDI | Node type is `'train'` only |
| Platform Drivers | Any node type is selected |
| Callbacks | Any node type is selected |
| Generated Files | Any node type is selected |

If the user is on a view that becomes disabled (e.g., they switch from Train to Basic
while on the FDI editor), `app.js` automatically navigates back to the config view.

---

## Node Types

| Key | Display Name | CDI | FDI | Notes |
|-----|-------------|-----|-----|-------|
| `'basic'` | Basic | — | — | Events only, no config memory |
| `'typical'` | Typical | ✓ | — | Events + CDI + Config Memory |
| `'train'` | Train | ✓ | ✓ | Locomotive / decoder, gets FDI address spaces (0xFA, 0xF9) and train callbacks |
| `'train-controller'` | Train Controller | ✓ | — | Throttle, gets throttle callbacks; no FDI |
| `'custom'` | Custom | — | — | Advanced — not yet implemented |

### Train vs Train Controller Callback Filtering

Train callbacks in `callback_defs.js` use a `section` field to separate functions
by role:

| Section | Shown for |
|---------|-----------|
| `Train Node — Notifications` | Train only |
| `Train Node — Decisions` | Train only |
| `Throttle — Replies` | Train Controller only |
| `Train Search` | Both Train and Train Controller |

Filtering happens in `callbacks/callbacks.html` via `_filterFunctionsForRole()`,
which checks `_currentNodeType` (received via the `restoreState` message from
`app.js`).

---

## Descriptor Warning Badges

Both the CDI and FDI sidebar tiles have a small dot badge (`.tile-badge`) that
indicates descriptor status. Badge logic is in `_computeCdiBadge()` and
`_computeFdiBadge()` in `app.js`:

| State | Badge colour | Meaning |
|-------|-------------|---------|
| Not visible | — | Descriptor not needed, or loaded with no issues |
| Amber | `#f59e0b` | Descriptor needed but not yet loaded, OR loaded with warnings |
| Red | `#ef4444` | Descriptor loaded but has validation errors |

`_updateDescriptorBadges()` is called when:
- A node type is selected
- CDI or FDI XML changes (`xmlChanged` message)
- Validation results arrive (`validationStatus` message)

The CDI and FDI editors both send `{ type: 'validationStatus', view: 'cdi'|'fdi', errors: N, warnings: N }` after each auto-validation run (500 ms debounce after the last keystroke).

---

## Sidebar Tile Selection Style

All tiles use a single `selected` CSS class (blue highlight) when their view is
active. Tiles with no view loaded use no highlight class. The Node Type tile is
always selectable (it loads the config view); all other tiles are disabled until a
node type is chosen.

---

## ZIP File Layout

`js/zip_export.js` uses JSZip to build and download a complete project archive.
The layout differs by platform:

**Non-Arduino (flat layout):**
```
<project>/
    main.c
    openlcb_user_config.h / .c
    application_callbacks/     callbacks_*.h / .c
    application_drivers/       openlcb_can_drivers.h / .c
    xml_files/                 cdi.xml, fdi.xml (if applicable)
    openlcb_c_lib/             openlcb/, drivers/canbus/, utilities/
    GETTING_STARTED.txt
    <type>_project.json
```

**Arduino (src/ wrapper):**
```
<project>/
    main.ino
    openlcb_user_config.h / .c
    src/
        application_callbacks/
        application_drivers/
        xml_files/
        openlcb_c_lib/
    GETTING_STARTED.txt
    <type>_project.json
```

The project JSON file embedded in the ZIP is the same `_buildWizardState()` object
used for Save Project, so the archive is self-describing and can be loaded back into
the wizard.

---

## Adding a New Sidebar Section

1. **Add the tile HTML** in `node_wizard.html` inside `<nav class="sidebar">`:
   ```html
   <div class="sidebar-section">
       <span class="sidebar-label">My Section</span>
       <div class="sidebar-tile disabled" id="tile-my-section" onclick="selectView('my-section')">
           <span class="tile-icon">🔧</span>
           <div class="tile-content">
               <span class="tile-name">My Section</span>
               <span class="tile-desc">Short description</span>
           </div>
       </div>
   </div>
   ```

2. **Register it in `app.js`** — three places:
   - Add a DOM ref constant: `const elTileMySection = document.getElementById('tile-my-section');`
   - Add to `VIEW_TILES`: `'my-section': elTileMySection`
   - Add to `VIEW_URLS`: `'my-section': 'my_section/my_section.html'`

3. **Add enable/disable logic** in `_updateTileStates()` if the section should be
   gated by node type or other state.

4. **Add init messages** in `_buildInitMessages()` under the `else if (view === 'my-section')` branch.

5. **Handle incoming messages** in the `window.addEventListener('message', ...)` switch in `app.js`.

6. **Create the view** as a self-contained HTML file in a new subdirectory. The view must send `{ type: 'ready' }` once its script has loaded, and respond to whatever init messages you defined in step 4.

7. **Update `js/zip_export.js`** if the new section produces files that should be included in the downloaded ZIP.

---

## Adding a New Driver or Callback Group

Driver and callback groups are entirely data-driven. The data files are the only
files that need to change:

- Drivers: `drivers/js/driver_defs.js` — `DRIVER_GROUPS` object
- Callbacks: `callbacks/js/callback_defs.js` — `CALLBACK_GROUPS` object

See [`../callbacks/maintenance_guide.md`](../callbacks/maintenance_guide.md) for the
full data structure format. The driver format is identical.

No HTML, CSS, or sidebar registration changes are needed — the panels render
group cards dynamically from the data.
