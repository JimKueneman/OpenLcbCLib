/* =========================================================================
 * wke_defs.js — Well Known Event definitions
 *
 * Shared between node_config.js (form UI) and file_preview (codegen).
 * Must be loaded BEFORE codegen.js and node_config.js.
 *
 * Each group has a title, helpKey (for side-panel help), and an array of
 * events with { id, define, desc, range? }.
 *
 * Omissions (intentional):
 *   EVENT_TRAIN_SEARCH_SPACE — internal protocol use only.
 *   EVENT_ID_TRAIN_PROXY — deprecated.
 * ========================================================================= */

'use strict';

var WELL_KNOWN_EVENT_GROUPS = [

    {
        title: 'Emergency',
        helpKey: 'wke-group-emergency',
        events: [
            { id: 'emergency-off',        define: 'EVENT_ID_EMERGENCY_OFF',           desc: 'Immediately stop all layout activity' },
            { id: 'clear-emergency-off',  define: 'EVENT_ID_CLEAR_EMERGENCY_OFF',     desc: 'Resume normal operation' },
            { id: 'emergency-stop',       define: 'EVENT_ID_EMERGENCY_STOP',          desc: 'Stop all trains, maintain power' },
            { id: 'clear-emergency-stop', define: 'EVENT_ID_CLEAR_EMERGENCY_STOP',    desc: 'Trains may resume' }
        ]
    },
    {
        title: 'Power Supply',
        helpKey: 'wke-group-power-supply',
        events: [
            { id: 'brownout-node',     define: 'EVENT_ID_POWER_SUPPLY_BROWN_OUT_NODE',     desc: 'Brown-out detected on this node' },
            { id: 'brownout-standard', define: 'EVENT_ID_POWER_SUPPLY_BROWN_OUT_STANDARD',  desc: 'Brown-out on standard power bus' }
        ]
    },
    {
        title: 'Node Diagnostics',
        helpKey: 'wke-group-diagnostics',
        events: [
            { id: 'new-log',        define: 'EVENT_ID_NODE_RECORDED_NEW_LOG',        desc: 'Node recorded a new log entry' },
            { id: 'ident-button',   define: 'EVENT_ID_IDENT_BUTTON_COMBO_PRESSED',   desc: 'Identification button combination pressed' },
            { id: 'duplicate-node', define: 'EVENT_ID_DUPLICATE_NODE_DETECTED',       desc: 'Duplicate Node ID detected on network' },
            { id: 'link-error-1',   define: 'EVENT_ID_LINK_ERROR_CODE_1',             desc: 'Link layer error code 1' },
            { id: 'link-error-2',   define: 'EVENT_ID_LINK_ERROR_CODE_2',             desc: 'Link layer error code 2' },
            { id: 'link-error-3',   define: 'EVENT_ID_LINK_ERROR_CODE_3',             desc: 'Link layer error code 3' },
            { id: 'link-error-4',   define: 'EVENT_ID_LINK_ERROR_CODE_4',             desc: 'Link layer error code 4' }
        ]
    },
    {
        title: 'Train Protocol',
        helpKey: 'wke-group-train',
        events: [
            { id: 'train', define: 'EVENT_ID_TRAIN', desc: 'Train node identification' }
        ]
    },
    {
        title: 'Firmware',
        helpKey: 'wke-group-firmware',
        events: [
            { id: 'firmware-corrupted', define: 'EVENT_ID_FIRMWARE_CORRUPTED',                  desc: 'Node firmware is corrupted' },
            { id: 'firmware-hw-switch', define: 'EVENT_ID_FIRMWARE_UPGRADE_BY_HARDWARE_SWITCH',  desc: 'Firmware upgrade by hardware switch' }
        ]
    },
    {
        title: 'CBUS (MERG)',
        helpKey: 'wke-group-cbus',
        events: [
            { id: 'cbus-off', define: 'EVENT_ID_CBUS_OFF_SPACE', desc: 'CBUS Off event space', range: true },
            { id: 'cbus-on',  define: 'EVENT_ID_CBUS_ON_SPACE',  desc: 'CBUS On event space',  range: true }
        ]
    },
    {
        title: 'DCC',
        helpKey: 'wke-group-dcc',
        events: [
            { id: 'dcc-acc-activate',   define: 'EVENT_ID_DCC_ACCESSORY_ACTIVATE',           desc: 'Accessory activate',                range: true },
            { id: 'dcc-acc-deactivate', define: 'EVENT_ID_DCC_ACCESSORY_DEACTIVATE',         desc: 'Accessory deactivate',              range: true },
            { id: 'dcc-turnout-high',   define: 'EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH',        desc: 'Turnout feedback high (thrown)',     range: true },
            { id: 'dcc-turnout-low',    define: 'EVENT_ID_DCC_TURNOUT_FEEDBACK_LOW',         desc: 'Turnout feedback low (closed)',      range: true },
            { id: 'dcc-sensor-high',    define: 'EVENT_ID_DCC_SENSOR_FEEDBACK_HIGH',         desc: 'Sensor feedback high (occupied)',    range: true },
            { id: 'dcc-sensor-low',     define: 'EVENT_ID_DCC_SENSOR_FEEDBACK_LO',           desc: 'Sensor feedback low (clear)',        range: true },
            { id: 'dcc-ext-acc',        define: 'EVENT_ID_DCC_EXTENDED_ACCESSORY_CMD_SPACE', desc: 'Extended accessory command space',   range: true }
        ]
    }

];

/* Flat list derived from groups (used by codegen and validation) */
var WELL_KNOWN_EVENTS = [];
WELL_KNOWN_EVENT_GROUPS.forEach(function (g) {
    g.events.forEach(function (evt) { WELL_KNOWN_EVENTS.push(evt); });
});
