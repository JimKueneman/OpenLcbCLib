# Callbacks Panel — Maintenance Guide

This guide covers how to add new callback groups, new callbacks to existing groups, and how visibility is controlled by node type and add-ons.

---

## Callback Definitions

**Data file:** `js/callback_defs.js`
**Data structure:** `CALLBACK_GROUPS` (object keyed by group ID)

### Data Structure

```javascript
const CALLBACK_GROUPS = {
    'cb-group-key': {
        title: 'Group Title',
        description: 'Short description',
        filePrefix: 'callbacks_group',       // generated filenames: callbacks_group.h, callbacks_group.c
        headerGuard: '__CALLBACKS_GROUP__',   // C header guard
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksGroup',    // C function prefix: CallbacksGroup_on_something()
        diStruct: 'some_config_t',           // struct in openlcb_config_t that holds these function pointers
        functions: [
            {
                name: 'on_something',            // function name suffix
                returnType: 'void',              // C return type
                params: 'openlcb_node_t *node',  // C parameter list
                description: 'Short description shown in checkbox label',
                detail: 'Extended help text shown in detail panel on hover/click',
                required: false,     // true = always included, checkbox disabled
                configField: 'on_something',  // member name in diStruct
                section: 'Section Name',      // optional — groups related callbacks under a subsection header
                advanced: true                // optional — hidden in "Advanced" collapsible
            }
        ]
    }
};
```

### Adding a New Callback to an Existing Group

Find the group in `CALLBACK_GROUPS` and append to its `functions` array:

```javascript
{
    name: 'on_new_thing',
    returnType: 'bool',
    params: 'openlcb_node_t *openlcb_node, uint16_t value',
    description: 'Called when new thing happens',
    detail: 'Extended explanation of when and why this fires, what the parameters mean, and what the return value controls.',
    required: false,
    configField: 'on_new_thing'
}
```

**Requirements:**
- `name` must be unique within the group
- `configField` must match the actual struct member in the library's DI struct
- `returnType` and `params` must match the function pointer typedef in the library

### Adding a New Callback Group

1. Add the group to `CALLBACK_GROUPS` in `callback_defs.js`:

```javascript
'cb-my-protocol': {
    title: 'My Protocol Callbacks',
    description: 'Notifications for my custom protocol',
    filePrefix: 'callbacks_my_protocol',
    headerGuard: '__CALLBACKS_MY_PROTOCOL__',
    includes: ['#include "src/openlcb/openlcb_types.h"'],
    functionPrefix: 'CallbacksMyProtocol',
    diStruct: 'my_protocol_config_t',
    functions: [ /* ...callback definitions... */ ]
}
```

2. Make the group visible by adding it to the appropriate place in `callbacks.html`:

**For always-visible groups** (shown when a node type is selected), add the group key to `_baseGroups`:

```javascript
var _baseGroups = {
    'basic':            ['cb-can', 'cb-olcb', 'cb-events', 'cb-my-protocol'],
    'typical':          ['cb-can', 'cb-olcb', 'cb-events', 'cb-config-mem', 'cb-my-protocol'],
    'train':            ['cb-can', 'cb-olcb', 'cb-events', 'cb-config-mem', 'cb-train', 'cb-my-protocol'],
    'train-controller': ['cb-can', 'cb-olcb', 'cb-events', 'cb-config-mem', 'cb-train', 'cb-my-protocol']
};
```

**For add-on groups** (shown only when an add-on is enabled), add to `_addonGroupMap`:

```javascript
var _addonGroupMap = [
    { key: 'broadcast', test: function (v) { return v && v !== 'none'; }, group: 'cb-bcast-time' },
    { key: 'firmware',  test: function (v) { return !!v; },              group: 'cb-firmware' },
    { key: 'myfeature', test: function (v) { return !!v; },              group: 'cb-my-protocol' }
];
```

### Callback Visibility Options

| Field | Effect |
|-------|--------|
| `required: true` | Checkbox is checked and disabled — always included |
| `advanced: true` | Hidden under an "Advanced" collapsible within the group |
| `section: 'Name'` | Groups related callbacks under a subsection header (used by Train callbacks) |

### Using Sections

Add a `section` field to related functions to organize them under subsection headers:

```javascript
functions: [
    { name: 'on_speed',    section: 'Train Node', ... },
    { name: 'on_function',  section: 'Train Node', ... },
    { name: 'on_throttle',  section: 'Throttle',   ... }
]
```

Functions with the same `section` value are grouped together. The section header is rendered automatically.

### Code Generation

Generated files use the group metadata:

**Header file** (`filePrefix.h`):
- Header guard from `headerGuard`
- Includes from `includes` array
- `extern` declarations for each checked function: `extern returnType functionPrefix_name(params);`

**Source file** (`filePrefix.c`):
- Matching `#include` for the header
- Stub function bodies for each checked function
- `void` functions: empty body
- `bool` functions: `return true;`
- Other return types: `return 0;`

### By-Design Omissions

The following callbacks exist in `openlcb_config_t` but are intentionally NOT exposed in the UI:
- `on_consumer_identified_reserved` — reserved by spec, not useful for applications
- `on_producer_identified_reserved` — reserved by spec, not useful for applications

See the comment in `callback_defs.js` near line 272 for details.

### Files Involved

| File | Role |
|------|------|
| `js/callback_defs.js` | Callback group and function definitions — single source of truth |
| `callbacks.html` | UI rendering logic, base group mapping, add-on group mapping |
| `callbacks.css` | Checkbox and section styles |
| Code generation via `CallbackCodegen.generateH()` / `generateC()` in callbacks.html |
