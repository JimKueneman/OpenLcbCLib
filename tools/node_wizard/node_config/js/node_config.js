/* =========================================================================
 * Node Configuration — node_config.js
 * Form logic, live code preview, and postMessage communication
 * ========================================================================= */

/* ---------- State ---------- */

let selectedNodeType = null;
let currentTab       = 'h';
let cdiUserBytes     = null;
let cdiUserText      = null;
let fdiUserBytes     = null;
let fdiUserText      = null;
let preserveWhitespace = false;
let driverStateFromParent   = {};   /* { 'can-drivers': { checked: [...] }, ... } */
let callbackStateFromParent = {};   /* { 'cb-events': { checked: [...] }, ... } */
let platformStateFromParent = null; /* { platform, params, framework, libraries, notes } */
let arduinoModeFromParent   = false;

const _isEmbedded = window.parent !== window;

/* ---------- Node type configuration ---------- */

var NODE_TYPE_DESCRIPTIONS = {

    'basic': {
        title: 'Basic Node',
        text: 'Produces and consumes events. Supports SNIP identification so configuration tools can discover the node on the bus. No configuration memory or datagrams.'
    },
    'typical': {
        title: 'Typical Node',
        text: 'Full-featured node with event transport, SNIP, datagrams, memory configuration, and CDI. Configuration tools can read and write persistent settings over the bus.'
    },
    'train': {
        title: 'Train (Locomotive) Node',
        text: 'Everything in Typical, plus the Train Protocol for speed/direction/function control and FDI for function discovery. Designed for DCC decoder or direct-drive locomotive nodes.'
    },
    'train-controller': {
        title: 'Train Controller (Throttle) Node',
        text: 'A throttle or cab that sends speed/direction/function commands to Train nodes. Includes Train Search for finding locomotives and the controller side of the Train Protocol.'
    },
    'bootloader': {
        title: 'Bootloader Node',
        text: 'Minimal firmware-upgrade-only build. Datagrams, Memory Configuration, and Firmware Upgrade are enabled. Events, Train Protocol, and Broadcast Time are compiled out to minimize image size.'
    }

};

/* Well Known Event data is in wke_defs.js (shared with file_preview) */

function _buildWellKnownEventLists() {

    var prodList = document.getElementById('wke-producer-list');
    var consList = document.getElementById('wke-consumer-list');
    if (!prodList || !consList) { return; }

    WELL_KNOWN_EVENT_GROUPS.forEach(function (group, gi) {

        /* Create collapsible group in each column */
        [prodList, consList].forEach(function (list, colIdx) {

            var prefix = colIdx === 0 ? 'wke-prod-' : 'wke-cons-';

            var details = document.createElement('details');
            details.className = 'wke-group';
            details.id = prefix + 'group-' + gi;

            var summary = document.createElement('summary');
            summary.className = 'wke-group-title';
            if (group.helpKey) { summary.setAttribute('data-help', group.helpKey); }
            summary.textContent = group.title + ' ';
            var badge = document.createElement('span');
            badge.className = 'wke-group-badge';
            badge.id = prefix + 'badge-' + gi;
            summary.appendChild(badge);
            details.appendChild(summary);

            group.events.forEach(function (evt) {

                var label = document.createElement('label');
                label.className = 'addon-item';
                label.setAttribute('data-help', 'wke-' + evt.id);
                label.innerHTML =
                    '<input type="checkbox" id="' + prefix + evt.id + '"> ' +
                    '<span class="addon-label">' + evt.define + '</span>' +
                    '<span class="addon-desc">' + evt.desc + (evt.range ? ' (range)' : '') + '</span>';
                details.appendChild(label);

            });

            list.appendChild(details);

        });

    });

}

_buildWellKnownEventLists();

function wkeExpandAll(listId) {

    var list = document.getElementById(listId);
    if (!list) { return; }
    list.querySelectorAll('details').forEach(function (d) { d.open = true; });
    updatePreview();

}

function wkeCollapseAll(listId) {

    var list = document.getElementById(listId);
    if (!list) { return; }
    list.querySelectorAll('details').forEach(function (d) { d.open = false; });
    updatePreview();

}

function _getWellKnownEventState() {

    var producers = [];
    var consumers = [];

    WELL_KNOWN_EVENTS.forEach(function (evt) {

        var pChk = document.getElementById('wke-prod-' + evt.id);
        var cChk = document.getElementById('wke-cons-' + evt.id);
        if (pChk && pChk.checked) { producers.push(evt.id); }
        if (cChk && cChk.checked) { consumers.push(evt.id); }

    });

    return { producers: producers, consumers: consumers };

}

function _restoreWellKnownEventState(wkeState) {

    if (!wkeState) { return; }

    (wkeState.producers || []).forEach(function (id) {
        var chk = document.getElementById('wke-prod-' + id);
        if (chk) { chk.checked = true; }
    });

    (wkeState.consumers || []).forEach(function (id) {
        var chk = document.getElementById('wke-cons-' + id);
        if (chk) { chk.checked = true; }
    });

}

function _highlightNodeTypeBtn(type) {

    document.querySelectorAll('.node-type-btn').forEach(function (btn) {

        btn.classList.toggle('active', btn.dataset.type === type);

    });

    /* Update description box */
    var descBox = document.getElementById('node-type-desc-box');
    if (!descBox) { return; }

    var info = NODE_TYPE_DESCRIPTIONS[type];
    if (info) {

        descBox.innerHTML = '<span class="desc-title">' + info.title + ':</span> ' + info.text;
        descBox.classList.remove('hidden');

    } else {

        descBox.classList.add('hidden');

    }

}

function applyNodeType(type) {

    selectedNodeType = type;
    _highlightNodeTypeBtn(type);

    const isBootloader = type === 'bootloader';

    /* Firmware: Basic node has no Config Memory, so firmware is unavailable;
     * Bootloader has firmware forced on */
    const firmwareCheckbox = document.getElementById('addon-firmware');
    const firmwareGroup    = document.getElementById('addon-firmware-label');
    const firmwareNote     = document.getElementById('firmware-note');

    if (type === 'basic') {

        firmwareCheckbox.checked  = false;
        firmwareCheckbox.disabled = true;
        firmwareGroup.classList.add('disabled');
        firmwareNote.classList.remove('hidden');

    } else if (isBootloader) {

        firmwareCheckbox.checked  = true;
        firmwareCheckbox.disabled = true;
        firmwareGroup.classList.remove('disabled');
        firmwareNote.classList.add('hidden');

    } else {

        firmwareCheckbox.disabled = false;
        firmwareGroup.classList.remove('disabled');
        firmwareNote.classList.add('hidden');

    }

    /* SNIP: Basic-only add-on (Typical/Train populate from CDI);
     * Bootloader: always shown, always checked, required */
    const snipGroup    = document.getElementById('addon-snip-group');
    const snipCheckbox = document.getElementById('addon-snip');
    const snipFields   = document.getElementById('snip-fields');

    if (type === 'basic' || isBootloader) {

        snipGroup.classList.remove('hidden');
        snipCheckbox.checked  = true;
        snipCheckbox.disabled = isBootloader;  /* bootloader: can't uncheck */
        snipFields.classList.remove('hidden');

    } else {

        snipGroup.classList.add('hidden');
        snipCheckbox.checked  = false;
        snipCheckbox.disabled = false;
        snipFields.classList.add('hidden');

    }

    /* Config Memory options: Typical and Train only (hidden for Basic and Bootloader) */
    const configGroup = document.getElementById('addon-config-group');
    configGroup.classList.toggle('hidden', type === 'basic' || isBootloader);

    /* Event Buffers section: hidden for Bootloader (no events) */
    const eventsGroup = document.getElementById('addon-events-group');
    if (eventsGroup) { eventsGroup.classList.toggle('hidden', isBootloader); }

    /* Well Known Events section: hidden for Bootloader (no events) */
    const wkeSection = document.getElementById('wellknown-events-section');
    if (wkeSection) { wkeSection.classList.toggle('hidden', isBootloader); }

    /* Add-Ons section: hidden for Bootloader (broadcast forced off, firmware forced on) */
    const addonsSection = document.getElementById('addon-extras-section');
    if (addonsSection) { addonsSection.classList.toggle('hidden', isBootloader); }

    /* Advanced section: hidden for Bootloader (all values are fixed minimums) */
    const advancedSection = document.getElementById('addon-advanced-section');
    if (advancedSection) { advancedSection.classList.toggle('hidden', isBootloader); }

    /* Advanced: Train Protocol and Listener groups — train/train-controller only */
    const isTrain = type === 'train' || type === 'train-controller';
    document.getElementById('adv-train-group').classList.toggle('hidden', !isTrain);
    document.getElementById('adv-listener-group').classList.toggle('hidden', !isTrain);

    /* Advanced: Set datagram buf default based on type */
    const dgBuf = document.getElementById('adv-datagram-buf');
    if (type === 'basic' && dgBuf.value === '8') {
        dgBuf.value = '0';
    } else if (isBootloader) {
        dgBuf.value = '2';
    } else if (type !== 'basic' && dgBuf.value === '0') {
        dgBuf.value = '8';
    }

    /* Auto-check and lock Well Known Events based on node type */
    var trainProdChk = document.getElementById('wke-prod-train');
    var trainConsChk = document.getElementById('wke-cons-train');
    if (trainProdChk) {
        trainProdChk.checked  = (type === 'train');
        trainProdChk.disabled = (type === 'train');
    }
    if (trainConsChk) {
        trainConsChk.checked  = (type === 'train-controller');
        trainConsChk.disabled = (type === 'train-controller');
    }

    /* Show the options section */
    document.getElementById('addons-section').classList.remove('hidden');

    updatePreview();

}

/* ---------- SNIP checkbox ---------- */

document.getElementById('addon-snip').addEventListener('change', function () {

    document.getElementById('snip-fields').classList.toggle('hidden', !this.checked);
    updatePreview();

});

/* ---------- Clamp number inputs to their min/max on change ---------- */

document.getElementById('config-panel').addEventListener('change', function (e) {

    var el = e.target;
    if (el.type !== 'number') { return; }

    var val = parseInt(el.value, 10);
    if (isNaN(val)) { return; }

    var min = el.hasAttribute('min') ? parseInt(el.min, 10) : null;
    var max = el.hasAttribute('max') ? parseInt(el.max, 10) : null;

    if (min !== null && val < min) { el.value = min; }
    if (max !== null && val > max) { el.value = max; }

});

/* ---------- Live tally helpers ---------- */

/* Map: input id -> tally <strong> id */
const _TALLY_MAP = {
    'event-producer-count':       'evt-prod-tally',
    'event-consumer-count':       'evt-cons-tally',
    'event-producer-range-count': 'evt-prod-range-tally',
    'event-consumer-range-count': 'evt-cons-range-tally',
    'adv-node-buf':               'node-buf-tally',
    'adv-can-buf':                'can-buf-tally',
    'adv-train-node-count':       'train-node-tally',
    'adv-train-listeners':        'train-listener-tally',
    'adv-train-functions':        'train-func-tally',
    'adv-probe-tick':             'probe-tick-tally',
    'adv-probe-interval':         'probe-interval-tally',
    'adv-verify-timeout':         'verify-timeout-tally'
};

function _updateTally(inputId) {

    var input = document.getElementById(inputId);
    var tally = document.getElementById(_TALLY_MAP[inputId]);
    if (!input || !tally) { return; }

    var val = parseInt(input.value, 10) || 0;
    var max = input.hasAttribute('max') ? parseInt(input.max, 10) : null;

    tally.textContent = val;
    tally.classList.toggle('over-limit', max !== null && val > max);

}

Object.keys(_TALLY_MAP).forEach(function (id) {

    document.getElementById(id).addEventListener('input', function () {
        _updateTally(id);
    });

});

/* Message Buffer Pool — composite total */
const _BUF_IDS = ['adv-basic-buf', 'adv-datagram-buf', 'adv-snip-buf', 'adv-stream-buf'];
const _BUF_MAX = 65534;

function _updateBufferTotal() {

    var total = 0;
    _BUF_IDS.forEach(function (id) {
        total += parseInt(document.getElementById(id).value, 10) || 0;
    });

    var el = document.getElementById('buffer-total-value');
    if (el) {
        el.textContent = total;
        el.classList.toggle('over-limit', total > _BUF_MAX);
    }

}

_BUF_IDS.forEach(function (id) {
    document.getElementById(id).addEventListener('input', _updateBufferTotal);
});

/* ---------- Wire inputs to live-update the preview ---------- */

/* Broadcast radio group */
document.querySelectorAll('input[name="addon-broadcast"]').forEach(el => {
    el.addEventListener('change', function () {
        _updateBroadcastNote();
        updatePreview();
    });
});

function _updateBroadcastNote() {
    var broadcastEl = document.querySelector('input[name="addon-broadcast"]:checked');
    var noteEl = document.getElementById('broadcast-note');
    if (!noteEl) { return; }
    var val = broadcastEl ? broadcastEl.value : 'none';
    if (val === 'none') {
        noteEl.classList.add('hidden');
    } else {
        noteEl.textContent = 'Uses 2 producer ranges + 2 consumer ranges in Event Buffers.';
        noteEl.classList.remove('hidden');
    }
}

/* Checkbox add-ons */
['addon-firmware', 'config-unaligned-reads', 'config-unaligned-writes'].forEach(id => {
    document.getElementById(id).addEventListener('change', updatePreview);
});

/* Well Known Event checkboxes (event delegation on both lists) */
['wke-producer-list', 'wke-consumer-list'].forEach(function (id) {
    var el = document.getElementById(id);
    if (el) { el.addEventListener('change', updatePreview); }
});

/* Project Options and text inputs (project-node-id handled by auto-format below) */
['project-name', 'project-author',
 'config-mem-highest-addr',
 'snip-manufacturer', 'snip-model', 'snip-hw-version', 'snip-sw-version'].forEach(id => {
    document.getElementById(id).addEventListener('input', updatePreview);
});

/* Event buffer count inputs */
['event-producer-count', 'event-producer-range-count',
 'event-consumer-count', 'event-consumer-range-count'].forEach(id => {
    document.getElementById(id).addEventListener('input', updatePreview);
});

/* Advanced panel inputs */
['adv-basic-buf', 'adv-datagram-buf', 'adv-snip-buf', 'adv-stream-buf',
 'adv-node-buf', 'adv-can-buf',
 'adv-train-node-count', 'adv-train-listeners', 'adv-train-functions',
 'adv-probe-tick', 'adv-probe-interval', 'adv-verify-timeout'].forEach(id => {
    document.getElementById(id).addEventListener('input', updatePreview);
});

/* ---------- Node ID auto-format (insert dots while typing) ---------- */
(function () {

    var el = document.getElementById('project-node-id');
    if (!el) { return; }

    el.addEventListener('input', function () {

        /* Strip 0x/0X prefix, dots, dashes, colons, spaces — then keep only hex */
        var raw = el.value.replace(/^0[xX]/, '').replace(/[\.\s\-:]/g, '').replace(/[^0-9A-Fa-f]/g, '').toUpperCase();

        /* Insert dots after every 2 hex chars */
        var parts = [];
        for (var i = 0; i < raw.length && i < 12; i += 2) {
            parts.push(raw.substring(i, Math.min(i + 2, raw.length)));
        }
        var formatted = parts.join('.');

        /* Only update if it actually changed (avoids cursor jump on no-op) */
        if (el.value !== formatted) {
            var cursor = el.selectionStart;
            var dotsBefore = (el.value.substring(0, cursor).match(/\./g) || []).length;
            el.value = formatted;
            var dotsAfter = (formatted.substring(0, cursor).match(/\./g) || []).length;
            el.selectionStart = el.selectionEnd = cursor + (dotsAfter - dotsBefore);
        }

        /* Update preview AFTER formatting so validation sees the dotted form */
        updatePreview();

    });

}());

/* ---------- Persist collapsible section open/closed on toggle ---------- */
document.addEventListener('toggle', function (e) {

    if (e.target.tagName === 'DETAILS') { updatePreview(); }

}, true);   /* useCapture — toggle doesn't bubble */

/* ---------- Collect current UI state ---------- */

/* ---------- Collapsible section persistence ---------- */

function _getOpenSections() {

    var result = {};
    document.querySelectorAll('details[id]').forEach(function (el) {

        result[el.id] = el.open;

    });
    return result;

}

function _restoreOpenSections(map) {

    if (!map) { return; }
    Object.keys(map).forEach(function (id) {

        var el = document.getElementById(id);
        if (el && el.tagName === 'DETAILS') {
            el.open = !!map[id];
        }

    });

}

function getState() {

    const broadcastEl  = document.querySelector('input[name="addon-broadcast"]:checked');
    const isTrainRole  = selectedNodeType === 'train' || selectedNodeType === 'train-controller';
    const isTrainNode  = selectedNodeType === 'train';   /* locomotive only — has FDI */
    const isBasic      = selectedNodeType === 'basic';
    const isBootloader = selectedNodeType === 'bootloader';

    /* Bootloader: no CDI or FDI — minimal null bytes only.
     * For Typical/Train/TrainController, fall back to the embedded default CDI when no user file. */
    const activeCdi    = (isBasic || isBootloader) ? null : (!cdiUserBytes ? DEFAULT_CDI_BYTES : cdiUserBytes);
    const activeCdiXml = (isBasic || isBootloader) ? null : (!cdiUserText  ? DEFAULT_CDI_XML  : cdiUserText);
    /* For Train (locomotive) only, fall back to the embedded default FDI when no user file.
     * Non-train node types never use FDI, so force null even if the user loaded one earlier. */
    const activeFdi    = isTrainNode ? (fdiUserBytes  || DEFAULT_FDI_BYTES) : null;
    const activeFdiXml = isTrainNode ? (fdiUserText   || DEFAULT_FDI_XML)  : null;

    return {
        nodeType:         selectedNodeType,
        projectName:      document.getElementById('project-name').value.trim(),
        projectAuthor:    document.getElementById('project-author').value.trim(),
        projectNodeId:    document.getElementById('project-node-id').value.trim(),
        broadcast:        broadcastEl ? broadcastEl.value : 'none',
        firmware:         document.getElementById('addon-firmware').checked,
        snipEnabled:      document.getElementById('addon-snip').checked,
        snipName:         document.getElementById('snip-manufacturer').value,
        snipModel:        document.getElementById('snip-model').value,
        snipHw:           document.getElementById('snip-hw-version').value,
        snipSw:           document.getElementById('snip-sw-version').value,
        unalignedReads:      document.getElementById('config-unaligned-reads').checked,
        unalignedWrites:     document.getElementById('config-unaligned-writes').checked,
        configMemHighest:    document.getElementById('config-mem-highest-addr').value.trim() || '0x200',
        producerCount:       parseInt(document.getElementById('event-producer-count').value,       10) || 64,
        producerRangeCount:  parseInt(document.getElementById('event-producer-range-count').value, 10) || 4,
        consumerCount:       parseInt(document.getElementById('event-consumer-count').value,       10) || 64,
        consumerRangeCount:  parseInt(document.getElementById('event-consumer-range-count').value, 10) || 4,
        cdiBytes:            activeCdi,
        cdiLength:           activeCdi ? activeCdi.length : 1,
        cdiXml:              activeCdiXml || null,
        fdiBytes:            activeFdi,
        fdiLength:           activeFdi ? activeFdi.length : 0,
        fdiXml:              activeFdiXml || null,
        preserveWhitespace:  preserveWhitespace,
        /* Advanced panel */
        advBasicBuf:         parseInt(document.getElementById('adv-basic-buf').value, 10) || 16,
        advDatagramBuf:      parseInt(document.getElementById('adv-datagram-buf').value, 10),
        advSnipBuf:          parseInt(document.getElementById('adv-snip-buf').value, 10) || 4,
        advStreamBuf:        parseInt(document.getElementById('adv-stream-buf').value, 10) || 0,
        advNodeBuf:          parseInt(document.getElementById('adv-node-buf').value, 10) || 1,
        advCanBuf:           parseInt(document.getElementById('adv-can-buf').value, 10) || 20,
        advTrainNodeCount:   parseInt(document.getElementById('adv-train-node-count').value, 10),
        advTrainListeners:   parseInt(document.getElementById('adv-train-listeners').value, 10),
        advTrainFunctions:   parseInt(document.getElementById('adv-train-functions').value, 10),
        advProbeTick:        parseInt(document.getElementById('adv-probe-tick').value, 10) || 1,
        advProbeInterval:    parseInt(document.getElementById('adv-probe-interval').value, 10) || 250,
        advVerifyTimeout:    parseInt(document.getElementById('adv-verify-timeout').value, 10) || 30,

        driverState:         driverStateFromParent,
        callbackState:       callbackStateFromParent,
        platformState:       platformStateFromParent,
        wellKnownEvents:     _getWellKnownEventState(),
        openSections:        _getOpenSections()
    };

}

/* ---------- Tab switching ---------- */

function _getMainFilename() {

    return arduinoModeFromParent ? 'main.ino' : 'main.c';

}

const TAB_LABELS_STATIC = {
    h: 'openlcb_user_config.h',
    c: 'openlcb_user_config.c',
    can: 'can_user_config.h'
};

function switchTab(tab) {

    currentTab = tab;

    /* Update selected state in file browser */
    var browser = document.getElementById('file-browser');
    if (browser) {

        var items = browser.querySelectorAll('.fb-item');
        items.forEach(function (el) {

            el.classList.toggle('selected', el.getAttribute('data-tab') === tab);

        });

    }

    /* Update filename display */
    var label = _getFilenameForTab(tab);
    var fnEl = document.getElementById('preview-filename');
    if (fnEl) { fnEl.textContent = label; }

    renderPreview();

}

/* ---------- Resolve filename for a composite tab ID ---------- */

function _getFilenameForTab(tab) {

    /* Core tabs */
    if (tab === 'main') { return _getMainFilename(); }
    if (TAB_LABELS_STATIC[tab]) { return TAB_LABELS_STATIC[tab]; }

    /* Composite: e.g. 'can-drivers-h' or 'cb-events-c' */
    var ext = tab.slice(-2);     /* '-h' or '-c' */
    var groupKey = tab.slice(0, -2);

    var group = (typeof DRIVER_GROUPS !== 'undefined' && DRIVER_GROUPS[groupKey])
        ? DRIVER_GROUPS[groupKey]
        : (typeof CALLBACK_GROUPS !== 'undefined' && CALLBACK_GROUPS[groupKey])
            ? CALLBACK_GROUPS[groupKey]
            : null;

    if (group) {
        var srcExt = arduinoModeFromParent ? '.cpp' : '.c';
        return group.filePrefix + (ext === '-h' ? '.h' : srcExt);
    }

    return tab;

}

/* ---------- Get active (required + checked) functions for a group ---------- */

function _getActiveFunctions(groupKey) {

    var group = null;
    var stateMap = null;

    if (typeof DRIVER_GROUPS !== 'undefined' && DRIVER_GROUPS[groupKey]) {
        group = DRIVER_GROUPS[groupKey];
        stateMap = driverStateFromParent;
    } else if (typeof CALLBACK_GROUPS !== 'undefined' && CALLBACK_GROUPS[groupKey]) {
        group = CALLBACK_GROUPS[groupKey];
        stateMap = callbackStateFromParent;
    }

    if (!group) { return []; }

    var gs = stateMap[groupKey];
    var checked = (gs && gs.checked) ? gs.checked : [];
    var active = [];
    var isBootloader = selectedNodeType === 'bootloader';

    group.functions.forEach(function (fn) {

        /* Bootloader: exclude config storage stubs — the bootloader uses
         * .firmware_write for flash and has no user config storage to read/write. */
        if (isBootloader && (fn.name === 'config_mem_read' || fn.name === 'config_mem_write')) { return; }

        if (fn.required || checked.indexOf(fn.name) >= 0) {
            active.push(fn);
        }

    });

    return active;

}

/* ---------- Rebuild the file browser sidebar ---------- */

function _rebuildFileBrowser() {

    var browser = document.getElementById('file-browser');
    if (!browser) { return; }

    var savedTab = currentTab;
    browser.innerHTML = '';

    /* Helper: create a section with a label and file items */
    function _addSection(label, items) {

        var sec = document.createElement('div');
        sec.className = 'fb-section';

        var lbl = document.createElement('span');
        lbl.className = 'fb-label';
        lbl.textContent = label;
        sec.appendChild(lbl);

        items.forEach(function (item) {

            var el = document.createElement('div');
            el.className = 'fb-item';
            el.setAttribute('data-tab', item.tab);
            el.textContent = item.filename;

            if (item.tab === savedTab) {
                el.classList.add('selected');
            }

            el.addEventListener('click', function () {

                switchTab(item.tab);

            });

            sec.appendChild(el);

        });

        browser.appendChild(sec);

    }

    /* Core files — always present */
    _addSection('Node Configuration', [
        { tab: 'h',    filename: 'openlcb_user_config.h' },
        { tab: 'c',    filename: 'openlcb_user_config.c' },
        { tab: 'can',  filename: 'can_user_config.h' },
        { tab: 'main', filename: _getMainFilename() }
    ]);

    /* Drivers — single section, all active driver groups merged */
    var srcExt = arduinoModeFromParent ? '.cpp' : '.c';

    if (typeof DRIVER_GROUPS !== 'undefined') {

        var driverItems = [];

        Object.keys(DRIVER_GROUPS).forEach(function (key) {

            var fns = _getActiveFunctions(key);
            if (fns.length === 0) { return; }

            var group = DRIVER_GROUPS[key];
            driverItems.push({ tab: key + '-h', filename: group.filePrefix + '.h' });
            driverItems.push({ tab: key + '-c', filename: group.filePrefix + srcExt });

        });

        if (driverItems.length > 0) {
            _addSection('Drivers', driverItems);
        }

    }

    /* Callbacks — single section, all active callback groups merged */
    if (typeof CALLBACK_GROUPS !== 'undefined') {

        var callbackItems = [];

        Object.keys(CALLBACK_GROUPS).forEach(function (key) {

            var fns = _getActiveFunctions(key);
            if (fns.length === 0) { return; }

            var group = CALLBACK_GROUPS[key];
            callbackItems.push({ tab: key + '-h', filename: group.filePrefix + '.h' });
            callbackItems.push({ tab: key + '-c', filename: group.filePrefix + srcExt });

        });

        if (callbackItems.length > 0) {
            _addSection('Callbacks', callbackItems);
        }

    }

    /* If saved tab is no longer valid, display 'h' but keep currentTab so a
     * later rebuild (e.g. after setDriverCallbackState arrives) can restore it */
    var hasTab = browser.querySelector('.fb-item[data-tab="' + savedTab + '"]');
    if (!hasTab) {

        var first = browser.querySelector('.fb-item[data-tab="h"]');
        if (first) { first.classList.add('selected'); }

    }

}

/* ---------- CodeMirror read-only viewer ---------- */

let _cmViewer = null;

function _ensureCMViewer() {

    if (_cmViewer) { return _cmViewer; }

    var container = document.getElementById('code-preview');
    if (!container) { return null; }
    _cmViewer = createCMReadonly(container);
    return _cmViewer;

}

/* ---------- Render the code preview ---------- */

function renderPreview() {

    var viewer = _ensureCMViewer();

    if (!selectedNodeType) {

        if (viewer) { viewer.value = '// Select a node type to preview the generated files'; }
        return;

    }

    const state = getState();

    if (viewer) {

        /* Rebuild the file dropdown (groups may have changed) */
        _rebuildFileBrowser();

        /* If the current tab isn't in the file browser yet (e.g. driver state
         * hasn't arrived), show 'h' content as a safe fallback */
        var browser = document.getElementById('file-browser');
        var tabExists = browser && browser.querySelector('.fb-item[data-tab="' + currentTab + '"]');
        var displayTab = tabExists ? currentTab : 'h';

        var code = _generateForTab(displayTab, state);
        viewer.value = code;

    }

    /* Check config memory size vs CDI requirements */
    _checkConfigMemSize(state);

    /* Lazy background validation */
    _runLazyValidation(state);

    /* Notify parent of state change */
    _postStateToParent(state);

}

/* ---------- Generate code for a given tab ---------- */

function _generateForTab(tab, state) {

    /* Core config files */
    if (tab === 'h')    { return generateH(state); }
    if (tab === 'c')    { return generateC(state); }
    if (tab === 'can')  { return generateCanH(state); }
    if (tab === 'main') { return generateMain(state); }

    /* Composite tab: '{groupKey}-h' or '{groupKey}-c' */
    var ext = tab.slice(-2);       /* '-h' or '-c' */
    var groupKey = tab.slice(0, -2);

    /* Driver group */
    if (typeof DRIVER_GROUPS !== 'undefined' && DRIVER_GROUPS[groupKey]) {

        var group = DRIVER_GROUPS[groupKey];
        var fns = _getActiveFunctions(groupKey);

        if (ext === '-h') {
            return DriverCodegen.generateH(group, fns, platformStateFromParent);
        } else {
            return DriverCodegen.generateC(group, fns, platformStateFromParent, arduinoModeFromParent);
        }

    }

    /* Callback group */
    if (typeof CALLBACK_GROUPS !== 'undefined' && CALLBACK_GROUPS[groupKey]) {

        var group = CALLBACK_GROUPS[groupKey];
        var fns = _getActiveFunctions(groupKey);

        if (ext === '-h') {
            return CallbackCodegen.generateH(group, fns);
        } else {
            return CallbackCodegen.generateC(group, fns, arduinoModeFromParent);
        }

    }

    return '// Unknown file: ' + tab;

}

function _checkConfigMemSize(state) {

    const warningEl = document.getElementById('config-mem-warning');
    if (!warningEl) { return; }

    const isBasic = state.nodeType === 'basic';
    if (isBasic || !state.cdiXml) {
        warningEl.style.display = 'none';
        return;
    }

    const cdiRequired = _cdiHighestAddrInSpace(state.cdiXml, 253);
    const cfgHighest  = parseInt(state.configMemHighest) || 0;

    if (cdiRequired > 0 && cfgHighest > 0 && cdiRequired > cfgHighest) {

        const reqHex = '0x' + cdiRequired.toString(16).toUpperCase();
        const cfgHex = '0x' + cfgHighest.toString(16).toUpperCase();
        warningEl.textContent =
            'CDI defines ' + cdiRequired + ' bytes (' + reqHex + ') in Space 253 ' +
            'but config memory size is only ' + cfgHighest + ' (' + cfgHex + '). ' +
            'Increase the value above to at least ' + reqHex + '.';
        warningEl.style.display = '';

    } else {
        warningEl.style.display = 'none';
    }

}

/* ---------- Lazy background validation ---------------------------------------- */

function _countCdiEventIds(xmlText) {

    if (!xmlText) { return 0; }
    try {
        var parser = new DOMParser();
        var doc    = parser.parseFromString(xmlText, 'text/xml');
        return doc.querySelectorAll('eventid').length;
    } catch (e) { return 0; }

}

function _countFdiFunctions(xmlText) {

    if (!xmlText) { return 0; }
    try {
        var parser = new DOMParser();
        var doc    = parser.parseFromString(xmlText, 'text/xml');
        return doc.querySelectorAll('function').length;
    } catch (e) { return 0; }

}

function _checkSnipLengths(state) {

    var warningEl = document.getElementById('snip-length-warning');
    if (!warningEl) { return; }

    var isBasic = state.nodeType === 'basic';
    if (isBasic || !state.cdiXml) {
        warningEl.style.display = 'none';
        return;
    }

    var ident = _extractCdiIdentification(state.cdiXml);
    var problems = [];

    if (ident.manufacturer    && ident.manufacturer.length    > 40) { problems.push('Manufacturer (' + ident.manufacturer.length + ' chars, max 40)'); }
    if (ident.model           && ident.model.length           > 40) { problems.push('Model (' + ident.model.length + ' chars, max 40)'); }
    if (ident.hardwareVersion && ident.hardwareVersion.length > 20) { problems.push('Hardware Version (' + ident.hardwareVersion.length + ' chars, max 20)'); }
    if (ident.softwareVersion && ident.softwareVersion.length > 20) { problems.push('Software Version (' + ident.softwareVersion.length + ' chars, max 20)'); }

    if (problems.length > 0) {
        warningEl.textContent = 'SNIP string(s) in CDI <identification> exceed ACDI limits and will be truncated: ' + problems.join('; ');
        warningEl.style.display = '';
    } else {
        warningEl.style.display = 'none';
    }

}

function _checkCdiSegment0xFD(state) {

    /* Intentionally removed: a CDI without <segment space="253"> is valid.
     * The node may only use ACDI (via <acdi/>) with no application config data.
     * The library returns "not present" for missing address spaces. */
    var warningEl = document.getElementById('cdi-segment-warning');
    if (warningEl) { warningEl.style.display = 'none'; }

}

/* Convert EVENT_ID_EMERGENCY_OFF → "Emergency Off" */
function _friendlyEventName(define) {

    return define
        .replace(/^EVENT_ID_/, '')
        .replace(/_/g, ' ')
        .toLowerCase()
        .replace(/\b\w/g, function (c) { return c.toUpperCase(); });

}

function _updateWellKnownEventSummary(state) {

    var wke = state.wellKnownEvents || { producers: [], consumers: [] };
    var _wkeMap = {};
    WELL_KNOWN_EVENTS.forEach(function (e) { _wkeMap[e.id] = e; });

    /* Update the summary hint — split events vs ranges */
    var pEvts = 0, pRanges = 0, cEvts = 0, cRanges = 0;
    wke.producers.forEach(function (id) { var e = _wkeMap[id]; if (e) { if (e.range) { pRanges++; } else { pEvts++; } } });
    wke.consumers.forEach(function (id) { var e = _wkeMap[id]; if (e) { if (e.range) { cRanges++; } else { cEvts++; } } });

    var hintEl = document.getElementById('wke-summary-hint');
    if (hintEl) {
        if (pEvts + pRanges + cEvts + cRanges > 0) {
            var badges = [];
            if (pEvts > 0)   { badges.push(pEvts + ' producer' + (pEvts !== 1 ? 's' : '')); }
            if (pRanges > 0) { badges.push(pRanges + ' producer range' + (pRanges !== 1 ? 's' : '')); }
            if (cEvts > 0)   { badges.push(cEvts + ' consumer' + (cEvts !== 1 ? 's' : '')); }
            if (cRanges > 0) { badges.push(cRanges + ' consumer range' + (cRanges !== 1 ? 's' : '')); }
            hintEl.textContent = '— ' + badges.join(', ') + ' selected';
        } else {
            hintEl.textContent = '— register standard OpenLCB event IDs as producers or consumers';
        }
    }

    /* Bottom tally — readable names on separate producer/consumer lines */
    var wkeTallyEl = document.getElementById('wke-tally');
    if (wkeTallyEl) {
        var prodNames = [];
        var consNames = [];
        wke.producers.forEach(function (id) {
            var e = _wkeMap[id];
            if (e) { prodNames.push(_friendlyEventName(e.define)); }
        });
        wke.consumers.forEach(function (id) {
            var e = _wkeMap[id];
            if (e) { consNames.push(_friendlyEventName(e.define)); }
        });
        if (prodNames.length + consNames.length > 0) {
            var lines = [];
            if (prodNames.length > 0) { lines.push('Producers: ' + prodNames.join(', ')); }
            if (consNames.length > 0) { lines.push('Consumers: ' + consNames.join(', ')); }
            lines.push('Make sure Event Buffers are sized to accommodate these.');
            wkeTallyEl.innerHTML = lines.join('<br>');
            wkeTallyEl.style.display = '';
        } else {
            wkeTallyEl.style.display = 'none';
        }
    }

    /* Per-group badges — count checked items in each sub-group */
    var prodSet = {};
    var consSet = {};
    wke.producers.forEach(function (id) { prodSet[id] = true; });
    wke.consumers.forEach(function (id) { consSet[id] = true; });
    WELL_KNOWN_EVENT_GROUPS.forEach(function (group, gi) {
        var pCount = 0, cCount = 0;
        group.events.forEach(function (evt) {
            if (prodSet[evt.id]) { pCount++; }
            if (consSet[evt.id]) { cCount++; }
        });
        var pBadge = document.getElementById('wke-prod-badge-' + gi);
        var cBadge = document.getElementById('wke-cons-badge-' + gi);
        if (pBadge) { pBadge.textContent = pCount > 0 ? '(' + pCount + ')' : ''; }
        if (cBadge) { cBadge.textContent = cCount > 0 ? '(' + cCount + ')' : ''; }
    });

}

function _runLazyValidation(state) {

    /* 1. CDI eventid count — informational */
    var evtInfoEl = document.getElementById('cdi-eventid-info');
    if (evtInfoEl) {
        if (state.cdiXml) {
            var evtCount = _countCdiEventIds(state.cdiXml);
            if (evtCount > 0) {
                evtInfoEl.textContent = 'CDI defines ' + evtCount + ' eventid field' + (evtCount !== 1 ? 's' : '') + ' — size buffers to cover these plus any events registered at runtime.';
                evtInfoEl.style.display = '';
            } else {
                evtInfoEl.style.display = 'none';
            }
        } else {
            evtInfoEl.style.display = 'none';
        }
    }

    /* 2. FDI function count vs Max Train Functions — informational/warning */
    var fdiInfoEl = document.getElementById('fdi-function-info');
    if (fdiInfoEl) {
        var isTrain = state.nodeType === 'train';
        if (isTrain && state.fdiXml) {
            var fdiCount = _countFdiFunctions(state.fdiXml);
            var maxFn    = state.advTrainFunctions || 29;
            if (fdiCount > 0) {
                var msg = 'FDI defines ' + fdiCount + ' function' + (fdiCount !== 1 ? 's' : '') + '. Max Train Functions is set to ' + maxFn + '.';
                if (fdiCount > maxFn) {
                    msg += ' ⚠ FDI exceeds the configured maximum — increase Max Train Functions to at least ' + fdiCount + '.';
                    fdiInfoEl.style.color = '#92400e';
                    fdiInfoEl.style.background = '#fef3c7';
                    fdiInfoEl.style.padding = '6px 8px';
                    fdiInfoEl.style.borderRadius = '4px';
                    fdiInfoEl.style.border = '1px solid #fcd34d';
                } else {
                    fdiInfoEl.style.color = '';
                    fdiInfoEl.style.background = '';
                    fdiInfoEl.style.padding = '';
                    fdiInfoEl.style.borderRadius = '';
                    fdiInfoEl.style.border = '';
                }
                fdiInfoEl.textContent = msg;
                fdiInfoEl.style.display = '';
            } else {
                fdiInfoEl.style.display = 'none';
            }
        } else {
            fdiInfoEl.style.display = 'none';
        }
    }

    /* 3. SNIP string lengths from CDI identification */
    _checkSnipLengths(state);

    /* 4. CDI missing 0xFD segment */
    _checkCdiSegment0xFD(state);

    /* 5. Node ID validation */
    var nodeIdEl = document.getElementById('project-node-id');
    var nodeIdValidEl = document.getElementById('node-id-validation');
    var _NODE_ID_RE = /^[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}$/;
    if (nodeIdEl && nodeIdValidEl) {
        var nid = nodeIdEl.value.trim();
        if (!nid) {
            nodeIdValidEl.textContent = '\u26A0 required — code will not compile without a Node ID';
            nodeIdValidEl.className = 'node-id-validation invalid';
        } else if (!_NODE_ID_RE.test(nid)) {
            nodeIdValidEl.textContent = '\u2717 use xx.xx.xx.xx.xx.xx hex format';
            nodeIdValidEl.className = 'node-id-validation invalid';
        } else {
            nodeIdValidEl.textContent = '\u2713 valid';
            nodeIdValidEl.className = 'node-id-validation valid';
        }
    }

    /* 5b. Project Options summary hint */
    var projHintEl = document.getElementById('project-summary-hint');
    if (projHintEl) {
        var parts = [];
        if (state.projectName) { parts.push(state.projectName); }
        if (state.projectAuthor) { parts.push('by ' + state.projectAuthor); }
        if (state.projectNodeId && /^[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}\.[0-9A-Fa-f]{2}$/.test(state.projectNodeId)) {
            parts.push('Node ID ' + state.projectNodeId);
        }
        if (parts.length > 0) {
            projHintEl.textContent = '— ' + parts.join(', ');
        } else {
            projHintEl.textContent = '— name, author, and Node ID';
        }
    }

    /* 6. Well Known Events — summary hint and bottom tally */
    _updateWellKnownEventSummary(state);

    /* 6. Event Buffers summary hint */
    var evtHintEl = document.getElementById('evt-buf-summary-hint');
    if (evtHintEl) {
        var wke = state.wellKnownEvents || { producers: [], consumers: [] };
        var _wkeMap = {};
        WELL_KNOWN_EVENTS.forEach(function (e) { _wkeMap[e.id] = e; });

        /* Count committed resources from Well Known Events + Broadcast Time */
        var cProd = 0, cProdR = 0, cCons = 0, cConsR = 0;
        wke.producers.forEach(function (id) { var e = _wkeMap[id]; if (e) { if (e.range) { cProdR++; } else { cProd++; } } });
        wke.consumers.forEach(function (id) { var e = _wkeMap[id]; if (e) { if (e.range) { cConsR++; } else { cCons++; } } });
        if (state.broadcast && state.broadcast !== 'none') {
            cProdR += 2;
            cConsR += 2;
        }

        /* Format: "P: 64 (3 spoken for)  C: 64  PR: 4 (2 spoken for)  CR: 4" */
        function _evtBadge(label, val, committed) {
            var s = label + ': ' + val;
            if (committed > 0) { s += ' (' + committed + ' spoken for)'; }
            return s;
        }
        var hint = '— ' +
            _evtBadge('Producers', state.producerCount, cProd) + '  ' +
            _evtBadge('Consumers', state.consumerCount, cCons) + '  ' +
            _evtBadge('Producer Ranges', state.producerRangeCount, cProdR) + '  ' +
            _evtBadge('Consumer Ranges', state.consumerRangeCount, cConsR);

        evtHintEl.textContent = hint;
    }

    /* 7. Add-Ons summary hint — only show what is actually selected */
    var addonsHintEl = document.getElementById('addons-summary-hint');
    if (addonsHintEl) {
        var parts = [];
        if (state.broadcast && state.broadcast !== 'none') {
            var role = state.broadcast === 'producer' ? 'Producer' : 'Consumer';
            parts.push('Broadcast Time ' + role + ' (2 producer ranges + 2 consumer ranges)');
        }
        if (state.firmware) { parts.push('Firmware Update'); }
        if (parts.length > 0) {
            addonsHintEl.textContent = '— ' + parts.join(', ');
        } else {
            addonsHintEl.textContent = '— none selected';
        }
    }

}

function updatePreview() {

    renderPreview();

}

/* ---------- Download generated files ---------- */

function downloadFiles() {

    if (!selectedNodeType) { return; }

    const state = getState();
    _download('openlcb_user_config.h', generateH(state));
    _download('openlcb_user_config.c', generateC(state));
    _download('can_user_config.h', generateCanH(state));
    _download(_getMainFilename(), generateMain(state));

}

function _download(filename, content) {

    const blob = new Blob([content], { type: 'text/plain' });
    const url  = URL.createObjectURL(blob);
    const a    = document.createElement('a');

    a.href     = url;
    a.download = filename;
    a.click();
    URL.revokeObjectURL(url);

}

/* ---------- Splitter drag (only when preview panel is present) ---------- */

(function () {

    const splitter = document.getElementById('splitter');
    if (!splitter) { return; }

    const configPanel = document.getElementById('config-panel');
    let startX, startWidth;

    splitter.addEventListener('mousedown', function (e) {

        startX     = e.clientX;
        startWidth = configPanel.offsetWidth;

        splitter.classList.add('dragging');
        document.body.style.userSelect = 'none';
        document.body.style.cursor     = 'col-resize';

        document.addEventListener('mousemove', onMove);
        document.addEventListener('mouseup',   onUp);
        e.preventDefault();

    });

    function onMove(e) {

        const dx       = e.clientX - startX;
        const newWidth = Math.max(280, Math.min(900, startWidth + dx));
        configPanel.style.flexBasis = newWidth + 'px';

    }

    function onUp() {

        splitter.classList.remove('dragging');
        document.body.style.userSelect = '';
        document.body.style.cursor     = '';
        document.removeEventListener('mousemove', onMove);
        document.removeEventListener('mouseup',   onUp);

        /* Persist splitter position */
        updatePreview();

    }

}());

/* ---------- postMessage communication (embedded mode) ---------- */

function _postStateToParent(state) {

    if (!_isEmbedded) { return; }

    /* Can't send Uint8Arrays through postMessage easily,
     * so send just the serializable parts the parent needs */
    window.parent.postMessage({
        type: 'stateChanged',
        state: {
            nodeType:           state.nodeType,
            cdiLength:          state.cdiLength,
            fdiLength:          state.fdiLength,
            configMemHighest:   state.configMemHighest,
            /* Full form state for persistence */
            projectName:        state.projectName,
            projectAuthor:      state.projectAuthor,
            projectNodeId:      state.projectNodeId,
            broadcast:          state.broadcast,
            firmware:           state.firmware,
            snipEnabled:        state.snipEnabled,
            snipName:           state.snipName,
            snipModel:          state.snipModel,
            snipHw:             state.snipHw,
            snipSw:             state.snipSw,
            unalignedReads:     state.unalignedReads,
            unalignedWrites:    state.unalignedWrites,
            producerCount:      state.producerCount,
            producerRangeCount: state.producerRangeCount,
            consumerCount:      state.consumerCount,
            consumerRangeCount: state.consumerRangeCount,
            /* Advanced panel */
            advBasicBuf:        state.advBasicBuf,
            advDatagramBuf:     state.advDatagramBuf,
            advSnipBuf:         state.advSnipBuf,
            advStreamBuf:       state.advStreamBuf,
            advNodeBuf:         state.advNodeBuf,
            advCanBuf:          state.advCanBuf,
            advTrainNodeCount:  state.advTrainNodeCount,
            advTrainListeners:  state.advTrainListeners,
            advTrainFunctions:  state.advTrainFunctions,
            advProbeTick:       state.advProbeTick,
            advProbeInterval:   state.advProbeInterval,
            advVerifyTimeout:   state.advVerifyTimeout,
            wellKnownEvents:    state.wellKnownEvents,
            openSections:       state.openSections,
            currentTab:         currentTab,
            splitterWidth:      document.getElementById('config-panel').offsetWidth
        }
    }, '*');

}

window.addEventListener('message', function (e) {

    if (!e.data) { return; }

    if (e.data.type === 'setNodeType') {

        applyNodeType(e.data.nodeType);

    } else if (e.data.type === 'setCdiBytes') {

        /* Parent sends updated CDI bytes from CDI editor */
        cdiUserText  = e.data.xml;
        if (e.data.preserveWhitespace !== undefined) {
            preserveWhitespace = !!e.data.preserveWhitespace;
        }
        cdiUserBytes = _xmlToBytes(cdiUserText, preserveWhitespace);
        updatePreview();

    } else if (e.data.type === 'setFdiBytes') {

        /* Parent sends updated FDI bytes from FDI editor */
        fdiUserText  = e.data.xml;
        fdiUserBytes = _xmlToBytes(fdiUserText, preserveWhitespace);
        updatePreview();

    } else if (e.data.type === 'setConfigMemSize') {

        /* CDI editor requested a config memory size update */
        document.getElementById('config-mem-highest-addr').value = e.data.value;
        updatePreview();

    } else if (e.data.type === 'setPreserveWhitespace') {

        /* CDI editor toggled the preserve-whitespace option */
        preserveWhitespace = !!e.data.value;

        /* Re-encode CDI/FDI bytes with the new setting */
        if (cdiUserText) {
            cdiUserBytes = _xmlToBytes(cdiUserText, preserveWhitespace);
        }
        if (fdiUserText) {
            fdiUserBytes = _xmlToBytes(fdiUserText, preserveWhitespace);
        }
        updatePreview();

    } else if (e.data.type === 'restoreFormState') {

        /* Parent sends saved form values to restore */
        const f = e.data.formState;
        if (!f) { return; }

        /* Project Options */
        if (f.projectName  !== undefined) { document.getElementById('project-name').value  = f.projectName; }
        if (f.projectAuthor !== undefined) { document.getElementById('project-author').value = f.projectAuthor; }
        if (f.projectNodeId !== undefined) { document.getElementById('project-node-id').value = f.projectNodeId; }

        /* Radio: broadcast */
        if (f.broadcast) {
            const radio = document.querySelector('input[name="addon-broadcast"][value="' + f.broadcast + '"]');
            if (radio) { radio.checked = true; }
        }

        /* Broadcast note */
        _updateBroadcastNote();

        /* Checkboxes */
        if (typeof f.firmware        === 'boolean') { document.getElementById('addon-firmware').checked         = f.firmware; }
        if (typeof f.snipEnabled     === 'boolean') { document.getElementById('addon-snip').checked             = f.snipEnabled; }
        if (typeof f.unalignedReads  === 'boolean') { document.getElementById('config-unaligned-reads').checked = f.unalignedReads; }
        if (typeof f.unalignedWrites === 'boolean') { document.getElementById('config-unaligned-writes').checked = f.unalignedWrites; }

        /* SNIP text fields */
        if (f.snipName  !== undefined) { document.getElementById('snip-manufacturer').value = f.snipName; }
        if (f.snipModel !== undefined) { document.getElementById('snip-model').value        = f.snipModel; }
        if (f.snipHw    !== undefined) { document.getElementById('snip-hw-version').value    = f.snipHw; }
        if (f.snipSw    !== undefined) { document.getElementById('snip-sw-version').value    = f.snipSw; }

        /* Config memory */
        if (f.configMemHighest) { document.getElementById('config-mem-highest-addr').value = f.configMemHighest; }

        /* Event counts */
        if (f.producerCount      !== undefined) { document.getElementById('event-producer-count').value       = f.producerCount; }
        if (f.producerRangeCount !== undefined) { document.getElementById('event-producer-range-count').value = f.producerRangeCount; }
        if (f.consumerCount      !== undefined) { document.getElementById('event-consumer-count').value       = f.consumerCount; }
        if (f.consumerRangeCount !== undefined) { document.getElementById('event-consumer-range-count').value = f.consumerRangeCount; }

        /* Advanced panel */
        if (f.advBasicBuf       !== undefined) { document.getElementById('adv-basic-buf').value       = f.advBasicBuf; }
        if (f.advDatagramBuf    !== undefined) { document.getElementById('adv-datagram-buf').value    = f.advDatagramBuf; }
        if (f.advSnipBuf        !== undefined) { document.getElementById('adv-snip-buf').value        = f.advSnipBuf; }
        if (f.advStreamBuf      !== undefined) { document.getElementById('adv-stream-buf').value      = f.advStreamBuf; }
        if (f.advNodeBuf        !== undefined) { document.getElementById('adv-node-buf').value        = f.advNodeBuf; }
        if (f.advCanBuf         !== undefined) { document.getElementById('adv-can-buf').value         = f.advCanBuf; }
        if (f.advTrainNodeCount !== undefined) { document.getElementById('adv-train-node-count').value = f.advTrainNodeCount; }
        if (f.advTrainListeners !== undefined) { document.getElementById('adv-train-listeners').value = f.advTrainListeners; }
        if (f.advTrainFunctions !== undefined) { document.getElementById('adv-train-functions').value = f.advTrainFunctions; }
        if (f.advProbeTick      !== undefined) { document.getElementById('adv-probe-tick').value      = f.advProbeTick; }
        if (f.advProbeInterval  !== undefined) { document.getElementById('adv-probe-interval').value  = f.advProbeInterval; }
        if (f.advVerifyTimeout  !== undefined) { document.getElementById('adv-verify-timeout').value  = f.advVerifyTimeout; }

        /* SNIP fields visibility */
        document.getElementById('snip-fields').classList.toggle('hidden', !document.getElementById('addon-snip').checked);

        /* Splitter width (only when preview panel is present) */
        if (f.splitterWidth && document.getElementById('splitter')) {
            document.getElementById('config-panel').style.flex = '0 0 ' + f.splitterWidth + 'px';
        }

        /* Well Known Events */
        if (f.wellKnownEvents) { _restoreWellKnownEventState(f.wellKnownEvents); }

        /* Collapsible section open/closed states */
        if (f.openSections) { _restoreOpenSections(f.openSections); }

        /* Tab */
        if (f.currentTab) { switchTab(f.currentTab); }

        updatePreview();

    } else if (e.data.type === 'setArduinoMode') {

        arduinoModeFromParent = !!e.data.arduino;
        _rebuildFileBrowser();
        /* Update filename display if on the main tab */
        if (currentTab === 'main') {
            var fnEl = document.getElementById('preview-filename');
            if (fnEl) { fnEl.textContent = _getMainFilename(); }
        }

    } else if (e.data.type === 'setDriverCallbackState') {

        /* Parent sends current driver/callback selections for main.c wiring */
        if (e.data.driverState)   { driverStateFromParent   = e.data.driverState; }
        if (e.data.callbackState) { callbackStateFromParent = e.data.callbackState; }
        if (e.data.platformState) { platformStateFromParent = e.data.platformState; }
        _rebuildFileBrowser();
        updatePreview();

    } else if (e.data.type === 'requestDownload') {

        downloadFiles();

    }

});

/* ---------- Detail panel (hover help) ---------- */

var HELP_DATA = {

    'project-name': {
        title: 'Project Name',
        description: 'A short name for this project. Used as the base filename for ZIP downloads and saved project files.',
        detail: 'Use lowercase with underscores for best compatibility across file systems (e.g. "my_signal_controller"). If left blank, the node type is used as the default name.'
    },
    'project-author': {
        title: 'Author',
        description: 'Your name or company. Inserted into the copyright headers of all generated source files.',
        detail: 'This replaces the placeholder "<YOUR NAME OR COMPANY>" in file headers. Leave blank to keep the placeholder for manual editing later.'
    },
    'project-node-id': {
        title: 'Node ID',
        description: 'A globally unique 48-bit identifier for this node, in xx.xx.xx.xx.xx.xx hexadecimal notation (e.g. 05.07.01.01.00.00).',
        detail: 'Every device on an OpenLCB network must have a unique Node ID. IDs are allocated in ranges by the OpenLCB registry. Manufacturers register a range and assign individual IDs within it. If you are experimenting, the range 05.07.01.01.xx.xx is reserved for self-assigned test IDs. For production devices, register your own range at https://registry.openlcb.org/uniqueidranges.'
    },
    'snip': {
        title: 'Simple Node Identification (SNIP)',
        description: 'Lets configuration tools identify this node by name, model, and version without needing memory configuration. Basic nodes use SNIP directly; Typical and Train nodes populate SNIP from the CDI automatically.',
        detail: 'SNIP strings are stored as null-terminated fields in a compact binary format. The protocol defines manufacturer name (up to 41 bytes), model (41 bytes), hardware version (21 bytes), and software version (21 bytes). Total payload is capped at 252 bytes. See Simple Node Information Protocol Standard.'
    },
    'snip-manufacturer': {
        title: 'Manufacturer Name',
        description: 'The name of the company or individual that manufactured this node. Up to 40 characters.',
        detail: 'This is the first field in the SNIP response. Configuration tools display it when listing nodes. For Basic nodes this is entered here; for Typical/Train nodes it comes from the CDI identification section.'
    },
    'snip-model': {
        title: 'Model Name',
        description: 'The model or product name for this node. Up to 40 characters.',
        detail: 'The second field in the SNIP response. Should uniquely identify the type of hardware or application, e.g. "Signal Controller v2" or "Turnout Driver".'
    },
    'snip-hw-version': {
        title: 'Hardware Version',
        description: 'The hardware revision or board version for this node. Up to 20 characters.',
        detail: 'Useful for distinguishing PCB revisions or hardware variants that share the same firmware. Configuration tools may display this in node property sheets.'
    },
    'snip-sw-version': {
        title: 'Software Version',
        description: 'The firmware or software version running on this node. Up to 20 characters.',
        detail: 'Typically follows semantic versioning (e.g. "1.2.3"). Helps users and support staff identify which firmware is installed when troubleshooting.'
    },
    'config-mem': {
        title: 'Memory Configuration',
        description: 'Controls how configuration tools read and write persistent settings on this node over the OpenLCB bus.',
        detail: 'Unaligned access flags tell configuration tools whether they can start a read or write at any byte address, or must align to word boundaries. The highest address defines the size of your configuration address space. The first 127 bytes (0x00\u20130x7E) are reserved by the default CDI for User Name and User Description fields.'
    },
    'config-unaligned-reads': {
        title: 'Unaligned Reads Supported',
        description: 'Whether configuration tools can start a read at any byte address in the configuration memory space.',
        detail: 'When checked, the node advertises that reads can begin at any byte offset. Uncheck this if your storage hardware (EEPROM, FLASH, FRAM, etc.) requires reads to be aligned to word or page boundaries. Most modern microcontrollers support unaligned reads.'
    },
    'config-unaligned-writes': {
        title: 'Unaligned Writes Supported',
        description: 'Whether configuration tools can start a write at any byte address in the configuration memory space.',
        detail: 'When checked, the node advertises that writes can begin at any byte offset. Uncheck this if your storage hardware requires writes to be aligned to word or page boundaries. Flash memory often requires page-aligned writes, while EEPROM and FRAM typically allow byte-level access.'
    },
    'config-highest-addr': {
        title: 'Config Memory Size (Highest Address)',
        description: 'The highest byte address in this node\u2019s configuration memory space. Defines the total size of the space.',
        detail: 'Enter the address in hex (e.g. 0x200) or decimal (e.g. 512) \u2014 both formats are accepted. The value is passed directly into the generated C code. Bytes 0x00\u20130x7E (0\u2013126) are reserved by the default CDI for User Name (63 bytes) and User Description (64 bytes). Your application data must start at byte 0x7F (127) or higher. Make sure this value is large enough to cover all configuration data defined in your CDI.'
    },
    'events': {
        title: 'Event Buffers',
        description: 'How many events this node can produce, consume, or match by range. Size these to cover every Event ID your application registers.',
        detail: 'Each event occupies an 8-byte Event ID. Producers are events this node sends; consumers are events this node listens for. Range events let a single entry match a contiguous block of Event IDs, which is useful for protocols like Broadcast Time. If Broadcast Time is enabled, account for its 2 producer ranges and 2 consumer ranges in your counts.'
    },
    'evt-producers': {
        title: 'Producer Count',
        description: 'Number of individual events this node can produce (send onto the bus).',
        detail: 'Each producer slot holds one 8-byte Event ID. When something happens on this node (e.g. an input changes state), it sends the corresponding produced event to the bus. Set this to the number of producer events your application registers.'
    },
    'evt-consumers': {
        title: 'Consumer Count',
        description: 'Number of individual events this node can consume (listen for on the bus).',
        detail: 'Each consumer slot holds one 8-byte Event ID. When a matching event arrives on the bus, the node can act on it (e.g. activate an output). Set this to the number of consumer events your application registers.'
    },
    'evt-producer-ranges': {
        title: 'Producer Range Count',
        description: 'Number of producer range entries. A range covers a contiguous block of Event IDs with a single entry.',
        detail: 'Range events use a masked Event ID to match an entire block at once. They are primarily used by protocols like Broadcast Time (which needs 2 producer ranges). If you are not using range-based protocols, the default of 4 is sufficient for most applications.'
    },
    'evt-consumer-ranges': {
        title: 'Consumer Range Count',
        description: 'Number of consumer range entries. A range covers a contiguous block of Event IDs with a single entry.',
        detail: 'Similar to producer ranges but for incoming events. Broadcast Time uses 2 consumer ranges. The default of 4 covers most use cases.'
    },
    'broadcast-time': {
        title: 'Broadcast Time',
        description: 'Enables the OpenLCB fast-clock protocol for synchronized model time across the layout.',
        detail: 'A Producer generates the clock signal that all other nodes follow. A Consumer receives and tracks the fast-clock time. Only one node on the bus should be the Producer; all others that need time should be Consumers. Each role uses 2 producer event ranges and 2 consumer event ranges, so adjust your Event Buffers accordingly. See Broadcast Time Standard.'
    },
    'broadcast-none': {
        title: 'Broadcast Time \u2014 None',
        description: 'This node does not participate in the fast-clock protocol.',
        detail: 'Select this if the node has no need for layout-wide synchronized time. No additional event ranges are consumed.'
    },
    'broadcast-producer': {
        title: 'Broadcast Time \u2014 Producer',
        description: 'This node generates and distributes fast-clock time to the layout.',
        detail: 'Only one node on the entire bus should be the Broadcast Time Producer. It sends periodic time updates that all Consumer nodes follow. Requires 2 producer event ranges and 2 consumer event ranges \u2014 make sure your Event Buffers account for these.'
    },
    'broadcast-consumer': {
        title: 'Broadcast Time \u2014 Consumer',
        description: 'This node receives and tracks fast-clock time from the Producer on the bus.',
        detail: 'A Consumer listens for time updates broadcast by the single Producer node and keeps its local time in sync. Requires 2 producer event ranges and 2 consumer event ranges \u2014 make sure your Event Buffers account for these.'
    },
    'firmware': {
        title: 'Firmware Update',
        description: 'Enables over-the-network firmware upgrades using the OpenLCB Firmware Upgrade Protocol.',
        detail: 'When enabled, a configuration tool can send a new firmware image to this node over the bus without physical access. Requires Memory Configuration (not available for Basic nodes). The node must implement the firmware write driver callback to flash the received image. See Firmware Upgrade Standard.'
    },
    'buf-pool': {
        title: 'Message Buffer Pool',
        description: 'Pre-allocated memory pools for different OpenLCB message types. Tune these for your platform\u2019s available RAM.',
        detail: 'Basic buffers (16 bytes) handle short messages like Verified Node ID and event notifications. Datagram buffers (72 bytes) handle configuration reads/writes and other datagram-based protocols. SNIP buffers (256 bytes) hold the full SNIP response payload. Stream buffers (512 bytes) are for large data transfers. The total of all pools must not exceed 65534. Fewer buffers save RAM but may slow throughput under heavy traffic.'
    },
    'buf-basic': {
        title: 'Basic Buffers',
        description: 'Number of 16-byte buffers for short OpenLCB messages.',
        detail: 'Basic buffers carry short protocol messages such as Verified Node ID, event identification, and simple protocol replies. 16 bytes covers the maximum CAN-frame payload plus internal header. At least 1 is required; 16 is a good default for moderate bus traffic.'
    },
    'buf-datagram': {
        title: 'Datagram Buffers',
        description: 'Number of 72-byte buffers for datagram-based protocols.',
        detail: 'Datagrams carry configuration memory reads/writes and other addressed data transfers up to 72 bytes. If Memory Configuration is enabled, you need at least 1. More buffers allow overlapping datagram transactions, which helps on busy buses. 8 is a good default.'
    },
    'buf-snip': {
        title: 'SNIP Buffers',
        description: 'Number of 256-byte buffers for SNIP response payloads.',
        detail: 'Each SNIP buffer holds one complete Simple Node Information Protocol response (manufacturer, model, hardware version, software version, user name, user description). At least 1 is required. More buffers allow the node to handle concurrent SNIP requests from multiple configuration tools. 4 is a good default.'
    },
    'buf-stream': {
        title: 'Stream Buffers',
        description: 'Number of 512-byte buffers for large stream data transfers.',
        detail: 'Stream buffers are used for large data transfers such as firmware upgrades. If you are not using streaming protocols, set this to 0 to save RAM. Each buffer uses 512 bytes of RAM.'
    },
    'virtual-nodes': {
        title: 'Virtual Node Allocation',
        description: 'How many OpenLCB nodes this single physical device hosts.',
        detail: 'Most devices use 1 node. Train decoders that manage multiple locomotive addresses may need more. Each virtual node gets its own Node ID and alias on the CAN bus, consuming additional buffer and alias resources. If set to 1, the library skips internal loopback processing for better efficiency.'
    },
    'can-buffers': {
        title: 'CAN Message Buffers',
        description: 'Queue depth for raw CAN frames waiting to be processed.',
        detail: 'Incoming CAN frames are stored in this FIFO until the main loop processes them. A larger queue prevents dropped frames during bursts (e.g. many nodes coming online at once), but uses more RAM. 20 is a good default; increase if you see lost messages on busy buses.'
    },
    'train-protocol': {
        title: 'Train Protocol',
        description: 'Configuration for nodes that implement the OpenLCB Train Protocol (locomotives or throttles).',
        detail: 'Train Node Count sets how many locomotive identities this device manages (usually 1 for a single decoder). Max Listeners is the number of throttles that can simultaneously control each train (6 is the typical default for consist support). Max Train Functions is the highest function number the node supports (F0\u2013F28 = 29 functions is standard DCC). See Train Control Standard.'
    },
    'train-node-count': {
        title: 'Train Node Count',
        description: 'How many locomotive identities this device manages.',
        detail: 'Each train node gets its own Node ID and alias on the bus. Most single-decoder devices use 1. A multi-decoder board or command station proxy might manage more. Each additional train node consumes buffer and alias resources.'
    },
    'train-listeners': {
        title: 'Max Listeners per Train',
        description: 'Maximum number of throttles that can simultaneously control each train.',
        detail: 'Listeners are throttles that have attached to this train, typically for consist (multiple-unit) operation. 6 is the standard default and handles most consist scenarios. If you need large consists, increase this value. Each listener slot uses a small amount of RAM.'
    },
    'train-functions': {
        title: 'Max Train Functions',
        description: 'The highest function number this train node supports.',
        detail: 'Standard DCC defines F0\u2013F28, giving 29 functions. Some extended protocols go higher. This sets the upper bound for function get/set commands the node will accept. Functions beyond this number are ignored.'
    },
    'listener-verify': {
        title: 'Listener Alias Verification',
        description: 'Timing parameters for verifying that consist member (listener) aliases are still valid on the CAN bus.',
        detail: 'When a train has listeners (throttles in a consist), the node periodically probes their CAN aliases to detect if they have gone offline. Probe Tick Interval is how often the verification timer fires (in 100ms units). Probe Interval is how many ticks between probe attempts. Verify Timeout is how long to wait for a response before declaring the listener gone. The defaults work well for most CAN bus configurations.'
    },
    'listener-probe-tick': {
        title: 'Probe Tick Interval',
        description: 'How often the listener verification timer fires, in 100ms units.',
        detail: 'This is the base timer resolution for alias probing. A value of 1 means the timer fires every 100ms. Lower values give faster detection of lost listeners but add more bus traffic. The default of 1 is suitable for most configurations.'
    },
    'listener-probe-interval': {
        title: 'Probe Interval (ticks)',
        description: 'How many ticks between alias probe attempts for each listener.',
        detail: 'Controls how frequently the node checks whether a listener\u2019s CAN alias is still active. At the default tick interval of 100ms, a probe interval of 250 means a probe every 25 seconds. Lower values detect lost listeners faster but increase bus traffic.'
    },
    'listener-verify-timeout': {
        title: 'Verify Timeout (ticks)',
        description: 'How many ticks to wait for a probe response before declaring a listener gone.',
        detail: 'If a listener does not respond within this many ticks after being probed, the node considers it offline and removes it from the consist. At the default tick interval of 100ms, a timeout of 30 means 3 seconds. Increase this if your bus has high latency or heavy traffic.'
    },

    /* ---- Well Known Events: section-level help ---- */

    'wellknown-producers': {
        title: 'Register Producer',
        description: 'Check the Well-Known Event IDs that this node produces (sends onto the bus).',
        detail: 'A producer tells the network "I generate this event." Configuration tools and gateways use producer/consumer identification to route events efficiently. Well-Known Automatically-Routed events (Emergency, Power Supply, Diagnostics) are forwarded through gateways to all segments. Well-Known events (Train, Firmware, CBUS, DCC) are not automatically routed but have globally defined meanings. See Event Identifiers Standard \u00a75.2\u20135.3.'
    },
    'wellknown-consumers': {
        title: 'Register Consumer',
        description: 'Check the Well-Known Event IDs that this node consumes (listens for on the bus).',
        detail: 'A consumer tells the network "I act on this event." When the event arrives on the bus, your node\u2019s callback is invoked. Well-Known Automatically-Routed events are forwarded through gateways automatically. Well-Known events are not auto-routed but gateways may maintain learned routing tables for them. See Event Identifiers Standard \u00a75.2\u20135.3.'
    },

    /* ---- Well Known Events: group headers ---- */

    'wke-group-emergency': {
        title: 'Emergency Events',
        description: 'Layout-wide safety events. Automatically routed through gateways to all OpenLCB segments.',
        detail: 'Emergency Off requests de-energization of outputs. Emergency Stop requests a safe state without necessarily removing power. Each has a corresponding Clear event to resume. These are Well-Known Automatically-Routed Event IDs (prefix 01.00). Gateways are required to forward them to all segments.'
    },
    'wke-group-power-supply': {
        title: 'Power Supply Events',
        description: 'Voltage monitoring events for diagnosing network power problems. Automatically routed through gateways.',
        detail: 'Two levels: Node-level brown-out (voltage below the node\u2019s own minimum) and Standard-level brown-out (voltage below a standard threshold such as the CAN bus 7.5V minimum). Because the node may be in a low-voltage state when these events are generated, successful transmission is not guaranteed.'
    },
    'wke-group-diagnostics': {
        title: 'Node Diagnostics Events',
        description: 'Diagnostic and identification events. Most are automatically routed; Duplicate Node ID is not.',
        detail: 'Includes log entry notifications, identification button press, duplicate Node ID detection, and link layer error codes. Link error meanings are wire-protocol specific (CAN, TCP/IP, etc.). The Duplicate Node ID event (01.01 prefix) is a Well-Known event, not automatically routed.'
    },
    'wke-group-train': {
        title: 'Train Protocol Events',
        description: 'Events used by the OpenLCB Train Control protocol. Not automatically routed.',
        detail: 'The "This node is a Train" event (01.01.00.00.00.00.03.03) allows throttles and configuration tools to discover locomotive nodes via the Train Search Protocol. Train nodes produce this event; train controllers consume it.'
    },
    'wke-group-firmware': {
        title: 'Firmware Events',
        description: 'Events related to firmware integrity and upgrade requests. Not automatically routed.',
        detail: 'Firmware Corrupted (01.01.00.00.00.00.06.01) signals a failed integrity check on startup. Firmware Upgrade by Hardware Switch (06.02) signals the user activated a physical upgrade mechanism. Configuration tools can watch for these events to prompt firmware recovery.'
    },
    'wke-group-cbus': {
        title: 'CBUS (MERG) Events',
        description: 'Maps MERG CBUS events into the OpenLCB event space. Not automatically routed. Registered as ranges.',
        detail: 'OpenLCB allocates a Node ID space for mapping CBUS events. CBUS events come in two types: long (uniquely identified by CBUS Node ID and Event ID) and short (using Node ID 00.00). The ON and OFF ranges each cover the full CBUS event space, enabling interoperability between OpenLCB and CBUS networks.'
    },
    'wke-group-dcc': {
        title: 'DCC Events',
        description: 'Maps DCC accessory commands, turnout feedback, and sensor feedback into the OpenLCB event space. Registered as ranges.',
        detail: 'These Well-Known Event ID ranges provide a suggested mapping between OpenLCB events and classic DCC accessories per NMRA S-9.2.1. Addresses use 11-bit accessory decoder format (0\u20134095) with a pair bit. The OpenLCB format stores address bits in standard (non 1\u2019s complement) form. Other mapping schemes are possible; this one is not required. Not automatically routed.'
    },

    /* ---- Well Known Events: individual items ---- */

    'wke-emergency-off': {
        title: 'Emergency Off (De-energize)',
        description: 'Request all nodes on the layout to de-energize their outputs immediately. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.FF. A node receiving this event should de-energize any outputs unrelated to maintaining OpenLCB communications. The node may continue as a powered bus participant. The meaning of "de-energize" is node-specific: a DCC power station might disable its amplified output; an accessory node might remove power from turnout motors. A node is not expected to know about Emergency Off status that was set before it joined the network.'
    },
    'wke-clear-emergency-off': {
        title: 'Clear Emergency Off (Energize)',
        description: 'Resume normal operation after an Emergency Off. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.FE. A node may revert to its previous energized state upon receiving this event. Nodes that joined the network after the original Emergency Off are not expected to react until the next Emergency Off or Clear Emergency Off event is produced.'
    },
    'wke-emergency-stop': {
        title: 'Emergency Stop',
        description: 'Command all nodes to enter a safe state. Outputs are not required to de-energize. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.FD. The meaning of "safe state" is defined by the node manufacturer or user. For example, a DCC command station might send the global emergency stop command; an accessory node might route outputs to a default or user-defined state. Unlike Emergency Off, power may remain applied.'
    },
    'wke-clear-emergency-stop': {
        title: 'Clear Emergency Stop',
        description: 'Resume normal operation after an Emergency Stop. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.FC. Nodes may revert to their previous non-Emergency Stop state. A node that recently joined the network is not expected to react to the current Emergency Stop status until the next Emergency Stop or Clear event is produced.'
    },
    'wke-brownout-node': {
        title: 'Power Supply Brown-out (Node)',
        description: 'This node detected a voltage below its own minimum operating level. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.F1. A generic indication that the node does not have enough voltage to function properly, regardless of source. The node is not required to know the exact voltage threshold. Because the node is in a brown-out state, the event may not be transmitted successfully. Some implementations log the condition and send the event after voltage recovers.'
    },
    'wke-brownout-standard': {
        title: 'Power Supply Brown-out (Standard)',
        description: 'This node detected a voltage below a level required by an OpenLCB standard (e.g. CAN bus minimum of 7.5V). Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.F0. Unlike the node-level brown-out, this event implies a violation of a specific OpenLCB standard threshold. Both brown-out events, when observed, are useful for diagnosing network power problems. The trade-off between false positives and false negatives in detection is left to the node designer.'
    },
    'wke-new-log': {
        title: 'Node Recorded a New Log Entry',
        description: 'Indicates that this node has written a new entry to its internal diagnostic log. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FF.F8. The contents of the log entry are not carried in the event itself. A configuration tool can retrieve the log through other protocols (e.g. Memory Configuration reads) after seeing this event.'
    },
    'wke-ident-button': {
        title: 'Identification Button Pressed',
        description: 'The user pressed the physical identification button combination on this node. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FE.00. This event lets configuration tools discover which physical node the user is pointing at. When a tool sees this event, it can highlight that node in its UI.'
    },
    'wke-duplicate-node': {
        title: 'Duplicate Node ID Detected',
        description: 'This node received a packet from another node using the same Node ID. Not automatically routed through gateways.',
        detail: 'Event ID: 01.01.00.00.00.00.02.01. Typically sent when a node sees its own Node ID in a message it did not originate. This is a serious configuration error that must be resolved by assigning unique Node IDs. Production of this event is not explicitly required by the standard.'
    },
    'wke-link-error-1': {
        title: 'Link Error Code 1',
        description: 'Link layer error \u2014 specific meaning is wire-protocol dependent. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FD.01. The meaning of each link error code is defined by the physical layer standard in use (e.g. CAN, TCP/IP). Consult the relevant physical layer standard for details.'
    },
    'wke-link-error-2': {
        title: 'Link Error Code 2',
        description: 'Link layer error \u2014 specific meaning is wire-protocol dependent. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FD.02.'
    },
    'wke-link-error-3': {
        title: 'Link Error Code 3',
        description: 'Link layer error \u2014 specific meaning is wire-protocol dependent. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FD.03.'
    },
    'wke-link-error-4': {
        title: 'Link Error Code 4',
        description: 'Link layer error \u2014 specific meaning is wire-protocol dependent. Automatically routed through gateways.',
        detail: 'Event ID: 01.00.00.00.00.00.FD.04.'
    },
    'wke-train': {
        title: 'This Node Is a Train',
        description: 'Identifies this node as an OpenLCB train (locomotive). Not automatically routed through gateways.',
        detail: 'Event ID: 01.01.00.00.00.00.03.03. Used by the Train Search Protocol so throttles can discover locomotive nodes on the bus. This event is automatically registered as a producer when you select the Train node type, and as a consumer for Train Controller nodes. See Train Control Standard.'
    },
    'wke-firmware-corrupted': {
        title: 'Firmware Corrupted',
        description: 'Indicates that this node\u2019s firmware image has been detected as corrupt. Not automatically routed through gateways.',
        detail: 'Event ID: 01.01.00.00.00.00.06.01. A node may produce this event on startup if a firmware integrity check (e.g. CRC) fails. Configuration tools can watch for this event to alert the user that a firmware re-flash is needed.'
    },
    'wke-firmware-hw-switch': {
        title: 'Firmware Upgrade by Hardware Switch',
        description: 'The user activated a physical switch or jumper requesting a firmware upgrade. Not automatically routed through gateways.',
        detail: 'Event ID: 01.01.00.00.00.00.06.02. This event signals that the node is ready to accept a new firmware image. The physical mechanism (button, DIP switch, jumper) is manufacturer-defined.'
    },
    'wke-cbus-off': {
        title: 'CBUS Off Event Space (Range)',
        description: 'Maps MERG CBUS OFF requests into the OpenLCB event space. Registered as a range. Not automatically routed.',
        detail: 'Event ID range: 01.01.01.01.{CBUS Node ID}.{CBUS Event ID}. CBUS short events use Node ID 00.00. This mapping allows OpenLCB nodes to interoperate with CBUS devices on layouts that bridge both protocols. A single producer/consumer range entry covers the entire CBUS OFF space.'
    },
    'wke-cbus-on': {
        title: 'CBUS On Event Space (Range)',
        description: 'Maps MERG CBUS ON requests into the OpenLCB event space. Registered as a range. Not automatically routed.',
        detail: 'Event ID range: 01.01.00.00.01.00.{CBUS Node ID}.{CBUS Event ID}. CBUS short events use Node ID 00.00. This mapping allows OpenLCB nodes to interoperate with CBUS devices. CBUS events may also arrive as Identify Producer requests, requiring both ON and OFF range responses.'
    },
    'wke-dcc-acc-activate': {
        title: 'DCC Accessory Activate (Range)',
        description: 'Activate a basic DCC accessory decoder address. Covers 4096 addresses (0\u20134095). Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FF.{addr}. Bytes 7\u20138 encode the 11-bit accessory decoder address plus a pair bit (R) per NMRA S-9.2.1. The address bits are in standard (non 1\u2019s complement) form in the OpenLCB mapping. Not automatically routed through gateways.'
    },
    'wke-dcc-acc-deactivate': {
        title: 'DCC Accessory Deactivate (Range)',
        description: 'Deactivate a basic DCC accessory decoder address. Covers 4096 addresses (0\u20134095). Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FE.{addr}. Same address encoding as Activate. This scheme is a suggestion for mapping OpenLCB events to classic DCC accessories; other mapping schemes are possible. Not automatically routed.'
    },
    'wke-dcc-turnout-high': {
        title: 'DCC Turnout Feedback High/Thrown (Range)',
        description: 'Report DCC turnout feedback active/on/high. Covers 4096 addresses. Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FD.{addr}. Used to report actual turnout state. Some proprietary protocols define messages that can be mapped to these events. Address encoding matches the basic DCC accessory scheme. Not automatically routed.'
    },
    'wke-dcc-turnout-low': {
        title: 'DCC Turnout Feedback Low/Closed (Range)',
        description: 'Report DCC turnout feedback inactive/off/low. Covers 4096 addresses. Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FC.{addr}. Complement of the High/Thrown feedback event. Not automatically routed.'
    },
    'wke-dcc-sensor-high': {
        title: 'DCC Sensor Feedback High/Occupied (Range)',
        description: 'Report DCC system sensor active/on/high. Covers 4096 sensor addresses (0\u20134095). Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FB.{addr}. Bytes 7\u20138 contain the 12-bit sensor address. Proprietary network protocols may define messages for generic sensor feedback that can be mapped to these events. Not automatically routed.'
    },
    'wke-dcc-sensor-low': {
        title: 'DCC Sensor Feedback Low/Clear (Range)',
        description: 'Report DCC system sensor inactive/off/low. Covers 4096 sensor addresses (0\u20134095). Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.00.FA.{addr}. Complement of the High/Occupied sensor event. Not automatically routed.'
    },
    'wke-dcc-ext-acc': {
        title: 'DCC Extended Accessory Command (Range)',
        description: 'Send a command to an extended DCC accessory decoder address. Registered as a range.',
        detail: 'Event ID range: 01.01.02.00.01.{addr}.{cmd}. The 11-bit extended accessory address is in bytes 6\u20137; byte 8 is the 8-bit command corresponding to the 3rd byte of a DCC extended accessory packet per NMRA S-9.2.1. Valid addresses are 0\u20132047. Not automatically routed.'
    },

    /* ---- Section-level help for collapsible panels ---- */

    'project-options': {
        title: 'Project Options',
        description: 'Name, author, and Node ID for this project. Used in generated file headers, ZIP archive names, and saved project files.',
        detail: 'Project Name becomes the base filename when downloading a ZIP or saving a project. Author is inserted into the copyright comment at the top of every generated source file. Node ID is the globally unique 48-bit identifier for this node on the OpenLCB network \u2014 enter it in xx.xx.xx.xx.xx.xx hexadecimal notation. Every device must have a unique Node ID; register your own range at https://registry.openlcb.org/uniqueidranges.'
    },

    'wellknown-events': {
        title: 'Well Known Events',
        description: 'Standard OpenLCB event IDs with globally defined meanings. Register them as producers (your node sends them) or consumers (your node acts on them).',
        detail: 'Well-Known Automatically-Routed events (Emergency, Power Supply, most Diagnostics) carry the prefix 01.00 and must be forwarded by gateways to every segment on the network. Well-Known events (Train, Firmware, CBUS, DCC) have globally defined meanings but are not auto-routed. Registrations are generated as openlcb_register_producer() / openlcb_register_consumer() calls in the main application file. If Broadcast Time is enabled, its range registrations are handled automatically by the protocol handler and do not appear here. See OpenLCB Event Identifiers Standard \u00a75.2\u20135.3.'
    },

    'add-ons': {
        title: 'Add-Ons',
        description: 'Optional protocol extensions: Broadcast Time (fast-clock) and Firmware Update (over-the-network firmware upgrade).',
        detail: 'Broadcast Time uses a structured set of event IDs to distribute model-railroad fast-clock time across the layout. Choose Producer if this node is the clock generator (one per layout), or Consumer if it tracks the time. Enabling either role reserves 2 producer ranges and 2 consumer ranges in your event buffer counts. Firmware Update relies on Memory Configuration (Memory Space 0xEF) and is not available for Basic nodes. When enabled, a configuration tool can freeze the node, write a new firmware image via datagrams or streaming, and unfreeze to restart. See OpenLCB Broadcast Time Standard and OpenLCB Firmware Upgrade Standard.'
    },

    'advanced': {
        title: 'Advanced',
        description: 'Memory buffer pool sizes, virtual node count, CAN message queue depth, and train protocol parameters. The defaults are suitable for most applications.',
        detail: 'Message Buffer Pool: each category allocates fixed-size blocks from RAM. Basic buffers (16 bytes) carry most protocol traffic. Datagram buffers (72 bytes) carry config-memory reads/writes. SNIP buffers (256 bytes) cache node identification strings. Stream buffers (512 bytes) are needed for streaming firmware upgrades. The combined total across all pools must not exceed 65534. Virtual Node Allocation sets how many logical nodes this hardware hosts; most simple devices use 1. CAN Message Buffers set the raw-frame queue depth \u2014 increase if the node is on a busy bus. Train Protocol and Listener Alias Verification settings only appear for Train and Train Controller node types.'
    }

};

var _elDetailPlaceholder   = document.getElementById('detail-placeholder');
var _elDetailContent       = document.getElementById('detail-content');
var _elDetailTitleBox      = document.getElementById('detail-title-box');
var _elDetailDesc          = document.getElementById('detail-desc');
var _elDetailDetailSection = document.getElementById('detail-detail-section');
var _elDetailDetail        = document.getElementById('detail-detail');

function _showHelp(key) {

    var info = HELP_DATA[key];
    if (!info || !_elDetailContent) { return; }

    _elDetailPlaceholder.style.display = 'none';
    _elDetailContent.style.display     = 'block';

    _elDetailTitleBox.textContent = info.title;
    _elDetailDesc.textContent    = info.description;

    if (info.detail) {

        _elDetailDetailSection.style.display = 'block';
        _elDetailDetail.textContent = info.detail;

    } else {

        _elDetailDetailSection.style.display = 'none';

    }

}

function _clearHelp() {

    if (!_elDetailContent) { return; }

    _elDetailContent.style.display     = 'none';
    _elDetailPlaceholder.style.display = 'block';

}

/* Attach hover listeners to all elements with data-help */
document.querySelectorAll('[data-help]').forEach(function (el) {

    el.addEventListener('mouseenter', function () {

        _showHelp(el.dataset.help);

    });

    el.addEventListener('mouseleave', function () {

        _clearHelp();

    });

});

/* ---------- Node type button bar ---------- */

document.querySelectorAll('.node-type-btn').forEach(function (btn) {

    btn.addEventListener('click', function () {

        if (btn.disabled) { return; }

        var type = btn.dataset.type;
        applyNodeType(type);

        /* Notify parent of the change */
        if (_isEmbedded) {
            window.parent.postMessage({
                type: 'nodeTypeChanged',
                nodeType: type
            }, '*');
        }

    });

});

/* ---------- Reset to Defaults ---------- */

document.getElementById('btn-reset-defaults').addEventListener('click', function () {

    if (!confirm('Reset all settings to their default values?')) { return; }

    /* Project Options */
    document.getElementById('project-name').value   = '';
    document.getElementById('project-author').value = '';
    document.getElementById('project-node-id').value = '';

    /* Config Memory */
    document.getElementById('config-unaligned-reads').checked  = true;
    document.getElementById('config-unaligned-writes').checked = true;
    document.getElementById('config-mem-highest-addr').value   = '0x200';

    /* Event Buffers */
    document.getElementById('event-producer-count').value       = '64';
    document.getElementById('event-consumer-count').value       = '64';
    document.getElementById('event-producer-range-count').value = '4';
    document.getElementById('event-consumer-range-count').value = '4';

    /* Well Known Events — uncheck all */
    WELL_KNOWN_EVENTS.forEach(function (evt) {
        var p = document.getElementById('wke-prod-' + evt.id);
        var c = document.getElementById('wke-cons-' + evt.id);
        if (p) { p.checked = false; p.disabled = false; }
        if (c) { c.checked = false; c.disabled = false; }
    });

    /* Add-Ons */
    var noneRadio = document.querySelector('input[name="addon-broadcast"][value="none"]');
    if (noneRadio) { noneRadio.checked = true; }
    document.getElementById('addon-firmware').checked = false;

    /* SNIP text fields */
    document.getElementById('snip-manufacturer').value = '';
    document.getElementById('snip-model').value        = '';
    document.getElementById('snip-hw-version').value   = '';
    document.getElementById('snip-sw-version').value   = '';

    /* Advanced — Message Buffer Pool */
    document.getElementById('adv-basic-buf').value    = '16';
    document.getElementById('adv-datagram-buf').value = '8';
    document.getElementById('adv-snip-buf').value     = '4';
    document.getElementById('adv-stream-buf').value   = '0';

    /* Advanced — Virtual Nodes & CAN */
    document.getElementById('adv-node-buf').value = '1';
    document.getElementById('adv-can-buf').value  = '20';

    /* Advanced — Train Protocol */
    document.getElementById('adv-train-node-count').value = '1';
    document.getElementById('adv-train-listeners').value  = '6';
    document.getElementById('adv-train-functions').value  = '29';

    /* Advanced — Listener Verification */
    document.getElementById('adv-probe-tick').value     = '1';
    document.getElementById('adv-probe-interval').value = '250';
    document.getElementById('adv-verify-timeout').value = '30';

    /* Update tallies */
    Object.keys(_TALLY_MAP).forEach(function (id) { _updateTally(id); });
    _updateBufferTotal();
    _updateBroadcastNote();

    /* Re-apply node type to reset type-dependent state (train locks, etc.) */
    if (selectedNodeType) {
        applyNodeType(selectedNodeType);
    } else {
        updatePreview();
    }

});

/* Notify parent that we are ready */
if (_isEmbedded) {

    window.parent.postMessage({ type: 'ready', view: 'config' }, '*');

}
