/* xmllint_api.js — async wrapper around the patched Emscripten xmllint module.
 *
 * Provides: window.xmllint.validateXML({ xml: [string], schema: [string] })
 * Returns:  Promise<{ errors: string[] | null }>
 *
 * The module is initialized once on first call and reused.
 * Each validation creates a fresh module instance to avoid state leaks.
 */

(function () {
    'use strict';

    /* Cache the factory function once it is loaded */
    var _factory = null;

    function _getFactory() {
        if (_factory) { return Promise.resolve(_factory); }

        /* The factory is loaded via <script> tag as createXmllintModule */
        if (typeof createXmllintModule === 'function') {
            _factory = createXmllintModule;
            return Promise.resolve(_factory);
        }

        return Promise.reject(new Error('createXmllintModule not loaded — include xmllint_patched.js before xmllint_api.js'));
    }

    function validateXML(options) {
        var xml    = options.xml;
        var schema = options.schema;
        if (!Array.isArray(xml))    { xml    = [xml]; }
        if (!Array.isArray(schema)) { schema = [schema]; }

        return _getFactory().then(function (factory) {

            var stderrLines = [];

            return factory({
                print:    function () {},
                printErr: function (text) { stderrLines.push(text); }
            }).then(function (Module) {

                /* Write XML and XSD files to virtual FS */
                for (var i = 0; i < xml.length; i++) {
                    Module.FS.writeFile('/file_' + i + '.xml', xml[i]);
                }
                for (var i = 0; i < schema.length; i++) {
                    Module.FS.writeFile('/file_' + i + '.xsd', schema[i]);
                }

                /* Build CLI arguments */
                var args = ['--noout'];
                for (var i = 0; i < schema.length; i++) {
                    args.push('--schema');
                    args.push('/file_' + i + '.xsd');
                }
                for (var i = 0; i < xml.length; i++) {
                    args.push('/file_' + i + '.xml');
                }

                /* Run xmllint */
                try {
                    Module.callMain(args);
                } catch (e) {
                    /* callMain may throw on exit — expected */
                }

                return {
                    errors: stderrLines.length > 0 ? stderrLines : null
                };
            });
        });
    }

    window.xmllint = { validateXML: validateXML };

}());
