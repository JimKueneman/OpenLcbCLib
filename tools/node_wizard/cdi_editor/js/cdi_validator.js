/* =========================================================================
 * cdi_validator.js  —  OpenLCB CDI XML 3-layer validator
 *
 * Exposes: window.CdiValidator.validate(xmlString)
 *
 * Returns: Array of { severity: 'error'|'warning'|'info', message: string, line: number|null }
 *
 * Layer 1 — Well-formedness   : DOMParser catches malformed XML
 * Layer 2 — Structural rules  : required attributes, known element names
 * Layer 3 — Semantic hints    : missing identification, ACDI, space 253, etc.
 * ========================================================================= */

(function () {

    const KNOWN_LEAF_TAGS = new Set(['bit', 'int', 'float', 'string', 'eventid']);
    const KNOWN_CDI_TAGS  = new Set([
        'cdi', 'identification', 'acdi', 'segment', 'group',
        'bit', 'int', 'float', 'string', 'eventid',
        'name', 'description', 'repname', 'hints', 'map',
        'relation', 'property', 'value', 'default', 'min', 'max',
        'manufacturer', 'model', 'hardwareVersion', 'softwareVersion',
        'checkbox', 'radiobutton', 'slider'
    ]);

    /* ----------------------------------------------------------------------- */
    function validate(xmlString) {

        const issues = [];

        /* ===== LAYER 1: well-formedness ==================================== */
        const parser  = new DOMParser();
        const xmlDoc  = parser.parseFromString(xmlString, 'text/xml');
        const parseEr = xmlDoc.getElementsByTagName('parsererror');

        if (parseEr.length > 0) {

            const raw     = parseEr[0].textContent;
            const lineM   = raw.match(/[Ll]ine\D*?(\d+)/);
            const colM    = raw.match(/[Cc]olumn\D*?(\d+)/);
            const first   = raw.split('\n')[0].trim();

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
        if (root.tagName !== 'cdi') {
            issues.push({ severity: 'error', line: null, col: null,
                message: `Root element must be <cdi>, found <${root.tagName}>` });
        }

        /* unknown top-level tags */
        Array.from(root.children).forEach(child => {
            if (!['identification', 'acdi', 'segment'].includes(child.tagName)) {
                issues.push({ severity: 'warning', line: null, col: null,
                    message: `Unexpected child <${child.tagName}> of <cdi>` });
            }
        });

        /* singleton elements — only one of each is allowed directly under <cdi> */
        const idCount   = Array.from(root.children).filter(c => c.tagName === 'identification').length;
        const acdiCount = Array.from(root.children).filter(c => c.tagName === 'acdi').length;

        if (idCount > 1) {
            issues.push({ severity: 'error', line: null, col: null,
                message: `Found ${idCount} <identification> elements — only 1 is permitted` });
        }

        if (acdiCount > 1) {
            issues.push({ severity: 'error', line: null, col: null,
                message: `Found ${acdiCount} <acdi> elements — only 1 is permitted` });
        }

        /* segments */
        const segments = Array.from(root.getElementsByTagName('segment'));
        segments.forEach((seg, i) => {

            const idx = i + 1;

            if (!seg.hasAttribute('space')) {
                issues.push({ severity: 'error', line: null, col: null,
                    message: `<segment> #${idx} is missing required 'space' attribute` });
            } else {
                const sp = parseInt(seg.getAttribute('space'));
                if (isNaN(sp) || sp < 0 || sp > 255) {
                    issues.push({ severity: 'error', line: null, col: null,
                        message: `<segment> #${idx} space="${seg.getAttribute('space')}" is not a valid space number (0–255)` });
                }
            }
        });

        /* strings must have size */
        Array.from(xmlDoc.getElementsByTagName('string')).forEach(el => {
            if (!el.hasAttribute('size')) {
                const n = el.getElementsByTagName('name')[0]?.textContent || '(unnamed)';
                issues.push({ severity: 'warning', line: null, col: null,
                    message: `<string> "${n}" is missing 'size' attribute — address calculation will be inaccurate` });
            }
        });

        /* ints must have valid sizes */
        Array.from(xmlDoc.getElementsByTagName('int')).forEach(el => {
            const sAttr = el.getAttribute('size');
            if (sAttr !== null) {
                const s = parseInt(sAttr);
                if (![1, 2, 4, 8].includes(s)) {
                    const n = el.getElementsByTagName('name')[0]?.textContent || '(unnamed)';
                    issues.push({ severity: 'error', line: null, col: null,
                        message: `<int> "${n}" has invalid size="${sAttr}" — must be 1, 2, 4, or 8` });
                }
            }
        });

        /* floats must have valid sizes */
        Array.from(xmlDoc.getElementsByTagName('float')).forEach(el => {
            const sAttr = el.getAttribute('size');
            if (sAttr !== null) {
                const s = parseInt(sAttr);
                if (![4, 8].includes(s)) {
                    const n = el.getElementsByTagName('name')[0]?.textContent || '(unnamed)';
                    issues.push({ severity: 'warning', line: null, col: null,
                        message: `<float> "${n}" has unusual size="${sAttr}" — expected 4 or 8` });
                }
            }
        });

        /* maps: checkbox hints need exactly 2 relations */
        Array.from(xmlDoc.getElementsByTagName('hints')).forEach(hints => {
            const chk = hints.getElementsByTagName('checkbox')[0];
            if (!chk) { return; }

            const parentEl = hints.parentElement;
            const mapEl    = parentEl ? parentEl.getElementsByTagName('map')[0] : null;
            if (!mapEl) {
                issues.push({ severity: 'error', line: null, col: null,
                    message: 'checkbox hint requires a <map> sibling element with exactly 2 relations' });
            } else {
                const rels = mapEl.getElementsByTagName('relation').length;
                if (rels !== 2) {
                    issues.push({ severity: 'error', line: null, col: null,
                        message: `checkbox hint map has ${rels} relation(s) — exactly 2 required` });
                }
            }
        });

        /* unknown element tags (warn but don't error) */
        const allTags = xmlDoc.getElementsByTagName('*');
        const unknown = new Set();
        for (let el of allTags) {
            if (!KNOWN_CDI_TAGS.has(el.tagName)) { unknown.add(el.tagName); }
        }
        unknown.forEach(tag => {
            issues.push({ severity: 'warning', line: null, col: null,
                message: `Unknown element <${tag}> — not part of the CDI schema` });
        });

        /* min/max consistency on int/float */
        [...xmlDoc.getElementsByTagName('int'),
         ...xmlDoc.getElementsByTagName('float')].forEach(el => {

            const minEl = el.getElementsByTagName('min')[0];
            const maxEl = el.getElementsByTagName('max')[0];

            if (minEl && maxEl) {
                const mn = parseFloat(minEl.textContent);
                const mx = parseFloat(maxEl.textContent);
                if (!isNaN(mn) && !isNaN(mx) && mn > mx) {
                    const n = el.getElementsByTagName('name')[0]?.textContent || '(unnamed)';
                    issues.push({ severity: 'warning', line: null, col: null,
                        message: `"${n}": <min> (${mn}) is greater than <max> (${mx})` });
                }
            }
        });

        /* ===== LAYER 3: semantic / best-practice hints ==================== */

        /* identification */
        if (!root.getElementsByTagName('identification')[0]) {
            issues.push({ severity: 'warning', line: null, col: null,
                message: 'No <identification> element — manufacturer/model info will be missing from node' });
        } else {
            const id = root.getElementsByTagName('identification')[0];
            if (!id.getElementsByTagName('manufacturer')[0]?.textContent.trim()) {
                issues.push({ severity: 'info', line: null, col: null,
                    message: '<identification> is missing or has an empty <manufacturer>' });
            }
            if (!id.getElementsByTagName('model')[0]?.textContent.trim()) {
                issues.push({ severity: 'info', line: null, col: null,
                    message: '<identification> is missing or has an empty <model>' });
            }
        }

        /* ACDI */
        if (!root.getElementsByTagName('acdi')[0]) {
            issues.push({ severity: 'info', line: null, col: null,
                message: 'No <acdi/> tag — ACDI protocol will not be advertised by this node' });
        }

        /* space 253 (0xFD) — user-accessible name/description */
        const hasSpace253 = segments.some(s => parseInt(s.getAttribute('space')) === 253);
        if (!hasSpace253) {
            issues.push({ severity: 'info', line: null, col: null,
                message: 'No segment in space 0xFD (253) — user name and description will not be configurable via CDI' });
        }

/* event IDs: warn if a segment in space 0xFE (all memory) has eventids */
        segments.filter(s => parseInt(s.getAttribute('space')) === 254).forEach(seg => {
            if (seg.getElementsByTagName('eventid').length > 0) {
                issues.push({ severity: 'info', line: null, col: null,
                    message: 'Event IDs found in space 0xFE (All Memory). This is unusual — event IDs are typically stored in the config space (0xFD).' });
            }
        });

        /* empty segments */
        segments.forEach((seg, i) => {
            const hasLeaf = KNOWN_LEAF_TAGS.has(seg.children[0]?.tagName) ||
                            seg.getElementsByTagName('group').length > 0  ||
                            Array.from(seg.children).some(c => KNOWN_LEAF_TAGS.has(c.tagName));
            if (!hasLeaf) {
                issues.push({ severity: 'info', line: null, col: null,
                    message: `<segment> #${i + 1} (space ${seg.getAttribute('space')}) appears to have no configuration items` });
            }
        });

        return issues;
    }

    /* ----------------------------------------------------------------------- */
    window.CdiValidator = { validate };

}());
