/* ============================================================================
 * utils.js — Shared helper functions
 * ============================================================================ */

function getVal(id) {
    return document.getElementById(id).value || '0';
}

function getStrVal(id) {
    return document.getElementById(id).value;
}

function boolStr(checkboxId) {
    return document.getElementById(checkboxId).checked ? 'true' : 'false';
}

function isChecked(id) {
    var el = document.getElementById(id);
    return el ? el.checked : false;
}

function setCheckbox(id, value) {
    var el = document.getElementById(id);
    if (el) el.checked = value;
}

function showHide(elementId, show) {
    var el = document.getElementById(elementId);
    if (!el) return;
    if (show) {
        el.classList.remove('hidden');
    } else {
        el.classList.add('hidden');
    }
}

function parseIntAuto(s) {
    s = (s + '').trim();
    if (s.toLowerCase().indexOf('0x') === 0) {
        return parseInt(s, 16);
    }
    return parseInt(s, 10) || 0;
}

function escapeC(s) {
    return s.replace(/\\/g, '\\\\').replace(/"/g, '\\"');
}

function escapeXml(s) {
    return s.replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;');
}

function byteHex(b) {
    return ('0' + b.toString(16).toUpperCase()).slice(-2);
}

function downloadTextFile(filename, content) {
    var blob = new Blob([content], { type: 'text/plain' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

function setFieldValue(fieldId, value, readOnly) {
    var el = document.getElementById(fieldId);
    if (!el) return;
    el.value = value;
    if (readOnly) {
        el.setAttribute('readonly', '');
    } else {
        el.removeAttribute('readonly');
    }
}

function getXmlText(parentEl, tagName) {
    var el = parentEl.querySelector(tagName);
    if (el) return el.textContent;
    return null;
}

function formatHexDec(n) {
    return '0x' + n.toString(16).toUpperCase() + ' (' + n + ')';
}

function initFileDropZone(zoneId, fileInputId, loadFn) {
    var zone = document.getElementById(zoneId);
    if (!zone) return;

    zone.addEventListener('click', function() {
        document.getElementById(fileInputId).click();
    });

    zone.addEventListener('dragover', function(e) {
        e.preventDefault();
        zone.classList.add('drag-over');
    });

    zone.addEventListener('dragleave', function() {
        zone.classList.remove('drag-over');
    });

    zone.addEventListener('drop', function(e) {
        e.preventDefault();
        zone.classList.remove('drag-over');
        var file = e.dataTransfer.files[0];
        if (file) loadFn(file);
    });
}

function addDefineIfChecked(lines, checkboxId, macroName) {
    if (isChecked(checkboxId)) {
        lines.push('#define ' + macroName);
    } else {
        lines.push('/* #define ' + macroName + ' */');
    }
}
