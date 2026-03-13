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

    /* Show the options section */
    document.getElementById('addons-section').classList.remove('hidden');

    updatePreview();

}

/* ---------- SNIP checkbox ---------- */

document.getElementById('addon-snip').addEventListener('change', function () {

    document.getElementById('snip-fields').classList.toggle('hidden', !this.checked);
    updatePreview();

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

/* ---------- Collect current UI state ---------- */

function getState() {

    const broadcastEl = document.querySelector('input[name="addon-broadcast"]:checked');
    const isTrainRole = selectedNodeType === 'train' || selectedNodeType === 'train-controller';
    const isTrainNode = selectedNodeType === 'train';   /* locomotive only — has FDI */
    const isBasic     = selectedNodeType === 'basic';

    /* For Typical/Train/TrainController, fall back to the embedded default CDI when no user file */
    const activeCdi    = (!isBasic && !cdiUserBytes) ? DEFAULT_CDI_BYTES : cdiUserBytes;
    const activeCdiXml = (!isBasic && !cdiUserText)  ? DEFAULT_CDI_XML  : cdiUserText;
    /* For Train (locomotive) only, fall back to the embedded default FDI when no user file */
    const activeFdi    = (isTrainNode && !fdiUserBytes)  ? DEFAULT_FDI_BYTES : fdiUserBytes;
    const activeFdiXml = (isTrainNode && !fdiUserText)   ? DEFAULT_FDI_XML  : fdiUserText;

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
        fdiLength:           activeFdi ? activeFdi.length : 1,
        fdiXml:              activeFdiXml || null,
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

/* ---------- Render the code preview ---------- */

function renderPreview() {

    const pre = document.getElementById('code-preview');

    if (!selectedNodeType) {

        pre.textContent = 'Select a node type to preview the generated files';
        pre.parentElement.classList.add('empty');
        pre.removeAttribute('data-highlighted');
        return;

    }

    pre.parentElement.classList.remove('empty');

    const state = getState();

    /* Rebuild the file dropdown (groups may have changed) */
    _rebuildFileBrowser();

    var code = _generateForTab(currentTab, state);
    pre.textContent = code;

    /* Apply syntax highlighting */
    pre.removeAttribute('data-highlighted');
    hljs.highlightElement(pre);

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

        /* Parent sends updated CDI bytes from the CDI editor */
        cdiUserText  = e.data.xml;
        const raw    = new TextEncoder().encode(cdiUserText);
        cdiUserBytes = new Uint8Array(raw.length + 1);
        cdiUserBytes.set(raw);
        cdiUserBytes[raw.length] = 0;
        updatePreview();

    } else if (e.data.type === 'setFdiBytes') {

        /* Parent sends updated FDI bytes from the FDI editor */
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
