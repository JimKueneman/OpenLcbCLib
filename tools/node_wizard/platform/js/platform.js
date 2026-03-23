/* =========================================================================
 * Platform Editor — UI logic
 * Card-style platform selector with images, dynamic parameter form.
 * Communicates with parent (app.js) via postMessage.
 * ========================================================================= */

let _selectedPlatform = 'none';
let _paramValues      = {};     /* { paramId: value } */
let _nodeType         = null;
let _firmwareEnabled  = false;

/* ---------- DOM refs ---------- */

const elCards     = document.getElementById('platform-cards');
const elConfig    = document.getElementById('config-area');

/* ========================================================================= */
/* Platform cards                                                            */
/* ========================================================================= */

function _renderCards() {

    elCards.innerHTML = '';

    /* "Under Construction" notice for upcoming platform support */
    var notice = document.createElement('div');
    notice.className = 'info-box notes';
    notice.textContent = 'Additional target platforms are under construction.';
    elCards.appendChild(notice);

    Object.keys(PLATFORM_DEFS).forEach(function (key) {

        var def = PLATFORM_DEFS[key];

        if (def.disabled) { return; }

        var card = document.createElement('div');
        card.className = 'platform-card' + (key === _selectedPlatform ? ' selected' : '');
        card.dataset.platform = key;

        if (def.image) {

            var img = document.createElement('img');
            img.className = 'platform-card-image';
            img.src = def.image;
            img.alt = def.name;
            card.appendChild(img);

        }

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

    /* Arduino indicator */
    if (def.isArduino) {

        var arduinoBox = document.createElement('div');
        arduinoBox.className = 'info-box arduino';
        arduinoBox.innerHTML = '<strong>Arduino mode:</strong> main file will be generated as <code>.ino</code>, driver files as <code>.cpp</code>';
        elConfig.appendChild(arduinoBox);

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
                _postStateToParent();

            });

            input.addEventListener('change', function () {

                _paramValues[p.id] = input.value;
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
/* State communication                                                       */
/* ========================================================================= */

function _postStateToParent() {

    var def = PLATFORM_DEFS[_selectedPlatform];

    window.parent.postMessage({
        type: 'platformStateChanged',
        state: {
            platform:   _selectedPlatform,
            params:     _paramValues,
            isArduino:  !!def.isArduino,
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

window.parent.postMessage({ type: 'ready' }, '*');
