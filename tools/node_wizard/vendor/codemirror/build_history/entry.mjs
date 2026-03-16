/* CodeMirror 6 — bundle entry point
 * Exports everything the CDI / FDI editors need as window.CM */

import { EditorView, basicSetup } from 'codemirror';
import { EditorState } from '@codemirror/state';
import { xml } from '@codemirror/lang-xml';
import { cpp } from '@codemirror/lang-cpp';
import { oneDark } from '@codemirror/theme-one-dark';
import { keymap } from '@codemirror/view';
import { indentWithTab } from '@codemirror/commands';
import { linter, lintGutter, forceLinting, setDiagnostics } from '@codemirror/lint';

window.CM = {
    EditorView,
    EditorState,
    basicSetup,
    xml,
    cpp,
    oneDark,
    keymap,
    indentWithTab,
    linter,
    lintGutter,
    forceLinting,
    setDiagnostics
};
