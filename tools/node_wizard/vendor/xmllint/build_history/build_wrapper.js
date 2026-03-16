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
// These are in the TTY staticInit - replace them to preserve our callbacks
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

// Build the wrapper
var wrapper = 'var xmllint = {};\n';
wrapper += 'xmllint.validateXML = function(options) {\n';
wrapper += '    var xml = options.xml;\n';
wrapper += '    var schema = options.schema;\n';
wrapper += '    if (!Array.isArray(xml)) xml = [xml];\n';
wrapper += '    if (!Array.isArray(schema)) schema = [schema];\n';
wrapper += '    var errorLines = [];\n';
wrapper += '\n';
wrapper += '    var Module = {};\n';
wrapper += '    Module["arguments"] = ["--noout"];\n';
wrapper += '    for (var si = 0; si < schema.length; si++) {\n';
wrapper += '        Module["arguments"].push("--schema");\n';
wrapper += '        Module["arguments"].push("file_" + si + ".xsd");\n';
wrapper += '    }\n';
wrapper += '    for (var xi = 0; xi < xml.length; xi++) {\n';
wrapper += '        Module["arguments"].push("file_" + xi + ".xml");\n';
wrapper += '    }\n';
wrapper += '    Module["preRun"] = [function() {\n';
wrapper += '        for (var i = 0; i < xml.length; i++) {\n';
wrapper += '            FS.writeFile("/file_" + i + ".xml", xml[i]);\n';
wrapper += '        }\n';
wrapper += '        for (var i = 0; i < schema.length; i++) {\n';
wrapper += '            FS.writeFile("/file_" + i + ".xsd", schema[i]);\n';
wrapper += '        }\n';
wrapper += '    }];\n';
wrapper += '    Module["print"] = function() {};\n';
wrapper += '    Module["printErr"] = function(text) { errorLines.push(text); };\n';
wrapper += '    Module["quit"] = function() {};\n';
wrapper += '\n';
wrapper += '    try {\n';
wrapper += core;
wrapper += '\n    } catch(e) { /* exit exception expected */ }\n';
wrapper += '    return { errors: errorLines.length > 0 ? errorLines : null };\n';
wrapper += '};\n';
wrapper += '\n';
wrapper += 'if (typeof window === "undefined") {\n';
wrapper += '    if (typeof module !== "undefined") {\n';
wrapper += '        module.exports = xmllint;\n';
wrapper += '    }\n';
wrapper += '}\n';

fs.writeFileSync('xmllint.js', wrapper);
console.log('Written xmllint.js:', (wrapper.length / 1024 / 1024).toFixed(2) + ' MB');
console.log('Patches applied:');
console.log('  - Module init removed');
console.log('  - out/err callbacks from Module');
console.log('  - FS.open no longer overrides out/err');
console.log('  - process.exitCode suppressed');
console.log('  - module.exports removed');
console.log('  - process.argv removed');
