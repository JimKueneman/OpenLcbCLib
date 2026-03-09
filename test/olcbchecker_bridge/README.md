# OlcbChecker Integration Test

Automated compliance testing of the OpenLcbCLib library against the
[OlcbChecker](https://github.com/openlcb/OlcbChecker) test suite.

## What it does

1. Copies the current library source into the Xcode BasicNode demo application
2. Compiles the demo via `xcodebuild`
3. Starts a TCP bridge server that connects the BasicNode to OlcbChecker
4. Runs the OlcbChecker compliance suite
5. Reports pass/fail results

## Prerequisites

- macOS with Xcode command-line tools installed
- Python 3.10+ (required by OlcbChecker)
- OlcbChecker installed at `/Users/jimkueneman/Documents/OlcbChecker`
  (change `CHECKER_DIR` in `run_olcbchecker.sh` and `OLCBCHECKER_DIR` in
  `bridge_server.py` if your path differs)
- Python packages: `xmlschema` (for CDI validation)

## Usage

```bash
# Run core tests + broadcast time (default)
./run_olcbchecker.sh

# Show GridConnect traffic passing through the bridge (for debugging)
./run_olcbchecker.sh --verbose

# Skip the copy+build steps (reuse last build)
./run_olcbchecker.sh --skip-build

# Skip broadcast time tests
./run_olcbchecker.sh --no-broadcast-time

# Include train protocol tests (Train Control, Train Search, FDI)
./run_olcbchecker.sh --trains

# Combine flags
./run_olcbchecker.sh --skip-build --no-broadcast-time --verbose
```

## Architecture

```
+----------------+          +------------------+          +----------------+
|   BasicNode    |  TCP     |  Bridge Server   |  TCP     |  OlcbChecker   |
|   (C binary)   |--------->|  (Python)        |<---------|  (Python)      |
|                | client   |  port 12021      | client   |                |
| connects to    | GridConn |  accepts 2 conns | GridConn | connects to    |
| 127.0.0.1:12021|          |  forwards bidir  |          | localhost:12021|
+----------------+          +------------------+          +----------------+
```

Both the BasicNode and OlcbChecker are TCP **clients**. The bridge server
listens on port 12021, accepts both connections, and forwards all GridConnect
strings bidirectionally between them.

## Files

| File | Purpose |
|------|---------|
| `run_olcbchecker.sh` | Master test runner script |
| `run_tests.py` | Python wrapper that selects which OlcbChecker sections to run |
| `bridge_server.py` | TCP bridge server |
| `README.md` | This file |

## Test sections

**Core tests** (always run):

| Section | Tests |
|---------|-------|
| Frame Transport | fr20-fr60 (fr10 init skipped via `-i` flag) |
| Message Network | me10-me50 |
| SNIP | sn10 |
| Event Transport | ev10-ev70 |
| Datagram Transport | da30 |
| Memory Configuration | mc10-mc50 |
| CDI | cd10-cd30 |

**Broadcast Time** (on by default, `--no-broadcast-time` to skip):

| Section | Tests |
|---------|-------|
| Broadcast Time (Consumer) | bt100-bt130 |

**Train protocols** (off by default, `--trains` to enable):

| Section | Tests |
|---------|-------|
| Train Control | tc tests |
| Train Search | ts tests |
| FDI | fd tests |

## Node reboot

The `OSxDrivers_reboot()` callback in the BasicNode demo calls
`OpenLcbNode_reset_state()`, which resets all nodes to `RUNSTATE_INIT` and
triggers a fresh CID/RID/AMD login sequence. This allows OlcbChecker's
restart test (`check_mc50_restart`) to work over the TCP connection.
