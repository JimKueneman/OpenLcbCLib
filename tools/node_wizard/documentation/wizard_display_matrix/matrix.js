/* ============================================================================
 * matrix.js — Rendering engine for the editable configuration matrix
 *
 * Reads MATRIX_SECTIONS from matrix_data.js, builds interactive tables with:
 *   - ON/OFF dropdowns for feature flag / PSI cells
 *   - Yes/No dropdowns for Visible columns
 *   - true/false dropdowns for boolean config options
 *   - Text inputs for all other editable cells
 *   - Change tracking with yellow highlight
 *   - Section collapse/expand
 *   - JSON export of current state
 *
 * Column layout:
 *   Field | --- Basic UI (6 cols) --- | --- Advanced UI (3 cols) --- |
 *         | Basic | Typical | Train | Visible | Derived From | Notes | Visible | Default | Notes |
 * ============================================================================ */

var allCollapsed = false;
var changeCount = 0;

/* ---------- Original values snapshot (for change tracking) ---------- */

var originalValues = {};   // key: "sectionId:rowIdx:colKey" → original string

function cellKey(sectionId, rowIdx, colKey) {

    return sectionId + ':' + rowIdx + ':' + colKey;

}

/* ---------- Detect appropriate input type for a cell ---------- */

function detectCellType(value, colKey) {

    // Visible columns always get Yes/No dropdown regardless of value
    if (colKey === 'bVis' || colKey === 'aVis') {
        return 'yesno-vis';
    }

    // Notes columns are always free text even if value happens to be Yes/No
    if (colKey === 'bNotes' || colKey === 'aNotes' || colKey === 'bDerived') {
        return 'text';
    }

    var v = value.trim();

    // ON / OFF (exact or with asterisk)
    if (v === 'ON' || v === 'OFF' || v === 'OFF*') {
        return 'onoff';
    }

    // true / false
    if (v === 'true' || v === 'false') {
        return 'truefalse';
    }

    // Yes / No by value (any column not already handled above)
    if (v === 'Yes' || v === 'No' || v === 'Yes (Basic only)') {
        return 'yesno-vis';
    }

    // Everything else is free text
    return 'text';

}

/* ---------- Build a <select> element ---------- */

function makeSelect(options, currentValue, cssClass) {

    var sel = document.createElement('select');
    var matched = false;

    for (var i = 0; i < options.length; i++) {
        var opt = document.createElement('option');
        opt.value = options[i];
        opt.textContent = options[i];
        if (options[i] === currentValue) {
            opt.selected = true;
            matched = true;
        }
        sel.appendChild(opt);
    }

    // If value didn't match any option, add it as a custom option
    if (!matched) {
        var custom = document.createElement('option');
        custom.value = currentValue;
        custom.textContent = currentValue;
        custom.selected = true;
        sel.insertBefore(custom, sel.firstChild);
    }

    applyValueClass(sel, currentValue);
    return sel;

}

/* ---------- Apply value-based CSS class ---------- */

function applyValueClass(el, value) {

    el.className = el.className.replace(/\bval-\S+/g, '').trim();

    var v = value.trim().toUpperCase();

    if (v === 'ON')    { el.classList.add('val-on');  }
    else if (v === 'OFF' || v === 'OFF*') { el.classList.add('val-off'); }
    else if (v === 'YES' || v === 'YES (BASIC ONLY)') { el.classList.add('val-yes'); }
    else if (v === 'NO')   { el.classList.add('val-no');  }
    else if (v === 'N/A')  { el.classList.add('val-na');  }
    else if (v === 'TRUE') { el.classList.add('val-on');  }
    else if (v === 'FALSE') { el.classList.add('val-off'); }
    else if (v === '?')    { el.classList.add('val-unknown'); }

}

/* ---------- Create an editable cell ---------- */

function makeEditableCell(td, value, sectionId, rowIdx, colKey) {

    var key = cellKey(sectionId, rowIdx, colKey);
    originalValues[key] = value;

    var type = detectCellType(value, colKey);
    var el;

    if (type === 'onoff') {
        el = makeSelect(['ON', 'OFF', 'OFF*'], value);
    } else if (type === 'truefalse') {
        el = makeSelect(['true', 'false'], value);
    } else if (type === 'yesno-vis') {
        el = makeSelect(['Yes', 'No', 'Yes (Basic only)'], value);
    } else {
        el = document.createElement('input');
        el.type = 'text';
        el.value = value;
        applyValueClass(el, value);
    }

    el.dataset.key = key;

    el.addEventListener('change', function () {

        var newVal = this.value;
        applyValueClass(this, newVal);

        if (newVal !== originalValues[key]) {
            td.classList.add('changed');
        } else {
            td.classList.remove('changed');
        }

        updateChangeCount();

    });

    td.appendChild(el);

}

/* ---------- Update the change counter badge ---------- */

function updateChangeCount() {

    var changed = document.querySelectorAll('td.changed');
    changeCount = changed.length;

    var badge = document.getElementById('change-count');

    if (changeCount > 0) {
        badge.textContent = changeCount + ' change' + (changeCount === 1 ? '' : 's');
    } else {
        badge.textContent = '';
    }

}

/* ---------- Build one section (header + table) ---------- */

function buildSection(section) {

    var root = document.getElementById('matrix-root');

    // Section header (clickable to collapse)
    var header = document.createElement('div');
    header.className = 'section-header';
    header.innerHTML = '<span class="toggle">&#9660;</span><h2>' + section.title + '</h2>';
    root.appendChild(header);

    // Table
    var table = document.createElement('table');
    table.id = 'table-' + section.id;

    // --- Header row 1: group spans ---
    var thead = document.createElement('thead');
    var tr1 = document.createElement('tr');

    // Field column — spans 2 header rows
    var thField = document.createElement('th');
    thField.rowSpan = 2;
    thField.textContent = section.fieldLabel || 'Field';
    tr1.appendChild(thField);

    // Basic UI group header — colspan 6 (Basic, Typical, Train, Visible, Derived From, Notes)
    var thBasicGroup = document.createElement('th');
    thBasicGroup.colSpan = 6;
    thBasicGroup.className = 'group-basic';
    thBasicGroup.textContent = 'Basic UI';
    tr1.appendChild(thBasicGroup);

    // Advanced UI group header — colspan 3 (Visible, Default, Notes)
    var thAdvGroup = document.createElement('th');
    thAdvGroup.colSpan = 3;
    thAdvGroup.className = 'group-advanced';
    thAdvGroup.textContent = 'Advanced UI';
    tr1.appendChild(thAdvGroup);

    thead.appendChild(tr1);

    // --- Header row 2: sub-columns ---
    var tr2 = document.createElement('tr');

    var basicSubs = ['Basic', 'Typical', 'Train', 'Visible', 'Derived From', 'Notes'];
    for (var b = 0; b < basicSubs.length; b++) {
        var thb = document.createElement('th');
        thb.className = 'sub-basic';
        thb.textContent = basicSubs[b];
        tr2.appendChild(thb);
    }

    var advSubs = ['Visible', 'Default', 'Notes'];
    for (var a = 0; a < advSubs.length; a++) {
        var tha = document.createElement('th');
        tha.className = 'sub-advanced';
        tha.textContent = advSubs[a];
        tr2.appendChild(tha);
    }

    thead.appendChild(tr2);
    table.appendChild(thead);

    // --- Body rows ---
    var tbody = document.createElement('tbody');

    for (var r = 0; r < section.rows.length; r++) {
        var row = section.rows[r];
        var tr = document.createElement('tr');

        // Field name (read-only)
        var tdField = document.createElement('td');
        tdField.className = 'field-name';
        tdField.textContent = row.field;
        tr.appendChild(tdField);

        // Basic UI — node type value columns
        var valCols = ['basic', 'typical', 'train'];
        for (var c = 0; c < valCols.length; c++) {
            var td = document.createElement('td');
            if (row[valCols[c]] === '') {
                td.className = 'cell-blank';
            } else {
                makeEditableCell(td, row[valCols[c]], section.id, r, valCols[c]);
            }
            tr.appendChild(td);
        }

        // Basic UI — Visible
        var tdBVis = document.createElement('td');
        makeEditableCell(tdBVis, row.bVis, section.id, r, 'bVis');
        tr.appendChild(tdBVis);

        // Basic UI — Derived From
        var tdBDer = document.createElement('td');
        makeEditableCell(tdBDer, row.bDerived, section.id, r, 'bDerived');
        tr.appendChild(tdBDer);

        // Basic UI — Notes
        var tdBNotes = document.createElement('td');
        tdBNotes.className = 'notes-cell';
        makeEditableCell(tdBNotes, row.bNotes, section.id, r, 'bNotes');
        tr.appendChild(tdBNotes);

        // Advanced UI — Visible
        var tdAVis = document.createElement('td');
        makeEditableCell(tdAVis, row.aVis, section.id, r, 'aVis');
        tr.appendChild(tdAVis);

        // Advanced UI — Default
        var tdADef = document.createElement('td');
        makeEditableCell(tdADef, row.aDefault, section.id, r, 'aDefault');
        tr.appendChild(tdADef);

        // Advanced UI — Notes
        var tdANotes = document.createElement('td');
        tdANotes.className = 'notes-cell';
        makeEditableCell(tdANotes, row.aNotes, section.id, r, 'aNotes');
        tr.appendChild(tdANotes);

        tbody.appendChild(tr);
    }

    table.appendChild(tbody);
    root.appendChild(table);

    // Wire collapse/expand
    header.addEventListener('click', function (tbl) {

        return function () {

            var toggle = this.querySelector('.toggle');
            tbl.classList.toggle('collapsed');
            toggle.classList.toggle('collapsed');

        };

    }(table));

}

/* ---------- Render all sections ---------- */

function renderAll() {

    for (var i = 0; i < MATRIX_SECTIONS.length; i++) {
        buildSection(MATRIX_SECTIONS[i]);
    }

}

/* ---------- Toggle All collapse/expand ---------- */

function toggleAll() {

    allCollapsed = !allCollapsed;

    var tables = document.querySelectorAll('#matrix-root table');
    var toggles = document.querySelectorAll('.section-header .toggle');

    for (var i = 0; i < tables.length; i++) {
        if (allCollapsed) {
            tables[i].classList.add('collapsed');
        } else {
            tables[i].classList.remove('collapsed');
        }
    }

    for (var j = 0; j < toggles.length; j++) {
        if (allCollapsed) {
            toggles[j].classList.add('collapsed');
        } else {
            toggles[j].classList.remove('collapsed');
        }
    }

    document.getElementById('btn-collapse').textContent =
        allCollapsed ? 'Expand All' : 'Collapse All';

}

/* ---------- Export current matrix state as JSON ---------- */

function exportJSON() {

    var result = [];
    var cols = ['basic', 'typical', 'train', 'bVis', 'bDerived', 'bNotes', 'aVis', 'aDefault', 'aNotes'];

    for (var s = 0; s < MATRIX_SECTIONS.length; s++) {
        var section = MATRIX_SECTIONS[s];
        var sectionOut = {
            id: section.id,
            title: section.title,
            rows: []
        };

        for (var r = 0; r < section.rows.length; r++) {
            var rowOut = { field: section.rows[r].field };

            for (var c = 0; c < cols.length; c++) {
                var key = cellKey(section.id, r, cols[c]);
                var el = document.querySelector('[data-key="' + key + '"]');
                rowOut[cols[c]] = el ? el.value : section.rows[r][cols[c]];
            }

            // Mark if any cell in this row changed
            var changed = false;
            for (var k = 0; k < cols.length; k++) {
                var ky = cellKey(section.id, r, cols[k]);
                var elem = document.querySelector('[data-key="' + ky + '"]');
                if (elem && elem.value !== originalValues[ky]) {
                    changed = true;
                    break;
                }
            }
            if (changed) {
                rowOut._changed = true;
            }

            sectionOut.rows.push(rowOut);
        }

        result.push(sectionOut);
    }

    var json = JSON.stringify(result, null, 2);

    // Download as file
    var blob = new Blob([json], { type: 'application/json' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = 'matrix_config.json';
    a.click();
    URL.revokeObjectURL(url);

}

/* ---------- Load JSON with backwards-compatible merge ---------- */

function loadJSON(fileInput) {

    if (!fileInput.files || !fileInput.files[0]) { return; }

    var reader = new FileReader();

    reader.onload = function (e) {

        var imported;
        try {
            imported = JSON.parse(e.target.result);
        } catch (err) {
            alert('Invalid JSON file: ' + err.message);
            fileInput.value = '';
            return;
        }

        if (!Array.isArray(imported)) {
            alert('Expected a JSON array of sections.');
            fileInput.value = '';
            return;
        }

        // Build lookup: sectionId → { fieldName → row data }
        var importMap = {};
        for (var s = 0; s < imported.length; s++) {
            var sec = imported[s];
            if (!sec.id || !sec.rows) { continue; }
            var rowMap = {};
            for (var r = 0; r < sec.rows.length; r++) {
                rowMap[sec.rows[r].field] = sec.rows[r];
            }
            importMap[sec.id] = rowMap;
        }

        var cols = ['basic', 'typical', 'train', 'bVis', 'bDerived', 'bNotes', 'aVis', 'aDefault', 'aNotes'];

        // Backwards-compat: map old column names to new ones
        var colAliases = {
            aDerived: 'aDefault',
            notes: 'bNotes'
        };

        // Merge: walk MATRIX_SECTIONS (canonical baseline), overlay imported values
        var merged = JSON.parse(JSON.stringify(MATRIX_SECTIONS));  // deep copy
        var mergedCount = 0;
        var newCount = 0;

        for (var ms = 0; ms < merged.length; ms++) {
            var mSec = merged[ms];
            var iRows = importMap[mSec.id];

            for (var mr = 0; mr < mSec.rows.length; mr++) {
                var mRow = mSec.rows[mr];

                if (iRows && iRows[mRow.field]) {
                    var iRow = iRows[mRow.field];

                    // Apply current columns
                    for (var c = 0; c < cols.length; c++) {
                        if (iRow.hasOwnProperty(cols[c])) {
                            mRow[cols[c]] = iRow[cols[c]];
                        }
                    }

                    // Apply old column aliases (backwards compat with previous exports)
                    for (var oldKey in colAliases) {
                        if (iRow.hasOwnProperty(oldKey) && !iRow.hasOwnProperty(colAliases[oldKey])) {
                            mRow[colAliases[oldKey]] = iRow[oldKey];
                        }
                    }

                    mergedCount++;
                } else {
                    newCount++;
                }
            }
        }

        // Clear and re-render
        document.getElementById('matrix-root').innerHTML = '';
        originalValues = {};
        allCollapsed = false;
        document.getElementById('btn-collapse').textContent = 'Collapse All';

        for (var i = 0; i < merged.length; i++) {
            buildSection(merged[i]);
        }

        updateChangeCount();

        var msg = 'Loaded: ' + mergedCount + ' rows merged.';
        if (newCount > 0) {
            msg += ' ' + newCount + ' new rows added from current schema.';
        }
        document.getElementById('change-count').textContent = msg;

        fileInput.value = '';

    };

    reader.readAsText(fileInput.files[0]);

}

/* ---------- Boot ---------- */

renderAll();
