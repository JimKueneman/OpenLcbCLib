/* ============================================================================
 * cdi_processing.js — Load CDI XML, generate byte array, extract SNIP fields
 * ============================================================================ */

var cdiXmlText = '';
var cdiBytes = null;
var cdiFileName = '';

function initCDI() {
    document.getElementById('file-load-cdi').addEventListener('change', function(e) {
        var file = e.target.files[0];
        if (!file) return;
        loadCdiFile(file);
    });

    // Click-to-browse and drag-drop on CDI warning boxes
    var cdiDropZones = ['snip-config-mem-note', 'cdi-needed-0xff'];
    for (var i = 0; i < cdiDropZones.length; i++) {
        initFileDropZone(cdiDropZones[i], 'file-load-cdi', loadCdiFile);
    }
}

function loadCdiFile(file) {
    cdiFileName = file.name;
    var reader = new FileReader();
    reader.onload = function(ev) {
        cdiXmlText = ev.target.result;
        processCDI();
    };
    reader.readAsText(file);
}

function processCDI() {
    if (!cdiXmlText) return;

    // UTF-8 encode + null terminator
    var encoder = new TextEncoder();
    var rawBytes = encoder.encode(cdiXmlText);
    cdiBytes = new Uint8Array(rawBytes.length + 1);
    cdiBytes.set(rawBytes);
    cdiBytes[rawBytes.length] = 0x00;

    // Update UI — compact picker in Feature Flags
    var ffName = document.getElementById('ff-cdi-file-name');
    if (ffName) { ffName.textContent = cdiFileName; ffName.classList.add('loaded'); }
    var ffBytes = document.getElementById('ff-cdi-bytes');
    if (ffBytes) { ffBytes.textContent = cdiBytes.length + ' B'; }

    // Update file-load button badge
    var loadBtns = document.querySelectorAll('#ff-cdi-picker .btn-file');
    for (var i = 0; i < loadBtns.length; i++) loadBtns[i].classList.add('loaded');

    // Show clear button
    showHide('btn-clear-cdi', true);

    // CDI info in Address Spaces 0xFF tab
    var cdiLenEl = document.getElementById('cdi-length');
    if (cdiLenEl) cdiLenEl.value = formatHexDec(cdiBytes.length);
    var cdiInfoEl = document.getElementById('cdi-info');
    if (cdiInfoEl) cdiInfoEl.style.display = 'block';

    // Space 0xFF highest address
    var highAddr = cdiBytes.length - 1;
    document.getElementById('addr-0xff-high').value = formatHexDec(highAddr);

    // Byte array preview
    var previewEl = document.getElementById('cdi-preview');
    if (previewEl) {
        previewEl.textContent = formatByteArrayPreview(cdiBytes, 200);
    }

    // Extract <identification> fields
    extractCDIIdentification();

    // Update SNIP panel state (readonly with CDI note)
    updateSnipFieldState();

    // Hide "CDI needed" warnings now that CDI is loaded
    updateCdiNeededWarnings();

    // Persist state after CDI load
    if (typeof saveState === 'function') saveState();
}

function extractCDIIdentification() {
    var parser = new DOMParser();
    var xmlDoc = parser.parseFromString(cdiXmlText, 'text/xml');

    if (xmlDoc.querySelector('parsererror')) {
        document.getElementById('status-bar').className = 'info-box warning';
        document.getElementById('status-bar').textContent =
            'CDI XML has parse errors — SNIP fields not extracted.';
        return;
    }

    var ident = xmlDoc.querySelector('identification');
    if (!ident) return;

    var mfg   = getXmlText(ident, 'manufacturer');
    var model = getXmlText(ident, 'model');
    var hwVer = getXmlText(ident, 'hardwareVersion');
    var swVer = getXmlText(ident, 'softwareVersion');

    if (mfg !== null)   setFieldValue('snip-name',       mfg,   true);
    if (model !== null)  setFieldValue('snip-model',      model, true);
    if (hwVer !== null)  setFieldValue('snip-hw-version', hwVer, true);
    if (swVer !== null)  setFieldValue('snip-sw-version', swVer, true);
}

function generateStarterCDI() {
    var name  = document.getElementById('snip-name').value || 'My Node';
    var model = document.getElementById('snip-model').value || 'My Model';
    var hwVer = document.getElementById('snip-hw-version').value || '1.0';
    var swVer = document.getElementById('snip-sw-version').value || '1.0';

    var xml =
        '<?xml version="1.0"?>\n'
      + '<?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>\n'
      + '<cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"\n'
      + '     xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/4/cdi.xsd">\n'
      + '  <identification>\n'
      + '    <manufacturer>' + escapeXml(name) + '</manufacturer>\n'
      + '    <model>' + escapeXml(model) + '</model>\n'
      + '    <hardwareVersion>' + escapeXml(hwVer) + '</hardwareVersion>\n'
      + '    <softwareVersion>' + escapeXml(swVer) + '</softwareVersion>\n'
      + '  </identification>\n'
      + '  <acdi/>\n'
      + '  <segment origin="0" space="253">\n'
      + '    <name>User Configuration</name>\n'
      + '    <group>\n'
      + '      <name>User Info</name>\n'
      + '      <description>Enter a name and description to identify this node.</description>\n'
      + '      <string size="63"><name>User Name</name></string>\n'
      + '      <string size="64"><name>User Description</name></string>\n'
      + '    </group>\n'
      + '  </segment>\n'
      + '</cdi>\n';

    downloadTextFile('node_cdi.xml', xml);
}

function clearCdi() {
    cdiXmlText = '';
    cdiBytes = null;
    cdiFileName = '';

    // Reset compact picker in Feature Flags
    var ffName = document.getElementById('ff-cdi-file-name');
    if (ffName) { ffName.textContent = 'No file loaded'; ffName.classList.remove('loaded'); }
    var ffBytes = document.getElementById('ff-cdi-bytes');
    if (ffBytes) { ffBytes.textContent = ''; }

    // Reset file-load button badge
    var loadBtns = document.querySelectorAll('#ff-cdi-picker .btn-file');
    for (var i = 0; i < loadBtns.length; i++) loadBtns[i].classList.remove('loaded');

    // Hide clear button
    showHide('btn-clear-cdi', false);

    // Reset Address Space 0xFF highest address
    document.getElementById('addr-0xff-high').value = '(from CDI)';

    // Hide CDI info section
    var cdiInfoEl = document.getElementById('cdi-info');
    if (cdiInfoEl) cdiInfoEl.style.display = 'none';
    var cdiLenEl = document.getElementById('cdi-length');
    if (cdiLenEl) cdiLenEl.value = '';
    var previewEl = document.getElementById('cdi-preview');
    if (previewEl) previewEl.textContent = '';

    // Reset SNIP fields to defaults (extractCDIIdentification set values + readonly)
    setFieldValue('snip-name',       'Basic OpenLcb Node', false);
    setFieldValue('snip-model',      'Test Application',   false);
    setFieldValue('snip-hw-version', '0.0.1',              false);
    setFieldValue('snip-sw-version', '0.0.1',              false);

    // Update SNIP panel state (may re-disable if Config Mem still on)
    updateSnipFieldState();

    // Show "CDI needed" warnings now that CDI is cleared
    updateCdiNeededWarnings();

    // Reset file input so the same file can be re-selected
    document.getElementById('file-load-cdi').value = '';

    // Persist cleared state
    if (typeof saveState === 'function') saveState();
}

/* Format helpers for byte arrays */

function formatByteArrayPreview(bytes, maxBytes) {
    var count = Math.min(bytes.length, maxBytes);
    var parts = [];
    for (var i = 0; i < count; i++) {
        parts.push('0x' + byteHex(bytes[i]));
    }
    var s = parts.join(', ');
    if (bytes.length > maxBytes) {
        s += ', ... (' + (bytes.length - maxBytes) + ' more bytes)';
    }
    return s;
}

function formatByteArrayAsC(bytes) {
    var result = [];
    var lineBytes = [];
    var lineAscii = '';

    for (var i = 0; i < bytes.length; i++) {
        lineBytes.push('0x' + byteHex(bytes[i]));
        lineAscii += (bytes[i] >= 0x20 && bytes[i] <= 0x7E)
            ? String.fromCharCode(bytes[i]) : '.';

        if (lineBytes.length === 16 || i === bytes.length - 1) {
            var hexPart = '    ' + lineBytes.join(', ');
            if (i < bytes.length - 1) hexPart += ',';
            while (hexPart.length < 88) hexPart += ' ';
            hexPart += '// ' + lineAscii;
            if (i < bytes.length - 1) hexPart += ' \\';
            result.push(hexPart);
            lineBytes = [];
            lineAscii = '';
        }
    }
    return result.join('\n');
}
