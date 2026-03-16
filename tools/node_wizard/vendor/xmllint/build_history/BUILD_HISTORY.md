# xmllint WASM/asm.js Build History

**Date:** March 16, 2026
**Author:** Jim Kueneman + Claude (Anthropic AI assistant)
**Purpose:** Complete record of how the patched xmllint library was created,
so it can be rebuilt from scratch years from now.

---

## Table of Contents

1. [The Problem](#1-the-problem)
2. [Why Not Use an Existing Library?](#2-why-not-use-an-existing-library)
3. [The Solution: Compile libxml2 to asm.js](#3-the-solution-compile-libxml2-to-asmjs)
4. [Prerequisites](#4-prerequisites)
5. [Step 1: Get libxml2 Source](#5-step-1-get-libxml2-source)
6. [Step 2: Configure libxml2 for Minimal Build](#6-step-2-configure-libxml2-for-minimal-build)
7. [Step 3: Build Native Object Files](#7-step-3-build-native-object-files)
8. [Step 4: Compile to asm.js with Emscripten](#8-step-4-compile-to-asmjs-with-emscripten)
9. [Step 5: Patch the Emscripten Output](#9-step-5-patch-the-emscripten-output)
10. [Step 6: Create the Browser API Wrapper](#10-step-6-create-the-browser-api-wrapper)
11. [The Files We Produce](#11-the-files-we-produce)
12. [How It All Fits Together at Runtime](#12-how-it-all-fits-together-at-runtime)
13. [What We Tried and Why It Failed](#13-what-we-tried-and-why-it-failed)
14. [Known Limitations](#14-known-limitations)
15. [Quick Rebuild Cheatsheet](#15-quick-rebuild-cheatsheet)

---

## 1. The Problem

The OpenLCB CDI and FDI editors need to validate user-typed XML against
XSD schemas (CDI 1.4 and FDI) and show errors with **accurate line numbers**
in the browser.

The browser's built-in `DOMParser` can detect malformed XML but:
- Does **not** support XSD schema validation at all
- Does **not** report line numbers for errors
- Cannot tell you "line 24: element 'segment' is missing required attribute 'space'"

We needed a real XML schema validator running in the browser that returns
structured error messages with line numbers.

## 2. Why Not Use an Existing Library?

### kripken/xml.js (the original)

The npm package `kripken/xml.js` is an older Emscripten compilation of
libxml2 that provides `xmllint.validateXML()` in the browser. We started
with this.

**The problem:** It was compiled from an old version of libxml2 (circa 2015)
using an old version of Emscripten. The compiled output:

- Was 4.1 MB (bloated)
- Used deprecated Emscripten patterns (`FS.createDataFile` instead of
  `FS.writeFile`, character-at-a-time stderr via `String.fromCharCode`)
- Collected stderr one character at a time into a single string with no
  line separation, making error parsing unreliable
- **Did not report accurate line numbers** for schema validation errors —
  many errors came back with line 0 or incorrect line numbers
- Used a synchronous blocking pattern that froze the browser during validation
- Could not be reused across calls (state leaked between validations)

The file `xmllint_original.js` (4.1 MB) in this directory is the kripken
original, kept for reference. It is **not** used by the application.

### Other JavaScript XML validators

- `libxmljs` / `libxmljs2` — Node.js only (native C++ addon, not browser)
- `xsd-schema-validator` — Java-based, not browser
- `xmllint-wasm` (npm) — Uses WASM, which is promising, but at the time
  of this work had compatibility issues and did not reliably return line
  numbers for all error types
- Pure JavaScript XSD validators — None exist that handle the full XSD 1.0
  spec needed for CDI/FDI schemas

**Conclusion:** We had to compile libxml2 ourselves.

## 3. The Solution: Compile libxml2 to asm.js

We cloned the official libxml2 source, configured it with only the features
we need (schemas, tree, SAX, XPath — no HTTP, FTP, Python, threads, etc.),
compiled it with Emscripten to asm.js, and then applied 6 surgical patches
to the Emscripten output to make it work correctly as a reusable browser
module.

**Why asm.js instead of WASM?**  asm.js is a subset of JavaScript that runs
everywhere without needing WASM support. It produces a single `.js` file
with no separate `.wasm` binary to serve. This simplifies deployment — the
editors are static HTML files served from any web server (or even `file://`).
The performance difference is negligible for our use case (validating a few
KB of XML takes < 100ms either way).

## 4. Prerequisites

### Emscripten SDK

```bash
# Install emsdk (one time)
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk
./emsdk install latest
./emsdk activate latest

# Verify
source ~/emsdk/emsdk_env.sh
emcc --version
```

**Version used:** Emscripten 5.0.3 (commit 285c424dfa9e83b03cf8490c65ceadb7c45f28eb)

### System

- macOS Darwin 24.6.0, arm64 (Apple Silicon iMac)
- Standard build tools (make, autoconf, etc.)

## 5. Step 1: Get libxml2 Source

```bash
cd /tmp
git clone https://gitlab.gnome.org/GNOME/libxml2.git
cd libxml2
```

**Version used:** libxml2 2.16.0 (commit 8f5f02b "xmlstring: Free cur on
every error for xmlStrncat")

**Important:** The specific commit matters less than the major version.
Any 2.x release with schema support should work. The build process does
NOT modify any libxml2 source files.

## 6. Step 2: Configure libxml2 for Minimal Build

```bash
cd /tmp/libxml2
./autogen.sh
./configure \
    --without-python \
    --without-threads \
    --without-zlib \
    --without-lzma \
    --without-iconv \
    --without-http \
    --without-ftp \
    --without-catalog \
    --without-docbook \
    --without-xptr \
    --without-xinclude \
    --without-c14n \
    --without-modules \
    --without-readline \
    --without-history \
    --without-debug \
    --without-legacy \
    --with-schemas \
    --with-tree \
    --with-output \
    --with-sax1 \
    --with-pattern \
    --with-push \
    --with-valid \
    --with-xpath
```

**Why these flags?**

The `--without-*` flags disable features we don't need, which reduces the
compiled output size from ~4 MB to ~1.5 MB. The critical `--with-*` flags
are:

| Flag | Why |
|------|-----|
| `--with-schemas` | **Essential** — XSD schema validation is the whole point |
| `--with-tree` | DOM tree building (needed by schema validator) |
| `--with-output` | Serialization (needed internally) |
| `--with-sax1` | SAX parser (needed by xmllint) |
| `--with-pattern` | Pattern matching (used by schema validation) |
| `--with-push` | Push parser (used internally) |
| `--with-valid` | DTD validation (used internally by schema code) |
| `--with-xpath` | XPath (used by schema identity constraints) |

## 7. Step 3: Build Native Object Files

```bash
cd /tmp/libxml2
make
```

This builds native `.o` files in `/tmp/libxml2/.libs/`. We need these as
input to Emscripten in the next step. There are approximately 30 object
files named `libxml2_la-*.o`.

**Note:** This native build is just to get the `.o` files. We do NOT use
the resulting native `libxml2.dylib` or `xmllint` binary. Emscripten's
`emcc` can consume these `.o` files and re-link them into asm.js.

## 8. Step 4: Compile to asm.js with Emscripten

This is the build script (`build_asmjs.sh`):

```bash
#!/bin/bash
# Build patched libxml2 xmllint as standalone asm.js
set -e

source ~/emsdk/emsdk_env.sh 2>/dev/null

LIBXML2_DIR=/tmp/libxml2
OUT_DIR=/tmp/xmllint_build

mkdir -p "$OUT_DIR"

# Collect all .o files from the libxml2 build
OBJ_FILES=$(ls "$LIBXML2_DIR"/.libs/libxml2_la-*.o)

# Compile xmllint.c (the CLI tool)
echo "=== Compiling xmllint.c ==="
emcc -c \
    -I"$LIBXML2_DIR"/include \
    -I"$LIBXML2_DIR" \
    -DHAVE_CONFIG_H \
    -O2 \
    "$LIBXML2_DIR"/xmllint.c \
    -o "$OUT_DIR"/xmllint_main.o

# Link everything into asm.js
echo "=== Linking to asm.js ==="
emcc \
    -O2 \
    -s WASM=0 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="createXmllintModule" \
    -s EXIT_RUNTIME=1 \
    -s INVOKE_RUN=0 \
    -s FORCE_FILESYSTEM=1 \
    -s ENVIRONMENT='web,node' \
    --memory-init-file 0 \
    "$OUT_DIR"/xmllint_main.o \
    $OBJ_FILES \
    -o "$OUT_DIR"/xmllint_raw.js

echo "=== Build complete: $OUT_DIR/xmllint_raw.js ==="
ls -lh "$OUT_DIR"/xmllint_raw.js
```

**Emscripten flags explained:**

| Flag | Purpose |
|------|---------|
| `-O2` | Optimization level 2 (good size/speed tradeoff) |
| `-s WASM=0` | Produce asm.js, not WebAssembly (single file, no .wasm) |
| `-s ALLOW_MEMORY_GROWTH=1` | Heap can grow (needed for large XML) |
| `-s MODULARIZE=1` | Wrap output as a factory function (not auto-init) |
| `-s EXPORT_NAME="createXmllintModule"` | Name of the factory function |
| `-s EXIT_RUNTIME=1` | Allow `main()` to exit cleanly |
| `-s INVOKE_RUN=0` | Don't auto-call `main()` — we call it explicitly |
| `-s FORCE_FILESYSTEM=1` | Enable the virtual filesystem for file I/O |
| `-s ENVIRONMENT='web,node'` | Target both browser and Node.js |
| `--memory-init-file 0` | Embed memory init in JS (no separate .mem file) |

**Output:** `/tmp/xmllint_build/xmllint_raw.js` (~1.5 MB)

This raw file is a valid factory function but has several problems that
must be fixed before it works in the browser. That's the next step.

## 9. Step 5: Patch the Emscripten Output

The raw Emscripten output needs 6 patches. These are applied by a Node.js
script (`build_wrapper.js`):

```bash
cd /tmp/xmllint_build
cp xmllint_raw.js xmllint_core.js   # work on a copy
node build_wrapper.js
```

Here is `build_wrapper.js` in full:

```javascript
/* Build the final xmllint.js wrapper with kripken-compatible API */
var fs = require('fs');
var core = fs.readFileSync('xmllint_core.js', 'utf8');

// Patch 1: Remove original Module init
core = core.replace(
    'var Module=typeof Module!="undefined"?Module:{}',
    '/* Module already set */'
);

// Patch 2: Use Module callbacks from the start
core = core.replace(
    'var out=console.log.bind(console)',
    'var out=Module["print"]||console.log.bind(console)'
);
core = core.replace(
    'var err=console.error.bind(console)',
    'var err=Module["printErr"]||console.error.bind(console)'
);

// Patch 3: Don't let FS.open override out/err with file descriptors
core = core.replace(
    /out=FS\.open\("\/dev\/stdout",1\)/g,
    'FS.open("/dev/stdout",1)'
);
core = core.replace(
    /err=FS\.open\("\/dev\/stderr",1\)/g,
    'FS.open("/dev/stderr",1)'
);

// Patch 4: Prevent process.exitCode from being set
core = core.replace(
    /process\.exitCode=status/g,
    '/* no exit */'
);

// Patch 5: Remove module.exports = Module
core = core.replace(
    /if\s*\(typeof module\s*!==?\s*["']undefined["']\)\s*\{?\s*module\[?["']exports["']\]?\s*=\s*Module;?\s*\}?/g,
    ''
);

// Patch 6: Don't use process.argv for arguments
core = core.replace(
    /arguments_=process\.argv\.slice\(2\)/g,
    'arguments_=[]'
);

// Write the patched file
fs.writeFileSync('xmllint_patched.js', core);
console.log('Patches applied to xmllint_patched.js');
```

### What each patch does and WHY

**Patch 1: Remove automatic Module initialization**
```
BEFORE: var Module=typeof Module!="undefined"?Module:{}
AFTER:  /* Module already set */
```
*Why:* Emscripten's default code checks for an existing `Module` object and
uses it if found. But with `MODULARIZE=1`, the Module is passed as a
parameter to the factory function. This leftover init line can clobber the
parameter in some environments. Removing it ensures the factory parameter
is always used.

**Patch 2: Use Module callbacks for stdout/stderr**
```
BEFORE: var out=console.log.bind(console)
AFTER:  var out=Module["print"]||console.log.bind(console)
```
*Why:* The raw output hardwires `out` and `err` to `console.log` and
`console.error` at the top of the file, before any Module callbacks are
checked. Our API wrapper passes `print` and `printErr` callbacks on the
Module object to capture validation output. Without this patch, those
callbacks are ignored and errors go to the console instead of being
captured.

**Patch 3: Prevent FS.open from overriding callbacks**
```
BEFORE: out=FS.open("/dev/stdout",1)
AFTER:  FS.open("/dev/stdout",1)
```
*Why:* During Emscripten's TTY/filesystem initialization, it opens
`/dev/stdout` and `/dev/stderr` and reassigns the `out` and `err`
variables to the file descriptors. This clobbers our carefully-set
callbacks from Patch 2. By removing the assignment (but keeping the
`FS.open` call so the filesystem is properly initialized), our callbacks
survive.

**Patch 4: Suppress process.exitCode**
```
BEFORE: process.exitCode=status
AFTER:  /* no exit */
```
*Why:* When `main()` finishes, Emscripten tries to set `process.exitCode`.
In the browser, `process` doesn't exist (crash). In Node.js, setting
`exitCode` to a non-zero value (xmllint returns 1 when validation fails)
would cause the Node process to exit with an error code. We suppress this
because a validation failure is not a process-level error.

**Patch 5: Remove module.exports**
```
BEFORE: if(typeof module!=='undefined'){module['exports']=Module}
AFTER:  (removed)
```
*Why:* The raw output tries to export the internal `Module` object via
CommonJS `module.exports`. We don't want this — our wrapper exports its
own API (`xmllint.validateXML`). Leaving this in causes conflicts when
loaded via `<script>` tag in the browser.

**Patch 6: Don't read process.argv**
```
BEFORE: arguments_=process.argv.slice(2)
AFTER:  arguments_=[]
```
*Why:* Emscripten tries to read command-line arguments from `process.argv`.
In the browser there is no `process` object. Even in Node.js, we don't
want CLI arguments to leak into the xmllint invocation. The arguments are
set explicitly via `Module["arguments"]` by our API wrapper.

**Output:** The patched file is saved as `xmllint_patched.js` and copied
to `tools/node_wizard/vendor/xmllint/xmllint_patched.js`.

## 10. Step 6: Create the Browser API Wrapper

The file `xmllint_api.js` provides a clean async API on top of the patched
Emscripten module. It is hand-written (not generated).

```javascript
/* xmllint_api.js — async wrapper around the patched Emscripten module */
(function () {
    'use strict';

    var _factory = null;

    function _getFactory() {
        if (_factory) { return Promise.resolve(_factory); }
        if (typeof createXmllintModule === 'function') {
            _factory = createXmllintModule;
            return Promise.resolve(_factory);
        }
        return Promise.reject(new Error(
            'createXmllintModule not loaded — include xmllint_patched.js first'
        ));
    }

    function validateXML(options) {
        var xml    = options.xml;
        var schema = options.schema;
        if (!Array.isArray(xml))    { xml    = [xml]; }
        if (!Array.isArray(schema)) { schema = [schema]; }

        return _getFactory().then(function (factory) {
            var stderrLines = [];

            return factory({
                print:    function () {},
                printErr: function (text) { stderrLines.push(text); }
            }).then(function (Module) {
                /* Write files to Emscripten virtual filesystem */
                for (var i = 0; i < xml.length; i++)
                    Module.FS.writeFile('/file_' + i + '.xml', xml[i]);
                for (var i = 0; i < schema.length; i++)
                    Module.FS.writeFile('/file_' + i + '.xsd', schema[i]);

                /* Build CLI arguments */
                var args = ['--noout'];
                for (var i = 0; i < schema.length; i++) {
                    args.push('--schema');
                    args.push('/file_' + i + '.xsd');
                }
                for (var i = 0; i < xml.length; i++)
                    args.push('/file_' + i + '.xml');

                /* Run xmllint main() */
                try { Module.callMain(args); }
                catch (e) { /* callMain throws on exit — expected */ }

                return {
                    errors: stderrLines.length > 0 ? stderrLines : null
                };
            });
        });
    }

    window.xmllint = { validateXML: validateXML };
}());
```

**Key design decisions:**

- **Fresh module per call:** Each `validateXML()` call creates a new
  Emscripten module instance. This avoids state leaks between validations
  (virtual filesystem files from previous runs, etc.).
- **Async/Promise-based:** The factory returns a Promise, so validation
  never blocks the UI thread.
- **Line-separated stderr:** Each `printErr` callback receives one complete
  line. This is critical — the old kripken library collected stderr one
  character at a time into a single string, making it impossible to
  reliably separate error messages.
- **`--noout` flag:** Tells xmllint to suppress stdout output (we only
  care about stderr errors).

## 11. The Files We Produce

```
vendor/xmllint/
├── xmllint_api.js        (2.5 KB)  Hand-written browser API wrapper
├── xmllint_patched.js    (1.5 MB)  Emscripten output with 6 patches applied
├── xmllint_original.js   (4.1 MB)  Original kripken library (reference only)
└── BUILD_HISTORY.md       (this file)
```

**In the HTML files, load order matters:**
```html
<script src="../vendor/xmllint/xmllint_patched.js"></script>
<script src="../vendor/xmllint/xmllint_api.js"></script>
```
`xmllint_patched.js` must load first (defines `createXmllintModule`),
then `xmllint_api.js` wraps it as `window.xmllint.validateXML()`.

## 12. How It All Fits Together at Runtime

```
User types XML in editor
        │
        ▼
onEditorChange() fires (500ms debounce)
        │
        ▼
CdiValidator.validate(xmlString)    ← cdi_validator.js
        │
        ├─► xmllint.validateXML({ xml, schema: [embedded XSD] })
        │       │
        │       ▼
        │   createXmllintModule()   ← xmllint_patched.js (Emscripten)
        │       │
        │       ▼
        │   Module.FS.writeFile()   writes XML + XSD to virtual filesystem
        │   Module.callMain()       runs xmllint --noout --schema file.xsd file.xml
        │       │
        │       ▼
        │   printErr callback       captures each stderr line
        │       │
        │       ▼
        │   Returns { errors: ["file_0.xml:24: Schemas validity error : ..."] }
        │
        ├─► _parseErrors()          extracts line numbers and messages
        │
        ├─► _runSemanticChecks()    Layer 3 domain-specific hints (DOMParser)
        │
        ▼
Array of { severity, line, col, message }
        │
        ├─► Validation bar          shows formatted error list
        │
        └─► CM6 linter              shows red dots in gutter
```

### Error output format

xmllint writes errors to stderr in this format:
```
/file_0.xml:24: Schemas validity error : Element 'segment': ...
/file_0.xml:6: parser error : expected '>'
```

Format: `/file_N.xml:<line>: <error type> : <message>`

**Note:** There is NO column number in the output. Schema validation
errors only report the line. Parser errors include a caret pointer
(`    ^`) on a separate line, but we skip those.

## 13. What We Tried and Why It Failed

### Attempt 1: Use kripken/xml.js as-is
**Result:** Line numbers were wrong/missing for schema validation errors.
The library was 4.1 MB. Stderr was collected character-by-character making
parsing fragile.

### Attempt 2: Use the npm xmllint-wasm package
**Result:** Had compatibility issues with our setup and did not reliably
return structured error output. Also required serving a separate `.wasm`
file which complicated our static-file deployment.

### Attempt 3: Compile libxml2 to WASM
**Result:** Produced a `.wasm` binary + `.js` loader. Worked but required
serving the `.wasm` file separately, which broke `file://` usage and
complicated deployment. Switched to asm.js (single file).

### Attempt 4: Compile to asm.js, use raw output directly
**Result:** The raw Emscripten output had 6 issues (see Patch section
above) that prevented it from working as a reusable module. Callbacks
were clobbered, exit codes crashed the browser, etc.

### Attempt 5: Wrap with pre.js/post.js (Emscripten's official approach)
**Result:** We created `pre.js` and `post.js` files to wrap the module.
This partially worked but the callback-clobbering issues (Patches 2 and 3)
persisted because the clobbering happens deep inside the generated code,
not at the boundaries where pre/post can intercept.

### Attempt 6 (final): Post-process with surgical string patches
**Result:** Success. The 6 regex-based patches in `build_wrapper.js` fix
each issue at the exact location in the generated code. This approach is
fragile (depends on Emscripten's output format) but works reliably for
our specific Emscripten version. If Emscripten updates change the output
format, the patches may need updating — but the error will be obvious
(string not found → patch fails → build_wrapper.js reports the failure).

### CM6 Linter Integration Attempts

After getting xmllint working, we integrated it with CodeMirror 6's
linter extension to show inline markers (red dots + squiggly underlines)
in the editor gutter.

**Problem:** We wanted an "Auto Validate" checkbox that could turn off
automatic validation (for performance on large files). When auto-validate
was off, clicking a "Verify" button should run validation once.

**Attempt A: `_lintEnabled` flag + `setDiagnostics`**
Used a flag in the CM6 lint source to return `[]` when disabled, and
`setDiagnostics()` to push results from the manual Verify button.
*Failed:* CM6's linter auto-fires 600ms after every edit. When disabled,
it returned `[]`, which overwrote the diagnostics pushed by `setDiagnostics`.
Both write to the same internal CM6 state.

**Attempt B: `_forceNextLint` flag**
Set a flag before calling `forceLinting()` so the lint source would return
real results on the next call.
*Failed 95% of the time:* CM6's auto-triggered lint run consumed the flag
before the `forceLinting` call got to use it.

**Attempt C: Cache diagnostics in lint source**
Cached the last manual verify results and returned them from the lint source
when auto-lint was off.
*Partially worked:* Diagnostics appeared but disappeared when the user
fixed errors — the cache went stale.

**Final solution:** Removed the manual verify mode entirely. The CM6 linter
ALWAYS runs automatically (it's what CM6 was designed for). Squiggly
underlines were suppressed via CSS (just red gutter dots remain). The
"Auto Validate" checkbox and "Verify" button were removed — validation is
always on, always automatic. Simple, reliable, no fighting the framework.

## 14. Known Limitations

1. **No column numbers** — xmllint schema validation errors only report
   line numbers, not columns. The red dot marks the whole line.

2. **File size** — `xmllint_patched.js` is 1.5 MB. This is the cost of
   running a real C library in the browser. It loads once and is cached.

3. **Emscripten version sensitivity** — The 6 patches in `build_wrapper.js`
   depend on specific strings in the Emscripten output. A different
   Emscripten version may produce different output, requiring patch updates.

4. **Single-threaded** — Validation runs on the main thread. For typical
   CDI/FDI files (< 100 KB), this is < 100ms. For very large files, a
   Web Worker would be needed (not implemented).

5. **Fresh module per call** — Each validation creates a new Emscripten
   module instance (~50ms overhead). This prevents state leaks but means
   rapid-fire validation (e.g., on every keystroke) would be expensive.
   The 500ms debounce in the editor and 600ms CM6 linter delay prevent
   this from being a problem in practice.

## 15. Quick Rebuild Cheatsheet

If you need to rebuild everything from scratch:

```bash
# 1. Install Emscripten (if not already)
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
source ~/emsdk/emsdk_env.sh

# 2. Clone and configure libxml2
cd /tmp
git clone https://gitlab.gnome.org/GNOME/libxml2.git
cd libxml2
./autogen.sh
./configure \
    --without-python --without-threads --without-zlib --without-lzma \
    --without-iconv --without-http --without-ftp --without-catalog \
    --without-docbook --without-xptr --without-xinclude --without-c14n \
    --without-modules --without-readline --without-history \
    --without-debug --without-legacy \
    --with-schemas --with-tree --with-output --with-sax1 \
    --with-pattern --with-push --with-valid --with-xpath

# 3. Build native object files
make

# 4. Run the Emscripten build script
chmod +x build_asmjs.sh
./build_asmjs.sh

# 5. Apply patches
cd /tmp/xmllint_build
cp xmllint_raw.js xmllint_core.js
node build_wrapper.js

# 6. Copy result to project
cp xmllint_patched.js /path/to/tools/node_wizard/vendor/xmllint/

# 7. Test in browser — open CDI editor, type invalid XML, check errors
```

**Files to preserve for rebuilding:**
- `build_asmjs.sh` — the Emscripten compile/link script
- `build_wrapper.js` — the 6-patch post-processing script
- This document

Everything else can be regenerated from the libxml2 source and Emscripten.
