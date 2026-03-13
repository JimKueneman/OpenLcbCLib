/* =========================================================================
 * cdi_renderer.js  —  OpenLCB CDI XML rendering engine
 *
 * Exposes a single global:
 *   window.CdiRenderer.render(xmlString, targetElement, opts)
 *
 * opts: { showAddressInfo: bool }
 *
 * Returns: { ok: bool, doc: XMLDocument | null }
 *   ok = false if XML is not well-formed (targetElement is left empty)
 * ========================================================================= */

(function () {

    /* -----------------------------------------------------------------------
     * render()  —  parse XML and populate targetElement with CDI DOM
     * --------------------------------------------------------------------- */

    function render(xmlString, targetElement, opts) {

        opts = opts || {};

        const showAddressInfo = !!opts.showAddressInfo;
        let   _addr           = 0;   /* running address — local to this render call */

        targetElement.innerHTML = '';

        /* --- parse -------------------------------------------------------- */
        const parser = new DOMParser();
        const xmlDoc = parser.parseFromString(xmlString, 'text/xml');

        if (xmlDoc.getElementsByTagName('parsererror').length > 0) {
            return { ok: false, doc: null };
        }

        const cdiRoot = xmlDoc.getElementsByTagName('cdi')[0];
        if (!cdiRoot) { return { ok: false, doc: xmlDoc }; }

        /* --- identification panel ----------------------------------------- */
        const identification = cdiRoot.getElementsByTagName('identification')[0];
        if (identification) {
            targetElement.appendChild(_renderIdentification(identification));
        }

        /* --- ACDI status -------------------------------------------------- */
        const acdiDiv = document.createElement('div');
        acdiDiv.className = 'cdi-acdi-status';
        acdiDiv.textContent = cdiRoot.getElementsByTagName('acdi')[0]
            ? 'Abbreviated CDI (ACDI) protocol is available.'
            : 'Abbreviated CDI (ACDI) protocol is not available.';
        targetElement.appendChild(acdiDiv);

        /* --- segments ----------------------------------------------------- */
        const segments = cdiRoot.getElementsByTagName('segment');
        for (let i = 0; i < segments.length; i++) {
            targetElement.appendChild(_renderSegment(segments[i]));
        }

        return { ok: true, doc: xmlDoc };

        /* =================================================================
         * INNER HELPERS  (closures share showAddressInfo and _addr)
         * =============================================================== */

        function _getText(node, tagName) {
            const el = node.getElementsByTagName(tagName)[0];
            return el ? el.textContent : '';
        }

        function _getNameAndDesc(node) {

            let name = '';
            let desc = '';

            for (let i = 0; i < node.children.length; i++) {
                if (node.children[i].tagName === 'name')        { name = node.children[i].textContent; }
                if (node.children[i].tagName === 'description') { desc = node.children[i].textContent; }
            }

            return { name, desc };
        }

        /* ------------------------------------------------------------------ */
        function _renderIdentification(node) {

            const div = document.createElement('div');
            div.className = 'cdi-identification';

            const mfr  = _getText(node, 'manufacturer');
            const mdl  = _getText(node, 'model');
            const hwv  = _getText(node, 'hardwareVersion');
            const swv  = _getText(node, 'softwareVersion');

            div.innerHTML =
                '<h3>Device Identification</h3>' +
                '<table>' +
                  (mfr ? `<tr><td>Manufacturer</td><td>${mfr}</td></tr>` : '') +
                  (mdl ? `<tr><td>Model</td><td>${mdl}</td></tr>` : '') +
                  (hwv ? `<tr><td>Hardware Version</td><td>${hwv}</td></tr>` : '') +
                  (swv ? `<tr><td>Software Version</td><td>${swv}</td></tr>` : '') +
                '</table>';

            return div;
        }

        /* ------------------------------------------------------------------ */
        function _renderSegment(node) {

            const wrapper = document.createElement('div');
            wrapper.className = 'cdi-segment';

            const { name, desc } = _getNameAndDesc(node);
            const space  = node.getAttribute('space')  || '?';
            const origin = node.getAttribute('origin');

            /* Reset running address to the segment's origin */
            _addr = parseInt(origin) || 0;

            const header = document.createElement('div');
            header.className = 'cdi-segment-header';
            header.innerHTML =
                `<span>${name || 'Segment'} ` +
                `<small>(Space: ${space}${origin ? ', Origin: ' + origin : ''})</small></span>`;
            header.addEventListener('click', () => wrapper.classList.toggle('collapsed'));

            const body = document.createElement('div');
            body.className = 'cdi-segment-body';

            if (desc) {
                const p = document.createElement('p');
                p.textContent = desc;
                body.appendChild(p);
            }

            const content = document.createElement('div');
            _processChildren(node, content);
            body.appendChild(content);

            wrapper.appendChild(header);
            wrapper.appendChild(body);
            return wrapper;
        }

        /* ------------------------------------------------------------------ */
        function _renderGroup(node) {

            const replication  = parseInt(node.getAttribute('replication')) || 1;
            const { name, desc } = _getNameAndDesc(node);
            const repNameText  = _getText(node, 'repname');
            const groupOffset  = node.getAttribute('offset');

            if (groupOffset && !isNaN(parseInt(groupOffset))) {
                _addr += parseInt(groupOffset);
            }

            /* --- replicated group (tabbed) --------------------------------- */
            if (replication > 1) {

                const wrapper = document.createElement('div');
                wrapper.className = 'cdi-tab-group';

                const info = document.createElement('div');
                info.className = 'cdi-tab-group-info';
                info.innerHTML = `<h3>${name || 'Replicated Group'}</h3>${desc ? `<p>${desc}</p>` : ''}`;
                wrapper.appendChild(info);

                const nav     = document.createElement('div');
                nav.className = 'cdi-tabs-nav';
                wrapper.appendChild(nav);

                const content = document.createElement('div');
                content.className = 'cdi-tab-content';
                wrapper.appendChild(content);

                const initialNum = parseInt(repNameText);
                const numericRep = !isNaN(initialNum) && repNameText.trim() !== '';

                for (let i = 0; i < replication; i++) {

                    const label = numericRep
                        ? (initialNum + i).toString()
                        : (repNameText ? `${repNameText} ${i + 1}` : `Item ${i + 1}`);

                    const btn = document.createElement('button');
                    btn.className   = 'cdi-tab-button' + (i === 0 ? ' active' : '');
                    btn.textContent = label;
                    btn.dataset.tabIndex = i;

                    const pane = document.createElement('div');
                    pane.className   = 'cdi-tab-pane' + (i === 0 ? ' active' : '');
                    pane.dataset.tabIndex = i;

                    const repHeader = document.createElement('h4');
                    repHeader.className = 'repname-header';
                    repHeader.textContent = label;
                    pane.appendChild(repHeader);

                    _processChildren(node, pane);
                    content.appendChild(pane);

                    btn.addEventListener('click', () => {
                        nav.querySelectorAll('.cdi-tab-button').forEach(b => b.classList.remove('active'));
                        content.querySelectorAll('.cdi-tab-pane').forEach(p => p.classList.remove('active'));
                        btn.classList.add('active');
                        pane.classList.add('active');
                    });

                    nav.appendChild(btn);
                }

                return wrapper;
            }

            /* --- plain group ---------------------------------------------- */
            const wrapper = document.createElement('div');
            wrapper.className = 'cdi-group';

            const header = document.createElement('div');
            header.className = 'cdi-group-header';
            header.innerHTML = `<span>${name || 'Group'}</span>`;
            header.addEventListener('click', () => wrapper.classList.toggle('collapsed'));

            const body = document.createElement('div');
            body.className = 'cdi-group-body';

            if (desc) {
                const p = document.createElement('p');
                p.className   = 'cdi-desc';
                p.textContent = desc;
                body.appendChild(p);
            }

            const content = document.createElement('div');
            _processChildren(node, content);
            body.appendChild(content);

            wrapper.appendChild(header);
            wrapper.appendChild(body);
            return wrapper;
        }

        /* ------------------------------------------------------------------ */
        function _processChildren(parentNode, targetDiv) {

            Array.from(parentNode.children).forEach(child => {

                const tag = child.tagName;
                if (['name', 'description', 'repname', 'hints'].includes(tag)) { return; }

                if (tag === 'group') {
                    targetDiv.appendChild(_renderGroup(child));
                } else if (['bit', 'int', 'float', 'string', 'eventid'].includes(tag)) {
                    targetDiv.appendChild(_renderLeaf(child, tag));
                }
            });
        }

        /* ------------------------------------------------------------------ */
        function _intLimits(size, isSigned) {

            if (size === 0) { return { min: 0, max: 0 }; }

            if (isSigned) {
                const p = (8 * size) - 1;
                if (p >= 53) { return { min: -Number.MAX_SAFE_INTEGER, max: Number.MAX_SAFE_INTEGER }; }
                return { min: -Math.pow(2, p), max: Math.pow(2, p) - 1 };
            }

            const p = 8 * size;
            if (p >= 53) { return { min: 0, max: Number.MAX_SAFE_INTEGER }; }
            return { min: 0, max: Math.pow(2, p) - 1 };
        }

        /* ------------------------------------------------------------------ */
        function _renderLeaf(node, type) {

            const div = document.createElement('div');
            div.className = 'cdi-item';

            const { name, desc }   = _getNameAndDesc(node);
            const elementOrigin    = node.getAttribute('origin');
            const elementOffset    = node.getAttribute('offset');
            const nodeSizeAttr     = node.getAttribute('size');
            const defaultValue     = node.getElementsByTagName('default')[0]?.textContent;
            const minTag           = node.getElementsByTagName('min')[0]?.textContent;
            const maxTag           = node.getElementsByTagName('max')[0]?.textContent;

            const labelText = name || (type.charAt(0).toUpperCase() + type.slice(1));

            if (elementOffset && !isNaN(parseInt(elementOffset))) {
                _addr += parseInt(elementOffset);
            }

            const map        = node.getElementsByTagName('map')[0];
            let inputHtml    = '';
            let labelHtml    = `<label>${labelText}</label>`;
            let extraInfo    = '';
            let warningHtml  = '';
            let calcSize     = null;

            /* ---- map (select) -------------------------------------------- */
            if (map) {

                calcSize = (!isNaN(parseInt(nodeSizeAttr))) ? parseInt(nodeSizeAttr) : 0;

                let options = '';
                Array.from(map.getElementsByTagName('relation')).forEach(rel => {
                    const prop = rel.getElementsByTagName('property')[0]?.textContent || '';
                    const val  = rel.getElementsByTagName('value')[0]?.textContent   || '';
                    options   += `<option value="${val}">${val} [${prop}]</option>`;
                });
                inputHtml = `<select>${options}</select>`;
                if (defaultValue) { extraInfo = `(Default: ${defaultValue})`; }

            } else {

                /* ---- typed leaf ------------------------------------------ */
                const parsedSize = !isNaN(parseInt(nodeSizeAttr)) ? parseInt(nodeSizeAttr) : NaN;
                let   hintUsed   = false;

                switch (type) {

                    case 'string':
                        if (showAddressInfo && isNaN(parsedSize)) {
                            warningHtml = _warn('&lt;string&gt; needs a size attribute for correct address calculation');
                        }
                        calcSize = isNaN(parsedSize) ? 0 : parsedSize;
                        inputHtml = `<input type="text" value="${defaultValue || ''}" ${calcSize ? `maxlength="${calcSize}"` : ''} placeholder="Enter text...">`;
                        if (calcSize) { extraInfo = `(Max ${calcSize} chars)`; }
                        break;

                    case 'float':
                        if (showAddressInfo && isNaN(parsedSize)) {
                            warningHtml = _warn('&lt;float&gt; needs a size attribute for correct address calculation');
                        }
                        calcSize  = isNaN(parsedSize) ? 0 : parsedSize;
                        inputHtml = `<input type="number" step="any" value="${defaultValue || ''}">`;
                        if (calcSize) { extraInfo = `(${calcSize} bytes)`; }
                        break;

                    case 'int': {

                        let intSize = 1;
                        if (!isNaN(parsedSize)) {
                            if ([1, 2, 4, 8].includes(parsedSize)) {
                                intSize = parsedSize;
                            } else {
                                warningHtml += _warn(`Invalid int size ${parsedSize} — must be 1, 2, 4, or 8`);
                            }
                        }
                        calcSize = intSize;

                        const parsedMin  = parseInt(minTag);
                        const isSigned   = !isNaN(parsedMin) && parsedMin < 0;
                        const limits     = _intLimits(calcSize, isSigned);
                        let   finalMin   = (minTag   != null && !isNaN(parsedMin))   ? Math.max(parsedMin,       limits.min) : limits.min;
                        let   finalMax   = (maxTag   != null && !isNaN(parseInt(maxTag))) ? Math.min(parseInt(maxTag), limits.max) : limits.max;

                        if (finalMin > finalMax) {
                            warningHtml += _warn(`min (${minTag}) > max (${maxTag}) — setting min = max`);
                            finalMin = finalMax;
                        }

                        /* hints */
                        const hints = node.getElementsByTagName('hints')[0];
                        if (hints) {
                            const chk    = hints.getElementsByTagName('checkbox')[0];
                            const radio  = hints.getElementsByTagName('radiobutton')[0];
                            const slider = hints.getElementsByTagName('slider')[0];

                            if (chk && map) {
                                const rels = map.getElementsByTagName('relation');
                                if (rels.length === 2) {
                                    hintUsed = true;
                                    const uid = 'chk_' + Math.random().toString(36).slice(2);
                                    const offVal = rels[0].getElementsByTagName('value')[0]?.textContent || '';
                                    const offLbl = rels[0].getElementsByTagName('property')[0]?.textContent || 'Off';
                                    const onVal  = rels[1].getElementsByTagName('value')[0]?.textContent || '';
                                    const onLbl  = rels[1].getElementsByTagName('property')[0]?.textContent || 'On';
                                    inputHtml = `<input type="checkbox" id="${uid}" ${defaultValue === onVal ? 'checked' : ''}>` +
                                                `<label for="${uid}" style="font-weight:normal;margin-left:5px">${offLbl} / ${onLbl}</label>`;
                                    extraInfo = `(Unchecked: ${offVal}, Checked: ${onVal})`;
                                } else {
                                    warningHtml += _warn('checkbox hint needs exactly 2 &lt;relation&gt; entries');
                                }
                            } else if (radio && map) {
                                const rels = map.getElementsByTagName('relation');
                                if (rels.length > 0) {
                                    hintUsed = true;
                                    const grp = 'radio_' + Math.random().toString(36).slice(2);
                                    let opts = '';
                                    Array.from(rels).forEach((rel, i) => {
                                        const v   = rel.getElementsByTagName('value')[0]?.textContent || '';
                                        const lbl = rel.getElementsByTagName('property')[0]?.textContent || v;
                                        const uid = `${grp}_${i}`;
                                        opts += `<span style="display:inline-flex;align-items:center;margin-right:10px">` +
                                                `<input type="radio" id="${uid}" name="${grp}" value="${v}" ${defaultValue === v ? 'checked' : ''}>` +
                                                `<label for="${uid}" style="font-weight:normal;margin-left:3px">${lbl}</label></span>`;
                                    });
                                    inputHtml = `<div style="display:flex;flex-wrap:wrap;gap:5px">${opts}</div>`;
                                    extraInfo = '';
                                }
                            } else if (slider) {
                                hintUsed = true;
                                const uid      = 'slider_' + Math.random().toString(36).slice(2);
                                const valSpan  = 'sval_'   + Math.random().toString(36).slice(2);
                                const tick     = parseInt(slider.getAttribute('tickSpacing')) || 0;
                                const showVal  = slider.getAttribute('showValue') === 'yes';
                                const initVal  = Math.min(Math.max(parseInt(defaultValue) || finalMin, finalMin), finalMax);

                                labelHtml = `<label for="${uid}">${labelText}${showVal ? ` <span id="${valSpan}" class="slider-val">(${initVal})</span>` : ''}</label>`;
                                inputHtml = `<input type="range" id="${uid}" min="${finalMin}" max="${finalMax}" step="1" value="${initVal}">`;

                                if (tick > 0) {
                                    const dlId = 'dl_' + Math.random().toString(36).slice(2);
                                    let ticks  = `<datalist id="${dlId}">`;
                                    for (let v = finalMin; v <= finalMax; v += tick) { ticks += `<option value="${v}"></option>`; }
                                    if ((finalMax - finalMin) % tick !== 0) { ticks += `<option value="${finalMax}"></option>`; }
                                    ticks    += '</datalist>';
                                    inputHtml = inputHtml.replace('<input', `<input list="${dlId}"`) + ticks;
                                }

                                extraInfo = `(Slider: ${finalMin}–${finalMax}${tick ? ', ticks ' + tick : ''}${showVal ? ', live value' : ''})`;

                                if (showVal) {
                                    setTimeout(() => {
                                        const sl = div.querySelector('#' + uid);
                                        const sp = div.querySelector('#' + valSpan);
                                        if (sl && sp) { sl.addEventListener('input', e => { sp.textContent = '(' + e.target.value + ')'; }); }
                                    }, 0);
                                }
                            }
                        }

                        if (!hintUsed) {
                            inputHtml = `<input type="number" value="${defaultValue || ''}" min="${finalMin}" max="${finalMax}">`;
                            extraInfo = `(${calcSize} bytes, ${finalMin}–${finalMax}${isSigned ? ', signed' : ''})`;
                        }
                        break;
                    }

                    case 'bit':
                        calcSize  = isNaN(parsedSize) ? 0 : parsedSize;
                        inputHtml = `<input type="checkbox" ${defaultValue === '1' ? 'checked' : ''}>`;
                        if (calcSize) { extraInfo = `(${calcSize} bits)`; }
                        break;

                    case 'eventid':
                        calcSize  = showAddressInfo ? 8 : (isNaN(parsedSize) ? 8 : parsedSize);
                        inputHtml = `<input type="text" value="${defaultValue || ''}" placeholder="00.00.00.00.00.00.00.00" style="font-family:monospace">`;
                        extraInfo = `(${calcSize} bytes, Event ID)`;
                        break;

                    default:
                        calcSize  = isNaN(parsedSize) ? 0 : parsedSize;
                        inputHtml = `<input type="text" value="${defaultValue || ''}">`;
                        if (calcSize) { extraInfo = `(${calcSize} bytes)`; }
                }
            }

            /* extra attribute info */
            if (elementOrigin) { extraInfo += ` | Origin: ${elementOrigin}`; }
            if (elementOffset) { extraInfo += ` | Offset: ${elementOffset}`; }

            /* address info */
            let addrHtml = '';
            if (showAddressInfo) {
                addrHtml = `<div class="cdi-address-info">Address: ${_addr} (0x${_addr.toString(16).toUpperCase()})</div>`;
            }

            div.innerHTML =
                labelHtml +
                (desc    ? `<div class="cdi-desc">${desc}</div>` : '') +
                warningHtml +
                addrHtml +
                `<div class="cdi-input-wrapper">${inputHtml}${extraInfo ? `<span class="cdi-extra">${extraInfo}</span>` : ''}</div>`;

            if (calcSize !== null && !isNaN(calcSize)) { _addr += calcSize; }

            return div;
        }

        /* ------------------------------------------------------------------ */
        function _warn(msg) {
            return `<div class="cdi-warning">${msg}</div>`;
        }
    }

    /* ----------------------------------------------------------------------- */

    /*
     * buildMap(xmlString)
     *
     * Walks the CDI XML and collects every leaf element's address and size.
     * Returns null on parse error, otherwise:
     *   Array of { space, origin, name, items[] }
     *   Each item: { name, path, type, addr, size, overlap? }
     */
    function buildMap(xmlString) {

        const parser = new DOMParser();
        const xmlDoc = parser.parseFromString(xmlString, 'text/xml');

        if (xmlDoc.getElementsByTagName('parsererror').length > 0) { return null; }

        const cdiRoot = xmlDoc.getElementsByTagName('cdi')[0];
        if (!cdiRoot) { return null; }

        const result  = [];
        const bySpace = {};   /* space → flat item list, for overlap detection */

        const segments = Array.from(cdiRoot.getElementsByTagName('segment'));

        segments.forEach(seg => {

            const space  = parseInt(seg.getAttribute('space'))  || 0;
            const origin = parseInt(seg.getAttribute('origin')) || 0;
            const segNameEl = Array.from(seg.children).find(c => c.tagName === 'name');
            const segName   = segNameEl ? segNameEl.textContent : '';

            let addr  = origin;
            const items = [];

            function walkNode(node, path) {

                Array.from(node.children).forEach(child => {

                    const tag = child.tagName;

                    /* skip metadata tags */
                    if (['name', 'description', 'repname', 'hints', 'map',
                         'default', 'min', 'max', 'relation', 'property',
                         'value', 'checkbox', 'radiobutton', 'slider'].includes(tag)) { return; }

                    /* apply element/group offset */
                    const off = parseInt(child.getAttribute('offset')) || 0;
                    if (off) { addr += off; }

                    const childNameEl = Array.from(child.children).find(c => c.tagName === 'name');
                    const childName   = childNameEl ? childNameEl.textContent : tag;
                    const childPath   = path ? `${path} › ${childName}` : childName;

                    if (tag === 'group') {

                        const replication = parseInt(child.getAttribute('replication')) || 1;
                        for (let r = 0; r < replication; r++) {
                            const repLabel = replication > 1 ? ` [${r + 1}]` : '';
                            walkNode(child, childPath + repLabel);
                        }

                    } else if (['int', 'float', 'string', 'eventid', 'bit'].includes(tag)) {

                        const sAttr = child.getAttribute('size');
                        const sParsed = parseInt(sAttr);
                        let size = 0;

                        switch (tag) {
                            case 'int':     size = [1, 2, 4, 8].includes(sParsed) ? sParsed : 1; break;
                            case 'float':   size = !isNaN(sParsed) ? sParsed : 4;                break;
                            case 'string':  size = !isNaN(sParsed) ? sParsed : 0;                break;
                            case 'eventid': size = 8;                                             break;
                            case 'bit':     size = !isNaN(sParsed) ? sParsed : 0;                break;
                        }

                        items.push({ name: childPath, type: tag, addr, size });
                        addr += size;
                    }
                });
            }

            walkNode(seg, segName);
            result.push({ space, origin, name: segName, items });

            if (!bySpace[space]) { bySpace[space] = []; }
            bySpace[space].push(...items);
        });

        /* detect overlaps within each address space */
        Object.values(bySpace).forEach(list => {

            const sorted = [...list].sort((a, b) => a.addr - b.addr);

            for (let i = 1; i < sorted.length; i++) {
                const prev = sorted[i - 1];
                const curr = sorted[i];
                if (prev.size > 0 && prev.addr + prev.size > curr.addr) {
                    prev.overlap = true;
                    curr.overlap = true;
                }
            }
        });

        return result;
    }

    /* ----------------------------------------------------------------------- */
    /* Shared identifier helpers — used by cdi_view.html and the test runner   */
    /* ----------------------------------------------------------------------- */

    /* Convert an arbitrary string to a valid, uppercase C identifier fragment */
    function toCName(str) {

        return str
            .toUpperCase()
            .replace(/[^A-Z0-9]+/g, '_')
            .replace(/^_+|_+$/g, '');
    }

    /* Convert an arbitrary string to a valid, lowercase C snake_case identifier */
    function toSnakeCase(str) {

        return str
            .toLowerCase()
            .replace(/[^a-z0-9]+/g, '_')
            .replace(/^_+|_+$/g, '')
            || '_field';
    }

    /* De-duplicate a C identifier within a set. If already used, appends _2, _3, etc. */
    function uniqueId(name, usedSet) {

        if (!usedSet.has(name)) { usedSet.add(name); return name; }
        let n = 2;
        while (usedSet.has(name + '_' + n)) { n++; }
        const unique = name + '_' + n;
        usedSet.add(unique);
        return unique;
    }

    /* ----------------------------------------------------------------------- */
    /* checkDefineCollisions(mapData)
     *
     * Simulates the #define name generation from _renderMapDefines() and
     * returns an array of { name, count } for any identifiers that collide.
     * Empty array = no collisions = compiles clean.
     * ----------------------------------------------------------------------- */

    function checkDefineCollisions(mapData) {

        if (!mapData) { return []; }

        const usedIds = new Set();
        const allNames = [];

        mapData.forEach(seg => {

            const baseName  = seg.name ? toCName(seg.name) : 'SPACE_' + seg.space;
            const segCName  = uniqueId(baseName, usedIds);
            const segPrefix = 'SPACE_' + seg.space + '_' + segCName;

            seg.items.forEach(item => {

                const parts   = item.name.split(' \u203a ').map(toCName);
                const defName = segPrefix + '_' + parts.join('_');
                allNames.push(defName);

                if (item.size != null) {
                    allNames.push(defName + '_SIZE');
                }
            });
        });

        const counts = {};
        allNames.forEach(n => { counts[n] = (counts[n] || 0) + 1; });
        return Object.entries(counts)
            .filter(([, c]) => c > 1)
            .map(([name, count]) => ({ name, count }));
    }

    /* ----------------------------------------------------------------------- */
    /* checkStructCollisions(mapData)
     *
     * Simulates the struct field name generation from _renderMapStruct() and
     * returns an array of { scope, name, count } for any fields that share the
     * same snake_case name within the same struct scope.
     * Empty array = no collisions = compiles clean.
     * ----------------------------------------------------------------------- */

    function checkStructCollisions(mapData) {

        if (!mapData) { return []; }

        const collisions = [];

        /* Build tree identical to _buildStructTree in cdi_view.html */
        function buildTree(items) {

            const order  = [];
            const groups = {};

            items.forEach(item => {

                const parts    = item.name.split(' \u203a ');
                const top      = parts[0];
                const repMatch = top.match(/^(.+) \[(\d+)\]$/);

                if (repMatch) {
                    const base = repMatch[1];
                    const idx  = parseInt(repMatch[2]);
                    if (!groups[base]) {
                        groups[base] = { kind: 'array', label: base, maxRep: 0, reps: {} };
                        order.push(base);
                    }
                    const g = groups[base];
                    g.maxRep = Math.max(g.maxRep, idx);
                    if (!g.reps[idx]) { g.reps[idx] = []; }
                    g.reps[idx].push({ ...item, name: parts.slice(1).join(' \u203a ') || top });

                } else if (parts.length > 1) {
                    if (!groups[top]) {
                        groups[top] = { kind: 'group', label: top, items: [] };
                        order.push(top);
                    }
                    groups[top].items.push({ ...item, name: parts.slice(1).join(' \u203a ') });

                } else {
                    order.push(top);
                    groups[top] = { kind: 'field', item };
                }
            });

            return order.map(key => {

                const g = groups[key];

                if (g.kind === 'array') {
                    const repKeys = Object.keys(g.reps).map(Number).sort((a, b) => a - b);
                    const rep1    = g.reps[repKeys[0]];
                    return { kind: 'array', label: g.label, children: buildTree(rep1) };
                }

                if (g.kind === 'group') {
                    return { kind: 'group', label: g.label, children: buildTree(g.items) };
                }

                return { kind: 'field', item: g.item };
            });
        }

        /* Walk tree and check for duplicate field names at each scope level */
        function checkScope(nodes, scopePath) {

            const namesAtThisLevel = {};

            for (const node of nodes) {

                let fieldName;
                if (node.kind === 'field') {
                    fieldName = toSnakeCase(node.item.name.split(' \u203a ').pop() || node.item.name);
                } else {
                    fieldName = toSnakeCase(node.label);
                }

                namesAtThisLevel[fieldName] = (namesAtThisLevel[fieldName] || 0) + 1;

                /* recurse into children */
                if (node.children) {
                    checkScope(node.children, scopePath + '.' + fieldName);
                }
            }

            Object.entries(namesAtThisLevel).forEach(([name, count]) => {

                if (count > 1) {
                    collisions.push({ scope: scopePath, name, count });
                }
            });
        }

        mapData.forEach(seg => {

            const scopeName = seg.name ? toSnakeCase(seg.name) : 'space_' + seg.space;
            const tree = buildTree(seg.items);
            checkScope(tree, scopeName);
        });

        return collisions;
    }

    /* ----------------------------------------------------------------------- */
    window.CdiRenderer = {
        render, buildMap, toCName, toSnakeCase, uniqueId,
        checkDefineCollisions, checkStructCollisions
    };

}());
