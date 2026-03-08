/* ============================================================================
 * validation.js — Pre-generation validation checks
 * ============================================================================ */

function runValidation() {
    var list = document.getElementById('validation-list');
    list.innerHTML = '';

    var issues = [];
    var passes = [];

    // 1. CDI loaded? (only relevant when Memory Configuration is enabled)
    if (isChecked('flag-config-mem')) {
        if (cdiBytes && cdiBytes.length > 0) {
            passes.push('CDI loaded: ' + cdiBytes.length + ' bytes');
        } else {
            issues.push({ level: 'warn', msg: 'No CDI XML loaded — byte array will be empty.' });
        }
    }

    // 2. Memory Configuration requires datagrams
    var configMem = isChecked('flag-config-mem');
    var datagrams = isChecked('flag-datagrams');
    if (configMem && !datagrams) {
        issues.push({ level: 'error',
            msg: 'Memory Configuration Protocol requires OPENLCB_COMPILE_DATAGRAMS.' });
    }

    // 2b. Firmware requires Memory Configuration
    var firmware = isChecked('flag-firmware');
    if (firmware && !configMem) {
        issues.push({ level: 'error',
            msg: 'OPENLCB_COMPILE_FIRMWARE requires OPENLCB_COMPILE_MEMORY_CONFIGURATION.' });
    }

    // 3. Total buffer count (8-bit warning at 126)
    var totalBuf = parseInt(getVal('buf-basic'))
                 + parseInt(getVal('buf-datagram'))
                 + parseInt(getVal('buf-snip'))
                 + parseInt(getVal('buf-stream'));
    if (totalBuf > 126) {
        issues.push({ level: 'warn',
            msg: 'Total buffer count (' + totalBuf + ') exceeds 126 — may not fit on 8-bit processors.' });
    } else {
        passes.push('Buffer pool total: ' + totalBuf + ' (OK for 8-bit)');
    }

    // 4. Configuration space (0xFD) size checks
    if (configMem) {
        var nameAddr = parseIntAuto(getVal('cfg-user-name-addr'));
        var descAddr = nameAddr + CONFIG_LEN_USER_NAME;
        var minHighest = nameAddr + CONFIG_LEN_USER_NAME + CONFIG_LEN_USER_DESC - 1;
        var highAddr = parseIntAuto(getVal('cfg-highest-addr'));

        if (highAddr < minHighest) {
            issues.push({ level: 'error',
                msg: 'Configuration space highest address (' + highAddr
                   + ') must be >= ' + minHighest
                   + ' to fit user name (' + CONFIG_LEN_USER_NAME
                   + ' bytes) + description (' + CONFIG_LEN_USER_DESC + ' bytes).' });
        } else {
            passes.push('Configuration space size OK (highest addr = ' + highAddr
                       + ', minimum = ' + minHighest + ')');
        }
    }

    // 5. SNIP field lengths
    var snipName = document.getElementById('snip-name').value;
    var snipModel = document.getElementById('snip-model').value;
    if (snipName.length > 40) {
        issues.push({ level: 'error', msg: 'SNIP name exceeds 40 characters (' + snipName.length + ').' });
    }
    if (snipModel.length > 40) {
        issues.push({ level: 'error', msg: 'SNIP model exceeds 40 characters (' + snipModel.length + ').' });
    }

    // 6. Event range counts >= 1
    if (isChecked('flag-events')) {
        if (parseInt(getVal('evt-producer-ranges')) < 1) {
            issues.push({ level: 'error', msg: 'Producer range count must be >= 1.' });
        }
        if (parseInt(getVal('evt-consumer-ranges')) < 1) {
            issues.push({ level: 'error', msg: 'Consumer range count must be >= 1.' });
        }
    }

    // 7. Train enabled but no FDI
    if (isChecked('flag-train') && (!fdiBytes || fdiBytes.length === 0)) {
        issues.push({ level: 'warn', msg: 'Train protocol enabled but no FDI XML loaded.' });
    }

    // 8. CDI length vs CDI_LENGTH buffer
    if (cdiBytes && cdiBytes.length > 20000) {
        issues.push({ level: 'warn',
            msg: 'CDI is very large (' + cdiBytes.length + ' bytes). Ensure target has enough flash.' });
    }

    // 9. Train Node Count > Virtual Node Buffer Depth
    if (isChecked('flag-train')) {
        var trainCount = parseInt(getVal('train-node-count'));
        var nodeDepth  = parseInt(getVal('buf-nodes'));
        if (trainCount > nodeDepth) {
            issues.push({ level: 'warn',
                msg: 'Train Node Count (' + trainCount
                   + ') exceeds Virtual Nodes (' + nodeDepth
                   + '). Each train consumes a virtual node slot.' });
        } else {
            passes.push('Train Node Count (' + trainCount + ') fits within Virtual Nodes (' + nodeDepth + ')');
        }
    }

    // 10. Address space description length checks (max 59 chars)
    var descFields = [
        { id: 'addr-0xff-desc', label: 'CDI space (0xFF)' },
        { id: 'addr-0xfe-desc', label: 'All Memory space (0xFE)' },
        { id: 'addr-0xfd-desc', label: 'Configuration space (0xFD)' },
        { id: 'addr-0xfc-desc', label: 'ACDI Manufacturer space (0xFC)' },
        { id: 'addr-0xfb-desc', label: 'ACDI User space (0xFB)' },
        { id: 'addr-0xef-desc', label: 'Firmware space (0xEF)' },
        { id: 'addr-0xfa-desc', label: 'Train FDI space (0xFA)' },
        { id: 'addr-0xf9-desc', label: 'Train Function Config space (0xF9)' }
    ];
    for (var d = 0; d < descFields.length; d++) {
        var el = document.getElementById(descFields[d].id);
        if (el && el.value.length > 59) {
            issues.push({ level: 'error',
                msg: descFields[d].label + ' description exceeds 59 characters (' + el.value.length + ').' });
        }
    }

    // Render results
    for (var p = 0; p < passes.length; p++) {
        addValidationItem(list, 'pass', passes[p]);
    }
    for (var j = 0; j < issues.length; j++) {
        addValidationItem(list, issues[j].level, issues[j].msg);
    }

    // Update badge
    var badge = document.getElementById('badge-validation');
    if (issues.length === 0) {
        badge.textContent = 'all OK';
        badge.style.color = '#4caf7a';
    } else {
        var errors = issues.filter(function(x) { return x.level === 'error'; }).length;
        var warns  = issues.filter(function(x) { return x.level === 'warn'; }).length;
        var parts = [];
        if (errors) parts.push(errors + ' error(s)');
        if (warns)  parts.push(warns + ' warning(s)');
        badge.textContent = parts.join(', ');
        badge.style.color = errors ? '#d94a4a' : '#d4a043';
    }
}

function addValidationItem(list, level, msg) {
    var li = document.createElement('li');
    var iconClass = (level === 'pass') ? 'pass' : (level === 'error' ? 'fail' : 'warn');
    var iconChar  = (level === 'pass') ? '\u2713' : (level === 'error' ? '\u2715' : '\u26A0');
    li.innerHTML = '<span class="icon ' + iconClass + '">' + iconChar
                 + '</span><span>' + msg + '</span>';
    list.appendChild(li);
}
