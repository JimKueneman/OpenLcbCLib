'use strict';

/* ======================================================================
 * bootloader_diagram_shared.js
 *
 * Shared utilities for all bootloader code-path diagram panels:
 *   – Detail panel (showDetail, showDetailRaw, closeDetail)
 *   – SVG helpers  (createSVG, svgLine, svgElbow, svgPath, svgText, svgRect)
 *   – Node builders (buildFlowNode, buildDiNode)
 *   – Text helpers  (escHtml, detailSection)
 *
 * Loaded before any panel JS file.
 * ====================================================================== */

/* ── Detail panel ─────────────────────────────────────────────────────── */

function closeDetail() {
  document.getElementById('detail-panel').classList.remove('open');
  document.querySelectorAll('.active-node').forEach(el => el.classList.remove('active-node'));
}

function showDetailRaw(title, text) {
  document.querySelectorAll('.active-node').forEach(el => el.classList.remove('active-node'));
  document.getElementById('detail-title').textContent = title;
  document.getElementById('detail-body').innerHTML =
    detailSection('Description', `<p>${escHtml(text).replace(/\n/g, '<br>')}</p>`);
  document.getElementById('detail-panel').classList.add('open');
}

function showDetail(node, kind) {
  document.querySelectorAll('.active-node').forEach(el => el.classList.remove('active-node'));

  const domEl = document.getElementById((kind === 'di' ? 'di-' : 'fn-') + node.id);
  if (domEl) domEl.classList.add('active-node');

  document.getElementById('detail-title').textContent = node.label;

  let html = '';
  if (node.desc) {
    html += detailSection('Description',
      `<p>${escHtml(node.desc).replace(/\n/g, '<br>')}</p>`);
  }
  if (kind === 'di') {
    if (node.struct) html += detailSection('DI Struct',
      `<div class="detail-code">${escHtml(node.struct)}</div>`);
    if (node.sig)    html += detailSection('Function Signature',
      `<div class="detail-code">${escHtml(node.sig)}</div>`);
    if (node.result) html += detailSection('Return Value (this path)',
      `<div class="detail-code">${escHtml(node.result)}</div>`);
    html += detailSection('Null Safety', node.nullable
      ? `<span class="detail-tag nullable">NULL-safe — library checks before calling</span>`
      : `<span class="detail-tag required">REQUIRED — must not be NULL</span>`);
    if (node.bypass) html += detailSection('Path Note',
      `<span class="detail-tag notaken">Not called on this path — shown for context only.</span>`);
  }
  if (node.file) html += detailSection('Source Location',
    `<div class="detail-code">${node.file}${node.line ? ' : ' + node.line : ''}</div>`);

  document.getElementById('detail-body').innerHTML = html;
  document.getElementById('detail-panel').classList.add('open');
}

function detailSection(title, content) {
  return `<div class="detail-section">
    <div class="detail-section-title">${title}</div>
    ${content}
  </div>`;
}

function escHtml(str) {
  if (!str) return '';
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

/* ── SVG helpers ──────────────────────────────────────────────────────── */

function createSVG(w, h) {
  const svg = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
  svg.setAttribute('width', w);
  svg.setAttribute('height', h);
  svg.style.cssText = 'position:absolute;top:0;left:0;pointer-events:none;overflow:visible;';
  ['blue:#58a6ff', 'green:#3fb950', 'orange:#f0883e',
   'gray:#484f58', 'purple:#a371f7', 'teal:#2dd4bf', 'amber:#fbbf24']
    .forEach(s => {
      const [id, color] = s.split(':');
      addMarker(svg, 'arrow-' + id, color);
    });
  return svg;
}

function addMarker(svg, id, color) {
  let defs = svg.querySelector('defs');
  if (!defs) {
    defs = document.createElementNS('http://www.w3.org/2000/svg', 'defs');
    svg.prepend(defs);
  }
  const marker = document.createElementNS('http://www.w3.org/2000/svg', 'marker');
  marker.setAttribute('id', id);
  marker.setAttribute('markerWidth', '8');
  marker.setAttribute('markerHeight', '8');
  marker.setAttribute('refX', '6');
  marker.setAttribute('refY', '3');
  marker.setAttribute('orient', 'auto');
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', 'M0,0 L0,6 L8,3 z');
  path.setAttribute('fill', color);
  marker.appendChild(path);
  defs.appendChild(marker);
}

/** Vertical or horizontal straight line with optional arrowhead + label. */
function svgLine(svg, x1, y1, x2, y2, color, sw, marker, dashed, label) {
  const isVert = (x1 === x2);
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', x1);
  line.setAttribute('y1', y1);
  line.setAttribute('x2', isVert ? x2 : x2 - 8);
  line.setAttribute('y2', isVert ? y2 - 8 : y2);
  line.setAttribute('stroke', color);
  line.setAttribute('stroke-width', sw || 1.5);
  if (dashed) line.setAttribute('stroke-dasharray', dashed);
  if (marker) line.setAttribute('marker-end', `url(#${marker})`);
  svg.appendChild(line);
  if (label) {
    const t = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    t.setAttribute('x', x1 + 6);
    t.setAttribute('y', (y1 + y2) / 2 - 4);
    t.setAttribute('fill', color);
    t.setAttribute('font-size', '10');
    t.setAttribute('font-family', 'monospace');
    t.textContent = label;
    svg.appendChild(t);
  }
}

/** Elbow connector: horizontal out, vertical jog, horizontal in. */
function svgElbow(svg, x1, y1, x2, y2, color, marker, dashed) {
  const midX = x1 + (x2 - x1) / 2;
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', `M ${x1} ${y1} H ${midX} V ${y2} H ${x2 - 8}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', color);
  path.setAttribute('stroke-width', dashed ? 1.2 : 1.5);
  if (dashed) path.setAttribute('stroke-dasharray', dashed);
  if (marker) path.setAttribute('marker-end', `url(#${marker})`);
  svg.appendChild(path);
}

/** Arbitrary SVG path. */
function svgPath(svg, d, color, sw, dashed, marker) {
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', d);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', color);
  path.setAttribute('stroke-width', sw || 1.5);
  if (dashed) path.setAttribute('stroke-dasharray', dashed);
  if (marker) path.setAttribute('marker-end', `url(#${marker})`);
  svg.appendChild(path);
  return path;
}

/** SVG text element. */
function svgText(svg, x, y, text, color, fontSize, anchor, transform) {
  const t = document.createElementNS('http://www.w3.org/2000/svg', 'text');
  t.setAttribute('x', x);
  t.setAttribute('y', y);
  t.setAttribute('fill', color || '#8b949e');
  t.setAttribute('font-size', fontSize || '11');
  t.setAttribute('font-family', 'monospace');
  if (anchor) t.setAttribute('text-anchor', anchor);
  if (transform) t.setAttribute('transform', transform);
  t.textContent = text;
  svg.appendChild(t);
  return t;
}

/** SVG rect. */
function svgRect(svg, x, y, w, h, fill, stroke, rx) {
  const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
  rect.setAttribute('x', x); rect.setAttribute('y', y);
  rect.setAttribute('width', w); rect.setAttribute('height', h);
  if (fill)   rect.setAttribute('fill', fill);
  if (stroke) rect.setAttribute('stroke', stroke);
  if (rx)     rect.setAttribute('rx', rx);
  svg.appendChild(rect);
  return rect;
}

/* ── Main-flow node builder ───────────────────────────────────────────── */

const NODE_TYPE_META = {
  start:     { icon: '⬡', label: 'RESET' },
  process:   { icon: '⬜', label: 'PROCESS' },
  state:     { icon: '◉', label: 'STATE' },
  decision:  { icon: '◆', label: 'DECISION' },
  di_inline: { icon: '⬡', label: 'DI CALLOUT — inline' },
  loop:      { icon: '↻', label: 'MAIN LOOP' },
  end_wait:  { icon: '◎', label: 'WAITING' },
  app_node:  { icon: '◈', label: 'APPLICATION' },
  bl_node:   { icon: '◈', label: 'BOOTLOADER' },
};

function buildFlowNode(n, x, y, w, h) {
  const el = document.createElement('div');
  el.className = `flow-node node-type-${n.type}`;
  el.id = 'fn-' + n.id;
  el.style.cssText = `left:${x}px;top:${y}px;width:${w}px;height:${h}px;`;

  const meta = NODE_TYPE_META[n.type] || { icon: '·', label: n.type.toUpperCase() };
  const fileTag = n.file
    ? `<span style="opacity:.6;font-family:monospace;font-size:9px;">${n.file}${n.line ? ':' + n.line : ''}</span>`
    : '';

  el.innerHTML = `
    <div class="node-header">
      <span>${meta.icon} ${meta.label}</span>${fileTag}
    </div>
    <div class="node-body">
      <div class="node-label">${escHtml(n.label)}</div>
      ${n.sublabel ? `<div class="node-sublabel">${escHtml(n.sublabel)}</div>` : ''}
    </div>`;

  if (n.desc || n.file) el.addEventListener('click', () => showDetail(n, 'flow'));
  return el;
}

/* ── DI callout node builder ──────────────────────────────────────────── */

function buildDiNode(di, x, y, w, h) {
  const el = document.createElement('div');
  el.className = 'di-node' + (di.bypass ? ' bypass-node' : '');
  el.id = 'di-' + di.id;
  el.style.cssText = `left:${x}px;top:${y}px;width:${w}px;height:${h}px;`;

  const nullBadge   = di.nullable
    ? `<span class="badge badge-nullable">NULL-safe</span>`
    : `<span class="badge badge-required">REQUIRED</span>`;
  const bypassBadge = di.bypass
    ? `<span class="badge badge-notaken">NOT TAKEN</span>` : '';
  const structBadge = `<span class="badge badge-struct">${di.struct}</span>`;

  el.innerHTML = `
    <div class="di-header">
      <span>DI CALLOUT</span>
      <div class="di-header-badges">${bypassBadge}${nullBadge}${structBadge}</div>
    </div>
    <div class="di-body">
      <div class="di-label">${escHtml(di.label)}</div>
      ${di.result ? `<div class="di-result">${escHtml(di.result)}</div>` : ''}
      <div class="di-sig">${escHtml(di.sig || '')}</div>
    </div>`;

  el.addEventListener('click', () => showDetail(di, 'di'));
  return el;
}
