/** @file openlcb_user_config.h
 *  @brief Project configuration for OpenLcbCLib -- dsPIC33E Basic Node
 */

#ifndef __OPENLCB_USER_CONFIG__
#define __OPENLCB_USER_CONFIG__

// =============================================================================
// Feature Flags
// =============================================================================

#define OPENLCB_COMPILE_EVENTS
#define OPENLCB_COMPILE_DATAGRAMS
#define OPENLCB_COMPILE_MEMORY_CONFIGURATION
#define OPENLCB_COMPILE_FIRMWARE

// =============================================================================
// Core Message Buffer Pool
// =============================================================================
// Tuned for dsPIC33EP512GP504 (52 KB RAM).
//
//   BASIC    (16 bytes each)  -- most OpenLCB messages fit in this size
//   DATAGRAM (72 bytes each)  -- datagram protocol messages
//   SNIP     (256 bytes each) -- SNIP replies and Events with Payload
//   STREAM   (512 bytes each) -- stream data transfer (future use)

#define USER_DEFINED_BASIC_BUFFER_DEPTH              16
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH           2
#define USER_DEFINED_SNIP_BUFFER_DEPTH               2
#define USER_DEFINED_STREAM_BUFFER_DEPTH             1

// =============================================================================
// Virtual Node Allocation
// =============================================================================

#define USER_DEFINED_NODE_BUFFER_DEPTH               2

// =============================================================================
// Events (requires OPENLCB_COMPILE_EVENTS)
// =============================================================================

#define USER_DEFINED_PRODUCER_COUNT                  4
#define USER_DEFINED_PRODUCER_RANGE_COUNT            1
#define USER_DEFINED_CONSUMER_COUNT                  4
#define USER_DEFINED_CONSUMER_RANGE_COUNT            1

// =============================================================================
// Memory Configuration (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION)
// =============================================================================

#define USER_DEFINED_CDI_LENGTH                      2000
#define USER_DEFINED_FDI_LENGTH                      100
#define USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS    0x00000000
#define USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS  62

// =============================================================================
// Train Protocol (requires OPENLCB_COMPILE_TRAIN -- not enabled)
// =============================================================================
// These must always be defined even when train features are not compiled.

#define USER_DEFINED_TRAIN_NODE_COUNT                1
#define USER_DEFINED_MAX_LISTENERS_PER_TRAIN         1
#define USER_DEFINED_MAX_TRAIN_FUNCTIONS             1

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
