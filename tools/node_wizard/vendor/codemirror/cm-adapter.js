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
 *   .format()        — pretty-print the XML in the editor
 *   .onChange(fn)    — register a callback for text changes
 *   .onCursorChange(fn)
 *   .view            — the underlying EditorView (escape hatch)
 *
 * Options:
 *   doc         — initial document text (string)
 *   placeholder — placeholder text shown when editor is empty (string)
 *   validator   — object with a validate(xmlString) method; when provided,
 *                 inline squiggles are enabled using @codemirror/lint.
 *                 The validate() method must return an array of:
 *                   { severity: 'error'|'warning'|'info',
 *                     message: string,
 *                     line: number|null }
 * ========================================================================= */

'use strict';

/* =========================================================================
 * CDI XML schema — passed to @codemirror/lang-xml so the editor knows which
 * elements and attributes are valid in each context.  This enables the
 * context-aware autocomplete dropdown (Ctrl+Space or just start typing "<").
 * ========================================================================= */
/* <<CDI_SCHEMA_START>> */
var CDI_SCHEMA = {
    elements: [
        {
            name: 'cdi',
            top: true,
            children: ['identification', 'acdi', 'segment']
        },

        {
            name: 'acdi',
            attributes: [
                { name: 'fixed' },
                { name: 'var' }
            ]
        },

        {
            name: 'action',
            attributes: [
                { name: 'size', values: ['1', '2', '4', '8'] },
                { name: 'offset' }
            ],
            children: ['name', 'description', 'buttonText', 'dialogText', 'value']
        },

        {
            name: 'blob',
            attributes: [
                { name: 'size', values: ['10'] },
                { name: 'offset' },
                { name: 'mode', values: ['read', 'write', 'readwrite'] }
            ],
            children: ['name', 'description']
        },

        {
            name: 'buttonText'
        },

        {
            name: 'checkbox'
        },

        {
            name: 'default'
        },

        {
            name: 'description'
        },

        {
            name: 'dialogText'
        },

        {
            name: 'eventid',
            attributes: [
                { name: 'offset' }
            ],
            children: ['name', 'description', 'map']
        },

        {
            name: 'float',
            attributes: [
                { name: 'size', values: ['2', '4', '8'] },
                { name: 'offset' },
                { name: 'formatting' }
            ],
            children: ['name', 'description', 'min', 'max', 'default', 'map']
        },

        {
            name: 'group',
            attributes: [
                { name: 'offset' },
                { name: 'replication' }
            ],
            children: ['name', 'description', 'link', 'repname', 'hints', 'group', 'string', 'int', 'eventid', 'float', 'action', 'blob']
        },

        {
            name: 'hardwareVersion'
        },

        {
            name: 'hints',
            attributes: [
                { name: 'tickSpacing' },
                { name: 'immediate' },
                { name: 'showValue' }
            ],
            children: ['slider', 'radiobutton', 'checkbox']
        },

        {
            name: 'identification',
            children: ['manufacturer', 'model', 'hardwareVersion', 'softwareVersion', 'link', 'map']
        },

        {
            name: 'int',
            attributes: [
                { name: 'size', values: ['1', '2', '4', '8'] },
                { name: 'offset' }
            ],
            children: ['name', 'description', 'min', 'max', 'default', 'map', 'hints']
        },

        {
            name: 'link',
            attributes: [
                { name: 'ref' }
            ]
        },

        {
            name: 'manufacturer'
        },

        {
            name: 'map',
            children: ['name', 'description', 'relation']
        },

        {
            name: 'max'
        },

        {
            name: 'min'
        },

        {
            name: 'model'
        },

        {
            name: 'name'
        },

        {
            name: 'property'
        },

        {
            name: 'radiobutton'
        },

        {
            name: 'readOnly'
        },

        {
            name: 'relation',
            children: ['property', 'value']
        },

        {
            name: 'repname'
        },

        {
            name: 'segment',
            attributes: [
                { name: 'space' },
                { name: 'origin' }
            ],
            children: ['name', 'description', 'link', 'group', 'string', 'int', 'eventid', 'float', 'action', 'blob']
        },

        {
            name: 'slider',
            attributes: [
                { name: 'tickSpacing' },
                { name: 'immediate' },
                { name: 'showValue' }
            ]
        },

        {
            name: 'softwareVersion'
        },

        {
            name: 'string',
            attributes: [
                { name: 'size' },
                { name: 'offset' }
            ],
            children: ['name', 'description', 'map']
        },

        {
            name: 'value'
        },

        {
            name: 'visibility',
            attributes: [
                { name: 'hideable' },
                { name: 'hidden' }
            ]
        }
    ]
};
/* <<CDI_SCHEMA_END>> */


/* =========================================================================
 * FDI XML schema — same idea as CDI_SCHEMA above, but for Function
 * Description Information.  Regenerate with:
 *   python3 generate_fdi_schema.py --xsd /path/to/fdi.xsd --patch
 * ========================================================================= */
/* <<FDI_SCHEMA_START>> */
var FDI_SCHEMA = {
    elements: [
        {
            name: 'fdi',
            top: true,
            children: ['segment']
        },

        {
            name: 'description'
        },

        {
            name: 'function',
            attributes: [
                { name: 'kind', values: ['binary', 'momentary', 'analog'] },
                { name: 'size', values: ['1'] }
            ],
            children: ['name', 'number', 'min', 'max']
        },

        {
            name: 'group',
            children: ['name', 'description', 'group', 'function']
        },

        {
            name: 'max'
        },

        {
            name: 'min'
        },

        {
            name: 'name'
        },

        {
            name: 'number'
        },

        {
            name: 'segment',
            attributes: [
                { name: 'space', values: ['249'] },
                { name: 'origin', values: ['0'] }
            ],
            children: ['name', 'description', 'group', 'function']
        }
    ]
};
/* <<FDI_SCHEMA_END>> */


/* =========================================================================
 * formatXml(xmlString)
 *
 * Takes a well-formed XML string and returns a pretty-printed version with
 * 2-space indentation.  Returns the original string unchanged if parsing
 * fails (so callers can always use the return value safely).
 * ========================================================================= */
function formatXml(xmlString) {

    var parser = new DOMParser();
    var xmlDoc = parser.parseFromString(xmlString.trim(), 'text/xml');

    if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
        return xmlString;   /* leave broken XML alone */
    }

    var INDENT = '  ';

    /* Recursively serialise a DOM node back to indented text */
    function serializeNode(node, depth) {

        var pad    = INDENT.repeat(depth);
        var result = '';
        var i;

        /* Text node — only output if it has non-whitespace content */
        if (node.nodeType === 3) {
            var t = node.textContent.trim();
            if (t) { result += pad + t + '\n'; }
            return result;
        }

        /* Comment node (nodeType 8) — preserve with current indentation.
         * The internal text is kept exactly as written so multi-line comments
         * (like the guidance blocks in the Basic XML template) are not mangled. */
        if (node.nodeType === 8) {
            result += pad + '<!--' + node.textContent + '-->\n';
            return result;
        }

        /* Skip any other non-element nodes (processing instructions, etc.) */
        if (node.nodeType !== 1) { return ''; }

        /* Build attribute string */
        var tag   = node.tagName;
        var attrs = '';
        for (i = 0; i < node.attributes.length; i++) {
            attrs += ' ' + node.attributes[i].name + '="' + node.attributes[i].value + '"';
        }

        /* Collect children that have real content.
         * Keep: elements (1), non-empty text (3), comments (8).
         * Drop: whitespace-only text nodes. */
        var kids = [];
        for (i = 0; i < node.childNodes.length; i++) {
            var child = node.childNodes[i];
            if (child.nodeType === 3 && !child.textContent.trim()) { continue; }
            kids.push(child);
        }

        /* If any child is a comment, force the multi-child branch so that
         * comments get their own indented lines rather than being squeezed
         * onto a single-text-child line. */
        var hasComment = false;
        for (i = 0; i < kids.length; i++) {
            if (kids[i].nodeType === 8) { hasComment = true; break; }
        }

        if (kids.length === 0) {
            /* Empty element — use self-closing form */
            result += pad + '<' + tag + attrs + '/>\n';

        } else if (kids.length === 1 && kids[0].nodeType === 3 && !hasComment) {
            /* Single text child with no comments — keep on one line */
            result += pad + '<' + tag + attrs + '>' + kids[0].textContent.trim() + '</' + tag + '>\n';

        } else {
            /* Element with child elements (or comments) — indent children */
            result += pad + '<' + tag + attrs + '>\n';
            for (i = 0; i < kids.length; i++) {
                result += serializeNode(kids[i], depth + 1);
            }
            result += pad + '</' + tag + '>\n';
        }

        return result;
    }

    return '<?xml version="1.0" encoding="utf-8"?>\n' +
           serializeNode(xmlDoc.documentElement, 0);
}


/* =========================================================================
 * createCMEditor(container, options)
 * ========================================================================= */
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
        },
        /* Suppress lint squiggles — keep only the gutter dots */
        '.cm-lintRange-error':   { backgroundImage: 'none', borderBottom: 'none' },
        '.cm-lintRange-warning': { backgroundImage: 'none', borderBottom: 'none' },
        '.cm-lintRange-info':    { backgroundImage: 'none', borderBottom: 'none' }
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

    /* Build extension list */
    var extensions = [
        CM.basicSetup,
        CM.xml(options.schema || CDI_SCHEMA),   /* schema — context-aware autocomplete */
        CM.oneDark,
        darkTheme,
        CM.keymap.of([CM.indentWithTab]),
        updateListener
    ];

    /* ---- Inline validation squiggles ------------------------------------ *
     * Only enabled when a validator is passed in options.
     * The linter waits 600 ms after the last keystroke before re-running so
     * it does not fire on every single character typed.
     * --------------------------------------------------------------------- */
    if (options.validator && CM.linter) {

        /* Build CM6 diagnostic objects from validator issues */
        var _buildDiagnostics = function (issues, viewState) {

            var results = [];
            var i;

            for (i = 0; i < issues.length; i++) {

                var issue = issues[i];
                var from  = 0;
                var to    = 1;

                /* If the validator gave us a line number, mark that whole line */
                if (issue.line && issue.line > 0) {
                    try {
                        var lineInfo = viewState.doc.line(issue.line);
                        from = lineInfo.from;
                        to   = lineInfo.to > lineInfo.from ? lineInfo.to : lineInfo.from + 1;
                    } catch (e) {
                        from = 0;
                        to   = 1;
                    }
                } else {
                    /* No line number — mark the first line so the squiggle is visible */
                    try {
                        var firstLine = viewState.doc.line(1);
                        from = firstLine.from;
                        to   = firstLine.to > firstLine.from ? firstLine.to : firstLine.from + 1;
                    } catch (e) {
                        from = 0;
                        to   = Math.min(viewState.doc.length, 1);
                    }
                }

                results.push({
                    from:     from,
                    to:       to,
                    severity: issue.severity === 'error'   ? 'error'   :
                              issue.severity === 'warning' ? 'warning' : 'info',
                    message:  issue.message
                });
            }

            return results;
        };

        /* The linter always runs — CM6 owns the inline markers (dots +
           squiggles).  The "Auto Validate Schema" checkbox only controls
           whether the validation *bar* auto-updates; the inline markers
           always reflect the current document state. */
        var lintSource = function (view) {

            var xml = view.state.doc.toString();

            return options.validator.validate(xml).then(function (issues) {
                return _buildDiagnostics(issues, view.state);
            });
        };

        extensions.push(CM.linter(lintSource, { delay: 600 }));
        extensions.push(CM.lintGutter());
    }

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
                    from:   0,
                    to:     view.state.doc.length,
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

        /* ----------------------------------------------------------------
         * format() — pretty-print the XML currently in the editor.
         * Does nothing if the document is empty or not well-formed.
         * --------------------------------------------------------------- */
        format: function () {

            var text = view.state.doc.toString().trim();
            if (!text) { return; }

            var formatted = formatXml(text);

            /* Only update if the text actually changed */
            if (formatted !== view.state.doc.toString()) {
                view.dispatch({
                    changes: {
                        from:   0,
                        to:     view.state.doc.length,
                        insert: formatted
                    }
                });
            }
        },

        onChange: function (fn) {
            _onChange = fn;
        },

        onCursorChange: function (fn) {
            _onCursor = fn;
        },

        /* Get line/column from a character offset */
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
        },

        /* Force the CM6 linter to re-run now (e.g. after loading new XML) */
        forceLint: function () {
            if (CM.forceLinting) {
                CM.forceLinting(view);
            }
        }

    };

    return adapter;

};
