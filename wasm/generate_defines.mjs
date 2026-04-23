#!/usr/bin/env node
// Generate wasm/dist/openlcb-defines.mjs from C headers.
//
// Scans a fixed set of OpenLcbCLib headers for `#define NAME VALUE` macros,
// runs the C preprocessor with the WASM build's user-config to know which
// OPENLCB_COMPILE_* blocks are active, then emits an ES module exporting
// those names as JS values. Keeps a single source of truth for protocol
// constants across C and JS.
//
// Usage (called from build.sh before emcmake):
//   node generate_defines.mjs
//
// Output:
//   wasm/dist/openlcb-defines.mjs

import { execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(__dirname, '..');

const HEADERS = [
    `${ROOT}/src/openlcb/openlcb_defines.h`,
    `${ROOT}/src/openlcb/openlcb_float16.h`,
    `${ROOT}/src/openlcb/protocol_train_search_handler.h`,
];

const OUT_PATH = `${__dirname}/dist/openlcb-defines.mjs`;

// ---------------------------------------------------------------------------
// 1. Collect the set of macro names defined in our headers.  Only names that
//    appear as `#define NAME ...` in these files are candidates — this filters
//    out every system macro the preprocessor would otherwise dump.
// ---------------------------------------------------------------------------

function collectMacroNames() {
    const names = new Set();
    const fnLikeNames = new Set();
    const defineRx = /^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)(\s*\()?/gm;
    for (const path of HEADERS) {
        const src = readFileSync(path, 'utf8');
        let m;
        while ((m = defineRx.exec(src)) !== null) {
            const [, name, openParen] = m;
            if (openParen) {
                fnLikeNames.add(name); // function-like macro — skip
            } else {
                names.add(name);
            }
        }
    }
    // Drop function-like macros from the candidate set
    for (const n of fnLikeNames) names.delete(n);
    return names;
}

// ---------------------------------------------------------------------------
// 2. Ask the preprocessor for final macro values under the WASM user config.
//    Uses `cc -E -dM`.  `clang` on macOS ships with Xcode CLT; `cc` maps to
//    either clang or gcc on Linux.  No emcc dependency needed for this step.
// ---------------------------------------------------------------------------

function dumpPreprocessorMacros() {
    const cc = process.env.CC || 'cc';
    const stubPath = `${__dirname}/dist/_defines_stub.c`;
    mkdirSync(`${__dirname}/dist`, { recursive: true });
    const stub = HEADERS.map((h) => `#include "${h}"`).join('\n') + '\n';
    writeFileSync(stubPath, stub);
    const out = execFileSync(cc, [
        '-E', '-dM',
        `-I${__dirname}`,                // picks up wasm/openlcb_user_config.h
        `-I${ROOT}/src`,
        `-I${ROOT}/src/openlcb`,
        `-I${ROOT}/src/drivers/canbus`,
        `-include`, `${__dirname}/openlcb_user_config.h`,
        stubPath,
    ], { encoding: 'utf8' });
    return out;
}

// ---------------------------------------------------------------------------
// 3. Filter the preprocessor dump to our candidate names, grab their RHS text.
// ---------------------------------------------------------------------------

function filterDump(dump, candidates) {
    const rx = /^#define\s+([A-Za-z_][A-Za-z0-9_]*)\s+(.*)$/;
    const defs = new Map();
    for (const line of dump.split('\n')) {
        const m = rx.exec(line);
        if (!m) continue;
        const [, name, rhs] = m;
        if (!candidates.has(name)) continue;
        defs.set(name, rhs.trim());
    }
    return defs;
}

// ---------------------------------------------------------------------------
// 4. Translate a C literal / expression into a JS value.
//    Supports: integer literals (with U/L/UL/LL suffixes, hex/dec/oct),
//    string literals, bitwise/arith expressions referencing already-known
//    macros.  Returns { kind: 'number'|'bigint'|'string', value }.
// ---------------------------------------------------------------------------

const INT_RX = /^[+-]?(?:0[xX][0-9A-Fa-f]+|0[0-7]*|\d+)(?:[uUlL]+)?$/;
const STRING_RX = /^"(.*)"$/;

function stripIntSuffix(tok) {
    return tok.replace(/[uUlL]+$/, '');
}

function toBigIntOrNumber(n) {
    // Store as Number if representable safely (<= 2^53 - 1), else BigInt.
    if (typeof n === 'bigint') {
        if (n >= -9007199254740991n && n <= 9007199254740991n) return { kind: 'number', value: Number(n) };
        return { kind: 'bigint', value: n };
    }
    return { kind: 'number', value: n };
}

function evalExpr(expr, resolved) {
    // Substitute known names with their JS values, then eval in a sandboxed
    // Function body.  All C operators used in these headers (|, &, <<, >>, +,
    // -, *, /, ~, ^, parentheses) match JS semantics when operating on
    // BigInts, so coerce everything to BigInt for safety.
    const tokens = expr.match(/[A-Za-z_][A-Za-z0-9_]*|0[xX][0-9A-Fa-f]+[uUlL]*|\d+[uUlL]*|[()|&~^+\-*/<>!]+|\s+/g) || [];
    let hasString = false;
    const js = tokens.map((t) => {
        if (/^\s+$/.test(t)) return t;
        if (/^[A-Za-z_][A-Za-z0-9_]*$/.test(t)) {
            if (!resolved.has(t)) throw new Error(`unresolved ref: ${t} in "${expr}"`);
            const v = resolved.get(t);
            if (v.kind === 'string') { hasString = true; return JSON.stringify(v.value); }
            return `(${typeof v.value === 'bigint' ? v.value.toString() + 'n' : v.value})`;
        }
        if (INT_RX.test(t)) {
            const clean = stripIntSuffix(t);
            return `(${BigInt(clean).toString()}n)`;
        }
        return t; // operator / paren
    }).join('');

    // eslint-disable-next-line no-new-func
    const result = Function(`"use strict"; return (${js});`)();
    if (hasString) return { kind: 'string', value: String(result) };
    return toBigIntOrNumber(typeof result === 'bigint' ? result : BigInt(result));
}

function translate(rhs, resolved) {
    // String literal
    const sm = STRING_RX.exec(rhs);
    if (sm) return { kind: 'string', value: sm[1] };

    // Strip a single outer paren if present
    let body = rhs.trim();
    while (body.startsWith('(') && body.endsWith(')')) {
        // Only strip if the parens are balanced at position 0
        let depth = 0, paired = true;
        for (let i = 0; i < body.length; i++) {
            if (body[i] === '(') depth++;
            else if (body[i] === ')') { depth--; if (depth === 0 && i !== body.length - 1) { paired = false; break; } }
        }
        if (paired) body = body.slice(1, -1).trim(); else break;
    }

    // Simple int literal
    if (INT_RX.test(body)) {
        return toBigIntOrNumber(BigInt(stripIntSuffix(body)));
    }

    // Bare identifier referencing another macro
    if (/^[A-Za-z_][A-Za-z0-9_]*$/.test(body)) {
        if (!resolved.has(body)) throw new Error(`unresolved ref: ${body}`);
        return resolved.get(body);
    }

    // Expression
    return evalExpr(body, resolved);
}

// ---------------------------------------------------------------------------
// 5. Resolve all defs.  Some reference others, so iterate until fixed point
//    or a cycle / unresolvable ref is detected.
// ---------------------------------------------------------------------------

function resolveAll(defs) {
    const resolved = new Map();
    const pending = new Map(defs);
    const skipped = new Map();

    let progress = true;
    while (pending.size > 0 && progress) {
        progress = false;
        for (const [name, rhs] of pending) {
            try {
                const value = translate(rhs, resolved);
                resolved.set(name, value);
                pending.delete(name);
                progress = true;
            } catch (e) {
                // try again next pass
            }
        }
    }

    for (const [name, rhs] of pending) {
        skipped.set(name, rhs);
    }
    return { resolved, skipped };
}

// ---------------------------------------------------------------------------
// 6. Emit ES module.
// ---------------------------------------------------------------------------

function emit(resolved, skipped) {
    const names = [...resolved.keys()].sort();
    const lines = [];
    lines.push('// AUTO-GENERATED by wasm/generate_defines.mjs from OpenLcbCLib headers.');
    lines.push('// Do not hand-edit. Rebuild: `node wasm/generate_defines.mjs`.');
    lines.push('');
    for (const n of names) {
        const v = resolved.get(n);
        if (v.kind === 'bigint') lines.push(`export const ${n} = ${v.value.toString()}n;`);
        else if (v.kind === 'string') lines.push(`export const ${n} = ${JSON.stringify(v.value)};`);
        else lines.push(`export const ${n} = ${v.value};`);
    }
    if (skipped.size > 0) {
        lines.push('');
        lines.push('// The following macros could not be resolved (function-like or complex):');
        for (const [n, rhs] of skipped) lines.push(`//   ${n} = ${rhs}`);
    }
    lines.push('');
    return lines.join('\n');
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

function main() {
    const candidates = collectMacroNames();
    const dump = dumpPreprocessorMacros();
    const defs = filterDump(dump, candidates);
    const { resolved, skipped } = resolveAll(defs);
    mkdirSync(dirname(OUT_PATH), { recursive: true });
    writeFileSync(OUT_PATH, emit(resolved, skipped));
    console.log(`Wrote ${OUT_PATH}  (${resolved.size} constants, ${skipped.size} skipped)`);
}

main();
