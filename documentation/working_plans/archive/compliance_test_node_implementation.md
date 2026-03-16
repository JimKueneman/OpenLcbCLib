<!--
  ============================================================
  STATUS: IMPLEMENTED
  The `ComplianceTestNode` is fully built under `test/compliance_node/` with a protocol-mode registry, multi-mode callback infrastructure, and orchestration via `run_olcbchecker.sh`.
  ============================================================
-->

# Implementation: ComplianceTestNode for OlcbChecker Testing

## Context

The existing BasicNode in `applications/xcode/` is a user-facing demo that should stay simple. OlcbChecker compliance testing needs a dedicated test node that supports runtime protocol selection without recompiling. This test node lives in `test/` alongside the other test infrastructure.

The design is extensible — adding a new protocol means adding one entry to a configuration table, one callback file, and one section in the test script. No changes to `main.c` argument parsing or the core architecture.

## Project Structure

```
test/
    olcbchecker_bridge/
        run_olcbchecker.sh              # updated to build/run ComplianceTestNode
        run_tests.py
        bridge_server.py
    compliance_node/
        ComplianceTestNode.xcodeproj    # references library sources directly from src/
        main.c                          # arg parsing + node creation via protocol registry
        protocol_modes.c                # protocol mode registry table
        protocol_modes.h
        callbacks_core.c                # always-active callbacks
        callbacks_core.h
        callbacks_broadcast_time.c      # broadcast time consumer/producer callbacks
        callbacks_broadcast_time.h
        callbacks_trains.c              # train protocol callbacks
        callbacks_trains.h
        openlcb_user_config.h           # all feature flags enabled, large buffers
        openlcb_user_config.c           # node_parameters definitions for each mode
        application_drivers/
            osx_can_drivers.c/h         # TCP/GridConnect transport (same as BasicNode)
            osx_drivers.c/h             # timer, reboot, config mem (same as BasicNode)
            threadsafe_stringlist.c/h    # thread-safe string queue
```

## Key Design: No Library Copies

Unlike the user-facing demos which copy library source files into their own directory, the ComplianceTestNode Xcode project references the library sources directly:

- **Source files**: Added to Xcode project as references to `src/openlcb/*.c`, `src/drivers/canbus/*.c`, `src/utilities/*.c`
- **Header search paths**: `../../src` (relative from `test/compliance_node/`)
- **No copy step**: `run_olcbchecker.sh` skips the file copy and just builds
- **No `update_applications` entry needed**

## Compile-Time Configuration (`openlcb_user_config.h`)

All feature flags enabled — runtime args control which protocols are active:

```c
#define OPENLCB_COMPILE_EVENTS
#define OPENLCB_COMPILE_DATAGRAMS
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION
#define OPENLCB_COMPILE_FIRMWARE
#define OPENLCB_COMPILE_BROADCAST_TIME
#define OPENLCB_COMPILE_TRAIN

#define USER_DEFINED_BASIC_BUFFER_DEPTH           117
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH        4
#define USER_DEFINED_SNIP_BUFFER_DEPTH            4
#define USER_DEFINED_STREAM_BUFFER_DEPTH          1
#define USER_DEFINED_NODE_BUFFER_DEPTH            4

#define USER_DEFINED_PRODUCER_COUNT               64
#define USER_DEFINED_PRODUCER_RANGE_COUNT         5
#define USER_DEFINED_CONSUMER_COUNT               32
#define USER_DEFINED_CONSUMER_RANGE_COUNT         5

#define USER_DEFINED_CDI_LENGTH                   20000
#define USER_DEFINED_FDI_LENGTH                   1000

#define USER_DEFINED_TRAIN_NODE_COUNT             4
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN      6
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS          29
```

## Extensible Protocol Mode Registry

### The pattern

Rather than a hardcoded enum/switch, each protocol mode is described by a `protocol_mode_t` struct. Adding a new protocol is just adding one entry to the `protocol_modes` array.

### `protocol_modes.h`

```c
#ifndef PROTOCOL_MODES_H
#define PROTOCOL_MODES_H

#include "src/openlcb/openlcb_types.h"

// Setup function signature: called after OpenLcb_create_node() to
// register protocol-specific event ranges, handlers, etc.
typedef void (*protocol_setup_fn)(openlcb_node_t *node);

typedef struct {
    const char *flag;                       // CLI flag (e.g., "--train")
    const char *name;                       // Human-readable name for logging
    const char *test_section;               // Section name for run_tests.py
    const node_parameters_t *params;        // Node parameters for this mode
    protocol_setup_fn setup;                // Post-creation setup (NULL if none)
} protocol_mode_t;

// Registry of all available protocol modes.
// Terminated by an entry with flag == NULL.
extern const protocol_mode_t protocol_modes[];

// Always returns a valid mode (defaults to basic if flag not found)
const protocol_mode_t *ProtocolModes_find(const char *flag);

// Returns the default (basic) mode
const protocol_mode_t *ProtocolModes_default(void);

// Print all available modes to stdout
void ProtocolModes_print_usage(void);

#endif
```

### `protocol_modes.c`

```c
#include "protocol_modes.h"
#include "openlcb_user_config.h"
#include "src/openlcb/openlcb_application_broadcast_time.h"
#include "src/openlcb/openlcb_application_train.h"

// Forward declarations for setup functions
static void setup_broadcast_time_consumer(openlcb_node_t *node);
static void setup_broadcast_time_producer(openlcb_node_t *node);
static void setup_train(openlcb_node_t *node);

// Node parameters defined in openlcb_user_config.c
extern const node_parameters_t compliance_basic_params;
extern const node_parameters_t compliance_broadcast_time_consumer_params;
extern const node_parameters_t compliance_broadcast_time_producer_params;
extern const node_parameters_t compliance_train_params;

const protocol_mode_t protocol_modes[] = {
    {
        .flag         = "--basic",
        .name         = "Basic Node",
        .test_section = "core",
        .params       = &compliance_basic_params,
        .setup        = NULL
    },
    {
        .flag         = "--broadcast-time-consumer",
        .name         = "Broadcast Time Consumer",
        .test_section = "broadcast_time_consumer",
        .params       = &compliance_broadcast_time_consumer_params,
        .setup        = setup_broadcast_time_consumer
    },
    {
        .flag         = "--broadcast-time-producer",
        .name         = "Broadcast Time Producer",
        .test_section = "broadcast_time_producer",
        .params       = &compliance_broadcast_time_producer_params,
        .setup        = setup_broadcast_time_producer
    },
    {
        .flag         = "--train",
        .name         = "Train Node",
        .test_section = "trains",
        .params       = &compliance_train_params,
        .setup        = setup_train
    },
    // ---- Add new protocol modes here ----
    // {
    //     .flag         = "--traction-proxy",
    //     .name         = "Traction Proxy",
    //     .test_section = "traction_proxy",
    //     .params       = &compliance_traction_proxy_params,
    //     .setup        = setup_traction_proxy
    // },
    { .flag = NULL }  // sentinel
};

const protocol_mode_t *ProtocolModes_find(const char *flag) {
    for (int i = 0; protocol_modes[i].flag != NULL; i++) {
        if (strcmp(protocol_modes[i].flag, flag) == 0)
            return &protocol_modes[i];
    }
    return NULL;
}

const protocol_mode_t *ProtocolModes_default(void) {
    return &protocol_modes[0];  // basic
}

void ProtocolModes_print_usage(void) {
    printf("Usage: ComplianceTestNode [--mode-flag] [--node-id 0xNNNNNNNNNNNN]\n\n");
    printf("Available protocol modes:\n");
    for (int i = 0; protocol_modes[i].flag != NULL; i++) {
        printf("  %-30s  %s\n", protocol_modes[i].flag, protocol_modes[i].name);
    }
    printf("\nDefault: %s\n", protocol_modes[0].name);
}

static void setup_broadcast_time_consumer(openlcb_node_t *node) {
    OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
}

static void setup_broadcast_time_producer(openlcb_node_t *node) {
    OpenLcbApplicationBroadcastTime_setup_producer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
}

static void setup_train(openlcb_node_t *node) {
    OpenLcbApplicationTrain_setup(node);
}
```

### Adding a new protocol

To add support for a new protocol (e.g., Traction Proxy):

1. **`openlcb_user_config.c`** — add `compliance_traction_proxy_params` with appropriate PIP bits, CDI, address spaces
2. **`callbacks_traction_proxy.c/h`** — implement the protocol callbacks
3. **`protocol_modes.c`** — add one entry to the `protocol_modes[]` array
4. **`openlcb_config_t`** — wire up the new callback function pointers
5. **`run_olcbchecker.sh`** — add a new run block (follows existing pattern)
6. **`run_tests.py`** — add the new section name

No changes to `main.c`, no new enum values, no new switch cases.

## Node Parameters (`openlcb_user_config.c`)

One `node_parameters_t` definition per protocol mode:

### `compliance_basic_params` (default)
- PIP: datagram, memory config, events, SNIP, CDI, firmware
- CDI: user name + description segments
- Same as current BasicNode

### `compliance_broadcast_time_consumer_params`
- PIP: same as basic + event exchange (broadcast time uses events)
- CDI: basic segments + broadcast time consumer configuration

### `compliance_broadcast_time_producer_params`
- PIP: same as basic + event exchange
- CDI: basic segments + broadcast time producer configuration

### `compliance_train_params`
- PIP: same as basic + PSI_TRAIN_CONTROL + PSI_FUNCTION_DESCRIPTION + PSI_FUNCTION_CONFIGURATION
- CDI/FDI: train-specific configuration

## `main.c` — Clean and Protocol-Agnostic

```c
#include "protocol_modes.h"

#define DEFAULT_NODE_ID 0x050701010033

int main(int argc, char *argv[]) {
    // 1. Find the requested protocol mode
    const protocol_mode_t *mode = ProtocolModes_default();
    uint64_t nodeid = DEFAULT_NODE_ID;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--node-id") == 0 && i + 1 < argc) {
            nodeid = strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "--help") == 0) {
            ProtocolModes_print_usage();
            return 0;
        } else {
            const protocol_mode_t *found = ProtocolModes_find(argv[i]);
            if (found)
                mode = found;
        }
    }

    printf("ComplianceTestNode: mode=%s\n", mode->name);

    // 2. Initialize drivers and library
    OSxDrivers_setup();
    OSxCanDriver_setup();
    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    // 3. Wait for threads to connect
    while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected()))
        sleep(2);

    // 4. Create node with the selected mode's parameters
    openlcb_node_t *node = OpenLcb_create_node(nodeid, mode->params);

    // 5. Run protocol-specific setup (if any)
    if (mode->setup)
        mode->setup(node);

    // 6. Main loop (lazy sleep pattern)
    int idle_count = 0;
    while (1) {
        OpenLcb_run();
        if (OSxCanDriver_data_was_received())
            idle_count = 0;
        else
            idle_count++;
        if (idle_count > 100)
            usleep(50);
    }
}
```

`main.c` never changes when protocols are added — it just looks up the mode and calls setup.

## Callback Files

### `callbacks_core.c`
Always-active callbacks — same as current BasicNode:
- `Callbacks_on_100ms_timer_callback()`
- `Callbacks_on_can_rx_callback()`
- `Callbacks_on_can_tx_callback()`
- `Callbacks_alias_change_callback()`
- `Callbacks_operations_request_factory_reset()`
- `Callbacks_freeze()` / `Callbacks_unfreeze()`
- `Callbacks_write_firmware()`

### `callbacks_broadcast_time.c`
Broadcast time callbacks:
- `CallbacksBroadcastTime_on_time_changed()` — log time changes
- `CallbacksBroadcastTime_on_login_complete()` — start clock and send initial query (consumer) or start broadcasting (producer)
- Producer-specific: handle Set/Query commands from consumers

### `callbacks_trains.c`
Train protocol callbacks for single-node conformance testing:
- `CallbacksTrains_on_speed_changed()` — log speed changes
- `CallbacksTrains_on_function_changed()` — log function changes
- `CallbacksTrains_on_emergency_entered()` — log emergency stop events
- `CallbacksTrains_on_emergency_exited()` — log emergency exit events
- `CallbacksTrains_on_controller_assign_request()` — accept all controller requests
- `CallbacksTrains_on_controller_changed_request()` — accept all takeover requests
- `CallbacksTrains_on_heartbeat_timeout()` — log but don't stop (keeps node alive for testing)

## Application Drivers

The `application_drivers/` directory contains the same TCP/GridConnect transport code as BasicNode:
- `osx_can_drivers.c/h` — buffered reads, lazy sleep flag, thread-safe outgoing queue
- `osx_drivers.c/h` — 100ms timer thread, reboot handler, config mem stubs
- `threadsafe_stringlist.c/h` — thread-safe string queue for outgoing GridConnect

These can be copied from BasicNode initially. They're identical and don't need to change between protocol modes.

## `openlcb_config_t` Setup

The `openlcb_config_t` struct wires library callbacks to application code. All callback slots are populated at compile time. Since unused callbacks are only invoked when the corresponding protocol is active on a node, having them wired up when no train/broadcast-time node exists is harmless.

```c
static const openlcb_config_t openlcb_config = {
    // Core (always)
    .lock_shared_resources   = &OSxCanDriver_pause_can_rx,
    .unlock_shared_resources = &OSxCanDriver_resume_can_rx,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,
    .factory_reset           = &Callbacks_operations_request_factory_reset,
    .freeze                  = &Callbacks_freeze,
    .unfreeze                = &Callbacks_unfreeze,
    .firmware_write          = &Callbacks_write_firmware,
    .on_100ms_timer          = &Callbacks_on_100ms_timer_callback,

    // Broadcast time
    .on_broadcast_time_changed = &CallbacksBroadcastTime_on_time_changed,

    // Train
    .on_train_speed_changed              = &CallbacksTrains_on_speed_changed,
    .on_train_function_changed           = &CallbacksTrains_on_function_changed,
    .on_train_emergency_entered          = &CallbacksTrains_on_emergency_entered,
    .on_train_emergency_exited           = &CallbacksTrains_on_emergency_exited,
    .on_train_controller_assign_request  = &CallbacksTrains_on_controller_assign_request,
    .on_train_controller_changed_request = &CallbacksTrains_on_controller_changed_request,
    .on_train_heartbeat_timeout          = &CallbacksTrains_on_heartbeat_timeout,
};
```

## Updated `run_olcbchecker.sh`

Key changes:
- Point at `ComplianceTestNode.xcodeproj` instead of `BasicNode.xcodeproj`
- Remove the library file copy step (project references sources directly)
- Use a generic `run_protocol_test` function for each pass
- Adding a new protocol run is one function call

```bash
# ============================================================================
# Configuration
# ============================================================================
COMPLIANCE_PROJECT="$REPO_ROOT/test/compliance_node/ComplianceTestNode.xcodeproj"
BUILD_DIR="$REPO_ROOT/build/compliance"

# ============================================================================
# Build (once)
# ============================================================================
xcodebuild -project "$COMPLIANCE_PROJECT" \
           -scheme ComplianceTestNode \
           -configuration Debug \
           -derivedDataPath "$BUILD_DIR" \
           clean build

BINARY="$BUILD_DIR/Build/Products/Debug/ComplianceTestNode"

# ============================================================================
# Start bridge (stays up for all runs)
# ============================================================================
python3 "$SCRIPT_DIR/bridge_server.py" --port "$BRIDGE_PORT" &
BRIDGE_PID=$!
sleep 1

# ============================================================================
# Generic test runner: start node, run tests, kill node
# ============================================================================
total=0

run_protocol_test() {
    local node_flag="$1"      # e.g., "--train" or "" for basic
    local test_section="$2"   # e.g., "trains" or "core"
    local label="$3"          # e.g., "Train Protocol"

    echo ""
    echo "=== $label ==="

    "$BINARY" --node-id "$NODE_ID" $node_flag </dev/null &
    NODE_PID=$!
    sleep 3

    if ! kill -0 "$NODE_PID" 2>/dev/null; then
        echo "ERROR: ComplianceTestNode failed to start for $label"
        total=$((total + 1))
        return
    fi

    RUN_SECTIONS="$test_section" \
    python3 "$SCRIPT_DIR/run_tests.py" \
        -a "127.0.0.1:$BRIDGE_PORT" -t "$NODE_ID" \
        -i --auto-reboot
    total=$((total + $?))

    kill "$NODE_PID" 2>/dev/null; wait "$NODE_PID" 2>/dev/null
    sleep 1
}

# ============================================================================
# Test runs
# ============================================================================

# Core compliance (always)
run_protocol_test "" "core" "Core Compliance"

# Broadcast time consumer (if enabled)
[ "$BROADCAST_TIME_CONSUMER" = true ] && \
    run_protocol_test "--broadcast-time-consumer" "broadcast_time_consumer" "Broadcast Time Consumer"

# Broadcast time producer (if enabled)
[ "$BROADCAST_TIME_PRODUCER" = true ] && \
    run_protocol_test "--broadcast-time-producer" "broadcast_time_producer" "Broadcast Time Producer"

# Train protocol (if enabled)
[ "$TRAINS" = true ] && \
    run_protocol_test "--train" "trains" "Train Protocol"

# ---- Add new protocol runs here ----
# [ "$TRACTION_PROXY" = true ] && \
#     run_protocol_test "--traction-proxy" "traction_proxy" "Traction Proxy"
```

## Updated `run_tests.py`

Add `RUN_SECTIONS` environment variable to control which test groups run per invocation:

```python
run_sections = os.environ.get("RUN_SECTIONS", "core").split(",")

if "core" in run_sections:
    # frame, message, snip, events, datagram, memory, cdi
    import control_frame
    logger.info("=== Frame Transport ===")
    total += min(control_frame.checkAll(), 1)
    # ... existing core sections ...

if "broadcast_time_consumer" in run_sections:
    import control_broadcasttime
    logger.info("=== Broadcast Time (Consumer) ===")
    total += min(control_broadcasttime.checkAllConsumer(), 1)

if "broadcast_time_producer" in run_sections:
    import control_broadcasttime
    logger.info("=== Broadcast Time (Producer) ===")
    total += min(control_broadcasttime.checkAllProducer(), 1)

if "trains" in run_sections:
    import control_traincontrol
    logger.info("=== Train Control ===")
    total += min(control_traincontrol.checkAll(), 1)

    import control_trainsearch
    logger.info("=== Train Search ===")
    total += min(control_trainsearch.checkAll(), 1)

    import control_fdi
    logger.info("=== FDI ===")
    total += min(control_fdi.checkAll(), 1)

# ---- Add new protocol sections here ----
# if "traction_proxy" in run_sections:
#     import control_tractionproxy
#     total += min(control_tractionproxy.checkAll(), 1)
```

## Implementation Order

1. Create `test/compliance_node/` directory structure
2. Create `openlcb_user_config.h` with all feature flags enabled
3. Create `openlcb_user_config.c` with all `node_parameters_t` definitions
4. Copy `application_drivers/` from BasicNode (identical transport code)
5. Create `callbacks_core.c/h` (extract from BasicNode's callbacks)
6. Create `callbacks_broadcast_time.c/h` (broadcast time callbacks)
7. Create `callbacks_trains.c/h` (train protocol callbacks)
8. Create `protocol_modes.c/h` (protocol mode registry)
9. Create `main.c` (protocol-agnostic, uses registry)
10. Create `ComplianceTestNode.xcodeproj` with direct source references
11. Update `run_olcbchecker.sh` with `run_protocol_test` function
12. Update `run_tests.py` with `RUN_SECTIONS` support
13. Test: `./run_olcbchecker.sh` — core tests should pass
14. Implement and test broadcast time consumer callbacks
15. Implement and test broadcast time producer callbacks
16. Implement and test train protocol callbacks

## Verification

For each run:
1. Build succeeds with all feature flags enabled
2. Core compliance tests pass in basic mode
3. Broadcast time consumer tests pass with `--broadcast-time-consumer`
4. Broadcast time producer tests pass with `--broadcast-time-producer`
5. Train conformance tests pass with `--train` (train control, train search, FDI)
6. No regressions — BasicNode demos remain unchanged
