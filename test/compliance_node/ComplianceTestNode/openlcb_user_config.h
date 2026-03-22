/** @file openlcb_user_config.h
 *  @brief Project configuration for ComplianceTestNode
 *
 *  All feature flags are enabled at compile time. Runtime CLI args control
 *  which protocols are active for a given test run.
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Feature Flags — all enabled for compliance testing
// =============================================================================

#define OPENLCB_COMPILE_EVENTS
#define OPENLCB_COMPILE_DATAGRAMS
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION
#define OPENLCB_COMPILE_FIRMWARE
#define OPENLCB_COMPILE_BROADCAST_TIME
#define OPENLCB_COMPILE_TRAIN
#define OPENLCB_COMPILE_TRAIN_SEARCH

// =============================================================================
// Core Message Buffer Pool
// =============================================================================

#define USER_DEFINED_BASIC_BUFFER_DEPTH              117 // must be >= 1; enforced by compiler
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           4   // must be >= 1; enforced by compiler
#define USER_DEFINED_SNIP_BUFFER_DEPTH               4   // must be >= 1; enforced by compiler
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1   // must be >= 1; enforced by compiler
// Maximum bytes in a single stream data frame (future use).
#define USER_DEFINED_STREAM_BUFFER_LEN               256    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined

// =============================================================================
// Virtual Node Allocation
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               4   // must be >= 1; enforced by compiler

// =============================================================================
// Events (requires OPENLCB_COMPILE_EVENTS)
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  64  // must be >= 1; enforced by compiler
#define USER_DEFINED_PRODUCER_RANGE_COUNT            5   // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_COUNT                  32  // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_RANGE_COUNT            5   // must be >= 1; enforced by compiler

// =============================================================================
// Memory Configuration (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)
// =============================================================================

// =============================================================================
// Train Protocol (requires OPENLCB_COMPILE_TRAIN)
// =============================================================================

#define USER_DEFINED_TRAIN_NODE_COUNT                4   // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         6   // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             29  // must be >= 1; enforced by compiler

// =============================================================================
// Listener Alias Verification (requires OPENLCB_COMPILE_TRAIN)
// =============================================================================

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30

// =============================================================================
// Node parameters for each protocol mode (defined in openlcb_user_config.c)
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

extern const struct node_parameters_TAG compliance_basic_params;
extern const struct node_parameters_TAG compliance_broadcast_time_consumer_params;
extern const struct node_parameters_TAG compliance_broadcast_time_producer_params;
extern const struct node_parameters_TAG compliance_train_params;

// Required by the library — points to basic params by default
extern const struct node_parameters_TAG OpenLcbUserConfig_node_parameters;

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_USER_CONFIG__ */
