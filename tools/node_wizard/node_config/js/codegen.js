/* ============================================================
 * codegen.js — Code generation for openlcb_user_config.h/.c
 *
 * Called by app.js whenever any UI field changes.
 * Reads a plain state object and returns file contents as strings.
 *
 * State object shape:
 *   nodeType         'basic' | 'typical' | 'train' | 'train-controller' | null
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

/* Pre-computed default byte arrays (XML → UTF-8 + null terminator) */

function _xmlToBytes(xml) {

    var raw    = new TextEncoder().encode(xml);
    var result = new Uint8Array(raw.length + 1);

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
 * Format a Uint8Array as a C initializer.
 * Shows at most maxRows rows of 16 bytes, then a "... N bytes total" comment.
 */
function _byteArrayStr(bytes, maxRows, indent) {

    maxRows = maxRows || 4;
    indent  = indent  || '    ';

    if (!bytes || bytes.length === 0) {
        return '{ 0x00 }';
    }

    var cols = 16;
    var lines = [];
    var rowCount = 0;

    for (var i = 0; i < bytes.length && rowCount < maxRows; i += cols, rowCount++) {

        var chunk = [];

        for (var j = i; j < Math.min(i + cols, bytes.length); j++) {
            chunk.push('0x' + (bytes[j] < 16 ? '0' : '') + bytes[j].toString(16).toUpperCase());
        }

        var comma = (i + cols < bytes.length) ? ',' : '';
        lines.push(indent + chunk.join(', ') + comma);

    }

    var result = '{\n' + lines.join('\n');

    if (rowCount * cols < bytes.length) {
        result += '\n' + indent + '/* ... ' + bytes.length + ' bytes total */';
    }

    result += '\n' + indent.slice(0, -4) + '}';

    return result;

}

/* ---------- section header helper ---------- */

function _section(title) {

    return '// =============================================================================\n'
         + '// ' + title + '\n'
         + '// =============================================================================';

}

/* ---------- generateH ---------- */

function generateH(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated .h file */';
    }

    var isTrainRole = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrain     = isTrainRole;   /* alias: both roles need train feature flags */
    var isBasic     = s.nodeType === 'basic';
    var hasCfgMem   = !isBasic;
    var broadcastOn = s.broadcast !== 'none';
    var firmwareOn  = s.firmware && hasCfgMem;
    var nodeLabel   = s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var L = [];

    L.push('/**');
    L.push(' * @file openlcb_user_config.h');
    L.push(' * @brief Feature flags, buffer sizes, and compile-time constants for a');
    L.push(' *        ' + nodeLabel + ' node built on OpenLcbCLib.');
    L.push(' *');
    L.push(' * @copyright');
    L.push(' *   Copyright (c) <YEAR>, <YOUR NAME OR COMPANY>');
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard — https://github.com/JimKuenworthy/OpenLcbCLib');
    L.push(' */');
    L.push('');
    L.push('#ifndef __OPENLCB_USER_CONFIG__');
    L.push('#define __OPENLCB_USER_CONFIG__');
    L.push('');

    /* ---- Feature Flags ---- */
    L.push(_section('Feature Flags'));
    L.push('');
    L.push('#define OPENLCB_COMPILE_EVENTS');

    if (hasCfgMem) {
        L.push('#define OPENLCB_COMPILE_DATAGRAMS');
        L.push('#define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
    } else {
        L.push('// #define OPENLCB_COMPILE_DATAGRAMS');
        L.push('// #define OPENLCB_COMPILE_MEMORY_CONFIGURATION');
    }

    if (firmwareOn) {
        L.push('#define OPENLCB_COMPILE_FIRMWARE');
    } else {
        L.push('// #define OPENLCB_COMPILE_FIRMWARE');
    }

    if (broadcastOn) {
        L.push('#define OPENLCB_COMPILE_BROADCAST_TIME');
    } else {
        L.push('// #define OPENLCB_COMPILE_BROADCAST_TIME');
    }

    if (isTrain) {
        L.push('#define OPENLCB_COMPILE_TRAIN');
    } else {
        L.push('// #define OPENLCB_COMPILE_TRAIN');
    }

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
    L.push('//   STREAM   (512 bytes each) -- stream data transfer (future use)');
    L.push('');
    L.push(_def('USER_DEFINED_BASIC_BUFFER_DEPTH',    (s.advBasicBuf    !== undefined ? s.advBasicBuf    : 16).toString()));
    L.push(_def('USER_DEFINED_DATAGRAM_BUFFER_DEPTH', (s.advDatagramBuf !== undefined ? s.advDatagramBuf : (hasCfgMem ? 8 : 0)).toString()));
    L.push(_def('USER_DEFINED_SNIP_BUFFER_DEPTH',     (s.advSnipBuf     !== undefined ? s.advSnipBuf     : 4).toString()));
    L.push(_def('USER_DEFINED_STREAM_BUFFER_DEPTH',   (s.advStreamBuf   !== undefined ? s.advStreamBuf   : 0).toString()));
    L.push('');

    /* ---- Virtual Node Allocation ---- */
    L.push(_section('Virtual Node Allocation'));
    L.push('// How many virtual nodes this device can host.  Most simple devices use 1.');
    L.push('// Train command stations may need more (one per locomotive being controlled).');
    L.push('');
    L.push(_def('USER_DEFINED_NODE_BUFFER_DEPTH',     (s.advNodeBuf !== undefined ? s.advNodeBuf : 1).toString()));
    L.push('');

    /* ---- Events ---- */
    L.push(_section('Events (requires OPENLCB_COMPILE_EVENTS)'));
    L.push('// Maximum number of produced/consumed events per node, and how many event ID');
    L.push('// ranges each node can handle.  Ranges are used by protocols like Train Search');
    L.push('// that work with contiguous blocks of event IDs.');
    L.push('// Range counts must be at least 1 for valid array sizing.');
    L.push('');
    L.push(_def('USER_DEFINED_PRODUCER_COUNT',       (s.producerCount      || 64).toString()));
    L.push(_def('USER_DEFINED_PRODUCER_RANGE_COUNT', (s.producerRangeCount ||  4).toString()));
    L.push(_def('USER_DEFINED_CONSUMER_COUNT',       (s.consumerCount      || 64).toString()));
    L.push(_def('USER_DEFINED_CONSUMER_RANGE_COUNT', (s.consumerRangeCount ||  4).toString()));
    L.push('');

    /* ---- Memory Configuration ---- */
    L.push(_section('Memory Configuration (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)'));
    L.push('// CDI_LENGTH -- size in bytes of the CDI (Configuration Description Information)');
    L.push('//   XML buffer.  Must be large enough to hold your node\'s complete CDI XML.');
    L.push('// FDI_LENGTH -- size in bytes of the FDI (Function Description Information)');
    L.push('//   buffer.  Only used by train nodes; set small (e.g. 100) if not a train.');
    L.push('//');
    L.push('// The two address values tell the SNIP protocol where in your node\'s');
    L.push('// configuration memory space the user-editable name and description strings');
    L.push(_def('USER_DEFINED_CDI_LENGTH',                                 s.cdiLength.toString()));
    L.push(_def('USER_DEFINED_FDI_LENGTH',                                 s.fdiLength.toString()));
    L.push('');

    /* ---- CAN Message Buffers ---- */
    L.push(_section('CAN Message Buffers'));
    L.push('// How many raw CAN messages can be queued.  Tune for your CAN driver.');
    L.push('');
    L.push(_def('USER_DEFINED_CAN_MSG_BUFFER_DEPTH', (s.advCanBuf !== undefined ? s.advCanBuf : 20).toString()));
    L.push('');

    /* ---- Train Protocol ---- */
    L.push(_section('Train Protocol (requires OPENLCB_COMPILE_TRAIN)'));
    L.push('// TRAIN_NODE_COUNT        -- max simultaneous train nodes (often equals');
    L.push('//                            NODE_BUFFER_DEPTH for a dedicated command station)');
    L.push('// MAX_LISTENERS_PER_TRAIN -- max consist members (listener slots) per train');
    L.push('// MAX_TRAIN_FUNCTIONS     -- number of DCC function outputs: 29 = F0 through F28');
    L.push('');
    L.push(_def('USER_DEFINED_TRAIN_NODE_COUNT',         isTrain ? (s.advTrainNodeCount  !== undefined ? s.advTrainNodeCount  : 1).toString() : '0'));
    L.push(_def('USER_DEFINED_MAX_LISTENERS_PER_TRAIN',  isTrain ? (s.advTrainListeners !== undefined ? s.advTrainListeners : 6).toString() : '0'));
    L.push(_def('USER_DEFINED_MAX_TRAIN_FUNCTIONS',      isTrain ? (s.advTrainFunctions !== undefined ? s.advTrainFunctions : 29).toString() : '0'));
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

/* ---------- generateC ---------- */

function generateC(s) {

    if (!s || !s.nodeType) {
        return '/* Select a node type above to see the generated .c file */';
    }

    var isTrainRole = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrain     = isTrainRole;   /* alias: both roles enable train address spaces */
    var isTrainNode = s.nodeType === 'train';   /* locomotive only — has FDI */
    var isBasic     = s.nodeType === 'basic';
    var hasCfgMem   = !isBasic;
    var broadcastOn = s.broadcast !== 'none';
    var firmwareOn  = s.firmware && hasCfgMem;
    var unalignedR  = hasCfgMem && s.unalignedReads;
    var unalignedW  = hasCfgMem && s.unalignedWrites;
    var cfgHighest  = hasCfgMem ? (s.configMemHighest || '0x200') : '0';
    var cfgHighestNum = parseInt(cfgHighest) || 0;
    var cdiRequired   = hasCfgMem ? _cdiHighestAddrInSpace(s.cdiXml, 253) : -1;
    var nodeLabel   = s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    var cdiHighest = (s.cdiLength > 0) ? (s.cdiLength - 1) : 0;
    var fdiHighest = (s.fdiLength > 0) ? (s.fdiLength - 1) : 0;

    var L = [];

    /* ---- File header ---- */
    L.push('/**');
    L.push(' * @file openlcb_user_config.c');
    L.push(' * @brief Const node_parameters_t struct that defines SNIP strings, protocol');
    L.push(' *        support bits, CDI/FDI data, and address space layout for a');
    L.push(' *        ' + nodeLabel + ' node.');
    L.push(' *');
    L.push(' * @copyright');
    L.push(' *   Copyright (c) <YEAR>, <YOUR NAME OR COMPANY>');
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard — https://github.com/JimKuenworthy/OpenLcbCLib');
    L.push(' */');
    L.push('');
    L.push('#include "openlcb_user_config.h"');
    L.push('#include "src/openlcb/openlcb_types.h"');
    L.push('#include "src/openlcb/openlcb_defines.h"');
    L.push('');
    L.push('');
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

    if (isBasic && s.snipEnabled) {

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

    var snipSource = (isBasic && s.snipEnabled) ? 'from wizard SNIP fields'
                   : (hasCfgMem && cdiIdent)   ? 'from CDI <identification>'
                   :                             '';

    L.push('    .snip.mfg_version = 4,  // SNIP protocol version — fixed by spec');
    L.push('    .snip.name = "' + _escC(snipName) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.model = "' + _escC(snipModel) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.hardware_version = "' + _escC(snipHw) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.software_version = "' + _escC(snipSw) + '",' + (snipSource ? '  // ' + snipSource : ''));
    L.push('    .snip.user_version = 2,  // SNIP user-data version — fixed by spec');
    L.push('');

    /* ---- 2. .protocol_support ---- */
    var psi = [];

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
    L.push('    .consumer_count_autocreate = 0,');
    L.push('    .producer_count_autocreate = 0,');
    L.push('');

    /* ---- 5. .cdi[] ---- */
    if (s.cdiBytes && s.cdiBytes.length > 1) {
        L.push('    .cdi =');
        L.push('    ' + _byteArrayStr(s.cdiBytes, 4, '        ') + ',');
    } else {
        L.push('    .cdi = { 0x00 },');
    }

    L.push('');

    /* ---- 6. .fdi[] ---- */
    if (isTrainNode && s.fdiBytes && s.fdiBytes.length > 1) {
        L.push('    .fdi =');
        L.push('    ' + _byteArrayStr(s.fdiBytes, 4, '        ') + ',');
    } else {
        L.push('    .fdi = { 0x00 },');
    }

    L.push('');

    /* ---- 7. Space 0xFF — CDI (address_space_configuration_definition) ---- */
    L.push('    // Space 0xFF — CDI (Configuration Description Information)');
    L.push('    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description)');
    L.push('    //    of the Config Memory System so make sure the CDI maps these 2 items to the first');
    L.push('    //    128 bytes as well');
    L.push('    .address_space_configuration_definition.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_configuration_definition.read_only = true,  // CDI is always read-only per spec');
    L.push('    .address_space_configuration_definition.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,');
    L.push('    .address_space_configuration_definition.highest_address = ' + cdiHighest + ',  // ' + s.cdiLength + ' CDI bytes including null terminator — from CDI XML');
    L.push('    .address_space_configuration_definition.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_configuration_definition.description = NULL,');
    L.push('');

    /* ---- 8. Space 0xFE — All Memory (address_space_all) ---- */
    L.push('    // Space 0xFE — All Memory (raw debug access)');
    L.push('    .address_space_all.present = false,  // not implemented — reserved by spec');
    L.push('    .address_space_all.read_only = true,  // not implemented — reserved by spec');
    L.push('    .address_space_all.low_address_valid = false,');
    L.push('    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,');
    L.push('    .address_space_all.highest_address = 0xFFFFFFFF,  // full 32-bit range — spec defined');
    L.push('    .address_space_all.low_address = 0,');
    L.push('    .address_space_all.description = NULL,');
    L.push('');

    /* ---- 9. Space 0xFD — Config Memory (address_space_config_memory) ---- */
    var cfgComment = '';

    if (cdiRequired > 0 && cfgHighestNum > 0 && cdiRequired > cfgHighestNum) {
        cfgComment = '  // WARNING: CDI defines ' + cdiRequired + ' bytes (0x' + cdiRequired.toString(16).toUpperCase() + ') but this is only ' + cfgHighestNum + ' (0x' + cfgHighestNum.toString(16).toUpperCase() + ') — INCREASE THIS VALUE';
    } else if (cdiRequired > 0) {
        cfgComment = '  // from wizard — CDI needs ' + cdiRequired + ' bytes (0x' + cdiRequired.toString(16).toUpperCase() + '), OK';
    } else {
        cfgComment = '  // from wizard "Config memory size" field';
    }

    L.push('    // Space 0xFD — Configuration Memory (user-configurable settings)');
    L.push('    .address_space_config_memory.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_config_memory.read_only = false,  // config memory is read/write');
    L.push('    .address_space_config_memory.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,');
    L.push('    .address_space_config_memory.highest_address = ' + cfgHighest + ',' + cfgComment);
    L.push('    .address_space_config_memory.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_config_memory.description = NULL,');
    L.push('');

    /* ---- 10. Space 0xFC — ACDI Manufacturer (address_space_acdi_manufacturer) ---- */
    L.push('    // Space 0xFC — ACDI Manufacturer (read-only identification)');
    L.push('    .address_space_acdi_manufacturer.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_acdi_manufacturer.read_only = true,  // manufacturer info is always read-only');
    L.push('    .address_space_acdi_manufacturer.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,');
    L.push('    .address_space_acdi_manufacturer.highest_address = 124,  // Zero indexed: 1 + 41 + 41 + 21 + 21 = 125 bytes (0-124)');
    L.push('    .address_space_acdi_manufacturer.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_acdi_manufacturer.description = NULL,');
    L.push('');

    /* ---- 11. Space 0xFB — ACDI User (address_space_acdi_user) ---- */
    L.push('    // Space 0xFB — ACDI User (user name & description)');
    L.push('    .address_space_acdi_user.present = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set: node type has config memory');
    L.push('    .address_space_acdi_user.read_only = false,  // user info is read/write — config tools can update');
    L.push('    .address_space_acdi_user.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,');
    L.push('    .address_space_acdi_user.highest_address = 127,  // Zero indexed: 1 + 63 + 64 = 128 bytes (0-127)');
    L.push('    .address_space_acdi_user.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_acdi_user.description = NULL,');
    L.push('');

    /* ---- 12. Space 0xFA — Train FDI (address_space_train_function_definition_info) ---- */
    L.push('    // Space 0xFA — Train FDI (Function Description Information)');
    L.push('    .address_space_train_function_definition_info.present = ' + (isTrainNode ? 'true' : 'false') + ',  // auto-set: train (locomotive) node type only');
    L.push('    .address_space_train_function_definition_info.read_only = true,  // FDI is always read-only per spec');
    L.push('    .address_space_train_function_definition_info.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_train_function_definition_info.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,');
    L.push('    .address_space_train_function_definition_info.highest_address = ' + fdiHighest + ',  // ' + s.fdiLength + ' FDI bytes including null terminator — from FDI XML');
    L.push('    .address_space_train_function_definition_info.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_train_function_definition_info.description = NULL,');
    L.push('');

    /* ---- 13. Space 0xF9 — Train Function Config (address_space_train_function_config_memory) ---- */
    L.push('    // Space 0xF9 — Train Function Configuration Memory');
    L.push('    .address_space_train_function_config_memory.present = ' + (isTrainNode ? 'true' : 'false') + ',  // auto-set: train (locomotive) node type only');
    L.push('    .address_space_train_function_config_memory.read_only = false,  // function config is read/write');
    L.push('    .address_space_train_function_config_memory.low_address_valid = false,  // assume the low address starts at 0');
    L.push('    .address_space_train_function_config_memory.address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG,');
    L.push('    .address_space_train_function_config_memory.highest_address = 0,  // library calculates from train function count at runtime');
    L.push('    .address_space_train_function_config_memory.low_address = 0,  // ignored if low_address_valid is false');
    L.push('    .address_space_train_function_config_memory.description = NULL,');
    L.push('');

    /* ---- 14. .configuration_options (user_configuration_options) ---- */
    var lowAddrSpace;

    if (isBasic) {
        lowAddrSpace = '0x00';
    } else if (firmwareOn) {
        lowAddrSpace = 'CONFIG_MEM_SPACE_FIRMWARE';
    } else if (isTrainNode) {
        lowAddrSpace = 'CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIG';
    } else {
        lowAddrSpace = 'CONFIG_MEM_SPACE_ACDI_USER_ACCESS';
    }

    L.push('    .configuration_options.write_under_mask_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
    L.push('    .configuration_options.unaligned_reads_supported = ' + (unalignedR ? 'true' : 'false') + ',  // from wizard Memory Configuration option');
    L.push('    .configuration_options.unaligned_writes_supported = ' + (unalignedW ? 'true' : 'false') + ',  // from wizard Memory Configuration option');
    L.push('    .configuration_options.read_from_manufacturer_space_0xfc_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
    L.push('    .configuration_options.read_from_user_space_0xfb_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
    L.push('    .configuration_options.write_to_user_space_0xfb_supported = ' + (hasCfgMem ? 'true' : 'false') + ',  // auto-set by node type');
    L.push('    .configuration_options.stream_read_write_supported = false,  // stream transport not implemented');
    L.push('    .configuration_options.high_address_space = ' + (hasCfgMem ? 'CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO' : '0x00') + ',  // derived from enabled features');
    L.push('    .configuration_options.low_address_space = ' + lowAddrSpace + ',  // derived from enabled features');
    L.push('    .configuration_options.description = NULL,');
    L.push('');

    /* ---- 15. Space 0xEF — Firmware (address_space_firmware) ---- */
    L.push('    // Space 0xEF — Firmware Upgrade');
    L.push('    .address_space_firmware.present = ' + (firmwareOn ? 'true' : 'false') + ',  // from wizard Firmware Update option');
    L.push('    .address_space_firmware.read_only = false,  // firmware space accepts writes for flashing');
    L.push('    .address_space_firmware.low_address_valid = false,  // Firmware ALWAYS assumes it starts at 0');
    L.push('    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,');
    L.push('    .address_space_firmware.highest_address = 0xFFFFFFFF,  // full range — firmware handler manages boundaries');
    L.push('    .address_space_firmware.low_address = 0,  // Firmware ALWAYS assumes it starts at 0');
    L.push('    .address_space_firmware.description = NULL,');

    L.push('');
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

    var isTrainRole = s.nodeType === 'train' || s.nodeType === 'train-controller';
    var isTrainNode = s.nodeType === 'train';
    var isTrain     = isTrainRole;
    var isBasic     = s.nodeType === 'basic';
    var hasCfgMem   = !isBasic;
    var broadcastOn = s.broadcast !== 'none';
    var firmwareOn  = s.firmware && hasCfgMem;
    var nodeLabel   = s.nodeType === 'train-controller' ? 'Train Controller' : s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

    /* Resolve which driver/callback functions are checked */
    var canDriverChecked  = _getCheckedDriverFns('can-drivers', s);
    var olcbDriverChecked = _getCheckedDriverFns('olcb-drivers', s);
    var cbCanChecked      = _getCheckedCallbackFns('cb-can', s);
    var cbOlcbChecked     = _getCheckedCallbackFns('cb-olcb', s);
    var cbEventsChecked   = _getCheckedCallbackFns('cb-events', s);
    var cbConfigChecked   = _getCheckedCallbackFns('cb-config-mem', s);
    var cbTrainChecked    = _getCheckedCallbackFns('cb-train', s);
    var cbBcastChecked    = _getCheckedCallbackFns('cb-bcast-time', s);

    var L = [];

    /* ---- File header with cliff notes ---- */
    L.push('/**');
    L.push(' * @file main.c');
    L.push(' * @brief Application entry point for a ' + nodeLabel + ' node.');
    L.push(' *        Wires driver and callback functions into the OpenLcbCLib config');
    L.push(' *        structs, sets the Node ID, and runs the main protocol loop.');
    L.push(' *');
    L.push(' * @copyright');
    L.push(' *   Copyright (c) <YEAR>, <YOUR NAME OR COMPANY>');
    L.push(' *   <YOUR LICENSE TEXT HERE>');
    L.push(' *');
    L.push(' * Generated by Node Wizard — https://github.com/JimKuenworthy/OpenLcbCLib');
    L.push(' *');
    L.push(' * QUICK START');
    L.push(' * ===========');
    L.push(' *  1. Copy the OpenLcbCLib library into the src/ folder.');
    L.push(' *  2. Implement the driver stubs in application_drivers/.');
    L.push(' *     At minimum: CAN transmit/receive, resources lock/unlock,');
    L.push(' *     and the 100ms timer are critical for basic operation.');
    L.push(' *  3. Implement any callback stubs in application_callbacks/.');
    L.push(' *  4. Set your unique Node ID in the NODE_ID define below.');
    L.push(' *  5. Build and flash. The main loop calls OpenLcb_run() which');
    L.push(' *     handles all protocol processing automatically.');
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
    L.push('#define NODE_ID    0x050701010000    // TODO: Replace with your unique Node ID');
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
        L.push('    .firmware_write = &Drivers_write_firmware,');
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
    L.push(_section('Application Event Registration'));
    L.push('');
    L.push('void _register_app_producers(openlcb_node_t *openlcb_node) {');
    L.push('');

    if (s.nodeType === 'train') {

        L.push('    // Train node produces "Is Train" to identify itself on the network');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_TRAIN, EVENT_STATUS_SET);');

    } else if (s.nodeType === 'train-controller') {

        L.push('    // Throttle produces emergency events to stop/resume all trains on the network');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_UNKNOWN);');
        L.push('    OpenLcbApplication_register_producer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_UNKNOWN);');

    } else {

        L.push('    // TODO: Register application-specific produced events');
        L.push('    // OpenLcbApplication_register_producer_eventid(openlcb_node, your_event_id, EVENT_STATUS_UNKNOWN);');

    }

    L.push('');
    L.push('}');
    L.push('');
    L.push('void _register_app_consumers(openlcb_node_t *openlcb_node) {');
    L.push('');

    if (s.nodeType === 'train') {

        L.push('    // Train node consumes global emergency events');
        L.push('    // NOTE: OpenLcbApplicationTrain_setup() also registers these — shown here for reference');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_OFF, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_SET);');
        L.push('    OpenLcbApplication_register_consumer_eventid(openlcb_node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_SET);');

    } else {

        L.push('    // TODO: Register application-specific consumed events');
        L.push('    // OpenLcbApplication_register_consumer_eventid(openlcb_node, your_event_id, EVENT_STATUS_UNKNOWN);');

    }

    L.push('');
    L.push('}');
    L.push('');

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

    L.push('    // Register application events');
    L.push('    _register_app_producers(node);');
    L.push('    _register_app_consumers(node);');
    L.push('');

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
