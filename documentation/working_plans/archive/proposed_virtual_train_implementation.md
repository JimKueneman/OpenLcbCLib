# ARCHIVED — HISTORICAL / BRAINSTORMING

> **Status:** Superseded. This document was exploratory brainstorming that led
> to the unified sibling dispatch plan.
>
> - **Categories 1-2** (CAN alias collision, CAN control frames): Folded into
>   `plan_unified_sibling_dispatch.md` as **Phase 3 — CAN Layer**.
> - **Categories 3-5** (OpenLCB message loopback): Fully replaced by the
>   zero-FIFO sequential dispatch design in `plan_unified_sibling_dispatch.md`
>   (Phase 1 implemented, Phase 2 pending).
>
> Archived: 2026-03-17

---

# Proposed Virtual Train Implementation

Date: 2026-03-17

## Overview

This document captures proposed changes to make OpenLcbCLib fully compliant for
virtual node support. Each category maps to a message class from the
`OpenLcbCLib_OpenMRN_message_comparison.md` analysis. Changes are evaluated
against what OpenLcbCLib already has implemented, reusing existing infrastructure
wherever possible.

---

## Category 1: CAN Alias Negotiation (Inhibited -> Permitted)

### Problem

When multiple virtual nodes share a single physical instance, each node
independently generates a 12-bit CAN alias via its LFSR seed. The current code
in `state_generate_alias()` only rejects alias `0x000` (invalid per spec). It
does not check whether a sibling virtual node already holds the generated alias.

Two siblings could derive the same alias, both register it in separate mapping
table slots (since `alias_mapping_register()` matches on `node_id`, not
`alias`), and neither would detect the collision. External nodes would see
duplicate aliases on the bus.

### Existing Infrastructure

The fix requires no new data structures, interfaces, or mechanisms. Everything
needed is already in place:

- **`alias_mapping_find_mapping_by_alias(uint16_t alias)`** — already wired into
  `interface_can_login_message_handler_t`. Returns a pointer to the mapping entry
  if the alias exists in the table, NULL if not.

- **`_generate_seed(seed)`** — advances the LFSR one step. Already used in the
  `while (alias == 0)` retry loop inside `state_generate_alias()`.

- **`_generate_alias(seed)`** — derives a 12-bit alias from the current seed.
  Already used in the same retry loop.

- **Conflict-retry path** — when a duplicate is detected externally, the RX ISR
  sets `is_duplicate` on the mapping entry. The main loop calls
  `_process_duplicate_aliases()` which calls `alias_mapping_unregister()` first
  (clearing the entry completely), then `_reset_node()` which sets
  `run_state = RUNSTATE_GENERATE_SEED`. This guarantees the old alias is removed
  from the mapping table before `state_generate_alias()` runs again.

### Proposed Change

**File:** `src/drivers/canbus/can_login_message_handler.c`
**Function:** `CanLoginMessageHandler_state_generate_alias()`

Widen the existing `while (alias == 0)` rejection loop to also reject aliases
that are already present in the mapping table.

#### Current Code (lines 144-165):

```c
    /** @brief State 3: Derives a 12-bit alias from the seed, registers it, and transitions to LOAD_CID07. */
void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    while (can_statemachine_info->openlcb_node->alias == 0) {

        can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
        can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    }

    _interface->alias_mapping_register(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    if (_interface->on_alias_change) {

        _interface->on_alias_change(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;

}
```

#### Proposed Code:

```c
    /** @brief State 3: Derives a 12-bit alias from the seed, registers it, and transitions to LOAD_CID07.
     *  Rejects alias 0x000 (invalid per spec) and any alias already held by a sibling virtual node
     *  in the alias mapping table. */
void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info) {

    can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    while ((can_statemachine_info->openlcb_node->alias == 0) ||
            (_interface->alias_mapping_find_mapping_by_alias(can_statemachine_info->openlcb_node->alias) != (void*) 0)) {

        can_statemachine_info->openlcb_node->seed = _generate_seed(can_statemachine_info->openlcb_node->seed);
        can_statemachine_info->openlcb_node->alias = _generate_alias(can_statemachine_info->openlcb_node->seed);

    }

    _interface->alias_mapping_register(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    if (_interface->on_alias_change) {

        _interface->on_alias_change(can_statemachine_info->openlcb_node->alias, can_statemachine_info->openlcb_node->id);

    }

    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;

}
```

#### Diff Summary:

- Line 143: Expanded Doxygen comment to document sibling collision rejection.
- Line 148-149: Changed `while (alias == 0)` to
  `while ((alias == 0) || (find_mapping_by_alias(alias) != NULL))`.
  This is the only behavioral change. The seed-advance and alias-derive body of
  the loop is unchanged.

### Why This Is Safe

- **No stale self-match:** On re-login after conflict, `_process_duplicate_aliases()`
  calls `alias_mapping_unregister()` before `_reset_node()`. The old entry is
  fully cleared (alias, node_id, flags all zeroed) before the node re-enters
  `state_generate_alias()`. So `find_mapping_by_alias()` will not find the
  node's own previous alias.

- **First login:** On initial startup, no entries exist in the mapping table. The
  check adds negligible overhead (one table scan per candidate alias, table size
  equals node count).

- **Bounded retry:** The alias space is 12 bits (4095 valid values). The number
  of virtual nodes is small (typically 2-20). The LFSR is designed to cycle
  through the full space. Finding an unused alias is near-instant.

### Scope

- One function modified
- Zero new interface pointers
- Zero new data structures
- Zero new files

---

## Category 2: CAN Control — Ongoing (Permitted + Initialized)

### Problem

CAN control frames (AME, AMD, AMR, RID defense) are handled globally in
`can_rx_message_handler.c` using the alias mapping table. Outgoing CAN control
frames are not looped back to sibling virtual nodes. The question is whether
this matters.

### Assessment

**No changes needed.** Every ongoing CAN control scenario is already covered by
the global handlers in `can_rx_message_handler.c`, which operate on the shared
alias mapping table containing ALL local node aliases. The Category 1 fix
(sibling alias collision prevention at generation time) closes the one remaining
hole.

### Code Evidence — Scenario by Scenario

**CID → RID Defense** (`can_rx_message_handler.c` lines 387-413):

An external node sends a CID whose alias matches one of our local nodes. The
global handler checks ALL local aliases via the mapping table and sends RID
defense. Sibling CIDs never need mutual checking because Category 1 guarantees
unique aliases at generation time.

```c
void CanRxMessageHandler_cid_frame(can_msg_t* can_msg) {

    if (!can_msg) {

        return;

    }

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (alias_mapping) {

        can_msg_t *reply_msg = _interface->can_buffer_store_allocate_buffer();

        if (reply_msg) {

            reply_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | source_alias;
            reply_msg->payload_count = 0;

            CanBufferFifo_push(reply_msg);

        }

    }

}
```

Coverage: `alias_mapping_find_mapping_by_alias()` searches the entire mapping
table. If any local node (including virtual nodes) holds the colliding alias,
RID is sent. One global handler defends all local aliases.

---

**AME → AMD — Targeted** (`can_rx_message_handler.c` lines 551-573):

An external node sends a targeted AME with a specific Node ID. The global
handler looks up by Node ID and responds with AMD if the node is local and
permitted.

```c
    if (can_msg->payload_count > 0) {

        alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_node_id(
                CanUtilities_extract_can_payload_as_node_id(can_msg));

        if (alias_mapping && alias_mapping->is_permitted) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping->alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping->node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

            return;

        }

        return;

    }
```

Coverage: `alias_mapping_find_mapping_by_node_id()` searches the entire mapping
table. Any local virtual node's Node ID will be found and AMD replied.

---

**AME → AMD — Global** (`can_rx_message_handler.c` lines 582-601):

An external node sends a global AME (empty payload). The handler iterates ALL
mapping table entries and sends AMD for every permitted alias.

```c
    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00 && alias_mapping_info->list[i].is_permitted) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping_info->list[i].node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

        }

    }
```

Coverage: Explicit loop over `ALIAS_MAPPING_BUFFER_DEPTH`. Every virtual node
with a permitted alias gets an AMD response. Complete coverage.

---

**AMR** (`can_rx_message_handler.c` lines 662-679):

An external node releases its alias. The global handler checks for duplicate
alias conflicts, scrubs the BufferList and FIFO of messages from the stale
alias, and clears the listener table.

```c
void CanRxMessageHandler_amr_frame(can_msg_t* can_msg) {

    _check_for_duplicate_alias(can_msg);

    uint16_t alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);

    _check_and_release_messages_by_source_alias(alias);

    OpenLcbBufferFifo_check_and_invalidate_messages_by_source_alias(alias);

    if (_interface->listener_clear_alias_by_alias) {

        _interface->listener_clear_alias_by_alias(alias);

    }

}
```

Coverage: All operations are alias-based, not node-specific. The scrub covers
all queued messages regardless of which virtual node they were destined for.

---

**Duplicate Alias Detection** (`can_rx_message_handler.c` lines 151-182):

Called by RID, AMD, AME, AMR, and Error Info handlers. Checks the source alias
of any received frame against ALL local aliases.

```c
static bool _check_for_duplicate_alias(can_msg_t* can_msg) {

    uint16_t source_alias = CanUtilities_extract_source_alias_from_can_identifier(can_msg);
    alias_mapping_t *alias_mapping = _interface->alias_mapping_find_mapping_by_alias(source_alias);

    if (!alias_mapping) {

        return false;

    }

    alias_mapping->is_duplicate = true;
    _interface->alias_mapping_set_has_duplicate_alias_flag();

    if (alias_mapping->is_permitted) {

        can_msg_t *outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

        if (outgoing_can_msg) {

            outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | source_alias;
            CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping->node_id, 0);
            CanBufferFifo_push(outgoing_can_msg);

        }

    }

    return true;

}
```

Coverage: `alias_mapping_find_mapping_by_alias()` searches all entries. If an
external frame's source alias collides with any local virtual node, the
duplicate is flagged and AMR is sent if the alias was permitted.

---

**Why sibling-to-sibling CAN control loopback is not needed:**

The global handlers act on behalf of ALL local nodes using the shared alias
mapping table. There is no per-node CAN control logic that would need to "see"
a sibling's CAN control frame. Specifically:

1. Sibling alias collisions are prevented at generation time (Category 1 fix).
2. External AME is answered for all local aliases by the global handler.
3. External CID collisions are defended for all local aliases by the global handler.
4. External AMR scrubs are alias-based, covering all virtual nodes.
5. Duplicate alias detection checks all local aliases in every received frame.

### Assumption

This assessment assumes **no CAN control frame loopback** is required between
sibling virtual nodes. The global handlers operate on shared data structures
(alias mapping table, listener table) that already contain information for all
local nodes. CAN control frames are a transport-level mechanism; the information
they carry is already available in memory. Looping them back to siblings would
be redundant — we already know the answer.

OpenMRN takes the same approach: `skipMember_` in `CanFrameWriteFlow::send()`
blocks all outgoing CAN frames from re-entering `frame_dispatcher()`. The one
exception (`send_global_alias_enquiry()`) is analyzed below.

---

### AME and Listener Alias Flush

#### Background

A **global AME** (empty payload) triggers a listener alias flush per
CanFrameTransferS §6.2.3. The current code in `ame_frame()` handles this for
externally received global AME:

```c
    // Global AME: discard cached listener aliases per CanFrameTransferS §6.2.3
    if (_interface->listener_flush_aliases) {

        _interface->listener_flush_aliases();

    }
```

`ListenerAliasTable_flush_aliases()` zeros the alias field on every listener
table entry, leaving node_ids intact (`alias_mapping_listener.c` lines 250-259):

```c
void ListenerAliasTable_flush_aliases(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        _table[i].alias = 0;
        _table[i].verify_pending = 0;

    }

}
```

#### Why OpenMRN Added Opt-In AME Loopback

OpenMRN's `send_global_alias_enquiry()` (`IfCan.cxx` lines 756-781) is NOT
about sibling visibility. It is a self-directed mechanism:

1. **Frame 1 (external):** Global AME with the sender's real alias → goes on
   the wire → tells every node on the bus to respond with AMD.
2. **Frame 2 (loopback with alias=0):** Injected directly into the local
   `frame_dispatcher()` → triggers `AMEGlobalQueryHandler::send()` → which
   calls `remote_aliases()->clear()` — flushing the local remote alias cache
   immediately.

Without frame 2, the local node's own cache would never be flushed because CAN
hardware does not echo transmitted frames. The `alias=0` prevents the handler
from responding with AMD (it only triggers the flush side effect).

The header comment says: *"This call should be used very sparingly because of
the effect it has on the entire bus."*

#### OpenLcbCLib Equivalent

We don't need CAN frame loopback for this. The listener table and alias mapping
table are shared data structures. On a global AME (incoming or self-originated),
one handler can do all the work in a single pass:

1. Flush the listener alias cache
2. Immediately repopulate entries whose `node_id` matches a local virtual node
   (alias is already known from the alias mapping table)
3. Send AMDs for all local virtual nodes
4. Done — no per-node propagation, no race conditions

#### Why One-Shot Matters for Virtual Nodes

If we propagated the global AME to each virtual node individually, two problems
arise:

1. **Flush-repopulate-flush race:** Node A processes the AME, flushes the cache.
   External AMD responses arrive and repopulate some entries. Node B then
   processes the looped-back AME and flushes again — wiping the fresh entries.

2. **Duplicate AMD responses:** Each virtual node would independently send its
   own AMD. But the global handler already sends AMD for ALL local aliases in
   one loop. Per-node dispatch would generate duplicate AMDs on the wire.

The one-shot approach avoids both: flush once, repopulate local entries
instantly, send all AMDs, free the message.

---

### Proposed Change — Incoming Global AME Handler (One-Shot)

The existing `ame_frame()` global AME branch already flushes and sends all AMDs.
The missing step is repopulating listener entries for local virtual nodes
immediately after the flush.

**File:** `src/drivers/canbus/can_rx_message_handler.c`
**Function:** `CanRxMessageHandler_ame_frame()` — global AME branch

#### Current Code (lines 575-601):

```c
    // Global AME: discard cached listener aliases per CanFrameTransferS §6.2.3
    if (_interface->listener_flush_aliases) {

        _interface->listener_flush_aliases();

    }

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00 && alias_mapping_info->list[i].is_permitted) {

            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping_info->list[i].node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

        }

    }
```

#### Proposed Code:

```c
    // Global AME: discard cached listener aliases per CanFrameTransferS §6.2.3
    if (_interface->listener_flush_aliases) {

        _interface->listener_flush_aliases();

    }

    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00 && alias_mapping_info->list[i].is_permitted) {

            // Repopulate listener table for local virtual nodes immediately.
            // Their aliases are already known — no need to wait for AMD off the wire.
            if (_interface->listener_set_alias) {

                _interface->listener_set_alias(alias_mapping_info->list[i].node_id,
                                               alias_mapping_info->list[i].alias);

            }

            // Send AMD for this local alias (external nodes need it)
            outgoing_can_msg = _interface->can_buffer_store_allocate_buffer();

            if (outgoing_can_msg) {

                outgoing_can_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(outgoing_can_msg, alias_mapping_info->list[i].node_id, 0);
                CanBufferFifo_push(outgoing_can_msg);

            }

        }

    }
```

#### Diff Summary:

- Added `listener_set_alias()` call inside the existing alias mapping loop,
  immediately after the flush and before the AMD response. This repopulates
  listener entries for local virtual nodes using the alias we already know.
- `listener_set_alias()` is already in the interface and already wired.
  If the `node_id` is not in the listener table it's a no-op (safe).
- Zero new interface pointers for this change.

---

### Proposed Change — Send Global AME with Local Flush (Self-Originated)

Same one-shot approach for when we originate the global AME ourselves.

**File:** `src/drivers/canbus/can_main_statemachine.h`

Add two OPTIONAL interface pointers and one new public function:

```c
        /** @brief OPTIONAL. Flush all cached listener aliases. NULL if unused.
         *  Typical: ListenerAliasTable_flush_aliases. */
        void (*listener_flush_aliases)(void);

        /** @brief OPTIONAL. Store a resolved alias for a listener Node ID. NULL if unused.
         *  Typical: ListenerAliasTable_set_alias. */
        void (*listener_set_alias)(node_id_t node_id, uint16_t alias);
```

**File:** `src/drivers/canbus/can_main_statemachine.c`

New function:

```c
    /**
     * @brief Sends a global AME on the wire and handles the local flush in one shot.
     *
     * @details Equivalent to OpenMRN's send_global_alias_enquiry() but without
     * CAN frame loopback. Performs three operations atomically:
     * -# Flush the listener alias cache (all aliases zeroed, node_ids preserved)
     * -# Repopulate entries for local virtual nodes immediately (alias is known
     *    from the alias mapping table — no need to wait for AMD off the wire)
     * -# Send AMD for every permitted local alias (external nodes need it)
     * -# Queue the global AME (external nodes will also send AMD back, which
     *    repopulates any remaining remote listener entries via amd_frame())
     *
     * This avoids per-node propagation of the AME, preventing flush-repopulate-flush
     * race conditions and duplicate AMD responses.
     *
     * @return true if the global AME was queued, false if no node available or
     *         buffer allocation failed.
     *
     * @warning Use sparingly — a global AME forces every node on the bus to
     *          respond with AMD. Per CanFrameTransferS §6.2.3.
     * @warning NOT thread-safe.
     */
bool CanMainStatemachine_send_global_alias_enquiry(void) {

    openlcb_node_t *node = _interface->openlcb_node_get_first(CAN_STATEMACHINE_NODE_ENUMRATOR_KEY);

    if (!node || node->alias == 0) {

        return false;

    }

    // Step 1: Flush the listener alias cache
    if (_interface->listener_flush_aliases) {

        _interface->listener_flush_aliases();

    }

    // Step 2: Repopulate local virtual node entries + send AMDs
    alias_mapping_info_t *alias_mapping_info = _interface->alias_mapping_get_alias_mapping_info();

    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping_info->list[i].alias != 0x00 && alias_mapping_info->list[i].is_permitted) {

            // Repopulate listener table for local virtual nodes immediately
            if (_interface->listener_set_alias) {

                _interface->listener_set_alias(alias_mapping_info->list[i].node_id,
                                               alias_mapping_info->list[i].alias);

            }

            // Send AMD for this local alias
            _interface->lock_shared_resources();
            can_msg_t *amd = CanBufferStore_allocate_buffer();
            _interface->unlock_shared_resources();

            if (amd) {

                amd->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | alias_mapping_info->list[i].alias;
                CanUtilities_copy_node_id_to_payload(amd, alias_mapping_info->list[i].node_id, 0);

                _interface->lock_shared_resources();
                CanBufferFifo_push(amd);
                _interface->unlock_shared_resources();

            }

        }

    }

    // Step 3: Queue the global AME (triggers AMD responses from external nodes)
    _interface->lock_shared_resources();
    can_msg_t *ame = CanBufferStore_allocate_buffer();
    _interface->unlock_shared_resources();

    if (!ame) {

        return false;

    }

    ame->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME | node->alias;
    ame->payload_count = 0;

    _interface->lock_shared_resources();
    CanBufferFifo_push(ame);
    _interface->unlock_shared_resources();

    return true;

}
```

**File:** `src/drivers/canbus/can_main_statemachine.h`

Add declaration:

```c
    /**
     * @brief Sends a global AME on the wire and handles the local flush in one shot.
     *
     * @details Flushes listener cache, repopulates local virtual node entries,
     * sends AMDs for all local aliases, then queues the global AME.
     *
     * @return true if the global AME was queued, false if no node available or
     *         buffer allocation failed.
     *
     * @warning Use sparingly — forces bus-wide AMD response.
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_send_global_alias_enquiry(void);
```

**File:** `src/drivers/canbus/can_config.c`

Wire the new interface pointers (inside `_build_main_statemachine()`):

```c
    // Listener alias management (OPTIONAL — NULL if OPENLCB_COMPILE_TRAIN not defined)
#ifdef OPENLCB_COMPILE_TRAIN
    _main_sm.listener_flush_aliases = &ListenerAliasTable_flush_aliases;
    _main_sm.listener_set_alias = &ListenerAliasTable_set_alias;
#endif
```

### Scope

- One new public function: `CanMainStatemachine_send_global_alias_enquiry()`
- One modified function: `CanRxMessageHandler_ame_frame()` (add repopulate step)
- Two new OPTIONAL interface pointers on `can_main_statemachine`: `listener_flush_aliases`, `listener_set_alias`
- Two new wiring lines in `can_config.c`
- Zero new files
- Zero CAN frame loopback infrastructure

---

## Categories 3-5: Unified OpenLCB Message Loopback

Categories 3 (Login), 4 (Global), and 5 (Addressed) all need the same loopback
mechanism. All outgoing OpenLCB messages — regardless of phase — should be
dispatched to sibling virtual nodes using a single, consistent path.

### Design Requirement

All OpenLCB messages go on the wire always (no optimization to suppress
sibling-to-sibling addressed messages). A loopback copy is also dispatched to
sibling virtual nodes so their protocol handlers can react.

---

### Failure Scenarios

All failure scenarios have been extracted to a dedicated document:

**→ `virtual_node_outgoing_failure_scenarios.md`**

That document catalogs 7 identified failure modes with flow diagrams, covering:
- Buffer accumulation during enumeration (Scenarios 1, 6)
- Reentrancy when loopback added to application send path (Scenarios 2, 3)
- Application send path bypasses loopback entirely (Scenarios 4, 5)
- One-level loopback guard suppresses multi-hop chains (Scenario 7)

The fundamental constraint: the outgoing transmit system has two independent
send paths (protocol handlers via outgoing slot, application helpers via direct
`send_openlcb_msg()` to the transport layer), neither of which provides correct
loopback for virtual nodes without unbounded buffer allocation or reentrancy
issues.

---

### Proposed Solution — OpenLCB-Layer Wrapper with Interleaved Loopback

**Core insight:** Both Path A and Path B already converge at a single DI
callback (`send_openlcb_msg`), wired in `openlcb_config.c`. Today all four
wiring points go directly to the transport layer
(`CanTxStatemachine_send_openlcb_message` for CAN). The fix: insert an
OpenLCB-layer wrapper between the callers and the transport.

#### 1. New Wrapper Function

**File:** `src/openlcb/openlcb_transmit.c` (new)

```
OpenLcbTransmit_send_with_loopback(openlcb_msg_t *msg)
    │
    ├─ 1. Call transport DI callback → message goes on wire
    │     (CAN, TCP/IP, whatever is wired)
    │
    └─ 2. Push loopback copy → dedicated loopback FIFO
          (allocate from pool, copy msg, set loopback flag)
```

The wrapper receives the real transport-send function pointer via DI:

```c
typedef struct {

    void (*transport_send)(openlcb_msg_t *msg);  // injected: actual transport
    // loopback FIFO pointer, pool allocator, etc.

} openlcb_transmit_interface_t;
```

This is an OpenLCB-layer concern, not a transport concern. The wrapper lives
in `src/openlcb/` and travels with the library regardless of the underlying
transport (CAN, TCP/IP, or future transports).

#### 2. DI Rewiring in `openlcb_config.c`

All four `send_openlcb_msg` assignments change from:

```c
_app_train.send_openlcb_msg = &CanTxStatemachine_send_openlcb_message;  // line 269
_login_sm.send_openlcb_msg  = &CanTxStatemachine_send_openlcb_message;  // line 318
_main_sm.send_openlcb_msg   = &CanTxStatemachine_send_openlcb_message;  // line 526
_app.send_openlcb_msg       = &CanTxStatemachine_send_openlcb_message;  // line 613
```

to:

```c
_app_train.send_openlcb_msg = &OpenLcbTransmit_send_with_loopback;
_login_sm.send_openlcb_msg  = &OpenLcbTransmit_send_with_loopback;
_main_sm.send_openlcb_msg   = &OpenLcbTransmit_send_with_loopback;
_app.send_openlcb_msg       = &OpenLcbTransmit_send_with_loopback;
```

The wrapper's own transport DI is wired once:

```c
_transmit.transport_send = &CanTxStatemachine_send_openlcb_message;
```

**Zero application code changes.** The `send_openlcb_msg` signature is
unchanged. Application helpers, protocol handlers, login — all call the same
DI pointer they always did. They don't know loopback exists.

**Transport-agnostic.** Swap to TCP/IP by changing one wiring line
(`_transmit.transport_send`). The loopback logic stays in `src/openlcb/` and
comes with the library.

**No propagation to user files.** Users call `OpenLcbApplication_send_*`
helpers which call `_interface->send_openlcb_msg()`. That DI pointer is
internal to the library. The rewiring is invisible to application code.

#### 3. Dedicated Loopback FIFO

Separate from the incoming FIFO. Why:

- Incoming FIFO holds messages from the wire (external nodes)
- Loopback FIFO holds messages from local siblings
- Different priority: loopback should be interleaved with, not queued behind,
  external messages
- Prevents loopback copies from starving external message processing

#### 4. Second `statemachine_info` Instance

The existing `_statemachine_info` holds live state during dispatch
(incoming_msg_info, outgoing_msg_info, current node, enumerate flag). Loopback
dispatch needs its own context so it doesn't corrupt the main dispatch.

Confirmed safe: all protocol handlers only have read-only `_interface` statics.
Two `statemachine_info` contexts dispatching to the same handlers will not
conflict.

#### 5. Interleaved Run Loop

Current:

```
1. handle_outgoing          → send pending msg
2. handle_try_reenumerate   → next enum step
3. handle_try_pop_incoming  → grab from wire FIFO
4. enumerate_first_node     → dispatch to first node
5. enumerate_next_node      → dispatch to next node
```

Proposed:

```
1.  handle_outgoing              → send pending msg (now goes through wrapper)
2a. loopback_pop                 → grab from loopback FIFO
2b. loopback_dispatch_first      → dispatch to first sibling
2c. loopback_dispatch_next       → dispatch to next sibling
2d. loopback_handle_outgoing     → send any response (through wrapper again)
2e. loopback_reenumerate         → if sibling enumerating, next step
3.  handle_try_reenumerate       → main enum step
4.  handle_try_pop_incoming      → grab from wire FIFO
5.  enumerate_first_node
6.  enumerate_next_node
```

Steps 2a-2e form a **mini run-loop** that processes ONE loopback message per
main `_run()` call. This bounds buffer accumulation:

- Main outgoing sends a message → wrapper pushes 1 loopback copy
- Next `_run()`: step 1 handles main outgoing, step 2a pops that 1 loopback
  copy, steps 2b-2e dispatch it to siblings
- If a sibling responds → its response goes through the wrapper → 1 more
  loopback copy
- Next `_run()`: process that copy
- **Net: 1 loopback buffer in flight at a time** (for Path A messages)

For Path B (application helpers calling `send_openlcb_msg()` directly), the
wrapper queues loopback copies to the FIFO. The callback may send N messages
before returning, accumulating N loopback copies. This is bounded by the
protocol (e.g., e-stop to 8 listeners = 8 copies). The loopback FIFO drains
these one per `_run()` call after the callback returns.

#### 6. How Each Failure Scenario Is Resolved

| # | Scenario | Resolution |
|---|----------|-----------|
| 1 | Enumerate + loopback buffer accumulation | Interleaved loop: step 2a-2e drain 1 loopback per `_run()` call, between enumerate steps. No accumulation. |
| 2 | App callback reentrancy | Wrapper queues to loopback FIFO (deferred). No inline dispatch. Buffer count = N messages sent by callback. Bounded by protocol. |
| 3 | Combined enum + callback | Both paths now go through wrapper. Both get loopback. Interleaved loop prevents accumulation from Path A; Path B accumulates N per callback (bounded). |
| 4 | App layer tight-loop bypasses loopback | Wrapper IS the send path. All application sends go through it. Loopback automatic. |
| 5 | Broadcast time rollover (LOW) | Same as #4 — wrapper adds loopback. Timer-tick pushes to FIFO (deferred), no reentrancy. |
| 6 | Train listener forwarding | Same as #1 — interleaved loop drains loopback between enumerate steps. |
| 7 | One-level loopback guard | Loopback FIFO + second `statemachine_info` enables proper chain tracking. Replace boolean flag with originator node_id or depth counter. |

#### 7. Implementation Cost

- **1 new file:** `src/openlcb/openlcb_transmit.c` + `.h` (wrapper + loopback
  FIFO management)
- **1 modified file:** `src/openlcb/openlcb_main_statemachine.c` (add steps
  2a-2e to run loop)
- **1 modified file:** `src/openlcb/openlcb_config.c` (rewire 4 DI pointers +
  wire transport DI)
- **1 new `statemachine_info`** instance (static in main statemachine or
  transmit module)
- **1 new FIFO** (loopback, same type as existing incoming FIFO)
- **Zero changes** to: application code, protocol handlers, login statemachine,
  transport drivers, user config files

---

## Related Documents

- `virtual_node_outgoing_failure_scenarios.md` — 7 failure scenario diagrams
- `OpenLcbCLib_OpenMRN_message_comparison.md` — full comparison analysis
- `plan_virtual_node_verification_protocol.md` — draft conformance testing spec

