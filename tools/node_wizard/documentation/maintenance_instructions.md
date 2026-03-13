# Node Wizard — Maintenance Instructions

## Feature Flags

### Platform Selection (`ENABLE_PLATFORM`)

**File:** `js/app.js`, line 10

```javascript
const ENABLE_PLATFORM = false;   // set to true to enable
```

Controls whether the **Platform** sidebar section is visible. When enabled, users can select a target chip/board (ESP32, RP2040, STM32, etc.) and the wizard pre-fills driver code with working implementations instead of empty stubs.

- `false` (default) — Platform section is hidden. All driver code generates as empty TODO stubs.
- `true` — Platform section appears between Descriptors and Drivers in the sidebar.

All platform code remains in the codebase regardless of this flag. Changing it requires no other modifications.

See the **Platform Definitions** section below for details on updating code templates or adding new platforms.

---

## File Structure Overview

```
tools/node_wizard/
    node_wizard.html        Main page — sidebar navigation + iframe workspace
    node_wizard.css         Sidebar and layout styles
    js/
        app.js              Sidebar controller, state persistence, postMessage relay
    node_config/            Node configuration view (generates openlcb_user_config.c/h)
        node_config.html
        js/
            node_config.js  UI logic
            codegen.js      Code generation for user config + main.c
    cdi_editor/             CDI XML editor view
        cdi_view.html
    fdi_editor/             FDI XML editor view
        fdi_editor.html
    drivers/                Driver stub editor view (CAN Drivers, OLCB/LCC Drivers)
        drivers.html
        js/
            driver_defs.js  Driver function definitions per group
            driver_codegen.js  Code generation for driver .c/.h files
    callbacks/              Callback stub editor view
        callbacks.html
        js/
            callback_defs.js   Callback function definitions per group
            callback_codegen.js  Code generation for callback .c/.h files
    platform/               Platform selection view (gated by ENABLE_PLATFORM)
        platform.html
        platform.css
        js/
            platform_defs.js   Platform definitions + code templates
            platform.js        UI logic
```

---

## Architecture

The wizard uses an **iframe-based architecture**. The sidebar (`node_wizard.html` + `js/app.js`) is the parent shell. Each view (node config, CDI editor, drivers, callbacks, platform) loads in an iframe. Communication between parent and child uses `postMessage`.

### State Flow

1. User interacts with a view inside the iframe
2. View sends a `postMessage` to the parent (e.g., `stateChanged`, `driverStateChanged`, `platformStateChanged`)
3. `app.js` stores the state and persists it to `localStorage`
4. When switching views, `app.js` sends init messages to the new iframe to restore state

### Persistence

All wizard state is saved to `localStorage` under the key `node_wizard_state` with a 300ms debounce. This includes: selected node type, current view, CDI/FDI XML, config form state, driver state, callback state, and platform state.

The **Clear Storage** button in the header wipes `localStorage` and reloads the page.

---

## Node Types: Train vs Train Controller

The wizard has two train-related node types:

- **Train** (`'train'`) — Locomotive / decoder node. Receives speed/function commands from throttles. Gets FDI editor, train FDI address spaces (0xFA, 0xF9), and train node callbacks (Notifications + Decisions + Search).
- **Train Controller** (`'train-controller'`) — Throttle. Sends commands to trains, receives replies. Gets throttle callbacks (Replies + Search). No FDI.

Both types enable `OPENLCB_FEATURE_TRAIN`, CDI, Config Memory, and the same hardware drivers.

### Callback Filtering

Train callbacks in `callbacks/js/callback_defs.js` have a `section` field:

| Section | Shown for |
|---------|-----------|
| `Train Node — Notifications` | Train only |
| `Train Node — Decisions` | Train only |
| `Throttle — Replies` | Train Controller only |
| `Train Search` | Both |

The filtering happens in `callbacks/callbacks.html` — `_filterFunctionsForRole()` checks `_currentNodeType` (forwarded via `setGroup` message from `app.js`).

### Event Registration in main.c

The codegen generates `_register_app_producers()` and `_register_app_consumers()` for **all** node types. Contents vary:

- **Train**: `_register_app_producers` has `EVENT_ID_TRAIN`; `_register_app_consumers` has 4 emergency events
- **Train Controller**: `_register_app_producers` has 4 emergency stop/off events
- **Basic / Typical**: both have empty TODO stubs

For Train nodes, `OpenLcbApplicationTrain_setup(node)` is also called in `main()` — it auto-registers the same events internally.

---

## Sidebar Tile Styling

Node type tiles use a **ListView-like focus/selection** pattern:

- **Blue border** (`selected` class) — the node type tile is blue when the Node Type section is focused (user just clicked a node type, or the config view is active)
- **Gray border** (`chosen` class) — the node type tile switches to gray when the user navigates to another section (CDI, Hardware Interface Drivers, Callbacks, etc.)

View tiles (CDI, Hardware Interface Drivers, Callbacks, Platform) use the standard `selected` class (blue) when their view is active.

### Descriptor Warning Badges

CDI and FDI tiles have small dot badges (`.tile-badge`) that indicate status:

| Badge State | Color | CSS Class | Meaning |
|-------------|-------|-----------|---------|
| Hidden | — | (no `visible`) | Descriptor not needed, or loaded with no issues |
| Amber dot | `#f59e0b` | `visible` | Descriptor needed but not loaded, OR loaded with warnings |
| Red dot | `#ef4444` | `visible badge-error` | Descriptor loaded but has validation errors |

Badge logic lives in `_updateDescriptorBadges()` in `app.js`. It is called when:
- A node type is selected
- CDI/FDI XML changes (`xmlChanged` message)
- Validation results arrive (`validationStatus` message)

The CDI editor sends a `validationStatus` postMessage after each auto-validation run (500ms debounce) with `{ type: 'validationStatus', view: 'cdi', errors: N, warnings: N }`. The FDI editor will send the same format when implemented.

---

## Adding a New Sidebar Section

1. Add the HTML in `node_wizard.html` inside the `<nav class="sidebar">` element
2. Add a tile ID and register it in `app.js`:
   - Add a `const` DOM ref (e.g., `const elTileMyView = document.getElementById('tile-my-view');`)
   - Add the key to `VIEW_TILES` and `VIEW_URLS`
3. Create the iframe view (HTML + JS) in a new subdirectory
4. Handle `postMessage` communication in `app.js` (`_buildInitMessages` and the message listener)
5. Add enable/disable logic in `_updateTileStates()` if the section is gated by node type or feature flags

---

## Adding a New Driver or Callback Group

Driver and callback groups are defined in their respective `*_defs.js` files:

- `drivers/js/driver_defs.js` — `DRIVER_GROUPS` object
- `callbacks/js/callback_defs.js` — `CALLBACK_GROUPS` object

Each group defines: file prefix, function prefix, header guard, includes, and the list of functions with their signatures and descriptions.

To add a new group:

1. Add the group definition to the appropriate `*_defs.js` file
2. Add a sidebar tile in `node_wizard.html`
3. Register the tile in `app.js` (`VIEW_TILES`, `VIEW_URLS`, DOM ref)
4. Add enable/disable gating in `_updateTileStates()` if needed

---

## Platform Definitions

All platform definitions live in a single file:

```
tools/node_wizard/platform/js/platform_defs.js
```

No other files need to change when updating code templates or adding a new platform.

### Platform Definition Structure

`platform_defs.js` exports a `PLATFORM_DEFS` object. Each key is a platform ID (kebab-case string):

```javascript
const PLATFORM_DEFS = {
    'none':           { ... },   // Default — empty stubs
    'esp32-twai':     { ... },   // ESP32 with built-in TWAI CAN
    'esp32-wifi':     { ... },   // ESP32 with WiFi GridConnect
    'rp2040-mcp2517': { ... },   // Pico with MCP2517FD via SPI
    'stm32f4':        { ... },   // STM32F4 with HAL CAN
    'ti-mspm0':       { ... },   // TI MSPM0 with MCAN
};
```

### Platform Entry Fields

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Display name shown on the selection card |
| `description` | string | Short description shown below the name |
| `framework` | string | Framework label (e.g. "Arduino / PlatformIO") |
| `libraries` | string[] | Required library names (shown as a warning) |
| `notes` | string[] | Additional notes/instructions (shown as info) |
| `parameters` | object[] | User-configurable values (pins, addresses, etc.) |
| `canDrivers` | object | CAN driver code templates |
| `olcbDrivers` | object | OLCB/LCC driver code templates |

### Parameters

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Template placeholder name (used as `{{id}}` in templates) |
| `label` | string | Label shown in the UI |
| `defaultValue` | string | Default value |
| `type` | string | `'number'`, `'text'`, or `'select'` |
| `options` | object[] | For `type: 'select'` only: `[{ value: '20', label: '20 MHz' }, ...]` |

### Driver Templates (canDrivers / olcbDrivers)

| Field | Type | Description |
|-------|------|-------------|
| `extraIncludes` | string[] | Additional `#include` lines for the `.c` file |
| `globals` | string | Module-level code (static vars, ISRs, helpers). Placed after includes, before functions. |
| `templates` | object | Map of function name to function body code (indented 4 spaces). |

- **`{{paramId}}`** placeholders are replaced with user-configured values at generation time.
- **Missing templates** generate TODO stubs automatically.

---

## How to Update Existing Platform Code

1. Open `platform/js/platform_defs.js`
2. Find the platform key (e.g. `'esp32-twai'`)
3. Edit the relevant template string array

Templates use array-of-strings format for clean diffs:

```javascript
setup: [
    '    twai_general_config_t g_config = ...;',
    '    twai_driver_install(&g_config, ...);'
].join('\n'),
```

**Testing**: Set `ENABLE_PLATFORM = true` in `js/app.js`, open `node_wizard.html`, select the platform, and check the code preview. No build step needed.

---

## How to Add a New Platform

### Step 1: Add the definition

Add a new key to `PLATFORM_DEFS` in `platform_defs.js`. Copy an existing platform as a starting point. Every platform needs at minimum: `name`, `description`, `framework`, `canDrivers`, and `olcbDrivers`.

### Step 2: That's it

The UI automatically generates a selection card and parameter form from the definition. No HTML, CSS, or other JS changes needed.

### Step 3: Update sidebar name map (optional)

Add a short name to the `names` map in `js/app.js` inside `_updatePlatformTile()`:

```javascript
var names = {
    'esp32-twai': 'ESP32 + TWAI',
    ...
    'my-new-board': 'My New Board'
};
```

Cosmetic only — falls back to showing the platform key.

---

## Code Template Tips

- **Indentation**: 4-space indent in template strings (code goes inside function bodies)
- **Globals**: static vars, ISRs, and helpers go here (module scope, after `#include` lines)
- **`setup` is special**: its code goes inside `_initialize()`. All others map to their named functions.
- **Array format** for readability: `['    line1', '    line2'].join('\n')`

---

## Where Template Code Comes From

| Platform | Demo Source Files |
|----------|-------------------|
| ESP32 + TWAI | `src/applications/arduino/esp32/BasicNode/application_drivers/` |
| ESP32 + WiFi | `src/applications/platformio/esp32/BasicNode_WiFi/application_drivers/` |
| RP2040 + MCP2517 | `src/applications/arduino/rpi_pico/BasicNode/application_drivers/` |
| STM32F4 + CAN | `src/applications/stm32_cubeide/stm32f407_discovery/BasicNode/Core/Src/` |
| TI MSPM0 + MCAN | `src/applications/ti_thiea/mspm03507_launchpad/BasicNode/` |

When updating templates, compare against the latest demo code to ensure they stay in sync.
