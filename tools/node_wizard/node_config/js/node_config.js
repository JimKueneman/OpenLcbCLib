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
let driverStateFromParent   = {};   /* { 'can-drivers': { checked: [...] }, ... } */
let callbackStateFromParent = {};   /* { 'cb-events': { checked: [...] }, ... } */
let platformStateFromParent = null; /* { platform, params, framework, libraries, notes } */
let arduinoModeFromParent   = false;

const _isEmbedded = window.parent !== window;

/* ---------- Node type configuration ---------- */

function applyNodeType(type) {

    selectedNodeType = type;

    /* Firmware: Basic node has no Config Memory, so firmware is unavailable */
    const firmwareCheckbox = document.getElementById('addon-firmware');
    const firmwareGroup    = document.getElementById('addon-firmware-label');
    const firmwareNote     = document.getElementById('firmware-note');

    if (type === 'basic') {

        firmwareCheckbox.checked  = false;
        firmwareCheckbox.disabled = true;
        firmwareGroup.classList.add('disabled');
        firmwareNote.classList.remove('hidden');

    } else {

        firmwareCheckbox.disabled = false;
        firmwareGroup.classList.remove('disabled');
        firmwareNote.classList.add('hidden');

    }

    /* SNIP: Basic-only add-on (Typical/Train populate SNIP from CDI) */
    const snipGroup    = document.getElementById('addon-snip-group');
    const snipCheckbox = document.getElementById('addon-snip');
    const snipFields   = document.getElementById('snip-fields');

    if (type === 'basic') {

        snipGroup.classList.remove('hidden');
        snipCheckbox.checked = true;
        snipFields.classList.remove('hidden');

    } else {

        snipGroup.classList.add('hidden');
        snipCheckbox.checked = false;
        snipFields.classList.add('hidden');

    }

    /* Config Memory options: Typical and Train only */
    const configGroup = document.getElementById('addon-config-group');
    configGroup.classList.toggle('hidden', type === 'basic');

    /* Advanced: Train Protocol and Listener groups — train/train-controller only */
    const isTrain = type === 'train' || type === 'train-controller';
    document.getElementById('adv-train-group').classList.toggle('hidden', !isTrain);
    document.getElementById('adv-listener-group').classList.toggle('hidden', !isTrain);

    /* Advanced: Set datagram buf default based on type */
    const dgBuf = document.getElementById('adv-datagram-buf');
    if (type === 'basic' && dgBuf.value === '8') {
        dgBuf.value = '0';
    } else if (type !== 'basic' && dgBuf.value === '0') {
        dgBuf.value = '8';
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
        var broadcastEl = document.querySelector('input[name="addon-broadcast"]:checked');
        var noteEl = document.getElementById('broadcast-note');
        if (noteEl) {
            noteEl.classList.toggle('hidden', !broadcastEl || broadcastEl.value === 'none');
        }
        updatePreview();
    });
});

/* Checkbox add-ons */
['addon-firmware', 'config-unaligned-reads', 'config-unaligned-writes'].forEach(id => {
    document.getElementById(id).addEventListener('change', updatePreview);
});

/* Text inputs */
['config-mem-highest-addr',
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

/* ---------- Collect current UI state ---------- */

function getState() {

    const broadcastEl = document.querySelector('input[name="addon-broadcast"]:checked');
    const isTrainRole = selectedNodeType === 'train' || selectedNodeType === 'train-controller';
    const isTrainNode = selectedNodeType === 'train';   /* locomotive only — has FDI */
    const isBasic     = selectedNodeType === 'basic';

    /* For Typical/Train/TrainController, fall back to the embedded default CDI when no user file */
    const activeCdi    = (!isBasic && !cdiUserBytes) ? DEFAULT_CDI_BYTES : cdiUserBytes;
    const activeCdiXml = (!isBasic && !cdiUserText)  ? DEFAULT_CDI_XML  : cdiUserText;
    /* For Train (locomotive) only, fall back to the embedded default FDI when no user file.
     * Non-train node types never use FDI, so force null even if the user loaded one earlier. */
    const activeFdi    = isTrainNode ? (fdiUserBytes  || DEFAULT_FDI_BYTES) : null;
    const activeFdiXml = isTrainNode ? (fdiUserText   || DEFAULT_FDI_XML)  : null;

    return {
        nodeType:         selectedNodeType,
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
        platformState:       platformStateFromParent
    };

}

/* ---------- Tab switching ---------- */

function _getMainFilename() {

    return arduinoModeFromParent ? 'main.ino' : 'main.c';

}

const TAB_LABELS_STATIC = {
    h: 'openlcb_user_config.h',
    c: 'openlcb_user_config.c'
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
    document.getElementById('preview-filename').textContent = label;

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
        return group.filePrefix + (ext === '-h' ? '.h' : '.c');
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

    group.functions.forEach(function (fn) {

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
        { tab: 'main', filename: _getMainFilename() }
    ]);

    /* Drivers — single section, all active driver groups merged */
    if (typeof DRIVER_GROUPS !== 'undefined') {

        var driverItems = [];

        Object.keys(DRIVER_GROUPS).forEach(function (key) {

            var fns = _getActiveFunctions(key);
            if (fns.length === 0) { return; }

            var group = DRIVER_GROUPS[key];
            driverItems.push({ tab: key + '-h', filename: group.filePrefix + '.h' });
            driverItems.push({ tab: key + '-c', filename: group.filePrefix + '.c' });

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
            callbackItems.push({ tab: key + '-c', filename: group.filePrefix + '.c' });

        });

        if (callbackItems.length > 0) {
            _addSection('Callbacks', callbackItems);
        }

    }

    /* If saved tab is no longer valid, fall back to 'h' */
    var hasTab = browser.querySelector('.fb-item[data-tab="' + savedTab + '"]');
    if (!hasTab) {

        currentTab = 'h';
        var first = browser.querySelector('.fb-item[data-tab="h"]');
        if (first) { first.classList.add('selected'); }

    }

}

/* ---------- CodeMirror read-only viewer ---------- */

let _cmViewer = null;

function _ensureCMViewer() {

    if (_cmViewer) { return _cmViewer; }

    var container = document.getElementById('code-preview');
    _cmViewer = createCMReadonly(container);
    return _cmViewer;

}

/* ---------- Render the code preview ---------- */

function renderPreview() {

    var viewer = _ensureCMViewer();

    if (!selectedNodeType) {

        viewer.value = '// Select a node type to preview the generated files';
        return;

    }

    const state = getState();

    /* Rebuild the file dropdown (groups may have changed) */
    _rebuildFileBrowser();

    var code = _generateForTab(currentTab, state);
    viewer.value = code;

    /* Check config memory size vs CDI requirements */
    _checkConfigMemSize(state);

    /* Notify parent of state change */
    _postStateToParent(state);

}

/* ---------- Generate code for a given tab ---------- */

function _generateForTab(tab, state) {

    /* Core config files */
    if (tab === 'h')    { return generateH(state); }
    if (tab === 'c')    { return generateC(state); }
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
            return DriverCodegen.generateC(group, fns, platformStateFromParent);
        }

    }

    /* Callback group */
    if (typeof CALLBACK_GROUPS !== 'undefined' && CALLBACK_GROUPS[groupKey]) {

        var group = CALLBACK_GROUPS[groupKey];
        var fns = _getActiveFunctions(groupKey);

        if (ext === '-h') {
            return CallbackCodegen.generateH(group, fns);
        } else {
            return CallbackCodegen.generateC(group, fns);
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

function updatePreview() {

    renderPreview();

}

/* ---------- Download generated files ---------- */

function downloadFiles() {

    if (!selectedNodeType) { return; }

    const state = getState();
    _download('openlcb_user_config.h', generateH(state));
    _download('openlcb_user_config.c', generateC(state));
    _download('main.c', generateMain(state));

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

/* ---------- Splitter drag ---------- */

(function () {

    const splitter   = document.getElementById('splitter');
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
        const raw    = new TextEncoder().encode(cdiUserText);
        cdiUserBytes = new Uint8Array(raw.length + 1);
        cdiUserBytes.set(raw);
        cdiUserBytes[raw.length] = 0;
        updatePreview();

    } else if (e.data.type === 'setFdiBytes') {

        /* Parent sends updated FDI bytes from FDI editor */
        fdiUserText  = e.data.xml;
        const raw    = new TextEncoder().encode(fdiUserText);
        fdiUserBytes = new Uint8Array(raw.length + 1);
        fdiUserBytes.set(raw);
        fdiUserBytes[raw.length] = 0;
        updatePreview();

    } else if (e.data.type === 'setConfigMemSize') {

        /* CDI editor requested a config memory size update */
        document.getElementById('config-mem-highest-addr').value = e.data.value;
        updatePreview();

    } else if (e.data.type === 'restoreFormState') {

        /* Parent sends saved form values to restore */
        const f = e.data.formState;
        if (!f) { return; }

        /* Radio: broadcast */
        if (f.broadcast) {
            const radio = document.querySelector('input[name="addon-broadcast"][value="' + f.broadcast + '"]');
            if (radio) { radio.checked = true; }
        }

        /* Broadcast note visibility */
        var bcastNoteEl = document.getElementById('broadcast-note');
        if (bcastNoteEl) {
            bcastNoteEl.classList.toggle('hidden', !f.broadcast || f.broadcast === 'none');
        }

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

        /* Splitter width */
        if (f.splitterWidth) {
            document.getElementById('config-panel').style.flex = '0 0 ' + f.splitterWidth + 'px';
        }

        /* Tab */
        if (f.currentTab) { switchTab(f.currentTab); }

        updatePreview();

    } else if (e.data.type === 'setArduinoMode') {

        arduinoModeFromParent = !!e.data.arduino;
        _rebuildFileBrowser();
        /* Update filename display if on the main tab */
        if (currentTab === 'main') {
            document.getElementById('preview-filename').textContent = _getMainFilename();
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

/* Notify parent that we are ready */
if (_isEmbedded) {

    window.parent.postMessage({ type: 'ready', view: 'config' }, '*');

}
