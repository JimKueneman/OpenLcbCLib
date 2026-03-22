/** @file openlcb_user_config.h
 *  @brief User-editable project configuration for OpenLcbCLib
 *
 *  REQUIRED: Copy this file to your project's include path and edit.
 *    Arduino:     place next to your .ino file (sketch dir is on include path)
 *    PlatformIO:  place in src/ directory
 *    STM32 Cube:  place in Core/Inc/ directory
 *    CMake/Make:  place anywhere, add -I flag pointing to its directory
 *
 *  ALL values in this file are MANDATORY. The library will not compile if
 *  any are missing. Edit the values to match your project's requirements.
 *
 *  --- Quick Recipes for Feature Flags ---
 *
 *  Simple sensor/button node (events only):
 *    #define OPENLCB_COMPILE_EVENTS
 *
 *  Standard configurable node (events + config memory):
 *    #define OPENLCB_COMPILE_EVENTS
 *    #define OPENLCB_COMPILE_DATAGRAMS
 *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
 *    #define OPENLCB_COMPILE_FIRMWARE
 *
 *  Train command station:
 *    #define OPENLCB_COMPILE_EVENTS
 *    #define OPENLCB_COMPILE_DATAGRAMS
 *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
 *    #define OPENLCB_COMPILE_FIRMWARE
 *    #define OPENLCB_COMPILE_TRAIN
 *    #define OPENLCB_COMPILE_TRAIN_SEARCH
 *
 *  Full-featured node (everything):
 *    #define OPENLCB_COMPILE_EVENTS
 *    #define OPENLCB_COMPILE_DATAGRAMS
 *    #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
 *    #define OPENLCB_COMPILE_FIRMWARE
 *    #define OPENLCB_COMPILE_BROADCAST_TIME
 *    #define OPENLCB_COMPILE_TRAIN
 *    #define OPENLCB_COMPILE_TRAIN_SEARCH
 *
 *  Minimal bootloader (firmware upgrade only):
 *    Use templates/bootloader/openlcb_user_config.h instead
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Feature Flags -- uncomment to enable optional protocols
// =============================================================================

 #define OPENLCB_COMPILE_EVENTS
 #define OPENLCB_COMPILE_DATAGRAMS
 #define OPENLCB_COMPILE_MEMORY_CONFIGURATION
 #define OPENLCB_COMPILE_FIRMWARE
 #define OPENLCB_COMPILE_BROADCAST_TIME
 #define OPENLCB_COMPILE_TRAIN
 #define OPENLCB_COMPILE_TRAIN_SEARCH

// =============================================================================
// Debug -- uncomment to print feature summary during compilation
// =============================================================================

// #define OPENLCB_COMPILE_VERBOSE

// =============================================================================
// Core Message Buffer Pool
// =============================================================================
// The library uses a pool of message buffers of different sizes.  Tune these
// for your platform's available RAM.  The total number of buffers is the sum
// of all four types.  On 8-bit processors the total must not exceed 126.
//
//   BASIC    (16 bytes each)  -- most OpenLCB messages fit in this size
//   DATAGRAM (72 bytes each)  -- datagram protocol messages
//   SNIP     (256 bytes each) -- SNIP replies and Events with Payload
//   STREAM   (USER_DEFINED_STREAM_BUFFER_LEN bytes each) -- stream data transfer (future use)

#define USER_DEFINED_BASIC_BUFFER_DEPTH              128    // must be >= 1; enforced by compiler
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           4      // must be >= 1; enforced by compiler
#define USER_DEFINED_SNIP_BUFFER_DEPTH               4      // must be >= 1; enforced by compiler
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1      // must be >= 1; enforced by compiler
// Maximum bytes in a single stream data frame (future use).
#define USER_DEFINED_STREAM_BUFFER_LEN               256    // ignored and overridden to 1 if OPENLCB_COMPILE_STREAM is not defined

// =============================================================================
// Virtual Node Allocation
// =============================================================================
// How many virtual nodes this device can host.  Most simple devices use 1.
// Train command stations may need more (one per locomotive being controlled).

#define USER_DEFINED_NODE_BUFFER_DEPTH               4      // must be >= 1; enforced by compiler

// =============================================================================
// Events (requires OPENLCB_COMPILE_EVENTS)
// =============================================================================
// Maximum number of produced/consumed events per node, and how many event ID
// ranges each node can handle.  Ranges are used by protocols like Train Search
// that work with contiguous blocks of event IDs.
#define USER_DEFINED_PRODUCER_COUNT                  64     // must be >= 1; enforced by compiler
#define USER_DEFINED_PRODUCER_RANGE_COUNT            5      // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_COUNT                  64     // must be >= 1; enforced by compiler
#define USER_DEFINED_CONSUMER_RANGE_COUNT            5      // must be >= 1; enforced by compiler

// =============================================================================
// Configuration Memory (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)
// =============================================================================
//
// The two address values tell the SNIP protocol where in your node's
// configuration memory space the user-editable name and description strings
// begin.  The standard layout puts the user name at address 0 and the user
// description immediately after at byte 62:
//   63 = LEN_SNIP_USER_NAME_BUFFER (63)

// =============================================================================
// Train Protocol (requires OPENLCB_COMPILE_TRAIN)
// =============================================================================
// TRAIN_NODE_COUNT        -- max simultaneous train nodes (often equals
//                            NODE_BUFFER_DEPTH for a dedicated command station)
// MAX_LISTENERS_PER_TRAIN -- max consist members (listener slots) per train
// MAX_TRAIN_FUNCTIONS     -- number of DCC function outputs: 29 = F0 through F28

#define USER_DEFINED_TRAIN_NODE_COUNT                4      // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         6      // must be >= 1; enforced by compiler
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             29     // must be >= 1; enforced by compiler

// =============================================================================
// Listener Alias Verification (requires OPENLCB_COMPILE_TRAIN)
// =============================================================================
// LISTENER_PROBE_TICK_INTERVAL  -- how many 100ms ticks between prober calls
//                                  (1 = every 100ms, 2 = every 200ms, etc.)
// LISTENER_PROBE_INTERVAL_TICKS -- 100ms ticks between probes of the SAME entry
//                                  (250 = 25 seconds)
// LISTENER_VERIFY_TIMEOUT_TICKS -- 100ms ticks to wait for AMD reply before
//                                  declaring stale (30 = 3 seconds)

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30

// =============================================================================
// Application-defined node parameters (forward-declared to avoid circular include)
// =============================================================================

#ifdef __cplusplus
extern "C" {
#endif

extern const struct node_parameters_TAG OpenLcbUserConfig_node_parameters;

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_USER_CONFIG__ */
