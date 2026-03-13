/* ============================================================================
 * validation_rules.js — Rendering engine for the validation rules matrix
 *
 * Reads VALIDATION_SECTIONS from validation_rules_data.js, builds interactive
 * tables with:
 *   - Severity dropdown (error / warning)
 *   - Operator dropdowns (===, !==, >, <, >=, <=)
 *   - Text inputs for field names and values
 *   - Change tracking with yellow highlight
 *   - Section collapse/expand
 *   - JSON export / import
 * ============================================================================ */

var allCollapsed = false;
var changeCount = 0;
var originalValues = {};

/* ---------- Column definitions ---------- */

var COLUMNS = [
    { key: 'id',        label: 'Rule ID',   type: 'text',     width: '70px'  },
    { key: 'severity',  label: 'Severity',  type: 'severity', width: '80px'  },
    { key: 'ifField',   label: 'If Field',  type: 'text',     width: ''      },
    { key: 'ifOp',      label: 'Op',        type: 'op',       width: '60px'  },
    { key: 'ifValue',   label: 'If Value',  type: 'text',     width: ''      },
    { key: 'thenField', label: 'Then Field', type: 'text',    width: ''      },
    { key: 'thenOp',    label: 'Must Be',   type: 'op',       width: '60px'  },
    { key: 'thenValue', label: 'Value',     type: 'text',     width: ''      },
    { key: 'message',   label: 'Message',   type: 'text',     width: ''      },
    { key: 'notes',     label: 'Notes',     type: 'text',     width: ''      }
];

var OPS = ['===', '!==', '>', '<', '>=', '<='];
var SEVERITIES = ['error', 'warning'];

/* ---------- Cell key ---------- */

function cellKey(sectionId, rowIdx, colKey) {

    return sectionId + ':' + rowIdx + ':' + colKey;

}

/* ---------- Build a <select> ---------- */

function makeSelect(options, currentValue) {

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

/* ---------- Apply value CSS class ---------- */

function applyValueClass(el, value) {

    el.className = el.className.replace(/\bval-\S+/g, '').trim();

    var v = (value || '').trim().toLowerCase();

    if (v === 'error')   { el.classList.add('val-error');   }
    if (v === 'warning') { el.classList.add('val-warning'); }
    if (v === '===')     { el.classList.add('val-op-eq');   }
    if (v === '!==')     { el.classList.add('val-op-neq');  }

}

/* ---------- Create an editable cell ---------- */

function makeEditableCell(td, value, sectionId, rowIdx, colKey, colType) {

    var key = cellKey(sectionId, rowIdx, colKey);
    originalValues[key] = value;

    var el;

    if (colType === 'severity') {
        el = makeSelect(SEVERITIES, value);
    } else if (colType === 'op') {
        el = makeSelect(OPS, value);
    } else {
        el = document.createElement('input');
        el.type = 'text';
        el.value = value;
        applyValueClass(el, value);
    }

    el.dataset.key = key;

    el.addEventListener('change', function () {

        applyValueClass(this, this.value);

        if (this.value !== originalValues[key]) {
            td.classList.add('changed');
        } else {
            td.classList.remove('changed');
        }

        updateChangeCount();

    });

    td.appendChild(el);

}

/* ---------- Update change counter ---------- */

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

/* ---------- Build one section ---------- */

function buildSection(section) {

    var root = document.getElementById('rules-root');

    var header = document.createElement('div');
    header.className = 'section-header';
    header.innerHTML = '<span class="toggle">&#9660;</span><h2>' + section.title + '</h2>' +
        (section.description ? '<p class="section-desc">' + section.description + '</p>' : '');
    root.appendChild(header);

    var table = document.createElement('table');
    table.id = 'table-' + section.id;

    // Header row
    var thead = document.createElement('thead');
    var tr = document.createElement('tr');

    for (var c = 0; c < COLUMNS.length; c++) {
        var th = document.createElement('th');
        th.textContent = COLUMNS[c].label;
        if (COLUMNS[c].width) { th.style.width = COLUMNS[c].width; }
        tr.appendChild(th);
    }

    thead.appendChild(tr);
    table.appendChild(thead);

    // Body rows
    var tbody = document.createElement('tbody');

    for (var r = 0; r < section.rows.length; r++) {
        var row = section.rows[r];
        var tr2 = document.createElement('tr');
        tr2.className = 'severity-' + (row.severity || 'warning');

        for (var col = 0; col < COLUMNS.length; col++) {
            var td = document.createElement('td');
            var colDef = COLUMNS[col];
            var val = row[colDef.key] !== undefined ? row[colDef.key] : '';

            if (colDef.key === 'id') {
                // Rule ID is read-only
                td.className = 'rule-id';
                td.textContent = val;
            } else {
                makeEditableCell(td, val, section.id, r, colDef.key, colDef.type);
            }

            tr2.appendChild(td);
        }

        tbody.appendChild(tr2);
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

/* ---------- Render all ---------- */

function renderAll() {

    for (var i = 0; i < VALIDATION_SECTIONS.length; i++) {
        buildSection(VALIDATION_SECTIONS[i]);
    }

}

/* ---------- Toggle all collapse ---------- */

function toggleAll() {

    allCollapsed = !allCollapsed;

    var tables = document.querySelectorAll('#rules-root table');
    var toggles = document.querySelectorAll('.section-header .toggle');

    for (var i = 0; i < tables.length; i++) {
        if (allCollapsed) { tables[i].classList.add('collapsed'); }
        else { tables[i].classList.remove('collapsed'); }
    }

    for (var j = 0; j < toggles.length; j++) {
        if (allCollapsed) { toggles[j].classList.add('collapsed'); }
        else { toggles[j].classList.remove('collapsed'); }
    }

    document.getElementById('btn-collapse').textContent =
        allCollapsed ? 'Expand All' : 'Collapse All';

}

/* ---------- Export JSON ---------- */

function exportJSON() {

    var result = [];
    var colKeys = COLUMNS.map(function (c) { return c.key; });

    for (var s = 0; s < VALIDATION_SECTIONS.length; s++) {
        var section = VALIDATION_SECTIONS[s];
        var sectionOut = { id: section.id, title: section.title, rows: [] };

        for (var r = 0; r < section.rows.length; r++) {
            var rowOut = {};

            for (var c = 0; c < colKeys.length; c++) {
                var key = cellKey(section.id, r, colKeys[c]);
                var el = document.querySelector('[data-key="' + key + '"]');
                rowOut[colKeys[c]] = el ? el.value : (section.rows[r][colKeys[c]] || '');
            }

            var changed = false;
            for (var k = 0; k < colKeys.length; k++) {
                var ky = cellKey(section.id, r, colKeys[k]);
                var elem = document.querySelector('[data-key="' + ky + '"]');
                if (elem && elem.value !== originalValues[ky]) { changed = true; break; }
            }
            if (changed) { rowOut._changed = true; }

            sectionOut.rows.push(rowOut);
        }

        result.push(sectionOut);
    }

    var json = JSON.stringify(result, null, 2);
    var blob = new Blob([json], { type: 'application/json' });
    var url = URL.createObjectURL(blob);
    var a = document.createElement('a');
    a.href = url;
    a.download = 'validation_rules.json';
    a.click();
    URL.revokeObjectURL(url);

}

/* ---------- Load JSON ---------- */

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

        var importMap = {};
        for (var s = 0; s < imported.length; s++) {
            var sec = imported[s];
            if (!sec.id || !sec.rows) { continue; }
            var rowMap = {};
            for (var r = 0; r < sec.rows.length; r++) {
                rowMap[sec.rows[r].id] = sec.rows[r];
            }
            importMap[sec.id] = rowMap;
        }

        var colKeys = COLUMNS.map(function (c) { return c.key; });
        var merged = JSON.parse(JSON.stringify(VALIDATION_SECTIONS));
        var mergedCount = 0;
        var newCount = 0;

        for (var ms = 0; ms < merged.length; ms++) {
            var mSec = merged[ms];
            var iRows = importMap[mSec.id];

            for (var mr = 0; mr < mSec.rows.length; mr++) {
                var mRow = mSec.rows[mr];

                if (iRows && iRows[mRow.id]) {
                    var iRow = iRows[mRow.id];
                    for (var c = 0; c < colKeys.length; c++) {
                        if (iRow.hasOwnProperty(colKeys[c])) {
                            mRow[colKeys[c]] = iRow[colKeys[c]];
                        }
                    }
                    mergedCount++;
                } else {
                    newCount++;
                }
            }
        }

        document.getElementById('rules-root').innerHTML = '';
        originalValues = {};
        allCollapsed = false;
        document.getElementById('btn-collapse').textContent = 'Collapse All';

        for (var i = 0; i < merged.length; i++) {
            buildSection(merged[i]);
        }

        updateChangeCount();

        var msg = 'Loaded: ' + mergedCount + ' rows merged.';
        if (newCount > 0) { msg += ' ' + newCount + ' new rows from current schema.'; }
        document.getElementById('change-count').textContent = msg;

        fileInput.value = '';

    };

    reader.readAsText(fileInput.files[0]);

}

/* ---------- Boot ---------- */

renderAll();
