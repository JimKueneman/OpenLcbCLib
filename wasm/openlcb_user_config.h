/** @file openlcb_user_config.h
 *  @brief WASM build configuration — TCP/IP transport, all protocol features enabled.
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Transport Selection
// =============================================================================
// Using CAN transport with gridconnect text frames sent over a WebSocket/TCP
// socket on the JS side.  (Native binary-TCP support planned for the future.)

#define OPENLCB_COMPILE_CAN

// =============================================================================
// Feature Flags
// =============================================================================

#define OPENLCB_COMPILE_EVENTS
#define OPENLCB_COMPILE_DATAGRAMS
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION
#define OPENLCB_COMPILE_BROADCAST_TIME
#define OPENLCB_COMPILE_TRAIN
#define OPENLCB_COMPILE_TRAIN_SEARCH
#define OPENLCB_COMPILE_STREAM
#define OPENLCB_COMPILE_DCC_DETECTOR

// =============================================================================
// Core Message Buffer Pool
// =============================================================================

#define USER_DEFINED_BASIC_BUFFER_DEPTH              255
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           64
#define USER_DEFINED_SNIP_BUFFER_DEPTH               64
#define USER_DEFINED_STREAM_BUFFER_DEPTH             64

// =============================================================================
// Stream Transport
// =============================================================================

#define USER_DEFINED_STREAM_BUFFER_LEN               1024
#define USER_DEFINED_MAX_CONCURRENT_ACTIVE_STREAMS   64

// =============================================================================
// Virtual Node Allocation
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               255

// =============================================================================
// Events
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  256
#define USER_DEFINED_PRODUCER_RANGE_COUNT            32
#define USER_DEFINED_CONSUMER_COUNT                  256
#define USER_DEFINED_CONSUMER_RANGE_COUNT            32

// =============================================================================
// Train Protocol
// =============================================================================

#define USER_DEFINED_TRAIN_NODE_COUNT                255
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         16
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             29

// =============================================================================
// Listener Alias Verification
// =============================================================================

#define USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL    1
#define USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS   250
#define USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS   30

#endif /* __OPENLCB_USER_CONFIG__ */
