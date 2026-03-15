# Node Config Panel — Maintenance Guide

This guide covers how to add, modify, or remove Well Known Events in the Node Config panel. All event data is defined in a single data structure — no HTML or code generation changes are needed.

---

## Well Known Events

**Data file:** `js/node_config.js`
**Data structure:** `WELL_KNOWN_EVENT_GROUPS` (array of group objects)

### Data Structure

```javascript
var WELL_KNOWN_EVENT_GROUPS = [
    {
        title: 'Group Name',           // displayed as collapsible section header
        events: [
            {
                id:     'unique-id',       // used in checkbox element IDs — must be unique across all groups
                define: 'EVENT_ID_NAME',   // C #define constant from openlcb_defines.h
                desc:   'Human-readable description',
                range:  true               // optional — if true, uses register_*_range() instead of register_*_eventid()
            }
        ]
    }
];
```

### Adding a New Event to an Existing Group

Find the group in `WELL_KNOWN_EVENT_GROUPS` and append to its `events` array:

```javascript
{
    title: 'Emergency',
    events: [
        // ...existing events...
        { id: 'my-new-event', define: 'EVENT_ID_MY_NEW_EVENT', desc: 'What this event does' }
    ]
}
```

**Requirements:**
- The `id` must be unique across ALL groups (it becomes part of checkbox IDs like `wke-prod-my-new-event`)
- The `define` must match a `#define` in `src/openlcb/openlcb_defines.h`
- Add `range: true` if this is an event range (covers a contiguous block of Event IDs)

### Adding a New Group

Append a new object to the `WELL_KNOWN_EVENT_GROUPS` array:

```javascript
{
    title: 'My New Group',
    events: [
        { id: 'new-evt-1', define: 'EVENT_ID_NEW_1', desc: 'First event' },
        { id: 'new-evt-2', define: 'EVENT_ID_NEW_2', desc: 'Second event', range: true }
    ]
}
```

The group automatically appears as a collapsible section in both the Producer and Consumer columns.

### Removing an Event or Group

Delete the event object or group object from the array. If users had that event checked, it will silently be ignored on restore (no errors).

### What Happens Automatically

When you add/modify the data:
- **UI:** Checkboxes are generated in both Producer and Consumer columns
- **Persistence:** Checked state is saved/restored using the `id` field
- **Code generation:** Selected events appear in `_register_app_producers()` / `_register_app_consumers()` in the generated main file
- **Validation tally:** Event/range counts update in the Well Known Events summary line

### Files Involved

| File | Role |
|------|------|
| `js/node_config.js` | Data definition (`WELL_KNOWN_EVENT_GROUPS`), UI builder, state get/restore |
| `js/codegen.js` | Reads `state.wellKnownEvents` and generates C registration calls |
| `node_config.html` | Container divs (`wke-producer-list`, `wke-consumer-list`) — no per-event HTML |
| `node_config.css` | Styles for `.wke-group`, `.wke-group-title` |
| `src/openlcb/openlcb_defines.h` | Source of truth for `#define` constant names and values |

### Excluded Events (by design)

These are intentionally omitted from the Well Known Events UI:
- **Broadcast Time events** — registered automatically by the protocol handler
- **EVENT_TRAIN_SEARCH_SPACE** — internal protocol use only
- **EVENT_ID_TRAIN_PROXY** — deprecated
