/* ============================================================
 * codegen.js — Code generation for openlcb_user_config.h/.c
 *
 * Called by app.js whenever any UI field changes.
 * Reads a plain state object and returns file contents as strings.
 *
 * State object shape:
 *   nodeType         'basic' | 'typical' | 'train' | 'train-controller' | 'bootloader' | null
 *   projectName      string   (project name for filenames)
 *   projectAuthor    string   (author for copyright headers)
 *   projectNodeId    string   (xx.xx.xx.xx.xx.xx hex Node ID)
 *   broadcast        'none' | 'producer' | 'consumer'
 *   firmware         boolean
 *   snipEnabled      boolean  (Basic only — SNIP add-on checkbox)
 *   snipName         string
 *   snipModel        string
 *   snipHw           string
 *   snipSw           string
 *   unalignedReads      boolean  (Typical/Train)
 *   unalignedWrites     boolean  (Typical/Train)
 *   configMemHighest    string   (hex or decimal, e.g. '0x200')
 *   producerCount       number   (default 64)
 *   producerRangeCount  number   (default 4; Broadcast Time needs +2)
 *   consumerCount       number   (default 64)
 *   consumerRangeCount  number   (default 4; Broadcast Time needs +2)
 *   cdiBytes            Uint8Array | null
 *   cdiLength        number   (bytes including null terminator)
 *   fdiBytes         Uint8Array | null
 *   fdiLength        number
 * ============================================================ */

/* ---------- Embedded default XML files ---------- */

var DEFAULT_CDI_XML = [
    '<?xml version="1.0"?>',
    '<cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"',
    '     xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/1/cdi.xsd">',
    '  <identification>',
    '    <manufacturer></manufacturer>',
    '    <model></model>',
    '    <hardwareVersion></hardwareVersion>',
    '    <softwareVersion></softwareVersion>',
    '  </identification>',
    '  <acdi/>',
    '  <segment space="253">',
    '    <name>User Settings</name>',
    '    <string size="63">',
    '      <name>User Name</name>',
    '    </string>',
    '    <string size="64">',
    '      <name>User Description</name>',
    '    </string>',
    '  </segment>',
    '</cdi>'
].join('\n');

var DEFAULT_FDI_XML = [
    '<?xml version="1.0"?>',
    '<fdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"',
    '     xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/fdi/1/1/fdi.xsd">',
    '  <segment>',
    '    <function kind="binary">',
    '      <name>Headlight</name>',
    '      <number>0</number>',
    '    </function>',
    '  </segment>',
    '</fdi>'
].join('\n');

/* ---------- XML preprocessing -------------------------------------------- */

/*
 * Strip XML comments and optionally whitespace before byte conversion.
 * Always strips <!-- ... --> comments (including multi-line).
 * When preserveWhitespace is false (default), also strips leading/trailing
 * whitespace from each line, drops blank lines, and removes newlines —
 * producing the minimal byte representation per the CDI spec.
 */
function _preprocessXml(xml, preserveWhitespace) {

    if (!xml) { return ''; }

    /* Always strip XML comments (single-line and multi-line) */
    var cleaned = xml.replace(/<!--[\s\S]*?-->/g, '');

    /* Normalize line endings */
    cleaned = cleaned.replace(/\r\n/g, '\n').replace(/\r/g, '\n');

    if (preserveWhitespace) {

        /* Drop lines that became whitespace-only after comment removal */
        var lines = cleaned.split('\n');
        var kept = [];

        for (var i = 0; i < lines.length; i++) {
            if (lines[i].trim().length > 0) {
                kept.push(lines[i]);
            }
        }

        return kept.join('\n');

    }

    /* Default: strip leading/trailing whitespace per line, drop blank lines.
     * Lines stay newline-separated so _byteArrayStr() can emit one row per
     * XML line — but no 0x0A bytes are emitted between them. */
    var lines = cleaned.split('\n');
    var kept = [];

    for (var i = 0; i < lines.length; i++) {

        var trimmed = lines[i].trim();

        if (trimmed.length > 0) {
            kept.push(trimmed);
        }

    }

    return kept.join('\n');

}

/* Pre-computed default byte arrays (XML → UTF-8 + null terminator) */

function _xmlToBytes(xml, preserveWhitespace) {

    var processed = _preprocessXml(xml, preserveWhitespace);

    /* _preprocessXml always returns \n-separated lines for display.
     * For byte encoding in stripped mode, remove those separators. */
    if (!preserveWhitespace) {
        processed = processed.replace(/\n/g, '');
    }

    var raw       = new TextEncoder().encode(processed);
    var result    = new Uint8Array(raw.length + 1);

    result.set(raw);
    result[raw.length] = 0;

    return result;

}

var DEFAULT_CDI_BYTES = _xmlToBytes(DEFAULT_CDI_XML);
var DEFAULT_FDI_BYTES = _xmlToBytes(DEFAULT_FDI_XML);

/* ---------- Internal helpers ---------- */

function _escC(str) {

    return (str || '').replace(/\\/g, '\\\\').replace(/"/g, '\\"').replace(/\n/g, '\\n');

}

/*
 * Convert dotted Node ID (xx.xx.xx.xx.xx.xx) to a 0x hex literal.
 * Returns null if the input is empty or invalid.
 */
function _nodeIdToHex(dotted) {

    if (!dotted) { return null; }
    var re = /^([0-9A-Fa-f]{2})\.([0-9A-Fa-f]{2})\.([0-9A-Fa-f]{2})\.([0-9A-Fa-f]{2})\.([0-9A-Fa-f]{2})\.([0-9A-Fa-f]{2})$/;
    var m = re.exec(dotted.trim());
    if (!m) { return null; }
    return '0x' + m[1] + m[2] + m[3] + m[4] + m[5] + m[6];

}

/*
 * Pad a #define name to a fixed column width.
 */
function _padName(name, width) {

    var s = name;

    while (s.length < width) {
        s += ' ';
    }

    return s;

}

function _def(name, value) {

    return '#define ' + _padName(name, 56) + value;

}

/*
 * Extract <identification> fields from CDI XML text.
 * Returns { manufacturer, model, hardwareVersion, softwareVersion }.
 */
function _extractCdiIdentification(xmlText) {

    var result = { manufacturer: '', model: '', hardwareVersion: '', softwareVersion: '' };

    if (!xmlText) { return result; }

    try {

        var parser = new DOMParser();
        var doc    = parser.parseFromString(xmlText, 'text/xml');
        var ident  = doc.querySelector('identification');

        if (ident) {

            var mfg = ident.querySelector('manufacturer');
            var mdl = ident.querySelector('model');
            var hw  = ident.querySelector('hardwareVersion');
            var sw  = ident.querySelector('softwareVersion');

            if (mfg) { result.manufacturer    = mfg.textContent.trim(); }
            if (mdl) { result.model           = mdl.textContent.trim(); }
            if (hw)  { result.hardwareVersion = hw.textContent.trim(); }
            if (sw)  { result.softwareVersion = sw.textContent.trim(); }

        }

    } catch (e) { /* ignore parse errors */ }

    return result;

}

/*
 * Walk CDI XML and calculate the highest byte address used in a given address space.
 * Returns -1 if no items found in that space, otherwise the highest byte address (exclusive).
 */
function _cdiHighestAddrInSpace(xmlText, spaceNum) {

    if (!xmlText) { return -1; }

    try {

        var parser = new DOMParser();
        var doc    = parser.parseFromString(xmlText, 'text/xml');
        var segs   = doc.querySelectorAll('segment');
        var highest = -1;

        for (var i = 0; i < segs.length; i++) {

            var seg   = segs[i];
            var space = parseInt(seg.getAttribute('space')) || 0;
            if (space !== spaceNum) { continue; }

            var origin = parseInt(seg.getAttribute('origin')) || 0;
            var addr   = origin;

            function walk(node) {

                var children = node.children;
                for (var j = 0; j < children.length; j++) {

                    var child = children[j];
                    var tag   = child.tagName;

                    if (['name', 'description', 'repname', 'hints', 'map',
                         'default', 'min', 'max', 'relation', 'property',
                         'value', 'checkbox', 'radiobutton', 'slider'].indexOf(tag) !== -1) { continue; }

                    var off = parseInt(child.getAttribute('offset')) || 0;
                    if (off) { addr += off; }

                    if (tag === 'group') {

                        var replication = parseInt(child.getAttribute('replication')) || 1;
                        for (var r = 0; r < replication; r++) { walk(child); }

                    } else if (['int', 'float', 'string', 'eventid', 'bit'].indexOf(tag) !== -1) {

                        var sAttr   = child.getAttribute('size');
                        var sParsed = parseInt(sAttr);
                        var size    = 0;

                        if (tag === 'int')          { size = [1, 2, 4, 8].indexOf(sParsed) !== -1 ? sParsed : 1; }
                        else if (tag === 'float')   { size = !isNaN(sParsed) ? sParsed : 4; }
                        else if (tag === 'string')  { size = !isNaN(sParsed) ? sParsed : 0; }
                        else if (tag === 'eventid') { size = 8; }
                        else if (tag === 'bit')     { size = !isNaN(sParsed) ? sParsed : 0; }

                        var end = addr + size;
                        if (end > highest) { highest = end; }
                        addr = end;
                    }
                }
            }

            walk(seg);
        }

        return highest;

    } catch (e) { return -1; }

}

/*
 * Format XML as a C byte-array initializer.
 * Each XML line becomes one row of hex bytes followed by a // comment
 * showing the original text.  A 0x00 null terminator is appended as the
 * final row.  When preserveWhitespace is true, 0x0A newline bytes are
 * emitted between lines.
 *
 * Falls back to a plain hex dump when xmlText is not provided.
 */
function _byteArrayStr(bytes, xmlText, preserveWhitespace, indent) {

    indent = indent || '    ';

    if (!bytes || bytes.length === 0) {
        return '{ 0x00 }';
    }

    /* If we have the original XML text, emit one row per line with a comment */
    if (xmlText) {

        var processed = _preprocessXml(xmlText, preserveWhitespace);
        var xmlLines  = processed.split('\n');
        var encoder   = new TextEncoder();
        var lines     = [];
        var totalBytes = 0;

        for (var i = 0; i < xmlLines.length; i++) {

            var lineText = xmlLines[i];
            if (!preserveWhitespace && lineText.trim().length === 0) { continue; }

            var lineBytes = encoder.encode(lineText);
            var chunk = [];

            for (var j = 0; j < lineBytes.length; j++) {
                chunk.push('0x' + (lineBytes[j] < 16 ? '0' : '') + lineBytes[j].toString(16).toUpperCase());
            }

            totalBytes += lineBytes.length;

            /* Add newline byte between lines when preserving whitespace */
            if (preserveWhitespace && i < xmlLines.length - 1) {
                chunk.push('0x0A');
                totalBytes += 1;
            }

            lines.push(indent + chunk.join(', ') + ',  // ' + lineText);

        }

        /* Null terminator */
        totalBytes += 1;
        lines.push(indent + '0x00  // null terminator (' + totalBytes + ' bytes total)');

        return '{\n' + lines.join('\n') + '\n' + indent.slice(0, -4) + '}';

    }

    /* Fallback: plain hex dump (no XML text available) */
    var cols = 16;
    var lines = [];

    for (var i = 0; i < bytes.length; i += cols) {

        var chunk = [];

        for (var j = i; j < Math.min(i + cols, bytes.length); j++) {
            chunk.push('0x' + (bytes[j] < 16 ? '0' : '') + bytes[j].toString(16).toUpperCase());
        }

        var comma = (i + cols < bytes.length) ? ',' : '';
        lines.push(indent + chunk.join(', ') + comma);

    }

    var result = '{\n' + lines.join('\n');
    result += '\n' + indent + '/* ' + bytes.length + ' bytes total */';
    result += '\n' + indent.slice(0, -4) + '}';

    return result;

}

/* ---------- section header helper ---------- */

function _section(title) {

    return '// =============================================================================\n'
         + '// ' + title + '\n'
         + '// =============================================================================';

}

/*
 * Lazily convert cdiUserXml / fdiUserXml to byte arrays if the caller
 * passed XML strings but not pre-built Uint8Arrays (file-preview path).
 */
function _ensureBytes(s) {

    var pw = !!s.preserveWhitespace;

    if (!s.cdiBytes && s.cdiUserXml) {
        s.cdiBytes  = _xmlToBytes(s.cdiUserXml, pw);
        s.cdiLength = s.cdiBytes.length;
    }

    if (!s.fdiBytes && s.fdiUserXml) {
        s.fdiBytes  = _xmlToBytes(s.fdiUserXml, pw);
        s.fdiLength = s.fdiBytes.length;
    }

}

/* ---------- generateH ---------- */

function generateH(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated .h file */';
    }

    _ensureBytes(s);

    var isTrainRole  = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrain      = isTrainRole;   /* alias: both roles need train feature flags */
    var isBasic      = s.nodeType === 'basic';
    var isBootloader = s.nodeType === 'bootloader';
    var hasCfgMem    = !isBasic && !isBootloader;
    var broadcastOn  = s.broadcast !== 'none';
    var firmwareOn   = s.firmware && hasCfgMem;
    var nodeLabel    = s.nodeType === 'train-controller' ? 'Train Controller'
                     : s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var L = [];

    var _author = s.projectAuthor || '<YOUR NAME OR COMPANY>';
    var _year   = new Date().getFullYear();

    L.push('/** \\copyright');
    L.push(' *   Copyright (c) ' + _year + ', ' + _author);
    L.push(' *   All rights reserved.');
    L.push(' *');
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard \u2014 https://github.com/JimKueneman/OpenLcbCLib');
    L.push(' *');
    L.push(' * @file openlcb_user_config.h');
    L.push(' *  @brief User-editable project configuration for OpenLcbCLib');
    L.push(' *');
    L.push(' *  REQUIRED: Copy this file to your project\'s include path and edit.');
    L.push(' *    Arduino:     place next to your .ino file (sketch dir is on include path)');
    L.push(' *    PlatformIO:  place in src/ directory');
    L.push(' *    STM32 Cube:  place in Core/Inc/ directory');
    L.push(' *    CMake/Make:  place anywhere, add -I flag pointing to its directory');
    L.push(' *');
    L.push(' *  ALL values in this file are MANDATORY. The library will not compile if');
    L.push(' *  any are missing. Edit the values to match your project\'s requirements.');
    L.push(' *');
    L.push(' *  --- Quick Recipes for Feature Flags ---');
    L.push(' *');
    L.push(' *  Simple sensor/button node (events only):');
    L.push(' *    #define OPENLCB_COMPILE_EVENTS');
    L.push(' *');
    L.push(' *  Standard configurable node (events + config memory):');
    L.push(' *    #define OPENLCB_COMPILE_EVENTS');
    L.push(' *    #define OPENLCB_COMPILE_DATAGRAMS');
    L.push(' *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
    L.push(' *    #define OPENLCB_COMPILE_FIRMWARE');
    L.push(' *');
    L.push(' *  Train command station:');
    L.push(' *    #define OPENLCB_COMPILE_EVENTS');
    L.push(' *    #define OPENLCB_COMPILE_DATAGRAMS');
    L.push(' *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
    L.push(' *    #define OPENLCB_COMPILE_FIRMWARE');
    L.push(' *    #define OPENLCB_COMPILE_TRAIN');
    L.push(' *    #define OPENLCB_COMPILE_TRAIN_SEARCH');
    L.push(' *');
    L.push(' *  Full-featured node (everything):');
    L.push(' *    #define OPENLCB_COMPILE_EVENTS');
    L.push(' *    #define OPENLCB_COMPILE_DATAGRAMS');
    L.push(' *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
    L.push(' *    #define OPENLCB_COMPILE_FIRMWARE');
    L.push(' *    #define OPENLCB_COMPILE_BROADCAST_TIME');
    L.push(' *    #define OPENLCB_COMPILE_TRAIN');
    L.push(' *    #define OPENLCB_COMPILE_TRAIN_SEARCH');
    L.push(' *');
    L.push(' *  Minimal bootloader (firmware upgrade only):');
    L.push(' *    Use templates/bootloader/openlcb_user_config.h instead');
    L.push(' */');
    L.push('');
    L.push('#ifndef __OPENLCB_USER_CONFIG__');
    L.push('#define __OPENLCB_USER_CONFIG__');
    L.push('');

    /* ---- Feature Flags ---- */
    L.push(_section('Feature Flags -- uncomment to enable optional protocols'));
    L.push('');

    if (isBootloader) {

        /* Bootloader: only the master switch is needed here.
         * openlcb_config.h already defines DATAGRAMS, MEMORY_CONFIGURATION, FIRMWARE
         * and undefines EVENTS, BROADCAST_TIME, TRAIN, TRAIN_SEARCH when this is set. */
        L.push('#define OPENLCB_COMPILE_BOOTLOADER');

    } else {

        L.push(' #define OPENLCB_COMPILE_EVENTS');

        if (hasCfgMem) {
            L.push(' #define OPENLCB_COMPILE_DATAGRAMS');
            L.push(' #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
        } else {
            L.push('// #define OPENLCB_COMPILE_DATAGRAMS');
            L.push('// #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
        }

        if (firmwareOn) {
            L.push(' #define OPENLCB_COMPILE_FIRMWARE');
        } else {
            L.push('// #define OPENLCB_COMPILE_FIRMWARE');
        }

        if (broadcastOn) {
            L.push(' #define OPENLCB_COMPILE_BROADCAST_TIME');
        } else {
            L.push('// #define OPENLCB_COMPILE_BROADCAST_TIME');
        }

        if (isTrain) {
            L.push(' #define OPENLCB_COMPILE_TRAIN');
            L.push(' #define OPENLCB_COMPILE_TRAIN_SEARCH');
        } else {
            L.push('// #define OPENLCB_COMPILE_TRAIN');
            L.push('// #define OPENLCB_COMPILE_TRAIN_SEARCH');
        }

    }

    L.push('');

    /* ---- Debug ---- */
    L.push(_section('Debug -- uncomment to print feature summary during compilation'));
    L.push('');
    L.push('// #define OPENLCB_COMPILE_VERBOSE');
    L.push('');

    /* ---- Core Message Buffer Pool ---- */
    L.push(_section('Core Message Buffer Pool'));
    L.push('// The library uses a pool of message buffers of different sizes.  Tune these');
    L.push('// for your platform\'s available RAM.  The total number of buffers is the sum');
    L.push('// of all four types.  On 8-bit processors the total must not exceed 126.');
    L.push('//');
    L.push('//   BASIC    (16 bytes each)  -- most OpenLCB messages fit in this size');
    L.push('//   DATAGRAM (72 bytes each)  -- datagram protocol messages');
    L.push('//   SNIP     (256 bytes each) -- SNIP replies and Events with Payload');
    L.push('//   STREAM   (USER_DEFINED_STREAM_BUFFER_LEN bytes each) -- stream data transfer (future use)');
    L.push('');
    if (isBootloader) {

        /* Bootloader: fixed minimums — no user controls for these */
        L.push(_def('USER_DEFINED_BASIC_BUFFER_DEPTH',    '8')      + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_DATAGRAM_BUFFER_DEPTH', '2')      + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_SNIP_BUFFER_DEPTH',     '1')      + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_STREAM_BUFFER_DEPTH',   '1')      + '      // must be >= 1; enforced by compiler');
        L.push('// Maximum bytes in a single stream data frame (future use).');
        L.push(_def('USER_DEFINED_STREAM_BUFFER_LEN',     '256')    + '    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined');

    } else {

        L.push(_def('USER_DEFINED_BASIC_BUFFER_DEPTH',    (s.advBasicBuf    !== undefined ? s.advBasicBuf    : 16).toString()) + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_DATAGRAM_BUFFER_DEPTH', Math.max(1, (s.advDatagramBuf !== undefined ? s.advDatagramBuf : (hasCfgMem ? 8 : 4))).toString()) + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_SNIP_BUFFER_DEPTH',     Math.max(1, (s.advSnipBuf     !== undefined ? s.advSnipBuf     : 4)).toString()) + '      // must be >= 1; enforced by compiler');
        L.push(_def('USER_DEFINED_STREAM_BUFFER_DEPTH',   Math.max(1, (s.advStreamBuf   !== undefined ? s.advStreamBuf   : 1)).toString()) + '      // must be >= 1; enforced by compiler');
        L.push('// Maximum bytes in a single stream data frame (future use).');
        L.push(_def('USER_DEFINED_STREAM_BUFFER_LEN',     '256')    + '    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined');

    }

    L.push('');

    /* ---- Virtual Node Allocation ---- */
    L.push(_section('Virtual Node Allocation'));
    L.push('// How many virtual nodes this device can host.  Most simple devices use 1.');
    L.push('// Train command stations may need more (one per locomotive being controlled).');
    L.push('');
    L.push(_def('USER_DEFINED_NODE_BUFFER_DEPTH',     (s.advNodeBuf !== undefined ? s.advNodeBuf : 1).toString()) + '      // must be >= 1; enforced by compiler');
    L.push('');

    /* ---- Events ---- */
    L.push(_section('Events (requires OPENLCB_COMPILE_EVENTS)'));
    L.push('// Maximum number of produced/consumed events per node, and how many event ID');
    L.push('// ranges each node can handle.  Ranges are used by protocols like Train Search');
    L.push('// that work with contiguous blocks of event IDs.');
    /* Bootloader: events not used — minimum safe values to avoid zero-length arrays */
    L.push(_def('USER_DEFINED_PRODUCER_COUNT',       isBootloader ? '1' : (s.producerCount      || 64).toString()) + '     // must be >= 1; enforced by compiler');
    L.push(_def('USER_DEFINED_PRODUCER_RANGE_COUNT', isBootloader ? '1' : (s.producerRangeCount ||  5).toString()) + '     // must be >= 1; enforced by compiler');
    L.push(_def('USER_DEFINED_CONSUMER_COUNT',       isBootloader ? '1' : (s.consumerCount      || 64).toString()) + '     // must be >= 1; enforced by compiler');
    L.push(_def('USER_DEFINED_CONSUMER_RANGE_COUNT', isBootloader ? '1' : (s.consumerRangeCount ||  5).toString()) + '     // must be >= 1; enforced by compiler');
    L.push('');

    /* ---- Memory Configuration ---- */
    L.push(_section('Memory Configuration (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)'));
    L.push('// The two address values tell the SNIP protocol where in your node\'s');
    L.push('// configuration memory space the user-editable name and description strings');
    L.push('// begin.  The standard layout puts the user name at address 0 and the user');
    L.push('// description immediately after at byte 62:');
    L.push('//   63 = LEN_SNIP_USER_NAME_BUFFER (63)');
    L.push('');

    /* ---- Train Protocol ---- */
    L.push(_section('Train Protocol (requires OPENLCB_COMPILE_TRAIN)'));
    L.push('// TRAIN_NODE_COUNT        -- max simultaneous train nodes (often equals');
    L.push('//                            NODE_BUFFER_DEPTH for a dedicated command station)');
    L.push('// MAX_LISTENERS_PER_TRAIN -- max consist members (listener slots) per train');
    L.push('// MAX_TRAIN_FUNCTIONS     -- number of DCC function outputs: 29 = F0 through F28');
    L.push('');
    /* Bootloader: train not used — minimum safe values to avoid zero-length arrays */
    L.push(_def('USER_DEFINED_TRAIN_NODE_COUNT',         isBootloader ? '1' : isTrain ? Math.max(1, (s.advTrainNodeCount  !== undefined ? s.advTrainNodeCount  : 4)).toString() : '4') + '      // must be >= 1; enforced by compiler');
    L.push(_def('USER_DEFINED_MAX_LISTENERS_PER_TRAIN',  isBootloader ? '1' : isTrain ? Math.max(1, (s.advTrainListeners !== undefined ? s.advTrainListeners : 6)).toString() : '6') + '      // must be >= 1; enforced by compiler');
    L.push(_def('USER_DEFINED_MAX_TRAIN_FUNCTIONS',      isBootloader ? '1' : isTrain ? Math.max(1, (s.advTrainFunctions !== undefined ? s.advTrainFunctions : 29)).toString() : '29') + '      // must be >= 1; enforced by compiler');
    L.push('');

    /* ---- Listener Alias Verification ---- */
    L.push(_section('Listener Alias Verification (requires OPENLCB_COMPILE_TRAIN)'));
    L.push('// LISTENER_PROBE_TICK_INTERVAL  -- how many 100ms ticks between prober calls');
    L.push('//                                  (1 = every 100ms, 2 = every 200ms, etc.)');
    L.push('// LISTENER_PROBE_INTERVAL_TICKS -- 100ms ticks between probes of the SAME entry');
    L.push('//                                  (250 = 25 seconds)');
    L.push('// LISTENER_VERIFY_TIMEOUT_TICKS -- 100ms ticks to wait for AMD reply before');
    L.push('//                                  declaring stale (30 = 3 seconds)');
    L.push('');
    L.push(_def('USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL',  (s.advProbeTick     !== undefined ? s.advProbeTick     : 1).toString()));
    L.push(_def('USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS', (s.advProbeInterval !== undefined ? s.advProbeInterval : 250).toString()));
    L.push(_def('USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS', (s.advVerifyTimeout !== undefined ? s.advVerifyTimeout : 30).toString()));
    L.push('');

    /* ---- Forward declaration ---- */
    L.push(_section('Application-defined node parameters (forward-declared to avoid circular include)'));
    L.push('');
    L.push('#ifdef __cplusplus');
    L.push('extern "C" {');
    L.push('#endif');
    L.push('');
    L.push('extern const struct node_parameters_TAG OpenLcbUserConfig_node_parameters;');
    L.push('');
    L.push('#ifdef __cplusplus');
    L.push('}');
    L.push('#endif');
    L.push('');
    L.push('#endif /* __OPENLCB_USER_CONFIG__ */');

    return L.join('\n');

}

/* ---------- generateCanH ---------- */

function generateCanH(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated file */';
    }

    var L = [];

    var _author = s.projectAuthor || '<YOUR NAME OR COMPANY>';
    var _year   = new Date().getFullYear();

    L.push('/** \\copyright');
    L.push(' *   Copyright (c) ' + _year + ', ' + _author);
    L.push(' *   All rights reserved.');
    L.push(' *');
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard \u2014 https://github.com/JimKueneman/OpenLcbCLib');
    L.push(' *');
    L.push(' * @file can_user_config.h');
    L.push(' *  @brief User-editable CAN bus driver configuration for OpenLcbCLib');
    L.push(' *');
    L.push(' *  OPTIONAL: If this file is not present the CAN driver will use built-in');
    L.push(' *  defaults.  Copy this file to your project\'s include path and edit to');
    L.push(' *  override any value.');
    L.push(' *');
    L.push(' *  All values use #ifndef guards in can_types.h so defining them here (or');
    L.push(' *  via -D compiler flags) takes priority over the library defaults.');
    L.push(' */');
    L.push('');
    L.push('#ifndef __CAN_USER_CONFIG__');
    L.push('#define __CAN_USER_CONFIG__');
    L.push('');

    L.push(_section('CAN Message Buffer Pool'));
    L.push('// Number of raw CAN message buffers in the driver pool.  Each buffer holds one');
    L.push('// CAN 2.0 frame (8 data bytes + header).  Tune for your platform\'s available');
    L.push('// RAM and expected bus traffic.');
    L.push('//');
    L.push('// Maximum value is 254 (0xFE).');
    L.push('');
    L.push(_def('USER_DEFINED_CAN_MSG_BUFFER_DEPTH', (s.advCanBuf !== undefined ? Math.max(1, s.advCanBuf) : 20).toString()) + '     // must be >= 1; enforced by compiler');
    L.push('');

    L.push('#endif /* __CAN_USER_CONFIG__ */');

    return L.join('\n');

}

/* ---------- generateC ---------- */

function generateC(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated .c file */';
    }

    _ensureBytes(s);

    var isTrainRole  = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrain      = isTrainRole;   /* alias: both roles enable train address spaces */
    var isTrainNode  = s.nodeType === 'train';   /* locomotive only — has FDI */
    var isBasic      = s.nodeType === 'basic';
    var isBootloader = s.nodeType === 'bootloader';
    var hasCfgMem    = !isBasic && !isBootloader;
    var broadcastOn  = s.broadcast !== 'none';
    var firmwareOn   = isBootloader ? true : (s.firmware && hasCfgMem);
    var unalignedR   = hasCfgMem && s.unalignedReads;
    var unalignedW   = hasCfgMem && s.unalignedWrites;
    var cfgHighest   = hasCfgMem ? (s.configMemHighest || '0x200') : '0';
    var cfgHighestNum = parseInt(cfgHighest) || 0;
    var cdiRequired   = hasCfgMem ? _cdiHighestAddrInSpace(s.cdiXml, 253) : -1;
    var nodeLabel    = s.nodeType === 'train-controller' ? 'Train Controller'
                     : s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var cdiHighest = (s.cdiLength > 0) ? (s.cdiLength - 1) : 0;
    var fdiHighest = (s.fdiLength > 0) ? (s.fdiLength - 1) : 0;

    var L = [];

    /* ---- File header ---- */
    var _author = s.projectAuthor || '<YOUR NAME OR COMPANY>';
    var _year   = new Date().getFullYear();

    L.push('/**');
    L.push(' * @file openlcb_user_config.c');
    L.push(' * @brief Const node_parameters_t struct that defines SNIP strings, protocol');
    L.push(' *        support bits, CDI/FDI data, and address space layout for a');
    L.push(' *        ' + nodeLabel + ' node.');
    L.push(' *');
    L.push(' * @copyright');
    L.push(' *   Copyright (c) ' + _year + ', ' + _author);
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard — https://github.com/JimKueneman/OpenLcbCLib');
    L.push(' */');
    L.push('');
    L.push('#include "openlcb_user_config.h"');
    L.push('#include "src/openlcb/openlcb_types.h"');
    L.push('#include "src/openlcb/openlcb_defines.h"');
    L.push('');
    L.push('');
    /* ---- standalone CDI/FDI byte arrays (before the struct so sizeof works) ---- */
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('static const uint8_t _cdi_data[] =');
        L.push('    ' + _byteArrayStr(s.cdiBytes, s.cdiXml || s.cdiUserXml, !!s.preserveWhitespace, '        ') + ';');
        L.push('');
    }
    if (isTrainNode && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('static const uint8_t _fdi_data[] =');
        L.push('    ' + _byteArrayStr(s.fdiBytes, s.fdiXml || s.fdiUserXml, !!s.preserveWhitespace, '        ') + ';');
        L.push('');
    }

    L.push('const node_parameters_t OpenLcbUserConfig_node_parameters = {');
    L.push('');

    /* ======================================================================
     * Field order matches node_parameters_t struct declaration in
     * openlcb_types.h — required by Arduino/AVR GCC in C++ mode.
     *
     * Struct order:
     *   1. snip                                    (user_snip_struct_t)
     *   2. protocol_support                        (uint64_t)
     *   3. consumer_count_autocreate               (uint8_t)
     *   4. producer_count_autocreate               (uint8_t)
     *   5. cdi[]                                   (uint8_t[])
     *   6. fdi[]                                   (uint8_t[])
     *   7. address_space_configuration_definition  (0xFF)
     *   8. address_space_all                       (0xFE)
     *   9. address_space_config_memory             (0xFD)
     *  10. address_space_acdi_manufacturer         (0xFC)
     *  11. address_space_acdi_user                 (0xFB)
     *  12. address_space_train_function_definition_info (0xFA)
     *  13. address_space_train_function_config_memory   (0xF9)
     *  14. configuration_options
     *  15. address_space_firmware                  (0xEF)
     *
     * Within each user_address_space_info_t:
     *   present, read_only, low_address_valid, address_space,
     *   highest_address, low_address, description
     *
     * Within user_configuration_options:
     *   write_under_mask_supported, unaligned_reads_supported,
     *   unaligned_writes_supported, read_from_manufacturer_space_0xfc_supported,
     *   read_from_user_space_0xfb_supported, write_to_user_space_0xfb_supported,
     *   stream_read_write_supported, high_address_space, low_address_space,
     *   description
     * ====================================================================== */

    /* ---- 1. .snip (user_snip_struct_t) ---- */
    var cdiIdent = hasCfgMem ? _extractCdiIdentification(s.cdiXml) : null;
    var snipName, snipModel, snipHw, snipSw;

    if (isBootloader) {

        snipName  = s.snipName  || 'OpenLCB Bootloader';
        snipModel = s.snipModel || 'Firmware Upgrade';
        snipHw    = s.snipHw    || '1.0';
        snipSw    = s.snipSw    || '0.1.0';

    } else if (isBasic && s.snipEnabled) {

        snipName  = s.snipName  || 'My Company';
        snipModel = s.snipModel || 'Signal Controller v2';
        snipHw    = s.snipHw    || '1.0';
        snipSw    = s.snipSw    || '0.1.0';

    } else if (hasCfgMem && cdiIdent) {

        snipName  = cdiIdent.manufacturer    || '';
        snipModel = cdiIdent.model           || '';
        snipHw    = cdiIdent.hardwareVersion || '';
        snipSw    = cdiIdent.softwareVersion || '';

    } else {

        snipName  = '';
        snipModel = '';
        snipHw    = '';
        snipSw    = '';

    }

    var snipSource = isBootloader                    ? 'from wizard SNIP fields'
                   : (isBasic && s.snipEnabled)      ? 'from wizard SNIP fields'
                   : (hasCfgMem && cdiIdent)         ? 'from CDI <identification>'
                   :                                   '';

    L.push('    .snip.mfg_version = 4,  // SNIP protocol version — fixed by spec');
    L.push('    .snip.name = "' + _escC(snipName) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.model = "' + _escC(snipModel) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.hardware_version = "' + _escC(snipHw) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.software_version = "' + _escC(snipSw) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.user_version = 2,  // SNIP user-data version — fixed by spec');
    L.push('');

    /* ---- 2. .protocol_support ---- */
    var psi = [];

    if (isBootloader) {

        /* Bootloader PSI: minimal set — no events, no CDI, no train */
        psi.push('PSI_SIMPLE_NODE_INFORMATION');
        psi.push('PSI_IDENTIFICATION');
        psi.push('PSI_DATAGRAM');
        psi.push('PSI_MEMORY_CONFIGURATION');
        psi.push('PSI_FIRMWARE_UPGRADE');

    } else {

        if (hasCfgMem) {
            psi.push('PSI_DATAGRAM');
            psi.push('PSI_MEMORY_CONFIGURATION');
        }

        psi.push('PSI_EVENT_EXCHANGE');
        psi.push('PSI_SIMPLE_NODE_INFORMATION');

        if (hasCfgMem) {
            psi.push('PSI_ABBREVIATED_DEFAULT_CDI');
            psi.push('PSI_CONFIGURATION_DESCRIPTION_INFO');
        }

        if (isTrainNode) {
            psi.push('PSI_TRAIN_CONTROL');
            psi.push('PSI_FUNCTION_DESCRIPTION');
        }

        if (firmwareOn) {
            psi.push('PSI_FIRMWARE_UPGRADE');
        }

    }

    if (psi.length === 1) {

        L.push('    .protocol_support = ' + psi[0] + ',');

    } else {

        L.push('    .protocol_support = (');

        for (var pi = 0; pi < psi.length; pi++) {
            var isLast = (pi === psi.length - 1);
            L.push('    ' + psi[pi] + (isLast ? '' : ' |'));
        }

        L.push('    ),');

    }

    L.push('');

    /* ---- 3–4. .consumer_count_autocreate / .producer_count_autocreate ---- */
    L.push('    // For internal testing only do not set to anything but 0');
    L.push('    .consumer_count_autocreate = 0,');
    L.push('    .producer_count_autocreate = 0,');
    L.push('');

    /* ---- 5. .configuration_options (user_configuration_options) ---- */
    var lowAddrSpace;

    if (isBootloader) {
        lowAddrSpace = 'CONFIG_MEM_SPACE_FIRMWARE';
    } else if (isBasic) {
        lowAddrSpace = '0x00';
    } else if (firmwareOn) {
        lowAddrSpace = 'CONFIG_MEM_SPACE_FIRMWARE';
    } else if (isTrainNode) {
        lowAddrSpace = 'CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG';
    } else {
        lowAddrSpace = 'CONFIG_MEM_SPACE_ACDI_USER_ACCESS';
    }

    L.push('    // 5. configuration_options');

    if (isBootloader) {

        /* Bootloader: all advanced options off; only firmware space accessible */
        L.push('    .configuration_options.high_address_space = CONFIG_MEM_SPACE_FIRMWARE,  // only firmware space');
        L.push('    .configuration_options.low_address_space = CONFIG_MEM_SPACE_FIRMWARE,   // only firmware space');
        L.push('    .configuration_options.read_from_manufacturer_space_0xfc_supported = false,  // not present in bootloader');
        L.push('    .configuration_options.read_from_user_space_0xfb_supported = false,          // not present in bootloader');
        L.push('    .configuration_options.stream_read_write_supported = false,');
        L.push('    .configuration_options.unaligned_reads_supported = false,   // not supported in bootloader');
        L.push('    .configuration_options.unaligned_writes_supported = false,  // not supported in bootloader');
        L.push('    .configuration_options.write_to_user_space_0xfb_supported = false,           // not present in bootloader');
        L.push('    .configuration_options.write_under_mask_supported = false,  // not supported in bootloader');
        L.push('    .configuration_options.description = "",');

    } else {

        L.push('    .configuration_options.high_address_space = ' + (hasCfgMem ? 'CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO' : '0x00') + ',  // derived from enabled features');
        L.push('    .configuration_options.low_address_space = ' + lowAddrSpace + ',  // derived from enabled features');
        L.push('    .configuration_options.read_from_manufacturer_space_0xfc_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
        L.push('    .configuration_options.read_from_user_space_0xfb_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
        L.push('    .configuration_options.stream_read_write_supported = false,  // stream transport not implemented');
        L.push('    .configuration_options.unaligned_reads_supported = ' + (unalignedR ? 'true' : 'false') + ',  // from wizard Memory Configuration option');
        L.push('    .configuration_options.unaligned_writes_supported = ' + (unalignedW ? 'true' : 'false') + ',  // from wizard Memory Configuration option');
        L.push('    .configuration_options.write_to_user_space_0xfb_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
        L.push('    .configuration_options.write_under_mask_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
        L.push('    .configuration_options.description = "",');

    }
    L.push('');

    /* ---- 6. Space 0xFF — CDI (address_space_configuration_definition) ---- */
    L.push('    // Space 0xFF — CDI (Configuration Description Information)');
    if (!isBootloader) {
    L.push('    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description)');
    L.push('    //    of the Config Memory System so make sure the CDI maps these 2 items to the first');
    L.push('    //    128 bytes as well');
    }
    L.push('    .address_space_configuration_definition.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_configuration_definition.read_only = true,  // CDI is always read-only per spec');
    L.push('    .address_space_configuration_definition.low_address_valid = false,');
    L.push('    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,');
    if (isBootloader) {
        L.push('    .address_space_configuration_definition.highest_address = 0,  // not present in bootloader');
    } else if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('    .address_space_configuration_definition.highest_address = sizeof(_cdi_data) - 1,  // auto-computed from CDI byte array');
    } else {
        L.push('    .address_space_configuration_definition.highest_address = 0,');
    }
    L.push('    .address_space_configuration_definition.low_address = 0,');
    L.push('    .address_space_configuration_definition.description = "",');
    L.push('');

    /* ---- 8. Space 0xFE — All Memory (address_space_all) ---- */
    L.push('    // Space 0xFE — All Memory (raw debug access)');
    L.push('    .address_space_all.present = false,  // not implemented — reserved by spec');
    L.push('    .address_space_all.read_only = true,  // not implemented — reserved by spec');
    L.push('    .address_space_all.low_address_valid = false,');
    L.push('    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,');
    L.push('    .address_space_all.highest_address = ' + (isBootloader ? '0' : '0xFFFFFFFF') + ',');
    L.push('    .address_space_all.low_address = 0,');
    L.push('    .address_space_all.description = "",');
    L.push('');

    /* ---- 9. Space 0xFD — Config Memory (address_space_config_memory) ---- */
    var cfgComment = '';

    if (!isBootloader) {
        if (cdiRequired > 0 && cfgHighestNum > 0 && cdiRequired > cfgHighestNum) {
            cfgComment = '  // WARNING: CDI defines ' + cdiRequired + ' bytes (0x' + cdiRequired.toString(16).toUpperCase() + ') but this is only ' + cfgHighestNum + ' (0x' + cfgHighestNum.toString(16).toUpperCase() + ') — INCREASE THIS VALUE';
        } else if (cdiRequired > 0) {
            cfgComment = '  // from wizard — CDI needs ' + cdiRequired + ' bytes (0x' + cdiRequired.toString(16).toUpperCase() + '), OK';
        } else {
            cfgComment = '  // from wizard "Config memory size" field';
        }
    }

    L.push('    // Space 0xFD — Configuration Memory (user-configurable settings)');
    L.push('    .address_space_config_memory.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_config_memory.read_only = ' + (isBootloader ? 'true' : 'false') + ',');
    L.push('    .address_space_config_memory.low_address_valid = false,');
    L.push('    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,');
    L.push('    .address_space_config_memory.highest_address = ' + (isBootloader ? '0' : cfgHighest) + ',' + cfgComment);
    L.push('    .address_space_config_memory.low_address = 0,');
    L.push('    .address_space_config_memory.description = "",');
    L.push('');

    /* ---- 10. Space 0xFC — ACDI Manufacturer (address_space_acdi_manufacturer) ---- */
    L.push('    // Space 0xFC — ACDI Manufacturer (read-only identification)');
    L.push('    .address_space_acdi_manufacturer.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_acdi_manufacturer.read_only = true,');
    L.push('    .address_space_acdi_manufacturer.low_address_valid = false,');
    L.push('    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,');
    L.push('    .address_space_acdi_manufacturer.highest_address = ' + (isBootloader ? '0' : '124') + ',' + (isBootloader ? '  // not present in bootloader' : '  // Zero indexed: 1 + 41 + 41 + 21 + 21 = 125 bytes (0-124)'));
    L.push('    .address_space_acdi_manufacturer.low_address = 0,');
    L.push('    .address_space_acdi_manufacturer.description = "",');
    L.push('');

    /* ---- 11. Space 0xFB — ACDI User (address_space_acdi_user) ---- */
    L.push('    // Space 0xFB — ACDI User (user name & description)');
    L.push('    .address_space_acdi_user.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_acdi_user.read_only = ' + (isBootloader ? 'true' : 'false') + ',');
    L.push('    .address_space_acdi_user.low_address_valid = false,');
    L.push('    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,');
    L.push('    .address_space_acdi_user.highest_address = ' + (isBootloader ? '0' : '127') + ',' + (isBootloader ? '  // not present in bootloader' : '  // Zero indexed: 1 + 63 + 64 = 128 bytes (0-127)'));
    L.push('    .address_space_acdi_user.low_address = 0,');
    L.push('    .address_space_acdi_user.description = "",');
    L.push('');

    /* ---- 12. Space 0xFA — Train FDI (address_space_train_function_definition_info) ---- */
    L.push('    // Space 0xFA — Train FDI (Function Description Information)');
    L.push('    .address_space_train_function_definition_info.present = ' + (isTrainNode ? 'true' : 'false') + ',  // auto-set: train (locomotive) node type only');
    L.push('    .address_space_train_function_definition_info.read_only = true,  // FDI is always read-only per spec');
    L.push('    .address_space_train_function_definition_info.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_train_function_definition_info.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,');
    if (isTrainNode && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('    .address_space_train_function_definition_info.highest_address = sizeof(_fdi_data) - 1,  // auto-computed from FDI byte array');
    } else {
        L.push('    .address_space_train_function_definition_info.highest_address = 0,');
    }
    L.push('    .address_space_train_function_definition_info.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_train_function_definition_info.description = "",');
    L.push('');

    /* ---- 13. Space 0xF9 — Train Function Config (address_space_train_function_config_memory) ---- */
    L.push('    // Space 0xF9 — Train Function Configuration Memory');
    L.push('    .address_space_train_function_config_memory.present = ' + (isTrainNode ? 'true' : 'false') + ',  // auto-set: train (locomotive) node type only');
    L.push('    .address_space_train_function_config_memory.read_only = false,  // function config is read/write');
    L.push('    .address_space_train_function_config_memory.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_train_function_config_memory.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG,');
    L.push('    .address_space_train_function_config_memory.highest_address = 0,  // library calculates from train function count at runtime');
    L.push('    .address_space_train_function_config_memory.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_train_function_config_memory.description = "",');
    L.push('');

    /* ---- 15. Space 0xEF — Firmware (address_space_firmware) ---- */
    L.push('    // Space 0xEF — Firmware Upgrade');
    L.push('    .address_space_firmware.present = ' + (firmwareOn ? 'true' : 'false') + ',  // from wizard Firmware Update option');
    L.push('    .address_space_firmware.read_only = false,  // firmware space accepts writes for flashing');
    L.push('    .address_space_firmware.low_address_valid = false,  // Firmware ALWAYS assumes it starts at 0');
    L.push('    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,');
    L.push('    .address_space_firmware.highest_address = 0xFFFFFFFF,  // full range — firmware handler manages boundaries');
    L.push('    .address_space_firmware.low_address = 0,  // Firmware ALWAYS assumes it starts at 0');
    L.push('    .address_space_firmware.description = "",');
    L.push('');

    /* ---- 14. .cdi ---- */
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('    .cdi = _cdi_data,');
    } else {
        L.push('    .cdi = NULL,  // no CDI');
    }

    L.push('');

    /* ---- 15. .fdi ---- */
    if (isTrainNode && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('    .fdi = _fdi_data,');
    } else {
        L.push('    .fdi = NULL,  // no FDI');
    }

    L.push('');
    L.push('};');

    return L.join('\n');

}

/* ---------- generateMain ---------- */

/*
 * Generates main.c with:
 *   - #include headers for checked drivers/callbacks
 *   - can_config_t wiring struct
 *   - openlcb_config_t wiring struct
 *   - main() with init calls and run loop skeleton
 *
 * State additions:
 *   s.driverState    — { 'can-drivers': { checked: [...] }, 'olcb-drivers': { checked: [...] } }
 *   s.callbackState  — { 'cb-events': { checked: [...] }, 'cb-config-mem': { checked: [...] }, ... }
 */
function generateMain(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated main.c file */';
    }

    var isTrainRole  = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrainNode  = s.nodeType === 'train';
    var isTrain      = isTrainRole;
    var isBasic      = s.nodeType === 'basic';
    var isBootloader = s.nodeType === 'bootloader';
    var hasCfgMem    = !isBasic && !isBootloader;
    var broadcastOn  = s.broadcast !== 'none';
    var firmwareOn   = isBootloader ? true : (s.firmware && hasCfgMem);
    var nodeLabel    = s.nodeType === 'train-controller' ? 'Train Controller'
                     : s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    /* Resolve which driver/callback functions are checked */
    var canDriverChecked  = _getCheckedDriverFns('can-drivers', s);
    var olcbDriverChecked = _getCheckedDriverFns('olcb-drivers', s);
    var cbCanChecked      = _getCheckedCallbackFns('cb-can', s);
    var cbOlcbChecked     = _getCheckedCallbackFns('cb-olcb', s);
    var cbEventsChecked   = _getCheckedCallbackFns('cb-events', s);
    var cbConfigChecked   = _getCheckedCallbackFns('cb-config-mem', s);
    var cbTrainChecked    = _getCheckedCallbackFns('cb-train', s);
    var cbBcastChecked    = _getCheckedCallbackFns('cb-bcast-time', s);

    var _author = s.projectAuthor || '<YOUR NAME OR COMPANY>';
    var _year   = new Date().getFullYear();
    var hasNodeId = _nodeIdToHex(s.projectNodeId);

    var L = [];

    /* ---- File header with cliff notes ---- */
    L.push('/**');
    L.push(' * @file main.c');
    L.push(' * @brief Application entry point for a ' + nodeLabel + ' node.');
    L.push(' *        Wires driver and callback functions into the OpenLcbCLib config');
    L.push(' *        structs, sets the Node ID, and runs the main protocol loop.');
    L.push(' *');
    L.push(' * @copyright');
    L.push(' *   Copyright (c) ' + _year + ', ' + _author);
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard — https://github.com/JimKueneman/OpenLcbCLib');
    L.push(' *');
    L.push(' * QUICK START');
    L.push(' * ===========');
    L.push(' *  1. Copy the OpenLcbCLib library source folders (openlcb/,');
    L.push(' *     drivers/, utilities/) into the openlcb_c_lib/ subfolder.');
    L.push(' *  2. Implement the driver stubs in application_drivers/.');
    L.push(' *     At minimum: CAN transmit/receive, resources lock/unlock,');
    L.push(' *     reboot, and the 100ms timer are critical for basic operation.');
    L.push(' *  3. Implement any callback stubs in application_callbacks/.');
    if (hasNodeId) {
    L.push(' *  4. Build and flash. The main loop calls OpenLcb_run() which');
    L.push(' *     handles all protocol processing automatically.');
    } else {
    L.push(' *  4. Set your unique Node ID in the NODE_ID define below.');
    L.push(' *  5. Build and flash. The main loop calls OpenLcb_run() which');
    L.push(' *     handles all protocol processing automatically.');
    }
    L.push(' *');
    L.push(' *  Search for "TODO" in all generated files to find what needs');
    L.push(' *  your attention. See GETTING_STARTED.txt for the full guide.');
    L.push(' */');
    L.push('');

    /* ---- #includes ---- */
    L.push('#include "openlcb_user_config.h"');
    L.push('#include "src/drivers/canbus/can_config.h"');
    L.push('#include "src/openlcb/openlcb_config.h"');
    L.push('');

    /* Driver headers — always needed for required DI functions */
    L.push('#include "openlcb_can_drivers.h"');
    L.push('#include "openlcb_drivers.h"');

    /* Callback headers */
    if (cbCanChecked.length > 0) {
        L.push('#include "callbacks_can.h"');
    }
    if (cbOlcbChecked.length > 0) {
        L.push('#include "callbacks_olcb.h"');
    }
    if (cbEventsChecked.length > 0) {
        L.push('#include "callbacks_events.h"');
    }
    if (cbConfigChecked.length > 0) {
        L.push('#include "callbacks_config_mem.h"');
    }
    if (cbTrainChecked.length > 0) {
        L.push('#include "callbacks_train.h"');
    }
    if (cbBcastChecked.length > 0) {
        L.push('#include "callbacks_broadcast_time.h"');
    }

    if (broadcastOn) {
        L.push('#include "src/openlcb/openlcb_application_broadcast_time.h"');
    }

    if (isTrainRole) {
        L.push('#include "src/openlcb/openlcb_application_train.h"');
    }

    L.push('#include "src/openlcb/openlcb_application.h"');
    L.push('#include "src/openlcb/openlcb_defines.h"');
    L.push('');

    /* ---- Platform info comment block ---- */
    var plat = s.platformState;
    if (plat && plat.platform && plat.platform !== 'none') {

        L.push('/*');
        L.push(' * Target platform: ' + (plat.platform || 'unknown'));
        if (plat.framework) {
            L.push(' * Framework: ' + plat.framework);
        }
        if (plat.libraries && plat.libraries.length > 0) {
            L.push(' * Required libraries: ' + plat.libraries.join(', '));
        }
        if (plat.notes && plat.notes.length > 0) {
            for (var ni = 0; ni < plat.notes.length; ni++) {
                L.push(' * NOTE: ' + plat.notes[ni]);
            }
        }
        L.push(' */');
        L.push('');

    }

    /* ---- Node ID ---- */
    var nodeIdHex = _nodeIdToHex(s.projectNodeId);
    if (nodeIdHex) {
        L.push('#define NODE_ID    ' + nodeIdHex);
    } else {
        L.push('#error "Set your unique Node ID in Project Options (Node Wizard) or replace the value below"');
        L.push('#define NODE_ID    0xXXXXXXXXXXXX    // TODO: Replace with your unique Node ID');
    }
    L.push('');
    L.push('openlcb_node_t *node = NULL;');
    L.push('');

    /* ---- can_config_t ---- */
    L.push(_section('CAN Transport Configuration'));
    L.push('');
    L.push('static const can_config_t can_config = {');
    L.push('');

    /* can_config_t fields — struct order: transmit_raw_can_frame, is_tx_buffer_clear,
     * lock_shared_resources, unlock_shared_resources, on_rx, on_tx, on_alias_change
     *
     * REQUIRED fields are always wired — NULL would crash the library. */

    L.push('    // REQUIRED — library will crash if NULL');
    L.push('    .transmit_raw_can_frame  = &CanDriver_transmit_raw_can_frame,');
    L.push('    .is_tx_buffer_clear      = &CanDriver_is_can_tx_buffer_clear,');
    L.push('    .lock_shared_resources   = &Drivers_lock_shared_resources,');
    L.push('    .unlock_shared_resources = &Drivers_unlock_shared_resources,');
    L.push('');
    /* Optional CAN callbacks */
    var canOnRx    = _findCallbackFnByConfigField(cbCanChecked, 'on_rx');
    var canOnTx    = _findCallbackFnByConfigField(cbCanChecked, 'on_tx');
    var canOnAlias = _findCallbackFnByConfigField(cbCanChecked, 'on_alias_change');

    L.push('    // Optional');
    L.push('    .on_rx                   = ' + (canOnRx    ? '&CallbacksCan_on_rx'           : 'NULL') + ',');
    L.push('    .on_tx                   = ' + (canOnTx    ? '&CallbacksCan_on_tx'           : 'NULL') + ',');
    L.push('    .on_alias_change         = ' + (canOnAlias ? '&CallbacksCan_on_alias_change' : 'NULL') + ',');

    L.push('');
    L.push('};');
    L.push('');

    /* ---- openlcb_config_t ---- */
    L.push(_section('OpenLCB Stack Configuration'));
    L.push('');
    L.push('static const openlcb_config_t openlcb_config = {');
    L.push('');

    /* -- Required hardware drivers — always wired, NULL would crash -- */
    L.push('    // REQUIRED — library will crash if NULL');
    L.push('    .lock_shared_resources   = &Drivers_lock_shared_resources,');
    L.push('    .unlock_shared_resources = &Drivers_unlock_shared_resources,');

    if (hasCfgMem) {

        L.push('    .config_mem_read         = &Drivers_config_mem_read,');
        L.push('    .config_mem_write        = &Drivers_config_mem_write,');
        L.push('    .reboot                  = &Drivers_reboot,');

    } else if (isBootloader) {

        /* Bootloader: firmware writes go through .firmware_write — no config_mem stubs needed */
        L.push('    .reboot                  = &Drivers_reboot,');

    }

    L.push('');

    /* -- Memory Configuration extensions (conditional) -- */
    if (hasCfgMem) {

        L.push('    // Memory Configuration extensions');

        var factoryResetFn = _findCallbackFnByConfigField(cbConfigChecked, 'factory_reset');
        L.push('    .factory_reset                       = ' + (factoryResetFn ? '&CallbacksConfigMem_factory_reset' : 'NULL') + ',');

        var readDelayFn = _findCallbackFnByConfigField(cbConfigChecked, 'config_mem_read_delayed_reply_time');
        L.push('    .config_mem_read_delayed_reply_time  = ' + (readDelayFn ? '&CallbacksConfigMem_config_mem_read_delayed_reply_time' : 'NULL') + ',');

        var writeDelayFn = _findCallbackFnByConfigField(cbConfigChecked, 'config_mem_write_delayed_reply_time');
        L.push('    .config_mem_write_delayed_reply_time = ' + (writeDelayFn ? '&CallbacksConfigMem_config_mem_write_delayed_reply_time' : 'NULL') + ',');
        L.push('');

    }

    /* -- Firmware extensions (conditional) -- */
    if (firmwareOn) {

        L.push('    // Firmware upgrade — REQUIRED when Firmware enabled');
        L.push('    .freeze         = &Drivers_freeze,');
        L.push('    .unfreeze       = &Drivers_unfreeze,');
        L.push('    .firmware_write = &Drivers_firmware_write,');
        L.push('');

    }

    /* -- Core application callbacks (from cb-olcb group) -- */
    L.push('    // Core application callbacks');

    var oirFn       = _findCallbackFnByConfigField(cbOlcbChecked, 'on_optional_interaction_rejected');
    var tdeFn       = _findCallbackFnByConfigField(cbOlcbChecked, 'on_terminate_due_to_error');
    var timerFn     = _findCallbackFnByConfigField(cbOlcbChecked, 'on_100ms_timer');
    var loginFn     = _findCallbackFnByConfigField(cbOlcbChecked, 'on_login_complete');
    L.push('    .on_optional_interaction_rejected = ' + (oirFn   ? '&CallbacksOlcb_on_optional_interaction_rejected' : 'NULL') + ',');
    L.push('    .on_terminate_due_to_error        = ' + (tdeFn   ? '&CallbacksOlcb_on_terminate_due_to_error'        : 'NULL') + ',');
    L.push('    .on_100ms_timer                   = ' + (timerFn ? '&CallbacksOlcb_on_100ms_timer'                   : 'NULL') + ',');
    L.push('    .on_login_complete                = ' + (loginFn ? '&CallbacksOlcb_on_login_complete'                : 'NULL') + ',');
    L.push('');

    /* -- Event callbacks (conditional) -- */
    if (cbEventsChecked.length > 0) {

        L.push('    // Event transport callbacks');
        _emitCallbackWiring(L, cbEventsChecked, 'CallbacksEvents');
        L.push('');

    }

    /* -- Broadcast Time callbacks (conditional) -- */
    if (broadcastOn && cbBcastChecked.length > 0) {

        L.push('    // Broadcast Time callbacks');
        _emitCallbackWiring(L, cbBcastChecked, 'CallbacksBroadcastTime');
        L.push('');

    }

    /* -- Train callbacks (conditional) -- */
    if (isTrainRole && cbTrainChecked.length > 0) {

        L.push('    // Train callbacks');
        _emitCallbackWiring(L, cbTrainChecked, 'CallbacksTrain');
        L.push('');

    }

    L.push('};');
    L.push('');

    /* ---- Event registration functions ---- */
    /* Bootloader has no event transport — skip producer/consumer registration */
    if (!isBootloader) {

    L.push(_section('Application Event Registration'));
    L.push('');

    /* Build a lookup map from event id to WELL_KNOWN_EVENTS entry */
    var _wkeMap = {};
    if (typeof WELL_KNOWN_EVENTS !== 'undefined') {
        WELL_KNOWN_EVENTS.forEach(function (evt) { _wkeMap[evt.id] = evt; });
    }

    var wke = s.wellKnownEvents || { producers: [], consumers: [] };

    /* ---- Producers ---- */
    L.push('void _register_app_producers(openlcb_node_t *openlcb_node) {');
    L.push('');

    var prodCount = 0;

    /* Node-type defaults */
    if (s.nodeType === 'train') {

        L.push('    // Train node produces "Is Train" to identify itself on the network');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_TRAIN, EVENT_STATUS_SET);');
        prodCount++;

    } else if (s.nodeType === 'train-controller') {

        L.push('    // Throttle produces emergency events to stop/resume all trains on the network');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_UNKNOWN);');
        prodCount += 4;

    }

    /* Broadcast Time — event ranges are registered internally by the setup call */
    if (broadcastOn) {

        if (prodCount > 0) { L.push(''); }
        if (s.broadcast === 'producer') {
            L.push('    // Broadcast Time Producer: 2 producer ranges + 2 consumer ranges');
            L.push('    // registered by OpenLcbApplicationBroadcastTime_setup_producer() in main()');
        } else {
            L.push('    // Broadcast Time Consumer: 2 producer ranges + 2 consumer ranges');
            L.push('    // registered by OpenLcbApplicationBroadcastTime_setup_consumer() in main()');
        }
        prodCount++;

    }

    /* User-selected well known producers */
    if (wke.producers.length > 0) {

        if (prodCount > 0) { L.push(''); }
        L.push('    // Well Known Event producers (selected in Node Wizard)');

        wke.producers.forEach(function (id) {
            var evt = _wkeMap[id];
            if (!evt) { return; }
            if (evt.range) {
                L.push('    OpenLcbApplication_register_producer_range(openlcb_node, ' + evt.define + ', EVENT_RANGE_COUNT_1);');
            } else {
                L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, ' + evt.define + ', EVENT_STATUS_UNKNOWN);');
            }
        });

    }

    if (prodCount === 0 && wke.producers.length === 0) {

        L.push('    // TODO: Register application-specific produced events');
        L.push('    // OpenLcbApplication_register_producer_eventid(openlcb_node, your_event_id, EVENT_STATUS_UNKNOWN);');

    }

    L.push('');
    L.push('}');
    L.push('');

    /* ---- Consumers ---- */
    L.push('void _register_app_consumers(openlcb_node_t *openlcb_node) {');
    L.push('');

    var consCount = 0;

    /* Node-type defaults */
    if (s.nodeType === 'train') {

        L.push('    // Train node consumes global emergency events');
        L.push('    // NOTE: OpenLcbApplicationTrain_setup() also registers these — shown here for reference');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_SET);');
        consCount += 4;

    }

    /* Broadcast Time — event ranges are registered internally by the setup call */
    if (broadcastOn) {

        if (consCount > 0) { L.push(''); }
        if (s.broadcast === 'consumer') {
            L.push('    // Broadcast Time Consumer: 2 consumer ranges + 2 producer ranges');
            L.push('    // registered by OpenLcbApplicationBroadcastTime_setup_consumer() in main()');
        } else {
            L.push('    // Broadcast Time Producer: 2 consumer ranges + 2 producer ranges');
            L.push('    // registered by OpenLcbApplicationBroadcastTime_setup_producer() in main()');
        }
        consCount++;

    }

    /* User-selected well known consumers */
    if (wke.consumers.length > 0) {

        if (consCount > 0) { L.push(''); }
        L.push('    // Well Known Event consumers (selected in Node Wizard)');

        wke.consumers.forEach(function (id) {
            var evt = _wkeMap[id];
            if (!evt) { return; }
            if (evt.range) {
                L.push('    OpenLcbApplication_register_consumer_range(openlcb_node, ' + evt.define + ', EVENT_RANGE_COUNT_1);');
            } else {
                L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, ' + evt.define + ', EVENT_STATUS_UNKNOWN);');
            }
        });

    }

    if (consCount === 0 && wke.consumers.length === 0) {

        L.push('    // TODO: Register application-specific consumed events');
        L.push('    // OpenLcbApplication_register_consumer_eventid(openlcb_node, your_event_id, EVENT_STATUS_UNKNOWN);');

    }

    L.push('');
    L.push('}');
    L.push('');

    } /* end if (!isBootloader) — event registration */

    /* ---- main() ---- */
    L.push(_section('Application Entry Point'));
    L.push('');
    L.push('int main(void) {');
    L.push('');
    L.push('    // Initialize CAN transport (must be before OpenLcb_initialize)');
    L.push('    CanConfig_initialize(&can_config);');
    L.push('');
    L.push('    // Initialize the OpenLCB stack');
    L.push('    OpenLcb_initialize(&openlcb_config);');
    L.push('');
    L.push('    // Initialize platform drivers');

    if (olcbDriverChecked.length > 0) {
        L.push('    Drivers_initialize();');
    }

    if (canDriverChecked.length > 0) {
        L.push('    CanDriver_initialize();');
    }

    L.push('');
    L.push('    // Create the node');
    L.push('    node = OpenLcb_create_node(NODE_ID, &OpenLcbUserConfig_node_parameters);');
    L.push('');

    if (broadcastOn) {
        L.push('    // Set up broadcast time');
        if (s.broadcast === 'consumer') {
            L.push('    OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);');
        } else {
            L.push('    OpenLcbApplicationBroadcastTime_setup_producer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);');
        }
        L.push('');
    }

    if (isTrainNode) {
        L.push('    // Set up train protocol (registers Is Train + emergency events automatically)');
        L.push('    OpenLcbApplicationTrain_setup(node);');
        L.push('');
    }

    if (!isBootloader) {
    L.push('    // Register application events');
    L.push('    _register_app_producers(node);');
    L.push('    _register_app_consumers(node);');
    L.push('');
    }

    L.push('    // Main loop');
    L.push('    while (1) {');
    L.push('');
    L.push('        OpenLcb_run();');
    L.push('');
    L.push('    }');
    L.push('');
    L.push('}');

    return L.join('\n');

}

/* ---------- Helper: resolve checked driver functions ---------- */

function _getCheckedDriverFns(groupKey, state) {

    if (!state.driverState || !state.driverState[groupKey]) { return []; }

    var checkedNames = state.driverState[groupKey].checked || [];
    if (typeof DRIVER_GROUPS === 'undefined' || !DRIVER_GROUPS[groupKey]) { return []; }

    var group = DRIVER_GROUPS[groupKey];
    var result = [];

    for (var i = 0; i < group.functions.length; i++) {

        var fn = group.functions[i];

        if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
            result.push(fn);
        }

    }

    return result;

}

/* ---------- Helper: resolve checked callback functions ---------- */

function _getCheckedCallbackFns(groupKey, state) {

    if (!state.callbackState || !state.callbackState[groupKey]) { return []; }

    var checkedNames = state.callbackState[groupKey].checked || [];
    if (typeof CALLBACK_GROUPS === 'undefined' || !CALLBACK_GROUPS[groupKey]) { return []; }

    var group = CALLBACK_GROUPS[groupKey];
    var result = [];

    for (var i = 0; i < group.functions.length; i++) {

        var fn = group.functions[i];

        if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
            result.push(fn);
        }

    }

    return result;

}

/* ---------- Helper: find driver fn by name ---------- */

function _findDriverFnByName(checkedList, name) {

    for (var i = 0; i < checkedList.length; i++) {

        if (checkedList[i].name === name) { return checkedList[i]; }

    }

    return null;

}

/* ---------- Helper: find callback fn by configField ---------- */

function _findCallbackFnByConfigField(checkedList, configField) {

    for (var i = 0; i < checkedList.length; i++) {

        if (checkedList[i].configField === configField) { return checkedList[i]; }

    }

    return null;

}

/* ---------- Helper: emit wiring lines for callback group ---------- */

function _emitCallbackWiring(lines, checkedFns, prefix) {

    for (var i = 0; i < checkedFns.length; i++) {

        var fn = checkedFns[i];

        if (fn.configField) {

            var pad = _padName('.' + fn.configField, 46);
            lines.push('    ' + pad + '= &' + prefix + '_' + fn.name + ',');

        }

    }

}
