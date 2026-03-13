/* =========================================================================
 * fdi_function_grid.js  —  Visual FDI function editor (RH panel)
 *
 * Exposes: window.FdiFunctionGrid
 *   .init(gridEl, popoverOverlayEl, onChangeCb)
 *   .syncFromXml(xmlString)
 *   .buildXmlFromSlots(slots)
 *
 * Manages 29 function slots (F0–F28) displayed as a button grid.
 * Bidirectional sync with the XML editor in the LH pane.
 * ========================================================================= */

(function () {

    'use strict';

    var SLOT_COUNT = 29;   /* F0 through F28 */

    /* Default names for the basic template slots */
    var DEFAULT_NAMES = [
        'Headlight', 'Rear light', 'Horn', 'Bell'
    ];

    /* ----------------------------------------------------------------------- */
    /* State                                                                    */
    /* ----------------------------------------------------------------------- */

    var _gridEl       = null;
    var _popOverlay   = null;
    var _onChangeCb   = null;
    var _editingIndex = -1;

    /**
     * Each slot: { defined: bool, name: string, kind: 'binary'|'momentary' }
     * If defined === false, the slot is empty (greyed out).
     */
    var _slots = [];

    function _emptySlot() {

        return { defined: false, name: '', kind: 'binary' };

    }

    function _initSlots() {

        _slots = [];

        for (var i = 0; i < SLOT_COUNT; i++) {

            _slots.push(_emptySlot());

        }

    }

    /* ----------------------------------------------------------------------- */
    /* Rendering                                                                */
    /* ----------------------------------------------------------------------- */

    function _render() {

        _gridEl.innerHTML = '';

        for (var i = 0; i < SLOT_COUNT; i++) {

            var slot = _slots[i];
            var btn  = document.createElement('div');

            btn.className = 'fn-btn' + (slot.defined ? ' fn-defined' : ' fn-empty');
            btn.dataset.index = i;

            /* F-number label */
            var numSpan = document.createElement('span');
            numSpan.className   = 'fn-number';
            numSpan.textContent = 'F' + i;
            btn.appendChild(numSpan);

            /* Name label */
            var nameSpan = document.createElement('span');
            nameSpan.className   = 'fn-name';
            nameSpan.textContent = slot.defined ? slot.name : '(empty)';
            btn.appendChild(nameSpan);

            /* Kind badge (only when defined) */
            if (slot.defined) {

                var kindSpan = document.createElement('span');
                kindSpan.className   = 'fn-kind-badge fn-kind-' + slot.kind;
                kindSpan.textContent = slot.kind;
                btn.appendChild(kindSpan);

            }

            btn.addEventListener('click', _onSlotClick);
            _gridEl.appendChild(btn);

        }

    }

    /* ----------------------------------------------------------------------- */
    /* Popover                                                                  */
    /* ----------------------------------------------------------------------- */

    var _popTitle  = null;
    var _popName   = null;
    var _popKind   = null;
    var _popSave   = null;
    var _popCancel = null;
    var _popRemove = null;

    function _wirePopover() {

        _popTitle  = document.getElementById('pop-title');
        _popName   = document.getElementById('pop-name');
        _popKind   = document.getElementById('pop-kind');
        _popSave   = document.getElementById('pop-save');
        _popCancel = document.getElementById('pop-cancel');
        _popRemove = document.getElementById('pop-remove');

        _popSave.addEventListener('click', _onPopSave);
        _popCancel.addEventListener('click', _onPopClose);
        _popRemove.addEventListener('click', _onPopRemove);

        /* Close on overlay click (outside the popover box) */
        _popOverlay.addEventListener('click', function (e) {

            if (e.target === _popOverlay) { _onPopClose(); }

        });

        /* Enter key saves */
        _popName.addEventListener('keydown', function (e) {

            if (e.key === 'Enter') { _onPopSave(); }

        });

    }

    function _openPopover(index) {

        _editingIndex = index;
        var slot = _slots[index];

        _popTitle.textContent = 'F' + index + (slot.defined ? ' — ' + slot.name : ' — (empty)');
        _popName.value = slot.defined ? slot.name : '';
        _popKind.value = slot.defined ? slot.kind : 'binary';

        _popRemove.style.display = slot.defined ? '' : 'none';

        _popOverlay.classList.add('visible');
        _popName.focus();
        _popName.select();

    }

    function _onPopClose() {

        _popOverlay.classList.remove('visible');
        _editingIndex = -1;

    }

    function _onPopSave() {

        if (_editingIndex < 0) { return; }

        var name = _popName.value.trim();

        if (!name) {

            /* Empty name = remove the function */
            _slots[_editingIndex] = _emptySlot();

        } else {

            _slots[_editingIndex] = {
                defined: true,
                name:    name,
                kind:    _popKind.value
            };

        }

        _onPopClose();
        _render();
        _notifyChange();

    }

    function _onPopRemove() {

        if (_editingIndex < 0) { return; }

        _slots[_editingIndex] = _emptySlot();
        _onPopClose();
        _render();
        _notifyChange();

    }

    function _onSlotClick(e) {

        var btn   = e.currentTarget;
        var index = parseInt(btn.dataset.index);

        _openPopover(index);

    }

    function _notifyChange() {

        if (_onChangeCb) { _onChangeCb(_slots); }

    }

    /* ----------------------------------------------------------------------- */
    /* XML → Slots  (parse FDI XML into the 29-slot array)                     */
    /* ----------------------------------------------------------------------- */

    function syncFromXml(xmlString) {

        _initSlots();

        if (!xmlString || !xmlString.trim()) {

            _render();
            return;

        }

        var parser = new DOMParser();
        var xmlDoc = parser.parseFromString(xmlString, 'text/xml');

        if (xmlDoc.getElementsByTagName('parsererror').length > 0) {

            _render();
            return;

        }

        var functions = xmlDoc.getElementsByTagName('function');

        for (var i = 0; i < functions.length; i++) {

            var fn = functions[i];

            /* Resolve function number */
            var numAttr = fn.getAttribute('number');
            var numEl   = fn.getElementsByTagName('number')[0];
            var numVal  = numAttr !== null ? numAttr : (numEl ? numEl.textContent.trim() : null);

            if (numVal === null || numVal === '') { continue; }

            var num = parseInt(numVal);
            if (isNaN(num) || num < 0 || num >= SLOT_COUNT) { continue; }

            /* Name */
            var nameEl = fn.getElementsByTagName('name')[0];
            var name   = nameEl ? nameEl.textContent.trim() : 'F' + num;

            /* Kind */
            var kind = fn.getAttribute('kind') || 'binary';
            if (kind !== 'binary' && kind !== 'momentary') { kind = 'binary'; }

            _slots[num] = {
                defined: true,
                name:    name,
                kind:    kind
            };

        }

        _render();

    }

    /* ----------------------------------------------------------------------- */
    /* Slots → XML  (rebuild full FDI XML from the 29-slot array)              */
    /* ----------------------------------------------------------------------- */

    function buildXmlFromSlots(slots) {

        var lines = [];

        lines.push('<?xml version="1.0"?>');
        lines.push('<fdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"');
        lines.push('     xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/fdi/1/1/fdi.xsd">');
        lines.push('');
        lines.push('  <segment space="249" origin="0">');
        lines.push('    <group>');
        lines.push('      <name>Train Functions</name>');

        for (var i = 0; i < slots.length; i++) {

            var s = slots[i];
            if (!s.defined) { continue; }

            lines.push('      <function size="1" kind="' + _escXml(s.kind) + '">');
            lines.push('        <name>' + _escXml(s.name) + '</name>');
            lines.push('        <number>' + i + '</number>');
            lines.push('      </function>');

        }

        lines.push('    </group>');
        lines.push('  </segment>');
        lines.push('');
        lines.push('</fdi>');

        return lines.join('\n');

    }

    function _escXml(str) {

        return str
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;');

    }

    /* ----------------------------------------------------------------------- */
    /* Public API                                                               */
    /* ----------------------------------------------------------------------- */

    function init(gridEl, popoverOverlayEl, onChangeCb) {

        _gridEl     = gridEl;
        _popOverlay = popoverOverlayEl;
        _onChangeCb = onChangeCb;

        _initSlots();
        _wirePopover();
        _render();

    }

    window.FdiFunctionGrid = {
        init:              init,
        syncFromXml:       syncFromXml,
        buildXmlFromSlots: buildXmlFromSlots
    };

}());
