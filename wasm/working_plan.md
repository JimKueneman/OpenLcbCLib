# WASM Build — Working Plan

## Goal

Make OpenLcbCLib the single source of truth for OpenLCB protocol logic, and produce a WebAssembly build that OpenLcbJSLib can consume as a thin wrapper. End state: bug fixes happen once (in C), and both libraries get them.

## Context

- Sibling repo `OpenLcbJSLib` currently has a parallel JS reimplementation of the protocol. It will be gutted down to a hand-written wrapper around the WASM artifact built here.
- Decision made in a prior planning conversation (rooted in OpenLcbJSLib). This file exists so execution can start cold in OpenLcbCLib with full context.
- Audit of OpenLcbCLib came back green for Emscripten compatibility — no hard blockers.

## Why OpenLcbCLib is well-suited

- HAL is already behind function-pointer interface structs wired in `openlcb_config.c` (CAN TX, timer tick, lock/unlock, config memory I/O). These become JS callbacks in the browser.
- No `malloc` in the core — static pools sized via `openlcb_user_config.h`.
- No threading, no atomics, no platform-specific headers, no inline asm.
- Bootloader folder is separable and will be excluded from the WASM build.

## Scope

**In scope:**
- New top-level `wasm/` folder (this directory) containing the Emscripten build.
- Minimal C-side changes: gate debug `printf` behind a flag, add any needed binding/trampoline code.
- Proof-of-concept build → callback round-trip → deliverable artifact.

**Out of scope (belongs in OpenLcbJSLib repo, separate conversation):**
- The JS wrapper.
- WebSocket transport shim.
- Vendoring the `.wasm` into OpenLcbJSLib.
- Removing `OpenLcbJSLib/src/`.

## Proposed layout

```
OpenLcbCLib/wasm/
├── working_plan.md         (this file)
├── CMakeLists.txt          Emscripten build config
├── exports.json            EXPORTED_FUNCTIONS list
├── bindings.c              glue C — callback trampolines, JS→C marshalling helpers
├── build.sh                one-command build
└── dist/                   build output: openlcb-core.wasm + .js glue (gitignored)
```

Confirm with user before creating any new files/folders — per CLAUDE.md, no folders created without explicit permission. This `wasm/` directory already exists.

## Milestones

### 1. Toolchain proof-of-concept
- Install/verify Emscripten (`emcc --version`).
- Minimal `CMakeLists.txt` or Makefile that compiles a tiny subset of OpenLcbCLib with `OPENLCB_COMPILE_EVENTS` only (plus whatever it depends on).
- Export `init`, `run`, `100ms_tick`. No callbacks wired yet.
- Confirm `.wasm` + `.js` glue is produced and loads in Node.
- **Exit criterion:** JS can call the three exported functions without crashing.

### 2. Printf cleanup
- Audit the 16 `.c` files that include `<stdio.h>` for debug `printf` usage. User confirmed these are debug-only.
- Introduce a `LOG_PRINTF` macro gated behind `OPENLCB_DEBUG_PRINTF` (propose pattern and location to user before editing).
- Replace raw `printf` calls.
- WASM build omits the flag → zero stdio in the binary.
- **Exit criterion:** WASM build links without pulling in `stdio.h`; debug builds still print.

### 3. One callback round-trip
- Pick one outbound callback (suggest `on_consumed_event_pcer` or `on_pc_event_report`) and one inbound call (suggest `send_event`).
- Wire C → JS via Emscripten's function pointer mechanism (`addFunction` or `EM_JS`).
- Wire JS → C via exported function.
- Prove marshalling pattern works end-to-end.
- **Exit criterion:** event sent from JS flows through C state machine and fires a JS callback.

### 4. Build artifact + handoff
- `build.sh` produces `dist/openlcb-core.wasm` + `dist/openlcb-core.js` reproducibly.
- Document: which feature flags are enabled, exported function list, expected callback signatures, memory layout assumptions.
- Tag/version the artifact.
- Hand off to a new conversation rooted in OpenLcbJSLib for the wrapper work.

## Known risks / watch items

1. **Callback marshalling is the one layer not shared between C and JS.** Keep it thin. Write conformance tests around it — this is where drift can still hide.
2. **Config memory semantics.** C assumes byte-addressable flash/EEPROM. The JS side will eventually back this with IndexedDB or similar; must match read/write offset model exactly. Not a problem for the C-side build, but design the callback signature cleanly now.
3. **Bundle size.** Expect ~100–150KB WASM. Measure once the real build exists.
4. **Bootloader exclusion.** Make sure the build config does not pull in anything from `bootloader/`.

## Rules reminders (from CLAUDE.md)

- Wait for explicit approval before making changes.
- No new folders without permission.
- Every new function/type reference wrapped in the appropriate `OPENLCB_COMPILE_*` ifdef guard. Verify feature flag gating before proposing code.
- No direct `#include` of module headers in core C/H files to call functions — all cross-module calls go through interface structs (function pointers) wired in `openlcb_config.c`.
- Never invent constants — reuse names from `openlcb_types.h` / `can_types.h`.
- Never force a failing test to pass. If a test that looks right fails, stop and report.
- Bootloader has its own rules; do not touch it as part of this work.

## First action for the new conversation

Read this file, then confirm the plan with the user. Do not start writing code until the user approves milestone 1 and the specific files that will be touched.
