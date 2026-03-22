/** @file openlcb_user_config.h
 *  @brief Bootloader test configuration — minimal firmware-upgrade-only build
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

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
#define USER_DEFINED_STREAM_BUFFER_LEN               256    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined

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
