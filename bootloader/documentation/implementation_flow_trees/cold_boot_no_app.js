'use strict';

/* ======================================================================
 * cold_boot_no_app.js
 *
 * Panel: "Cold Boot, No Application Image"
 *
 * Shared utilities (escHtml, showDetail, buildFlowNode, buildDiNode,
 * createSVG, svgLine, svgElbow, etc.) are in bootloader_diagram_shared.js
 * which must be loaded first.
 *
 * Expose:  renderPanel_ColdBootNoApp(container)
 * ====================================================================== */

/* ── LAYOUT CONSTANTS ───────────────────────────────────────────────── */
const L = {
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
const MAIN_NODES = [
  {
    id:       'n_reset',
    type:     'start',
    label:    'CPU RESET',
    sublabel: 'Hardware reset vector — PC = 0x00000000',
    h:        L.nodeH,
    desc:     'The MCU starts from its hardware reset vector. For Cortex-M devices the initial stack pointer is loaded from address 0x00000000 and the reset handler address from 0x00000004. Execution branches to the reset handler.',
    file:     null,
  },
  {
    id:       'n_main',
    type:     'process',
    label:    'main()',
    sublabel: 'Wire DI tables  ·  bootloader_main.c',
    h:        L.nodeH,
    desc:     'Application entry point. Creates the two constant DI structs:\n\n• bootloader_can_driver_t — read_received_frame, try_send_frame, get_cached_alias\n• bootloader_openlcb_driver_t — all hardware + lifecycle hooks\n\nEach field is a function pointer to a platform-specific driver. These structs are the ONLY coupling between the bootloader library and your hardware.',
    file:     'bootloader_main.c',
  },
  {
    id:       'n_entry',
    type:     'process',
    label:    'Bootloader_entry()',
    sublabel: 'bootloader.c : 172',
    h:        L.nodeH,
    desc:     'Top-level entry point called by main(). Calls initialize_hardware() through the DI pointer, then calls Bootloader_init(). If Bootloader_init() returns false (the normal case), enters an infinite while(true) loop calling Bootloader_loop() on every iteration.',
    file:     'bootloader.c',
    line:     169,
  },
  {
    id:       'n_init_hw',
    type:     'di_inline',
    label:    'openlcb_driver→initialize_hardware()',
    sublabel: 'DI CALLOUT  ·  called before Bootloader_init()',
    h:        L.nodeH,
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    sig:      'void (*initialize_hardware)(void)',
    desc:     'Platform must perform full hardware initialization:\n\n1. Read the reset cause register\n   • If NOT a software reset (power-on, brown-out, watchdog, ECC fault, etc.):\n     – Clear bootloader_request_flag = 0\n     – Clear bootloader_cached_alias = 0\n     This prevents random SRAM garbage accidentally matching the magic value.\n   • If IS a software reset: leave .noinit SRAM untouched so\n     is_bootloader_requested() can read the values the application wrote.\n\n2. Initialize all hardware the bootloader needs:\n   – System clocks / PLL\n   – CAN peripheral (baud rate, acceptance filters)\n   – GPIO (status LEDs, boot button)\n   – Optional: debug UART\n   – Timer for get_100ms_timer_tick() (e.g. SysTick)\n\nEvery MCU family has its own reset-cause register. Read your datasheet\nand map the "software reset" cause code to gate the .noinit clear correctly.',
  },
  {
    id:       'n_bl_init',
    type:     'process',
    label:    'Bootloader_init()',
    sublabel: 'bootloader.c : 111',
    h:        L.nodeH,
    desc:     'Core initialization. In order:\n1. Calls is_bootloader_requested() DI to determine why we are running\n2. Calls set_status_led(REQUEST, ...) to reflect the reason on an LED\n3. If NOT_REQUESTED:\n   a. Runs _check_application_checksum()  — OR if NO_CHECKSUM is defined,\n      skips the checksum and forces csum_ok = true\n   b. If csum_ok: calls get_flash_boundaries() and checks flash_min[0] != 0xFFFFFFFF\n   c. If app present: calls cleanup_before_handoff() then jump_to_application()\n   d. If flash blank (this path): skips cleanup+jump entirely — hardware untouched\n4. Zeros bootloader_state\n5. Sets firmware_active based on request reason\n6. Calls BootloaderCanSM_init() and BootloaderOpenlcbSM_init()',
    file:     'bootloader.c',
    line:     111,
  },
  {
    id:       'n_bl_req',
    type:     'di_inline',
    label:    'openlcb_driver→is_bootloader_requested()',
    sublabel: 'DI CALLOUT  ·  returns BOOTLOADER_NOT_REQUESTED on cold boot',
    h:        L.nodeH_lg,
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    result:   '→ BOOTLOADER_NOT_REQUESTED',
    sig:      'bootloader_request_t (*is_bootloader_requested)(void)',
    desc:     'Determines why the bootloader is running. Must check in this order:\n\n1. Read bootloader_request_flag (.noinit SRAM)\n   • If == BOOTLOADER_REQUEST_MAGIC (0xB00710AD):\n     – Clear the flag\n     – Return BOOTLOADER_REQUESTED_BY_APP\n\n2. Read a hardware button / GPIO pin\n   • If held at boot:\n     – Return BOOTLOADER_REQUESTED_BY_BUTTON\n\n3. Otherwise:\n   • Return BOOTLOADER_NOT_REQUESTED\n\nOn a cold boot with erased flash: no magic, no button → returns BOOTLOADER_NOT_REQUESTED. The library then checks the application checksum.',
  },
  {
    id:       'n_dec_req',
    type:     'decision',
    label:    'Bootloader Requested?',
    sublabel: 'Cold boot path: NOT_REQUESTED  →  check checksum',
    h:        L.nodeH,
    desc:     'Branches on the return value of is_bootloader_requested().\n\nNOT_REQUESTED (this path):\n  → Proceed to _check_application_checksum()\n\nREQUESTED_BY_APP:\n  → Skip checksum, set firmware_active = 1\n  → Enter bootloader mode with cached alias (separate diagram)\n\nREQUESTED_BY_BUTTON:\n  → Skip checksum, set firmware_active = 0\n  → Enter bootloader mode with full CID negotiation (same alias path as this diagram)',
  },
  {
    id:       'n_csum',
    type:     'process',
    label:    '_check_application_checksum()',
    sublabel: 'bootloader.c : 53',
    h:        L.nodeH,
    desc:     'Validates the application firmware image in two passes:\n\n• get_flash_boundaries() DI → flash_min, flash_max, app_header pointer\n• Copy app_header to RAM\n• Validate app_size ≤ (flash_max − flash_min)\n• compute_checksum() over [flash_min, app_header) → compare with header.checksum_pre[]\n• compute_checksum() over [app_header + sizeof(header), app_size) → compare with header.checksum_post[]\n\nReturns false immediately at the first failure. On erased flash (all 0xFF), app_size will be 0xFFFFFFFF which exceeds flash_size → returns false.\n\nNO_CHECKSUM bypass: If NO_CHECKSUM is defined in bootloader_types.h, this function is not called at all — csum_ok is forced to true. The library still performs the flash vector check (flash_min[0] != 0xFFFFFFFF) before calling cleanup+jump, so a blank flash device stays in bootloader mode correctly even with NO_CHECKSUM.',
    file:     'bootloader.c',
    line:     53,
  },
  {
    id:       'n_dec_csum',
    type:     'decision',
    label:    'Checksum Valid?',
    sublabel: 'No app image → INVALID  →  enter bootloader mode',
    h:        L.nodeH,
    desc:     'If checksum passes (or NO_CHECKSUM forces csum_ok=true):\n  • Library calls get_flash_boundaries() and reads flash_min[0]\n  • If flash_min[0] != 0xFFFFFFFF — app present:\n      cleanup_before_handoff() → jump_to_application()\n  • If flash_min[0] == 0xFFFFFFFF — flash blank (this path):\n      cleanup and jump are skipped entirely — hardware stays intact\n      fall through to bootloader mode\n\nIf checksum fails (this path — no app image, checksum enabled):\n  • set_status_led(BOOTLOADER_LED_CSUM_ERROR, true)\n  • Fall through to bootloader mode\n  • firmware_active will be set to 0',
  },
  {
    id:       'n_bl_mode',
    type:     'state',
    label:    'Bootloader Mode Entry',
    sublabel: 'firmware_active = 0  ·  CT must send Freeze before transfer',
    h:        L.nodeH,
    desc:     'bootloader_state is zero-initialized (memset to 0).\n\nfirmware_active = 0 because we were NOT requested by an app that had already received a Freeze command. This means:\n  • PIP will advertise PSI_FIRMWARE_UPGRADE (0x000020) — "I support firmware upgrade"\n  • NOT PSI_FIRMWARE_UPGRADE_ACTIVE (0x000010) — "transfer in progress"\n\nThe Configuration Tool must send a Freeze command (datagram, command 0xA1, space 0xEF) before it can transfer firmware data.',
  },
  {
    id:       'n_cansm_init',
    type:     'process',
    label:    'BootloaderCanSM_init()   BootloaderOpenlcbSM_init()',
    sublabel: 'bootloader_can_statemachine.c : 385   ·   bootloader_openlcb_statemachine.c : 470',
    h:        L.nodeH,
    desc:     'BootloaderCanSM_init():\n  • Zeroes internal _sm state\n  • Stores can_driver and openlcb_driver pointers\n  • Calls BootloaderRx_init(can_driver) — zeros RX buffers and flags\n  • Calls BootloaderTx_init(can_driver) — stores TX driver pointer\n  • init_state starts at INIT_PICK_ALIAS\n\nBootloaderOpenlcbSM_init():\n  • Stores openlcb_driver pointer for use in protocol handlers',
    file:     'bootloader_can_statemachine.c',
    line:     385,
  },
  {
    id:       'n_loop',
    type:     'loop',
    label:    '↻  Bootloader_loop()  —  infinite main loop',
    sublabel: 'bootloader.c : 155  ·  calls BootloaderCanSM_loop() each iteration',
    h:        L.nodeH,
    desc:     'The main loop. Each iteration:\n1. Calls BootloaderCanSM_loop() — the primary work\n2. Checks bootloader_state.request_reset AND BootloaderCanSM_is_initialized():\n   — both must be true before reboot() is called\n   — gating on is_initialized() ensures the node is on the network when it reboots\n   — reboot() calls NVIC_SystemReset() directly; no cleanup_before_handoff() needed here\n     because NVIC_SystemReset() resets ALL peripheral registers to power-on defaults — it IS the cleanup\n3. Updates BOOTLOADER_LED_ACTIVE: lit while not yet initialized, cleared after INIT_INITIALIZED\n\nNo RTOS, no interrupts needed — pure cooperative polling.',
    file:     'bootloader.c',
    line:     152,
  },
  {
    id:       'n_cansm_loop',
    type:     'process',
    label:    'BootloaderCanSM_loop()',
    sublabel: 'bootloader_can_statemachine.c : 396',
    h:        L.nodeH,
    desc:     'One pass of the CAN state machine. In order:\n1. BootloaderRx_poll(alias, tick) — drain HW FIFO, filter, assemble datagrams\n2. _handle_collisions() — check RID/non-CID collision flags\n3. Try to send pending frame if pending_frame_full\n4. If still initializing: run _handle_init() to advance alias state machine\n5. If initialized: check for datagram errors, dispatch completed datagrams, dispatch non-datagram frames',
    file:     'bootloader_can_statemachine.c',
    line:     396,
  },
  {
    id:       'n_pick_alias',
    type:     'state',
    label:    'INIT_PICK_ALIAS',
    sublabel: 'No cached alias on cold boot  →  full CID negotiation',
    h:        L.nodeH,
    desc:     'Entry state of alias negotiation. Calls get_cached_alias() DI:\n\n• Returns non-zero (app drop-back): use cached alias, skip to INIT_SEND_INIT_COMPLETE — the alias is already mapped on the bus.\n\n• Returns 0 (cold boot — this path): generate a new alias. Calls get_persistent_node_id() to get the 48-bit Node ID and uses it as the LFSR seed. Advances to INIT_SEND_CID_7.',
    file:     'bootloader_can_statemachine.c',
    line:     180,
  },
  {
    id:       'n_gen_alias',
    type:     'process',
    label:    'Generate Alias from LFSR Seed',
    sublabel: '_generate_alias()  ·  retry loop if alias == 0',
    h:        L.nodeH,
    desc:     'Alias generation — two 24-bit LFSRs seeded from the Node ID:\n\n  lfsr1 = upper 24 bits of Node ID\n  lfsr2 = lower 24 bits of Node ID\n  Each LFSR: shift left, XOR-rotate, XOR with constant\n  alias = (lfsr1 ^ lfsr2 ^ (lfsr1>>12) ^ (lfsr2>>12)) & 0x0FFF\n\nIf alias == 0 (invalid): advance seed and retry. Alias 0x000 is reserved and must not be used.',
    file:     'bootloader_can_statemachine.c',
    line:     87,
  },
  {
    id:       'n_cid',
    type:     'process',
    label:    'Send CID7 → CID6 → CID5 → CID4',
    sublabel: 'Four Check ID frames  ·  each encodes 12 bits of Node ID',
    h:        L.nodeH,
    desc:     'Four consecutive CAN control frames are sent with the candidate alias in the low 12 bits of the CAN ID.\n\nEach CID frame encodes a different 12-bit slice of the 48-bit Node ID in the variable field of the CAN ID. Any node already using this alias that sees its own Node ID bits will know there is a conflict and must send RID.\n\nIf a collision flag is raised (by BootloaderRx_poll detecting a RID or non-CID frame from our alias), the alias state machine restarts from INIT_PICK_ALIAS with a new seed.',
    file:     'bootloader_can_statemachine.c',
    line:     214,
  },
  {
    id:       'n_wait_rid',
    type:     'state',
    label:    'INIT_WAIT_RID — 200 ms hold-off',
    sublabel: 'Waiting for any RID collision response',
    h:        L.nodeH,
    desc:     'After the four CID frames, the OpenLCB CAN specification requires a minimum 200 ms delay before claiming the alias. This allows any conflicting node time to respond with RID.\n\nget_100ms_timer_tick() DI is called each loop iteration. The condition is: elapsed = (current_tick − wait_tick) > 2, which means at least 2 × 100 ms = 200 ms have passed.\n\nDuring this wait, BootloaderRx_poll() continues draining the CAN FIFO each loop iteration, so any incoming RID collision is detected immediately and causes a restart.',
    file:     'bootloader_can_statemachine.c',
    line:     239,
  },
  {
    id:       'n_rid',
    type:     'process',
    label:    'Send RID',
    sublabel: 'Reserve ID Definite  ·  alias claim confirmed',
    h:        L.nodeH,
    desc:     'A Reserve ID Definite (RID) CAN control frame asserts that no collision was seen during or after the CID sequence and the 200 ms hold-off. After this frame the node has formally claimed the alias.\n\nCAN ID: RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | alias\nDLC: 0 (no payload).',
    file:     'bootloader_can_statemachine.c',
    line:     254,
  },
  {
    id:       'n_amd',
    type:     'process',
    label:    'Send AMD',
    sublabel: 'Alias Map Definition  ·  broadcast Node ID → alias mapping',
    h:        L.nodeH,
    desc:     'An Alias Map Definition (AMD) control frame broadcasts the mapping of the full 48-bit Node ID to the 12-bit alias. The 6-byte Node ID is in the data payload (DLC=6).\n\nOther nodes receiving AMD update their alias mapping tables so they can send addressed messages to us using the alias rather than the full Node ID.\n\nCalls get_persistent_node_id() DI to fill the 6-byte payload.',
    file:     'bootloader_can_statemachine.c',
    line:     260,
  },
  {
    id:       'n_init_complete',
    type:     'process',
    label:    'Send Init Complete',
    sublabel: 'MTI 0x0100  ·  node is officially on the CAN bus',
    h:        L.nodeH,
    desc:     'Initialization Complete is a global OpenLCB message (MTI 0x0100) with the 6-byte Node ID in the payload.\n\nThis is the official announcement that the node has finished initialization and is now active. Any Configuration Tool waiting to discover the bootloader will react to this frame and begin its discovery sequence (PIP, SNIP, Verify Node ID).\n\nCAN ID encodes MTI 0x100 in the variable field with the standard global message format.',
    file:     'bootloader_can_statemachine.c',
    line:     267,
  },
  {
    id:       'n_initialized',
    type:     'state',
    label:    'INIT_INITIALIZED',
    sublabel: 'Node fully logged in  ·  dispatching messages',
    h:        L.nodeH,
    desc:     'Alias negotiation is complete. The node is visible and addressable on the CAN bus.\n\nEach subsequent loop iteration through BootloaderCanSM_loop():\n  • Drains RX FIFO\n  • Checks for datagram errors → send Datagram Rejected\n  • Dispatches completed datagrams → BootloaderOpenlcbSM_on_datagram_received()\n  • Dispatches non-datagram frames → BootloaderOpenlcbSM_on_addressed_message() or _on_global_message()\n  • Handles AME requests → sends AMD reply',
    file:     'bootloader_can_statemachine.c',
    line:     274,
  },
  {
    id:       'n_end',
    type:     'end_wait',
    label:    'Awaiting CT Freeze Command',
    sublabel: 'Serving PIP / SNIP / Verify Node ID  ·  firmware_active = 0',
    h:        L.nodeH_lg,
    desc:     'The bootloader is fully operational as an OpenLCB node.\n\nResponds to:\n  • PIP (0x0828) → PSI_FIRMWARE_UPGRADE (firmware_active=0) — tells CT a Freeze is needed\n  • SNIP (0x0DE8) → manufacturer, model, hw/sw version strings\n  • Verify Node ID Addressed (0x0488) / Global (0x0490) → Verified Node ID reply\n  • AME → AMD reply\n  • Unknown MTIs → Optional Interaction Rejected\n\nWhen the CT sends Freeze (datagram, command 0xA1, space 0xEF):\n  • firmware_active becomes 1\n  • Sends Init Complete again\n  • PIP now reports PSI_FIRMWARE_UPGRADE_ACTIVE\n  • CT proceeds to send Write datagrams',
  },
];

/* ── DI CALLOUT NODES ───────────────────────────────────────────────── */
const DI_NODES = [
  {
    id:       'di_flash_bounds',
    attachTo: 'n_csum',
    label:    'get_flash_boundaries()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        L.diNodeH,
    sig:      'void (*)(const void **flash_min, const void **flash_max, const bootloader_app_header_t **app_header)',
    desc:     'Return the application flash region boundaries.\n\n• flash_min — start address of the app image (e.g. 0x00004000 on MSPM0G3507)\n• flash_max — exclusive end address\n• app_header — pointer to the fixed offset where the bootloader_app_header_t struct is linked into the app binary\n\nTypically these are just addresses defined in your linker script, returned as constants.',
  },
  {
    id:       'di_compute_csum',
    attachTo: 'n_csum',
    label:    'compute_checksum()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        L.diNodeH,
    sig:      'void (*)(const void *data, uint32_t size, uint32_t *checksum)',
    desc:     'Compute a checksum over a memory region into a uint32_t[4] result (128 bits total — three independent CRC-16/IBM values packed into the array).\n\nCalled twice: once for the pre-header region and once for the post-header region. The bootloader library provides bootloader_crc_compute() as a default implementation. You may substitute a hardware CRC accelerator.',
  },
  {
    id:       'di_jump_bypass',
    attachTo: 'n_dec_csum',
    label:    'jump_to_application()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    bypass:   true,
    h:        L.diNodeH,
    sig:      'void (*)(void)',
    desc:     'NOT TAKEN on this path — only called when the checksum is valid.\n\nCortex-M implementation:\n  1. Set SCB->VTOR to app flash start address\n  2. Load new MSP: __set_MSP(*(uint32_t*)APP_START)\n  3. Jump: ((void(*)(void))(*(uint32_t*)(APP_START+4)))()\n\nIf this function returns, the app image was present but not executable (e.g. thumb bit not set, vector table corrupt). Execution falls back into the bootloader.',
  },
  {
    id:       'di_led_csum',
    attachTo: 'n_dec_csum',
    label:    'set_status_led(CSUM_ERROR, true)',
    struct:   'bootloader_openlcb_driver_t',
    nullable: true,
    h:        L.diNodeH,
    sig:      'void (*)(bootloader_led_enum led, bool state)',
    result:   'BOOTLOADER_LED_CSUM_ERROR = 8',
    desc:     'Illuminate the checksum error indicator. Called with BOOTLOADER_LED_CSUM_ERROR (= 8) and true.\n\nMay be NULL — the library checks for NULL before calling. If no LED is available, assign NULL to this field in the DI struct.\n\nGives a hardware-visible indication that no valid app image was found. Useful for bring-up debugging.',
  },
  {
    id:       'di_cached_alias',
    attachTo: 'n_pick_alias',
    label:    'get_cached_alias()',
    struct:   'bootloader_can_driver_t',
    nullable: false,
    h:        L.diNodeH,
    result:   '→ returns 0  (cold boot)',
    sig:      'uint16_t (*)(void)',
    desc:     'Returns the CAN alias the application was using before it triggered a reset into the bootloader.\n\nStored in .noinit SRAM as bootloader_cached_alias (defined in bootloader_shared_ram.h). The application writes its current alias here before calling software reset.\n\nNo POR detection needed.\nThe implementation must gate this on the magic value — only return the cached alias if bootloader_request_flag == BOOTLOADER_REQUEST_MAGIC. On cold boot, the magic is not present, so get_cached_alias() returns 0 even though the .noinit memory contains random garbage. This prevents a garbage alias value from being used as if it were a valid cached alias.\n\nClear bootloader_cached_alias after reading it so it does not persist into the next boot.',
  },
  {
    id:       'di_node_id_seed',
    attachTo: 'n_pick_alias',
    label:    'get_persistent_node_id()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        L.diNodeH,
    result:   '→ 48-bit Node ID used as LFSR seed',
    sig:      'uint64_t (*)(void)',
    desc:     'Returns the 48-bit OpenLCB Node ID. Must be globally unique per device.\n\nTypically stored in a reserved flash sector or EEPROM. May also be derived from a factory-programmed hardware UID (shifted/masked to the allowed range).\n\nOn a blank device (all 0xFF) a non-zero fallback must be provided for bring-up — returning 0 is invalid (alias generator would loop forever).\n\nThis call seeds the alias LFSR. The same function is called again later to populate the AMD and Init Complete payloads.',
  },
  {
    id:       'di_try_send_cid',
    attachTo: 'n_cid',
    label:    'try_send_frame()',
    struct:   'bootloader_can_driver_t',
    nullable: false,
    h:        L.diNodeH,
    sig:      'bool (*)(const bootloader_can_frame_t *frame)',
    desc:     'Write a single CAN frame to the hardware TX FIFO or mailbox.\n\nReturns true if the frame was accepted, false if the hardware TX queue is full. The CAN SM retries on the next loop iteration if false.\n\nCalled for each CID frame (×4), RID, AMD, Init Complete, and every other outbound frame. Must be non-blocking — never spin-wait for TX completion here.',
  },
  {
    id:       'di_read_rx',
    attachTo: 'n_wait_rid',
    label:    'read_received_frame()',
    struct:   'bootloader_can_driver_t',
    nullable: false,
    h:        L.diNodeH,
    sig:      'bool (*)(bootloader_can_frame_t *frame)',
    desc:     'Poll the CAN hardware for the next received frame. Called by BootloaderRx_poll() on every loop iteration.\n\nFills *frame and returns true if a frame is available. Returns false immediately if the RX FIFO is empty.\n\nMust handle 29-bit extended frames only (EFF = 1). Standard 11-bit frames are not used by OpenLCB.\n\nAlso called from inside blocking TX spin loops (BootloaderTx_send_multiframe, _send_datagram) to drain the FIFO while waiting for TX arbitration.',
  },
  {
    id:       'di_timer_tick',
    attachTo: 'n_wait_rid',
    label:    'get_100ms_timer_tick()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        L.diNodeH,
    sig:      'uint8_t (*)(void)',
    desc:     'Returns a monotonic 8-bit counter that increments every 100 ms.\n\nUsed for:\n  • 200 ms hold-off after CID frames (wait until elapsed > 2 ticks)\n  • TX timeout during alias negotiation (restart if elapsed > 30 ticks = 3 s)\n\nDoesn\'t need to be perfectly precise — the spec requires a minimum, not an exact duration. Implement with a timer ISR that increments a volatile uint8_t every 100 ms, or with a SysTick poll.\n\nThe 8-bit counter wraps at 255 — the arithmetic is unsigned, so wrap-around comparisons are safe.',
  },
  {
    id:       'di_node_id_amd',
    attachTo: 'n_amd',
    label:    'get_persistent_node_id()',
    struct:   'bootloader_openlcb_driver_t',
    nullable: false,
    h:        L.diNodeH,
    result:   '→ 48-bit Node ID for AMD + Init Complete payload',
    sig:      'uint64_t (*)(void)',
    desc:     'Called a second time to get the 48-bit Node ID for the AMD frame payload (DLC=6, bytes 0–5) and again for the Init Complete message payload.\n\nMust return the same value as the first call — return your Node ID consistently. Do not read from hardware registers that may change; read from your validated flash storage.',
  },
  {
    id:       'di_led_active',
    attachTo: 'n_initialized',
    label:    'set_status_led(ACTIVE, false)',
    struct:   'bootloader_openlcb_driver_t',
    nullable: true,
    h:        L.diNodeH,
    result:   'BOOTLOADER_LED_ACTIVE = 1',
    sig:      'void (*)(bootloader_led_enum led, bool state)',
    desc:     'Clears BOOTLOADER_LED_ACTIVE (= 1).\n\nDuring alias negotiation, Bootloader_loop() calls set_status_led(ACTIVE, !BootloaderCanSM_is_initialized()). This means the ACTIVE LED is illuminated while the node is still negotiating, and cleared once INIT_INITIALIZED is reached.\n\nMay be NULL — the library checks before calling. Map this LED to a physical indicator on your board (e.g. the green network-active LED).',
  },
  {
    id:       'di_rx_poll_end',
    attachTo: 'n_end',
    label:    'read_received_frame()',
    struct:   'bootloader_can_driver_t',
    nullable: false,
    h:        L.diNodeH_lg,
    sig:      'bool (*)(bootloader_can_frame_t *frame)',
    desc:     'Called on every loop iteration via BootloaderRx_poll() — including while waiting for the CT.\n\nEach call drains one frame from the CAN hardware RX FIFO. BootloaderCanSM_loop() continues calling this until it returns false (FIFO empty), then dispatches whatever was buffered:\n\n  • PIP request (0x0828)   → reply with PSI_FIRMWARE_UPGRADE\n  • SNIP request (0x0DE8)  → reply with manufacturer/model strings\n  • Verify Node ID          → reply with Verified Node ID\n  • Datagram (Freeze 0xA1) → sets firmware_active=1, sends Init Complete\n  • AME control frame       → reply with AMD\n\nThere is no blocking or sleeping — the loop spins continuously, returning here each pass.',
  },
];

/* ── COLLISION NOTE DATA ─────────────────────────────────────────────── */
const COLLISION_NOTE = {
  fromNode: 'n_wait_rid',
  toNode:   'n_pick_alias',
  label:    '⚡ collision → restart alias',
  body:     'During CID send or 200 ms wait, BootloaderRx_poll() may flag a collision:\n\n• collision_cid flag: another node sent RID for our alias during alias negotiation → restart from INIT_PICK_ALIAS with a new LFSR seed\n\n• collision_non_cid flag (post-initialization): hard conflict, send AMR frame, restart alias reservation\n\nThe loop arrow returns to INIT_PICK_ALIAS and a new alias is generated.',
};

/* ======================================================================
 * RENDERING
 * ====================================================================== */

function renderPanel_ColdBootNoApp(container) {
  /* Pre-compute row indices for each main node */
  MAIN_NODES.forEach((n, i) => { n._row = i; });

  /* Pre-compute DI node Y positions */
  let diRunningY  = L.topPad;
  let prevParentRow = -1;
  DI_NODES.forEach(di => {
    const p = MAIN_NODES.find(n => n.id === di.attachTo);
    if (!p) return;
    if (p._row !== prevParentRow) {
      diRunningY    = Math.max(diRunningY, flowY(p._row));
      prevParentRow = p._row;
    }
    di._y = diRunningY;
    diRunningY += (di.h || L.diNodeH) + 8;
  });

  /* Total height */
  const lastMain = MAIN_NODES[MAIN_NODES.length - 1];
  const mainH    = flowY(lastMain._row) + (lastMain.h || L.nodeH) + L.topPad;
  const totalH   = Math.max(mainH, diRunningY + 60);

  container.style.width  = L.totalW + 'px';
  container.style.height = totalH  + 'px';

  /* SVG overlay */
  const svg = createSVG(L.totalW, totalH);
  container.appendChild(svg);

  /* Render main flow nodes */
  MAIN_NODES.forEach(n => {
    const y  = flowY(n._row);
    const h  = n.h || L.nodeH;
    const el = buildFlowNode(n, L.flowX, y, L.flowW, h);
    container.appendChild(el);
    n._top    = y;
    n._bottom = y + h;
    n._cx     = L.flowX + L.flowW / 2;
    n._ry     = y + h / 2;
    n._rx     = L.flowX + L.flowW;
    n._lx     = L.flowX;
  });

  /* Render DI callout nodes */
  DI_NODES.forEach(di => {
    const h  = di.h || L.diNodeH;
    const el = buildDiNode(di, L.diX, di._y, L.diW, h);
    container.appendChild(el);
    di._cy = di._y + h / 2;
    di._lx = L.diX;
  });

  drawConnections(svg);
}

/* ── Position helpers ─────────────────────────────────────────────────── */
function flowY(rowIdx) {
  return L.topPad + rowIdx * L.rowH;
}

/* ── Draw everything ──────────────────────────────────────────────────── */
function drawConnections(svg) {
  /* Main flow vertical arrows */
  for (let i = 0; i < MAIN_NODES.length - 1; i++) {
    const from = MAIN_NODES[i];
    const to   = MAIN_NODES[i + 1];
    const x    = from._cx;
    const y1   = from._bottom;
    const y2   = to._top;
    if (from.type === 'decision') {
      drawLine(svg, x, y1, x, y2, '#3fb950', 2, 'arrow-green', false, 'NO — this path');
    } else {
      drawLine(svg, x, y1, x, y2, '#58a6ff', 1.5, 'arrow-blue', false, null);
    }
  }

  /* DI connection lines */
  DI_NODES.forEach(di => {
    const parent = MAIN_NODES.find(n => n.id === di.attachTo);
    if (!parent) return;
    const color  = di.bypass ? '#484f58' : '#f0883e';
    const marker = di.bypass ? 'arrow-gray' : 'arrow-orange';
    drawElbow(svg, parent._rx, parent._ry, di._lx, di._cy, color, marker, di.bypass);
  });

  /* Decision YES bypass */
  drawDecisionBypass(svg, 'n_dec_req',
    'YES → REQUESTED_BY_APP\nor REQUESTED_BY_BUTTON\n(see drop-back / button diagrams)',
    'left');

  /* Collision restart arc */
  const collFrom = MAIN_NODES.find(n => n.id === COLLISION_NOTE.fromNode);
  const collTo   = MAIN_NODES.find(n => n.id === COLLISION_NOTE.toNode);
  if (collFrom && collTo) drawCollisionArc(svg, collFrom, collTo);
}

function drawLine(svg, x1, y1, x2, y2, color, sw, markerEnd, dashed, labelText) {
  const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  line.setAttribute('x1', x1);
  line.setAttribute('y1', y1);
  line.setAttribute('x2', x2);
  line.setAttribute('y2', y2 - 8);
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

function drawElbow(svg, x1, y1, x2, y2, color, marker, dashed) {
  const midX = x1 + (x2 - x1) / 2;
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', `M ${x1} ${y1} H ${midX} V ${y2} H ${x2 - 8}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', color);
  path.setAttribute('stroke-width', dashed ? 1.2 : 1.5);
  if (dashed) path.setAttribute('stroke-dasharray', '5,4');
  path.setAttribute('marker-end', `url(#${marker})`);
  svg.appendChild(path);
}

function drawDecisionBypass(svg, nodeId, labelText) {
  const n = MAIN_NODES.find(n => n.id === nodeId);
  if (!n) return;
  const bx = n._lx - 30;
  const by = n._top + (n.h || L.nodeH) / 2;
  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', `M ${n._cx} ${n._bottom - (n.h || L.nodeH) / 2} H ${bx}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', '#484f58');
  path.setAttribute('stroke-width', '1.2');
  path.setAttribute('stroke-dasharray', '5,4');
  svg.appendChild(path);
  labelText.split('\n').forEach((line, i) => {
    const t = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    t.setAttribute('x', bx - 4);
    t.setAttribute('y', by - 10 + i * 12);
    t.setAttribute('fill', '#484f58');
    t.setAttribute('font-size', '9');
    t.setAttribute('font-family', 'monospace');
    t.setAttribute('text-anchor', 'end');
    t.textContent = line;
    svg.appendChild(t);
  });
}

function drawCollisionArc(svg, fromNode, toNode) {
  const arcX  = L.flowX - 22;
  const fromY = fromNode._bottom - 10;
  const toY   = toNode._top + (toNode.h || L.nodeH) / 2;

  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', `M ${L.flowX} ${fromY} H ${arcX} V ${toY} H ${L.flowX + 8}`);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', '#a371f7');
  path.setAttribute('stroke-width', '2');
  path.setAttribute('stroke-dasharray', '6,3');
  path.setAttribute('marker-end', 'url(#arrow-purple)');
  path.style.cursor = 'pointer';
  path.addEventListener('click', () => showDetailRaw('Alias Collision Handling', COLLISION_NOTE.body));
  svg.appendChild(path);

  const midY = (fromY + toY) / 2;
  const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
  text.setAttribute('x', arcX - 2);
  text.setAttribute('y', midY);
  text.setAttribute('fill', '#a371f7');
  text.setAttribute('font-size', '10');
  text.setAttribute('font-family', 'monospace');
  text.setAttribute('text-anchor', 'middle');
  text.setAttribute('transform', `rotate(-90, ${arcX - 2}, ${midY})`);
  text.style.cursor = 'pointer';
  text.textContent = COLLISION_NOTE.label;
  text.addEventListener('click', () => showDetailRaw('Alias Collision Handling', COLLISION_NOTE.body));
  svg.appendChild(text);
}
