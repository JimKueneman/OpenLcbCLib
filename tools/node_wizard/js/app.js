/* =========================================================================
 * Node Wizard — app.js (sidebar shell controller)
 * Manages sidebar navigation, iframe switching, postMessage relay,
 * and persistence of all wizard state to localStorage.
 * ========================================================================= */

const STORAGE_KEY = 'node_wizard_state';

/* ---------- State ---------- */

let selectedNodeType  = null;
let currentView       = null;    /* 'config' | 'cdi' | 'fdi' | 'platform-drivers' | 'callbacks' | 'file-preview' */
let cdiUserXml        = null;    /* XML text from CDI editor */
let fdiUserXml        = null;    /* XML text from FDI editor */
let cdiFilename       = null;    /* loaded CDI filename */
let fdiFilename       = null;    /* loaded FDI filename */
let configMemHighest  = 0x200;   /* config memory size — tracked from node_config */
let configFormState   = null;    /* last-known form state from node_config */
let driverState       = {};      /* per-group state from drivers iframe, keyed by group */
let callbackState     = {};      /* per-group state from callbacks iframe, keyed by group */
let platformState     = null;    /* { platform, params, isArduino, framework, libraries, notes } */
let cdiValidation     = null;    /* { errors: N, warnings: N } from CDI editor */
let fdiValidation     = null;    /* { errors: N, warnings: N } from FDI editor (future) */
let filePreviewSelection = null; /* last selected file path in file-preview */
let preserveWhitespace   = false; /* CDI editor: preserve whitespace in output byte array */

/* ---------- DOM refs ---------- */

const elIframe        = document.getElementById('workspace-iframe');
const elEmpty         = document.getElementById('workspace-empty');

/* Node Type tile */
const elTileNodeType  = document.getElementById('tile-node-type');

/* Descriptor tiles */
const elTileCdi       = document.getElementById('tile-cdi');
const elTileFdi       = document.getElementById('tile-fdi');
const elBadgeCdi      = document.getElementById('badge-cdi');
const elBadgeFdi      = document.getElementById('badge-fdi');

/* New tiles */
const elTilePlatformDrivers = document.getElementById('tile-platform-drivers');
const elTileCallbacks       = document.getElementById('tile-callbacks');
const elTileFilePreview     = document.getElementById('tile-file-preview');

/* All selectable view tiles (for highlight management) */
const VIEW_TILES = {
    'config':           elTileNodeType,
    'cdi':              elTileCdi,
    'fdi':              elTileFdi,
    'platform-drivers': elTilePlatformDrivers,
    'callbacks':        elTileCallbacks,
    'file-preview':     elTileFilePreview
};

/* ---------- iframe paths ---------- */

const VIEW_URLS = {
    'config':           'node_config/node_config.html',
    'cdi':              'cdi_editor/cdi_view.html',
    'fdi':              'fdi_editor/fdi_editor.html',
    'platform-drivers': 'platform/platform.html',
    'callbacks':        'callbacks/callbacks.html',
    'file-preview':     'file_preview/file_preview.html'
};

/* Track which view is currently loaded in the iframe */
let _loadedView   = null;
let _iframeReady  = false;
let _pendingMsgs  = [];   /* messages to send once iframe is ready */

/* ---------- Derived state helpers ---------- */

function _isArduino() {

    return !!(platformState && platformState.isArduino);

}

function _getAddons() {

    return {
        broadcast: (configFormState && configFormState.broadcast) || 'none',
        firmware:  !!(configFormState && configFormState.firmware)
    };

}

/* ---------- Persistence -------------------------------------------------- */

let _saveTimer = null;

function _saveState() {

    clearTimeout(_saveTimer);
    _saveTimer = setTimeout(function () {

        const state = {
            selectedNodeType: selectedNodeType,
            currentView:      currentView,
            cdiUserXml:       cdiUserXml,
            fdiUserXml:       fdiUserXml,
            cdiFilename:      cdiFilename,
            fdiFilename:      fdiFilename,
            configMemHighest: configMemHighest,
            configFormState:  configFormState,
            driverState:      driverState,
            callbackState:    callbackState,
            platformState:    platformState,
            filePreviewSelection: filePreviewSelection,
            preserveWhitespace: preserveWhitespace
        };

        try {
            localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
        } catch (e) { /* quota exceeded — ignore */ }

    }, 300);

}

function _restoreState() {

    try {

        const raw = localStorage.getItem(STORAGE_KEY);
        if (!raw) { return; }

        const state = JSON.parse(raw);

        if (state.selectedNodeType) {
            selectedNodeType = state.selectedNodeType;
        }
        if (state.currentView) {
            currentView = state.currentView;
        }
        if (state.cdiUserXml) {
            cdiUserXml = state.cdiUserXml;
        }
        if (state.fdiUserXml) {
            fdiUserXml = state.fdiUserXml;
        }
        if (state.cdiFilename) {
            cdiFilename = state.cdiFilename;
        }
        if (state.fdiFilename) {
            fdiFilename = state.fdiFilename;
        }
        if (state.configMemHighest) {
            configMemHighest = parseInt(state.configMemHighest) || 0x200;
        }
        if (state.configFormState) {
            configFormState = state.configFormState;
        }
        if (state.driverState) {
            driverState = state.driverState;
        }
        if (state.callbackState) {
            callbackState = state.callbackState;
        }
        if (state.platformState) {
            platformState = state.platformState;
        }
        if (state.filePreviewSelection) {
            filePreviewSelection = state.filePreviewSelection;
        }
        if (state.preserveWhitespace !== undefined) {
            preserveWhitespace = !!state.preserveWhitespace;
        }

        /* Migration: convert old arduinoMode to platform isArduino */
        if (state.arduinoMode && platformState && !platformState.isArduino) {
            platformState.isArduino = true;
        }

    } catch (e) { /* corrupt data — ignore */ }

}

/* ---------- Sidebar tile filename display ---------- */

function _updateTileFilename(tileEl, filename, defaultDesc) {

    const descEl = tileEl.querySelector('.tile-desc');
    if (!descEl) { return; }

    if (filename) {
        descEl.textContent = filename;
        descEl.title       = filename;
        descEl.classList.add('has-file');
    } else {
        descEl.textContent = defaultDesc;
        descEl.title       = '';
        descEl.classList.remove('has-file');
    }

}

/* ---------- Update platform drivers tile description ---------- */

function _updatePlatformTile() {

    var descEl = elTilePlatformDrivers.querySelector('.tile-desc');
    if (!descEl) { return; }

    if (platformState && platformState.platform && platformState.platform !== 'none') {

        var names = {
            'esp32-twai': 'ESP32 + TWAI',
            'esp32-wifi': 'ESP32 + WiFi',
            'rp2040-mcp2517': 'RP2040 + MCP2517',
            'stm32f4': 'STM32F4 + CAN',
            'ti-mspm0': 'TI MSPM0 + MCAN'
        };
        descEl.textContent = names[platformState.platform] || platformState.platform;
        descEl.classList.add('has-file');

    } else {

        descEl.textContent = 'None / Custom';
        descEl.classList.remove('has-file');

    }

}

/* ---------- Warning badges for descriptors ------------------------------- */

function _computeCdiBadge() {

    var needsCdi = selectedNodeType && selectedNodeType !== 'basic' && selectedNodeType !== 'bootloader';
    if (needsCdi && !cdiUserXml) {
        return { visible: true, error: false, title: 'CDI not loaded — required for this node type' };
    } else if (needsCdi && cdiValidation && cdiValidation.errors > 0) {
        return { visible: true, error: true, title: cdiValidation.errors + ' validation error' + (cdiValidation.errors > 1 ? 's' : '') };
    } else if (needsCdi && cdiValidation && cdiValidation.warnings > 0) {
        return { visible: true, error: false, title: cdiValidation.warnings + ' validation warning' + (cdiValidation.warnings > 1 ? 's' : '') };
    }
    return { visible: false, error: false, title: '' };

}

function _computeFdiBadge() {

    var needsFdi = selectedNodeType === 'train';
    if (needsFdi && !fdiUserXml) {
        return { visible: true, error: false, title: 'FDI not loaded — required for Train nodes' };
    } else if (needsFdi && fdiValidation && fdiValidation.errors > 0) {
        return { visible: true, error: true, title: fdiValidation.errors + ' validation error' + (fdiValidation.errors > 1 ? 's' : '') };
    } else if (needsFdi && fdiValidation && fdiValidation.warnings > 0) {
        return { visible: true, error: false, title: fdiValidation.warnings + ' validation warning' + (fdiValidation.warnings > 1 ? 's' : '') };
    }
    return { visible: false, error: false, title: '' };

}

function _updateDescriptorBadges() {

    var cdiBadge = _computeCdiBadge();
    var fdiBadge = _computeFdiBadge();

    /* CDI badge */
    elBadgeCdi.classList.toggle('visible', cdiBadge.visible);
    elBadgeCdi.classList.toggle('badge-error', cdiBadge.error);
    elBadgeCdi.title = cdiBadge.title;

    /* FDI badge */
    elBadgeFdi.classList.toggle('visible', fdiBadge.visible);
    elBadgeFdi.classList.toggle('badge-error', fdiBadge.error);
    elBadgeFdi.title = fdiBadge.title;

    /* Update tile filenames */
    _updateTileFilename(elTileCdi, cdiFilename, 'Configuration Description');
    _updateTileFilename(elTileFdi, fdiFilename, 'Function Description');

}

/* ---------- Update Node Type tile description and icon ------------------- */

const NODE_TYPE_INFO = {
    'basic':            { icon: '\uD83D\uDCE6', name: 'Basic',            desc: 'Events only' },
    'typical':          { icon: '\u2699\uFE0F',  name: 'Typical',          desc: 'Events + CDI + Config' },
    'train':            { icon: '\uD83D\uDE82',  name: 'Train',            desc: 'Locomotive' },
    'train-controller': { icon: '\uD83C\uDFAE',  name: 'Train Controller', desc: 'Throttle' },
    'bootloader':       { icon: '\u26A1',        name: 'Bootloader',       desc: 'Firmware upgrade only' },
    'custom':           { icon: '\uD83D\uDD27',  name: 'Custom',           desc: 'Advanced \u2014 coming soon' }
};

function _updateNodeTypeTile() {

    var info = NODE_TYPE_INFO[selectedNodeType];
    var descEl = document.getElementById('tile-node-type-desc');
    var iconEl = elTileNodeType.querySelector('.tile-icon');

    if (info) {
        if (descEl) { descEl.textContent = info.name + ' \u2014 ' + info.desc; }
        if (iconEl) { iconEl.textContent = info.icon; }
    } else {
        if (descEl) { descEl.textContent = 'Select a type...'; }
        if (iconEl) { iconEl.textContent = '\uD83D\uDCE6'; }
    }

}

/* ---------- Enable/disable tiles based on node type ---------------------- */

function _updateTileStates(type) {

    var hasCfgMem    = type !== 'basic';
    var isTrainNode  = type === 'train';
    var isBootloader = type === 'bootloader';

    /* CDI — enabled for anything with config memory, but not bootloader */
    elTileCdi.classList.toggle('disabled', !hasCfgMem || isBootloader);

    /* FDI — only for train (locomotive) */
    elTileFdi.classList.toggle('disabled', !isTrainNode);

    /* Platform Drivers — enabled when a node type is selected */
    elTilePlatformDrivers.classList.remove('disabled');

    /* Callbacks — enabled when a node type is selected */
    elTileCallbacks.classList.remove('disabled');

    /* File Preview — enabled when a node type is selected */
    elTileFilePreview.classList.remove('disabled');

}

/* ---------- Node type selection ---------- */

function selectNodeType(type, fromConfig) {

    selectedNodeType = type;

    _updateNodeTypeTile();
    _updateTileStates(type);
    _updateDescriptorBadges();

    /* If currently on a view that just became disabled, switch to config */
    const tileEl = VIEW_TILES[currentView];
    if (tileEl && tileEl.classList.contains('disabled')) {

        _loadView('config');

    } else if (!fromConfig && _loadedView === 'config' && _iframeReady) {

        /* Config view is already showing — send updated node type */
        elIframe.contentWindow.postMessage({
            type: 'setNodeType',
            nodeType: selectedNodeType
        }, '*');

    }

    _saveState();

}

/* ---------- View selection ---------- */

function selectView(view) {

    const tileEl = VIEW_TILES[view];
    if (tileEl && tileEl.classList.contains('disabled')) { return; }

    _loadView(view);
    _saveState();

}

/* ---------- View loading ---------- */

function _loadView(view) {

    currentView = view;

    /* Update tile highlights across all sections */
    Object.keys(VIEW_TILES).forEach(function (key) {

        VIEW_TILES[key].classList.toggle('selected', key === view);

    });

    /* Hide the empty placeholder, show the iframe */
    elEmpty.style.display  = 'none';
    elIframe.style.display = 'block';

    /* Determine the actual URL for this view */
    const newUrl = VIEW_URLS[view];

    /* If the same URL is loaded, just send new init messages */
    if (_loadedView && VIEW_URLS[_loadedView] === newUrl && _iframeReady) {

        _loadedView = view;
        _sendInitMessages(view);
        return;

    }

    /* Different page — load new src */
    _iframeReady = false;
    _loadedView  = view;

    /* Queue the init messages for when the iframe signals ready */
    _pendingMsgs = _buildInitMessages(view);

    elIframe.src = newUrl;

}

/* ---------- Init messages per view ---------- */

function _buildInitMessages(view) {

    const msgs = [];

    if (view === 'config') {

        msgs.push({ type: 'setNodeType', nodeType: selectedNodeType });

        /* Forward any CDI/FDI XML the user has edited */
        if (cdiUserXml) {
            msgs.push({ type: 'setCdiBytes', xml: cdiUserXml, preserveWhitespace: preserveWhitespace });
        }
        if (fdiUserXml) {
            msgs.push({ type: 'setFdiBytes', xml: fdiUserXml });
        }

        /* Forward preserve-whitespace setting */
        msgs.push({ type: 'setPreserveWhitespace', value: preserveWhitespace });

        /* Restore form state if we have it */
        if (configFormState) {
            msgs.push({ type: 'restoreFormState', formState: configFormState });
        }

        /* Forward driver/callback/platform state for main.c wiring */
        msgs.push({
            type: 'setDriverCallbackState',
            driverState:   driverState,
            callbackState: callbackState,
            platformState: platformState
        });

        /* Forward Arduino mode so preview shows main.ino vs main.c */
        msgs.push({ type: 'setArduinoMode', arduino: _isArduino() });

    } else if (view === 'cdi') {

        /* Send config memory size BEFORE loadXml so the map has it when rendering */
        msgs.push({ type: 'setConfigMemSize', value: configMemHighest });

        /* Restore preserve-whitespace checkbox state */
        msgs.push({ type: 'setPreserveWhitespace', value: preserveWhitespace });

        /* Only send loadXml if we have user XML to restore */
        if (cdiUserXml) {
            msgs.push({ type: 'loadXml', xml: cdiUserXml, filename: cdiFilename });
        }

    } else if (view === 'fdi') {

        if (fdiUserXml) {
            msgs.push({ type: 'loadXml', xml: fdiUserXml, filename: fdiFilename });
        }

    } else if (view === 'platform-drivers') {

        if (platformState) {
            msgs.push({ type: 'restoreState', state: platformState });
        }

    } else if (view === 'callbacks') {

        msgs.push({
            type: 'restoreState',
            state: callbackState,
            nodeType: selectedNodeType,
            addons: _getAddons()
        });

    } else if (view === 'file-preview') {

        msgs.push({
            type: 'setWizardState',
            state: _buildWizardState()
        });

    }

    return msgs;

}

function _sendInitMessages(view) {

    if (!_iframeReady) { return; }

    const msgs = _buildInitMessages(view);
    msgs.forEach(msg => {
        elIframe.contentWindow.postMessage(msg, '*');
    });

}

/* ---------- Build full wizard state for file preview / ZIP ---------- */

function _buildWizardState() {

    return {
        _format:          'node_wizard_project',
        _version:         1,
        selectedNodeType: selectedNodeType,
        currentView:      currentView,
        cdiUserXml:       cdiUserXml,
        fdiUserXml:       fdiUserXml,
        cdiFilename:      cdiFilename,
        fdiFilename:      fdiFilename,
        preserveWhitespace: preserveWhitespace,
        configMemHighest: configMemHighest,
        configFormState:  configFormState,
        driverState:      driverState,
        callbackState:    callbackState,
        platformState:    platformState,
        arduino:          _isArduino(),
        filePreviewSelection: filePreviewSelection
    };

}

/* ---------- Listen for messages from iframes ---------- */

window.addEventListener('message', function (e) {

    if (!e.data || !e.data.type) { return; }

    switch (e.data.type) {

        case 'ready':

            _iframeReady = true;

            /* Send the queued init messages */
            if (_pendingMsgs.length > 0) {

                _pendingMsgs.forEach(msg => {
                    elIframe.contentWindow.postMessage(msg, '*');
                });
                _pendingMsgs = [];

            }

            break;

        case 'stateChanged':

            /* Track config memory size and full form state from node_config */
            if (e.data.state) {
                if (e.data.state.configMemHighest) {
                    configMemHighest = parseInt(e.data.state.configMemHighest) || 0x200;
                }
                configFormState = e.data.state;

                /* Re-gate tiles when feature flags change */
                if (selectedNodeType) {
                    _updateTileStates(selectedNodeType);

                    /* If current view just became disabled, switch to config */
                    var tileEl = VIEW_TILES[currentView];
                    if (tileEl && tileEl.classList.contains('disabled')) {
                        _loadView('config');
                    }
                }

                _saveState();
            }
            break;

        case 'xmlChanged':

            /* CDI or FDI editor reports XML changes */
            if (currentView === 'cdi') {

                cdiUserXml  = e.data.xml;
                cdiFilename = e.data.filename || null;
                if (e.data.preserveWhitespace !== undefined) {
                    preserveWhitespace = !!e.data.preserveWhitespace;
                }
                if (!cdiUserXml) { cdiValidation = null; }

            } else if (currentView === 'fdi') {

                fdiUserXml  = e.data.xml;
                fdiFilename = e.data.filename || null;
                if (!fdiUserXml) { fdiValidation = null; }

            }
            _updateDescriptorBadges();
            _saveState();
            break;

        case 'preserveWhitespaceChanged':

            preserveWhitespace = !!e.data.value;
            _saveState();

            /* Forward to config iframe if loaded */
            if (_loadedView === 'config' && _iframeReady) {
                elIframe.contentWindow.postMessage({
                    type: 'setPreserveWhitespace',
                    value: preserveWhitespace
                }, '*');
            }
            break;

        case 'updateConfigMemSize':

            /* CDI editor requests config memory size update — apply in background */
            configMemHighest = parseInt(e.data.value) || configMemHighest;

            /* Update the saved form state so config view picks it up next load */
            if (configFormState) {
                configFormState.configMemHighest = e.data.value;
            }

            /* If config iframe is currently loaded, send the update directly */
            if (_loadedView === 'config' && _iframeReady) {
                elIframe.contentWindow.postMessage({
                    type: 'setConfigMemSize',
                    value: e.data.value
                }, '*');
            }

            _saveState();
            break;

        case 'driverStateChanged':

            /* Driver editor reports state for a specific group */
            if (e.data.group) {
                driverState[e.data.group] = e.data.state;
                _saveState();
            }
            break;

        case 'callbackStateChanged':

            /* Legacy: single-group callback state (kept for compatibility) */
            if (e.data.group) {
                callbackState[e.data.group] = e.data.state;
                _saveState();
            }
            break;

        case 'allCallbackStateChanged':

            /* Unified callbacks editor reports state for all groups */
            if (e.data.state) {
                callbackState = e.data.state;
                _saveState();
            }
            break;

        case 'nodeTypeChanged':

            /* Config editor button bar changed node type */
            if (e.data.nodeType) {
                selectNodeType(e.data.nodeType, true);
            }
            break;

        case 'platformStateChanged':

            /* Platform editor reports new selection */
            if (e.data.state) {
                platformState = e.data.state;
                _updatePlatformTile();
                _saveState();
            }
            break;

        case 'filePreviewSelection':

            /* File preview reports which file the user selected */
            if (e.data.selectedFile) {
                filePreviewSelection = e.data.selectedFile;
                _saveState();
            }
            break;

        case 'requestDownload':

            /* File preview requests ZIP generation */
            requestDownload();
            break;

        case 'switchView':

            /* Child iframe requests a view switch */
            if (e.data.view) {
                selectView(e.data.view);
            }
            break;

        case 'validationStatus':

            /* CDI or FDI editor reports validation results */
            if (e.data.view === 'cdi') {
                cdiValidation = { errors: e.data.errors || 0, warnings: e.data.warnings || 0 };
            } else if (e.data.view === 'fdi') {
                fdiValidation = { errors: e.data.errors || 0, warnings: e.data.warnings || 0 };
            }
            _updateDescriptorBadges();
            break;

    }

});

/* ---------- Generate Files ---------- */

function requestDownload() {

    if (!selectedNodeType) { return; }

    ZipExport.generateZip(_buildWizardState());

}

/* ---------- Save / Load Project ----------------------------------------- */

function saveProject() {

    var state = _buildWizardState();

    var json = JSON.stringify(state, null, 2);
    var blob = new Blob([json], { type: 'application/json' });
    var url  = URL.createObjectURL(blob);

    var a = document.createElement('a');
    a.href     = url;
    var projName = (configFormState && configFormState.projectName)
        ? configFormState.projectName.replace(/\s+/g, '_').replace(/[^a-zA-Z0-9_\-]/g, '')
        : '';
    a.download = (projName || selectedNodeType || 'node') + '_project.json';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

}

document.getElementById('file-load-project').addEventListener('change', function (e) {

    var file = e.target.files[0];
    if (!file) { return; }

    var reader = new FileReader();
    reader.onload = function (ev) {

        try {

            var state = JSON.parse(ev.target.result);

            if (state._format !== 'node_wizard_project') {
                alert('Not a valid Node Wizard project file.');
                return;
            }

            /* Apply the loaded state */
            selectedNodeType = state.selectedNodeType || null;
            currentView      = state.currentView      || null;
            cdiUserXml       = state.cdiUserXml        || null;
            fdiUserXml       = state.fdiUserXml        || null;
            cdiFilename      = state.cdiFilename       || null;
            fdiFilename      = state.fdiFilename       || null;
            configMemHighest = parseInt(state.configMemHighest) || 0x200;
            configFormState  = state.configFormState    || null;
            driverState      = state.driverState       || {};
            callbackState    = state.callbackState     || {};
            platformState    = state.platformState     || null;
            filePreviewSelection = state.filePreviewSelection || null;
            preserveWhitespace   = !!state.preserveWhitespace;
            cdiValidation    = null;
            fdiValidation    = null;

            /* Migration: old arduinoMode flag */
            if (state.arduinoMode && platformState && !platformState.isArduino) {
                platformState.isArduino = true;
            }

            /* Persist to localStorage so refresh works */
            _saveState();

            /* Reload UI */
            if (selectedNodeType) {

                _updateNodeTypeTile();
                _updateTileStates(selectedNodeType);
                _updateDescriptorBadges();

                /* CDI/FDI filenames shown on sidebar tiles */
                _updatePlatformTile();

                var viewToLoad = currentView || 'config';
                currentView = null;
                _loadView(viewToLoad);

            } else {

                location.reload();

            }

        } catch (err) {

            alert('Error reading project file: ' + err.message);

        }

    };
    reader.readAsText(file);
    e.target.value = '';

});

/* ---------- Init: restore state and apply sidebar ----------------------- */

_restoreState();

if (selectedNodeType) {

    _updateNodeTypeTile();
    _updateTileStates(selectedNodeType);
    _updateDescriptorBadges();

    /* CDI/FDI filenames shown on sidebar tiles */
    _updatePlatformTile();

    /* Load the last active view */
    const viewToLoad = currentView || 'config';
    currentView = null;   /* force _loadView to actually load */
    _loadView(viewToLoad);

} else {

    /* No node type yet — load config view so user can pick one from the button bar */
    _loadView('config');

}
