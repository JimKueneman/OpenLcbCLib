/* =========================================================================
 * zip_export.js  —  Generate a project ZIP from a LanguageTarget's entries.
 *
 * Target-agnostic: looks up the language target on wizardState.targetLanguage
 * (defaults to 'c'), asks it for the file/dir entries to include, and writes
 * them into a JSZip blob the user can download.  All language-specific code
 * lives in the target (c_target.js, future js_target.js, ...).
 *
 * Depends on globals: JSZip, LanguageTargets
 * ========================================================================= */

var ZipExport = (function () {

    'use strict';

    function generateZip(wizardState) {

        if (!wizardState.selectedNodeType) {

            alert('No node type selected -- cannot generate files.');
            return;

        }

        if (typeof JSZip === 'undefined') {

            alert('JSZip library not loaded. Please check your internet connection and reload.');
            return;

        }

        var target  = LanguageTargets.get(wizardState.targetLanguage || 'c');
        var entries = target.buildFiles(wizardState);
        var label   = target.projectLabel(wizardState) || 'node';

        var zip = new JSZip();

        entries.forEach(function (e) {

            if (e.dir) {
                zip.folder(e.path);
            } else {
                zip.file(e.path, e.content);
            }

        });

        var zipFilename = label + '_project.zip';

        zip.generateAsync({ type: 'blob' }).then(function (blob) {

            var url = URL.createObjectURL(blob);
            var a   = document.createElement('a');
            a.href     = url;
            a.download = zipFilename;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);

        }).catch(function (err) {

            alert('Error generating ZIP: ' + err.message);

        });

    }

    return { generateZip: generateZip };

}());
