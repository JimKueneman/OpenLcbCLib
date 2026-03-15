# Platform Drivers Panel — Maintenance Guide

This guide covers how to add new target platforms, modify existing ones, or add parameters. All platform data is defined in a single data structure — no HTML changes are needed.

---

## Platform Definitions

**Data file:** `js/platform_defs.js`
**Data structure:** `PLATFORM_DEFS` (object keyed by platform ID)

### Data Structure

```javascript
const PLATFORM_DEFS = {
    'platform-key': {
        name: 'Display Name',
        description: 'One-line description',
        image: 'images/platform-icon.svg',
        isArduino: true,           // true = .ino main file, .cpp drivers; false = .c files
        framework: 'Arduino / PlatformIO',
        libraries: ['LibraryName'],  // Arduino libraries (displayed as info)
        notes: ['Setup note 1', 'Setup note 2'],

        parameters: [
            {
                id: 'param_name',        // used as {{param_name}} in templates
                label: 'Display Label',
                defaultValue: '21',
                type: 'number'           // 'number', 'text', or 'select'
            },
            {
                id: 'osc_freq',
                label: 'Oscillator Frequency',
                defaultValue: '40',
                type: 'select',
                options: [
                    { value: '20', label: '20 MHz' },
                    { value: '40', label: '40 MHz' }
                ]
            }
        ],

        canDrivers: {
            extraIncludes: ['#include "driver/twai.h"'],
            globals: 'static bool _connected = false;',
            templates: {
                setup: '// CAN init code using {{param_name}}',
                transmit_raw_can_frame: '// transmit code',
                is_can_tx_buffer_clear: 'return true;',
                is_connected: 'return _connected;'
            }
        },

        olcbDrivers: {
            extraIncludes: ['#include "esp32-hal-timer.h"'],
            globals: 'static hw_timer_t *_timer = NULL;',
            templates: {
                setup: '// Timer init code',
                lock_shared_resources: '// disable interrupts',
                unlock_shared_resources: '// enable interrupts',
                reboot: '// reboot code'
                // config_mem_read / config_mem_write — optional, stubs generated if missing
            }
        }
    }
};
```

### Adding a New Platform

Add a new key to `PLATFORM_DEFS`:

```javascript
'my-new-board': {
    name: 'My Board + CAN Controller',
    description: 'Custom board with MCP2515 over SPI',
    image: 'images/my-board.svg',
    isArduino: false,
    framework: 'GCC',
    libraries: [],
    notes: ['Connect CAN_H/CAN_L to transceiver'],
    parameters: [ /* ... */ ],
    canDrivers: { /* ... */ },
    olcbDrivers: { /* ... */ }
}
```

The platform automatically appears as a selectable card in the UI.

### Adding a Parameter to an Existing Platform

Append to the `parameters` array:

```javascript
parameters: [
    // ...existing params...
    { id: 'spi_speed', label: 'SPI Clock (MHz)', defaultValue: '10', type: 'number' }
]
```

Use `{{spi_speed}}` in any template string and it will be substituted with the user's value.

### Template Substitution

All `{{paramId}}` placeholders in `canDrivers.templates` and `olcbDrivers.templates` are replaced at code generation time with the corresponding parameter values from the UI.

If a template key is missing from the object, the code generator produces a `// TODO` stub for that function.

### Adding a Platform Icon

Place an SVG file in the `images/` directory and reference it in the `image` field. Keep icons simple and small (roughly 48x48 viewbox).

### Files Involved

| File | Role |
|------|------|
| `js/platform_defs.js` | Platform data definitions — single source of truth |
| `js/platform.js` | Card rendering, config form rendering, parameter collection |
| `platform.html` | Container elements — no per-platform HTML |
| `platform.css` | Card and form styles |
| Code generation uses templates from `canDrivers` and `olcbDrivers` |
