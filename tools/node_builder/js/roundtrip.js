/* ============================================================================
 * roundtrip.js — Load openlcb_user_config.h/.c and populate the form
 *
 * The file picker accepts multiple files.  Select both the .h and .c to get
 * a full round-trip.  The .h supplies #define macros (feature flags, buffer
 * sizes, events, memory config, train protocol).  The .c supplies the
 * OpenLcbUserConfig_node_parameters struct (SNIP, address spaces, config options).
 * Either file can be loaded on its own for a partial import.
 * ============================================================================ */

function initRoundtrip() {
    document.getElementById('btn-load').addEventListener('click', function() {
        document.getElementById('file-load-config').click();
    });

    document.getElementById('file-load-config').addEventListener('change', function(e) {
        var files = e.target.files;
        if (!files || files.length === 0) return;

        var pending = files.length;
        var hText = '';
        var cText = '';

        for (var i = 0; i < files.length; i++) {
            (function(file) {
                var reader = new FileReader();
                reader.onload = function(ev) {
                    var content = ev.target.result;
                    if (file.name.match(/\.c$/i)) {
                        cText = content;
                    } else {
                        hText = content;
                    }
                    pending--;
                    if (pending === 0) {
                        applyLoadedConfig(hText, cText);
                    }
                };
                reader.readAsText(file);
            })(files[i]);
        }

        // Reset so the same files can be re-selected
        e.target.value = '';
    });
}

function applyLoadedConfig(hText, cText) {
    var loaded = [];

    if (hText) {
        parseUserConfigH(hText);
        loaded.push('.h');
    }
    if (cText) {
        parseUserConfigC(cText);
        loaded.push('.c');
    }

    // Cascade UI updates
    onFeatureFlagChange();
    onAddrSpaceChange();
    onConfigSpaceChange();

    document.getElementById('status-bar').className = 'info-box success';
    document.getElementById('status-bar').innerHTML =
        '<strong>Loaded</strong> — openlcb_user_config ' + loaded.join(' + ') +
        ' parsed successfully. Modify and re-generate.';

    // Persist loaded state
    if (typeof saveState === 'function') saveState();
}

/* ── Parse openlcb_user_config.h ── */

function parseUserConfigH(text) {

    // Feature flags
    setCheckbox('flag-events',         hasDefine(text, 'OPENLCB_COMPILE_EVENTS'));
    setCheckbox('flag-datagrams',      hasDefine(text, 'OPENLCB_COMPILE_DATAGRAMS'));
    setCheckbox('flag-config-mem',     hasDefine(text, 'OPENLCB_COMPILE_MEMORY_CONFIGURATION'));
    setCheckbox('flag-firmware',       hasDefine(text, 'OPENLCB_COMPILE_FIRMWARE'));
    setCheckbox('flag-broadcast-time', hasDefine(text, 'OPENLCB_COMPILE_BROADCAST_TIME'));
    setCheckbox('flag-train',          hasDefine(text, 'OPENLCB_COMPILE_TRAIN'));
    setCheckbox('flag-stream',         hasDefine(text, 'OPENLCB_COMPILE_STREAM'));

    // Buffer sizing
    extractNumericDefine(text, 'USER_DEFINED_BASIC_BUFFER_DEPTH',    'buf-basic');
    extractNumericDefine(text, 'USER_DEFINED_DATAGRAM_BUFFER_DEPTH', 'buf-datagram');
    extractNumericDefine(text, 'USER_DEFINED_SNIP_BUFFER_DEPTH',     'buf-snip');
    extractNumericDefine(text, 'USER_DEFINED_STREAM_BUFFER_DEPTH',   'buf-stream');
    extractNumericDefine(text, 'USER_DEFINED_NODE_BUFFER_DEPTH',     'buf-nodes');

    // Events
    extractNumericDefine(text, 'USER_DEFINED_PRODUCER_COUNT',       'evt-producers');
    extractNumericDefine(text, 'USER_DEFINED_PRODUCER_RANGE_COUNT', 'evt-producer-ranges');
    extractNumericDefine(text, 'USER_DEFINED_CONSUMER_COUNT',       'evt-consumers');
    extractNumericDefine(text, 'USER_DEFINED_CONSUMER_RANGE_COUNT', 'evt-consumer-ranges');

    // Memory configuration
    extractRawDefine(text, 'USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS', 'cfg-user-name-addr');

    // Train
    extractNumericDefine(text, 'USER_DEFINED_TRAIN_NODE_COUNT',        'train-node-count');
    extractNumericDefine(text, 'USER_DEFINED_MAX_LISTENERS_PER_TRAIN', 'train-listeners');
    extractNumericDefine(text, 'USER_DEFINED_MAX_TRAIN_FUNCTIONS',     'train-functions');
}

/* ── Parse openlcb_user_config.c ── */

function parseUserConfigC(text) {

    // SNIP strings — extract from .snip.field = "value"
    extractStructString(text, '.snip.name',             'snip-name');
    extractStructString(text, '.snip.model',            'snip-model');
    extractStructString(text, '.snip.hardware_version', 'snip-hw-version');
    extractStructString(text, '.snip.software_version', 'snip-sw-version');

    // Config options
    extractStructBool(text, '.configuration_options.stream_read_write_supported',  'cfgopt-stream-rw');
    extractStructBool(text, '.configuration_options.unaligned_reads_supported',    'cfgopt-unaligned-reads');
    extractStructBool(text, '.configuration_options.unaligned_writes_supported',   'cfgopt-unaligned-writes');
    extractStructString(text, '.configuration_options.description',                'cfgopt-desc');

    // Address spaces — 0xFE is the only user-editable presence/settings
    extractStructBool(text,   '.address_space_all.present',           'addr-0xfe-present');
    extractStructBool(text,   '.address_space_all.read_only',         'addr-0xfe-readonly');
    extractStructBool(text,   '.address_space_all.low_address_valid', 'addr-0xfe-low-valid');
    extractStructValue(text,  '.address_space_all.low_address',       'addr-0xfe-low');
    extractStructValue(text,  '.address_space_all.highest_address',   'addr-0xfe-high');

    // Address space descriptions
    extractStructString(text, '.address_space_configuration_definition.description', 'addr-0xff-desc');
    extractStructString(text, '.address_space_all.description',                      'addr-0xfe-desc');
    extractStructString(text, '.address_space_config_memory.description',            'addr-0xfd-desc');
    extractStructString(text, '.address_space_acdi_manufacturer.description',        'addr-0xfc-desc');
    extractStructString(text, '.address_space_acdi_user.description',                'addr-0xfb-desc');
    extractStructString(text, '.address_space_firmware.description',                 'addr-0xef-desc');
    extractStructString(text, '.address_space_train_function_definition_info.description', 'addr-0xfa-desc');
    extractStructString(text, '.address_space_train_function_config_memory.description',  'addr-0xf9-desc');

    // 0xFD highest address
    extractStructValue(text, '.address_space_config_memory.highest_address', 'cfg-highest-addr');

    // 0xF9 highest address
    extractStructValue(text, '.address_space_train_function_config_memory.highest_address', 'addr-0xf9-high');
}

/* ── Parser helpers — #define macros ── */

function hasDefine(text, name) {
    var re = new RegExp('^\\s*#define\\s+' + name + '\\b', 'm');
    return re.test(text);
}

function extractStringDefine(text, macro, fieldId) {
    var re = new RegExp('#define\\s+' + macro + '\\s+"([^"]*)"');
    var m = text.match(re);
    if (m) document.getElementById(fieldId).value = m[1];
}

function extractNumericDefine(text, macro, fieldId) {
    var re = new RegExp('#define\\s+' + macro + '\\s+(\\S+)');
    var m = text.match(re);
    if (m) document.getElementById(fieldId).value = m[1];
}

function extractRawDefine(text, macro, fieldId) {
    var re = new RegExp('#define\\s+' + macro + '\\s+(\\S+)');
    var m = text.match(re);
    if (m) document.getElementById(fieldId).value = m[1];
}

function extractBoolDefine(text, macro, checkboxId) {
    var re = new RegExp('#define\\s+' + macro + '\\s+(\\S+)');
    var m = text.match(re);
    if (m) setCheckbox(checkboxId, m[1] === 'true');
}

/* ── Parser helpers — C struct initializers ── */

function extractStructString(text, field, fieldId) {
    var escaped = field.replace(/\./g, '\\.');
    var re = new RegExp(escaped + '\\s*=\\s*"([^"]*)"');
    var m = text.match(re);
    if (m) document.getElementById(fieldId).value = m[1];
}

function extractStructBool(text, field, checkboxId) {
    var escaped = field.replace(/\./g, '\\.');
    var re = new RegExp(escaped + '\\s*=\\s*(true|false)');
    var m = text.match(re);
    if (m) setCheckbox(checkboxId, m[1] === 'true');
}

function extractStructValue(text, field, fieldId) {
    var escaped = field.replace(/\./g, '\\.');
    var re = new RegExp(escaped + '\\s*=\\s*([^,]+)');
    var m = text.match(re);
    if (m) {
        var val = m[1].trim();
        document.getElementById(fieldId).value = val;
    }
}
