/* =========================================================================
 * Node Wizard — app.js (sidebar shell controller)
 * Manages sidebar navigation, iframe switching, postMessage relay,
 * and persistence of all wizard state to localStorage.
 * ========================================================================= */

const STORAGE_KEY = 'node_wizard_state';

/* Set to true to show the Platform section in the sidebar */
const ENABLE_PLATFORM = false;

/* ---------- State ---------- */

let selectedNodeType  = null;
let currentView       = null;    /* 'config' | 'cdi' | 'fdi' | 'can-drivers' | 'olcb-drivers' | 'cb-events' | ... */
let cdiUserXml        = null;    /* XML text from CDI editor */
let fdiUserXml        = null;    /* XML text from FDI editor */
let cdiFilename       = null;    /* loaded CDI filename */
let fdiFilename       = null;    /* loaded FDI filename */
let configMemHighest  = 0x200;   /* config memory size — tracked from node_config */
let configFormState   = null;    /* last-known form state from node_config */
let driverState       = {};      /* per-group state from drivers iframe, keyed by group */
let callbackState     = {};      /* per-group state from callbacks iframe, keyed by group */
let platformState     = null;    /* { platform, params, framework, libraries, notes } */
let arduinoMode       = false;   /* true → ZIP uses main.ino instead of main.c */
let cdiValidation     = null;    /* { errors: N, warnings: N } from CDI editor */
let fdiValidation     = null;    /* { errors: N, warnings: N } from FDI editor (future) */

/* ---------- DOM refs ---------- */

const elIframe        = document.getElementById('workspace-iframe');
const elEmpty         = document.getElementById('workspace-empty');
const elBtnGenerate   = document.getElementById('btn-generate');
const elTileCdi       = document.getElementById('tile-cdi');
const elTileFdi       = document.getElementById('tile-fdi');

/* Driver tiles */
const elTileCanDrivers  = document.getElementById('tile-can-drivers');
const elTileOlcbDrivers = document.getElementById('tile-olcb-drivers');

/* Callback tiles */
const elTileCbEvents    = document.getElementById('tile-cb-events');
const elTileCbConfigMem = document.getElementById('tile-cb-config-mem');
const elTileCbTrain     = document.getElementById('tile-cb-train');
const elTileCbBcastTime = document.getElementById('tile-cb-bcast-time');

/* Platform tile */
const elTilePlatform    = document.getElementById('tile-platform');

/* Warning badges */
const elBadgeCdi        = document.getElementById('badge-cdi');
const elBadgeFdi        = document.getElementById('badge-fdi');

/* All selectable view tiles (for highlight management) */
const elTileCbCan       = document.getElementById('tile-cb-can');
const elTileCbOlcb      = document.getElementById('tile-cb-olcb');

const VIEW_TILES = {
    'platform':     elTilePlatform,
    'cdi':          elTileCdi,
    'fdi':          elTileFdi,
    'can-drivers':  elTileCanDrivers,
    'olcb-drivers': elTileOlcbDrivers,
    'cb-can':       elTileCbCan,
    'cb-olcb':      elTileCbOlcb,
    'cb-events':    elTileCbEvents,
    'cb-config-mem': elTileCbConfigMem,
    'cb-train':     elTileCbTrain,
    'cb-bcast-time': elTileCbBcastTime
};

/* ---------- Arduino checkbox ---------- */

document.getElementById('chk-arduino').addEventListener('change', function () {

    arduinoMode = this.checked;
    _saveState();

    /* Notify config iframe so the preview updates the filename */
    if (_loadedView === 'config' && _iframeReady) {
        elIframe.contentWindow.postMessage({ type: 'setArduinoMode', arduino: arduinoMode }, '*');
    }

});

/* ---------- iframe paths ---------- */

const VIEW_URLS = {
    'config':       'node_config/node_config.html',
    'platform':     'platform/platform.html',
    'cdi':          'cdi_editor/cdi_view.html',
    'fdi':          'fdi_editor/fdi_editor.html',
    'can-drivers':  'drivers/drivers.html',
    'olcb-drivers': 'drivers/drivers.html',
    'cb-can':       'callbacks/callbacks.html',
    'cb-olcb':      'callbacks/callbacks.html',
    'cb-events':    'callbacks/callbacks.html',
    'cb-config-mem': 'callbacks/callbacks.html',
    'cb-train':     'callbacks/callbacks.html',
    'cb-bcast-time': 'callbacks/callbacks.html'
};

/* Track which view is currently loaded in the iframe */
let _loadedView   = null;
let _iframeReady  = false;
let _pendingMsgs  = [];   /* messages to send once iframe is ready */

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
            arduinoMode:      arduinoMode
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
        if (state.arduinoMode) {
            arduinoMode = state.arduinoMode;
            document.getElementById('chk-arduino').checked = true;
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

/* ---------- Update platform tile description ---------- */

function _updatePlatformTile() {

    var descEl = elTilePlatform.querySelector('.tile-desc');
    if (!descEl) { return; }

    if (platformState && platformState.platform && platformState.platform !== 'none') {

        /* Show a short name from the state — we don't load PLATFORM_DEFS here */
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

function _updateDescriptorBadges() {

    if (!selectedNodeType) {

        elBadgeCdi.classList.remove('visible');
        elBadgeCdi.classList.remove('badge-error');
        elBadgeFdi.classList.remove('visible');
        elBadgeFdi.classList.remove('badge-error');
        return;

    }

    /* CDI is needed for Typical, Train, and Train Controller */
    var needsCdi = selectedNodeType !== 'basic';

    if (needsCdi && !cdiUserXml) {

        /* Not loaded — amber badge */
        elBadgeCdi.classList.add('visible');
        elBadgeCdi.classList.remove('badge-error');
        elBadgeCdi.title = 'CDI not loaded — required for this node type';

    } else if (needsCdi && cdiValidation && cdiValidation.errors > 0) {

        /* Loaded but has validation errors — red badge */
        elBadgeCdi.classList.add('visible');
        elBadgeCdi.classList.add('badge-error');
        elBadgeCdi.title = cdiValidation.errors + ' validation error' + (cdiValidation.errors > 1 ? 's' : '');

    } else if (needsCdi && cdiValidation && cdiValidation.warnings > 0) {

        /* Loaded with warnings only — amber badge */
        elBadgeCdi.classList.add('visible');
        elBadgeCdi.classList.remove('badge-error');
        elBadgeCdi.title = cdiValidation.warnings + ' validation warning' + (cdiValidation.warnings > 1 ? 's' : '');

    } else {

        /* All good or not needed */
        elBadgeCdi.classList.remove('visible');
        elBadgeCdi.classList.remove('badge-error');

    }

    /* FDI is needed only for Train (locomotive) */
    var needsFdi = selectedNodeType === 'train';

    if (needsFdi && !fdiUserXml) {

        elBadgeFdi.classList.add('visible');
        elBadgeFdi.classList.remove('badge-error');
        elBadgeFdi.title = 'FDI not loaded — required for Train nodes';

    } else if (needsFdi && fdiValidation && fdiValidation.errors > 0) {

        elBadgeFdi.classList.add('visible');
        elBadgeFdi.classList.add('badge-error');
        elBadgeFdi.title = fdiValidation.errors + ' validation error' + (fdiValidation.errors > 1 ? 's' : '');

    } else if (needsFdi && fdiValidation && fdiValidation.warnings > 0) {

        elBadgeFdi.classList.add('visible');
        elBadgeFdi.classList.remove('badge-error');
        elBadgeFdi.title = fdiValidation.warnings + ' validation warning' + (fdiValidation.warnings > 1 ? 's' : '');

    } else {

        elBadgeFdi.classList.remove('visible');
        elBadgeFdi.classList.remove('badge-error');

    }

}

/* ---------- Enable/disable tiles based on node type ---------------------- */

function _updateTileStates(type) {

    var hasCfgMem    = type !== 'basic';
    var isTrainRole  = type === 'train' || type === 'train-controller';
    var hasBcastTime = configFormState && configFormState.broadcast && configFormState.broadcast !== 'none';
    var hasEvents    = type !== 'basic';

    /* Descriptors */
    elTileCdi.classList.toggle('disabled', type === 'basic');
    elTileFdi.classList.toggle('disabled', type !== 'train');   /* FDI only for locomotive nodes */

    /* Platform — enabled when a node type is selected and feature is on */
    if (ENABLE_PLATFORM) { elTilePlatform.classList.remove('disabled'); }

    /* Drivers — always enabled when a node type is selected */
    elTileCanDrivers.classList.remove('disabled');
    elTileOlcbDrivers.classList.remove('disabled');

    /* Callbacks — gated by feature flags */
    elTileCbCan.classList.remove('disabled');                                   /* all types */
    elTileCbOlcb.classList.remove('disabled');                                  /* all types */
    elTileCbEvents.classList.toggle('disabled', !hasEvents);
    elTileCbConfigMem.classList.toggle('disabled', !hasCfgMem);
    elTileCbTrain.classList.toggle('disabled', !isTrainRole);
    elTileCbBcastTime.classList.toggle('disabled', !hasBcastTime);

}

/* ---------- Node type selection ---------- */

function selectNodeType(type) {

    selectedNodeType = type;

    /* Highlight the chosen node-type tile as "selected" (blue — it's focused) */
    document.querySelectorAll('.sidebar-tile[data-type]').forEach(tile => {
        tile.classList.toggle('selected', tile.dataset.type === type);
        tile.classList.remove('chosen');
    });

    /* Clear "selected" from all view tiles (node type area is now focused) */
    Object.keys(VIEW_TILES).forEach(function (key) {
        VIEW_TILES[key].classList.remove('selected');
    });

    _updateTileStates(type);
    _updateDescriptorBadges();

    /* Enable Generate button */
    elBtnGenerate.disabled = false;

    /* If currently on a view that just became disabled, switch to config */
    const tileEl = VIEW_TILES[currentView];
    if (tileEl && tileEl.classList.contains('disabled')) {

        _loadView('config');

    } else {

        /* Load or notify the config view */
        _loadView('config');

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

    /* Downgrade node-type tiles from "selected" (blue) to "chosen" (gray)
       only when a real view tile is being activated (not the config view,
       which has no sidebar tile — node types stay blue there) */
    if (VIEW_TILES[view]) {

        document.querySelectorAll('.sidebar-tile[data-type]').forEach(tile => {

            if (tile.classList.contains('selected')) {
                tile.classList.remove('selected');
                tile.classList.add('chosen');
            }

        });

    }

    /* Hide the empty placeholder, show the iframe */
    elEmpty.style.display  = 'none';
    elIframe.style.display = 'block';

    /* Determine the actual URL for this view */
    const newUrl = VIEW_URLS[view];

    /* If the same URL is loaded (e.g., switching callback groups), just send new group */
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
            msgs.push({ type: 'setCdiBytes', xml: cdiUserXml });
        }
        if (fdiUserXml) {
            msgs.push({ type: 'setFdiBytes', xml: fdiUserXml });
        }

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
        msgs.push({ type: 'setArduinoMode', arduino: arduinoMode });

    } else if (view === 'cdi') {

        /* Send config memory size BEFORE loadXml so the map has it when rendering */
        msgs.push({ type: 'setConfigMemSize', value: configMemHighest });

        /* Only send loadXml if we have user XML to restore */
        if (cdiUserXml) {
            msgs.push({ type: 'loadXml', xml: cdiUserXml, filename: cdiFilename });
        }

    } else if (view === 'fdi') {

        if (fdiUserXml) {
            msgs.push({ type: 'loadXml', xml: fdiUserXml, filename: fdiFilename });
        }

    } else if (view === 'platform') {

        if (platformState) {
            msgs.push({ type: 'restoreState', state: platformState });
        }

    } else if (view === 'can-drivers' || view === 'olcb-drivers') {

        var firmwareOn = configFormState && configFormState.firmware;
        msgs.push({ type: 'setNodeType', nodeType: selectedNodeType, firmwareEnabled: !!firmwareOn });
        msgs.push({ type: 'setGroup', group: view });

        /* Forward platform state so driver codegen can use templates */
        if (platformState) {
            msgs.push({ type: 'setPlatform', state: platformState });
        }

        if (driverState[view]) {
            msgs.push({ type: 'restoreState', state: driverState[view] });
        }

    } else if (view.startsWith('cb-')) {

        msgs.push({ type: 'setGroup', group: view, nodeType: selectedNodeType });

        if (callbackState[view]) {
            msgs.push({ type: 'restoreState', state: callbackState[view] });
        }

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
                if (!cdiUserXml) { cdiValidation = null; }
                _updateTileFilename(elTileCdi, cdiFilename, 'Configuration Description');

            } else if (currentView === 'fdi') {

                fdiUserXml  = e.data.xml;
                fdiFilename = e.data.filename || null;
                if (!fdiUserXml) { fdiValidation = null; }
                _updateTileFilename(elTileFdi, fdiFilename, 'Function Description');

            }
            _updateDescriptorBadges();
            _saveState();
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

            /* Callback editor reports state for a specific group */
            if (e.data.group) {
                callbackState[e.data.group] = e.data.state;
                _saveState();
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

    /* Gather the full wizard state and generate a ZIP with all project files */
    var wizardState = {
        _format:          'node_wizard_project',
        _version:         1,
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
        arduino:          arduinoMode
    };

    ZipExport.generateZip(wizardState);

}

/* ---------- Save / Load Project ----------------------------------------- */

function saveProject() {

    /* Gather the same state blob that localStorage uses */
    var state = {
        _format:          'node_wizard_project',
        _version:         1,
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
        arduinoMode:      arduinoMode
    };

    var json = JSON.stringify(state, null, 2);
    var blob = new Blob([json], { type: 'application/json' });
    var url  = URL.createObjectURL(blob);

    var a = document.createElement('a');
    a.href     = url;
    a.download = (selectedNodeType || 'node') + '_project.json';
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
            arduinoMode      = !!state.arduinoMode;
            document.getElementById('chk-arduino').checked = arduinoMode;
            cdiValidation    = null;
            fdiValidation    = null;

            /* Persist to localStorage so refresh works */
            _saveState();

            /* Reload UI */
            if (selectedNodeType) {

                document.querySelectorAll('.sidebar-tile[data-type]').forEach(function (tile) {
                    tile.classList.toggle('selected', tile.dataset.type === selectedNodeType);
                    tile.classList.remove('chosen');
                });

                _updateTileStates(selectedNodeType);
                _updateDescriptorBadges();
                elBtnGenerate.disabled = false;

                if (cdiFilename) { _updateTileFilename(elTileCdi, cdiFilename, 'Configuration Description'); }
                if (fdiFilename) { _updateTileFilename(elTileFdi, fdiFilename, 'Function Description'); }
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

/* Hide the Platform sidebar section if disabled */
if (!ENABLE_PLATFORM) {

    var platSection = elTilePlatform.closest('.sidebar-section');
    if (platSection) { platSection.style.display = 'none'; }

}

_restoreState();

if (selectedNodeType) {

    /* Restore sidebar tile selection — blue if config view (no competing tile),
       gray if a view tile will be focused */
    var viewToRestore = currentView || 'config';
    var useChosen = !!VIEW_TILES[viewToRestore];
    document.querySelectorAll('.sidebar-tile[data-type]').forEach(tile => {
        var isMatch = tile.dataset.type === selectedNodeType;
        tile.classList.toggle('chosen', isMatch && useChosen);
        tile.classList.toggle('selected', isMatch && !useChosen);
    });

    _updateTileStates(selectedNodeType);
    _updateDescriptorBadges();
    elBtnGenerate.disabled = false;

    /* Restore filenames on sidebar tiles */
    if (cdiFilename) { _updateTileFilename(elTileCdi, cdiFilename, 'Configuration Description'); }
    if (fdiFilename) { _updateTileFilename(elTileFdi, fdiFilename, 'Function Description'); }
    _updatePlatformTile();

    /* Load the last active view */
    const viewToLoad = currentView || 'config';
    currentView = null;   /* force _loadView to actually load */
    _loadView(viewToLoad);

}
