<!--
  ============================================================
  STATUS: REJECTED
  TR090/100/110 are confirmed wired in the external OlcbChecker project. TR120/130 were explicitly deferred in the production release plan due to requiring a multi-node test harness not yet built; the plan is therefore only partially fulfilled.
  ============================================================
-->

# Plan: OlcbChecker Train Control Test Completion

## 1. Summary

The OlcbChecker suite has 8 train control tests (TR010-TR080) covering events, speed,
functions, emergency stop (point-to-point and global), memory spaces, and listener lifecycle.
Three major areas have zero coverage:

- **Controller Assignment** (assign, release, query, reject with Node ID)
- **Reserve/Release** (train management reservation)
- **Consist Forwarding** (listener command forwarding, P bit, emergency stop forwarding)

This plan adds 5 new test files (TR090-TR130) following the existing OlcbChecker conventions,
and updates `control_traincontrol.py` to include them.

**Target directory:** `/Users/jimkueneman/Documents/OlcbChecker/`

---

## 2. OlcbChecker Test Conventions

All tests follow these patterns (derived from check_tr080_listener.py):

- File: `check_trNNN_name.py` with a `check()` function
- Returns: `0` = pass, `2` = setup failure, `3` = test failure
- Setup: `olcbchecker.purgeMessages()`, `getTargetID()`, `gatherPIP()`, check PIP
- Messages: `Message(MTI.Traction_Control_Command, src, dest, [payload_bytes])`
- Replies: helper `getTrainControlReply(destination)` filters by MTI + source + dest
- Cleanup: `finally` block resets state regardless of pass/fail
- Logging: `logger.info("Passed")` / `logger.warning("Failure: ...")`
- Protocol bytes: defined as module-level constants

---

## 3. New Test Files

### TR090: Controller Configuration (`check_tr090_controller.py`)

**Standard:** TrainControlS Section 6.1, TrainControlTN Section 2.8

**Protocol bytes:**
```python
CONTROLLER_CONFIG = 0x20
CONTROLLER_ASSIGN = 0x01
CONTROLLER_RELEASE = 0x02
CONTROLLER_QUERY = 0x03
```

**Test sequence:**

1. **Query controller (no controller assigned)**
   - Send: `[0x20, 0x03]`
   - Expect reply: `[0x20, 0x03, flags, 0,0,0,0,0,0]` with flags=0x00 (no controller)
   - Verify Node ID is all zeros

2. **Assign controller (our node)**
   - Send: `[0x20, 0x01, 0x00] + our_node_id_bytes`
   - Expect reply: `[0x20, 0x01, 0x00]` (result=0, accepted)
   - Verify reply length is 3 bytes (no Node ID on accept)

3. **Query controller (our node should be assigned)**
   - Send: `[0x20, 0x03]`
   - Expect reply: `[0x20, 0x03, 0x01, our_node_id_bytes]`
   - Verify flags=0x01 (controller assigned)
   - Verify returned Node ID matches ours

4. **Re-assign same controller (should still accept)**
   - Send: `[0x20, 0x01, 0x00] + our_node_id_bytes`
   - Expect reply: `[0x20, 0x01, 0x00]` (result=0, accepted)

5. **Assign different controller (may accept or reject — both valid)**
   - Send: `[0x20, 0x01, 0x00] + fabricated_node_id_bytes`
   - Read reply:
     - If `result == 0x00`: accepted — query to verify new controller
     - If `result != 0x00`: rejected — **verify reply includes 6-byte Node ID of
       current controller** (our node). This is the Item #5 compliance check.
       Verify reply length >= 9 bytes. Verify bytes 3-8 match our Node ID.

6. **Release controller**
   - If step 5 was accepted: release the fabricated controller
   - If step 5 was rejected: release our own controller
   - Send: `[0x20, 0x02, 0x00] + releasing_node_id_bytes`
   - No reply expected per spec (release is fire-and-forget)

7. **Query controller (should be empty again)**
   - Send: `[0x20, 0x03]`
   - Verify flags=0x00, Node ID all zeros

**Cleanup (finally):** Release both our node and the fabricated node (ignoring errors).

**Validation coverage:**
- Controller assign accept flow
- Controller query with and without active controller
- Controller assign reject reply format (**Item #5: 6-byte Node ID in reject**)
- Controller release
- Re-assign same controller

---

### TR100: Reserve/Release (`check_tr100_reserve.py`)

**Standard:** TrainControlS Section 6.6, TrainControlTN Section 2.10

**Protocol bytes:**
```python
MANAGEMENT = 0x40
MGMT_RESERVE = 0x01
MGMT_RELEASE = 0x02
```

**Test sequence:**

1. **Reserve train (first time — should succeed)**
   - Send: `[0x40, 0x01]`
   - Expect reply: `[0x40, 0x01, 0x00]` (result=0, OK)

2. **Reserve train again (already reserved — should fail)**
   - Send: `[0x40, 0x01]`
   - Expect reply: `[0x40, 0x01, result]` with result != 0

3. **Release train**
   - Send: `[0x40, 0x02]`
   - No reply expected

4. **Reserve train (after release — should succeed again)**
   - Send: `[0x40, 0x01]`
   - Expect reply: `[0x40, 0x01, 0x00]`

**Cleanup (finally):** Send release.

**Validation coverage:**
- Reserve succeeds on first call
- Double-reserve rejected
- Release allows subsequent reserve

---

### TR110: Heartbeat (`check_tr110_heartbeat.py`)

**Standard:** TrainControlS Section 6.4, TrainControlTN Section 2.7

This test validates the heartbeat mechanism. The train node sends heartbeat requests
to the active controller. The controller must respond with a NOOP to keep the train alive.

**Protocol bytes:**
```python
MANAGEMENT = 0x40
MGMT_NOOP = 0x03
CONTROLLER_CONFIG = 0x20
CONTROLLER_ASSIGN = 0x01
CONTROLLER_RELEASE = 0x02
```

**Approach:**

Heartbeat testing is limited by the OlcbChecker's single-node architecture. The checker
can assign itself as controller and then listen for an unsolicited heartbeat request
(MTI_TRAIN_REPLY with [0x40, 0x03, timeout_bytes]) from the train node. However, the
heartbeat timeout period is implementation-defined and may be long (e.g., 30 seconds),
making this slow in a test suite.

**Test sequence:**

1. **Assign controller (our node)**
   - Send: `[0x20, 0x01, 0x00] + our_node_id_bytes`
   - Expect accept reply

2. **Send NOOP (heartbeat keepalive)**
   - Send: `[0x40, 0x03]` (train command NOOP)
   - No reply expected (NOOP is fire-and-forget when sent as a command)

3. **Wait for heartbeat request from train (up to 60 seconds)**
   - Listen for `MTI.Traction_Control_Reply` with `data[0] == 0x40, data[1] == 0x03`
   - If received: verify it contains timeout bytes
   - If timeout: log as informational skip (heartbeat may not be configured on the DUT)

4. **Respond with NOOP to keep alive**
   - Send: `[0x40, 0x03]`
   - The train should not enter emergency stop

5. **Release controller**

**Note:** This test is optional/informational since the heartbeat timeout period is
implementation-defined. If no heartbeat request is received within 60 seconds, the test
passes with an informational note. The test is mainly useful for implementations that have
short heartbeat periods configured.

**Cleanup (finally):** Release controller, set speed to zero.

---

### TR120: Consist Forwarding — Speed (`check_tr120_consist_speed.py`)

**Standard:** TrainControlS Section 6.5, TrainControlTN Section 2.6.5

This is the critical consist forwarding test. It requires the OlcbChecker's own node to be
registered as a listener on the train, then send speed commands and observe that the train
forwards them back to the checker (as a listener) with P=1.

**Architecture:**

The OlcbChecker has its own Node ID and alias on the bus. By attaching itself as a listener
on the DUT train node, any speed/function commands sent to the train should be forwarded
back to the checker — but the spec says forwarded messages skip the source. So we need a
different source for the original command.

**Problem:** The OlcbChecker has only one identity on the bus. If the checker sends a Set
Speed to the train AND is also a listener, the train must NOT forward back to the checker
(source-skip rule). We need a way to send from a different source.

**Solution options:**

**Option A (Recommended): Use a second fabricated alias.**
The OlcbChecker could reserve a second alias using the CAN alias negotiation protocol
(CID/RID/AMD sequence) and send commands from that alias. This is complex but correct.

**Option B: Accept the limitation and test what we can.**
Attach the checker as a listener. Send speed from the checker. Verify that the train does
NOT forward back to us (testing the source-skip rule). Then verify state via query.

**Option C: Use two physical checker nodes.**
Not practical for automated testing.

**Recommended test sequence (Option B — source-skip verification + state query):**

1. **Attach checker as listener with REVERSE flag**
   - Send: `[0x30, 0x01, 0x02] + our_node_id_bytes`
   - Expect attach reply OK

2. **Set speed to 0.5 forward**
   - Send: `[0x00, 0x38, 0x00]` (0.5 forward in float16)

3. **Verify no forwarded message arrives (source-skip rule)**
   - Wait briefly (500ms) for any message
   - Expect: no Traction_Control_Command arrives with P=1 (because we are the source
     and should be skipped)

4. **Query speed to verify train acted on it**
   - Send: `[0x10]`
   - Expect: speed = 0x38 0x00

5. **Detach checker as listener**

**If Option A is implemented (second alias), the full forwarding test becomes:**

1. Attach checkerB (second alias) as listener with REVERSE flag
2. Send Set Speed 0.5 forward from checkerA (first alias)
3. Wait for Traction_Control_Command at checkerB
4. Verify: instruction byte has P=1 (0x80), direction is reversed (0xB8 0x00)
5. Detach checkerB

**Note:** Option A requires CAN-level frame construction (CID/RID/AMD) which may not be
available in the current OlcbChecker infrastructure. Option B is implementable today and
still provides useful coverage of the source-skip rule.

**Cleanup (finally):** Detach listener, set speed to zero.

---

### TR130: Consist Forwarding — Functions and E-Stop (`check_tr130_consist_func.py`)

**Standard:** TrainControlS Section 6.5

Same architecture constraints as TR120. Using Option B (source-skip verification):

**Test sequence:**

1. **Attach checker as listener with LINK_F0 + LINK_FN flags**
   - Send: `[0x30, 0x01, 0x0C] + our_node_id_bytes` (flags = 0x04 | 0x08 = 0x0C)
   - Expect attach reply OK

2. **Set function F0 on**
   - Send: `[0x01, 0x00, 0x00, 0x00, 0x00, 0x01]`

3. **Verify no forwarded function message arrives (source-skip)**
   - Wait briefly, expect no Traction_Control_Command with P=1

4. **Query F0 to verify train acted on it**
   - Expect: F0 = 0x00 0x01

5. **Send Emergency Stop**
   - Send: `[0x02]`

6. **Verify no forwarded estop arrives (source-skip)**
   - Wait briefly, expect no Traction_Control_Command with P=1

7. **Query speed to verify estop took effect**
   - Expect: speed magnitude = 0

8. **Detach listener**

9. **Attach checker as listener WITHOUT LINK_F0 flag (only LINK_FN)**
   - Send: `[0x30, 0x01, 0x08] + our_node_id_bytes` (flags = 0x08, no LINK_F0)
   - Expect attach reply OK

10. **Set function F0 on — should NOT be forwarded (LINK_F0 not set)**
    - This verifies the flag filtering even though we can't observe the non-forwarding
      directly. The train should still act on it locally.

11. **Detach listener**

**Cleanup (finally):** Detach listener, set F0 off, set speed to zero.

---

## 4. Updates to `control_traincontrol.py`

Add imports:
```python
import check_tr090_controller
import check_tr100_reserve
import check_tr110_heartbeat
import check_tr120_consist_speed
import check_tr130_consist_func
```

Add to `prompt()`:
```python
print(" 9  Controller configuration checking")
print(" 10 Reserve/Release checking")
print(" 11 Heartbeat checking")
print(" 12 Consist speed forwarding checking")
print(" 13 Consist function/estop forwarding checking")
```

Add to `checkAll()`:
```python
logger.info("Controller configuration checking")
result += check_tr090_controller.check()

logger.info("Reserve/Release checking")
result += check_tr100_reserve.check()

logger.info("Heartbeat checking")
result += check_tr110_heartbeat.check()

logger.info("Consist speed forwarding checking")
result += check_tr120_consist_speed.check()

logger.info("Consist function/estop forwarding checking")
result += check_tr130_consist_func.check()
```

Add match/case entries in `main()`.

---

## 5. Coverage Matrix

| Feature | Existing Test | New Test | Notes |
|---------|--------------|----------|-------|
| isTrain event | TR010 | — | |
| Set/query speed | TR020 | — | |
| Set/query function | TR030 | — | |
| Emergency stop | TR040 | — | |
| Global E-stop | TR050 | — | |
| Global E-off | TR060 | — | |
| Memory spaces | TR070 | — | |
| Listener lifecycle | TR080 | — | |
| Controller assign accept | — | **TR090** | |
| Controller query | — | **TR090** | |
| Controller assign reject + Node ID | — | **TR090** | **Item #5** |
| Controller release | — | **TR090** | |
| Reserve (first) | — | **TR100** | |
| Double-reserve reject | — | **TR100** | |
| Release + re-reserve | — | **TR100** | |
| Heartbeat request | — | **TR110** | Optional/informational |
| Consist source-skip (speed) | — | **TR120** | Source-skip only |
| Consist source-skip (function) | — | **TR130** | Source-skip only |
| Consist source-skip (estop) | — | **TR130** | Source-skip only |
| Flag filtering (LINK_F0/FN) | — | **TR130** | Indirect (no observer) |

### What remains untestable from a single OlcbChecker node:

| Feature | Reason |
|---------|--------|
| P bit on forwarded messages | Requires observing messages to a *different* listener node |
| Direction reversal on forwarded speed | Same — requires second observer |
| Chained consist forwarding | Requires 3+ nodes |
| Controller Changed Notify to old controller | Requires second controller node |
| Heartbeat timeout → estop | Requires waiting for full timeout + querying state |

These would require either a second alias identity on the bus (CAN-level frame injection)
or a multi-node test harness. They are noted as future enhancements.

---

## 6. Implementation Order

| Priority | File | Effort | Dependencies |
|----------|------|--------|--------------|
| 1 | `check_tr090_controller.py` | Medium | None — tests existing handler |
| 2 | `check_tr100_reserve.py` | Small | None — tests existing handler |
| 3 | `check_tr120_consist_speed.py` | Medium | Plan #3 must be implemented first |
| 4 | `check_tr130_consist_func.py` | Medium | Plan #3 must be implemented first |
| 5 | `check_tr110_heartbeat.py` | Small | Existing handler, but long timeout |
| 6 | Update `control_traincontrol.py` | Small | After all test files exist |

**TR090 and TR100** can be written and run immediately — they test existing protocol
handlers that are already implemented. TR090 will initially fail the reject-with-Node-ID
check until Plan #5 is implemented, which is useful as a regression test.

**TR120 and TR130** should be written after Plan #3 (consist forwarding) is implemented,
since there's nothing to test until forwarding exists.

---

## 7. File Summary

| File | Status |
|------|--------|
| `check_tr090_controller.py` | **New** — controller assign/release/query |
| `check_tr100_reserve.py` | **New** — reserve/release management |
| `check_tr110_heartbeat.py` | **New** — heartbeat keepalive (informational) |
| `check_tr120_consist_speed.py` | **New** — consist speed forwarding (source-skip) |
| `check_tr130_consist_func.py` | **New** — consist function/estop forwarding (source-skip) |
| `control_traincontrol.py` | **Modified** — add 5 new tests to runner and menu |
