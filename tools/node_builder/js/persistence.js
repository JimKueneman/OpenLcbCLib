/* ============================================================================
 * persistence.js — Save/restore form state to localStorage
 * ============================================================================ */

var STORAGE_KEY = 'nodeBuilderState';

var PERSIST_CHECKBOXES = [
    'flag-events', 'flag-datagrams', 'flag-config-mem', 'flag-firmware',
    'flag-broadcast-time', 'flag-train', 'flag-stream',
    'cfgopt-unaligned-reads', 'cfgopt-unaligned-writes', 'cfgopt-stream-rw',
    'addr-0xfe-present', 'addr-0xfe-readonly', 'addr-0xfe-low-valid'
];

var PERSIST_INPUTS = [
    'snip-name', 'snip-model', 'snip-hw-version', 'snip-sw-version',
    'cfgopt-desc',
    'buf-basic', 'buf-datagram', 'buf-snip', 'buf-stream', 'buf-nodes',
    'evt-producers', 'evt-producer-ranges', 'evt-consumers', 'evt-consumer-ranges',
    'cfg-user-name-addr', 'cfg-highest-addr',
    'train-node-count', 'train-listeners', 'train-functions',
    'addr-0xfe-low', 'addr-0xfe-high',
    'addr-0xff-desc', 'addr-0xfe-desc', 'addr-0xfd-desc',
    'addr-0xfc-desc', 'addr-0xfb-desc',
    'addr-0xfa-desc', 'addr-0xf9-desc', 'addr-0xf9-high',
    'addr-0xef-desc'
];

function saveState() {
    var state = { checkboxes: {}, inputs: {} };

    for (var i = 0; i < PERSIST_CHECKBOXES.length; i++) {
        var id = PERSIST_CHECKBOXES[i];
        var el = document.getElementById(id);
        if (el) state.checkboxes[id] = el.checked;
    }

    for (var i = 0; i < PERSIST_INPUTS.length; i++) {
        var id = PERSIST_INPUTS[i];
        var el = document.getElementById(id);
        if (el) state.inputs[id] = el.value;
    }

    // CDI/FDI XML text and filenames
    if (cdiXmlText) {
        state.cdiXmlText = cdiXmlText;
        state.cdiFileName = cdiFileName;
    }
    if (fdiXmlText) {
        state.fdiXmlText = fdiXmlText;
        state.fdiFileName = fdiFileName;
    }

    try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
    } catch (e) {
        // localStorage may be full or disabled — silently ignore
    }
}

function restoreState() {
    var json;
    try {
        json = localStorage.getItem(STORAGE_KEY);
    } catch (e) {
        return false;
    }
    if (!json) return false;

    var state;
    try {
        state = JSON.parse(json);
    } catch (e) {
        return false;
    }

    // Restore checkboxes
    if (state.checkboxes) {
        for (var id in state.checkboxes) {
            var el = document.getElementById(id);
            if (el) el.checked = state.checkboxes[id];
        }
    }

    // Restore inputs
    if (state.inputs) {
        for (var id in state.inputs) {
            var el = document.getElementById(id);
            if (el) el.value = state.inputs[id];
        }
    }

    // Restore CDI/FDI by reprocessing the saved XML text
    if (state.cdiXmlText) {
        cdiXmlText = state.cdiXmlText;
        cdiFileName = state.cdiFileName || 'restored.xml';
        processCDI();
    }
    if (state.fdiXmlText) {
        fdiXmlText = state.fdiXmlText;
        fdiFileName = state.fdiFileName || 'restored.xml';
        processFDI();
    }

    return true;
}

function clearState() {
    try {
        localStorage.removeItem(STORAGE_KEY);
    } catch (e) {
        // silently ignore
    }
}

/* Debounced auto-save — waits 500ms after last change */
var _saveTimer = null;

function initPersistence() {
    var main = document.getElementById('main-content');
    if (!main) return;

    main.addEventListener('input', function() {
        if (_saveTimer) clearTimeout(_saveTimer);
        _saveTimer = setTimeout(saveState, 500);
    });

    main.addEventListener('change', function() {
        if (_saveTimer) clearTimeout(_saveTimer);
        _saveTimer = setTimeout(saveState, 500);
    });
}
