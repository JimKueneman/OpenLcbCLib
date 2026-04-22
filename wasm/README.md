# OpenLcbCLib — WebAssembly build

Emscripten build of OpenLcbCLib that `OpenLcbJSLib` consumes as a thin wrapper.
Transport is CAN with gridconnect text frames; the JS side is expected to
move those frames over a WebSocket or equivalent socket.  Native binary TCP
is planned for the future.

## Build

```
cd wasm
./build.sh
```

Produces `wasm/dist/openlcb-core.{js,wasm}`.  Requires Emscripten on `PATH`
(`brew install emscripten`).

Current artifact size: ~91 KB `.wasm` + ~12 KB `.js` glue.

## Feature flags enabled

Set in [`openlcb_user_config.h`](openlcb_user_config.h):

- `OPENLCB_COMPILE_CAN`
- `OPENLCB_COMPILE_EVENTS`
- `OPENLCB_COMPILE_DATAGRAMS`
- `OPENLCB_COMPILE_MEMORY_CONFIGURATION`
- `OPENLCB_COMPILE_FIRMWARE`
- `OPENLCB_COMPILE_BROADCAST_TIME`
- `OPENLCB_COMPILE_TRAIN`
- `OPENLCB_COMPILE_TRAIN_SEARCH`
- `OPENLCB_COMPILE_STREAM`
- `OPENLCB_COMPILE_DCC_DETECTOR`

## Exported functions

All exports use the C ABI.  Use `Module.cwrap` from JS.

| Export | Signature | Purpose |
|--------|-----------|---------|
| `wasm_init` | `(node_id: bigint) => void` | Initialize CAN + OpenLCB and create one node at the given 48-bit ID. |
| `wasm_run` | `() => void` | Run one pass of the main state machine.  Call in a loop. |
| `wasm_100ms_tick` | `() => void` | Increment the global 100 ms tick counter.  Call every 100 ms. |
| `wasm_rx_gridconnect` | `(cstr: string) => void` | Feed a null-terminated gridconnect string (one or more concatenated `:X...;` frames) into the CAN RX path. |

Also exported: `_malloc`, `_free`.  Runtime methods: `ccall`, `cwrap`,
`HEAPU8`.

## JS-side hooks

The JS module accepts hooks via the factory options object:

```js
import factory from "./openlcb-core.js";

const Module = await factory({
    onGridconnectTx: (frame) => { /* send `frame` over your socket */ },
});
```

| Hook | Signature | Purpose |
|------|-----------|---------|
| `Module.onGridconnectTx` | `(frame: string) => void` | Called for every outgoing gridconnect frame.  The string is a single `:X...;` message (no trailing newline). |

Driver callbacks not listed above (config-memory read/write, firmware
freeze/unfreeze, etc.) are currently stubbed.  Wire them through JS when the
OpenLcbJSLib wrapper needs persistent storage or firmware upgrade.

## Minimum JS usage

```js
import factory from "./openlcb-core.js";

const Module = await factory({
    onGridconnectTx: (frame) => socket.send(frame + "\n"),
});

const init = Module.cwrap("wasm_init",            null, ["bigint"]);
const run  = Module.cwrap("wasm_run",             null, []);
const tick = Module.cwrap("wasm_100ms_tick",      null, []);
const rx   = Module.cwrap("wasm_rx_gridconnect",  null, ["string"]);

init(0x050101010707n);

setInterval(() => {
    run();
}, 10);

setInterval(() => {
    tick();
}, 100);

socket.onmessage = (ev) => rx(ev.data);
```

## Memory and threading

- Static pools only — no `malloc` in the protocol core.  Pool depths are set
  in [`openlcb_user_config.h`](openlcb_user_config.h).
- Single-threaded.  The `lock_shared_resources` / `unlock_shared_resources`
  callbacks are no-ops in this build.
- `ALLOW_MEMORY_GROWTH` is enabled; initial stack is 64 KB.

## Layout

```
wasm/
  CMakeLists.txt          Emscripten build config
  build.sh                one-command build
  bindings.c              C entry points + JS callback trampolines
  openlcb_user_config.h   feature flags, buffer sizes
  openlcb_user_config.c   node_parameters_t
  can_user_config.h       CAN driver pool size
  dist/                   build output (gitignored)
```

## Known gaps

1. Persistent config memory is stubbed.  A real wrapper needs to route
   `config_mem_read` / `config_mem_write` to IndexedDB or similar.
2. Firmware write is stubbed to "success" without actually writing.
3. Only one JS callback is wired (`onGridconnectTx`).  Event notifications,
   train callbacks, etc., are available at the C API but not yet exposed.
