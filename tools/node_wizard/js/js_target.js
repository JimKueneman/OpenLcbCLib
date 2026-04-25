/* =========================================================================
 * js_target.js  —  JavaScript (OpenLcbJSLib) language target.
 *
 * Emits an ESM-format openlcb_user_config.js whose two exports —
 * NODE_ID (BigInt) and OpenLcbUserConfig_node_parameters (object) —
 * are consumed directly by the WASM wrapper's openlcb.createNode().
 *
 * Output layout (flat, no Arduino branch — JS doesn't need it):
 *
 *   <project>/
 *   |-- openlcb_user_config.js           ESM module
 *   |-- cdi.xml                          (when CDI present)
 *   |-- fdi.xml                          (when FDI present, train nodes)
 *   |-- GETTING_STARTED.txt
 *   |-- <type>_project.json              wizard reload file
 *
 * Field shapes match the hand-written examples in OpenLcbJSLib/examples/*.
 *
 * Depends on globals: generateH (no — kept local), CDI/FDI bytes via
 * _ensureBytes (called via codegen.js helper), DOMParser for CDI parsing,
 * LanguageTargets registry.
 *
 * Bootloader is not supported by the JS lib — throws if selected.
 * ========================================================================= */

var JsTarget = (function () {

    'use strict';

    var DEFAULT_PSI_IMPORT = '../../src/openlcb/constants.js';

    /* Map wke_defs.js `id` → JS Event constant name (in src/openlcb/constants.js).
     * Entries with no mapping (lookup returns undefined) cause register_events.js
     * to emit a TODO comment so the user can wire it up manually. */
    var WKE_TO_JS_EVENT = {
        'emergency-off':        'EMERGENCY_OFF',
        'clear-emergency-off':  'CLEAR_EMERGENCY_OFF',
        'emergency-stop':       'EMERGENCY_STOP',
        'clear-emergency-stop': 'CLEAR_EMERGENCY_STOP',
        'brownout-node':        'POWER_BROWN_OUT_NODE',
        'brownout-standard':    'POWER_BROWN_OUT_STANDARD',
        'new-log':              'NODE_RECORDED_NEW_LOG',
        'ident-button':         'IDENT_BUTTON_COMBO_PRESSED',
        'duplicate-node':       'DUPLICATE_NODE_DETECTED',
        'firmware-corrupted':   'FIRMWARE_CORRUPTED',
        'firmware-hw-switch':   'FIRMWARE_UPGRADE_BY_HW_SWITCH',
        'train':                'IS_TRAIN',

        'link-error-1':         'LINK_ERROR_CODE_1',
        'link-error-2':         'LINK_ERROR_CODE_2',
        'link-error-3':         'LINK_ERROR_CODE_3',
        'link-error-4':         'LINK_ERROR_CODE_4',

        'cbus-off':             'CBUS_OFF_SPACE',
        'cbus-on':              'CBUS_ON_SPACE',

        'dcc-acc-activate':     'DCC_ACCESSORY_ACTIVATE',
        'dcc-acc-deactivate':   'DCC_ACCESSORY_DEACTIVATE',
        'dcc-turnout-high':     'DCC_TURNOUT_FEEDBACK_HIGH',
        'dcc-turnout-low':      'DCC_TURNOUT_FEEDBACK_LOW',
        'dcc-sensor-high':      'DCC_SENSOR_FEEDBACK_HIGH',
        'dcc-sensor-low':       'DCC_SENSOR_FEEDBACK_LO',
        'dcc-ext-acc':          'DCC_EXTENDED_ACCESSORY_CMD_SPACE'
    };

    /* ----------------------------------------------------------------------- */
    /* State shaping (mirror of CTarget._buildCodegenState)                     */
    /* ----------------------------------------------------------------------- */

    function _buildCodegenState(wizardState) {

        var s = {};

        if (wizardState.configFormState) {
            Object.keys(wizardState.configFormState).forEach(function (key) {
                s[key] = wizardState.configFormState[key];
            });
        }

        s.nodeType           = wizardState.selectedNodeType;
        s.cdiUserXml         = wizardState.cdiUserXml || null;
        s.fdiUserXml         = wizardState.fdiUserXml || null;
        s.preserveWhitespace = !!wizardState.preserveWhitespace;

        return s;

    }

    /* ----------------------------------------------------------------------- */
    /* SNIP — matches CTarget logic.  mfg/user version are spec-fixed counts.   */
    /* ----------------------------------------------------------------------- */

    function _resolveSnip(s, isBasic, hasCfgMem) {

        var cdiIdent = hasCfgMem ? _extractCdiIdentification(s.cdiUserXml) : null;

        if (isBasic && s.snipEnabled) {
            return {
                name:  s.snipName  || 'My Company',
                model: s.snipModel || 'Signal Controller v2',
                hw:    s.snipHw    || '1.0',
                sw:    s.snipSw    || '0.1.0',
                source: 'from wizard SNIP fields'
            };
        }

        if (hasCfgMem && cdiIdent) {
            return {
                name:  cdiIdent.manufacturer    || '',
                model: cdiIdent.model           || '',
                hw:    cdiIdent.hardwareVersion || '',
                sw:    cdiIdent.softwareVersion || '',
                source: 'from CDI <identification>'
            };
        }

        return { name: '', model: '', hw: '', sw: '', source: '' };

    }

    /* ----------------------------------------------------------------------- */
    /* PSI auto-derivation — same logic as C codegen.js:853-907                 */
    /* ----------------------------------------------------------------------- */

    function _resolveProtocolSupport(hasCfgMem, isTrainNode, firmwareOn) {

        var psi = [];

        if (hasCfgMem) {
            psi.push('DATAGRAM');
            psi.push('MEMORY_CONFIGURATION');
        }

        psi.push('EVENT_EXCHANGE');
        psi.push('SIMPLE_NODE_INFORMATION');

        if (hasCfgMem) {
            psi.push('ABBREVIATED_DEFAULT_CDI');
            psi.push('CONFIGURATION_DESCRIPTION_INFO');
        }

        if (isTrainNode) {
            psi.push('TRAIN_CONTROL');
            psi.push('FUNCTION_DESCRIPTION');
        }

        if (firmwareOn) {
            psi.push('FIRMWARE_UPGRADE');
        }

        return psi;

    }

    /* ----------------------------------------------------------------------- */
    /* Address space lowest pointer — mirrors C lowAddrSpace selection.         */
    /* Hex literals chosen to match openlcb_defines.h CONFIG_MEM_SPACE_* values:
     *   0x00 = none  0xEF = FIRMWARE  0xF9 = TRAIN_FUNCTION_CONFIG
     *   0xFB = ACDI_USER_ACCESS                                                */
    /* ----------------------------------------------------------------------- */

    function _lowestAddressSpace(isBasic, firmwareOn, isTrainNode) {

        if (isBasic)     { return 0x00; }
        if (firmwareOn)  { return 0xEF; }
        if (isTrainNode) { return 0xF9; }
        return 0xFB;

    }

    /* ----------------------------------------------------------------------- */
    /* Node ID conversion: 'xx.xx.xx.xx.xx.xx' → '0x...n' BigInt literal       */
    /* ----------------------------------------------------------------------- */

    function _nodeIdLiteral(dotted) {

        if (!dotted) { return '0x000000000000n'; }
        var hex = dotted.replace(/\./g, '').toLowerCase();
        return '0x' + hex + 'n';

    }

    /* ----------------------------------------------------------------------- */
    /* Byte array → 'Uint8Array.from([0x3C, 0x3F, ...])' formatted in rows     */
    /* ----------------------------------------------------------------------- */

    function _uint8ArrayLiteral(bytes, indent) {

        var pad = indent || '        ';
        var rows = [];
        var BYTES_PER_ROW = 12;

        for (var i = 0; i < bytes.length; i += BYTES_PER_ROW) {
            var row = [];
            for (var j = i; j < Math.min(i + BYTES_PER_ROW, bytes.length); j++) {
                row.push('0x' + bytes[j].toString(16).toUpperCase().padStart(2, '0'));
            }
            rows.push(pad + row.join(', '));
        }

        return 'Uint8Array.from([\n' + rows.join(',\n') + ',\n    ])';

    }

    /* ----------------------------------------------------------------------- */
    /* JS string escaping for snip values                                       */
    /* ----------------------------------------------------------------------- */

    function _escJs(str) {
        return (str || '').replace(/\\/g, '\\\\').replace(/'/g, "\\'").replace(/\n/g, '\\n');
    }

    /* ----------------------------------------------------------------------- */
    /* openlcb_user_config.js builder                                           */
    /* ----------------------------------------------------------------------- */

    function _generateUserConfigJs(wizardState, codegenState, psiImportPath) {

        var s = codegenState;

        if (s.nodeType === 'bootloader') {
            throw new Error('JS target does not support Bootloader node type');
        }

        var isTrainRole  = s.nodeType === 'train' || s.nodeType === 'train-controller';
        var isTrainNode  = s.nodeType === 'train';
        var isBasic      = s.nodeType === 'basic';
        var hasCfgMem    = !isBasic;
        var firmwareOn   = s.firmware && hasCfgMem;
        var nodeLabel    = s.nodeType === 'train-controller' ? 'Train Controller'
                         : s.nodeType.charAt(0).toUpperCase() + s.nodeType.slice(1);

        /* CDI/FDI bytes are pre-built by _ensureBytes() if user XML is set */
        _ensureBytes(s);

        var snip   = _resolveSnip(s, isBasic, hasCfgMem);
        var psi    = _resolveProtocolSupport(hasCfgMem, isTrainNode, firmwareOn);
        var lowAS  = _lowestAddressSpace(isBasic, firmwareOn, isTrainNode);

        var cdiHighest = (s.cdiBytes && s.cdiBytes.length > 1) ? (s.cdiBytes.length - 1) : 0;
        var fdiHighest = (s.fdiBytes && s.fdiBytes.length > 1) ? (s.fdiBytes.length - 1) : 0;
        var hasCdi     = s.cdiBytes && s.cdiBytes.length > 1;
        var hasFdi     = isTrainNode && s.fdiBytes && s.fdiBytes.length > 1;

        var author = s.projectAuthor || '<YOUR NAME OR COMPANY>';
        var year   = new Date().getFullYear();
        var nodeIdLit = _nodeIdLiteral(s.projectNodeId);

        var L = [];

        /* ---- File header ---- */
        L.push('// ============================================================================');
        L.push('// openlcb_user_config.js  —  ' + nodeLabel + ' Node');
        L.push('// ============================================================================');
        L.push('//');
        L.push('// Generated by Node Wizard.  Mirrors the OpenLcbCLib openlcb_user_config.c');
        L.push('// pattern — same field names, same value semantics — but written as an ESM');
        L.push('// module consumed by openlcb.createNode() in the WASM wrapper.');
        L.push('//');
        L.push('// Copyright (c) ' + year + ', ' + author);
        L.push('// <YOUR LICENSE TEXT HERE>');
        L.push('');
        L.push("import { PSI } from '" + psiImportPath + "';");
        L.push('');

        /* ---- NODE_ID ---- */
        L.push('// ----------------------------------------------------------------------------');
        L.push('// 48-bit OpenLCB Node ID (BigInt literal — note the trailing `n`).');
        L.push('// ----------------------------------------------------------------------------');
        L.push('');
        L.push('export const NODE_ID = ' + nodeIdLit + ';');
        L.push('');

        /* ---- CDI/FDI byte arrays (before the parameters object) ---- */
        if (hasCdi) {
            L.push('// ----------------------------------------------------------------------------');
            L.push('// CDI (Configuration Description Information) — UTF-8 bytes + NUL terminator.');
            L.push('// Mirror of the static const uint8_t _cdi_data[] in openlcb_user_config.c.');
            L.push('// Configuration tools read this as a NUL-terminated string.');
            L.push('// ----------------------------------------------------------------------------');
            L.push('');
            L.push('const _cdi_data = ' + _uint8ArrayLiteral(s.cdiBytes, '    ') + ';');
            L.push('');
        }

        if (hasFdi) {
            L.push('// ----------------------------------------------------------------------------');
            L.push('// FDI (Function Description Information) — UTF-8 bytes + NUL terminator.');
            L.push('// ----------------------------------------------------------------------------');
            L.push('');
            L.push('const _fdi_data = ' + _uint8ArrayLiteral(s.fdiBytes, '    ') + ';');
            L.push('');
        }

        /* ---- OpenLcbUserConfig_node_parameters ---- */
        L.push('// ----------------------------------------------------------------------------');
        L.push('// Node parameters — keys mirror node_parameters_t in openlcb_types.h.');
        L.push('// ----------------------------------------------------------------------------');
        L.push('');
        L.push('export const OpenLcbUserConfig_node_parameters = {');
        L.push('');

        /* 1. snip */
        var snipComment = snip.source ? '  // ' + snip.source : '';
        L.push('    // 1. snip — mfgVersion/userVersion are spec-fixed string counts');
        L.push('    snip: {');
        L.push('        mfgVersion:      4,  // 4 manufacturer strings — fixed by spec');
        L.push("        name:            '" + _escJs(snip.name)  + "'," + snipComment);
        L.push("        model:           '" + _escJs(snip.model) + "'," + snipComment);
        L.push("        hardwareVersion: '" + _escJs(snip.hw)    + "'," + snipComment);
        L.push("        softwareVersion: '" + _escJs(snip.sw)    + "'," + snipComment);
        L.push('        userVersion:     2,  // 2 user strings — fixed by spec');
        L.push('    },');
        L.push('');

        /* 2. protocolSupport */
        L.push('    // 2. protocolSupport — auto-derived from node type and add-ons');
        L.push('    protocolSupport: [');
        psi.forEach(function (name) {
            L.push('        PSI.' + name + ',');
        });
        L.push('    ],');
        L.push('');

        /* 3-4. autocreate counts */
        L.push('    // 3-4. event auto-create counts (internal testing only — leave 0)');
        L.push('    consumerCountAutocreate: 0,');
        L.push('    producerCountAutocreate: 0,');
        L.push('');

        /* 5. configurationOptions */
        L.push('    // 5. configurationOptions — capability flags advertised to peers');
        if (hasCfgMem) {
            L.push('    configurationOptions: {');
            L.push('        readFromManufacturerSpace0xfcSupported: true,');
            L.push('        readFromUserSpace0xfbSupported:         true,');
            L.push('        writeToUserSpace0xfbSupported:          true,');
            L.push('        highestAddressSpace:                    0xFF,');
            L.push('        lowestAddressSpace:                     0x' + lowAS.toString(16).toUpperCase().padStart(2, '0') + ',');
            L.push('    },');
        } else {
            L.push('    configurationOptions: {},');
        }
        L.push('');

        /* 6-13 + 15. Address spaces — all 8 emitted, present toggles */
        _emitAddressSpaceJs(L, 'addressSpaceConfigurationDefinitionInfo', {
            present:        hasCfgMem,
            readOnly:       true,
            highestAddress: hasCdi ? '_cdi_data.length - 1' : '0',
            description:    'Configuration Description Information'
        });

        _emitAddressSpaceJs(L, 'addressSpaceAll', {
            present:        false,
            readOnly:       true,
            highestAddress: '0xFFFFFFFF',
            description:    'All Memory (debug)'
        });

        _emitAddressSpaceJs(L, 'addressSpaceConfigMemory', {
            present:        hasCfgMem,
            readOnly:       false,
            highestAddress: hasCfgMem ? (parseInt(s.configMemHighest) || 0x200) : 0,
            description:    'Configuration Memory'
        });

        _emitAddressSpaceJs(L, 'addressSpaceAcdiManufacturer', {
            present:        hasCfgMem,
            readOnly:       true,
            highestAddress: 124,
            description:    'ACDI manufacturer access'
        });

        _emitAddressSpaceJs(L, 'addressSpaceAcdiUser', {
            present:        hasCfgMem,
            readOnly:       false,
            highestAddress: 127,
            description:    'ACDI user access'
        });

        _emitAddressSpaceJs(L, 'addressSpaceTrainFunctionDefinitionInfo', {
            present:        isTrainNode,
            readOnly:       true,
            highestAddress: hasFdi ? '_fdi_data.length - 1' : '0',
            description:    'Train FDI'
        });

        _emitAddressSpaceJs(L, 'addressSpaceTrainFunctionConfigMemory', {
            present:        isTrainNode,
            readOnly:       false,
            highestAddress: 0,  /* library calculates from train function count at runtime */
            description:    'Train function config memory'
        });

        _emitAddressSpaceJs(L, 'addressSpaceFirmware', {
            present:        firmwareOn,
            readOnly:       false,
            highestAddress: '0xFFFFFFFF',
            description:    'Firmware upgrade'
        });

        /* 14-15. cdi / fdi */
        L.push('    // 14-15. cdi / fdi byte arrays');
        L.push('    cdi: ' + (hasCdi ? '_cdi_data' : 'null') + ',');
        L.push('    fdi: ' + (hasFdi ? '_fdi_data' : 'null') + ',');
        L.push('};');
        L.push('');

        return L.join('\n');

    }

    function _emitAddressSpaceJs(L, key, props) {

        L.push('    ' + key + ': {');
        L.push('        present:         ' + (props.present  ? 'true' : 'false') + ',');
        L.push('        readOnly:        ' + (props.readOnly ? 'true' : 'false') + ',');
        L.push('        lowAddressValid: false,');
        L.push('        highestAddress:  ' + props.highestAddress + ',');
        L.push("        description:     '" + _escJs(props.description) + "',");
        L.push('    },');
        L.push('');

    }

    /* ----------------------------------------------------------------------- */
    /* GETTING_STARTED.txt — JS-flavored                                        */
    /* ----------------------------------------------------------------------- */

    function _buildGettingStarted(wizardState, codegenState, psiImportPath) {

        var nodeLabel = wizardState.selectedNodeType === 'train-controller'
            ? 'Train Controller'
            : (wizardState.selectedNodeType || 'Node').charAt(0).toUpperCase() + (wizardState.selectedNodeType || 'node').slice(1);

        var L = [];

        L.push('================================================================================');
        L.push('  OpenLcbJSLib -- ' + nodeLabel + ' Node Project');
        L.push('  Generated by Node Wizard');
        L.push('================================================================================');
        L.push('');
        L.push('');
        L.push('GETTING STARTED');
        L.push('===============');
        L.push('');
        L.push('1. Place openlcb_user_config.js where your application can import it.');
        L.push('   The default import path is:');
        L.push('');
        L.push("       import { PSI } from '" + psiImportPath + "';");
        L.push('');
        L.push('   If you put the file in a different location, edit that path so it points');
        L.push('   to OpenLcbJSLib/src/openlcb/constants.js relative to this file.');
        L.push('');
        L.push('2. Place cdi.xml (and fdi.xml for train nodes) alongside the .js file or');
        L.push('   wherever your application loads them from.  The byte arrays embedded in');
        L.push('   openlcb_user_config.js are the canonical wire-format representation;');
        L.push('   the XML files are the human-readable source.');
        L.push('');
        L.push('3. In your application code, import and pass the parameters to createNode(),');
        L.push('   then call registerEvents() to register all the producers/consumers you');
        L.push('   selected in the Node Wizard:');
        L.push('');
        L.push("       import { OpenLcb } from 'openlcb-js-lib';");
        L.push("       import { NODE_ID, OpenLcbUserConfig_node_parameters }");
        L.push("           from './openlcb_user_config.js';");
        L.push("       import { registerEvents } from './register_events.js';");
        L.push('');
        L.push("       const lib = await OpenLcb.create({ /* transport options */ });");
        L.push('       const node = lib.createNode(NODE_ID, OpenLcbUserConfig_node_parameters);');
        L.push('       registerEvents(node);');
        L.push('');
        L.push('');
        L.push('FILE LIST');
        L.push('=========');
        L.push('');
        L.push('  ' + nodeLabel + '_project/');
        L.push('  |-- openlcb_user_config.js     ESM module: NODE_ID + node parameters');
        L.push('  |-- register_events.js         Helper: registers Well Known Events + BT setup');
        if (codegenState.cdiBytes && codegenState.cdiBytes.length > 1) {
            L.push('  |-- cdi.xml                    Configuration Description Information (source)');
        }
        if (codegenState.fdiBytes && codegenState.fdiBytes.length > 1 && wizardState.selectedNodeType === 'train') {
            L.push('  |-- fdi.xml                    Function Description Information (source)');
        }
        L.push('  |-- GETTING_STARTED.txt        This file');
        L.push('  |-- <type>_project.json        Node Wizard project (reload to edit)');
        L.push('');
        L.push('');
        L.push('NOTES');
        L.push('=====');
        L.push('');
        L.push('  - mfgVersion (4) and userVersion (2) are SNIP string-count constants');
        L.push('    fixed by the OpenLCB spec.  Do not change them.');
        L.push('');
        L.push('  - All address-space blocks are emitted with explicit "present" flags.');
        L.push('    The wrapper ignores blocks where present is false.');
        L.push('');
        L.push('  - Buffer depths (max producers, consumers, nodes, etc.) are baked into');
        L.push('    the WASM at build time.  They are not tunable from this file.');
        L.push('');
        L.push('  - To regenerate or modify these files, reload the project in Node Wizard');
        L.push('    using the Save/Load Project feature (use the <type>_project.json file).');
        L.push('');

        return L.join('\n');

    }

    /* ----------------------------------------------------------------------- */
    /* register_events.js builder                                               */
    /*                                                                          */
    /* Mirrors C's _register_app_producers() / _register_app_consumers() in     */
    /* generated main.c, but as a single exported registerEvents() function.   */
    /* The application calls it once after creating its node — see header      */
    /* comment in the generated file for usage.                                 */
    /* ----------------------------------------------------------------------- */

    function _generateRegisterEventsJs(wizardState, codegenState, psiImportPath) {

        var s = codegenState;

        var isTrainNode  = s.nodeType === 'train';
        var isTrainCtrl  = s.nodeType === 'train-controller';
        var broadcastOn  = s.broadcast && s.broadcast !== 'none';

        var wke      = s.wellKnownEvents || { producers: [], consumers: [] };
        var wkeMap   = {};
        if (typeof WELL_KNOWN_EVENTS !== 'undefined') {
            WELL_KNOWN_EVENTS.forEach(function (evt) { wkeMap[evt.id] = evt; });
        }

        var year = new Date().getFullYear();

        var L = [];

        /* ---- Header comment with usage instructions ---- */
        L.push('// ============================================================================');
        L.push('// register_events.js  —  Event registration helper');
        L.push('// ============================================================================');
        L.push('//');
        L.push('// HOW TO USE');
        L.push('// ----------');
        L.push('//');
        L.push('// 1. Import the helper next to your other openlcb imports:');
        L.push('//');
        L.push("//      import { OpenLcb } from 'openlcb-js-lib';");
        L.push('//      import { NODE_ID, OpenLcbUserConfig_node_parameters }');
        L.push("//          from './openlcb_user_config.js';");
        L.push("//      import { registerEvents } from './register_events.js';");
        L.push('//');
        L.push('// 2. Call registerEvents() once after creating your node:');
        L.push('//');
        L.push('//      const lib  = await OpenLcb.create({ /* transport options */ });');
        L.push('//      const node = lib.createNode(NODE_ID, OpenLcbUserConfig_node_parameters);');
        L.push('//      registerEvents(node);');
        L.push('//');
        L.push('// 3. After this returns, the node is registered as producer / consumer of');
        L.push('//    every Well Known Event you selected in the Node Wizard, plus any');
        L.push('//    protocol-required defaults for the chosen node type.');
        L.push('//');
        L.push('// Generated by Node Wizard — Copyright (c) ' + year + ', '
            + (s.projectAuthor || '<YOUR NAME OR COMPANY>'));
        L.push('// <YOUR LICENSE TEXT HERE>');
        L.push('');

        /* ---- Imports ---- */
        var importNames = ['Event', 'EventStatus', 'EventRangeCount'];
        if (broadcastOn) { importNames.push('BroadcastTimeClock'); }
        L.push('import { ' + importNames.join(', ') + " } from '" + psiImportPath + "';");
        L.push('');

        /* ---- Function ---- */
        L.push('export function registerEvents(node) {');
        L.push('');

        /* ---- Producers ---- */
        L.push('    // -------------------------------------------------------------------');
        L.push('    // Producers');
        L.push('    // -------------------------------------------------------------------');
        L.push('');

        var prodCount = 0;

        if (isTrainNode) {
            L.push('    // Train node produces "Is Train" to identify itself on the network');
            L.push('    node.registerProducer(Event.IS_TRAIN, EventStatus.EVENT_STATUS_SET);');
            prodCount++;
        } else if (isTrainCtrl) {
            L.push('    // Throttle produces emergency events to stop/resume all trains');
            L.push('    node.registerProducer(Event.EMERGENCY_STOP,       EventStatus.EVENT_STATUS_UNKNOWN);');
            L.push('    node.registerProducer(Event.CLEAR_EMERGENCY_STOP, EventStatus.EVENT_STATUS_UNKNOWN);');
            L.push('    node.registerProducer(Event.EMERGENCY_OFF,        EventStatus.EVENT_STATUS_UNKNOWN);');
            L.push('    node.registerProducer(Event.CLEAR_EMERGENCY_OFF,  EventStatus.EVENT_STATUS_UNKNOWN);');
            prodCount += 4;
        }

        if (broadcastOn) {
            if (prodCount > 0) { L.push(''); }
            L.push('    // Broadcast Time: 2 producer ranges + 2 consumer ranges are');
            L.push('    // registered automatically by node.broadcastTime.setup' +
                (s.broadcast === 'producer' ? 'Producer' : 'Consumer') + '() at the end.');
            prodCount++;
        }

        if (wke.producers.length > 0) {
            if (prodCount > 0) { L.push(''); }
            L.push('    // Well Known Event producers (selected in Node Wizard)');
            wke.producers.forEach(function (id) {
                _emitWkeRegistration(L, wkeMap[id], 'producer');
            });
            prodCount += wke.producers.length;
        }

        if (prodCount === 0) {
            L.push('    // TODO: Register application-specific produced events');
            L.push('    // node.registerProducer(yourEventId, EventStatus.UNKNOWN);');
        }

        L.push('');

        /* ---- Consumers ---- */
        L.push('    // -------------------------------------------------------------------');
        L.push('    // Consumers');
        L.push('    // -------------------------------------------------------------------');
        L.push('');

        var consCount = 0;

        if (isTrainNode) {
            L.push('    // Train node consumes global emergency events');
            L.push('    node.registerConsumer(Event.EMERGENCY_OFF,        EventStatus.EVENT_STATUS_SET);');
            L.push('    node.registerConsumer(Event.CLEAR_EMERGENCY_OFF,  EventStatus.EVENT_STATUS_SET);');
            L.push('    node.registerConsumer(Event.EMERGENCY_STOP,       EventStatus.EVENT_STATUS_SET);');
            L.push('    node.registerConsumer(Event.CLEAR_EMERGENCY_STOP, EventStatus.EVENT_STATUS_SET);');
            consCount += 4;
        }

        if (broadcastOn) {
            if (consCount > 0) { L.push(''); }
            L.push('    // Broadcast Time consumer ranges registered by setup' +
                (s.broadcast === 'producer' ? 'Producer' : 'Consumer') + '() below.');
            consCount++;
        }

        if (wke.consumers.length > 0) {
            if (consCount > 0) { L.push(''); }
            L.push('    // Well Known Event consumers (selected in Node Wizard)');
            wke.consumers.forEach(function (id) {
                _emitWkeRegistration(L, wkeMap[id], 'consumer');
            });
            consCount += wke.consumers.length;
        }

        if (consCount === 0) {
            L.push('    // TODO: Register application-specific consumed events');
            L.push('    // node.registerConsumer(yourEventId, EventStatus.UNKNOWN);');
        }

        L.push('');

        /* ---- Broadcast Time setup ---- */
        if (broadcastOn) {
            L.push('    // -------------------------------------------------------------------');
            L.push('    // Broadcast Time setup');
            L.push('    // -------------------------------------------------------------------');
            L.push('');
            L.push('    // Sets up the default fast clock — the underlying call also');
            L.push('    // registers the BT producer/consumer event ranges.  Switch to');
            L.push('    // BroadcastTimeClock.DEFAULT_REALTIME or ALTERNATE_1/2 if needed.');
            if (s.broadcast === 'producer') {
                L.push('    node.broadcastTime.setupProducer(BroadcastTimeClock.DEFAULT_FAST);');
            } else {
                L.push('    node.broadcastTime.setupConsumer(BroadcastTimeClock.DEFAULT_FAST);');
            }
            L.push('');
        }

        L.push('}');
        L.push('');

        return L.join('\n');

    }

    /* Emit a single producer/consumer registration line for a WKE entry. */
    function _emitWkeRegistration(L, evt, role) {

        if (!evt) { return; }

        var jsName = WKE_TO_JS_EVENT[evt.id];

        if (!jsName) {
            /* No JS Event constant yet — leave a clear marker for the user. */
            L.push('    // TODO: ' + evt.define + ' is not yet exported from the JS lib Event enum');
            L.push('    //       (raw 64-bit event id needed).  Add the constant to');
            L.push('    //       src/openlcb/constants.js or pass the BigInt directly.');
            return;
        }

        if (evt.range) {
            /* Ranges: count enum varies by event; default to 1 here, user can adjust. */
            var fnName = role === 'producer' ? 'registerProducerRange' : 'registerConsumerRange';
            L.push('    node.' + fnName + '(Event.' + jsName + ', EventRangeCount.EVENT_RANGE_COUNT_1);  // adjust range count if needed');
        } else {
            var fn2 = role === 'producer' ? 'registerProducer' : 'registerConsumer';
            L.push('    node.' + fn2 + '(Event.' + jsName + ', EventStatus.EVENT_STATUS_UNKNOWN);');
        }

    }

    /* ----------------------------------------------------------------------- */
    /* Project label (used for filenames) — mirror of CTarget.projectLabel      */
    /* ----------------------------------------------------------------------- */

    function projectLabel(wizardState) {

        var projName = (wizardState.configFormState && wizardState.configFormState.projectName)
            ? wizardState.configFormState.projectName.replace(/\s+/g, '_').replace(/[^a-zA-Z0-9_\-]/g, '')
            : '';

        return projName || (wizardState.selectedNodeType === 'train-controller'
            ? 'train_controller'
            : wizardState.selectedNodeType);

    }

    /* ----------------------------------------------------------------------- */
    /* buildFiles                                                                */
    /* ----------------------------------------------------------------------- */

    function buildFiles(wizardState) {

        if (wizardState.selectedNodeType === 'bootloader') {
            throw new Error('JS target does not support Bootloader node type');
        }

        var codegenState = _buildCodegenState(wizardState);

        /* PSI import path — wizard field with documented default */
        var psiImportPath = (wizardState.jsTarget && wizardState.jsTarget.psiImportPath)
            ? wizardState.jsTarget.psiImportPath
            : DEFAULT_PSI_IMPORT;

        var entries = [];

        entries.push({
            path:    'openlcb_user_config.js',
            content: _generateUserConfigJs(wizardState, codegenState, psiImportPath)
        });

        entries.push({
            path:    'register_events.js',
            content: _generateRegisterEventsJs(wizardState, codegenState, psiImportPath)
        });

        if (wizardState.cdiUserXml && wizardState.cdiUserXml.trim()) {
            entries.push({ path: 'cdi.xml', content: wizardState.cdiUserXml });
        }

        if (wizardState.selectedNodeType === 'train' && wizardState.fdiUserXml && wizardState.fdiUserXml.trim()) {
            entries.push({ path: 'fdi.xml', content: wizardState.fdiUserXml });
        }

        entries.push({
            path:        'GETTING_STARTED.txt',
            content:     _buildGettingStarted(wizardState, codegenState, psiImportPath),
            previewable: false
        });

        var label = projectLabel(wizardState);
        entries.push({
            path:        (label || 'node') + '_project.json',
            content:     JSON.stringify(wizardState, null, 2),
            previewable: false
        });

        return entries;

    }

    /* ----------------------------------------------------------------------- */
    /* renderByteArray — used by the CDI/FDI editors' "Array" view             */
    /* ----------------------------------------------------------------------- */

    function renderByteArray(rows, kind) {

        var lower = (kind || 'cdi').toLowerCase();
        var upper = lower.toUpperCase();
        var lines = [];
        var total = 0;

        lines.push('// ' + upper + ' byte array.');
        lines.push('// Paste this into openlcb_user_config.js as the value of _' + lower + '_data,');
        lines.push('// then reference it from OpenLcbUserConfig_node_parameters.' + lower + '.');
        lines.push('//');
        lines.push('// NOTE: THIS IS FOR CONVENIENCE OF THOSE MANUALLY BUILDING THEIR openlcb_user_config.js FILE.');
        lines.push('');
        lines.push('const _' + lower + '_data = Uint8Array.from([');

        rows.forEach(function (row) {
            var line = '';
            row.bytes.forEach(function (b) {
                line += '0x' + b.toString(16).toUpperCase().padStart(2, '0') + ', ';
                total++;
            });
            line += '  // ' + row.comment;
            lines.push(line);
        });

        lines.push(']);');
        lines.push('');
        lines.push('// Total bytes (including null terminator): ' + total);

        return { text: lines.join('\n'), totalBytes: total };

    }

    return {
        id:               'js',
        label:            'JavaScript (OpenLcbJSLib)',
        applicablePanels: ['config', 'cdi', 'fdi', 'file-preview'],
        buildFiles:       buildFiles,
        projectLabel:     projectLabel,
        renderByteArray:  renderByteArray
    };

}());

if (typeof LanguageTargets !== 'undefined') {
    LanguageTargets.register(JsTarget);
}
