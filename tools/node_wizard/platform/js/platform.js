/* =========================================================================
 * Platform Editor — UI logic
 * Card-style platform selector, dynamic parameter form, live code preview.
 * Communicates with parent (app.js) via postMessage.
 * ========================================================================= */

let _selectedPlatform = 'none';
let _paramValues      = {};     /* { paramId: value } */
let _currentTab       = 'can';  /* 'can' | 'olcb' */
let _nodeType         = null;
let _firmwareEnabled  = false;

/* ---------- DOM refs ---------- */

const elCards     = document.getElementById('platform-cards');
const elConfig    = document.getElementById('config-area');
const elFnameEl   = document.getElementById('preview-filename');
const elBadge     = document.getElementById('preview-badge');

let _cmViewer = null;

function _ensureCMViewer() {

    if (_cmViewer) { return _cmViewer; }
    _cmViewer = createCMReadonly(document.getElementById('code-preview'));
    return _cmViewer;

}
const elTabCan    = document.getElementById('tab-can');
const elTabOlcb   = document.getElementById('tab-olcb');

/* ========================================================================= */
/* Platform cards                                                            */
/* ========================================================================= */

function _renderCards() {

    elCards.innerHTML = '';

    Object.keys(PLATFORM_DEFS).forEach(function (key) {

        var def = PLATFORM_DEFS[key];

        var card = document.createElement('div');
        card.className = 'platform-card' + (key === _selectedPlatform ? ' selected' : '');
        card.dataset.platform = key;

        var nameEl = document.createElement('span');
        nameEl.className   = 'platform-card-name';
        nameEl.textContent = def.name;

        var descEl = document.createElement('span');
        descEl.className   = 'platform-card-desc';
        descEl.textContent = def.description;

        card.appendChild(nameEl);
        card.appendChild(descEl);

        card.addEventListener('click', function () {

            _selectPlatform(key);

        });

        elCards.appendChild(card);

    });

}

function _selectPlatform(key) {

    _selectedPlatform = key;

    /* Initialize default param values for this platform */
    var def = PLATFORM_DEFS[key];
    _paramValues = {};

    def.parameters.forEach(function (p) {

        _paramValues[p.id] = p.defaultValue;

    });

    _renderCards();
    _renderConfig();
    _renderPreview();
    _postStateToParent();

}

/* ========================================================================= */
/* Configuration form                                                        */
/* ========================================================================= */

function _renderConfig() {

    elConfig.innerHTML = '';

    var def = PLATFORM_DEFS[_selectedPlatform];

    /* Framework info */
    if (def.framework) {

        var fwBox = document.createElement('div');
        fwBox.className = 'info-box framework';
        fwBox.innerHTML = '<strong>Framework:</strong> ' + _escHtml(def.framework);
        elConfig.appendChild(fwBox);

    }

    /* Library requirements */
    if (def.libraries && def.libraries.length > 0) {

        var libBox = document.createElement('div');
        libBox.className = 'info-box libraries';
        libBox.innerHTML = '<strong>Required libraries:</strong> ' + def.libraries.map(_escHtml).join(', ');
        elConfig.appendChild(libBox);

    }

    /* Notes */
    if (def.notes && def.notes.length > 0) {

        var noteBox = document.createElement('div');
        noteBox.className = 'info-box notes';
        noteBox.innerHTML = def.notes.map(function (n) { return _escHtml(n); }).join('<br>');
        elConfig.appendChild(noteBox);

    }

    /* Parameters */
    if (def.parameters.length > 0) {

        var section = document.createElement('div');
        section.className = 'config-section';

        var title = document.createElement('div');
        title.className   = 'config-section-title';
        title.textContent = 'Pin / Hardware Configuration';
        section.appendChild(title);

        def.parameters.forEach(function (p) {

            var row = document.createElement('div');
            row.className = 'config-row';

            var lbl = document.createElement('label');
            lbl.textContent = p.label;
            row.appendChild(lbl);

            var input;

            if (p.type === 'select' && p.options) {

                input = document.createElement('select');
                p.options.forEach(function (opt) {

                    var optEl = document.createElement('option');
                    optEl.value       = opt.value;
                    optEl.textContent = opt.label;
                    if (opt.value === _paramValues[p.id]) { optEl.selected = true; }
                    input.appendChild(optEl);

                });

            } else {

                input = document.createElement('input');
                input.type  = p.type || 'text';
                input.value = _paramValues[p.id] || '';

            }

            input.dataset.paramId = p.id;

            input.addEventListener('input', function () {

                _paramValues[p.id] = input.value;
                _renderPreview();
                _postStateToParent();

            });

            input.addEventListener('change', function () {

                _paramValues[p.id] = input.value;
                _renderPreview();
                _postStateToParent();

            });

            row.appendChild(input);
            section.appendChild(row);

        });

        elConfig.appendChild(section);

    }

    /* No-config message for "None" */
    if (_selectedPlatform === 'none') {

        var msg = document.createElement('div');
        msg.className = 'info-box notes';
        msg.textContent = 'No platform selected. Driver files will contain empty TODO stub functions.';
        elConfig.appendChild(msg);

    }

}

/* ========================================================================= */
/* Template substitution                                                     */
/* ========================================================================= */

function _substituteParams(text) {

    if (!text) { return text; }

    return text.replace(/\{\{(\w+)\}\}/g, function (match, paramId) {

        return (_paramValues[paramId] !== undefined) ? _paramValues[paramId] : match;

    });

}

/* ========================================================================= */
/* Code preview                                                              */
/* ========================================================================= */

function switchTab(tab) {

    _currentTab = tab;
    elTabCan.classList.toggle('active', tab === 'can');
    elTabOlcb.classList.toggle('active', tab === 'olcb');
    _renderPreview();

}

function _renderPreview() {

    var viewer = _ensureCMViewer();
    var def = PLATFORM_DEFS[_selectedPlatform];

    if (_selectedPlatform === 'none') {

        viewer.value = '// Select a platform to preview the generated driver code';
        elBadge.style.display = 'none';
        elFnameEl.textContent = '';
        return;

    }

    elBadge.textContent   = def.name;
    elBadge.style.display = 'inline-block';

    var drivers;
    var filePrefix;

    if (_currentTab === 'can') {

        drivers    = def.canDrivers;
        filePrefix = 'openlcb_can_drivers';
        elFnameEl.textContent = filePrefix + '.c';

    } else {

        drivers    = def.olcbDrivers;
        filePrefix = 'openlcb_drivers';
        elFnameEl.textContent = filePrefix + '.c';

    }

    /* Build preview code */
    var lines = [];

    lines.push('');
    lines.push('#include "' + filePrefix + '.h"');

    /* Extra includes */
    if (drivers.extraIncludes && drivers.extraIncludes.length > 0) {

        drivers.extraIncludes.forEach(function (inc) {

            lines.push(_substituteParams(inc));

        });

    }

    /* Globals */
    if (drivers.globals) {

        lines.push(_substituteParams(drivers.globals));

    }

    lines.push('');
    lines.push('');

    /* Show each template function */
    var templateKeys = Object.keys(drivers.templates);

    if (templateKeys.length === 0) {

        lines.push('// No platform-specific code — functions will be empty TODO stubs.');

    } else {

        /* Determine which driver group to use for function signatures */
        var groupKey = (_currentTab === 'can') ? 'can-drivers' : 'olcb-drivers';

        /* We don't have DRIVER_GROUPS loaded here, so just show the template bodies */
        templateKeys.forEach(function (fnName) {

            var body = _substituteParams(drivers.templates[fnName]);

            var prefix = (_currentTab === 'can') ? 'CanDriver' : 'Drivers';
            lines.push('// ' + prefix + '_' + fnName + '()');
            lines.push('// ────────────────────────────');
            lines.push(body);
            lines.push('');

        });

    }

    viewer.value = lines.join('\n');

}

/* ========================================================================= */
/* State communication                                                       */
/* ========================================================================= */

function _postStateToParent() {

    var def = PLATFORM_DEFS[_selectedPlatform];

    window.parent.postMessage({
        type: 'platformStateChanged',
        state: {
            platform:   _selectedPlatform,
            params:     _paramValues,
            framework:  def.framework || '',
            libraries:  def.libraries || [],
            notes:      def.notes || []
        }
    }, '*');

}

function _restoreState(state) {

    if (!state) { return; }

    if (state.platform && PLATFORM_DEFS[state.platform]) {

        _selectedPlatform = state.platform;

    }

    if (state.params) {

        /* Merge saved params with defaults so new params get their defaults */
        var def = PLATFORM_DEFS[_selectedPlatform];
        _paramValues = {};

        def.parameters.forEach(function (p) {

            _paramValues[p.id] = (state.params[p.id] !== undefined) ? state.params[p.id] : p.defaultValue;

        });

    }

    _renderCards();
    _renderConfig();
    _renderPreview();

}

/* ========================================================================= */
/* Message listener                                                          */
/* ========================================================================= */

window.addEventListener('message', function (e) {

    if (!e.data || !e.data.type) { return; }

    switch (e.data.type) {

        case 'setNodeType':

            _nodeType        = e.data.nodeType || null;
            _firmwareEnabled = !!e.data.firmwareEnabled;
            break;

        case 'restoreState':

            _restoreState(e.data.state);
            break;

    }

});

/* ========================================================================= */
/* Helpers                                                                   */
/* ========================================================================= */

function _escHtml(str) {

    var div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;

}

/* ========================================================================= */
/* Init                                                                      */
/* ========================================================================= */

_renderCards();
_renderConfig();
_renderPreview();

window.parent.postMessage({ type: 'ready' }, '*');
