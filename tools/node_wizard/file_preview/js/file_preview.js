/* =========================================================================
 * File Preview — UI logic
 * Shows a tree of generated files (mirroring ZIP layout) on the left
 * and a CodeMirror read-only viewer on the right.
 * Receives full wizard state from parent via postMessage.
 * ========================================================================= */

'use strict';

/* ---- State ---- */

var _wizardState = null;
var _fileMap     = {};   /* { 'path/to/file.c': contentString, ... } */
var _selectedFile = null;
var _cmViewer    = null;

/* ---- DOM refs ---- */

var _elTree     = document.getElementById('file-tree');
var _elFilename = document.getElementById('viewer-filename');
var _elViewer   = document.getElementById('code-viewer');

/* ========================================================================= */
/* CodeMirror viewer                                                         */
/* ========================================================================= */

function _ensureCMViewer() {

    if (_cmViewer) { return; }

    if (typeof createCMReadonly === 'function') {

        _cmViewer = createCMReadonly(_elViewer);

    }

}

/* ========================================================================= */
/* Codegen state builder (mirrors zip_export.js _buildCodegenState)          */
/* ========================================================================= */

function _buildCodegenState(ws) {

    var s = {};

    if (ws.configFormState) {

        Object.keys(ws.configFormState).forEach(function (key) {
            s[key] = ws.configFormState[key];
        });

    }

    s.nodeType      = ws.selectedNodeType;
    s.driverState   = ws.driverState   || {};
    s.callbackState = ws.callbackState || {};
    s.platformState = ws.platformState || null;
    s.cdiUserXml         = ws.cdiUserXml         || null;
    s.fdiUserXml         = ws.fdiUserXml         || null;
    s.preserveWhitespace = !!ws.preserveWhitespace;

    return s;

}

/* ========================================================================= */
/* Active group detection (mirrors zip_export.js)                            */
/* ========================================================================= */

function _getActiveCallbackGroups(state) {

    var active = [];

    if (typeof CALLBACK_GROUPS === 'undefined') { return active; }

    var groupKeys = Object.keys(CALLBACK_GROUPS);

    for (var i = 0; i < groupKeys.length; i++) {

        var key   = groupKeys[i];
        var group = CALLBACK_GROUPS[key];
        var cs = (state.callbackState && state.callbackState[key]) ? state.callbackState[key] : null;
        var checkedNames = (cs && cs.checked) ? cs.checked : [];

        var activeFns = [];
        for (var j = 0; j < group.functions.length; j++) {

            var fn = group.functions[j];
            if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
                activeFns.push(fn);
            }

        }

        if (activeFns.length > 0) {
            active.push({ key: key, group: group, functions: activeFns });
        }

    }

    return active;

}

function _getActiveDriverGroups(state) {

    var active = [];

    if (typeof DRIVER_GROUPS === 'undefined') { return active; }

    var groupKeys = Object.keys(DRIVER_GROUPS);
    var isBootloader = state.nodeType === 'bootloader';

    for (var i = 0; i < groupKeys.length; i++) {

        var key   = groupKeys[i];
        var group = DRIVER_GROUPS[key];
        var ds = (state.driverState && state.driverState[key]) ? state.driverState[key] : null;
        var checkedNames = (ds && ds.checked) ? ds.checked : [];

        var activeFns = [];
        for (var j = 0; j < group.functions.length; j++) {

            var fn = group.functions[j];
            if (isBootloader && (fn.name === 'config_mem_read' || fn.name === 'config_mem_write')) { continue; }
            if (fn.required || checkedNames.indexOf(fn.name) >= 0) {
                activeFns.push(fn);
            }

        }

        if (activeFns.length > 0) {
            active.push({ key: key, group: group, functions: activeFns });
        }

    }

    return active;

}

/* ========================================================================= */
/* Include path fixup helpers (mirrors zip_export.js)                        */
/* ========================================================================= */

function _fixMainIncludes(code, isArduino) {

    var prefix = isArduino ? 'src/' : '';

    code = code.replace(
        /#include "src\/(openlcb|drivers|utilities)\//g,
        '#include "' + prefix + 'openlcb_c_lib/$1/'
    );

    code = code.replace(
        /#include "openlcb_can_drivers\.h"/g,
        '#include "' + prefix + 'application_drivers/openlcb_can_drivers.h"'
    );
    code = code.replace(
        /#include "openlcb_drivers\.h"/g,
        '#include "' + prefix + 'application_drivers/openlcb_drivers.h"'
    );

    code = code.replace(
        /#include "(callbacks_\w+\.h)"/g,
        '#include "' + prefix + 'application_callbacks/$1"'
    );

    return code;

}

function _fixSubfolderIncludes(code) {

    code = code.replace(
        /#include "src\/(openlcb|drivers|utilities)\//g,
        '#include "../openlcb_c_lib/$1/'
    );

    return code;

}

function _fixConfigIncludes(code, isArduino) {

    var prefix = isArduino ? 'src/' : '';

    code = code.replace(
        /#include "src\/(openlcb|drivers|utilities)\//g,
        '#include "' + prefix + 'openlcb_c_lib/$1/'
    );

    return code;

}

/* ========================================================================= */
/* Build file map from wizard state                                          */
/* ========================================================================= */

function _buildFileMap(ws) {

    var map = {};

    if (!ws || !ws.selectedNodeType) { return map; }

    var codegenState = _buildCodegenState(ws);
    var isArduino    = !!ws.arduino;
    var srcExt       = isArduino ? '.cpp' : '.c';
    var mainFilename = isArduino ? 'main.ino' : 'main.c';

    /* Subfolder prefix for Arduino layout */
    var sub = isArduino ? 'src/' : '';

    /* Core files */
    if (typeof generateMain === 'function') {
        map[mainFilename] = _fixMainIncludes(generateMain(codegenState), isArduino);
    }
    if (typeof generateH === 'function') {
        map['openlcb_user_config.h'] = _fixConfigIncludes(generateH(codegenState), isArduino);
    }
    if (typeof generateC === 'function') {
        map['openlcb_user_config.c'] = _fixConfigIncludes(generateC(codegenState), isArduino);
    }
    if (typeof generateCanH === 'function') {
        map['can_user_config.h'] = generateCanH(codegenState);
    }

    /* Driver files */
    var activeDrivers = _getActiveDriverGroups(codegenState);

    if (typeof DriverCodegen !== 'undefined') {

        activeDrivers.forEach(function (entry) {

            var hCode = DriverCodegen.generateH(entry.group, entry.functions, ws.platformState);
            var cCode = DriverCodegen.generateC(entry.group, entry.functions, ws.platformState, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            map[sub + 'application_drivers/' + entry.group.filePrefix + '.h']      = hCode;
            map[sub + 'application_drivers/' + entry.group.filePrefix + srcExt] = cCode;

        });

    }

    /* Callback files */
    var activeCallbacks = _getActiveCallbackGroups(codegenState);

    if (typeof CallbackCodegen !== 'undefined') {

        activeCallbacks.forEach(function (entry) {

            var hCode = CallbackCodegen.generateH(entry.group, entry.functions);
            var cCode = CallbackCodegen.generateC(entry.group, entry.functions, isArduino);

            hCode = _fixSubfolderIncludes(hCode);
            cCode = _fixSubfolderIncludes(cCode);

            map[sub + 'application_callbacks/' + entry.group.filePrefix + '.h']      = hCode;
            map[sub + 'application_callbacks/' + entry.group.filePrefix + srcExt] = cCode;

        });

    }

    /* XML files — bootloader has no CDI or FDI */
    if (ws.selectedNodeType !== 'bootloader') {
        if (ws.cdiUserXml && ws.cdiUserXml.trim()) {
            map[sub + 'xml_files/cdi.xml'] = ws.cdiUserXml;
        }
        if (ws.fdiUserXml && ws.fdiUserXml.trim()) {
            map[sub + 'xml_files/fdi.xml'] = ws.fdiUserXml;
        }
    }

    /* Placeholder library folders (shown as empty folders in tree) */
    map[sub + 'openlcb_c_lib/openlcb/'] = null;
    map[sub + 'openlcb_c_lib/drivers/canbus/'] = null;
    map[sub + 'openlcb_c_lib/utilities/'] = null;

    return map;

}

/* ========================================================================= */
/* Tree rendering                                                            */
/* ========================================================================= */

/**
 * Convert flat file map into a nested tree structure:
 * { name, children: [ { name, children, path }, ... ], path }
 * Leaves have `path` pointing to _fileMap key and no children.
 */
function _buildTree(fileMap) {

    var root = { name: '', children: [] };

    Object.keys(fileMap).sort().forEach(function (path) {

        var parts    = path.split('/');
        var current  = root;

        for (var i = 0; i < parts.length; i++) {

            var part     = parts[i];
            var isLast   = (i === parts.length - 1);
            var isFolder = !isLast || path.endsWith('/');

            if (isFolder && isLast && path.endsWith('/')) {

                /* Explicit empty folder */
                var existing = null;
                for (var k = 0; k < current.children.length; k++) {
                    if (current.children[k].name === part && current.children[k].children) {
                        existing = current.children[k];
                        break;
                    }
                }
                if (!existing) {
                    current.children.push({ name: part, children: [], emptyFolder: true });
                }

            } else if (!isLast) {

                /* Intermediate folder */
                var found = null;
                for (var k = 0; k < current.children.length; k++) {
                    if (current.children[k].name === part && current.children[k].children) {
                        found = current.children[k];
                        break;
                    }
                }
                if (!found) {
                    found = { name: part, children: [] };
                    current.children.push(found);
                }
                current = found;

            } else {

                /* File leaf */
                current.children.push({ name: part, path: path });

            }

        }

    });

    return root;

}

function _renderTree(container, nodes) {

    nodes.forEach(function (node) {

        if (node.children) {

            /* Folder */
            var folderEl = document.createElement('div');

            var headerEl = document.createElement('div');
            headerEl.className = 'tree-folder';

            var startCollapsed = (node.name === 'openlcb_c_lib');

            var iconEl = document.createElement('span');
            iconEl.className = 'tree-folder-icon' + (startCollapsed ? '' : ' open');
            iconEl.textContent = '\u25B6';

            var nameEl = document.createElement('span');
            nameEl.className = 'tree-folder-name';
            nameEl.textContent = node.name + '/';

            headerEl.appendChild(iconEl);
            headerEl.appendChild(nameEl);

            var childrenEl = document.createElement('div');
            childrenEl.className = 'tree-children' + (startCollapsed ? ' collapsed' : '');

            if (node.emptyFolder) {

                var placeholder = document.createElement('div');
                placeholder.className = 'tree-placeholder';
                placeholder.textContent = '(copy library files here)';
                childrenEl.appendChild(placeholder);

            } else if (node.children.length > 0) {

                _renderTree(childrenEl, node.children);

            }

            headerEl.addEventListener('click', function () {

                var collapsed = childrenEl.classList.toggle('collapsed');
                iconEl.classList.toggle('open', !collapsed);

            });

            folderEl.appendChild(headerEl);
            folderEl.appendChild(childrenEl);
            container.appendChild(folderEl);

        } else {

            /* File */
            var fileEl = document.createElement('div');
            fileEl.className = 'tree-file';
            fileEl.dataset.path = node.path;

            var fileIconEl = document.createElement('span');
            fileIconEl.className = 'tree-file-icon';

            var fileNameEl = document.createElement('span');
            fileNameEl.textContent = node.name;

            fileEl.appendChild(fileIconEl);
            fileEl.appendChild(fileNameEl);

            fileEl.addEventListener('click', function () {

                _selectFile(node.path);

            });

            container.appendChild(fileEl);

        }

    });

}

function _selectFile(path) {

    _selectedFile = path;

    /* Update selection highlight */
    _elTree.querySelectorAll('.tree-file').forEach(function (el) {
        el.classList.toggle('selected', el.dataset.path === path);
    });

    /* Update filename header */
    _elFilename.textContent = path;

    /* Show content */
    _ensureCMViewer();

    var content = _fileMap[path];
    if (content != null && _cmViewer) {
        _cmViewer.value = content;
    } else if (_cmViewer) {
        _cmViewer.value = '// (empty placeholder folder)';
    }

    /* Persist selection to parent */
    window.parent.postMessage({ type: 'filePreviewSelection', selectedFile: path }, '*');

}

/* ========================================================================= */
/* Full refresh                                                              */
/* ========================================================================= */

function _refresh() {

    _fileMap = _buildFileMap(_wizardState);

    var tree = _buildTree(_fileMap);

    _elTree.innerHTML = '';

    if (tree.children.length === 0) {

        var msg = document.createElement('div');
        msg.className = 'tree-placeholder';
        msg.textContent = 'No node type selected';
        _elTree.appendChild(msg);
        return;

    }

    _renderTree(_elTree, tree.children);

    /* Restore saved selection from wizard state (parent persists this) */
    if (!_selectedFile && _wizardState && _wizardState.filePreviewSelection) {
        _selectedFile = _wizardState.filePreviewSelection;
    }

    /* Re-select previously selected file if still present */
    if (_selectedFile && _fileMap.hasOwnProperty(_selectedFile) && _fileMap[_selectedFile] != null) {

        _selectFile(_selectedFile);

    } else {

        /* Auto-select the first file */
        var firstFile = _elTree.querySelector('.tree-file');
        if (firstFile && firstFile.dataset.path) {
            _selectFile(firstFile.dataset.path);
        }

    }

}

/* ========================================================================= */
/* Generate button                                                           */
/* ========================================================================= */

function requestDownload() {

    window.parent.postMessage({ type: 'requestDownload' }, '*');

}

/* ========================================================================= */
/* Splitter drag                                                             */
/* ========================================================================= */

(function () {

    var splitter  = document.getElementById('splitter');
    var treePanel = document.querySelector('.tree-panel');

    if (!splitter || !treePanel) { return; }

    var dragging = false;

    splitter.addEventListener('mousedown', function (e) {

        e.preventDefault();
        dragging = true;
        document.body.style.cursor = 'col-resize';
        document.body.style.userSelect = 'none';

    });

    document.addEventListener('mousemove', function (e) {

        if (!dragging) { return; }

        var newWidth = e.clientX;
        if (newWidth < 180) { newWidth = 180; }
        if (newWidth > 500) { newWidth = 500; }
        treePanel.style.flex = '0 0 ' + newWidth + 'px';

    });

    document.addEventListener('mouseup', function () {

        if (!dragging) { return; }
        dragging = false;
        document.body.style.cursor = '';
        document.body.style.userSelect = '';

    });

}());

/* ========================================================================= */
/* Message listener                                                          */
/* ========================================================================= */

window.addEventListener('message', function (e) {

    if (!e.data || !e.data.type) { return; }

    switch (e.data.type) {

        case 'setWizardState':

            _wizardState = e.data.state || null;
            _refresh();
            break;

    }

});

/* ========================================================================= */
/* Init                                                                      */
/* ========================================================================= */

window.parent.postMessage({ type: 'ready' }, '*');
