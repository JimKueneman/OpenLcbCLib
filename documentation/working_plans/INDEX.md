# Working Plans — Implementation Index

Updated: 2026-03-19
Source folder: `documentation/working_plans/`

This index lists all working plans — both active and archived. Each archive file
carries a `STATUS` banner at the top that records the verdict and the specific
evidence (or reason for rejection) found in the canonical source tree
(`src/openlcb/`, `src/drivers/canbus/`, `test/`).

---

## Implemented Plans (28)

These plans have been verified against the codebase and their described changes are
present in the canonical source.

| # | File | Title / Topic |
|---|------|---------------|
| 1 | [plan_item_2_datagram_timeout_COMPLETE.md](archive/plan_item_2_datagram_timeout_COMPLETE.md) | Datagram Timeout and Retry Limit |
| 2 | [plan_item_4_train_search_allocate_COMPLETE.md](archive/plan_item_4_train_search_allocate_COMPLETE.md) | Train Search Allocate Callback Never Called |
| 3 | [plan_item_5_controller_assign_reject.md](archive/plan_item_5_controller_assign_reject.md) | Controller Assign Reject Reply — Include Current Controller Node ID |
| 4 | [plan_item_7_multiframe_timeout_COMPLETE.md](archive/plan_item_7_multiframe_timeout_COMPLETE.md) | Timeout on Partially Assembled Multi-Frame Datagrams |
| 5 | [plan_item_8_oir_tde_handlers_COMPLETE.md](archive/plan_item_8_oir_tde_handlers_COMPLETE.md) | OIR and TDE Receipt Handlers |
| 6 | [plan_item_9_unhandled_datagrams_COMPLETE.md](archive/plan_item_9_unhandled_datagrams_COMPLETE.md) | Unhandled Datagrams Send OIR/Reject |
| 7 | [plan_item_10_cid_handler_COMPLETE.md](archive/plan_item_10_cid_handler_COMPLETE.md) | CID Handler — Always RID (Documentation Only) |
| 8 | [plan_item_13_protocol_support_48bit_COMPLETE.md](archive/plan_item_13_protocol_support_48bit_COMPLETE.md) | Protocol Support Reply — Full 48-Bit Encoding |
| 9 | [plan_item_15_snip_null_terminator_COMPLETE.md](archive/plan_item_15_snip_null_terminator_COMPLETE.md) | SNIP Null Terminator Always Appended |
| 10 | [plan_item_16_snip_null_guard_COMPLETE.md](archive/plan_item_16_snip_null_guard_COMPLETE.md) | SNIP NULL Guard on config_memory_read Callback |
| 11 | [plan_item_17_heartbeat_direction_COMPLETE.md](archive/plan_item_17_heartbeat_direction_COMPLETE.md) | Heartbeat Timeout Preserves Speed Direction |
| 12 | [plan_item_18_train_search_disambiguation_COMPLETE.md](archive/plan_item_18_train_search_disambiguation_COMPLETE.md) | Train Search Short/Long Address Disambiguation |
| 13 | [plan_item_19_stream_stubs_COMPLETE.md](archive/plan_item_19_stream_stubs_COMPLETE.md) | Stream Support Stubs Send OIR |
| 14 | [plan_item_20_write_under_mask_COMPLETE.md](archive/plan_item_20_write_under_mask_COMPLETE.md) | Write Under Mask — All Eight Spaces |
| 15 | [plan_amr_alias_invalidation_COMPLETE.md](archive/plan_amr_alias_invalidation_COMPLETE.md) | AMR Alias Invalidation — CAN-Layer Defense |
| 16 | [plan_global_clock_refactor_COMPLETE.md](archive/plan_global_clock_refactor_COMPLETE.md) | Global Clock Refactor — Timer Work Moved to Main Loop |
| 17 | [sibling_dispatch_plan.md](archive/sibling_dispatch_plan.md) | Virtual Node Sibling Dispatch (final implementation plan) |
| 18 | [OLCBChecker_New_Tests.md](archive/OLCBChecker_New_Tests.md) | OLCBChecker New Tests + Three Library Bug Fixes |
| 19 | [compliance_test_node_implementation.md](archive/compliance_test_node_implementation.md) | ComplianceTestNode for OlcbChecker Testing |
| 20 | [listener_alias_verification_plan.md](archive/listener_alias_verification_plan.md) | Periodic Listener Alias Verification |
| 21 | [olcbchecker_auto_reboot_implementation_plan.md](archive/olcbchecker_auto_reboot_implementation_plan.md) | OlcbChecker `auto_reboot` Flag Integration |
| 22 | [olcbchecker_coverage_analysis.md](archive/olcbchecker_coverage_analysis.md) | OlcbChecker Test Coverage Analysis (64 tests) |
| 23 | [olcbchecker_integration_test_suite.md](archive/olcbchecker_integration_test_suite.md) | OlcbChecker Integration Test Suite |
| 24 | [production_release_1_0_0.md](archive/production_release_1_0_0.md) | v1.0.0 Production Release Preparation |
| 25 | [plan_item_3_consist_forwarding.md](archive/plan_item_3_consist_forwarding.md) | Consist Forwarding (Train Control) — wiring gap fixed 2026-03-15 |
| 26 | [plan_unified_sibling_dispatch.md](archive/plan_unified_sibling_dispatch.md) | Unified Sibling Dispatch (Zero-FIFO, Sequential) — all 3 phases implemented + 46 tests |
| 27 | [plan_listener_alias_table_integration.md](archive/plan_listener_alias_table_integration.md) | Listener Alias Table — CAN Layer Integration (v5) — core wiring complete; hold-until-AMD deferred (v1.0.0 known limitation) |
| 28 | [bootloader_config_plan.md](archive/bootloader_config_plan.md) | Bootloader Build Configuration — whole-module #ifdef guards, internal #ifndef BOOTLOADER guards, 30 tests, buffer-count fix |

---

## Rejected / Not Implemented Plans (7)

These plans were not implemented as written, or were superseded.  Each file's STATUS
banner gives the specific reason.

| # | File | Title / Topic | Reason Summary |
|---|------|---------------|----------------|
| 1 | [plan_item_1_datagram_ok_reply_pending_COMPLETE.md](archive/plan_item_1_datagram_ok_reply_pending_COMPLETE.md) | Datagram OK Reply — Conditional REPLY_PENDING Flag | Intentional design decision: bit is always set by design (comment documents the rationale); setting it unconditionally is protocol-safe and simpler |
| 2 | [plan_item_6_datagram_retry_limit_COMPLETE.md](archive/plan_item_6_datagram_retry_limit_COMPLETE.md) | Datagram Retry Limit Enforcement | Intentional design decision: retry goal IS implemented but via a 3-bit packed field in `openlcb_msg_timer_t` (per-message) rather than a per-node field as this plan proposed; the plan itself notes "SUPERSEDED — Fully covered by Item 2" |
| 3 | [plan_item_14_verified_node_id_dest_COMPLETE.md](archive/plan_item_14_verified_node_id_dest_COMPLETE.md) | Verified Node ID — Non-Zero Dest for Addressed Requests | Intentional design decision: MessageNetworkS §3.4.2 requires Verified Node ID to always be unaddressed; plan pre-dated closer spec reading |
| 4 | [plan_listener_alias_resolution_COMPLETE.md](archive/plan_listener_alias_resolution_COMPLETE.md) | Listener-to-Listener Alias Resolution on CAN | Intentional design decision: alias embedded-in-struct approach superseded by separate `alias_mapping_listener.h/c` module, which is transport-agnostic and correct |
| 5 | [plan_virtual_node_sibling_dispatch.md](archive/plan_virtual_node_sibling_dispatch.md) | Virtual Node Sibling Dispatch (early sketch) | Superseded by `sibling_dispatch_plan.md`, which was fully implemented instead |
| 6 | [plan_stream_transport.md](archive/plan_stream_transport.md) | Stream Transport Protocol Handler | No `protocol_stream_handler.h/c` files exist; handler module was never written |
| 7 | [plan_olcbchecker_train_control_tests.md](archive/plan_olcbchecker_train_control_tests.md) | OlcbChecker Train Control Test Completion | TR090/100/110 done externally; TR120/130 explicitly deferred (requires multi-node harness) |

---

## Superseded / Historical Plans (2)

These documents were exploratory brainstorming that led to implementation plans.
Their content has been folded into active plans. Retained for historical reference.

| # | File | Title / Topic | Superseded By |
|---|------|---------------|---------------|
| 1 | [proposed_virtual_train_implementation.md](archive/proposed_virtual_train_implementation.md) | Proposed Virtual Train Implementation (Categories 1-5) | Categories 1-2 → Phase 3 of `plan_unified_sibling_dispatch.md`; Categories 3-5 → Phases 1-2 |
| 2 | [virtual_node_outgoing_failure_scenarios.md](archive/virtual_node_outgoing_failure_scenarios.md) | Virtual Node Outgoing Path — 7 Failure Scenarios | All resolved by `plan_unified_sibling_dispatch.md` Phase 1 |

---

## Active Plans

| File | Title / Topic | Status |
|------|---------------|--------|
| [plan_virtual_node_verification_protocol.md](plan_virtual_node_verification_protocol.md) | Virtual Node Verification Protocol (DRAFT) | Draft protocol spec — not yet proposed to OpenLCB standards group |
| [stream_plan.md](stream_plan.md) | Stream Transport Protocol | DEFERRED — spec is Preliminary (Apr 2021); awaiting OpenMRN payload layout confirmation |

---

## Hardware Regression Investigation (2026-03-19)

ESP32/STM32/TI platforms stopped working after week of 2026-03-18 changes. Three separate fault case documents — to be consolidated once common root cause is identified.

| File | Case | Symptom |
|------|------|---------|
| [fault_case_1_can_login_hang.md](fault_case_1_can_login_hang.md) | Case 1 | `OpenLcb_run()` blocks during CAN login (CID/RID frames) |
| [fault_case_2_snip_hang_no_concurrent.md](fault_case_2_snip_hang_no_concurrent.md) | Case 2 | `OpenLcb_run()` blocks mid-SNIP TX (no concurrent incoming msg) |
| [fault_case_3_snip_hang_psi_concurrent.md](fault_case_3_snip_hang_psi_concurrent.md) | Case 3 | `OpenLcb_run()` blocks mid-SNIP TX while PSI arrives concurrently |

---

## Also in this folder

- [todo.md](todo.md) — Current active task list (not an archive plan)
- [archive/OpenLcbCLib_OpenMRN_virtual_node_comparison_v3.md](archive/OpenLcbCLib_OpenMRN_virtual_node_comparison_v3.md) — Current-state comparison of OpenLcbCLib vs OpenMRN virtual node implementation (v3, 2026-03-17)
