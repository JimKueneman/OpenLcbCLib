/* =========================================================================
 * zip_export.js  —  Generate a complete project ZIP with all files
 *
 * Two folder layouts depending on platform:
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
 *                     DriverCodegen, generateH, generateC, generateMain
 * ========================================================================= */

var ZipExport = (function () {

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
    /* Main export function                                                     */
    /* ----------------------------------------------------------------------- */

    function generateZip(wizardState) {

        if (!wizardState.selectedNodeType) {

            alert('No node type selected -- cannot generate files.');
            return;

        }

        if (typeof JSZip === 'undefined') {

            alert('JSZip library not loaded. Please check your internet connection and reload.');
            return;

        }

        var codegenState = _buildCodegenState(wizardState);

        /* ---- Arduino mode ---- */
        var isArduino  = !!wizardState.arduino;
        var mainFilename = isArduino ? 'main.ino' : 'main.c';

        /* ---- Core config files ---- */
        var configH    = _fixConfigIncludes(generateH(codegenState), isArduino);
        var configC    = _fixConfigIncludes(generateC(codegenState), isArduino);
        var mainC      = _fixMainIncludes(generateMain(codegenState), isArduino);

        /* ---- Driver files ---- */
        var activeDrivers = _getActiveDriverGroups(codegenState);

        /* ---- Callback files ---- */
        var activeCallbacks = _getActiveCallbackGroups(codegenState);

        /* ---- Label for filenames ---- */
        var projName = (wizardState.configFormState && wizardState.configFormState.projectName)
            ? wizardState.configFormState.projectName.replace(/\s+/g, '_').replace(/[^a-zA-Z0-9_\-]/g, '')
            : '';
        var nodeLabel = projName || (wizardState.selectedNodeType === 'train-controller'
            ? 'train_controller'
            : wizardState.selectedNodeType);

        /* ---- Build ZIP ---- */
        var zip = new JSZip();

        /*
         * Arduino requires all .c/.h source files under a src/ folder.
         * Non-Arduino: everything sits at the project root.
         */
        var baseFolder = isArduino ? zip.folder('src') : zip;

        /* Root files */
        zip.file(mainFilename, mainC);
        zip.file('openlcb_user_config.h', configH);
        zip.file('openlcb_user_config.c', configC);
        zip.file('can_user_config.h', generateCanH(codegenState));

        /* Arduino uses .cpp for driver/callback source files so users can
         * call Serial and other C++ Arduino APIs in their implementations. */
        var srcExt = isArduino ? '.cpp' : '.c';

        /* Driver files under {base}/application_drivers/ */
        var driversFolder = baseFolder.folder('application_drivers');

        activeDrivers.forEach(function (entry) {

            var hCode = DriverCodegen.generateH(entry.group, entry.functions, wizardState.platformState);
            var cCode = DriverCodegen.generateC(entry.group, entry.functions, wizardState.platformState, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            driversFolder.file(entry.group.filePrefix + '.h', hCode);
            driversFolder.file(entry.group.filePrefix + srcExt, cCode);

        });

        /* Callback files under {base}/application_callbacks/ */
        var callbacksFolder = baseFolder.folder('application_callbacks');

        activeCallbacks.forEach(function (entry) {

            var hCode = CallbackCodegen.generateH(entry.group, entry.functions);
            var cCode = CallbackCodegen.generateC(entry.group, entry.functions, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            callbacksFolder.file(entry.group.filePrefix + '.h', hCode);
            callbacksFolder.file(entry.group.filePrefix + srcExt, cCode);

        });

        /* XML files under {base}/xml_files/ — bootloader has no CDI or FDI */
        if (wizardState.selectedNodeType !== 'bootloader') {

            var xmlFolder = baseFolder.folder('xml_files');

            if (wizardState.cdiUserXml && wizardState.cdiUserXml.trim()) {
                xmlFolder.file('cdi.xml', wizardState.cdiUserXml);
            }

            if (wizardState.fdiUserXml && wizardState.fdiUserXml.trim()) {
                xmlFolder.file('fdi.xml', wizardState.fdiUserXml);
            }

        }

        /* Placeholder library folders under {base}/openlcb_c_lib/ */
        var libFolder = baseFolder.folder('openlcb_c_lib');
        libFolder.folder('openlcb');
        libFolder.folder('drivers/canbus');
        libFolder.folder('utilities');

        /* Getting Started document */
        zip.file('GETTING_STARTED.txt', _buildGettingStarted(wizardState, codegenState, mainFilename, isArduino));

        /* Project file — allows reloading this configuration in Node Wizard */
        var projectJson = JSON.stringify(wizardState, null, 2);
        zip.file((nodeLabel || 'node') + '_project.json', projectJson);

        /* ---- Trigger download ---- */
        var zipFilename = (nodeLabel || 'node') + '_project.zip';

        zip.generateAsync({ type: 'blob' }).then(function (blob) {

            var url = URL.createObjectURL(blob);
            var a   = document.createElement('a');
            a.href     = url;
            a.download = zipFilename;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);

        }).catch(function (err) {

            alert('Error generating ZIP: ' + err.message);

        });

    }

    /* ----------------------------------------------------------------------- */
    return { generateZip: generateZip };

}());
