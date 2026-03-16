/* =========================================================================
 * fdi_validator.js  —  OpenLCB FDI XML 3-layer validator
 *
 * Exposes: window.FdiValidator.validate(xmlString)
 *
 * Returns: Array of { severity: 'error'|'warning'|'info', message: string, line: number|null }
 *
 * Layer 1 — Well-formedness   : DOMParser catches malformed XML
 * Layer 2 — Structural rules  : required attributes, known element names
 * Layer 3 — Semantic hints    : missing functions, duplicate numbers, etc.
 * ========================================================================= */

(function () {

    const KNOWN_FDI_TAGS = new Set([
        'fdi', 'segment', 'group', 'function',
        'name', 'description', 'number', 'min', 'max'
    ]);

    const VALID_KINDS = new Set(['binary', 'momentary', 'analog']);

    /* Find the 1-based line number where `needle` first appears in `text`.
     * Returns null when not found.                                           */
    function _findLine(text, needle) {
        var idx = text.indexOf(needle);
        if (idx === -1) { return null; }
        var lineNum = 1;
        for (var i = 0; i < idx; i++) {
            if (text.charCodeAt(i) === 10) { lineNum++; }
        }
        return lineNum;
    }

    /* ----------------------------------------------------------------------- */
    function validate(xmlString) {

        const issues = [];

        /* ===== LAYER 1: well-formedness ==================================== */
        const parser  = new DOMParser();
        const xmlDoc  = parser.parseFromString(xmlString, 'text/xml');
        const parseEr = xmlDoc.getElementsByTagName('parsererror');

        if (parseEr.length > 0) {

            const raw   = parseEr[0].textContent;
            const lineM = raw.match(/[Ll]ine\D*?(\d+)/);
            const colM  = raw.match(/[Cc]olumn\D*?(\d+)/);
            const first = raw.split('\n')[0].trim();

            issues.push({
                severity: 'error',
                line:     lineM ? parseInt(lineM[1]) : null,
                col:      colM  ? parseInt(colM[1])  : null,
                message:  'XML parse error: ' + first
            });

            return issues;   /* can't continue with broken XML */

        }

        /* ===== LAYER 2: structural rules =================================== */
        const root = xmlDoc.documentElement;

        /* root element */
        if (root.tagName !== 'fdi') {

            issues.push({ severity: 'error', line: null, col: null,
                message: 'Root element must be <fdi>, found <' + root.tagName + '>' });

        }

        /* unknown top-level children */
        Array.from(root.children).forEach(function (child) {

            if (child.tagName !== 'segment') {

                issues.push({ severity: 'warning', line: null, col: null,
                    message: 'Unexpected child <' + child.tagName + '> of <fdi>' });

            }

        });

        /* singleton segment check */
        var segmentCount = Array.from(root.children).filter(function (c) { return c.tagName === 'segment'; }).length;
        if (segmentCount > 1) {
            issues.push({ severity: 'error', line: null, col: null,
                message: 'FDI allows only one <segment> element, found ' + segmentCount });
        }

        /* segments */
        var segments = Array.from(root.getElementsByTagName('segment'));

        segments.forEach(function (seg, i) {

            var idx = i + 1;

            if (!seg.hasAttribute('space')) {

                issues.push({ severity: 'warning', line: null, col: null,
                    message: '<segment> #' + idx + ' is missing \'space\' attribute — should be 249 for FDI' });

            } else {

                var sp = parseInt(seg.getAttribute('space'));

                if (isNaN(sp)) {

                    issues.push({ severity: 'error', line: null, col: null,
                        message: '<segment> #' + idx + ' space="' + seg.getAttribute('space') + '" is not a valid number' });

                } else if (sp !== 249) {

                    issues.push({ severity: 'warning', line: null, col: null,
                        message: '<segment> #' + idx + ' space="' + sp + '" — FDI should use space 249 (0xF9)' });

                }

            }

        });

        /* function elements */
        var allFunctions = Array.from(xmlDoc.getElementsByTagName('function'));

        allFunctions.forEach(function (fn) {

            var nameEl = fn.getElementsByTagName('name')[0];
            var label  = nameEl ? nameEl.textContent.trim() : '(unnamed)';

            /* kind attribute */
            if (!fn.hasAttribute('kind')) {

                issues.push({ severity: 'error', line: null, col: null,
                    message: '<function> "' + label + '" is missing required \'kind\' attribute (binary or momentary)' });

            } else if (!VALID_KINDS.has(fn.getAttribute('kind'))) {

                issues.push({ severity: 'error', line: null, col: null,
                    message: '<function> "' + label + '" has invalid kind="' + fn.getAttribute('kind') + '" — must be "binary", "momentary", or "analog"' });

            }

            /* name child */
            if (!nameEl || !nameEl.textContent.trim()) {

                issues.push({ severity: 'warning', line: null, col: null,
                    message: '<function> is missing a <name> element — function will have no label' });

            }

            /* number attribute or child */
            var numAttr = fn.getAttribute('number');
            var numEl   = fn.getElementsByTagName('number')[0];
            var numVal  = numAttr !== null ? numAttr : (numEl ? numEl.textContent.trim() : null);

            if (numVal === null || numVal === '') {

                issues.push({ severity: 'warning', line: null, col: null,
                    message: '<function> "' + label + '" has no function number' });

            } else {

                var n = parseInt(numVal);

                if (isNaN(n) || n < 0) {

                    issues.push({ severity: 'error', line: null, col: null,
                        message: '<function> "' + label + '" has invalid number "' + numVal + '" — must be a non-negative integer' });

                }

            }

            /* size attribute */
            if (fn.hasAttribute('size')) {

                var s = parseInt(fn.getAttribute('size'));

                if (isNaN(s) || s < 1) {

                    issues.push({ severity: 'warning', line: null, col: null,
                        message: '<function> "' + label + '" has unusual size="' + fn.getAttribute('size') + '"' });

                }

            }

        });

        /* unknown element tags */
        var allTags = xmlDoc.getElementsByTagName('*');
        var unknown = new Set();

        for (var el of allTags) {

            if (!KNOWN_FDI_TAGS.has(el.tagName)) { unknown.add(el.tagName); }

        }

        unknown.forEach(function (tag) {

            issues.push({ severity: 'warning', line: null, col: null,
                message: 'Unknown element <' + tag + '> — not part of the FDI schema' });

        });

        /* unexpected text content inside container elements ---
         * Elements like <fdi>, <segment>, <group>, etc. should only contain
         * child elements, not raw text.  Whitespace between tags is fine, but
         * anything else (e.g. stray characters typed by accident) is flagged.
         */
        var FDI_CONTAINER_TAGS = new Set([
            'fdi', 'segment', 'group', 'function'
        ]);

        for (var el2 of allTags) {
            if (!FDI_CONTAINER_TAGS.has(el2.tagName)) { continue; }
            for (var node of el2.childNodes) {
                if (node.nodeType === 3) {                /* TEXT_NODE */
                    var trimmed = node.textContent.trim();
                    if (trimmed.length > 0) {
                        issues.push({ severity: 'error',
                            line: _findLine(xmlString, trimmed),
                            col:  null,
                            message: 'Unexpected text "' + trimmed.substring(0, 40) + '" inside <' + el2.tagName + '> — only child elements are allowed here' });
                        break;   /* one report per container is enough */
                    }
                }
            }
        }

        /* ===== LAYER 3: semantic / best-practice hints ==================== */

        /* no segments */
        if (segments.length === 0) {

            issues.push({ severity: 'warning', line: null, col: null,
                message: 'No <segment> element found — FDI needs at least one segment' });

        }

        /* segments with no functions */
        segments.forEach(function (seg, i) {

            var fns = seg.getElementsByTagName('function');

            if (fns.length === 0) {

                issues.push({ severity: 'info', line: null, col: null,
                    message: '<segment> #' + (i + 1) + ' has no <function> elements' });

            }

        });

        /* duplicate function numbers */
        var numbersSeen = {};

        allFunctions.forEach(function (fn) {

            var numAttr = fn.getAttribute('number');
            var numEl   = fn.getElementsByTagName('number')[0];
            var numVal  = numAttr !== null ? numAttr : (numEl ? numEl.textContent.trim() : null);

            if (numVal === null || numVal === '') { return; }

            var n = parseInt(numVal);
            if (isNaN(n)) { return; }

            if (numbersSeen[n]) {

                var nameEl = fn.getElementsByTagName('name')[0];
                var label  = nameEl ? nameEl.textContent.trim() : '(unnamed)';

                issues.push({ severity: 'warning', line: null, col: null,
                    message: 'Duplicate function number ' + n + ' on "' + label + '" — also used by "' + numbersSeen[n] + '"' });

            } else {

                var nameEl2 = fn.getElementsByTagName('name')[0];
                numbersSeen[n] = nameEl2 ? nameEl2.textContent.trim() : '(unnamed)';

            }

        });

        /* function numbers outside standard DCC range */
        allFunctions.forEach(function (fn) {

            var numAttr = fn.getAttribute('number');
            var numEl   = fn.getElementsByTagName('number')[0];
            var numVal  = numAttr !== null ? numAttr : (numEl ? numEl.textContent.trim() : null);

            if (numVal === null) { return; }

            var n = parseInt(numVal);

            if (!isNaN(n) && n > 28) {

                var nameEl = fn.getElementsByTagName('name')[0];
                var label  = nameEl ? nameEl.textContent.trim() : '(unnamed)';

                issues.push({ severity: 'info', line: null, col: null,
                    message: '<function> "' + label + '" uses number ' + n + ' — outside standard DCC range (0–28)' });

            }

        });

        return issues;

    }

    /* ----------------------------------------------------------------------- */
    window.FdiValidator = { validate: validate };

}());
