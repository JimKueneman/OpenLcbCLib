<!--
  ============================================================
  STATUS: IMPLEMENTED
  All three library bug fixes are confirmed in source (write-under-mask byte order, controller config node ID offset, reset/reboot datagram flow). The integration test runner in `test/olcbchecker_bridge/` wires up the new OlcbChecker test scripts.
  ============================================================
-->

# OLCBChecker New Tests

New check scripts and infrastructure changes added to the OlcbChecker test
suite.  All scripts live in `/Users/jimkueneman/Documents/OlcbChecker/`
alongside Bob Jacobsen's original checks.

---

## New Test Scripts

### check_cd40_acdi_writeback.py

**Category:** CDI / ACDI
**Spec references:** ConfigurationDescriptionInformationS section 5.1.2,
MemoryConfigurationS section 4.14

Comprehensive ACDI conformance test covering consistency between PIP, Configuration
Options, Address Space Information, SNIP, and the ACDI address spaces (0xFC and 0xFB).

**Steps:**

1. **PIP check** — Verifies ACDI and Memory Configuration are both present in PIP.
2. **Configuration Options flags** — Reads the Options reply and checks that the
   three ACDI flags (0x0800 Manufacturer Read, 0x0400 User Read, 0x0200 User Write)
   are all set consistently.  Also verifies that spaces 0xFC and 0xFB fall within
   the highest/lowest address space range reported in the Options reply.
3. **Address Space Info** — For each ACDI space flagged as readable, queries Address
   Space Information and verifies: present, read-only (0xFC always; 0xFB only when
   User Write is not advertised), highest address (124 for 0xFC, 127 for 0xFB), and
   lowest address (0 for both).
4. **SNIP comparison** — Reads all fields from 0xFC (manufacturer, model, hw version,
   sw version) and 0xFB (user name, user description) and compares each against the
   corresponding SNIP value.  Also compares the version bytes: ACDI 0xFC address 0
   vs SNIP manufacturer version, ACDI 0xFB address 0 vs SNIP user version.
5. **Writeback roundtrip** — If User Write is flagged, saves the current user name
   from 0xFB address 1, writes the test string "OLCB_TST", reads it back and
   verifies, then restores the original value.
6. **Negative: write to read-only 0xFC** — Attempts a write to the manufacturer
   space and verifies the node rejects it (either Datagram Rejected or Write Reply
   Fail).
7. **Negative: write to 0xFB version byte** — Attempts a write to 0xFB address 0
   (the version byte) and verifies the node rejects it.

---

### check_da40_unknown_protocol.py

**Category:** Datagram Transport
**Spec references:** DatagramTransportS section 6

Sends a datagram whose first byte is not a recognized protocol identifier and
verifies the node rejects it with a permanent error code (0x1xxx range).

---

### check_fd30_readonly.py

**Category:** FDI
**Spec references:** TrainControlS section 7.2

Queries the Address Space Information for the FDI space (0xFA) and verifies
it is flagged as read-only.

---

### check_mc60_out_of_range.py

**Category:** Memory Configuration
**Spec references:** MemoryConfigurationS sections 4.3, 4.5

Sends a read request to an address beyond the highest address reported by
the address space and verifies the node returns a Read Reply with the Fail
bit set and an appropriate error code (0x1082 permanent out-of-bounds).

---

### check_mc70_write_under_mask.py

**Category:** Memory Configuration
**Spec references:** MemoryConfigurationS section 4.10
**Requires:** `--force-writes` (`-w`)

Tests the Write Under Mask command.  Only runs if the Configuration Options
reply indicates write-under-mask support.  Uses alternating (mask, data) byte
pairs to selectively modify bits in config memory, then reads back to
verify the mask was applied correctly (mask=1 bits written, mask=0 bits
preserved).  Skipped unless `-w` is specified because it writes to 0xFD.

---

### check_mc80_write.py

**Category:** Memory Configuration
**Spec references:** MemoryConfigurationS sections 4.3, 4.5, 4.14
**Requires:** `--force-writes` (`-w`)

Basic config memory (0xFD) write test.  Queries Address Space Information to
determine boundaries and writability, then performs a read-write-verify-restore
cycle on a single byte.

**Steps:**

1. **PIP check** — Verifies Memory Configuration Protocol is present in PIP.
2. **Force-writes guard** — Skips with "Passed" if `-w` is not specified.
3. **Address Space Info** — Queries 0xFD space: verifies present (0x87),
   extracts highest_address, checks read-only flag, extracts low_address
   if the valid flag is set.
4. **Read original** — Reads one byte at the low address.
5. **Write test pattern** — Writes the bitwise complement of the original.
6. **Verify write** — Reads back and confirms the complement was stored.
7. **Restore original** — Writes the original value back.
8. **Verify restore** — Reads back and confirms the original was restored.

---

### check_me60_pip_simple.py

**Category:** Message Network
**Spec references:** MessageNetworkS section 4.1.4

Checks that the PIP Simple Protocol subset bit is consistent with the MTI
used by the node.  Per the spec, Simple nodes use Init Complete 0x0101 and
Verified Node ID 0x0171; Full nodes use 0x0100 and 0x0170.

---

### check_tr090_controller.py

**Category:** Train Control
**Spec references:** TrainControlS section 6.1

Tests the Controller Assign/Release/Query lifecycle:

1. **Query with no controller** — Sends Query [0x20, 0x03] and verifies
   flags = 0x00 (no controller assigned).
2. **Assign controller** — Sends Assign [0x20, 0x01, 0x00] with our Node ID
   and verifies reply [0x20, 0x01, 0x00] (accepted).
3. **Query with controller** — Sends Query and verifies flags has bit 0 set
   plus our Node ID is returned.
4. **Release controller** — Sends Release [0x20, 0x02, 0x00] with our
   Node ID.
5. **Query after release** — Verifies flags = 0x00 again.

---

### check_tr100_reserve.py

**Category:** Train Control
**Spec references:** TrainControlS section 4.3

Tests the Train Control Reserve/Release management commands:

1. **First reserve** — Sends Reserve and expects success (result byte 0x00).
2. **Same-source re-reserve** — Sends Reserve again from the same node and
   expects success (per TrainControlS, a second reserve from the same source
   shall be accepted).
3. **Release** — Sends Release and expects success.
4. **Reserve after release** — Sends Reserve again and expects success
   (verifying the release actually freed the reservation).

---

### check_tr110_heartbeat.py

**Category:** Train Control
**Spec references:** TrainControlS section 6.6

Tests the Heartbeat / Keep-alive mechanism:

1. **Assign controller** — Prerequisite setup.
2. **Set speed** — Set speed to a non-zero value (required: spec says trains
   shall not initiate heartbeat when Set Speed is zero).
3. **Wait for Heartbeat Request** — Expect Management Reply [0x40, 0x03,
   timeout] from the train node.  Extract the timeout value.  If no
   heartbeat is received within 10 seconds, pass with "optional per spec".
4. **Keepalive test** — Send No-op [0x40, 0x03] before deadline expires
   and verify speed is still non-zero.
5. **Wait for second Heartbeat Request** — Verify counter was reset by
   the keepalive.
6. **Timeout test** — Do NOT send keepalive and wait for the full heartbeat
   timeout to expire.
7. **Query speed** — Verify speed is 0 (train emergency-stopped due to
   missed heartbeat).  Optionally checks E-Stop status bit.
8. **No heartbeat when speed is zero** — Speed is already zero from step 7.
   Wait past the heartbeat timeout and verify no Heartbeat Request is sent
   (per TrainControlS 6.6: "Trains shall not initiate a Heartbeat Request
   if the last Set Speed is zero").
9. **No heartbeat after Emergency Stop** — Set speed non-zero, wait for a
   heartbeat to confirm the counter is active, send Emergency Stop, then
   verify no heartbeat is sent (per TrainControlS 6.6: trains in Emergency
   Stop state shall not send heartbeat).
10. **Speed retained after controller release** — Set speed non-zero,
    release controller, wait past the heartbeat timeout, re-assign
    controller, query speed.  Verify speed is still non-zero (per
    TrainControlS 6.6: "In case there is no assigned Controller node,
    the Train Node shall continue operating as last commanded").

Note: This test is inherently slow since it must wait for real timeouts.

---

## Modified Existing OlcbChecker Files

### --auto-reboot Support

Added `--auto-reboot` flag to `configure.py` and `defaults.py`.  When set,
tests that need a node restart send the Memory Configuration Reset/Reboot
datagram (`[0x20, 0xA9]`) instead of prompting the operator.

**Files modified:**

- `defaults.py` — Added `auto_reboot = False` default
- `configure.py` — Added `--auto-reboot` CLI option and `localoverrides.py`
  support
- `check_me10_init.py` — Sends reboot datagram when `auto_reboot` is set
- `check_mc50_restart.py` — Sends reboot datagram when `auto_reboot` is set
- `check_ev50_ini.py` — Sends reboot datagram when `auto_reboot` is set
- `check_bt80_startup.py` — Sends reboot datagram when `auto_reboot` is set

### --force-writes (-w) Flag

Added `--force-writes` / `-w` flag to `configure.py` and `defaults.py`.
Tests that physically write to config memory space 0xFD are dangerous on real
hardware — they can corrupt user configuration.  These tests are opt-in via
this flag.

**Files modified:**

- `defaults.py` — Added `force_writes = False` default
- `configure.py` — Added `-w` / `--force-writes` CLI option and
  `localoverrides.py` support
- `olcbchecker/__init__.py` — Added `isForceWrites()` helper function

When the flag is **not** set (default):
- `check_mc70_write_under_mask.py` and `check_mc80_write.py` skip with
  "Passed - skipped (use --force-writes to enable write tests)"
- The interactive menu in `control_memory.py` annotates these items with
  "(requires -w)"

When the flag **is** set:
- Both tests execute normally
- Menu items appear without annotation

The flag can also be set permanently in `localoverrides.py`:
```python
force_writes = True
```

### Controller Menu Registrations

Each new test script was registered in its parent controller for both the
`checkAll()` sequence and the interactive menu:

- `control_cdi.py` — Added `check_cd40_acdi_writeback`
- `control_datagram.py` — Added `check_da40_unknown_protocol`
- `control_fdi.py` — Added `check_fd30_readonly`
- `control_memory.py` — Added `check_mc60_out_of_range`,
  `check_mc70_write_under_mask`, `check_mc80_write`
- `control_message.py` — Added `check_me60_pip_simple`
- `control_traincontrol.py` — Added `check_tr090_controller`,
  `check_tr100_reserve`, `check_tr110_heartbeat`

### FDI Read Fix

- `check_fd20_read.py` — Minor fix for FDI read test

---

## Library Bug Fixes Discovered Through Testing

Running the new OlcbChecker tests against the ComplianceTestNode revealed
three spec-compliance bugs in the OpenLcbCLib library:

### 1. Write-Under-Mask Byte Order (protocol_config_mem_write_handler.c)

**Found by:** `check_mc70_write_under_mask.py`
**Spec:** MemoryConfigurationS section 4.10

The library treated the write-under-mask payload as split halves — N data
bytes followed by N mask bytes.  The spec requires interleaved (Mask, Data)
byte pairs: `[Mask0, Data0, Mask1, Data1, ...]`.

**Symptom:** `expected 0xAF, got 0xEF` — masking formula used wrong bytes.

**Fix:** Rewrote `_write_data_under_mask()` to iterate interleaved pairs.
Updated all unit tests to use the interleaved format.

### 2. Controller Config Command Node ID Offset (protocol_train_handler.c)

**Found by:** `check_tr090_controller.py`
**Spec:** TrainControlS section 6.1 (page 4)

The library extracted the controller Node ID from payload byte offset 2,
but the spec format is `[0x20, sub_cmd, Flags, NodeID[0..5]]` — the Flags
byte at offset 2 was being consumed as the first byte of the Node ID.

**Symptom:** `controller node ID mismatch: got ['0x0', '0x3', ...] expected
['0x3', '0x0', ...]` — shifted by one byte.

**Fix:** Changed offset from 2 to 3 in `_handle_controller_config()` for
ASSIGN, RELEASE, and CHANGED sub-commands.  Updated 19 unit test locations
to include the flags byte at offset 2 and move the Node ID to offset 3.

### 3. Reset/Reboot Datagram ACK (protocol_config_mem_operations_handler.c)

**Found by:** `check_me10_init.py` (with `--auto-reboot`)
**Spec:** MemoryConfigurationS section 4.24

The library sent a Datagram Received OK before rebooting.  The spec says:
"The receiving node may acknowledge this command with a Node Initialization
Complete instead of a Datagram Received OK response."

**Symptom:** `received unexpected message type: Datagram_Received_OK` during
Node Initialized checking — the checker saw the unexpected ack before the
expected Initialization Complete.

**Fix:** Changed `ProtocolConfigMemOperationsHandler_reset_reboot()` from the
two-phase ACK-then-execute pattern to a single-phase dispatch that skips the
datagram ack entirely.  The Initialization Complete from the reboot serves as
acknowledgment.

---

## Planned — Phase 2 Tests

The following tests are planned but not yet implemented.

---

### check_fr70_multi_alias.py (DEFERRED)

**Category:** Frame Transport
**Spec references:** CanFrameTransferS section 6.2.2, section 6.2.5
**Status:** Deferred — no generic trigger mechanism

There is no universal way to force a node to allocate multiple aliases.
Multi-alias creation is entirely application-specific:

- A virtual command station creates nodes via Train Search (dynamic allocation)
- A gateway creates them on startup based on its configuration
- A consist controller creates them when building a consist

The only generic approach would be to observe AMD frames already present at
startup, but this depends on the node's configuration and protocol mode.
For the ComplianceTestNode in `--train` mode, Train Search can trigger
dynamic allocation, but that couples this test to Train Search rather than
testing frame-level alias management in isolation.

Multi-alias correctness is better verified through the library's unit tests
(`protocol_train_handler_Test.cxx`, `canbus_statemachine_Test.cxx`) which
can directly control node creation and inspect alias state.

**Original test idea (for reference):**

1. **Observe AMD frames** — During startup, capture all AMD (Alias Map
   Definition) frames from the device under test.
2. **Unique Node IDs** — Verify every AMD frame carries a distinct Node ID.
3. **Unique aliases** — Verify every AMD frame carries a distinct CAN alias.
4. **Collision detection** — Send a CID (Check ID) frame using one of the
   virtual node aliases and verify the device responds with RID (Reserved ID).
5. **Repeat collision** — Send a CID using a different virtual node alias
   and verify RID response — confirming the device monitors collisions on
   all reserved aliases, not just the primary.
