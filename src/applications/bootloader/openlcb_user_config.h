/** @file openlcb_user_config.h
 *  @brief Bootloader project configuration for OpenLcbCLib
 *
 *  Minimal firmware-upgrade-only build.  Only DATAGRAMS + MEMORY_CONFIGURATION
 *  + FIRMWARE are enabled.  All buffer counts are set to the minimum safe
 *  values (1 where zero would create a zero-length array).
 *
 *  See documentation/working_plans/bootloader_config_plan.md for details.
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Bootloader preset — enables DATAGRAMS + MEMORY_CONFIGURATION + FIRMWARE,
// disables everything else.
// =============================================================================

#define OPENLCB_COMPILE_BOOTLOADER

#ifdef OPENLCB_COMPILE_BOOTLOADER

    #define OPENLCB_COMPILE_DATAGRAMS
    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
    #define OPENLCB_COMPILE_FIRMWARE

    #undef OPENLCB_COMPILE_EVENTS
    #undef OPENLCB_COMPILE_BROADCAST_TIME
    #undef OPENLCB_COMPILE_TRAIN
    #undef OPENLCB_COMPILE_TRAIN_SEARCH

#endif

// =============================================================================
// Core Message Buffer Pool — minimized for bootloader
// =============================================================================

#define USER_DEFINED_BASIC_BUFFER_DEPTH              8
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           2
#define USER_DEFINED_SNIP_BUFFER_DEPTH               1
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1
#define USER_DEFINED_STREAM_BUFFER_LEN               256    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined

// =============================================================================
// Virtual Node Allocation — bootloader is a single-node device
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               1

// =============================================================================
// Events — not used, minimum safe value to avoid zero-length arrays
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  1
#define USER_DEFINED_PRODUCER_RANGE_COUNT            1
#define USER_DEFINED_CONSUMER_COUNT                  1
#define USER_DEFINED_CONSUMER_RANGE_COUNT            1

// =============================================================================
// Configuration Memory — not used, minimum safe value
// =============================================================================

#define USER_DEFINED_CDI_LENGTH                      1
#define USER_DEFINED_FDI_LENGTH                      1

// =============================================================================
// Train Protocol — not used, minimum safe value
// =============================================================================

#define USER_DEFINED_TRAIN_NODE_COUNT                1
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         1
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             1

// =============================================================================
// Listener Alias Verification — not used, but must be defined
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
