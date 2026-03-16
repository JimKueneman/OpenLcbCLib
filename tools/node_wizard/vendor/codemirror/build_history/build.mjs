/* Build script — produces a single IIFE bundle at dist/codemirror.min.js */

import { build } from 'esbuild';

await build({
    entryPoints: ['entry.mjs'],
    bundle: true,
    format: 'iife',
    minify: true,
    outfile: '../dist/codemirror.min.js',
    target: ['es2020']
});

console.log('Built ../dist/codemirror.min.js');
