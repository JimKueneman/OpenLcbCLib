/* ============================================================================
 * fdi_processing.js — Load FDI XML, generate byte array (train nodes)
 * ============================================================================ */

var fdiXmlText = '';
var fdiBytes = null;
var fdiFileName = '';

function initFDI() {
    document.getElementById('file-load-fdi').addEventListener('change', function(e) {
        var file = e.target.files[0];
        if (!file) return;
        loadFdiFile(file);
    });

    // Click-to-browse and drag-drop on FDI warning box
    initFileDropZone('fdi-needed-0xfa', 'file-load-fdi', loadFdiFile);
}

function loadFdiFile(file) {
    fdiFileName = file.name;
    var reader = new FileReader();
    reader.onload = function(ev) {
        fdiXmlText = ev.target.result;
        processFDI();
    };
    reader.readAsText(file);
}

function processFDI() {
    if (!fdiXmlText) return;

    var encoder = new TextEncoder();
    var rawBytes = encoder.encode(fdiXmlText);
    fdiBytes = new Uint8Array(rawBytes.length + 1);
    fdiBytes.set(rawBytes);
    fdiBytes[rawBytes.length] = 0x00;

    // Update UI — compact picker in Feature Flags
    var ffName = document.getElementById('ff-fdi-file-name');
    if (ffName) { ffName.textContent = fdiFileName; ffName.classList.add('loaded'); }
    var ffBytes = document.getElementById('ff-fdi-bytes');
    if (ffBytes) { ffBytes.textContent = fdiBytes.length + ' B'; }

    // Update file-load button badge
    var loadBtns = document.querySelectorAll('#ff-fdi-picker .btn-file');
    for (var i = 0; i < loadBtns.length; i++) loadBtns[i].classList.add('loaded');

    // Show clear button
    showHide('btn-clear-fdi', true);

    var highAddr = fdiBytes.length - 1;
    document.getElementById('addr-0xfa-high').value = formatHexDec(highAddr);

    // Hide "FDI needed" warning now that FDI is loaded
    updateFdiNeededWarnings();

    // Persist state after FDI load
    if (typeof saveState === 'function') saveState();
}

function clearFdi() {
    fdiXmlText = '';
    fdiBytes = null;
    fdiFileName = '';

    // Reset compact picker in Feature Flags
    var ffName = document.getElementById('ff-fdi-file-name');
    if (ffName) { ffName.textContent = 'No file loaded'; ffName.classList.remove('loaded'); }
    var ffBytes = document.getElementById('ff-fdi-bytes');
    if (ffBytes) { ffBytes.textContent = ''; }

    // Reset file-load button badge
    var loadBtns = document.querySelectorAll('#ff-fdi-picker .btn-file');
    for (var i = 0; i < loadBtns.length; i++) loadBtns[i].classList.remove('loaded');

    // Hide clear button
    showHide('btn-clear-fdi', false);

    // Reset Address Space 0xFA highest address
    document.getElementById('addr-0xfa-high').value = '(from FDI)';

    // Show "FDI needed" warning now that FDI is cleared
    updateFdiNeededWarnings();

    // Reset file input so the same file can be re-selected
    document.getElementById('file-load-fdi').value = '';

    // Persist cleared state
    if (typeof saveState === 'function') saveState();
}

function generateStarterFDI() {
    var xml =
        '<?xml version="1.0"?>\n'
      + '<fdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"\n'
      + '     xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/fdi/1/1/fdi.xsd">\n'
      + '  <segment origin="0" space="249">\n'
      + '    <n>Train Functions</n>\n'
      + '  </segment>\n'
      + '</fdi>\n';

    downloadTextFile('node_fdi.xml', xml);
}
