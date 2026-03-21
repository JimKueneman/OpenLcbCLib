<!--
  ============================================================
  STATUS: REJECTED — SUPERSEDED BY DESIGN DECISION

  This plan proposed embedding a `uint16_t alias` field directly in
  train_listener_entry_t (openlcb_types.h) and caching the alias at attach time.
  The plan itself identified the core problem: the Listener Attach command arrives
  FROM the throttle, not the listener, so source_alias is the throttle's alias,
  not the listener's.  Option 3 at the bottom of the plan pointed toward using the
  existing alias_mapping infrastructure instead.

  The chosen design (implemented in alias_mapping_listener.h/c) keeps alias
  resolution entirely in the CAN driver layer — a separate table of
  listener_alias_entry_t records indexed by node_id, populated on-demand via AMD
  replies, and queried transparently by the TX path via a nullable DI pointer.
  This is cleaner: train_listener_entry_t remains transport-agnostic (no CAN alias
  field that would be meaningless on non-CAN transports).

  All functions specified in plan_item_3 (Part A) ARE implemented, just in
  alias_mapping_listener.h/c instead of as a field on the listener struct.
  This plan is closed as superseded by the better design, not as a defect.
  ============================================================
-->

# Plan: Listener-to-Listener Alias Resolution on CAN

## 1. Summary

To forward Train Control commands (speed, function) to listener (consist member) nodes on CAN,
the train node needs the CAN alias of each listener node. The listener infrastructure stores
only the 48-bit Node ID (`train_listener_entry_t.node_id`). On CAN, messages are addressed by
12-bit alias, not by Node ID.

This is a prerequisite for Item 3 (Consist Forwarding). Without alias resolution, forwarded
commands cannot be built for CAN transmission.

## 2. Files to Modify

### Source (canonical)

| File | Change |
|------|--------|
| `src/openlcb/openlcb_types.h` | Add `uint16_t alias` field to `train_listener_entry_t` |
| `src/openlcb/protocol_train_handler.c` | Populate listener alias on attach; clear on detach |
| `src/drivers/canbus/can_rx_message_handler.c` | (Possibly) Maintain an alias-to-node-id cache from observed AMD frames |

**Note:** Application copies in `applications/` are updated automatically via the
project's copy shell script and should NOT be edited manually.

### Tests

| File | Change |
|------|--------|
| `src/openlcb/protocol_train_handler_Test.cxx` | Add tests for alias storage on attach, alias lookup for forwarding |

## 3. Implementation Steps

### Approach A (Recommended): Cache alias at attach time

When a Listener Attach command arrives, the incoming message contains the requester's CAN alias
in `source_alias`. Store this alongside the Node ID in the listener entry.

### Step 1. Add alias field to train_listener_entry_t

In `openlcb_types.h`:

```c
    /** @brief A single listener entry for a train consist. */
typedef struct {

    node_id_t node_id; /**< Listener node ID (0 = unused) */
    uint16_t alias;    /**< Listener CAN alias (0 = unknown) */
    uint8_t flags;     /**< Listener flags (reverse, link F0, link Fn, hide) */

} train_listener_entry_t;
```

### Step 2. Populate alias on Listener Attach

In `protocol_train_handler.c`, in the Listener Attach handler, after storing the Node ID:

```c
    listener->node_id = listener_node_id;
    listener->alias = statemachine_info->incoming_msg_info.msg_ptr->source_alias;
    listener->flags = flags;
```

**Note:** The Listener Attach command contains the listener's Node ID in the payload. The
`source_alias` in the incoming message is the alias of the node that SENT the attach command
(which may be a throttle, not the listener itself). We need to verify the protocol:

Per TrainControlS: The Listener Attach command is sent BY the throttle, specifying the
listener Node ID in the payload. The command comes FROM the throttle's alias, not the
listener's alias. This means `source_alias` is the throttle's alias, NOT the listener's.

**Revised approach:** We cannot simply cache `source_alias`. We need to resolve the listener
Node ID to an alias through one of these mechanisms:

#### Option 1: Verify Node ID / AME to resolve alias

Before forwarding, send a Verify Node ID (addressed) to the listener Node ID. The response
(Verified Node ID) comes from the listener's actual alias. Cache it.

#### Option 2: Maintain a global alias-to-node-id mapping

The CAN layer already processes AMD (Alias Map Definition) frames in `can_rx_message_handler.c`.
If a global alias cache is maintained (mapping Node IDs to their CAN aliases for all nodes
seen on the bus), the train handler can look up any Node ID.

#### Option 3: Use the existing alias_mapping infrastructure

The `alias_mapping_t` structure and `alias_mapping_find_mapping_by_alias()` are already part
of the CAN driver's interface. If there is a reverse lookup
(`alias_mapping_find_mapping_by_node_id()`), it can be used.

### Step 3. Verify existing alias mapping capabilities

Check if `alias_mapping_find_mapping_by_node_id()` or equivalent exists. If it does, the
`send_train_command` callback (from Item 3) can resolve aliases on-the-fly without caching
in the listener entry.

If no reverse lookup exists, add one:

```c
    /** @brief Find alias mapping by Node ID (reverse lookup). */
alias_mapping_t *AliasMappingTable_find_by_node_id(node_id_t node_id);
```

### Step 4. Wire alias resolution into the forwarding path

In the `send_train_command` callback (Item 3), before building the CAN frame:

```c
    alias_mapping_t *mapping = _interface->alias_mapping_find_by_node_id(dest_node_id);

    if (!mapping) {

        // Alias unknown — cannot forward on CAN
        return;

    }

    uint16_t dest_alias = mapping->alias;
    // ... build and send CAN frame using dest_alias ...
```

## 4. Test Changes

### Test 1: Alias resolved for listener node

```cpp
TEST(ProtocolTrainHandler, listener_alias_resolved)
{
    // Setup: alias mapping with known listener Node ID -> alias
    // Forward speed to listener
    // Verify outgoing message uses correct alias
}
```

### Test 2: Unknown alias — forwarding skipped

```cpp
TEST(ProtocolTrainHandler, listener_alias_unknown_skipped)
{
    // Setup: listener Node ID not in alias mapping
    // Forward speed
    // Verify no outgoing message (graceful skip)
}
```

### Verification procedure

```bash
cd ~/Documents/OpenLcbCLib/test && rm -rf build && mkdir build && cd build && cmake .. && make
```

## 5. Risk Assessment

| Risk | Level | Mitigation |
|------|-------|------------|
| Alias stale after reallocation | MEDIUM | If a listener node reallocates its alias (after CID conflict), the cached alias is stale. Need to update on AMD/AMR events. |
| No reverse alias lookup | MEDIUM | May need to implement `find_by_node_id()` if it doesn't exist. Linear scan of the alias table is acceptable for small node counts. |
| Transport independence | LOW | The alias field in `train_listener_entry_t` is CAN-specific. On non-CAN transports, it would be ignored. The `send_train_command` callback abstracts the transport. |

## 6. Estimated Effort

| Task | Time |
|------|------|
| Add alias field to train_listener_entry_t | 5 min |
| Verify/implement reverse alias lookup | 20 min |
| Wire into forwarding path | 10 min |
| Add 2 tests | 15 min |
| Build and run full test suite | 5 min |
| **Total** | **~55 min** |

## 7. Dependencies

- This is a **prerequisite** for Item 3 (Consist Forwarding).
- Item 3 should be implemented after this item is complete.
