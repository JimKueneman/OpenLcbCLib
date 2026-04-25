/* =========================================================================
 * c_target.js  —  C language target for the wizard.
 *
 * Implements the LanguageTarget contract: emits openlcb_user_config.h/.c,
 * can_user_config.h, main.c/.ino, driver/callback stubs, GETTING_STARTED.txt,
 * and the project JSON.  Two folder layouts depending on platform:
 *
 * NON-ARDUINO (flat):                    ARDUINO (src/ wrapper):
 *
 *   <project>/                             <project>/
 *   |-- main.c                             |-- main.ino
 *   |-- openlcb_user_config.h              |-- openlcb_user_config.h
 *   |-- openlcb_user_config.c              |-- openlcb_user_config.c
 *   |-- can_user_config.h                  |-- can_user_config.h
 *   |-- application_callbacks/             |-- src/
 *   |   |-- callbacks_*.h / .c             |   |-- application_callbacks/
 *   |-- application_drivers/               |   |-- application_drivers/
 *   |   |-- openlcb_can_drivers.h / .c     |   |-- xml_files/
 *   |-- xml_files/                         |   |-- openlcb_c_lib/
 *   |   |-- cdi.xml                        |-- GETTING_STARTED.txt
 *   |   |-- fdi.xml                        |-- <type>_project.json
 *   |-- openlcb_c_lib/
 *   |   |-- openlcb/
 *   |   |-- drivers/canbus/
 *   |   |-- utilities/
 *   |-- GETTING_STARTED.txt
 *   |-- <type>_project.json
 *
 * Include path convention (all relative to file location):
 *
 *   NON-ARDUINO main.c / config (root):
 *     #include "openlcb_c_lib/openlcb/..."
 *     #include "application_drivers/..."
 *     #include "application_callbacks/..."
 *
 *   ARDUINO main.ino / config (root):
 *     #include "src/openlcb_c_lib/openlcb/..."
 *     #include "src/application_drivers/..."
 *     #include "src/application_callbacks/..."
 *
 *   application_drivers/* and application_callbacks/* (both modes):
 *     #include "../openlcb_c_lib/openlcb/..."
 *     #include "../openlcb_c_lib/drivers/canbus/..."
 *
 * Depends on globals: CALLBACK_GROUPS, CallbackCodegen, DRIVER_GROUPS,
 *                     DriverCodegen, generateH, generateC, generateMain,
 *                     generateCanH, LanguageTargets
 * ========================================================================= */

var CTarget = (function () {

    'use strict';

    /* ----------------------------------------------------------------------- */
    /* Include path fixup helpers                                               */
    /* ----------------------------------------------------------------------- */

    /**
     * Fix includes in main.c / main.ino.
     * Arduino: prefix with "src/", non-Arduino: no prefix.
     */
    function _fixMainIncludes(code, isArduino) {

        var prefix = isArduino ? 'src/' : '';

        /* Library includes: "src/openlcb/..." or "src/drivers/..." or "src/utilities/..." → "{prefix}openlcb_c_lib/..." */
        code = code.replace(
            /#include "src\/(openlcb|drivers|utilities)\//g,
            '#include "' + prefix + 'openlcb_c_lib/$1/'
        );

        /* Driver includes: bare name → {prefix}application_drivers/ */
        code = code.replace(
            /#include "openlcb_can_drivers\.h"/g,
            '#include "' + prefix + 'application_drivers/openlcb_can_drivers.h"'
        );
        code = code.replace(
            /#include "openlcb_drivers\.h"/g,
            '#include "' + prefix + 'application_drivers/openlcb_drivers.h"'
        );

        /* Callback includes: bare name → {prefix}application_callbacks/ */
        code = code.replace(
            /#include "(callbacks_\w+\.h)"/g,
            '#include "' + prefix + 'application_callbacks/$1"'
        );

        return code;

    }

    /**
     * Fix includes in driver/callback files.
     * These live under application_drivers/ or application_callbacks/.
     * Library headers use "src/openlcb/..." in defs — need "../openlcb_c_lib/..."
     * (same for both Arduino and non-Arduino since the relative path is identical).
     */
    function _fixSubfolderIncludes(code) {

        code = code.replace(
            /#include "src\/(openlcb|drivers|utilities)\//g,
            '#include "../openlcb_c_lib/$1/'
        );

        return code;

    }

    /**
     * Fix includes in openlcb_user_config.h / .c.
     * These sit at project root. Arduino: "src/openlcb_c_lib/...", else: "openlcb_c_lib/..."
     */
    function _fixConfigIncludes(code, isArduino) {

        var prefix = isArduino ? 'src/' : '';

        code = code.replace(
            /#include "src\/(openlcb|drivers|utilities)\//g,
            '#include "' + prefix + 'openlcb_c_lib/$1/'
        );

        return code;

    }

    /* ----------------------------------------------------------------------- */
    /* Build the state object that codegen.js expects                           */
    /* ----------------------------------------------------------------------- */

    function _buildCodegenState(wizardState) {

        var s = {};

        if (wizardState.configFormState) {

            Object.keys(wizardState.configFormState).forEach(function (key) {
                s[key] = wizardState.configFormState[key];
            });

        }

        s.nodeType       = wizardState.selectedNodeType;
        s.driverState    = wizardState.driverState    || {};
        s.callbackState  = wizardState.callbackState  || {};
        s.platformState  = wizardState.platformState   || null;
        s.cdiUserXml         = wizardState.cdiUserXml || null;
        s.fdiUserXml         = wizardState.fdiUserXml || null;
        s.preserveWhitespace = !!wizardState.preserveWhitespace;

        return s;

    }

    /* ----------------------------------------------------------------------- */
    /* Determine which callback/driver groups have content to generate          */
    /* ----------------------------------------------------------------------- */

    function _getActiveCallbackGroups(state) {

        var active = [];
        var groupKeys = Object.keys(CALLBACK_GROUPS);

        for (var i = 0; i < groupKeys.length; i++) {

            var key   = groupKeys[i];
            var group = CALLBACK_GROUPS[key];
            var cs = (state.callbackState && state.callbackState[key]) ? state.callbackState[key] : null;
            var checkedNames = (cs && cs.checked) ? cs.checked : [];

            var activeFns = [];
            for (var j = 0; j < group.functions.length; j++) {

                var fn = group.functions[j];
                if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
                    activeFns.push(fn);
                }

            }

            if (activeFns.length > 0) {
                active.push({ key: key, group: group, functions: activeFns });
            }

        }

        return active;

    }

    function _getActiveDriverGroups(state) {

        var active = [];
        var groupKeys = Object.keys(DRIVER_GROUPS);
        var isBootloader = state.nodeType === 'bootloader';

        for (var i = 0; i < groupKeys.length; i++) {

            var key   = groupKeys[i];
            var group = DRIVER_GROUPS[key];
            var ds = (state.driverState && state.driverState[key]) ? state.driverState[key] : null;
            var checkedNames = (ds && ds.checked) ? ds.checked : [];

            var activeFns = [];
            for (var j = 0; j < group.functions.length; j++) {

                var fn = group.functions[j];
                if (isBootloader && (fn.name === 'config_mem_read' || fn.name === 'config_mem_write')) { continue; }
                if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
                    activeFns.push(fn);
                }

            }

            if (activeFns.length > 0) {
                active.push({ key: key, group: group, functions: activeFns });
            }

        }

        return active;

    }

    /* ----------------------------------------------------------------------- */
    /* Getting Started document                                                 */
    /* ----------------------------------------------------------------------- */

    function _buildGettingStarted(wizardState, codegenState, mainFilename, isArduino) {

        var nodeLabel = wizardState.selectedNodeType === 'train-controller'
            ? 'Train Controller'
            : (wizardState.selectedNodeType || 'Node').charAt(0).toUpperCase() + (wizardState.selectedNodeType || 'node').slice(1);

        var L = [];

        L.push('================================================================================');
        L.push('  OpenLcbCLib -- ' + nodeLabel + ' Node Project');
        L.push('  Generated by Node Wizard');
        L.push('================================================================================');
        L.push('');
        L.push('');
        L.push('GETTING STARTED');
        L.push('===============');
        L.push('');
        var p = isArduino ? 'src/' : '';   /* path prefix for folder references */

        L.push('1. Copy the OpenLcbCLib library source into the ' + p + 'openlcb_c_lib/ folder:');
        L.push('');
        L.push('     ' + p + 'openlcb_c_lib/openlcb/         <-- core library (.c/.h files)');
        L.push('     ' + p + 'openlcb_c_lib/drivers/canbus/  <-- CAN bus transport layer');
        L.push('     ' + p + 'openlcb_c_lib/utilities/       <-- helper utilities');
        L.push('');
        L.push('2. Open ' + mainFilename + ' -- this is your application entry point.');
        L.push('   It wires the driver and callback functions into the library');
        L.push('   configuration structs and runs the main loop.');
        L.push('');
        L.push('3. Implement the driver stubs in ' + p + 'application_drivers/.');
        L.push('   These connect the library to your hardware (CAN controller,');
        L.push('   EEPROM/flash for config memory, etc.).');
        L.push('   At minimum: CAN transmit/receive, lock/unlock shared resources,');
        L.push('   and the 100ms timer are critical for basic operation.');
        L.push('');
        L.push('4. Implement the callback stubs in ' + p + 'application_callbacks/.');
        L.push('   These are where your application logic goes -- responding to');
        L.push('   events, handling configuration changes, etc.');
        L.push('');
        L.push('5. Build and flash to your target hardware.');
        L.push('');
        L.push('');
        L.push('FOLDER STRUCTURE');
        L.push('================');
        L.push('');
        L.push('  ' + nodeLabel + '_project/');
        L.push('  |');
        var pad = '                                        '.slice(0, 40 - mainFilename.length);
        L.push('  |-- ' + mainFilename + pad + 'Application entry point');
        L.push('  |-- openlcb_user_config.h              Feature flags and node parameters');
        L.push('  |-- openlcb_user_config.c              Node parameters struct (const data)');
        L.push('  |-- can_user_config.h                  CAN bus driver configuration');
        L.push('  |');

        if (isArduino) {

            L.push('  |-- src/');
            L.push('  |   |-- application_drivers/');
            L.push('  |   |   |-- openlcb_can_drivers.h      CAN bus hardware interface');
            L.push('  |   |   |-- openlcb_can_drivers.cpp');
            L.push('  |   |   |-- openlcb_drivers.h          Platform drivers (memory, reboot, etc.)');
            L.push('  |   |   |-- openlcb_drivers.cpp');
            L.push('  |   |');
            L.push('  |   |-- application_callbacks/');
            L.push('  |   |   |-- callbacks_*.h / .cpp       Application callback stubs');
            L.push('  |   |');
            L.push('  |   |-- xml_files/');
            L.push('  |   |   |-- cdi.xml                    Configuration Description Information');
            L.push('  |   |   |-- fdi.xml                    Function Description (train nodes)');
            L.push('  |   |');
            L.push('  |   |-- openlcb_c_lib/                 <-- Copy OpenLcbCLib library here');
            L.push('  |       |-- openlcb/                   Core library');
            L.push('  |       |-- drivers/canbus/            CAN transport layer');
            L.push('  |       |-- utilities/                 Helper utilities');
            L.push('  |');
            L.push('  NOTE: All subfolders are under src/ for Arduino IDE compatibility.');

        } else {

            L.push('  |-- application_drivers/');
            L.push('  |   |-- openlcb_can_drivers.h          CAN bus hardware interface');
            L.push('  |   |-- openlcb_can_drivers.c');
            L.push('  |   |-- openlcb_drivers.h              Platform drivers (memory, reboot, etc.)');
            L.push('  |   |-- openlcb_drivers.c');
            L.push('  |');
            L.push('  |-- application_callbacks/');
            L.push('  |   |-- callbacks_*.h / .c             Application callback stubs');
            L.push('  |');
            L.push('  |-- xml_files/');
            L.push('  |   |-- cdi.xml                        Configuration Description Information');
            L.push('  |   |-- fdi.xml                        Function Description (train nodes)');
            L.push('  |');
            L.push('  |-- openlcb_c_lib/                     <-- Copy OpenLcbCLib library here');
            L.push('  |   |-- openlcb/                       Core library');
            L.push('  |   |-- drivers/canbus/                CAN transport layer');
            L.push('  |   |-- utilities/                     Helper utilities');
            L.push('  |');

        }

        L.push('  |-- GETTING_STARTED.txt                This file');
        L.push('  |-- <type>_project.json                Node Wizard project (reload to edit)');
        L.push('');
        L.push('');
        L.push('INCLUDE PATH CONVENTION');
        L.push('=======================');
        L.push('');
        L.push('All #include paths are relative to the file that contains them.');
        L.push('No special compiler -I flags are required.');
        L.push('');
        L.push('  From ' + mainFilename + ' (project root):');
        L.push('    #include "' + p + 'openlcb_c_lib/openlcb/openlcb_config.h"');
        L.push('    #include "' + p + 'application_drivers/openlcb_can_drivers.h"');
        L.push('    #include "' + p + 'application_callbacks/callbacks_events.h"');
        L.push('');
        L.push('  From ' + p + 'application_drivers/ or ' + p + 'application_callbacks/:');
        L.push('    #include "../openlcb_c_lib/openlcb/openlcb_types.h"');
        L.push('    #include "../openlcb_c_lib/drivers/canbus/can_types.h"');
        L.push('');
        L.push('');
        L.push('WHAT TO IMPLEMENT');
        L.push('=================');
        L.push('');
        L.push('Every generated .c file contains TODO comments where you need to add');
        L.push('your platform-specific code. Search for "TODO" to find them all.');
        L.push('');
        L.push('The most important files to implement first:');
        L.push('');
        L.push('  1. src/application_drivers/openlcb_can_drivers.c');
        L.push('     Set up your CAN controller, implement transmit and receive.');
        L.push('');
        L.push('  2. src/application_drivers/openlcb_drivers.c');
        L.push('     Implement config memory read/write (EEPROM, flash, etc.).');
        L.push('');
        L.push('  3. src/application_callbacks/ (your chosen callbacks)');
        L.push('     Add your application logic for events, configuration, etc.');
        L.push('');
        L.push('');
        L.push('NODE CONFIGURATION');
        L.push('==================');
        L.push('');
        L.push('  Node type:     ' + nodeLabel);

        if (codegenState.snipName) {
            L.push('  SNIP name:     ' + codegenState.snipName);
        }
        if (codegenState.snipModel) {
            L.push('  SNIP model:    ' + codegenState.snipModel);
        }

        var plat = wizardState.platformState;
        if (plat && plat.platform && plat.platform !== 'none') {
            L.push('  Platform:      ' + plat.platform);
        }

        L.push('');
        L.push('  To regenerate or modify these files, reload the project in Node Wizard');
        L.push('  using the Save/Load Project feature (use the <type>_project.json file).');
        L.push('');

        return L.join('\n');

    }

    /* ----------------------------------------------------------------------- */
    /* Project label (used for filenames)                                       */
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
    /* buildFiles — produce the entry list for the ZIP                          */
    /* ----------------------------------------------------------------------- */

    function buildFiles(wizardState) {

        var codegenState = _buildCodegenState(wizardState);

        var isArduino    = !!wizardState.arduino;
        var mainFilename = isArduino ? 'main.ino' : 'main.c';
        var srcExt       = isArduino ? '.cpp' : '.c';
        var basePrefix   = isArduino ? 'src/' : '';

        var configH = _fixConfigIncludes(generateH(codegenState), isArduino);
        var configC = _fixConfigIncludes(generateC(codegenState), isArduino);
        var mainC   = _fixMainIncludes(generateMain(codegenState), isArduino);

        var entries = [];

        /* Root files */
        entries.push({ path: mainFilename,             content: mainC });
        entries.push({ path: 'openlcb_user_config.h',  content: configH });
        entries.push({ path: 'openlcb_user_config.c',  content: configC });
        entries.push({ path: 'can_user_config.h',      content: generateCanH(codegenState) });

        /* Driver files under {base}/application_drivers/ */
        var activeDrivers = _getActiveDriverGroups(codegenState);

        activeDrivers.forEach(function (entry) {

            var hCode = DriverCodegen.generateH(entry.group, entry.functions, wizardState.platformState);
            var cCode = DriverCodegen.generateC(entry.group, entry.functions, wizardState.platformState, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            entries.push({ path: basePrefix + 'application_drivers/' + entry.group.filePrefix + '.h',     content: hCode });
            entries.push({ path: basePrefix + 'application_drivers/' + entry.group.filePrefix + srcExt, content: cCode });

        });

        /* Callback files under {base}/application_callbacks/ */
        var activeCallbacks = _getActiveCallbackGroups(codegenState);

        activeCallbacks.forEach(function (entry) {

            var hCode = CallbackCodegen.generateH(entry.group, entry.functions);
            var cCode = CallbackCodegen.generateC(entry.group, entry.functions, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            entries.push({ path: basePrefix + 'application_callbacks/' + entry.group.filePrefix + '.h',     content: hCode });
            entries.push({ path: basePrefix + 'application_callbacks/' + entry.group.filePrefix + srcExt, content: cCode });

        });

        /* XML files under {base}/xml_files/ — bootloader has no CDI or FDI.
         * Folder is created unconditionally for non-bootloader to mirror the
         * pre-refactor behavior (xml_files/ was always added even when empty). */
        if (wizardState.selectedNodeType !== 'bootloader') {

            entries.push({ path: basePrefix + 'xml_files', dir: true });

            if (wizardState.cdiUserXml && wizardState.cdiUserXml.trim()) {
                entries.push({ path: basePrefix + 'xml_files/cdi.xml', content: wizardState.cdiUserXml });
            }

            if (wizardState.fdiUserXml && wizardState.fdiUserXml.trim()) {
                entries.push({ path: basePrefix + 'xml_files/fdi.xml', content: wizardState.fdiUserXml });
            }

        }

        /* Placeholder library folders under {base}/openlcb_c_lib/ */
        entries.push({ path: basePrefix + 'openlcb_c_lib',               dir: true });
        entries.push({ path: basePrefix + 'openlcb_c_lib/openlcb',       dir: true });
        entries.push({ path: basePrefix + 'openlcb_c_lib/drivers/canbus', dir: true });
        entries.push({ path: basePrefix + 'openlcb_c_lib/utilities',     dir: true });

        /* Getting Started document — meta file, not shown in file preview */
        entries.push({ path: 'GETTING_STARTED.txt', content: _buildGettingStarted(wizardState, codegenState, mainFilename, isArduino), previewable: false });

        /* Project file — allows reloading this configuration in Node Wizard */
        var nodeLabel = projectLabel(wizardState);
        entries.push({ path: (nodeLabel || 'node') + '_project.json', content: JSON.stringify(wizardState, null, 2), previewable: false });

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
        lines.push('// Paste this into the .' + lower + ' field of the node_parameters_t struct');
        lines.push("// in openlcb_user_config.c to define the node's " +
            (lower === 'cdi' ? 'configuration layout' : 'train function descriptions') + '.');
        lines.push('//');
        lines.push('// NOTE: THIS IS FOR CONVENIENCE OF THOSE MANUALLY BUILDING THEIR OPENLCB_USER_CONFIG.C FILE.');
        lines.push('');
        lines.push('.' + lower + ' = {');

        rows.forEach(function (row) {
            var line = '';
            row.bytes.forEach(function (b) {
                line += '0x' + b.toString(16).toUpperCase().padStart(2, '0') + ', ';
                total++;
            });
            line += '  // ' + row.comment;
            lines.push(line);
        });

        lines.push('},');
        lines.push('');
        lines.push('#define USER_' + upper + '_ARRAY_SIZE  ' + total +
            '  // Total bytes including null terminator');

        return { text: lines.join('\n'), totalBytes: total };

    }

    return {
        id:               'c',
        label:            'C (OpenLcbCLib)',
        applicablePanels: ['config', 'cdi', 'fdi', 'platform-drivers', 'callbacks', 'file-preview'],
        buildFiles:       buildFiles,
        projectLabel:     projectLabel,
        renderByteArray:  renderByteArray
    };

}());

if (typeof LanguageTargets !== 'undefined') {
    LanguageTargets.register(CTarget);
}
