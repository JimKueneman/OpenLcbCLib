/* =========================================================================
 * cm-readonly.js — Read-only CodeMirror 6 viewer for C code previews
 *
 * Replaces highlight.js for syntax-highlighted code display.
 *
 * Usage:
 *   var viewer = createCMReadonly(container);
 *   viewer.value = cCodeString;    // updates displayed code
 * ========================================================================= */

'use strict';

window.createCMReadonly = function createCMReadonly(container) {

    /* Dark theme matching the vs2015 look */
    var darkTheme = CM.EditorView.theme({
        '&': {
            backgroundColor: '#1e1e1e',
            color: '#d4d4d4',
            height: '100%',
            fontSize: '10.5px'
        },
        '.cm-content': {
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace",
            lineHeight: '1.5',
            padding: '8px 0',
            tabSize: '4',
            caretColor: 'transparent'
        },
        '.cm-gutters': {
            backgroundColor: '#1e1e1e',
            color: '#858585',
            border: 'none',
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace",
            fontSize: '10.5px',
            lineHeight: '1.5'
        },
        '.cm-lineNumbers .cm-gutterElement': {
            padding: '0 8px 0 12px',
            minWidth: '32px'
        },
        '.cm-activeLine': {
            backgroundColor: 'transparent'
        },
        '.cm-activeLineGutter': {
            backgroundColor: 'transparent'
        },
        '&.cm-focused': {
            outline: 'none'
        },
        '.cm-scroller': {
            overflow: 'auto',
            fontFamily: "'Consolas', 'Menlo', 'Courier New', monospace"
        }
    }, { dark: true });

    var view = new CM.EditorView({
        state: CM.EditorState.create({
            doc: '',
            extensions: [
                CM.basicSetup,
                CM.cpp(),
                CM.oneDark,
                darkTheme,
                CM.EditorState.readOnly.of(true),
                CM.EditorView.editable.of(false)
            ]
        }),
        parent: container
    });

    return {

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

        }

    };

};
