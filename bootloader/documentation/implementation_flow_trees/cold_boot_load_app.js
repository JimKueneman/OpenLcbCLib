'use strict';

/* ======================================================================
 * cold_boot_load_app.js
 *
 * Panel: "Cold Boot — Load Application"
 *
 * Traces the path from hardware reset when a VALID application image is
 * present: checksum passes, jump_to_application() is called, and the
 * application takes over.
 *
 * Shared utilities are in bootloader_diagram_shared.js (loaded first).
 *
 * Expose:  renderPanel_ColdBootLoadApp(container)
 * ====================================================================== */

/* ── LAYOUT CONSTANTS ───────────────────────────────────────────────── */
const CL = {
  flowX:      40,
  flowW:     500,
  diX:       610,
  diW:       390,
  rowH:      118,
  nodeH:      82,
  nodeH_lg:  100,
  diNodeH:    86,
  diNodeH_lg: 100,
  topPad:     20,
  totalW:   1040,
};

/* ── MAIN FLOW NODES ────────────────────────────────────────────────── */
const CL_MAIN_NODES = [
  {
    id:       'cl_reset',
    type:     'start',
    label:    'CPU RESET',
    sublabel: 'Hardware reset vector — PC = 0x00000000',
    h:        CL.nodeH,
    desc:     'The MCU starts from its hardware reset vector. For Cortex-M devices the initial stack pointer is loaded from address 0x00000000 and the reset handler address from 0x00000004. Execution branches to the reset handler.',
  },
  {
    id:       'cl_main',
    type:     'process',
    label:    'main()',
    sublabel: 'Wire DI tables  ·  bootloader_main.c',
    h:        CL.nodeH,
    desc:     'Application entry point. Creates the two constant DI structs:\n\n• bootloader_can_driver_t — read_received_frame, try_send_frame, get_cached_alias\n• bootloader_openlcb_driver_t — all hardware + lifecycle hooks\n\nEach field is a function pointer to a platform-specific driver. These structs are the ONLY coupling between the bootloader library and your hardware.',
    file:     'bootloader_main.c',
  },
  {
    id:       'cl_entry',
    type:     'process',
    label:    'Bootloader_entry()',
    sublabel: 'bootloader.c : 172',
    h:        CL.nodeH,
    desc:     'Top-level entry point called by main(). Calls is_bootloader_requested() first to determine why the bootloader was entered, then passes the result to initialize_hardware(request). Finally calls Bootloader_init(). If jump_to_application() succeeds inside Bootloader_init(), this function never returns to its caller.',
    file:     'bootloader.c',
    line:     169,
  },
  {
    id:       'cl_init_hw',
    type:     'di_inline',
    label:    'openlcb_driver→initialize_hardware(request)',
    sublabel: 'DI CALLOUT  ·  called after is_bootloader_requested()',
    h:        CL.nodeH,
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    sig:      'void (*initialize_hardware)(bootloader_request_t request)',
    desc:     'Platform must perform hardware initialization. The request parameter tells the implementation why the bootloader was entered (NOT_REQUESTED on this cold-boot path).\n\nThe implementation must:\n  1. Clear bootloader_request_flag unconditionally (centralized .noinit cleanup -- already consumed by is_bootloader_requested())\n  2. Clear the VIVT unconditionally -- prevents stale handler pointers from being called regardless of reset reason\n  3. If request != BOOTLOADER_REQUESTED_BY_APP: clear bootloader_cached_alias -- stale .noinit garbage would cause the CAN state machine to skip CID negotiation with a bogus alias\n  4. If request == BOOTLOADER_REQUESTED_BY_APP: disable CAN interrupt and clear pending flag (not applicable on this path)\n  5. Initialize CAN peripheral for bootloader use\n  6. Enable global interrupts (VIVT is safe, CAN interrupt is clean)\n\nOn this path all initialized peripherals will be torn down by cleanup_before_handoff() and never used for messaging -- the bootloader exits via jump_to_application() before any CAN frames are sent.',
  },
  {
    id:       'cl_bl_init',
    type:     'process',
    label:    'Bootloader_init()',
    sublabel: 'bootloader.c : 111',
    h:        CL.nodeH,
    desc:     'Core initialization. On the "valid app" path:\n\n1. is_bootloader_requested() → NOT_REQUESTED\n2. set_status_led(REQUEST, false) — LED stays off\n3. _check_application_checksum() → true  (or csum_ok=true when NO_CHECKSUM is defined)\n4. set_status_led(CSUM_ERROR, false) — LED stays off\n5. get_flash_boundaries() → read flash_min\n6. Check: ((uint32_t*)flash_min)[0] != 0xFFFFFFFF — app present\n7. cleanup_before_handoff() [nullable] — stop SysTick, disable + clear all IRQs\n8. jump_to_application() — hands off to app; never returns on success\n\nThe flash vector check (step 6) is done by the library unconditionally — it is true for all MCUs that erase flash to 0xFF. This ensures cleanup_before_handoff() is never called when flash is blank, which would leave CAN hardware torn down with no re-init path.\n\nIf flash is blank (0xFFFFFFFF) execution falls through directly to bootloader mode without touching hardware.',
    file:     'bootloader.c',
    line:     111,
  },
  {
    id:       'cl_bl_req',
    type:     'di_inline',
    label:    'openlcb_driver→is_bootloader_requested()',
    sublabel: 'DI CALLOUT  ·  returns BOOTLOADER_NOT_REQUESTED',
    h:        CL.nodeH_lg,
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    result:   '→ BOOTLOADER_NOT_REQUESTED',
    sig:      'bootloader_request_t (*is_bootloader_requested)(void)',
    desc:     'Determines why the bootloader is running. Checks in order:\n\n1. bootloader_request_flag (.noinit SRAM) -- not BOOTLOADER_REQUEST_MAGIC on cold boot\n   (Flag is cleared centrally by initialize_hardware(), not here)\n2. Hardware button / GPIO -- not held\n3. -> Returns BOOTLOADER_NOT_REQUESTED\n\nWith NOT_REQUESTED, Bootloader_init() proceeds to validate the application checksum.',
  },
  {
    id:       'cl_dec_req',
    type:     'decision',
    label:    'Bootloader Requested?',
    sublabel: 'Cold boot path: NOT_REQUESTED  →  check checksum',
    h:        CL.nodeH,
    desc:     'Branches on is_bootloader_requested() return value.\n\nNOT_REQUESTED (this path → downward):\n  → Check application checksum\n\nREQUESTED_BY_APP or REQUESTED_BY_BUTTON (bypass — not this path):\n  → Skip checksum, enter bootloader mode\n  → See "Enter Bootloader from App" or button-entry diagrams',
  },
  {
    id:       'cl_csum',
    type:     'process',
    label:    '_check_application_checksum()',
    sublabel: 'bootloader.c : 53',
    h:        CL.nodeH,
    desc:     'Validates the application firmware image in two passes:\n\n1. get_flash_boundaries() → flash_min, flash_max, app_header ptr\n2. memcpy(app_header → RAM) — avoids flash access during compute\n3. Validate: app_size ≤ (flash_max − flash_min)\n4. compute_checksum(flash_min, pre_size) → compare header.checksum_pre[]\n5. compute_checksum(app_header + sizeof, post_size) → compare header.checksum_post[]\n\nOn this path, all checks pass → returns true.',
    file:     'bootloader.c',
    line:     53,
  },
  {
    id:       'cl_dec_csum',
    type:     'decision',
    label:    'Checksum Valid?',
    sublabel: 'Valid app image → YES  →  jump to application',
    h:        CL.nodeH,
    desc:     'Branches on _check_application_checksum() return value.\n\nYES — checksum valid (this path → downward):\n  • set_status_led(CSUM_ERROR, false) — LED stays off\n  • Call jump_to_application() DI\n  • On success: bootloader never regains control\n\nNO — checksum invalid (bypass — not this path):\n  • set_status_led(CSUM_ERROR, true) — CSUM ERROR LED on\n  • Fall through to bootloader mode\n  • See "Cold Boot, No App Image" diagram',
  },
  {
    id:       'cl_cleanup',
    type:     'di_inline',
    label:    'openlcb_driver→cleanup_before_handoff()',
    sublabel: 'DI CALLOUT  ·  nullable — stop SysTick + pending IRQs before jump',
    h:        CL.nodeH_lg,
    struct:   'bootloader_openlcb_driver_t',
    nullable: true,
    sig:      'void (*cleanup_before_handoff)(void)',
    desc:     'Called by the library immediately before jump_to_application(), but ONLY after confirming the application is present (flash_min[0] != 0xFFFFFFFF). The library owns this decision — driver implementers do not need to think about when to call it.\n\nPointer is checked for NULL before calling — implementers that do not need teardown may leave it NULL.\n\nWhy this matters — the timer interrupt trap:\nThe bootloader\'s initialize_hardware() armed a periodic timer (e.g. SysTick on Cortex-M) for the 100ms tick. That timer keeps firing during and after the handoff. If an interrupt fires in the window between the application\'s Reset_Handler entry and its interrupt table being armed, the CPU vectors through a stale or uninitialised handler — causing silent memory corruption or a fault. This is a very common embedded bootloader bug across all MCU families.\n\nWhat to implement is entirely platform-specific — only you know what initialize_hardware() started. The implementer must:\n  1. Stop any timers started by initialize_hardware()\n  2. Disable and clear any pending peripheral interrupts\n  3. Reset any peripherals that must start in a known state for the application\n\n── EXAMPLE ONLY — one possible Cortex-M0+ implementation ──\n(Your MCU, RTOS, and peripheral set will differ)\n\n  SysTick->CTRL = 0;           // stop SysTick\n  SysTick->LOAD = 0;           // clear reload register\n  SysTick->VAL  = 0;           // clear current-value register\n  NVIC->ICER[0] = 0xFFFFFFFF;  // disable all IRQs  (M0+ has 1 NVIC bank)\n  NVIC->ICPR[0] = 0xFFFFFFFF;  // clear all pending IRQs\n  mcan_peripheral_reset(CAN);  // reset CAN peripheral to power-on state\n\nHigher-end Cortex-M cores (M3/M4/M7) have up to 16 NVIC banks — loop\nover all ICER[]/ICPR[] registers. AVR, dsPIC, PIC32, RISC-V all have\ntheir own interrupt-disable mechanisms.',
  },
  {
    id:       'cl_jump',
    type:     'di_inline',
    label:    'openlcb_driver→jump_to_application()',
    sublabel: 'DI CALLOUT  ·  TAKEN — does not return on success',
    h:        CL.nodeH_lg,
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    sig:      'void (*jump_to_application)(void)',
    desc:     'Hands control to the application image. SysTick and all pending IRQs were already stopped by cleanup_before_handoff(), which the library called immediately before this.\n\nThe implementation must call __disable_irq() immediately before the jump — a final guard against any IRQ that races in between cleanup and the actual branch.\n\nThe library has already verified that flash_min[0] != 0xFFFFFFFF before reaching this call, so the driver implementation does NOT need to re-check for erased flash.\n\nCortex-M implementation:\n1. __disable_irq()\n2. SCB->VTOR = APP_FLASH_START\n3. __set_MSP(app_vectors[0])   — load application initial stack pointer\n4. ((void(*)(void))app_vectors[1])()  — branch to application Reset_Handler\n\nOffset +4 in the vector table is always the Reset_Handler address — NOT main(). The application therefore runs its full C startup sequence:\n  copy .data from flash → RAM\n  zero .bss  (but NOT .noinit — the linker script excludes it)\n  call SystemInit()  — clocks, PLLs, etc. re-configured by the application\n  call main()\n\nThis function does not return on success. If the vector table is corrupt (thumb-bit not set, address misaligned) the CPU will fault — the library\'s while(1) safety loop prevents runaway execution.',
  },
  {
    id:       'cl_app',
    type:     'state',
    label:    'Application Running',
    sublabel: 'Bootloader has no further involvement  ·  CPU is in application code',
    h:        CL.nodeH,
    desc:     'The application is now in full control of the CPU. The bootloader will not regain control unless the application deliberately triggers a reset into bootloader mode (writing BOOTLOADER_REQUEST_MAGIC to .noinit SRAM before calling NVIC_SystemReset()).\n\nThe bootloader\'s stack frame, DI structs, and variables are overwritten by the application\'s startup code.',
  },
];

/* ── DI CALLOUT NODES ───────────────────────────────────────────────── */
const CL_DI_NODES = [
  {
    id:       'cl_di_led_req',
    attachTo: 'cl_bl_req',
    label:    'set_status_led(REQUEST, false)',
    struct:   'bootloader_openlcb_driver_t',
    nullable: true,
    h:        CL.diNodeH,
    result:   'BOOTLOADER_LED_REQUEST — LED off (not requested)',
    sig:      'void (*)(bootloader_led_enum led, bool state)',
    desc:     'Called immediately after is_bootloader_requested() with state=false (not requested).\n\nMay be NULL — library checks before calling. If implemented, the REQUEST LED should remain off (or flash briefly then go off).\n\nOn other paths (REQUESTED_BY_APP or REQUESTED_BY_BUTTON), state=true and the LED illuminates to signal intentional bootloader entry.',
  },
  {
    id:       'cl_di_flash_bounds',
    attachTo: 'cl_csum',
    label:    'get_flash_boundaries()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        CL.diNodeH,
    sig:      'void (*)(const void **flash_min, const void **flash_max, const bootloader_app_header_t **app_header)',
    desc:     'Return the application flash region boundaries.\n\n• flash_min — start address of the app image (e.g. 0x00004000 on MSPM0G3507)\n• flash_max — exclusive end address\n• app_header — pointer to the fixed offset where bootloader_app_header_t is linked into the app binary\n\nTypically these are linker-script symbols exported as constants.',
  },
  {
    id:       'cl_di_compute_csum',
    attachTo: 'cl_csum',
    label:    'compute_checksum()  ×2 passes',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        CL.diNodeH,
    sig:      'void (*)(const void *data, uint32_t size, uint32_t *checksum)',
    desc:     'Called twice by _check_application_checksum():\n\n  Pass 1 (pre): data=flash_min, size=(app_header − flash_min)\n    Result compared with header.checksum_pre[4]\n\n  Pass 2 (post): data=app_header+sizeof(header), size=(app_size − pre_size − sizeof(header))\n    Result compared with header.checksum_post[4]\n\nBoth passes must match their stored values for the function to return true.\n\nThe bootloader library provides bootloader_crc_compute() as a default. You may substitute a hardware CRC accelerator.',
  },
  {
    id:       'cl_di_led_csum',
    attachTo: 'cl_dec_csum',
    label:    'set_status_led(CSUM_ERROR, false)',
    struct:   'bootloader_openlcb_driver_t',
    nullable: true,
    h:        CL.diNodeH,
    result:   'BOOTLOADER_LED_CSUM_ERROR = 8  ·  LED off (checksum OK)',
    sig:      'void (*)(bootloader_led_enum led, bool state)',
    desc:     'Called with state=false immediately after _check_application_checksum() returns true.\n\nCode: openlcb_driver->set_status_led(BOOTLOADER_LED_CSUM_ERROR, !csum_ok)\n  → !true → false → LED off\n\nMay be NULL — library checks before calling.\n\nContrast with the "No App" path where csum_ok=false → LED on.',
  },
  {
    id:       'cl_di_bl_mode_bypass',
    attachTo: 'cl_dec_csum',
    label:    'Bootloader Mode  (checksum fail path)',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    bypass:   true,
    h:        CL.diNodeH,
    sig:      '— falls through inside Bootloader_init() —',
    desc:     'NOT TAKEN on this path — checksum is valid.\n\nIf the checksum had failed, _check_application_checksum() would have returned false, set_status_led(CSUM_ERROR, true) would be called, and execution would fall through to the memset(bootloader_state) and CAN SM init — entering bootloader mode.\n\nSee the "Cold Boot, No App Image" diagram for that path.',
  },
];

/* ── "IF JUMP RETURNS" arc data ─────────────────────────────────────── */
const CL_JUMP_RETURN_NOTE = {
  label: '↩ fault/corrupt vector — bootloader mode',
  body:  'jump_to_application() should not return — the library checked flash_min[0] != 0xFFFFFFFF before calling it.\n\nThe only way it can return is if the vector table is present but corrupt (thumb-bit not set, reset handler address misaligned, or CPU fault during the branch). In that case execution falls through inside Bootloader_init() to:\n  memset(&bootloader_state, 0, sizeof(bootloader_state))\n  BootloaderCanSM_init(can_driver, openlcb_driver)\n  BootloaderOpenlcbSM_init(openlcb_driver)\n\nThe bootloader then enters its infinite loop and joins the CAN bus as a normal node — starting alias negotiation from scratch with firmware_active=0.\n\nNote: set_status_led(CSUM_ERROR, false) was called before jump_to_application(), so the CSUM ERROR LED will NOT be illuminated. The node will appear healthy — check UART debug output.',
};

/* ======================================================================
 * RENDERING
 * ====================================================================== */

function renderPanel_ColdBootLoadApp(container) {
  /* Pre-compute row indices */
  CL_MAIN_NODES.forEach((n, i) => { n._row = i; });

  /* Pre-compute DI Y positions */
  let diRunningY    = CL.topPad;
  let prevParentRow = -1;
  CL_DI_NODES.forEach(di => {
    const p = CL_MAIN_NODES.find(n => n.id === di.attachTo);
    if (!p) return;
    if (p._row !== prevParentRow) {
      diRunningY    = Math.max(diRunningY, clFlowY(p._row));
      prevParentRow = p._row;
    }
    di._y = diRunningY;
    diRunningY += (di.h || CL.diNodeH) + 8;
  });

  /* Total height */
  const lastMain = CL_MAIN_NODES[CL_MAIN_NODES.length - 1];
  const mainH    = clFlowY(lastMain._row) + (lastMain.h || CL.nodeH) + CL.topPad;
  const totalH   = Math.max(mainH, diRunningY + 60);

  container.style.width  = CL.totalW + 'px';
  container.style.height = totalH   + 'px';

  /* SVG overlay */
  const svg = createSVG(CL.totalW, totalH);
  container.appendChild(svg);

  /* Render main flow nodes */
  CL_MAIN_NODES.forEach(n => {
    const y  = clFlowY(n._row);
    const h  = n.h || CL.nodeH;
    const el = buildFlowNode(n, CL.flowX, y, CL.flowW, h);
    container.appendChild(el);
    n._top    = y;
    n._bottom = y + h;
    n._cx     = CL.flowX + CL.flowW / 2;
    n._ry     = y + h / 2;
    n._rx     = CL.flowX + CL.flowW;
    n._lx     = CL.flowX;
  });

  /* Render DI callout nodes */
  CL_DI_NODES.forEach(di => {
    const h  = di.h || CL.diNodeH;
    const el = buildDiNode(di, CL.diX, di._y, CL.diW, h);
    container.appendChild(el);
    di._cy = di._y + h / 2;
    di._lx = CL.diX;
  });

  clDrawConnections(svg);
}

/* ── Position helper ──────────────────────────────────────────────────── */
function clFlowY(rowIdx) {
  return CL.topPad + rowIdx * CL.rowH;
}

/* ── Connections ──────────────────────────────────────────────────────── */
function clDrawConnections(svg) {
  /* Main flow vertical arrows */
  for (let i = 0; i < CL_MAIN_NODES.length - 1; i++) {
    const from = CL_MAIN_NODES[i];
    const to   = CL_MAIN_NODES[i + 1];
    const x    = from._cx;
    const y1   = from._bottom;
    const y2   = to._top;
    if (from.type === 'decision') {
      clDrawLine(svg, x, y1, x, y2, '#3fb950', 2, 'arrow-green', false, 'YES — this path');
    } else {
      clDrawLine(svg, x, y1, x, y2, '#58a6ff', 1.5, 'arrow-blue', false, null);
    }
  }

  /* DI connection lines */
  CL_DI_NODES.forEach(di => {
    const parent = CL_MAIN_NODES.find(n => n.id === di.attachTo);
    if (!parent) return;
    const color  = di.bypass ? '#484f58' : '#f0883e';
    const marker = di.bypass ? 'arrow-gray' : 'arrow-orange';
    clDrawElbow(svg, parent._rx, parent._ry, di._lx, di._cy, color, marker, di.bypass);
  });

  /* Decision bypass — NOT_REQUESTED stays on main path; REQUESTED goes left */
  clDrawDecisionBypass(svg, 'cl_dec_req',
    'YES → REQUESTED_BY_APP or REQUESTED_BY_BUTTON\n(skip checksum — see other diagrams)');

  /* Decision bypass — checksum invalid path goes left */
  clDrawDecisionBypass(svg, 'cl_dec_csum',
    'NO — checksum invalid\n→ Bootloader mode\n(see Cold Boot, No App diagram)');

  /* "If jump returns" right-side arc from cl_jump back up to somewhere */
  clDrawJumpReturnArc(svg);
}

/* ── "If jump returns" right-side arc ────────────────────────────────── */
function clDrawJumpReturnArc(svg) {
  const jumpNode = CL_MAIN_NODES.find(n => n.id === 'cl_jump');
  const appNode  = CL_MAIN_NODES.find(n => n.id === 'cl_app');
  if (!jumpNode || !appNode) return;

  /* Arc goes right of the app node, curves back to indicate fall-through */
  const arcX  = CL.flowX + CL.flowW + 18;
  const fromY = jumpNode._bottom - 10;
  const toY   = appNode._top + (appNode.h || CL.nodeH) / 2;

  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d',
    `M ${CL.flowX + CL.flowW} ${fromY} H ${arcX} V ${toY} H ${CL.flowX + CL.flowW - 8}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', '#484f58');
  path.setAttribute('stroke-width', '1.5');
  path.setAttribute('stroke-dasharray', '5,3');
  path.setAttribute('marker-end', 'url(#arrow-gray)');
  path.style.cursor = 'pointer';
  path.addEventListener('click', () => showDetailRaw('If jump_to_application() Returns', CL_JUMP_RETURN_NOTE.body));
  svg.appendChild(path);

  const midY = (fromY + toY) / 2;
  const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
  text.setAttribute('x', arcX + 2);
  text.setAttribute('y', midY);
  text.setAttribute('fill', '#484f58');
  text.setAttribute('font-size', '10');
  text.setAttribute('font-family', 'monospace');
  text.setAttribute('text-anchor', 'middle');
  text.setAttribute('transform', `rotate(90, ${arcX + 2}, ${midY})`);
  text.style.cursor = 'pointer';
  text.textContent = CL_JUMP_RETURN_NOTE.label;
  text.addEventListener('click', () => showDetailRaw('If jump_to_application() Returns', CL_JUMP_RETURN_NOTE.body));
  svg.appendChild(text);
}

/* ── Local SVG helpers ───────────────────────────────────────────────── */
function clDrawLine(svg, x1, y1, x2, y2, color, sw, markerEnd, dashed, labelText) {
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', x1); line.setAttribute('y1', y1);
  line.setAttribute('x2', x2); line.setAttribute('y2', y2 - 8);
  line.setAttribute('stroke', color);
  line.setAttribute('stroke-width', sw);
  if (dashed) line.setAttribute('stroke-dasharray', '5,4');
  if (markerEnd) line.setAttribute('marker-end', `url(#${markerEnd})`);
  svg.appendChild(line);
  if (labelText) {
    const t = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    t.setAttribute('x', x1 + 6);
    t.setAttribute('y', (y1 + y2) / 2);
    t.setAttribute('fill', color);
    t.setAttribute('font-size', '10');
    t.setAttribute('font-family', 'monospace');
    t.textContent = labelText;
    svg.appendChild(t);
  }
}

function clDrawElbow(svg, x1, y1, x2, y2, color, marker, dashed) {
  const midX = x1 + (x2 - x1) / 2;
  const el = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  el.setAttribute('d', `M ${x1} ${y1} H ${midX} V ${y2} H ${x2 - 8}`);
  el.setAttribute('fill', 'none');
  el.setAttribute('stroke', color);
  el.setAttribute('stroke-width', dashed ? 1.2 : 1.5);
  if (dashed) el.setAttribute('stroke-dasharray', '5,4');
  el.setAttribute('marker-end', `url(#${marker})`);
  svg.appendChild(el);
}

function clDrawDecisionBypass(svg, nodeId, labelText) {
  const n = CL_MAIN_NODES.find(n => n.id === nodeId);
  if (!n) return;
  const bx = n._lx - 30;
  const by = n._top + (n.h || CL.nodeH) / 2;
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', `M ${n._cx} ${n._bottom - (n.h || CL.nodeH) / 2} H ${bx}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', '#484f58');
  path.setAttribute('stroke-width', '1.2');
  path.setAttribute('stroke-dasharray', '5,4');
  svg.appendChild(path);
  labelText.split('\n').forEach((line, i) => {
    const t = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    t.setAttribute('x', bx - 4);
    t.setAttribute('y', by - 14 + i * 12);
    t.setAttribute('fill', '#484f58');
    t.setAttribute('font-size', '9');
    t.setAttribute('font-family', 'monospace');
    t.setAttribute('text-anchor', 'end');
    t.textContent = line;
    svg.appendChild(t);
  });
}
