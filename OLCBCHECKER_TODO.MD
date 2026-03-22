# OlcbChecker — Remaining Work

Moved from main `TODO.MD`.

---

## OPEN — Active / Future Work

---

### 1. OlcbChecker Consist Forwarding Tests (TR120, TR130)

**Issue:** TR120 (Consist Speed Forwarding source-skip) and TR130 (Consist Function/EStop Forwarding source-skip) require a second alias identity on the bus — a multi-node test harness not currently available.

**Status:** Deferred pending multi-node test harness capability.

**Detail:** See `archive/plan_olcbchecker_train_control_tests.md` and `OlcbChecker/ChecksToAdd.md`.

---

### 2. OlcbChecker — Additional Checks from ChecksToAdd.md

Outstanding items in `OlcbChecker/ChecksToAdd.md` not yet implemented:
- Multi-PIP and multi-SNIP message capacity cases
- PIP Simple bit vs Verify Node ID Simple bit cross-check
- Datagram error and overlap handling (4 cases)
- Memory Configuration error handling (section 4.3)
- ACDI user space writeback verification
- Train Control subsection 8–11 detailed plan and implementation
- Memory Configuration client-side checking (dummy server)
- Multi-alias AMD response checks
- Firmware Update check plan implementation
- Broadcast Time check plan completion (check_bt scripts exist but plan notes gaps)
- Three-PIP and three-SNIP message cases
- Frame section 3 message level checks
- FDI read-only address-space reply handling

---

### 3. Implement Stream Transport Tests

**Status:** Deferred — Stream Transport specification has not been officially released by the OpenLCB standards group.

**Scope:** Once the spec is stable and the library stream handler is implemented, create OlcbChecker test suites for the Stream Transport protocol.

---
