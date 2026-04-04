'use strict';

/* ======================================================================
 * enter_bootloader_from_app.js
 *
 * Panel: "Enter Bootloader from Application"
 *
 * Waterfall sequence diagram — two vertical lifelines with activity
 * boxes and bounce arrows, time flowing downward.
 *
 * Arrow `from`/`to` values:
 *   'app'  — Application lifeline (left)
 *   'bl'   — Bootloader lifeline (right)
 *   'bus'  — CAN bus / CT column (far right edge)
 *
 * Expose:  renderPanel_EnterFromApp(container)
 * ====================================================================== */

/* ── LAYOUT ─────────────────────────────────────────────────────────── */
const EL = {
  totalW:   590,

  /* Lifeline centres */
  appCX:    120,
  blCX:     390,
  busCX:    560,   /* CAN BUS / CT column — narrow, right edge */

  /* Activity box geometry */
  boxW:     210,
  boxH:      72,
  boxH_lg:  100,

  /* Header band */
  hdrH:      50,
  appHdrW:  200,
  blHdrW:   200,
  busHdrW:   90,

  /* Vertical rhythm */
  stepH:    108,
  topPad:    28,

  /* Divider bar */
  divH:      40,
};

/* Y of step i, summing divider heights for any dividers before it */
function elStepY(i, steps) {
  let y = EL.hdrH + EL.topPad;
  for (let k = 0; k < i; k++) {
    y += steps[k].type === 'divider' ? EL.divH : (steps[k].h || EL.stepH);
  }
  return y;
}

/* ── STEPS ───────────────────────────────────────────────────────────
 * type:
 *   'app'     — activity box on App lifeline
 *   'bl'      — activity box on Bootloader lifeline
 *   'arrow'   — horizontal message (from/to: 'app' | 'bl' | 'bus')
 *   'divider' — full-width reset banner
 * ──────────────────────────────────────────────────────────────────── */
const EL_STEPS = [
  {
    type:     'app',
    h:        EL.stepH,
    label:    'Running — Normal Operation',
    sublabel: 'Serving PIP / SNIP / Event transport',
    desc:     'The application is running normally as a fully logged-in OpenLCB node. It responds to standard protocol messages (PIP, SNIP, Verify Node ID, Event Transport) and has no knowledge that a firmware upgrade is about to be requested.',
  },
  {
    type:     'arrow',
    h:        EL.stepH,
    from:     'bus',
    to:       'app',
    label:    '← Freeze datagram  (space = 0xEF FIRMWARE)',
    color:    '#58a6ff',
    desc:     'A Configuration Tool (CT) on the CAN bus sends a Freeze datagram addressed to the application node.\n\nThe datagram payload specifies address space 0xEF (CONFIG_MEM_SPACE_FIRMWARE). The library\'s ProtocolConfigMemOperationsHandler_freeze() dispatches on this space in two phases:\n\nPhase 1 (first call):\n  → Send Datagram Received OK and return\n\nPhase 2 (second call, after ACK is sent):\n  1. cleanup_before_handoff() [DI — nullable] — stop timers, disable IRQs\n  2. freeze callback — writes magic + alias to .noinit SRAM, then triggers reboot\n\nThe freeze callback is responsible for triggering the reboot — the library does not call reboot automatically.',
  },
  {
    type:     'app',
    h:        EL.stepH,
    label:    'Library: cleanup_before_handoff()  [DI — nullable]',
    sublabel: 'ProtocolConfigMemOperationsHandler_freeze()  ·  space == 0xEF  ·  phase 2 only',
    desc:     'Called automatically by the library BEFORE the freeze callback runs, but only on phase 2 (after the Datagram Received OK has been sent).\n\nStopping SysTick and clearing pending IRQs at this point is critical — if an interrupt fires between the callback writing .noinit SRAM and the reset completing, it could corrupt the magic value or execute stale handler code.\n\nNullable — platforms that re-initialise all peripherals unconditionally from reset may leave this NULL.\n\nCode path (protocol_config_mem_operations_handler.c):\n  bool _is_firmware_space = (space_info != NULL &&\n      space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE);\n  bool _phase2 = openlcb_datagram_ack_sent;\n  if (_is_firmware_space && _phase2 && _interface->cleanup_before_handoff)\n      _interface->cleanup_before_handoff();',
  },
  {
    type:     'app',
    h:        EL.boxH_lg,
    label:    'Freeze callback: Write .noinit SRAM + reboot',
    sublabel: 'bootloader_request_flag =\n  BOOTLOADER_REQUEST_MAGIC (0xB00710AD)\nbootloader_cached_alias =\n  node->alias\nDL_SYSCTL_resetDevice()',
    desc:     'The application\'s freeze callback (operations_request_freeze) is called by the library via _handle_operations_request().\n\nThe callback is responsible for the full handoff:\n\n1. Write .noinit SRAM:\n   bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC (0xB00710AD)\n     Discriminates a deliberate software-reset entry from a cold power-on.\n     Probability of a cold-boot false match: 1 in 4,294,967,296.\n\n   bootloader_cached_alias = statemachine_info->openlcb_node->alias\n     The application\'s current 12-bit CAN alias. Allows the bootloader to\n     skip the full CID7→RID negotiation and reuse the alias immediately.\n\n2. Trigger the software reset — MCU-specific call, e.g.:\n   DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_SYSRST)  — MSPM0G3507\n   NVIC_SystemReset()                               — generic Cortex-M\n\nThe library does NOT call reboot after the callback returns — the callback triggers the reset itself and does not return.',
  },
  {
    type:     'divider',
    label:    '⟳  CPU RESET  —  software-initiated  (hardware resets, .noinit survives)',
    desc:     'NVIC_SystemReset() has fired. All peripheral registers return to their power-on defaults. RAM is NOT cleared by the hardware reset — the C startup code will zero .bss, but .noinit is deliberately excluded from that initialisation by the linker script.\n\nThe magic value and cached alias are still present in .noinit SRAM. Execution restarts at the bootloader binary\'s Reset_Handler.',
  },
  {
    type:     'bl',
    h:        EL.stepH,
    label:    'initialize_hardware(request)  [DI]',
    sublabel: 'VIVT clear · CAN IRQ disable · CAN reinit',
    desc:     'Called by Bootloader_entry() after is_bootloader_requested() returns. The request parameter (BOOTLOADER_REQUESTED_BY_APP on this path) tells the implementation why the bootloader was entered.\n\nOn app drop-back the implementation must:\n  1. Clear bootloader_request_flag unconditionally (centralized .noinit cleanup -- already consumed by is_bootloader_requested())\n  2. Clear the VIVT unconditionally -- stale app handler pointers must never be called\n  3. Disable CAN interrupt (C1IE) and clear pending flag (C1IF) -- SYSTEM_Initialize() already enabled C1IE, and the VIVT is now NULL\n  4. Leave bootloader_cached_alias intact -- the library CAN state machine reads it via get_cached_alias() during INIT_PICK_ALIAS and clears it after pickup\n  5. Reinitialize the CAN peripheral for bootloader use\n  6. Enable global interrupts (VIVT is safe, CAN interrupt is clean)',
  },
  {
    type:     'arrow',
    h:        EL.stepH,
    from:     'app',
    to:       'bl',
    label:    '.noinit SRAM  →  MAGIC + alias',
    color:    '#a371f7',
    desc:     'is_bootloader_requested() reads bootloader_request_flag from .noinit SRAM.\n\nValue equals BOOTLOADER_REQUEST_MAGIC (0xB00710AD) -> this was a deliberate software-reset entry from the application, not a cold power-on. Flag is cleared centrally by initialize_hardware(), not by is_bootloader_requested().\n\nget_cached_alias() reads bootloader_cached_alias (left intact by initialize_hardware() on the app drop-back path).\n\nValue is non-zero -> the application\'s alias is available for immediate reuse. The library CAN state machine clears it after pickup. The bootloader skips the full CID7->CID6->CID5->CID4 -> 200 ms wait -> RID -> AMD negotiation sequence entirely.',
  },
  {
    type:     'bl',
    h:        EL.boxH_lg,
    label:    'Skip alias negotiation',
    sublabel: 'Cached alias reused directly\nfirmware_active = 1\nNo Freeze required from CT',
    desc:     'Because the cached alias is non-zero and the request flag matches, the bootloader:\n\n1. Reuses the cached alias — the bus already has an AMD entry for this alias from the application\'s session. No CID frames are sent.\n\n2. Sets firmware_active = 1 — the CT sent a Freeze to the application and received Datagram OK before the reset. The bootloader therefore enters active transfer mode immediately without requiring another Freeze.\n\n3. Advertises PSI_FIRMWARE_UPGRADE_ACTIVE in PIP (bit 0x000010) — not PSI_FIRMWARE_UPGRADE (0x000020).\n\n4. Proceeds directly to Send Init Complete.',
  },
  {
    type:     'arrow',
    h:        EL.stepH,
    from:     'bl',
    to:       'bus',
    label:    'Init Complete  [PSI_FIRMWARE_UPGRADE_ACTIVE] →',
    color:    '#f0883e',
    desc:     'The bootloader sends Init Complete (MTI 0x0100) on the CAN bus.\n\nPIP flags set: PSI_FIRMWARE_UPGRADE_ACTIVE (0x000010)\n  → The CT sees the node is back on the bus and already in active transfer mode\n  → The CT may begin sending firmware data datagrams immediately\n  → No second Freeze is needed\n\nThe node is now indistinguishable to the CT from a node that was cold-booted into bootloader mode and received a Freeze — the alias handshake made the transition seamless.',
  },
];

/* ======================================================================
 * RENDERING
 * ====================================================================== */

function renderPanel_EnterFromApp(container) {

  /* Total height */
  let totalH = EL.hdrH + EL.topPad;
  EL_STEPS.forEach(s => { totalH += s.type === 'divider' ? EL.divH : (s.h || EL.stepH); });
  totalH += 40;

  container.style.width  = EL.totalW + 'px';
  container.style.height = totalH    + 'px';

  /* SVG for lifelines + arrows */
  const svg = createSVG(EL.totalW, totalH);
  container.appendChild(svg);

  /* ── Lane headers ──────────────────────────────────────────────── */
  _elHeader(container, EL.appCX, EL.appHdrW, 'APPLICATION BINARY', '#58a6ff', '#0d2137');
  _elHeader(container, EL.blCX,  EL.blHdrW,  'BOOTLOADER BINARY',  '#3fb950', '#0d2b1a');
  _elBusHeader(container);

  /* ── Lifelines ─────────────────────────────────────────────────── */
  const lifeTop = EL.hdrH;
  _elLifeline(svg, EL.appCX, lifeTop, totalH - 20, '#58a6ff');
  _elLifeline(svg, EL.blCX,  lifeTop, totalH - 20, '#3fb950');
  _elLifeline(svg, EL.busCX, lifeTop, totalH - 20, '#e3b341');

  /* ── Steps ─────────────────────────────────────────────────────── */
  EL_STEPS.forEach((step, i) => {
    const y = elStepY(i, EL_STEPS);
    if      (step.type === 'divider') _elDivider(container, y, step);
    else if (step.type === 'arrow')   _elArrow(svg, y, step);
    else                              _elBox(container, y, step);
  });
}

/* ── Lane header ──────────────────────────────────────────────────── */
function _elHeader(container, cx, w, label, color, bg) {
  const el = document.createElement('div');
  el.style.cssText = `
    position:absolute; left:${cx - w/2}px; top:0; width:${w}px; height:${EL.hdrH - 6}px;
    background:${bg}; border:2px solid ${color}; border-radius:6px;
    display:flex; align-items:center; justify-content:center;
    font-size:10px; font-weight:700; letter-spacing:.12em; color:${color};
    text-transform:uppercase; font-family:'Courier New',monospace; user-select:none;
    text-align:center; padding:0 6px;
  `;
  el.textContent = label;
  container.appendChild(el);
}

function _elBusHeader(container) {
  const w = EL.busHdrW;
  const el = document.createElement('div');
  el.style.cssText = `
    position:absolute; left:${EL.busCX - w/2}px; top:0; width:${w}px; height:${EL.hdrH - 6}px;
    background:#1c1600; border:2px solid #e3b341; border-radius:6px;
    display:flex; align-items:center; justify-content:center;
    font-size:9px; font-weight:700; letter-spacing:.1em; color:#e3b341;
    text-transform:uppercase; font-family:'Courier New',monospace; user-select:none;
    text-align:center; padding:0 4px;
  `;
  el.textContent = 'CAN BUS / CT';
  container.appendChild(el);
}

/* ── Lifeline ──────────────────────────────────────────────────────── */
function _elLifeline(svg, cx, y1, y2, color) {
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', cx); line.setAttribute('y1', y1);
  line.setAttribute('x2', cx); line.setAttribute('y2', y2);
  line.setAttribute('stroke', color);
  line.setAttribute('stroke-width', '1.5');
  line.setAttribute('stroke-dasharray', '6,4');
  line.setAttribute('opacity', '0.3');
  svg.appendChild(line);
}

/* ── Activity box ─────────────────────────────────────────────────── */
function _elBox(container, y, step) {
  const cx  = step.type === 'app' ? EL.appCX : EL.blCX;
  const w   = EL.boxW;
  const h   = (step.h || EL.stepH) - 20;
  const by  = y + 10;
  const col = step.type === 'app'
    ? { bg:'#0d2137', border:'#58a6ff', hbg:'#1a3c5c', hcol:'#79c0ff', htxt:'▣  APPLICATION' }
    : { bg:'#0d2b1a', border:'#3fb950', hbg:'#163b22', hcol:'#56d364', htxt:'▣  BOOTLOADER'  };

  const el = document.createElement('div');
  el.style.cssText = `
    position:absolute; left:${cx - w/2}px; top:${by}px; width:${w}px; height:${h}px;
    background:${col.bg}; border:1.5px solid ${col.border}; border-radius:6px;
    cursor:pointer; overflow:hidden; user-select:none;
    transition:filter .15s, transform .1s;
  `;
  el.onmouseenter = () => { el.style.filter = 'brightness(1.18)'; el.style.transform = 'translateY(-1px)'; };
  el.onmouseleave = () => { el.style.filter = ''; el.style.transform = ''; };

  const hdr = document.createElement('div');
  hdr.style.cssText = `background:${col.hbg}; color:${col.hcol};
    padding:4px 9px; font-size:9px; font-weight:700; letter-spacing:.1em; text-transform:uppercase;`;
  hdr.textContent = col.htxt;

  const body = document.createElement('div');
  body.style.cssText = 'padding:5px 9px;';

  const lbl = document.createElement('div');
  lbl.style.cssText = `font-size:11.5px; font-weight:700; color:#e6edf3; line-height:1.3;
    font-family:'Courier New',monospace; white-space:pre-line;`;
  lbl.textContent = step.label;
  body.appendChild(lbl);

  if (step.sublabel) {
    const sub = document.createElement('div');
    sub.style.cssText = `font-size:9px; color:#8b949e; margin-top:3px;
      font-family:'Courier New',monospace; white-space:pre-line; line-height:1.4;`;
    sub.textContent = step.sublabel;
    body.appendChild(sub);
  }

  el.appendChild(hdr);
  el.appendChild(body);
  if (step.desc) el.addEventListener('click', () => showDetailRaw(step.label, step.desc));
  container.appendChild(el);
}

/* ── Horizontal bounce arrow ──────────────────────────────────────── */
function _elArrow(svg, y, step) {
  const midY = y + (step.h || EL.stepH) / 2;

  const xMap = { app: EL.appCX, bl: EL.blCX, bus: EL.busCX };
  const fromX = xMap[step.from];
  const toX   = xMap[step.to];
  const color = step.color || '#e3b341';

  const goRight = toX > fromX;
  const marker  = color === '#58a6ff' ? 'arrow-blue'
                : color === '#3fb950' ? 'arrow-green'
                : color === '#f0883e' ? 'arrow-orange'
                : color === '#a371f7' ? 'arrow-purple'
                : 'arrow-yellow';

  /* Arrow line */
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', fromX);
  line.setAttribute('y1', midY);
  line.setAttribute('x2', toX + (goRight ? -8 : 8));
  line.setAttribute('y2', midY);
  line.setAttribute('stroke', color);
  line.setAttribute('stroke-width', '2');
  line.setAttribute('marker-end', `url(#${marker})`);
  line.style.cursor = 'pointer';
  svg.appendChild(line);

  /* Label — above the arrow, centred between endpoints */
  const lx   = (fromX + toX) / 2;
  const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
  text.setAttribute('x', lx);
  text.setAttribute('y', midY - 7);
  text.setAttribute('fill', color);
  text.setAttribute('font-size', '10');
  text.setAttribute('font-family', 'monospace');
  text.setAttribute('text-anchor', 'middle');
  text.setAttribute('font-weight', '600');
  text.style.cursor = 'pointer';
  text.textContent  = step.label;
  svg.appendChild(text);

  if (step.desc) {
    const handler = () => showDetailRaw(step.label, step.desc);
    line.addEventListener('click', handler);
    text.addEventListener('click', handler);
  }
}

/* ── Divider / reset banner ───────────────────────────────────────── */
function _elDivider(container, y, step) {
  const el = document.createElement('div');
  el.style.cssText = `
    position:absolute; left:4px; top:${y + 5}px;
    width:${EL.totalW - 8}px; height:${EL.divH - 10}px;
    background:#1a0a00; border:1.5px solid #e3b341; border-radius:5px;
    display:flex; align-items:center; justify-content:center;
    font-size:10px; font-weight:700; letter-spacing:.13em; color:#f0c040;
    text-transform:uppercase; font-family:'Courier New',monospace;
    cursor:pointer; user-select:none; text-align:center; padding:0 8px;
  `;
  el.textContent = step.label;
  if (step.desc) el.addEventListener('click', () => showDetailRaw('CPU Reset', step.desc));
  container.appendChild(el);
}
