<!--
  ============================================================
  STATUS: IMPLEMENTED
  The plan required documentation-only updates (no code change). The CID handler in `can_rx_message_handler.c` correctly cites CanFrameTransferS §6.2.5 in its Doxygen block, confirming the implementation was already correct per spec.
  ============================================================
-->

# Plan: Item 10 -- CID Handler — Always RID (No Change Needed)

**Status:** COMPLETE — original implementation was already correct.

## 1. Summary

The CID frame handler in `can_rx_message_handler.c` (`CanRxMessageHandler_cid_frame()`)
always responds with RID regardless of whether the node is Inhibited or Permitted.

Per **CanFrameTransferS §6.2.5** (Node ID Alias Collision Handling):

> "If the frame is a Check ID (CID) frame, send a Reserve ID (RID) frame in response."

There is **no distinction** between Permitted and Inhibited nodes for CID handling.
The correct response is always RID. This differs from non-CID frames (RID, AMD, AMR)
which trigger full duplicate-alias handling (AMR + alias reallocation for Permitted
nodes) via the `_check_for_duplicate_alias()` helper.

## 2. Correction Notice

The original version of this plan incorrectly cited "CanFrameTransferS Section 3.5.3"
(which does not exist) and proposed differentiating Permitted vs Inhibited nodes for
CID — sending AMR for Permitted nodes. **That proposal was wrong.** Implementing it
would have introduced a spec violation.

The implementation was reviewed against the actual spec text (§6.2.5) and confirmed
correct as-is. No code changes were made. Only the doxygen comments in
`can_rx_message_handler.c` and `can_rx_message_handler.h` were updated to cite the
correct spec section (§6.2.5 instead of the non-existent §3.5.3).

## 3. Files Modified

| File | Change |
|------|--------|
| `src/drivers/canbus/can_rx_message_handler.c` | Doxygen: corrected spec reference from "Section 3.5.3" to "§6.2.5" |
| `src/drivers/canbus/can_rx_message_handler.h` | Doxygen: corrected spec reference from "3.5.3" to "§6.2.5" |

## 4. Spec Reference

**CanFrameTransferS §6.2.5 — Node ID Alias Collision Handling:**

A node shall compare the source Node ID alias in each received frame against all
reserved Node ID aliases it currently holds. In case of a match:

- **CID frame:** Send RID in response.
- **Non-CID frame, Permitted, current alias:** Transition to Inhibited, send AMR.
- **Non-CID frame, not Permitted:** Stop using the matching alias.
- **Non-CID frame, not current alias:** Stop using the matching alias.
