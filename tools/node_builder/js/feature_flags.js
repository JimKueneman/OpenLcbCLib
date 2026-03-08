/* ============================================================================
 * feature_flags.js — Feature flag checkboxes, dependency cascade,
 *                    sidebar nav visibility, and Event System disable logic
 * ============================================================================ */

/* Dependency rules:
 *   Firmware        → requires Memory Configuration → requires Datagrams
 *   Broadcast Time  → requires Events
 *   Stream          → requires Datagrams + Memory Configuration (future)
 *   Train           → (no auto-deps for now)
 */

var _cascading = false;   // guard against re-entry during cascade

function onFeatureFlagChange(sourceId) {

    if (_cascading) return;
    _cascading = true;

    // ── Auto-check prerequisites (checking a flag) ──
    if (sourceId === 'flag-firmware') {

        if (isChecked('flag-firmware')) {
            setCheckbox('flag-config-mem', true);
            setCheckbox('flag-datagrams', true);
        }

    } else if (sourceId === 'flag-config-mem') {

        if (isChecked('flag-config-mem')) {
            setCheckbox('flag-datagrams', true);
        } else {
            setCheckbox('flag-firmware', false);
        }

    } else if (sourceId === 'flag-datagrams') {

        if (!isChecked('flag-datagrams')) {
            setCheckbox('flag-config-mem', false);
            setCheckbox('flag-firmware', false);
        }

    } else if (sourceId === 'flag-broadcast-time') {

        if (isChecked('flag-broadcast-time')) {
            setCheckbox('flag-events', true);
        }

    } else if (sourceId === 'flag-events') {

        if (!isChecked('flag-events')) {
            setCheckbox('flag-broadcast-time', false);
        }

    }

    _cascading = false;

    // ── Read final state ──
    var events     = isChecked('flag-events');
    var datagrams  = isChecked('flag-datagrams');
    var configMem  = isChecked('flag-config-mem');
    var firmware   = isChecked('flag-firmware');
    var broadcastT = isChecked('flag-broadcast-time');
    var train      = isChecked('flag-train');
    var stream     = isChecked('flag-stream');

    // Badge count
    var count = 0;
    if (events) count++;
    if (datagrams) count++;
    if (configMem) count++;
    if (firmware) count++;
    if (broadcastT) count++;
    if (train) count++;
    if (stream) count++;
    document.getElementById('badge-features').textContent = count + ' enabled';

    // ── Compact file-pickers inside Feature Flags ──
    showHide('ff-cdi-picker', configMem);
    showHide('ff-fdi-picker', train);

    // ── Stream checkbox drives stream read/write in config options ──
    var streamCb = document.getElementById('cfgopt-stream-rw');
    if (streamCb) {
        streamCb.disabled = !stream;
        if (!stream) streamCb.checked = false;
    }

    // ── Protocol Support (inside Feature Flags) ──
    updateProtocolSupport();

    // ── Sidebar nav item visibility ──
    showHide('nav-config-options', configMem);
    showHide('nav-train', train);

    // ── Address space tabs ──
    showHide('tab-0xff', configMem);
    showHide('tab-0xfe', configMem);
    showHide('tab-0xfd', configMem);
    showHide('tab-0xfc', configMem);
    showHide('tab-0xfb', configMem);
    showHide('tab-0xef', firmware);
    showHide('tab-0xfa', train);
    showHide('tab-0xf9', train);

    // If the currently active addr tab became hidden, switch to first visible tab
    var activeTab = document.querySelector('.addr-tab-btn.active');
    if (activeTab && activeTab.classList.contains('hidden')) {
        var firstVisible = document.querySelector('.addr-tab-btn:not(.hidden)');
        if (firstVisible) {
            var spaceId = firstVisible.id.replace('tab-', '');
            switchAddrTab(spaceId);
        }
    }

    // ── If active content panel's nav item is now hidden, switch to Feature Flags ──
    var activePanel = document.querySelector('.content-panel.active');
    if (activePanel) {
        var navId = activePanel.id.replace('panel-', 'nav-');
        var navItem = document.getElementById(navId);
        if (navItem && navItem.classList.contains('hidden')) {
            selectPanel('panel-features');
        }
    }

    // ── Event System: disable/enable fields ──
    var evtFields = ['evt-producers', 'evt-producer-ranges', 'evt-consumers', 'evt-consumer-ranges'];
    var evtMsg = document.getElementById('events-disabled-msg');
    for (var i = 0; i < evtFields.length; i++) {
        document.getElementById(evtFields[i]).disabled = !events;
    }
    if (evtMsg) evtMsg.style.display = events ? 'none' : 'block';

    // ── SNIP field state (disabled when Config Mem on, editable when off) ──
    updateSnipFieldState();

    // ── CDI / FDI "not loaded" warnings ──
    updateCdiNeededWarnings();
    updateFdiNeededWarnings();

    // Cascade to other modules
    updateCallbacksList();
}

/* ── SNIP field state management ──
 * Config Mem OFF           → fields editable
 * Config Mem ON, no CDI    → fields disabled, note to load CDI
 * Config Mem ON, CDI loaded → fields readonly (set by extractCDIIdentification)
 */
function updateSnipFieldState() {
    var configMem = isChecked('flag-config-mem');
    var hasCdi = cdiBytes && cdiBytes.length > 0;
    var snipFields = ['snip-name', 'snip-model', 'snip-hw-version', 'snip-sw-version'];

    if (configMem && hasCdi) {
        // CDI loaded — fields are readonly (already set by extractCDIIdentification)
        showHide('snip-config-mem-note', false);
        showHide('snip-cdi-note', true);
    } else if (configMem) {
        // Config Mem enabled but no CDI — disable fields
        for (var i = 0; i < snipFields.length; i++) {
            var el = document.getElementById(snipFields[i]);
            el.removeAttribute('readonly');
            el.disabled = true;
        }
        showHide('snip-config-mem-note', true);
        showHide('snip-cdi-note', false);
    } else {
        // Config Mem disabled — fields are editable
        for (var i = 0; i < snipFields.length; i++) {
            var el = document.getElementById(snipFields[i]);
            el.removeAttribute('readonly');
            el.disabled = false;
        }
        showHide('snip-config-mem-note', false);
        showHide('snip-cdi-note', false);
    }
}

/* ── CDI / FDI "not loaded" warnings ──
 * Show red warning in panels that depend on CDI/FDI data when the
 * relevant file has not been loaded yet.
 */

function updateCdiNeededWarnings() {
    var configMem = isChecked('flag-config-mem');
    var hasCdi = cdiBytes && cdiBytes.length > 0;
    showHide('cdi-needed-0xff', configMem && !hasCdi);
}

function updateFdiNeededWarnings() {
    var train = isChecked('flag-train');
    var hasFdi = fdiBytes && fdiBytes.length > 0;
    showHide('fdi-needed-0xfa', train && !hasFdi);
}

/* ── Address space tab switching ── */

function switchAddrTab(spaceId) {

    var btns = document.querySelectorAll('.addr-tab-btn');
    for (var i = 0; i < btns.length; i++) {
        btns[i].classList.remove('active');
    }

    var panes = document.querySelectorAll('.addr-tab-pane');
    for (var i = 0; i < panes.length; i++) {
        panes[i].classList.remove('active');
    }

    var btn = document.getElementById('tab-' + spaceId);
    if (btn) btn.classList.add('active');

    var pane = document.getElementById('pane-' + spaceId);
    if (pane) pane.classList.add('active');
}

/* ── Drag-and-drop for CDI/FDI file-picker rows ── */

function initDragDrop() {

    setupDropZone('ff-cdi-picker', function(file) {
        loadCdiFile(file);
    });

    setupDropZone('ff-fdi-picker', function(file) {
        loadFdiFile(file);
    });
}

function setupDropZone(elementId, onFileDrop) {

    var el = document.getElementById(elementId);
    if (!el) return;

    el.addEventListener('dragover', function(e) {
        e.preventDefault();
        e.stopPropagation();
        el.classList.add('drag-over');
    });

    el.addEventListener('dragleave', function(e) {
        e.preventDefault();
        e.stopPropagation();
        el.classList.remove('drag-over');
    });

    el.addEventListener('drop', function(e) {
        e.preventDefault();
        e.stopPropagation();
        el.classList.remove('drag-over');
        if (e.dataTransfer.files.length > 0) {
            onFileDrop(e.dataTransfer.files[0]);
        }
    });
}
