/** @file openlcb_user_config.h
 *  @brief Bootloader test configuration — minimal firmware-upgrade-only build
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Transport Selection -- exactly one must be defined
// =============================================================================

 #define OPENLCB_COMPILE_CAN
// #define OPENLCB_COMPILE_TCP


// =============================================================================
// Bootloader preset
// =============================================================================

#define OPENLCB_COMPILE_BOOTLOADER     // auto-defines DATAGRAMS + MEMORY_CONFIGURATION + FIRMWARE

// =============================================================================
// Core Message Buffer Pool — minimized
// =============================================================================

#define USER_DEFINED_BASIC_BUFFER_DEPTH              8      // must be >= 1; enforced by compiler
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           2      // must be >= 1; enforced by compiler
#define USER_DEFINED_SNIP_BUFFER_DEPTH               1      // must be >= 1; enforced by compiler
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1      // must be >= 1; enforced by compiler

// =============================================================================
// Stream Transport (requires OPENLCB_COMPILE_STREAM)
// =============================================================================
// STREAM_BUFFER_LEN is the maximum bytes per stream data frame this node can
// accept.  The spec uses a 2-byte field so the protocol max is 65535.  During
// negotiation the smaller of the two nodes' buffer sizes wins.
//
// MAX_CONCURRENT_ACTIVE_STREAMS controls how many streams can be open at the
// same time across all nodes.  Each active stream uses a small state struct,
// not a full payload buffer.  The expensive RAM is governed by
// STREAM_BUFFER_DEPTH in the buffer pool above.
#define USER_DEFINED_STREAM_BUFFER_LEN               256    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined
#define USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS   1      // must be >= 1; enforced by compiler

// =============================================================================
// Virtual Node Allocation
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               1      // must be >= 1; enforced by compiler

// =============================================================================
// Minimums to avoid zero-length arrays
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  1      // must be >= 1; enforced by compiler
#define USER_DEFINED_PRODUCER_RANGE_COUNT            1      // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_COUNT                  1      // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_RANGE_COUNT            1      // must be >= 1; enforced by compiler
#define USER_DEFINED_TRAIN_NODE_COUNT                1      // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         1      // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             1      // must be >= 1; enforced by compiler

// =============================================================================
// Listener Alias Verification — must be defined
// =============================================================================

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30

// =============================================================================
// Application-defined node parameters
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

extern const struct node_parameters_TAG OpenLcbUserConfig_node_parameters;

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_USER_CONFIG__ */
