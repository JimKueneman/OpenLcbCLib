<!--
  ============================================================
  STATUS: IMPLEMENTED
  The 64-test coverage analysis is reflected in the integration suite; `run_olcbchecker.sh` controls test sections via `RUN_SECTIONS`, and all referenced new test scripts are wired into the runner.
  ============================================================
-->

# OlcbChecker Test Coverage Analysis

**Date:** 9 March 2026 (updated)
**Scope:** All OlcbChecker tests vs. ComplianceTestNode capabilities
**Goal:** Identify tests we can add to improve compliance coverage

---

## Current Test Inventory

### Core Compliance (always runs) — 7 sub-sections, 32 individual checks

| Section | Tests | What They Cover |
|---------|-------|-----------------|
| Frame Transport | fr10–fr60 | Initialization (requires `--auto-reboot`), AME, CAN collision, reserved bit, capacity, standard frames |
| Message Network | me10–me60 | Initialization, Verify Node, PIP, Optional Interaction Rejected, Duplicate Node ID, PIP Simple bit consistency |
| SNIP | sn10 | Simple Node Information Protocol query/reply |
| Event Transport | ev10–ev70 | Identify Addressed/Global, Identify Producer/Consumer, Initial Advertisement, Events With Payload, Learn Event |
| Datagram Transport | da30, da40 | Datagram Received, Unknown protocol rejection |
| Memory Configuration | mc10–mc80 | Config Options, Address Space Info, Read, Lock/Reserve, Restart, Out-of-range read, Write under mask, Basic write |
| CDI | cd10–cd40 | Validate XML, Read entire CDI, ACDI checking, ACDI write-back roundtrip |

### Broadcast Time Consumer — 4 checks

| Test | What It Covers |
|------|----------------|
| bt100 | Consumer startup — sends Query, verifies clock subscribes |
| bt110 | Consumer synchronization — verifies consumer tracks time reports |
| bt120 | Consumer stop/start — verifies consumer responds to Start/Stop events |
| bt130 | Consumer rate change — verifies consumer handles rate changes |

### Broadcast Time Producer — 8 checks

| Test | What It Covers |
|------|----------------|
| bt10 | Clock Query produces sync sequence (6 messages in order) |
| bt20 | Clock Set updates time, produces sync sequence |
| bt30 | Set immediate Report event within 1 sec before delayed sync |
| bt40 | Multiple sets within 3 sec single sync sequence |
| bt50 | Report Time frequency |
| bt60 | Requested time via Query event |
| bt70 | Date rollover at midnight |
| bt80 | Startup sequence (requires `--auto-reboot`) |

### Train Protocol — 20 checks across 3 sub-sections

| Sub-section | Tests | What They Cover |
|-------------|-------|-----------------|
| Train Control | tr010–tr110 | isTrain event, speed set/query, function set/query, emergency stop, global estop, global eoff, memory spaces 0xFA/0xF9, listener lifecycle, controller assign/release/query, reserve/release, heartbeat keep-alive |
| Train Search | ts10–ts40 | Create/allocate train, partial matches, reserved address |
| FDI | fd10–fd30 | FDI XML validation, FDI complete read, FDI read-only flag |

### Total: 64 individual check scripts

---

## Tests Requiring `--auto-reboot`

OlcbChecker already supports `--auto-reboot` via `configure.py`. Our
`run_olcbchecker.sh` already passes the flag through. Four of the five
tests below have auto-reboot support built in; `check_fr10_init.py` does
not yet.

| Test | Auto-reboot support in OlcbChecker? | Status |
|------|--------------------------------------|--------|
| me10 (init) | Yes | Ready |
| ev50 (init advertisement) | Yes | Ready |
| bt80 (startup) | Yes | Ready |
| mc50 (restart) | Yes | Ready |
| fr10 (init) | No — still interactive only | Waiting on OlcbChecker |

**Action:** Run with `--auto-reboot` to unlock 4 tests now. Monitor
OlcbChecker for `check_fr10_init.py` auto-reboot support.

---

## Tests Added by OpenLcbCLib Project

The following tests were written specifically for OpenLcbCLib compliance
coverage and contributed to the OlcbChecker suite.

| Test | Category | What It Tests |
|------|----------|---------------|
| `check_me60_pip_simple` | Message Network | PIP Simple bit vs Verified Node ID MTI consistency |
| `check_da40_unknown_protocol` | Datagram | Unknown datagram protocol byte rejection |
| `check_mc60_out_of_range` | Memory Config | Read beyond highest address returns fail |
| `check_mc70_write_under_mask` | Memory Config | Write Under Mask command (requires `-w`) |
| `check_mc80_write` | Memory Config | Basic config memory write/verify/restore (requires `-w`) |
| `check_cd40_acdi_writeback` | CDI/ACDI | Full ACDI conformance: PIP, Config Options, Address Space Info, SNIP comparison, write roundtrip, negative writes |
| `check_fd30_readonly` | FDI | FDI address space read-only flag |
| `check_tr090_controller` | Train Control | Controller Assign/Release/Query lifecycle |
| `check_tr100_reserve` | Train Control | Reserve/Release management |
| `check_tr110_heartbeat` | Train Control | Heartbeat keep-alive (optional per spec — passes gracefully if not implemented) |

---

## Remaining Coverage Gaps

The gaps below were identified from the OlcbChecker backlog at
`/Users/jimkueneman/Documents/OlcbChecker/ChecksToAdd.md`.  Each gap
references the specific item(s) in that document.

### Gap 1: Train Control §10 — Controller Changed Notify

**Source:** `ChecksToAdd.md` line 46 — "Code the Train Control checks
for subsection 8, 9, 10, 11" (subsections 8 and 9 are done; 10 remains)

**Spec reference:** TrainControlS section 6.1 (Controller Changed Notify)

When a second controller attempts to assign itself while one is already
assigned, the train should notify the current controller via Controller
Changed Notify. Our library implements this (`on_controller_assign_request`
callback, `protocol_train_handler.c`).

**Action:** Write `check_tr120_controller_notify.py` — requires a test
that simulates a second controller attempting takeover and verifies the
existing controller receives the notification.

**Complexity:** Medium — requires the checker to act as two distinct
source nodes (or use a helper alias).

### Gap 2: Multi-PIP / Multi-SNIP Capacity

**Source:** `ChecksToAdd.md` line 3 — "Add multi-PIP and multi-SNIP cases
to message capacity check"; also lines 40-41 — "The three-PIP message
part" and "The three-SNIP message part"

Currently `check_me30_pip.py` tests a single PIP request. The standard
allows PIP responses up to 3 frames (24 bytes).

**Our readiness:** Our library sends 6-byte PIP responses. Multi-frame
path exists but isn't exercised.

**Action:** Low priority. Could implement if needed.

### Gap 3: Datagram Error and Overlap Handling

**Source:** `ChecksToAdd.md` lines 7-11 — "Check datagram error and
overlap handling" (four sub-items; unknown-protocol sub-item is done
as `check_da40`)

Remaining sub-tests:
- Overlapping datagrams from two sources
- Incorrect first-middle-last ordering error, then valid datagram
- Node without PIP Datagram declaration

**Our readiness:** Our library handles datagram framing and error cases.

**Action:** Implement remaining datagram error tests as needed.

### Gap 4: CDI Section 2.4

**Source:** `ChecksToAdd.md` line 45 — "CDI section 2.4 not implemented"

Covers segment definitions and valid element structure.

**Action:** Implement when ready.

### Gap 5: Firmware Update Checks

**Source:** `ChecksToAdd.md` line 38 — "Implement the Firmware Update
check plan"

We declare `PSI_FIRMWARE_UPGRADE` in PIP and configure memory space 0xEF,
but no real firmware handler exists. No OlcbChecker firmware tests exist
yet either.

**Action:** Requires implementing a test-mode firmware handler in the
ComplianceTestNode before the test script can be written.

### Gap 6: Multi-Alias Node Checks (Deferred)

**Source:** `ChecksToAdd.md` lines 27-30 — "From discussion of a node
that reserves multiple aliases"

**Status:** Deferred. There is no universal way to force a node to
allocate multiple aliases. Multi-alias creation is application-specific
(virtual command station via Train Search, gateway on startup, consist
controller). The only generic approach would be to observe AMD frames
already present at startup, but this depends on the node's configuration
and protocol mode. See `OLCBChecker_New_Tests.md` for full rationale.

---

## Summary Table

| Category | Count | Notes |
|----------|-------|-------|
| Total check scripts | 64 | All tests in OlcbChecker |
| Tests passing (core + all modes) | ~59 | With `-i` flag (skipping reboot tests) |
| Tests unlockable with `--auto-reboot` | 4 | me10, ev50, bt80, mc50 |
| fr10 auto-reboot support | 1 | Needs auto-reboot support added to check_fr10_init.py |
| Remaining gaps (new tests to write) | 5 | TC §10 Controller Changed Notify, multi-PIP/SNIP, datagram errors, CDI §2.4, firmware |
| Deferred | 1 | Multi-alias (no generic trigger) |
| Deprecated | 1 | Traction Proxy (protocol deprecated) |

---

## Recommended Next Steps

1. **Run with `--auto-reboot`** — Unlocks 4 skipped tests (me10, ev50,
   bt80, mc50) with no code changes.

2. **Write `check_tr120_controller_notify.py`** — The only remaining gap
   where we have library support but no test.

3. **Implement remaining gaps** — Datagram error tests, CDI §2.4, and
   firmware checks as priorities allow. Our node already supports the
   datagram and CDI paths; firmware needs a test-mode handler first.
