/* =========================================================================
 * zip_export.js  —  Generate a complete project ZIP with all files
 *
 * Arduino-compatible folder structure (all subfolders under src/):
 *
 *   <project>/
 *   |-- main.c
 *   |-- openlcb_user_config.h
 *   |-- openlcb_user_config.c
 *   |-- src/
 *   |   |-- application_callbacks/
 *   |   |   |-- callbacks_*.h / .c     (only groups with checked functions)
 *   |   |-- application_drivers/
 *   |   |   |-- openlcb_can_drivers.h / .c
 *   |   |   |-- openlcb_drivers.h / .c
 *   |   |-- xml_files/
 *   |   |   |-- cdi.xml                (if CDI loaded)
 *   |   |   |-- fdi.xml                (if FDI loaded, train only)
 *   |   |-- src/                       <-- Copy OpenLcbCLib library here
 *   |       |-- openlcb/
 *   |       |-- drivers/canbus/
 *   |       |-- utilities/
 *   |-- GETTING_STARTED.txt
 *   |-- <type>_project.json
 *
 * Include path convention (all relative to file location):
 *   main.c (root):
 *     #include "src/src/openlcb/..."
 *     #include "src/application_drivers/..."
 *     #include "src/application_callbacks/..."
 *
 *   src/application_drivers/* and src/application_callbacks/*:
 *     #include "../src/openlcb/..."
 *     #include "../src/drivers/canbus/..."
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
     * Fix includes in main.c:
     *   - Library: "src/openlcb/..." → "src/src/openlcb/..."
     *             "src/drivers/..." → "src/src/drivers/..."
     *   - Drivers: bare "openlcb_can_drivers.h" → "src/application_drivers/openlcb_can_drivers.h"
     *   - Callbacks: bare "callbacks_*.h" → "src/application_callbacks/callbacks_*.h"
     */
    function _fixMainIncludes(code) {

        /* Library includes: "src/" → "src/src/" (library is nested one level deeper) */
        code = code.replace(
            /#include "src\/(openlcb|drivers)\//g,
            '#include "src/src/$1/'
        );

        /* Driver includes: bare name → src/application_drivers/ */
        code = code.replace(
            /#include "openlcb_can_drivers\.h"/g,
            '#include "src/application_drivers/openlcb_can_drivers.h"'
        );
        code = code.replace(
            /#include "openlcb_drivers\.h"/g,
            '#include "src/application_drivers/openlcb_drivers.h"'
        );

        /* Callback includes: bare name → src/application_callbacks/ */
        code = code.replace(
            /#include "(callbacks_\w+\.h)"/g,
            '#include "src/application_callbacks/$1"'
        );

        return code;

    }

    /**
     * Fix includes in driver/callback files.
     * These live under src/application_drivers/ or src/application_callbacks/.
     * Library headers use "src/openlcb/..." in defs — need "../src/openlcb/..."
     * (up one level from application_drivers to src, then into src/openlcb).
     */
    function _fixSubfolderIncludes(code) {

        code = code.replace(
            /#include "src\//g,
            '#include "../src/'
        );

        return code;

    }

    /**
     * Fix includes in openlcb_user_config.h / .c.
     * These are at root level. Library includes "src/" → "src/src/".
     */
    function _fixConfigIncludes(code) {

        code = code.replace(
            /#include "src\/(openlcb|drivers)\//g,
            '#include "src/src/$1/'
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
        s.cdiUserXml     = wizardState.cdiUserXml || null;
        s.fdiUserXml     = wizardState.fdiUserXml || null;

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

        for (var i = 0; i < groupKeys.length; i++) {

            var key   = groupKeys[i];
            var group = DRIVER_GROUPS[key];
            var ds = (state.driverState && state.driverState[key]) ? state.driverState[key] : null;
            var checkedNames = (ds && ds.checked) ? ds.checked : [];

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

    /* ----------------------------------------------------------------------- */
    /* Getting Started document                                                 */
    /* ----------------------------------------------------------------------- */

    function _buildGettingStarted(wizardState, codegenState, mainFilename) {

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
        L.push('1. Copy the OpenLcbCLib library source into the src/src/ folder:');
        L.push('');
        L.push('     src/src/openlcb/         <-- core library (.c/.h files)');
        L.push('     src/src/drivers/canbus/  <-- CAN bus transport layer');
        L.push('     src/src/utilities/       <-- helper utilities');
        L.push('');
        L.push('2. Open ' + mainFilename + ' -- this is your application entry point.');
        L.push('   It wires the driver and callback functions into the library');
        L.push('   configuration structs and runs the main loop.');
        L.push('');
        L.push('3. Implement the driver stubs in src/application_drivers/.');
        L.push('   These connect the library to your hardware (CAN controller,');
        L.push('   EEPROM/flash for config memory, etc.).');
        L.push('   At minimum: CAN transmit/receive, lock/unlock shared resources,');
        L.push('   and the 100ms timer are critical for basic operation.');
        L.push('');
        L.push('4. Implement the callback stubs in src/application_callbacks/.');
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
        L.push('  |');
        L.push('  |-- src/');
        L.push('  |   |-- application_drivers/');
        L.push('  |   |   |-- openlcb_can_drivers.h      CAN bus hardware interface');
        L.push('  |   |   |-- openlcb_can_drivers.c');
        L.push('  |   |   |-- openlcb_drivers.h          Platform drivers (memory, reboot, etc.)');
        L.push('  |   |   |-- openlcb_drivers.c');
        L.push('  |   |');
        L.push('  |   |-- application_callbacks/');
        L.push('  |   |   |-- callbacks_*.h / .c         Application callback stubs');
        L.push('  |   |');
        L.push('  |   |-- xml_files/');
        L.push('  |   |   |-- cdi.xml                    Configuration Description Information');
        L.push('  |   |   |-- fdi.xml                    Function Description (train nodes)');
        L.push('  |   |');
        L.push('  |   |-- src/                           <-- Copy OpenLcbCLib library here');
        L.push('  |       |-- openlcb/                   Core library');
        L.push('  |       |-- drivers/canbus/            CAN transport layer');
        L.push('  |       |-- utilities/                 Helper utilities');
        L.push('  |');
        L.push('  |-- GETTING_STARTED.txt                This file');
        L.push('  |-- <type>_project.json                Node Wizard project (reload to edit)');
        L.push('');
        L.push('  NOTE: All subfolders are under src/ for Arduino IDE compatibility.');
        L.push('  The library lives under src/src/ which preserves the original');
        L.push('  OpenLcbCLib path structure for include consistency.');
        L.push('');
        L.push('');
        L.push('INCLUDE PATH CONVENTION');
        L.push('=======================');
        L.push('');
        L.push('All #include paths are relative to the file that contains them.');
        L.push('No special compiler -I flags are required.');
        L.push('');
        L.push('  From main.c (project root):');
        L.push('    #include "src/src/openlcb/openlcb_config.h"');
        L.push('    #include "src/application_drivers/openlcb_can_drivers.h"');
        L.push('    #include "src/application_callbacks/callbacks_events.h"');
        L.push('');
        L.push('  From src/application_drivers/ or src/application_callbacks/:');
        L.push('    #include "../src/openlcb/openlcb_types.h"');
        L.push('    #include "../src/drivers/canbus/can_types.h"');
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
        var configH    = _fixConfigIncludes(generateH(codegenState));
        var configC    = _fixConfigIncludes(generateC(codegenState));
        var mainC      = _fixMainIncludes(generateMain(codegenState));

        /* ---- Driver files ---- */
        var activeDrivers = _getActiveDriverGroups(codegenState);

        /* ---- Callback files ---- */
        var activeCallbacks = _getActiveCallbackGroups(codegenState);

        /* ---- Label for filenames ---- */
        var nodeLabel = wizardState.selectedNodeType === 'train-controller'
            ? 'train_controller'
            : wizardState.selectedNodeType;

        /* ---- Build ZIP ---- */
        var zip = new JSZip();
        var srcFolder = zip.folder('src');

        /* Root files */
        zip.file(mainFilename, mainC);
        zip.file('openlcb_user_config.h', configH);
        zip.file('openlcb_user_config.c', configC);

        /* Driver files under src/application_drivers/ */
        var driversFolder = srcFolder.folder('application_drivers');

        activeDrivers.forEach(function (entry) {

            var hCode = DriverCodegen.generateH(entry.group, entry.functions, wizardState.platformState);
            var cCode = DriverCodegen.generateC(entry.group, entry.functions, wizardState.platformState);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            driversFolder.file(entry.group.filePrefix + '.h', hCode);
            driversFolder.file(entry.group.filePrefix + '.c', cCode);

        });

        /* Callback files under src/application_callbacks/ */
        var callbacksFolder = srcFolder.folder('application_callbacks');

        activeCallbacks.forEach(function (entry) {

            var hCode = CallbackCodegen.generateH(entry.group, entry.functions);
            var cCode = CallbackCodegen.generateC(entry.group, entry.functions);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            callbacksFolder.file(entry.group.filePrefix + '.h', hCode);
            callbacksFolder.file(entry.group.filePrefix + '.c', cCode);

        });

        /* XML files under src/xml_files/ */
        var xmlFolder = srcFolder.folder('xml_files');

        if (wizardState.cdiUserXml && wizardState.cdiUserXml.trim()) {
            xmlFolder.file('cdi.xml', wizardState.cdiUserXml);
        }

        if (wizardState.fdiUserXml && wizardState.fdiUserXml.trim()) {
            xmlFolder.file('fdi.xml', wizardState.fdiUserXml);
        }

        /* Placeholder library folders under src/src/ */
        var libFolder = srcFolder.folder('src');
        libFolder.folder('openlcb');
        libFolder.folder('drivers/canbus');
        libFolder.folder('utilities');

        /* Getting Started document */
        zip.file('GETTING_STARTED.txt', _buildGettingStarted(wizardState, codegenState, mainFilename));

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
