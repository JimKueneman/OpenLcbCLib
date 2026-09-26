# OlcbChecker Integration Test

Automated compliance testing of the OpenLcbCLib library against the
[OlcbChecker](https://github.com/openlcb/OlcbChecker) test suite.

## What it does

1. Builds the ComplianceTestNode (`test/compliance_node`) via `xcodebuild`.
   Its `openlcb_c_lib/` folders are symlinks into `src/`, so it always builds
   against the current library; no copy step is needed.
2. Starts a TCP bridge server that connects the node to OlcbChecker
3. Runs each selected protocol mode as a separate pass: the node is started
   with that mode's flag, the matching OlcbChecker sections run, and the node
   is stopped. The bridge stays up for all passes.
4. Reports pass/fail results

## Builds

Stream support is a compile-time option (`OPENLCB_COMPILE_STREAM`), so the
runner builds up to two binaries, only the ones the selected modes need:

| Build | Stream | Binary | Used by |
|-------|--------|--------|---------|
| base | off | `/tmp/compliance_node_base` | core, broadcast time, trains, dcc-detector |
| stream | on, 2 concurrent streams | `/tmp/compliance_node_stream` | stream, trains-stream |

Both builds share the derived data folder `/tmp/compliance_build`; each
finished binary is copied out so the second build does not overwrite the
first. The node's output for the current pass goes to `/tmp/compliance_node.log`.

`--stream [N]` builds every mode with stream on (N concurrent streams,
default 1) and uses that one build for all passes.

## Prerequisites

- macOS with Xcode command-line tools installed
- Python 3.10+ (required by OlcbChecker). The runner uses
  `/opt/homebrew/bin/python3.13`; set the `PYTHON` environment variable to
  use another interpreter.
- OlcbChecker installed at `/Users/jimkueneman/Documents/OlcbCheckerClone`
  (change `CHECKER_DIR` in `run_olcbchecker.sh`, `OLCBCHECKER_DIR` in
  `run_tests.py` and `OLCBCHECKER_DIR` in `bridge_server.py` if your path
  differs)
- Python packages: `xmlschema` (for CDI validation)

## Usage

```bash
# Default: core, stream, trains-stream
./run_olcbchecker.sh

# Every mode
./run_olcbchecker.sh -m all

# Selected modes
./run_olcbchecker.sh -m core,trains

# One check or control script, with the node in the given mode
./run_olcbchecker.sh -m trains -s check_tr090_controller

# Include the restart checks (programmatic reboot instead of operator prompt)
./run_olcbchecker.sh -r

# Skip the config memory write checks
./run_olcbchecker.sh --no-writes

# Every mode built with stream on, 2 concurrent streams
./run_olcbchecker.sh -m all --stream 2

# Show GridConnect traffic passing through the bridge (for debugging)
./run_olcbchecker.sh -v
```

| Option | Meaning |
|--------|---------|
| `-m, --mode MODES` | Comma-separated modes to run, or `all` (default: `core,stream,trains-stream`) |
| `-s, --single SCRIPT` | Run one check or control script in the first selected mode |
| `-r, --auto-reboot` | Pass `--auto-reboot` to OlcbChecker so restart checks run |
| `-w, --force-writes` | Enable config memory write checks (default: on) |
| `--no-writes` | Skip config memory write checks |
| `--stream [N]` | Build every mode with stream on, N concurrent streams (default: 1) |
| `-v, --verbose` | Show GridConnect traffic in the bridge |
| `-h, --help` | Show usage |

## Architecture

```
+----------------+          +------------------+          +----------------+
| ComplianceTest |  TCP     |  Bridge Server   |  TCP     |  OlcbChecker   |
| Node (C binary)|--------->|  (Python)        |<---------|  (Python)      |
|                | client   |  port 12021      | client   |                |
| connects to    | GridConn |  accepts 2 conns | GridConn | connects to    |
| 127.0.0.1:12021|          |  forwards bidir  |          | localhost:12021|
+----------------+          +------------------+          +----------------+
```

Both the node and OlcbChecker are TCP **clients**. The bridge server listens
on port 12021, accepts both connections, and forwards all GridConnect strings
bidirectionally between them.

## Files

| File | Purpose |
|------|---------|
| `run_olcbchecker.sh` | Master test runner script: builds, starts the bridge, runs each mode |
| `run_tests.py` | Python wrapper that selects which OlcbChecker sections to run |
| `bridge_server.py` | TCP bridge server |
| `README.md` | This file |

## Modes

Each mode starts the node with one protocol flag on one build and runs one
`run_tests.py` section. OlcbChecker is always run with `-i` (skip interactive
checks).

| Mode | Node flag | Build | Sections |
|------|-----------|-------|----------|
| `core` | `--basic` | base | Frame Transport, Message Network, SNIP, Event Transport, Datagram Transport, Memory Configuration, CDI, Stream Transport |
| `stream` | `--basic` | stream | Datagram Transport, Memory Configuration, CDI, Stream Transport |
| `trains-stream` | `--train` | stream | FDI |
| `broadcast-time-consumer` | `--broadcast-time-consumer` | base | Broadcast Time (Consumer) |
| `broadcast-time-producer` | `--broadcast-time-producer` | base | Broadcast Time (Producer) |
| `trains` | `--train` | base | Train Control, Train Search, FDI |
| `dcc-detector` | `--dcc-detector` | base | DCC Detector |

Why the stream-enabled modes rerun some sections:

- **core** runs Stream Transport on the stream-off build. That is the only
  place `st20` (reject a stream when unsupported) is exercised; the other
  stream checks pass because the node does not advertise streams.
- **stream** reruns Memory Configuration and CDI because `mc30`, `mc60`,
  `mc80` and `cd20` each run their datagram pass first and then a stream pass
  that executes only when the node advertises streams. Datagram Transport is
  rerun because the stream build changes buffer sizing. With 2 concurrent
  streams the concurrent checks (`st100`, `st110`, `st140`) open two streams
  instead of passing on a rejection of the second.
- **trains-stream** reruns FDI because `fd20` has a stream pass for space
  0xFA. Train Control and Train Search do not use streams.

## Test sections

| Section | Checks |
|---------|--------|
| Frame Transport | fr10-fr60 |
| Message Network | me10-me60 |
| SNIP | sn10 |
| Event Transport | ev10-ev70 |
| Datagram Transport | da30, da40 |
| Memory Configuration | mc10-mc80 |
| CDI | cd10-cd40 |
| Stream Transport | st10-st210 |
| Broadcast Time (Consumer) | bt100-bt130 |
| Broadcast Time (Producer) | bt10-bt80 |
| Train Control | tr010-tr110 |
| Train Search | ts10-ts40 |
| FDI | fd10-fd30 |
| DCC Detector | dd10-dd40 |

Checks that are skipped unless enabled:

- `mc50` restart: needs `-r` (otherwise reported as an interactive check
  skipped)
- `mc70` write under mask and `mc80` write: on by default; `--no-writes`
  skips them. They write to space 0xFD on the node.

## Node reboot

The `OSxDrivers_reboot()` callback in the ComplianceTestNode calls
`OpenLcbNode_reset_state()`, which resets all nodes to `RUNSTATE_INIT` and
triggers a fresh CID/RID/AMD login sequence. This allows OlcbChecker's
restart check (`check_mc50_restart`) to work over the TCP connection when
`-r` is given.
