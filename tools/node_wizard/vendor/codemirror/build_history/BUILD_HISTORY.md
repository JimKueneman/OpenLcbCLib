# CodeMirror 6 Bundle — Build History

**Date:** March 16, 2026
**Author:** Jim Kueneman + Claude (Anthropic AI assistant)
**Purpose:** Complete record of how the CodeMirror 6 bundle was created and how
to rebuild or extend it.

---

## Why a Custom Bundle?

The CDI and FDI XML editors run as standalone HTML files — no server, no module
bundler at runtime. CodeMirror 6 is distributed as ES modules (dozens of small
npm packages), which cannot be loaded directly via `<script>` tags.

The solution: use esbuild to pre-bundle all needed CM6 packages into a single
IIFE file (`dist/codemirror.min.js`) that exposes everything on `window.CM`.
The editors then use `window.CM.EditorView`, `window.CM.linter`, etc.

---

## What's in the Bundle

The entry point (`entry.mjs`) controls exactly which CM6 symbols are exported:

```javascript
import { EditorView, basicSetup } from 'codemirror';
import { EditorState } from '@codemirror/state';
import { xml } from '@codemirror/lang-xml';
import { cpp } from '@codemirror/lang-cpp';
import { oneDark } from '@codemirror/theme-one-dark';
import { keymap } from '@codemirror/view';
import { indentWithTab } from '@codemirror/commands';
import { linter, lintGutter, forceLinting, setDiagnostics } from '@codemirror/lint';

window.CM = {
    EditorView, EditorState, basicSetup, xml, cpp, oneDark,
    keymap, indentWithTab, linter, lintGutter, forceLinting, setDiagnostics
};
```

| Package | Why |
|---------|-----|
| `codemirror` (basicSetup) | Core editor with line numbers, bracket matching, etc. |
| `@codemirror/state` | EditorState for creating editor instances |
| `@codemirror/lang-xml` | XML syntax highlighting + schema-aware autocomplete |
| `@codemirror/lang-cpp` | C/C++ highlighting (used by Node Builder code preview) |
| `@codemirror/theme-one-dark` | Dark theme matching VS Code's look |
| `@codemirror/view` | keymap extension |
| `@codemirror/commands` | indentWithTab for tab key behavior |
| `@codemirror/lint` | Linter extension for inline validation (red dots in gutter) |

---

## npm Dependencies

From `package.json`:

```json
{
  "dependencies": {
    "@codemirror/lang-cpp": "^6.0.2",
    "@codemirror/lang-xml": "^6.1.0",
    "@codemirror/lint": "^6.8.4",
    "@codemirror/theme-one-dark": "^6.1.3",
    "codemirror": "^6.0.2",
    "esbuild": "^0.27.4"
  }
}
```

`esbuild` is the bundler (extremely fast, no config file needed).

---

## Build Script

The build script (`build.mjs`) is minimal:

```javascript
import { build } from 'esbuild';

await build({
    entryPoints: ['entry.mjs'],
    bundle: true,
    format: 'iife',
    minify: true,
    outfile: '../dist/codemirror.min.js',
    target: ['es2020']
});
```

| Flag | Purpose |
|------|---------|
| `bundle: true` | Resolve all imports and inline them |
| `format: 'iife'` | Wrap as immediately-invoked function (no module system needed) |
| `minify: true` | Reduce file size |
| `target: ['es2020']` | Don't transpile modern JS features browsers already support |

---

## How to Rebuild

```bash
cd tools/node_wizard/vendor/codemirror/build_history

# Install dependencies (first time, or after package.json changes)
npm install

# Build the bundle
npm run build
# — or equivalently —
node build.mjs
```

**Output:** `dist/codemirror.min.js` (in the parent directory)

> **Note:** The `node_modules/` folder is NOT committed to the repo. Run
> `npm install` to recreate it. It contains platform-specific esbuild binaries,
> so it must be installed on the same OS/architecture where the build runs.

---

## How to Add a New CM6 Extension

1. Install the npm package:
   ```bash
   cd tools/node_wizard/vendor/codemirror/build_history
   npm install @codemirror/new-package
   ```

2. Add the import to `entry.mjs`:
   ```javascript
   import { newThing } from '@codemirror/new-package';
   ```

3. Add it to the `window.CM` export object:
   ```javascript
   window.CM = {
       // ... existing exports ...
       newThing
   };
   ```

4. Rebuild:
   ```bash
   npm run build
   ```

5. Use it in `cm-adapter.js` as `CM.newThing`.

---

## History of Changes

### Initial bundle (pre-March 2026)
- Core CM6 with XML and C++ language support, oneDark theme
- Used by CDI and FDI editors for syntax highlighting and autocomplete

### March 2026 — Added lint support
- Added `@codemirror/lint` package
- Exported `linter`, `lintGutter`, `forceLinting`, `setDiagnostics`
- Enabled inline validation markers (red dots in editor gutter)
- Integrated with xmllint-based XSD validation

---

## Files

```
build_history/
├── BUILD_HISTORY.md           (this file)
├── build.mjs                  esbuild script
├── entry.mjs                  bundle entry point (controls window.CM exports)
├── generate_cdi_schema.py     regenerates CDI autocomplete schema in cm-adapter.js
├── generate_fdi_schema.py     regenerates FDI autocomplete schema in cm-adapter.js
├── package.json               npm dependencies
└── package-lock.json          npm lockfile

Parent directory (runtime files):
├── cm-adapter.js              editor wrapper (used by CDI/FDI HTML pages)
├── cm-readonly.js             read-only editor variant
└── dist/
    └── codemirror.min.js      the built bundle (loaded via <script> tag)
```
