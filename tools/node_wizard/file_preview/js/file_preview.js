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
/* Build file map from wizard state — delegates to the active LanguageTarget */
/* ========================================================================= */

function _buildFileMap(ws) {

    var map = {};

    if (!ws || !ws.selectedNodeType) { return map; }
    if (typeof LanguageTargets === 'undefined') { return map; }

    var target;
    try {
        target = LanguageTargets.get(ws.targetLanguage || 'c');
    } catch (e) {
        return map;
    }

    var entries;
    try {
        entries = target.buildFiles(ws);
    } catch (e) {
        /* Target rejected this state (e.g. JS + bootloader).  Show a single
         * pseudo-file in the tree so the user sees what happened. */
        map['__error__.txt'] = String(e.message || e);
        return map;
    }

    entries.forEach(function (entry) {

        /* Targets can mark meta files (GETTING_STARTED.txt, *_project.json,
         * etc.) as previewable:false — they ship in the ZIP but don't appear
         * in the file-preview tree. */
        if (entry.previewable === false) { return; }

        if (entry.dir) {
            /* Trailing slash signals "empty folder" to the tree renderer. */
            map[entry.path + '/'] = null;
        } else {
            map[entry.path] = entry.content;
        }

    });

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

    /* Restore saved selection from wizard state (parent persists this), but
     * only when the saved path actually exists in the current file map.
     * Otherwise we'd carry a stale selection across target switches —
     * e.g. a 'main.c' selection bleeding into a JS-target session. */
    if (!_selectedFile && _wizardState && _wizardState.filePreviewSelection
        && _fileMap.hasOwnProperty(_wizardState.filePreviewSelection)
        && _fileMap[_wizardState.filePreviewSelection] != null) {
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

        case 'setTargetLanguage':

            /* Local selection becomes stale when target changes — clear it so
             * the next refresh picks a file that exists in the new target. */
            _selectedFile = null;
            break;

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
