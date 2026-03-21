<!--
  ============================================================
  STATUS: REJECTED
  This early design sketch was superseded by `sibling_dispatch_plan.md`, which was fully implemented. No code was written directly from this document; the newer plan replaces it.
  ============================================================
-->

# Plan: Virtual Node Sibling Dispatch

## 1. Summary

The library supports multiple virtual nodes, and outbound Tx messages go on the wire correctly.
However, when one virtual node sends a message, it is not dispatched to sibling virtual nodes
within the same library instance. Only external nodes see the message. This means virtual nodes
hosted on the same device cannot communicate with each other.

The fix adds a loopback mechanism: after a message is placed on the outgoing wire, a copy is
also fed back into the incoming message processing path for delivery to sibling virtual nodes.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine.c` | Add loopback injection after outgoing message is sent; filter to avoid self-delivery |
| `src/openlcb/openlcb_buffer_fifo.c` | (Possibly) Add a second FIFO for loopback messages, or reuse existing |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/openlcb_main_statemachine_Test.cxx` | Add tests for sibling dispatch of global and addressed messages |

## 3. Implementation Steps

### Design Considerations

The main statemachine processes messages by:
1. Popping from `OpenLcbBufferFifo` (incoming)
2. Iterating all nodes via `get_first()`/`get_next()` enumeration
3. Dispatching to the appropriate handler for each node

For sibling dispatch, when a virtual node produces an outgoing message:
1. The message goes on the CAN wire (normal path)
2. A copy of the message is injected back into the FIFO (or a secondary queue)
3. On the next processing cycle, all nodes (except the sender) see the loopback message

### Step 1. Identify the injection point

After the outgoing message is transmitted in `_handle_run()` or `_handle_try_transmit()`,
check if the message was produced by a local virtual node. If so, allocate a copy and push
it into `OpenLcbBufferFifo`.

### Step 2. Implement loopback copy

```c
    /**
     * @brief Copy an outgoing message back into the incoming FIFO for sibling dispatch.
     *
     * @details Allocates a new buffer, copies the outgoing message content,
     * and pushes it into the incoming FIFO. The message is processed by all
     * sibling nodes on the next dispatch cycle.
     *
     * @param outgoing_msg  The message that was just transmitted.
     */
static void _loopback_to_siblings(openlcb_msg_t *outgoing_msg) {

    openlcb_msg_t *loopback = OpenLcbBufferStore_allocate_buffer(outgoing_msg->payload_type);

    if (!loopback) { return; }  // Best-effort: skip if no buffers available

    // Copy message header
    loopback->mti = outgoing_msg->mti;
    loopback->source_alias = outgoing_msg->source_alias;
    loopback->source_id = outgoing_msg->source_id;
    loopback->dest_alias = outgoing_msg->dest_alias;
    loopback->dest_id = outgoing_msg->dest_id;
    loopback->payload_count = outgoing_msg->payload_count;

    // Copy payload
    for (uint16_t i = 0; i < outgoing_msg->payload_count; i++) {

        (*loopback->payload)[i] = (*outgoing_msg->payload)[i];

    }

    OpenLcbBufferFifo_push(loopback);

}
```

### Step 3. Filter self-delivery

In the main dispatch loop, when iterating nodes for an incoming message, skip the node
whose ID/alias matches the message's source:

```c
    if (statemachine_info->openlcb_node->id == incoming_msg->source_id) {

        continue;  // Don't deliver to the sender itself

    }
```

### Step 4. Conditional compilation

Wrap the loopback feature in a feature flag to allow disabling on memory-constrained systems:

```c
#ifdef OPENLCB_FEATURE_SIBLING_DISPATCH
    _loopback_to_siblings(outgoing_msg);
#endif
```

## 4. Test Changes

### Test 1: Global message delivered to sibling

```cpp
TEST(OpenLcbMainStatemachine, sibling_dispatch_global_message)
{
    // Setup: two virtual nodes (A and B)
    // Node A sends a global message (e.g., Verified Node ID)
    // Verify Node B receives and processes it
    // Verify Node A does NOT re-process its own message
}
```

### Test 2: Addressed message delivered to sibling

```cpp
TEST(OpenLcbMainStatemachine, sibling_dispatch_addressed_message)
{
    // Setup: two virtual nodes (A and B)
    // Node A sends a message addressed to Node B
    // Verify Node B receives it
}
```

### Test 3: No loopback when only one node

```cpp
TEST(OpenLcbMainStatemachine, sibling_dispatch_single_node_no_loopback)
{
    // Setup: one virtual node
    // Send a message
    // Verify no duplicate processing
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Buffer exhaustion | MEDIUM | Loopback allocates an extra buffer per outgoing message. On memory-constrained devices, this could exhaust the pool. Feature flag allows disabling. |
| Infinite loop | HIGH | If Node A sends a message, loopback delivers to Node B, which produces a reply, which loops back to Node A, etc. Must ensure loopback messages are NOT themselves looped back. Mark loopback messages with a flag. |
| Performance | MEDIUM | Extra buffer allocation + copy + dispatch cycle per outgoing message. Acceptable on full-size systems, may be too expensive on 8-bit MCUs. |
| Ordering | LOW | Loopback messages are processed after external messages in FIFO order. This is correct — sibling delivery is asynchronous on real networks too. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Design loopback mechanism | 15 min |
| Implement _loopback_to_siblings | 15 min |
| Add self-delivery filtering | 10 min |
| Add loopback flag to prevent re-loopback | 10 min |
| Add feature flag gating | 5 min |
| Add 3 tests | 20 min |
| Build and run full test suite | 5 min |
| **Total** | **~80 min** |
