/* =========================================================================
 * language_target.js — Multi-language code-generation contract.
 *
 * Each language target (C, JS, …) implements this interface and registers
 * itself with LanguageTargets.  ZipExport orchestrates the actual ZIP build
 * by calling target.buildFiles(state) and writing the returned entries.
 *
 * Why an interface: the wizard supports more than one output language.
 * ZipExport stays target-agnostic; targets own all language-specific code.
 *
 * Entry shape:
 *   { path: 'foo/bar.h', content: '...' }   — file with text content
 *   { path: 'empty/dir',  dir: true }       — empty placeholder directory
 *
 * @typedef {Object} TargetEntry
 * @property {string} path
 * @property {string} [content]
 * @property {boolean} [dir]
 * @property {boolean} [previewable] — defaults to true.  When explicitly
 *           false, this entry is omitted from the file-preview tree but is
 *           still included in the generated ZIP.  Use for meta files (e.g.
 *           GETTING_STARTED.txt, <project>_project.json) that ship to the
 *           user but aren't source code worth previewing in the editor.
 *
 * @typedef {Object} ByteRow
 * @property {number[]} bytes — bytes for this row (each 0..255)
 * @property {string}   comment — text shown after the bytes (e.g. originating XML line)
 *
 * @typedef {Object} ByteArrayResult
 * @property {string} text       — full formatted output ready to copy/paste
 * @property {number} totalBytes — total byte count across all rows
 *
 * @typedef {Object} LanguageTarget
 * @property {string} id
 * @property {string} label
 * @property {string[]} applicablePanels — sidebar view ids this target supports.
 *           Recognised: 'config', 'cdi', 'fdi', 'platform-drivers', 'callbacks',
 *           'file-preview'.  Tiles for views NOT in this list are hidden.
 * @property {function(Object): TargetEntry[]} buildFiles
 * @property {function(Object): string} projectLabel
 * @property {function(ByteRow[], string): ByteArrayResult} [renderByteArray]
 *           — used by the CDI/FDI editors' "Array" view.  Takes pre-converted
 *           byte rows (each row carries a comment with the originating XML line)
 *           plus a `kind` string ('cdi' or 'fdi') and returns the language-
 *           specific copy/pasteable byte-array literal.  New languages plug in
 *           a new implementation here without the editors needing to change.
 * ========================================================================= */

var LanguageTargets = (function () {

    'use strict';

    var _registry = {};

    function register(target) {
        _registry[target.id] = target;
    }

    function get(id) {
        var t = _registry[id || 'c'];
        if (!t) {
            throw new Error('Unknown language target: ' + id);
        }
        return t;
    }

    return { register: register, get: get };

}());
