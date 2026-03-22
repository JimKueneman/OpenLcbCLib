/* ============================================================
 * codegen.js — Code generation for openlcb_user_config.h/.c
 *
 * Called by app.js whenever any UI field changes.
 * Reads a plain state object and returns file contents as strings.
 *
 * State object shape:
 *   nodeType         'basic' | 'typical' | 'train' | null
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
    '    <function number="0" kind="binary">',
    '      <name>Headlight</name>',
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

function _padName(name, width) {

    var s = name;

    while (s.length < width) {
        s += ' ';
    }

    return s;

}

function _def(name, value) {

    return '#define ' + _padName(name, 50) + value;

}

function _escC(str) {

    return (str || '').replace(/\\/g, '\\\\').replace(/"/g, '\\"').replace(/\n/g, '\\n');

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
function _byteArrayStr(bytes, xmlText, preserveWhitespace) {

    var indent = '    ';

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

        return '{\n' + lines.join('\n') + '\n}';

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
    result += '\n}';

    return result;

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

    var isTrain     = s.nodeType === 'train';
    var isBasic     = s.nodeType === 'basic';
    var hasCfgMem   = !isBasic;
    var broadcastOn = s.broadcast !== 'none';
    var firmwareOn  = s.firmware && hasCfgMem;
    var nodeLabel   = s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var L = [];

    L.push('/* ============================================================');
    L.push(' * openlcb_user_config.h  —  Generated by Node Wizard');
    L.push(' * Node Type:  ' + nodeLabel);
    L.push(' * ============================================================ */');
    L.push('');
    L.push('#ifndef __OPENLCB_USER_CONFIG_H__');
    L.push('#define __OPENLCB_USER_CONFIG_H__');
    L.push('');
    L.push('#ifdef __cplusplus');
    L.push('extern "C" {');
    L.push('#endif');
    L.push('');

    /* ---- Feature Flags ---- */
    L.push('/* ---- Feature Flags ---- */');
    L.push(_def('OPENLCB_COMPILE_EVENTS',               'ON'));
    L.push(_def('OPENLCB_COMPILE_DATAGRAMS',            hasCfgMem   ? 'ON' : 'OFF'));
    L.push(_def('OPENLCB_COMPILE_MEMORY_CONFIGURATION', hasCfgMem   ? 'ON' : 'OFF'));
    L.push(_def('OPENLCB_COMPILE_FIRMWARE',             firmwareOn  ? 'ON' : 'OFF'));
    L.push(_def('OPENLCB_COMPILE_BROADCAST_TIME',       broadcastOn ? 'ON' : 'OFF'));
    L.push(_def('OPENLCB_COMPILE_TRAIN',                isTrain     ? 'ON' : 'OFF'));
    L.push(_def('OPENLCB_COMPILE_STREAM',               'OFF'));
    L.push('');

    /* ---- Buffer Pool ---- */
    L.push('/* ---- Buffer Pool ---- */');
    L.push(_def('USER_DEFINED_BASIC_BUFFER_DEPTH',    '16'));
    L.push(_def('USER_DEFINED_DATAGRAM_BUFFER_DEPTH', hasCfgMem ? '8' : '0'));
    L.push(_def('USER_DEFINED_SNIP_BUFFER_DEPTH',     '4'));
    L.push(_def('USER_DEFINED_STREAM_BUFFER_DEPTH',   '0'));
    L.push(_def('USER_DEFINED_NODE_BUFFER_DEPTH',     '1'));
    L.push(_def('USER_DEFINED_CAN_MSG_BUFFER_DEPTH',  '20'));
    L.push('');

    /* ---- Events ---- */
    L.push('/* ---- Events ---- */');
    L.push(_def('USER_DEFINED_PRODUCER_COUNT',       (s.producerCount      || 64).toString()));
    L.push(_def('USER_DEFINED_PRODUCER_RANGE_COUNT', (s.producerRangeCount ||  4).toString()));
    L.push(_def('USER_DEFINED_CONSUMER_COUNT',       (s.consumerCount      || 64).toString()));
    L.push(_def('USER_DEFINED_CONSUMER_RANGE_COUNT', (s.consumerRangeCount ||  4).toString()));
    L.push('');

    /* ---- Train Protocol ---- */
    L.push('/* ---- Train Protocol ---- */');
    L.push(_def('USER_DEFINED_TRAIN_NODE_COUNT',        '1'));
    L.push(_def('USER_DEFINED_MAX_LISTENERS_PER_TRAIN', isTrain ? '6' : '0'));
    L.push(_def('USER_DEFINED_MAX_TRAIN_FUNCTIONS',     isTrain ? '29' : '0'));
    L.push('');

    /* ---- Alias Mapping Listener ---- */
    L.push('/* ---- Alias Mapping Listener ---- */');
    L.push(_def('USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL',   '1'));
    L.push(_def('USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS',  '250'));
    L.push(_def('USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS',  '30'));
    L.push('');

    L.push('#ifdef __cplusplus');
    L.push('}');
    L.push('#endif');
    L.push('');
    L.push('#endif /* __OPENLCB_USER_CONFIG_H__ */');

    return L.join('\n');

}

/* ---------- generateC ---------- */

function generateC(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated .c file */';
    }

    _ensureBytes(s);

    var isTrain     = s.nodeType === 'train';
    var isBasic     = s.nodeType === 'basic';
    var hasCfgMem   = !isBasic;
    var broadcastOn = s.broadcast !== 'none';
    var firmwareOn  = s.firmware && hasCfgMem;
    var unalignedR  = hasCfgMem && s.unalignedReads;
    var unalignedW  = hasCfgMem && s.unalignedWrites;
    var cfgHighest  = hasCfgMem ? (s.configMemHighest || '0x200') : '0';
    var nodeLabel   = s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var L = [];

    /* ---- File header ---- */
    L.push('/* ============================================================');
    L.push(' * openlcb_user_config.c  —  Generated by Node Wizard');
    L.push(' * Node Type:  ' + nodeLabel);
    L.push(' * ============================================================ */');
    L.push('');
    L.push('#include "openlcb_user_config.h"');
    L.push('#include "openlcb_node_parameters.h"');
    L.push('');
    /* ---- standalone CDI/FDI byte arrays (before the struct so sizeof works) ---- */
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('static const uint8_t _cdi_data[] =');
        L.push('    ' + _byteArrayStr(s.cdiBytes, s.cdiXml || s.cdiUserXml, !!s.preserveWhitespace) + ';');
        L.push('');
    }
    if (isTrain && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('static const uint8_t _fdi_data[] =');
        L.push('    ' + _byteArrayStr(s.fdiBytes, s.fdiXml || s.fdiUserXml, !!s.preserveWhitespace) + ';');
        L.push('');
    }

    L.push('const node_parameters_t openlcb_node_parameters = {');
    L.push('');

    /* ---- .snip ---- */
    L.push('    .snip = {');
    L.push('        .mfg_version      = 4,');

    if (isBasic && s.snipEnabled) {

        /* Use entered text; fall back to the placeholder defaults */
        const name  = s.snipName  || 'My Company';
        const model = s.snipModel || 'Signal Controller v2';
        const hw    = s.snipHw    || '1.0';
        const sw    = s.snipSw    || '0.1.0';

        L.push('        .name             = "' + _escC(name)  + '",');
        L.push('        .model            = "' + _escC(model) + '",');
        L.push('        .hardware_version = "' + _escC(hw)    + '",');
        L.push('        .software_version = "' + _escC(sw)    + '",');

    } else if (hasCfgMem) {

        L.push('        .name             = "",       /* populated from CDI */');
        L.push('        .model            = "",       /* populated from CDI */');
        L.push('        .hardware_version = "",       /* populated from CDI */');
        L.push('        .software_version = "",       /* populated from CDI */');

    } else {

        L.push('        .name             = "",');
        L.push('        .model            = "",');
        L.push('        .hardware_version = "",');
        L.push('        .software_version = "",');

    }

    L.push('        .user_version     = 2,');
    L.push('        .user_name        = "",');
    L.push('        .user_description = ""');
    L.push('    },');
    L.push('');

    /* ---- .protocol_support ---- */
    var psi = ['PSI_EVENT_EXCHANGE'];

    if (hasCfgMem) {
        psi.push('PSI_DATAGRAM');
        psi.push('PSI_MEMORY_CONFIGURATION');
        psi.push('PSI_CONFIGURATION_DESCRIPTION_INFO');
        psi.push('PSI_ABBREVIATED_DEFAULT_CDI');
    }

    if (isTrain) {
        psi.push('PSI_TRAIN_CONTROL');
        psi.push('PSI_FUNCTION_DESCRIPTION');
    }

    if (firmwareOn) {
        psi.push('PSI_FIRMWARE_UPGRADE');
    }

    if (psi.length === 1) {

        L.push('    .protocol_support = ' + psi[0] + ',');

    } else {

        L.push('    .protocol_support = (' + psi[0]);

        for (var pi = 1; pi < psi.length; pi++) {
            var isLast = (pi === psi.length - 1);
            L.push('                       | ' + psi[pi] + (isLast ? '),' : ''));
        }

    }

    L.push('');

    /* ---- .configuration_options ---- */
    var lowAddrSpace;

    if (isBasic) {
        lowAddrSpace = '0x00';
    } else if (firmwareOn) {
        lowAddrSpace = '0xEF';
    } else if (isTrain) {
        lowAddrSpace = '0xF9';
    } else {
        lowAddrSpace = '0xFB';
    }

    L.push('    .configuration_options = {');
    L.push('        .write_under_mask_supported               = ' + (hasCfgMem  ? 'true' : 'false') + ',');
    L.push('        .unaligned_reads_supported                = ' + (unalignedR ? 'true' : 'false') + ',');
    L.push('        .unaligned_writes_supported               = ' + (unalignedW ? 'true' : 'false') + ',');
    L.push('        .read_from_manufacturer_space_0xfc_supported = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('        .read_from_user_space_0xfb_supported      = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('        .write_to_user_space_0xfb_supported       = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('        .stream_read_write_supported              = false,');
    L.push('        .high_address_space                       = ' + (hasCfgMem ? '0xFF' : '0x00') + ',');
    L.push('        .low_address_space                        = ' + lowAddrSpace + ',');
    L.push('        .description                              = NULL');
    L.push('    },');
    L.push('');

    /* ---- auto-create counts ---- */
    L.push('    // For internal testing only do not set to anything but 0');
    L.push('    .consumer_count_autocreate = 0,');
    L.push('    .producer_count_autocreate = 0,');
    L.push('');

    /* ---- address spaces ---- */
    L.push('    .addr_space_count = 8,');
    L.push('    .addr_space = {');
    L.push('');

    /* 0xFF — CDI */
    L.push('        [0] = {  /* 0xFF — CDI */');
    L.push('            .present           = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('            .read_only         = true,');
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('            .highest_address   = sizeof(_cdi_data) - 1,  // auto-computed from CDI byte array');
    } else {
        L.push('            .highest_address   = 0,');
    }
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xFE — All Memory */
    L.push('        [1] = {  /* 0xFE — All Memory */');
    L.push('            .present           = false,');
    L.push('            .read_only         = false,');
    L.push('            .highest_address   = 0xFFFFFFFF,');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xFD — Config Memory */
    L.push('        [2] = {  /* 0xFD — Config Memory */');
    L.push('            .present           = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('            .read_only         = false,');
    L.push('            .highest_address   = ' + cfgHighest + ',');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xFC — ACDI Manufacturer */
    L.push('        [3] = {  /* 0xFC — ACDI Manufacturer */');
    L.push('            .present           = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('            .read_only         = true,');
    L.push('            .highest_address   = 124,');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xFB — ACDI User */
    L.push('        [4] = {  /* 0xFB — ACDI User */');
    L.push('            .present           = ' + (hasCfgMem ? 'true' : 'false') + ',');
    L.push('            .read_only         = false,');
    L.push('            .highest_address   = 127,');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xFA — Train FDI */
    L.push('        [5] = {  /* 0xFA — Train FDI */');
    L.push('            .present           = ' + (isTrain ? 'true' : 'false') + ',');
    L.push('            .read_only         = true,');
    if (isTrain && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('            .highest_address   = sizeof(_fdi_data) - 1,  // auto-computed from FDI byte array');
    } else {
        L.push('            .highest_address   = 0,');
    }
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xF9 — Train Function Config */
    L.push('        [6] = {  /* 0xF9 — Train Function Config */');
    L.push('            .present           = ' + (isTrain ? 'true' : 'false') + ',');
    L.push('            .read_only         = false,');
    L.push('            .highest_address   = 0,');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        },');
    L.push('');

    /* 0xEF — Firmware */
    L.push('        [7] = {  /* 0xEF — Firmware */');
    L.push('            .present           = ' + (firmwareOn ? 'true' : 'false') + ',');
    L.push('            .read_only         = false,');
    L.push('            .highest_address   = 0xFFFFFFFF,');
    L.push('            .low_address_valid = false,');
    L.push('            .low_address       = 0,');
    L.push('            .description       = NULL');
    L.push('        }');
    L.push('');
    L.push('    },');
    L.push('');

    /* ---- .cdi ---- */
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('    .cdi = _cdi_data,');
    } else {
        L.push('    .cdi = NULL,  // no CDI');
    }

    L.push('');

    /* ---- .fdi ---- */
    if (isTrain && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('    .fdi = _fdi_data,');
    } else {
        L.push('    .fdi = NULL,  // no FDI');
    }

    L.push('');
    L.push('};');

    return L.join('\n');

}
