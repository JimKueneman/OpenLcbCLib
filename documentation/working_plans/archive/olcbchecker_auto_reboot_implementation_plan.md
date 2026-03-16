<!--
  ============================================================
  STATUS: IMPLEMENTED
  `run_olcbchecker.sh` passes `--auto-reboot` to the test runner, and `OSxDrivers_reboot()` in `ComplianceTestNode` calls `OpenLcbNode_reset_state()`, enabling programmatic restart.
  ============================================================
-->

# Implementation Plan: Add `auto_reboot` Flag to OlcbChecker

## Context

OlcbChecker's `-i` flag (`skip_interactive`) skips all tests that involve node restart — both manual-prompt tests and the fully programmatic restart test (`check_mc50_restart.py`). The agreed approach with the OlcbChecker developer is two separate flags: `skip_interactive` for operator-dependent tests, and `auto_reboot` for programmatic restart via the Memory Configuration restart datagram (`[0x20, 0xA9]`).

## Design Decision: `check_fr10_init.py` Excluded

Per feedback from the OlcbChecker author, `check_fr10_init.py` is intentionally **excluded** from `auto_reboot`. That test operates at the CAN frame level — Memory Configuration operations are three abstraction levels above frames. Sending a message-level restart datagram inside a frame-level test would violate the layered test architecture. `fr10` remains gated on `skip_interactive` only and can only run via manual operator restart.

## Files to Modify

All files are in the OlcbChecker project directory.

### 1. `defaults.py` — Add default value  ✅ DONE

`auto_reboot = False` already present (line 23).

### 2. `configure.py` — Wire up the new flag  ✅ DONE

Four changes applied:

**Stage 1 — Copy from defaults:**
```python
auto_reboot = defaults.auto_reboot
```

**Stage 2 — Local overrides:**
```python
if 'auto_reboot' in dir(localoverrides) : auto_reboot = localoverrides.auto_reboot
```

**Stage 3 — CLI argument parsing:**
Added `"auto-reboot"` to the long options list in `getopt.getopt()`.

**Opt processing loop:**
```python
elif opt == "--auto-reboot":
    auto_reboot = True
```

**`options()` function — Help text:**
```python
print ("--auto-reboot  use restart datagram for reboot tests instead of prompting operator")
```

### 3. `check_mc50_restart.py` — Guard with `auto_reboot` instead of `skip_interactive`  ✅ DONE

Added an `auto_reboot`-aware guard before the PIP check:

```python
    if not olcbchecker.setup.configure.auto_reboot :
        if olcbchecker.setup.configure.skip_interactive :
            logger.info("Interactive check skipped")
            return 0
```

Logic: if `auto_reboot` is on, always run (it already sends the datagram). If `auto_reboot` is off, fall back to `skip_interactive` behavior.

### 4. `check_me10_init.py` — Add programmatic restart path  ✅ DONE

This test already imports `Message`, `MTI`, `NodeID`. Replaced the `skip_interactive` guard + manual prompt with:

```python
    if olcbchecker.setup.configure.auto_reboot :
        message = Message(MTI.Datagram, NodeID(olcbchecker.ownnodeid()),
                          destination, [0x20, 0xA9])
        olcbchecker.sendMessage(message)
    else :
        if olcbchecker.setup.configure.skip_interactive :
            logger.info("Interactive check skipped")
            return 0
        print("Please reset/restart the device being checked now")
```

The existing `Initialization_Complete` wait loop handles the rest.

### 5. `check_ev50_ini.py` — Add programmatic restart path  ✅ DONE

Same pattern as me10. This test already imports `Message`, `MTI`, `NodeID`.

```python
    if olcbchecker.setup.configure.auto_reboot :
        message = Message(MTI.Datagram, NodeID(olcbchecker.ownnodeid()),
                          destination, [0x20, 0xA9])
        olcbchecker.sendMessage(message)
    else :
        if olcbchecker.setup.configure.skip_interactive :
            logger.info("Interactive check skipped")
            return 0
        print("Please reset/restart the checked node now")
```

### 6. `check_bt80_startup.py` — Add programmatic restart path  ✅ DONE

Same pattern as me10/ev50. This test already imports `Message`, `MTI`, `NodeID`.

```python
    if olcbchecker.setup.configure.auto_reboot :
        message = Message(MTI.Datagram, NodeID(olcbchecker.ownnodeid()),
                          destination, [0x20, 0xA9])
        olcbchecker.sendMessage(message)
    else :
        if olcbchecker.setup.configure.skip_interactive :
            logger.info("Interactive check skipped")
            return 0
        print("Please reset/restart the clock producer node now")
```

### 7. `check_me50_dup.py` — No change

This test's `skip_interactive` usage is in the exception handler (fallback when automated event detection fails). It requires the operator to observe a physical indication on hardware. It stays on `skip_interactive` only. `auto_reboot` does not apply here.

### 8. `check_fr10_init.py` — No change (excluded)

Frame-level test. Cannot use message-level restart datagram. Stays on `skip_interactive` only.

### 9. Update `run_olcbchecker.sh` — Pass `--auto-reboot`  ✅ ALREADY DONE

The shell script already has `--auto-reboot` CLI flag support and pass-through logic:
```bash
CHECKER_FLAGS="-i"
if [ "$AUTO_REBOOT" = true ]; then
    CHECKER_FLAGS="$CHECKER_FLAGS --auto-reboot"
fi
```

## How the Two Flags Work Together

| `auto_reboot` | `skip_interactive` | Restart tests (mc50, me10, ev50, bt80) | Frame init (fr10) | Visual tests (me50) |
|---|---|---|---|---|
| `True` | `True` | Run — send restart datagram | Skip | Skip |
| `True` | `False` | Run — send restart datagram | Run — prompt operator | Run — prompt operator |
| `False` | `False` | Run — prompt operator | Run — prompt operator | Run — prompt operator |
| `False` | `True` | Skip (current behavior) | Skip | Skip |

## Tests Unlocked by `--auto-reboot`

| Test | Section | What It Verifies |
|------|---------|-----------------|
| `check_mc50_restart.py` | Memory Configuration | Send restart datagram → Initialization Complete received |
| `check_me10_init.py` | Message Network | Initialization Complete message has correct source, data length, node ID |
| `check_ev50_ini.py` | Event Transport | Startup event identification matches addressed event identification |
| `check_bt80_startup.py` | Broadcast Time Producer | Producer/Consumer Range Identified + full sync sequence after startup |

## Usage Examples

```bash
# Automated CI — programmatic restart, skip visual-confirmation tests:
./run_olcbchecker.sh --auto-reboot

# With verbose output:
./run_olcbchecker.sh --verbose --auto-reboot --all

# Manual testing with physical hardware — all interactive prompts enabled:
python3 run_tests.py -a /dev/ttyUSB0 -t "05.07.01.01.00.33"

# Automated but no restart support — skip all restart tests (current behavior):
./run_olcbchecker.sh
```

## Verification

1. Run `./run_olcbchecker.sh --auto-reboot` — all 4 restart tests should pass
2. Verify `check_mc50_restart` actually executes (not skipped) by checking for "Passed" in Memory Configuration
3. Verify `check_me10_init`, `check_ev50_ini`, `check_bt80_startup` also run and pass
4. Verify `check_fr10_init` is still skipped (logged as "Interactive check skipped")
5. Verify `check_me50_dup` still uses its existing `skip_interactive` behavior (unchanged)
