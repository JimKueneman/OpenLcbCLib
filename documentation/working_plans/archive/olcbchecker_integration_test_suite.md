<!--
  ============================================================
  STATUS: IMPLEMENTED
  `bridge_server.py`, `run_olcbchecker.sh`, and `run_tests.py` provide end-to-end automation: build, bridge, per-mode node restarts, and multi-section OlcbChecker execution.
  ============================================================
-->

# OlcbChecker Integration Test Suite Plan

## Goal

Create an automated end-to-end test harness that:
1. Copies the current library source into the Xcode BasicNode demo
2. Compiles the demo via `xcodebuild`
3. Starts a Python bridge server that connects the BasicNode to OlcbChecker
4. Runs the OlcbChecker compliance suite against the BasicNode
5. Prints/logs the results

## Constraints

- **No modifications to OlcbChecker.** It is an external project and must be
  used as-is. All integration is done through its existing command-line
  interface (`-a`, `-t`, `-i`, `-r` flags) and its standard TCP GridConnect
  connection.

---

## Architecture Overview

```
+----------------+          +------------------+          +----------------+
|   BasicNode    |  TCP     |  Bridge Server   |  TCP     |  OlcbChecker   |
|   (C binary)   |--------->|  (Python)        |<---------|  (Python)      |
|                | client   |  port 12021      | client   |                |
| connects to    | GridConn |  accepts 2 conns | GridConn | connects to    |
| 127.0.0.1:12021|          |  forwards bidir  |          | localhost:12021|
+----------------+          +------------------+          +----------------+
```

### Why a bridge server is needed

Both the BasicNode and OlcbChecker are TCP **clients**. Currently the BasicNode
connects to `127.0.0.1:12021` expecting a server there. OlcbChecker also
connects as a client via `-a host:port`. A bridge server sits in the middle:

- Listens on port 12021
- Accepts the BasicNode connection (first client)
- Accepts the OlcbChecker connection (second client)
- Forwards all GridConnect strings bidirectionally between the two
- Can inject control commands (e.g., trigger node reboot between test sections)

---

## The Reboot Problem

### Background

Several OlcbChecker tests (notably `check_fr10_init`) require the node to
perform a full CID/RID/AMD login sequence. The checker prompts:
`"Please reset/restart the checked node now"` and waits up to 30 seconds for
CID frames.

On real hardware a physical reset button is pressed. With a software node
running over TCP/IP we need a programmatic way to reset the node.

### Current state

| Mechanism | Location | Status |
|-----------|----------|--------|
| `OpenLcbNode_reset_state()` | `openlcb_node.h:149` | EXISTS - resets all nodes to `RUNSTATE_INIT`, clears `permitted` and `initialized` flags |
| `OSxDrivers_reboot()` | `osx_drivers.c:151-153` | **EMPTY** - callback does nothing |
| Manual 'r' key | `osx_drivers.c:84-94` | Works but requires keyboard input |
| Memory Config reboot command | Protocol datagram | Dispatches to `OSxDrivers_reboot()` which is empty |

### Solution

1. **Fix `OSxDrivers_reboot()`** to call `OpenLcbNode_reset_state()`. This
   allows OlcbChecker's `check_mc50_restart` test to programmatically reboot
   the node via the standard Memory Configuration protocol.

2. **Bridge server injects reboot command** between test sections. Before each
   test group that needs a fresh node, the bridge server sends the Memory
   Configuration Reboot datagram to the node side, then waits for the CID
   sequence to complete before allowing the checker to proceed.

3. **Skip interactive tests** for fully automated runs by passing `-i` to
   OlcbChecker (sets `skip_interactive = True`). Tests like `check_fr10_init`
   that require watching the raw CID startup sequence can be handled separately
   or skipped in CI.

4. **Alternative: bridge-level reset** - The bridge server disconnects and
   reconnects the node socket. If the BasicNode is modified to reconnect
   after a dropped TCP connection (instead of calling `exit(1)`), this
   simulates a full power-cycle. This is the most thorough reset but
   requires changes to `osx_can_drivers.c` to add reconnect logic.

### Recommended approach

For Phase 1: Fix `OSxDrivers_reboot()` + use `-i` (skip interactive) flag.
This gets all non-interactive tests running immediately.

For Phase 2: Add reconnect logic to the CAN driver and bridge-level
disconnect/reconnect for full CID startup testing.

---

## Implementation Steps

### Step 1: Library Copy Script

Reuse the existing `tools/update_applications/update_applications.sh` pattern.
The test runner script will call it (or a subset targeting only xcode/BasicNode)
before building:

```bash
# Files copied (from update_applications.sh lines 53-61):
cp src/openlcb/*.c   applications/xcode/BasicNode/src/openlcb/
cp src/openlcb/*.h   applications/xcode/BasicNode/src/openlcb/
cp src/drivers/canbus/*.c applications/xcode/BasicNode/src/drivers/canbus/
cp src/drivers/canbus/*.h applications/xcode/BasicNode/src/drivers/canbus/
cp src/utilities/*.c applications/xcode/BasicNode/src/utilities/
cp src/utilities/*.h applications/xcode/BasicNode/src/utilities/
```

### Step 2: Compile the Xcode Demo

```bash
xcodebuild -project applications/xcode/BasicNode.xcodeproj \
           -scheme BasicNode \
           -configuration Debug \
           build
```

The built binary will be at a path like:
`~/Library/Developer/Xcode/DerivedData/BasicNode-<hash>/Build/Products/Debug/BasicNode`

To get a predictable path, use `-derivedDataPath`:

```bash
xcodebuild -project applications/xcode/BasicNode.xcodeproj \
           -scheme BasicNode \
           -configuration Debug \
           -derivedDataPath build/xcode \
           build

# Binary at: build/xcode/Build/Products/Debug/BasicNode
```

### Step 3: Python Bridge Server

Create `test/olcbchecker_bridge/bridge_server.py`:

```
Purpose:  Accept two TCP client connections on port 12021 and forward
          GridConnect strings between them.

Client 1: BasicNode (C binary) - identified as first connection
Client 2: OlcbChecker (Python) - identified as second connection

Behavior:
  - Listen on port 12021 (configurable)
  - Accept connection from BasicNode
  - Accept connection from OlcbChecker
  - Forward all data: Node -> Checker and Checker -> Node
  - Log all GridConnect traffic (optional, for debugging)
  - On shutdown: close both connections

Threading model:
  - Main thread: accept connections, manage lifecycle
  - Reader thread per client: read from socket, write to the other
  - Or: use asyncio/select for single-threaded I/O multiplexing
```

Minimal implementation outline:

```python
import socket
import threading
import sys

PORT = 12021

def forward(src, dst, label):
    """Read from src socket, write to dst socket."""
    try:
        while True:
            data = src.recv(4096)
            if not data:
                break
            dst.sendall(data)
    except (ConnectionResetError, BrokenPipeError, OSError):
        pass

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(('127.0.0.1', PORT))
    server.listen(2)

    print(f"Bridge listening on port {PORT}")

    # Wait for BasicNode to connect
    node_conn, node_addr = server.accept()
    print(f"BasicNode connected from {node_addr}")

    # Wait for OlcbChecker to connect
    checker_conn, checker_addr = server.accept()
    print(f"OlcbChecker connected from {checker_addr}")

    # Forward bidirectionally
    t1 = threading.Thread(target=forward, args=(node_conn, checker_conn, "Node->Checker"), daemon=True)
    t2 = threading.Thread(target=forward, args=(checker_conn, node_conn, "Checker->Node"), daemon=True)
    t1.start()
    t2.start()

    t1.join()
    t2.join()

if __name__ == '__main__':
    main()
```

### Step 4: Run OlcbChecker

```bash
cd /Users/jimkueneman/Documents/OlcbChecker

python3.10 control_master.py \
    -a 127.0.0.1:12021 \
    -t 05.07.01.01.00.33 \
    -i \
    -r
```

Flags:
- `-a 127.0.0.1:12021` - connect to the bridge server
- `-t 05.07.01.01.00.33` - target node ID (matches `NODE_ID` in main.c)
- `-i` - skip interactive checks (no human to press reset)
- `-r` - run all checks immediately (no menu)

### Step 5: Master Test Runner Script

Create `test/olcbchecker_bridge/run_olcbchecker.sh` (the single entry point):

```bash
#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CHECKER_DIR="/Users/jimkueneman/Documents/OlcbChecker"
BRIDGE_PORT=12021
NODE_ID="05.07.01.01.00.33"
BUILD_DIR="$REPO_ROOT/build/xcode"

echo "=== Step 1: Copy library to Xcode BasicNode ==="
cd "$REPO_ROOT/tools/update_applications"
bash update_applications.sh
# (or just the xcode lines if a targeted script is created)

echo "=== Step 2: Build BasicNode ==="
xcodebuild -project "$REPO_ROOT/applications/xcode/BasicNode.xcodeproj" \
           -scheme BasicNode \
           -configuration Debug \
           -derivedDataPath "$BUILD_DIR" \
           build

BINARY="$BUILD_DIR/Build/Products/Debug/BasicNode"

echo "=== Step 3: Start bridge server ==="
python3 "$REPO_ROOT/test/olcbchecker_bridge/bridge_server.py" &
BRIDGE_PID=$!
sleep 1  # let it bind

echo "=== Step 4: Start BasicNode ==="
"$BINARY" &
NODE_PID=$!
sleep 3  # let it connect and complete login

echo "=== Step 5: Run OlcbChecker ==="
cd "$CHECKER_DIR"
python3.10 control_master.py \
    -a "127.0.0.1:$BRIDGE_PORT" \
    -t "$NODE_ID" \
    -i \
    -r
RESULT=$?

echo "=== Step 6: Cleanup ==="
kill $NODE_PID 2>/dev/null || true
kill $BRIDGE_PID 2>/dev/null || true
wait $NODE_PID 2>/dev/null || true
wait $BRIDGE_PID 2>/dev/null || true

echo ""
if [ $RESULT -eq 0 ]; then
    echo "=== ALL CHECKS PASSED ==="
else
    echo "=== SOME CHECKS FAILED (exit code: $RESULT) ==="
fi

exit $RESULT
```

---

## Code Changes Required

### 1. Fix `OSxDrivers_reboot()` (REQUIRED)

**File:** `applications/xcode/BasicNode/application_drivers/osx_drivers.c`

Current (empty):
```c
void OSxDrivers_reboot(...) {
}
```

Change to:
```c
void OSxDrivers_reboot(openlcb_statemachine_info_t *statemachine_info,
                       config_mem_operations_request_info_t *config_mem_operations_request_info) {
    printf("Reboot requested via protocol\n");
    OpenLcbNode_reset_state();
}
```

This allows the OlcbChecker's `check_mc50_restart` test to work, and allows
the bridge server to inject reboot commands between test sections.

### 2. New files to create

| File | Purpose |
|------|---------|
| `test/olcbchecker_bridge/bridge_server.py` | TCP bridge server |
| `test/olcbchecker_bridge/run_olcbchecker.sh` | Master test runner script |
| `test/olcbchecker_bridge/README.md` | Explains what this test does and how to run it |

### 3. Future enhancements (Phase 2)

- **Reconnect logic in `osx_can_drivers.c`**: Instead of `exit(1)` on
  connection error (line 256), loop back and try `_connect_to_server()` again.
  This enables the bridge server to simulate a power-cycle by dropping and
  re-accepting the node connection.

- **Bridge-level reboot**: The bridge server disconnects the node socket,
  waits for reconnect, then signals the checker. This would allow
  `check_fr10_init` (full CID startup sequence) to pass without human
  intervention.

---

## Test Coverage

With `-i` (skip interactive) the following test sections will run:

| Section | Tests | Notes |
|---------|-------|-------|
| Frame Transport | fr20-fr60 | fr10 (init) skipped - interactive |
| Message Network | me10-me50 | All should pass |
| SNIP | sn10 | Should pass |
| Event Transport | ev10-ev70 | Should pass |
| Datagram Transport | da30 | Should pass |
| Memory Configuration | mc10-mc50 | mc50 (restart) needs reboot fix |
| CDI | cd10-cd30 | cd10 validates XML, cd20 reads CDI, cd30 checks ACDI |
| Broadcast Time | bt10-bt130 | Node is configured as BT consumer |

Train Control, Train Search, and FDI tests are N/A for BasicNode (no train
protocol compiled in) and will be skipped by PIP checking (`-P` flag behavior).

---

## Decisions

1. **Bridge server port**: Single port 12021 for both clients. The bridge
   accepts two connections in order (node first, checker second). No value
   in separate ports given the deterministic startup sequence.

2. **OlcbChecker location**: Defined as a constant at the top of the bridge
   server Python file. Default: `/Users/jimkueneman/Documents/OlcbChecker`.
   Change the constant if the path moves.

---

## Phase 2: Full CID Startup Testing (check_fr10_init)

### Problem

`check_fr10_init` is an interactive test. It prints
`"Please reset/restart the checked node now"` and then waits up to 30 seconds
for the four CID frames, the RID frame, and the AMD frame that a node sends
during its initial alias reservation. This is the most fundamental compliance
check - it verifies the node follows the correct startup handshake.

Calling `OpenLcbNode_reset_state()` alone is not enough for this test because
it resets the node's logical state but the TCP connection stays alive. A real
power-cycle would cause a disconnect and reconnect, which is what the checker
expects to see (a fresh stream of CID frames on an existing connection).

Actually, `OpenLcbNode_reset_state()` DOES cause the node to re-run the full
CID/RID/AMD login sequence over the existing TCP connection. The node goes back
to `RUNSTATE_INIT` and the main loop (`OpenLcb_run()`) drives it through alias
reservation again. So the CID frames WILL appear on the wire. The question is
timing: the bridge needs to trigger the reset at the right moment.

### Approach: Bridge-initiated reset with reconnect fallback

**Option A - Protocol-level reset (simpler, try first):**

The bridge server monitors traffic from the checker side. When it detects the
checker is waiting (no traffic for a few seconds after connecting, which is
the `check_fr10_init` wait period), the bridge injects a reboot command to the
node side. This triggers `OpenLcbNode_reset_state()` via `OSxDrivers_reboot()`,
causing the node to re-run its CID/RID/AMD sequence. The checker sees the
frames and the test passes.

The reboot command is a Memory Configuration datagram. The bridge would need
to construct and send the correct GridConnect-encoded CAN frames for this.

**Option B - TCP disconnect/reconnect (more thorough):**

1. Modify `osx_can_drivers.c` to reconnect instead of `exit(1)` on
   connection loss:

   ```c
   // Current (line 251-257):
   _is_connected = false;
   printf("Connection error detected: %d\n", errno);
   exit(1);

   // Change to:
   _is_connected = false;
   printf("Connection lost, reconnecting...\n");
   close(socket_fd);
   sleep(2);
   socket_fd = _connect_to_server(ip_address, port);
   if (socket_fd >= 0) {
       _is_connected = true;
       OpenLcbNode_reset_state();  // reset node to re-login
   }
   ```

2. The bridge server, when it needs to trigger a node restart:
   - Closes the node-side socket
   - Waits for the node to reconnect (new `accept()`)
   - The node's reconnect triggers `OpenLcbNode_reset_state()`
   - CID frames flow through the bridge to the checker

3. The bridge needs to know WHEN to trigger this. Options:
   - A separate control channel (e.g., the test runner sends a signal)
   - The bridge monitors checker traffic patterns
   - The test runner script orchestrates: run non-interactive tests first,
     then restart the node process, then run `check_fr10_init` separately

**Option C - Process restart (simplest for Phase 2):**

The test runner script runs the tests in two passes:
1. First pass: run all tests with `-i` (skip interactive)
2. Kill the BasicNode process
3. Restart BasicNode (it reconnects to bridge, sends CID frames)
4. Second pass: run only `check_fr10_init` (which sees the fresh CID frames)

This requires no bridge modifications. The runner script controls the timing.

```bash
# Pass 1: all non-interactive tests
python3.10 control_master.py -a 127.0.0.1:12021 -t $NODE_ID -i -r

# Kill and restart node for CID test
kill $NODE_PID
sleep 1
"$BINARY" &
NODE_PID=$!
sleep 1  # node connects to bridge, sends CID/RID/AMD

# Pass 2: just the frame init test
python3.10 check_fr10_init.py -a 127.0.0.1:12021 -t $NODE_ID
```

**BUT** this has a problem: OlcbChecker creates its TCP connection in
`olcbchecker/setup.py` at import time. When the node is killed, the bridge
loses one side. The checker's connection is also affected. Both the checker
and the bridge need to handle this gracefully.

### Recommended Phase 2 approach

**Option C with bridge enhancement**: The bridge server is designed to handle
node disconnect/reconnect while keeping the checker connection alive:

1. Bridge accepts checker connection and holds it open for the full run
2. Bridge accepts node connection
3. When node disconnects, bridge buffers/waits for a new node connection
4. When new node connects, bridge resumes forwarding
5. CID frames from the new node connection flow to the checker

The test runner script:
1. Start bridge
2. Start node (connects to bridge, CID frames buffered)
3. Start checker for `check_fr10_init` - it connects and waits for CID
4. Kill node process
5. Restart node process - fresh CID frames flow to checker
6. Checker sees CID frames, test passes
7. Continue with remaining tests (checker stays connected)

### Changes required for Phase 2

| File | Change |
|------|--------|
| `osx_can_drivers.c` | Replace `exit(1)` with reconnect loop |
| `bridge_server.py` | Handle node disconnect/reconnect while keeping checker alive |
| `run_olcbchecker.sh` | Add two-pass test orchestration |
