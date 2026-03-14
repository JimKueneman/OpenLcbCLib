/* =========================================================================
 * cm-adapter.js — Thin adapter between CodeMirror 6 and the CDI/FDI editors
 *
 * Creates a CodeMirror editor inside a given container and returns an object
 * with the same API surface the editors used with the textarea:
 *
 *   .value          — get/set the full document text
 *   .selectionStart — cursor offset (read-only)
 *   .focus()        — focus the editor
 *   .setSelectionRange(from, to)
 *   .onChange(fn)    — register a callback for text changes
 *   .onCursorChange(fn)
 *   .view            — the underlying EditorView (escape hatch)
 * ========================================================================= */

'use strict';

window.createCMEditor = function createCMEditor(container, options) {

    options = options || {};

    var _onChange = null;
    var _onCursor = null;

    /* Custom theme to match the vs2015 dark look */
    var darkTheme = CM.EditorView.theme({
        '&': {
            backgroundColor: '#1e1e1e',
            color: '#d4d4d4',
            height: '100%',
            fontSize: '12px'
        },
        '.cm-content': {
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace",
            lineHeight: '1.5',
            padding: '8px 0',
            tabSize: '2',
            caretColor: '#aeafad'
        },
        '.cm-gutters': {
            backgroundColor: '#1e1e1e',
            color: '#858585',
            border: 'none',
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace",
            fontSize: '12px',
            lineHeight: '1.5'
        },
        '.cm-lineNumbers .cm-gutterElement': {
            padding: '0 8px 0 12px',
            minWidth: '32px'
        },
        '.cm-activeLine': {
            backgroundColor: '#2a2d2e'
        },
        '.cm-activeLineGutter': {
            backgroundColor: '#2a2d2e'
        },
        '&.cm-focused .cm-cursor': {
            borderLeftColor: '#aeafad'
        },
        '&.cm-focused .cm-selectionBackground, ::selection': {
            backgroundColor: '#264f78'
        },
        '.cm-selectionBackground': {
            backgroundColor: '#264f78'
        },
        '.cm-scroller': {
            overflow: 'auto',
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace"
        }
    }, { dark: true });

    /* Update listener — fires on every document change */
    var updateListener = CM.EditorView.updateListener.of(function (update) {

        if (update.docChanged && _onChange) {

            _onChange();

        }

        if (update.selectionSet && _onCursor) {

            _onCursor();

        }

    });

    /* Placeholder extension */
    var placeholderText = options.placeholder || '';
    var extensions = [
        CM.basicSetup,
        CM.xml(),
        CM.oneDark,
        darkTheme,
        CM.keymap.of([CM.indentWithTab]),
        updateListener,
        CM.EditorView.lineWrapping
    ];

    /* Disable line wrapping — XML should be unwrapped by default */
    extensions.pop();  /* remove lineWrapping */

    var startDoc = options.doc || '';

    var view = new CM.EditorView({
        state: CM.EditorState.create({
            doc: startDoc,
            extensions: extensions
        }),
        parent: container
    });

    /* ---------- Public API ---------- */

    var adapter = {

        view: view,

        get value() {

            return view.state.doc.toString();

        },

        set value(text) {

            view.dispatch({
                changes: {
                    from: 0,
                    to: view.state.doc.length,
                    insert: text || ''
                }
            });

        },

        get selectionStart() {

            return view.state.selection.main.head;

        },

        focus: function () {

            view.focus();

        },

        setSelectionRange: function (from, to) {

            view.dispatch({
                selection: { anchor: from, head: to }
            });
            view.focus();

        },

        onChange: function (fn) {

            _onChange = fn;

        },

        onCursorChange: function (fn) {

            _onCursor = fn;

        },

        /* Get line/column from offset */
        posToLineCol: function (pos) {

            var line = view.state.doc.lineAt(pos);
            return { line: line.number, col: pos - line.from + 1 };

        },

        /* Line count */
        get lineCount() {

            return view.state.doc.lines;

        },

        /* Byte count (UTF-8) */
        get byteCount() {

            return new TextEncoder().encode(view.state.doc.toString()).length;

        }

    };

    return adapter;

};
