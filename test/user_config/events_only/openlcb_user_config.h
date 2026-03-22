/** @file openlcb_user_config.h
 *  @brief Events-only config -- only OPENLCB_COMPILE_EVENTS enabled
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Feature Flags
// =============================================================================

#define OPENLCB_COMPILE_EVENTS

// =============================================================================
// Core Message Buffer Pool — minimized
// =============================================================================

#define USER_DEFINED_BASIC_BUFFER_DEPTH              8
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           2
#define USER_DEFINED_SNIP_BUFFER_DEPTH               1
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1
#define USER_DEFINED_STREAM_BUFFER_LEN               256

// =============================================================================
// Virtual Node Allocation
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               1

// =============================================================================
// Events
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  4
#define USER_DEFINED_PRODUCER_RANGE_COUNT            1
#define USER_DEFINED_CONSUMER_COUNT                  4
#define USER_DEFINED_CONSUMER_RANGE_COUNT            1

// =============================================================================
// Minimums to avoid zero-length arrays
// =============================================================================

#define USER_DEFINED_TRAIN_NODE_COUNT                1
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         1
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             1

// =============================================================================
// Listener Alias Verification — must be defined
// =============================================================================

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30

#endif /* __OPENLCB_USER_CONFIG__ */
