# Config-Mem Stream Handler: Static/Global State Analysis for Concurrency

**Date:** April 4, 2026  
**Scope:** Complete exploration of static and global state requiring pooling for concurrent streams

---

## 1. Core Static State - Protocol Config-Mem Stream Handler

**File:** `/src/openlcb/protocol_config_mem_stream_handler.c` (lines 60-76)

### Static Variables

#### `_interface` (Line 64)
```c
static interface_protocol_config_mem_stream_handler_t *_interface;
```
- Stores DI interface pointer (wired at initialization)
- Set by `ProtocolConfigMemStreamHandler_initialize()`
- **Shared by all concurrent streams** (no pooling needed)
- Contains all per-space read/write callbacks and Layer 1 API

#### `_context` (Line 67)
```c
static config_mem_stream_context_t _context;
```
- **Single active operation context** - the primary bottleneck
- Tracks state for one config-mem stream at a time (read or write)
- **MUST BE POOLED** - one per concurrent stream
- Used in `_is_our_stream()` to identify stream ownership

#### `_pump_outgoing_msg` (Line 70)
```c
static openlcb_worker_message_t _pump_outgoing_msg;
```
- Dedicated output buffer for pump's outgoing messages
- Contains embedded `openlcb_msg_t` and payload buffer
- Wired into `_pump_sm_info.outgoing_msg_info`
- **MUST BE POOLED** - one per concurrent stream

#### `_pump_sm_info` (Line 73)
```c
static openlcb_statemachine_info_t _pump_sm_info;
```
- Pump's own state machine context
- Pointers initialized to reference `_pump_outgoing_msg`
- Used as the context for Layer 1 API calls
- **MUST BE POOLED** - one per concurrent stream

#### `_read_buffer` (Line 76)
```c
static uint8_t _read_buffer[LEN_MESSAGE_BYTES_STREAM];
```
- Temporary buffer where per-space read callbacks write data
- Data staging area before `stream_send_data()` call
- **SHOULD BE POOLED** - one per concurrent stream
- Alternative: stack-allocate in pump loop if RAM limited

---

## 2. Layer 1 Stream State Table Configuration

**File:** `protocol_stream_handler.c` (lines 53-55)

```c
#ifndef USER_DEFINED_MAX_STREAM_COUNT
#define USER_DEFINED_MAX_STREAM_COUNT 1
#endif
```

**Current Status:**
- Defaults to 1 stream maximum
- **NOT defined in any `openlcb_user_config.h`** - all use default
- Must be increased to support concurrent config-mem streams
- Recommendation: User can define `USER_DEFINED_MAX_STREAM_COUNT` in their config

---

## 3. Config-Mem Stream Context Structure

**File:** `protocol_config_mem_stream_handler.h` (lines 139-192)

```c
typedef struct {
    config_mem_stream_phase_enum phase;
    stream_state_t *stream;
    openlcb_node_t *node;
    const user_address_space_info_t *space_info;
    config_mem_stream_read_request_func_t read_request_func;
    config_mem_stream_write_request_func_t write_request_func;
    uint32_t start_address;
    uint32_t current_address;
    uint32_t total_bytes;
    uint32_t bytes_sent;
    uint8_t reply_cmd_byte;
    uint8_t reply_fail_cmd_byte;
    uint8_t dest_stream_id;
    uint8_t source_stream_id;
    uint8_t tick_snapshot;
    uint16_t remote_alias;
    node_id_t remote_node_id;
} config_mem_stream_context_t;
```

**Critical for pooling:**
- `stream->context` pointer points to the context instance
- Used in `_is_our_stream()` to identify ownership
- Each pooled context will have different identity

---

## 4. Stream Ownership Check Mechanism

**File:** `protocol_config_mem_stream_handler.c` (lines 89-93)

```c
static bool _is_our_stream(stream_state_t *stream) {
    return (stream && stream->context == &_context);
}
```

**How it works:**
- Stream callbacks check if stream belongs to config-mem layer
- Compares `stream->context` pointer against `&_context`
- If match: config-mem handles it
- If no match: forward to user callbacks

**For pooling:**
- Each pooled context will have unique address
- Dispatch checks against the correct pooled context pointer
- Allows multiple concurrent config-mem streams to coexist

---

## 5. Pump Loop & State Machine

**File:** `protocol_config_mem_stream_handler.c` (lines 882-951)

### Phase Enumeration

```c
typedef enum {
    CONFIG_MEM_STREAM_PHASE_IDLE,
    CONFIG_MEM_STREAM_PHASE_WAIT_INITIATE_REPLY,
    CONFIG_MEM_STREAM_PHASE_SEND_REPLY_DATAGRAM,
    CONFIG_MEM_STREAM_PHASE_PUMPING,
    CONFIG_MEM_STREAM_PHASE_SEND_COMPLETE,
    CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE,
    CONFIG_MEM_STREAM_PHASE_WRITE_RECEIVING,
    CONFIG_MEM_STREAM_PHASE_WRITE_SEND_REPLY
} config_mem_stream_phase_enum;
```

### Pump Loop Behavior

Called once per main loop via `ProtocolConfigMemStreamHandler_run()`:

1. **Transmit outgoing message:** If `_pump_sm_info.outgoing_msg_info.valid`, send and clear
2. **State machine switch on `_context.phase`:**
   - **IDLE:** Nothing to do
   - **WAIT_INITIATE_REPLY:** Passive - handled by callback
   - **SEND_REPLY_DATAGRAM:** Load Read Stream Reply OK datagram
   - **PUMPING:** Call `_pump_next_chunk()` to load and send data
   - **SEND_COMPLETE:** Send Stream Data Complete, reset context
   - **WRITE_WAIT_STREAM_INITIATE:** Passive - handled by callback
   - **WRITE_RECEIVING:** Passive - handled by callback
   - **WRITE_SEND_REPLY:** Load Write Stream Reply OK datagram

**Concurrency implication:**
- Current pump only handles one phase at a time
- Must iterate pooled contexts to handle multiple active operations

---

## 6. Callback Router Functions

All callbacks receive `stream_state_t *stream` and use `_is_our_stream()` to dispatch:

### `ProtocolConfigMemStreamHandler_on_initiate_request()` (lines 1015-1041)

For write operations:
- Matches by `_context.phase == WRITE_WAIT_STREAM_INITIATE` and `remote_alias`
- Assigns `stream->context = &_context`
- Transitions phase to `WRITE_RECEIVING`

**For pooling:** Match against pooled context's remote alias, assign its address

### `ProtocolConfigMemStreamHandler_on_initiate_reply()` (lines 1054-1082)

For read operations:
- Checks `_is_our_stream(stream)`
- On accept: transitions to `SEND_REPLY_DATAGRAM`
- On reject: resets context to IDLE

### `ProtocolConfigMemStreamHandler_on_data_received()` (lines 1096-1138)

For write operations:
- Checks `_is_our_stream() && phase == WRITE_RECEIVING`
- Calls per-space write callback
- Sends Stream Data Proceed when window full

### `ProtocolConfigMemStreamHandler_on_data_proceed()` (lines 1145-1169)

For read operations:
- Checks `_is_our_stream()`
- Extends send window
- Refreshes timeout tick

### `ProtocolConfigMemStreamHandler_on_complete()` (lines 1179-1199)

For both:
- Checks `_is_our_stream()`
- Resets context to IDLE

---

## 7. Initialization and Teardown

### Initialization (`ProtocolConfigMemStreamHandler_initialize()`)
**File:** lines 327-335

```c
void ProtocolConfigMemStreamHandler_initialize(
        const interface_protocol_config_mem_stream_handler_t *interface) {
    _interface = (interface_protocol_config_mem_stream_handler_t *) interface;
    _reset_context();
    _init_pump_outgoing();
}
```

**For pooling:** Initialize each pooled context, buffer, and statemachine info

### Reset Context (`_reset_context()`)
**File:** lines 98-103

```c
static void _reset_context(void) {
    memset(&_context, 0, sizeof(_context));
    _context.phase = CONFIG_MEM_STREAM_PHASE_IDLE;
}
```

### Initialize Pump Outgoing (`_init_pump_outgoing()`)
**File:** lines 111-129

Wires `_pump_sm_info` pointers to `_pump_outgoing_msg`:
- `msg_ptr` → outgoing message
- `payload` → worker payload buffer
- Sets `payload_type = WORKER`
- Clears initial state

---

## 8. Timeout Handling

**File:** `ProtocolConfigMemStreamHandler_check_timeouts()` (lines 965-994)

```c
void ProtocolConfigMemStreamHandler_check_timeouts(uint8_t current_tick) {
    if (_context.phase not in active states) return;
    
    uint8_t elapsed = (uint8_t) (current_tick - _context.tick_snapshot);
    if (elapsed < CONFIG_MEM_STREAM_TIMEOUT_TICKS) return;
    
    // Timed out
    if (_context.stream) {
        _interface->stream_send_terminate(&_pump_sm_info, _context.stream,
                ERROR_TEMPORARY_TIME_OUT);
    }
    _reset_context();
}
```

**For pooling:**
- Iterate all pooled contexts checking `tick_snapshot`
- Each context tracks its own timeout independently

---

## 9. Layer 1 Stream Handler Integration

**File:** `protocol_stream_handler.c`

### Stream Table (Already Supports Pooling)

```c
static stream_state_t _stream_table[USER_DEFINED_MAX_STREAM_COUNT];
```

- Already dimensioned by user configuration
- Config-mem handler sets `stream->context` to claim ownership
- Default USER_DEFINED_MAX_STREAM_COUNT = 1

### Stream ID Counters (Global, Shared)

```c
static uint8_t _next_dest_stream_id = 0;
static uint8_t _next_source_stream_id = 0;
```

- Monotonic (wrap at 0xFE)
- Assigned per stream, not per context
- **No pooling needed** - global counters work across all streams

---

## 10. Pooling Strategy Summary

### Items to Pool

| Item | File | Type | Count | Notes |
|------|------|------|-------|-------|
| `config_mem_stream_context_t` | config_mem_handler.c | Struct | USER_DEFINED_MAX_STREAM_COUNT | Per-operation state |
| `openlcb_worker_message_t` | config_mem_handler.c | Buffer | USER_DEFINED_MAX_STREAM_COUNT | Outgoing message |
| `openlcb_statemachine_info_t` | config_mem_handler.c | Struct | USER_DEFINED_MAX_STREAM_COUNT | Pump context |
| `uint8_t[]` read_buffer | config_mem_handler.c | Array | USER_DEFINED_MAX_STREAM_COUNT | Data staging (or stack) |

### Items NOT to Pool

| Item | Reason |
|------|--------|
| `_interface` | Single shared DI interface |
| `_next_dest_stream_id` | Global counter, not per-context |
| `_next_source_stream_id` | Global counter, not per-context |
| `_stream_table[]` in Layer 1 | Already pooled by Layer 1 |

---

## 11. Implementation Approach

### Option A: Static Pool (Simplest)

Define pooled arrays at module scope:

```c
static config_mem_stream_context_t _context_pool[USER_DEFINED_MAX_STREAM_COUNT];
static openlcb_worker_message_t _pump_outgoing_msg_pool[USER_DEFINED_MAX_STREAM_COUNT];
static openlcb_statemachine_info_t _pump_sm_info_pool[USER_DEFINED_MAX_STREAM_COUNT];
static uint8_t _read_buffer_pool[USER_DEFINED_MAX_STREAM_COUNT][LEN_MESSAGE_BYTES_STREAM];
```

Initialize each in `ProtocolConfigMemStreamHandler_initialize()`:
```c
for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {
    memset(&_context_pool[i], 0, sizeof(config_mem_stream_context_t));
    _context_pool[i].phase = CONFIG_MEM_STREAM_PHASE_IDLE;
    
    memset(&_pump_outgoing_msg_pool[i], 0, sizeof(openlcb_worker_message_t));
    memset(&_pump_sm_info_pool[i], 0, sizeof(openlcb_statemachine_info_t));
    
    _pump_sm_info_pool[i].outgoing_msg_info.msg_ptr = 
        &_pump_outgoing_msg_pool[i].openlcb_msg;
    _pump_sm_info_pool[i].outgoing_msg_info.msg_ptr->payload =
        (openlcb_payload_t *) _pump_outgoing_msg_pool[i].openlcb_payload;
    // ... rest of init
}
```

### Pump Loop Changes

```c
void ProtocolConfigMemStreamHandler_run(void) {
    for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {
        _context = &_context_pool[i];  // Select context for this iteration
        _pump_sm_info = &_pump_sm_info_pool[i];
        
        // Try to send pending message
        if (_pump_sm_info->outgoing_msg_info.valid) {
            if (_interface->send_openlcb_msg(_pump_sm_info->outgoing_msg_info.msg_ptr)) {
                _pump_sm_info->outgoing_msg_info.valid = false;
            }
            continue;  // Try next context if this one has pending message
        }
        
        // State machine on _context->phase
        switch (_context->phase) {
            case CONFIG_MEM_STREAM_PHASE_IDLE:
                break;
            // ... other phases ...
        }
    }
}
```

### Callback Dispatch Changes

```c
bool ProtocolConfigMemStreamHandler_on_initiate_request(...) {
    for (int i = 0; i < USER_DEFINED_MAX_STREAM_COUNT; i++) {
        if (_context_pool[i].phase == CONFIG_MEM_STREAM_PHASE_WRITE_WAIT_STREAM_INITIATE &&
                stream->remote_alias == _context_pool[i].remote_alias) {
            
            stream->context = &_context_pool[i];
            _context_pool[i].stream = stream;
            // ... rest of assignment
            return true;
        }
    }
    
    // Forward to user callbacks
    if (_interface->user_on_initiate_request) {
        return _interface->user_on_initiate_request(statemachine_info, stream);
    }
    return false;
}
```

---

## 12. Critical Implementation Notes

### Pointer vs Array Index

Current code uses pointers:
```c
static config_mem_stream_context_t _context;
// Used as: _context.phase, _context.stream, etc.
```

With pooling:
```c
static config_mem_stream_context_t _context_pool[USER_DEFINED_MAX_STREAM_COUNT];
// Option 1: Iterator with pointer
config_mem_stream_context_t *_current_context = &_context_pool[i];

// Option 2: Macro to access by index
#define _context (_context_pool[_context_index])

// Option 3: Dynamic pointer (more refactoring)
static config_mem_stream_context_t *_context = NULL;
```

**Recommendation:** Use a pointer that changes per loop iteration (Option 1) or thread-local-like pattern with careful scoping.

### read_buffer Allocation

Three choices:

1. **Pool it:** `static uint8_t _read_buffer_pool[MAX][LEN_MESSAGE_BYTES_STREAM];`
   - Wastes RAM if not all streams active
   - Simplest implementation

2. **Stack allocate:** `uint8_t read_buffer[LEN_MESSAGE_BYTES_STREAM];` in `_pump_next_chunk()`
   - Saves RAM if LEN_MESSAGE_BYTES_STREAM is small
   - Verify stack depth is acceptable

3. **Shared with spinlock:** Single buffer protected by atomics/mutex
   - Most complex, only needed for true concurrency

**Recommendation:** Pool it for simplicity, or stack-allocate if stack space is available and LEN_MESSAGE_BYTES_STREAM fits.

---

## 13. Testing Strategy

### Unit Test Coverage

1. **Single stream (baseline):** Verify existing tests pass
2. **Two concurrent reads:** Different remote nodes, verify no context corruption
3. **Two concurrent writes:** Different remote nodes, verify data correctness
4. **Read + Write overlapping:** One node reading while another writing, verify isolation
5. **Timeout handling:** Ensure one timeout doesn't affect other streams
6. **Context exhaustion:** USER_DEFINED_MAX_STREAM_COUNT = 2, send 3 requests, verify rejection

### Edge Cases

- Stream completion doesn't affect other active streams
- Callback dispatch routes to correct pooled context
- Timeout tick refresh only affects the correct context

---

## 14. File Changes Summary

### Files Requiring Changes

| File | Changes |
|------|---------|
| `protocol_config_mem_stream_handler.c` | Pool static buffers, add loop iteration, refactor pump and callbacks |
| `protocol_config_mem_stream_handler.h` | Possibly define pool size constant |
| `openlcb_user_config.h` (template) | Document USER_DEFINED_MAX_STREAM_COUNT |

### Files NOT Requiring Changes

- `protocol_stream_handler.c/.h` - already supports pooling via USER_DEFINED_MAX_STREAM_COUNT
- `can_tx_message_handler.c` - stateless
- Other modules - only interact via callbacks

---

## 15. Backward Compatibility

**Concern:** Default USER_DEFINED_MAX_STREAM_COUNT = 1 preserves single-stream behavior

**Solution:** Add to `openlcb_user_config.h` template:
```c
/** Maximum concurrent config-mem streams (1 = single-threaded, no concurrency) */
#ifndef USER_DEFINED_MAX_STREAM_COUNT
#define USER_DEFINED_MAX_STREAM_COUNT 1
#endif
```

Users wishing concurrent streams increase this value in their config.

---

## 16. Performance Implications

### With Pooling

- **Pump loop:** O(N) per iteration where N = USER_DEFINED_MAX_STREAM_COUNT
- **Callbacks:** O(N) to find matching context (linear search through pool)
- **Memory:** +N × (sizeof(context) + sizeof(outgoing_msg) + sizeof(sm_info) + LEN_MESSAGE_BYTES_STREAM)

### Optimization Opportunities (Future)

- Hash table on (remote_alias, phase) for O(1) callback dispatch
- Per-phase lists to avoid checking IDLE contexts in pump
- Double-buffer reads to eliminate separate `_read_buffer`

---

## Summary

| Aspect | Details |
|--------|---------|
| **Static State to Pool** | `_context`, `_pump_outgoing_msg`, `_pump_sm_info`, `_read_buffer` |
| **Pool Size** | `USER_DEFINED_MAX_STREAM_COUNT` (default 1) |
| **Ownership Check** | `stream->context` pointer comparison |
| **Pump Loop** | Iterate all pooled contexts each loop |
| **Callbacks** | Search pool to find correct context |
| **Timeout** | Independent per context, checked during iteration |
| **Layer 1 Integration** | No changes - already supports pooling |
| **Backward Compat** | Default = 1 preserves existing behavior |
| **Effort** | Medium - local refactoring, no architectural changes |

