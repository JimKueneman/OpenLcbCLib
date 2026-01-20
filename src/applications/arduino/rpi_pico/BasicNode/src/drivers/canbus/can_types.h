/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * @file can_types.h
 * @brief Type definitions and constants for CAN operations
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_COMMON_CAN_TYPES__
#define __DRIVERS_COMMON_CAN_TYPES__

#include <stdbool.h>
#include <stdint.h>

#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // ************************ USER DEFINED VARIABLES *****************************

    /**
     * @brief The number of CAN messages buffers that are available to allocate through the can_buffer_store.h
     * The default value for the buffer depth is 10 (typically more than enough) but can be overridden by using a global
     * define for the compiler such as USER_DEFINED_CAN_MSG_BUFFER_DEPTH=20
     * @warning the maximum size for the buffer depth is 254 (0xFE)
     */
#ifndef USER_DEFINED_CAN_MSG_BUFFER_DEPTH  // USER DEFINED MAX VALUE = 0xFE = 254
#define USER_DEFINED_CAN_MSG_BUFFER_DEPTH 10
#endif

    // *********************END USER DEFINED VARIABLES *****************************


    /**
     * @brief The number of Alias Mapping slots available for alias_mappings.h
     * The default value for the buffer is equal to the value used for /ref USER_DEFINED_NODE_BUFFER_DEPTH
     * as there must be the same number mapping slot as there are allocated nodes.
     */
#ifndef ALIAS_MAPPING_BUFFER_DEPTH
#define ALIAS_MAPPING_BUFFER_DEPTH USER_DEFINED_NODE_BUFFER_DEPTH
#endif

    /**
     * @brief The number of Pointer slots in the FIFO to hold CAN Buffers from can_buffer_store.h
     * The default value is size of the buffer depth plus one \ref USER_DEFINED_CAN_MSG_BUFFER_DEPTH
     * so all buffers can be held in the FIFO implementation.  The buffer can then be checked for full
     * without the head == tail.
     */
#define LEN_CAN_FIFO_BUFFER (USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1)


    /**
     * @brief The number of data bytes in a CAN frame
     */
#define LEN_CAN_BYTE_ARRAY 8

    /**
     * @brief Some CAN frames depending, of what OpenLcb/LCC message is being worked on, will
     * have an destination Alias as the first 2 bytes.  This is a constant passed to many
     * functions to allow the function to know where the actual data for the message starts.
     * This means there is no Alias in the data and the first position in the array is data.
     */
#define OFFSET_CAN_WITHOUT_DEST_ADDRESS 0

    /**
     * @brief Some CAN frames depending, of what OpenLcb/LCC message is being worked on, will
     * have an destination Alias as the first 2 bytes.  This is a constant passed to many
     * functions to allow the function to know where the actual data for the message starts.
     * This means there is an Alias in the data and the 2 second position in the array is data.
     */
#define OFFSET_CAN_WITH_DEST_ADDRESS 2

    /**
     * @brief Creates a value that can be used to build the bits in the higher positions of the CAN MTI
     */
#define _OPENLCB_GLOBAL_ADDRESSED (RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED)

    /**
     * @brief Creates a value that can be used to build the bits in the higher positions of the CAN MTI for Datagram reject
     */
#define _DATAGRAM_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_DATAGRAM_REJECTED_REPLY & 0x0FFF) << 12))

    /**
     * @brief Creates a value that can be used to build the bits in the higher positions of the CAN MTI for Datagram reject reply
     */
#define _OPTIONAL_INTERACTION_REJECT_REPLY (_OPENLCB_GLOBAL_ADDRESSED | ((uint32_t) (MTI_OPTIONAL_INTERACTION_REJECTED & 0x0FFF) << 12))


    // Structure for a basic CAN payload

    /**
     * @typedef payload_bytes_can_t
     * @brief Creates a type that is an array of 8 bytes to carry the CAN frame data
     *
     * @details Defines the standard CAN payload buffer size according to the CAN 2.0
     * specification which allows up to 8 data bytes per frame. This type is used
     * throughout the CAN layer to ensure consistent payload sizing.
     *
     * Use cases:
     * - Allocating CAN frame payload buffers
     * - Passing CAN data between functions
     * - Ensuring compile-time type safety for CAN payloads
     *
     * @see LEN_CAN_BYTE_ARRAY - Defines the array size
     * @see can_msg_t - Uses this type for payload storage
     */
    typedef uint8_t payload_bytes_can_t[LEN_CAN_BYTE_ARRAY];

    /**
     * @typedef can_msg_state_t
     * @struct can_msg_state_struct
     * @brief Structure to hold information needed to track the state of a CAN message/frame
     *
     * @details This structure uses bit fields to efficiently track the allocation status
     * of CAN message buffers. The allocated flag indicates whether a buffer is currently
     * in use by the system.
     *
     * Use cases:
     * - Tracking buffer allocation in the CAN buffer store
     * - Determining if a CAN message buffer is available for reuse
     * - Managing buffer lifecycle during message processing
     *
     * @note Uses bit fields to minimize memory footprint
     *
     * @see can_msg_t - Contains this state structure
     * @see CanBufferStore_allocate_buffer - Sets the allocated flag
     * @see CanBufferStore_free_buffer - Clears the allocated flag
     */
    typedef struct can_msg_state_struct {
        uint8_t allocated : 1; /**< @brief Flag to define if the can message buffer is allocated or not. */

    } can_msg_state_t;

    /**
     * @typedef can_msg_t
     * @struct can_msg_struct
     * @brief Structure to hold information needed by a CAN message/frame
     *
     * @details This structure represents a complete CAN 2.0B extended frame with
     * 29-bit identifier and up to 8 data bytes. It includes state tracking for
     * buffer management and is the fundamental unit for CAN communication throughout
     * the library.
     *
     * Use cases:
     * - Storing received CAN frames from hardware
     * - Building outgoing CAN frames for transmission
     * - Queuing frames in the CAN buffer FIFO
     * - Converting between OpenLCB messages and CAN frames
     *
     * @warning Maximum payload is 8 bytes per CAN 2.0 specification
     * @warning NOT thread-safe - caller must handle synchronization
     *
     * @note The identifier field holds the full 29-bit extended CAN ID
     * @note payload_count indicates valid bytes (0-8)
     *
     * @see can_buffer_store.h - Manages allocation of these structures
     * @see can_buffer_fifo.h - Queues pointers to these structures
     */
    typedef struct can_msg_struct {
        can_msg_state_t state; /**< @brief Flags for the current state of this buffer. */
        uint32_t identifier; /**< @brief CAN 29 bit extended identifier for this buffer. */
        uint8_t payload_count; /**< @brief Defines how many bytes in the payload are valid. */
        payload_bytes_can_t payload; /**< @brief Data bytes of the message/frame. */

    } can_msg_t;

    /**
     * @typedef can_msg_array_t
     * @brief Structure to hold the CAN message/frame buffers in an array up to \ref USER_DEFINED_CAN_MSG_BUFFER_DEPTH in count
     *
     * @details Defines a fixed-size array of CAN message buffers that forms the
     * pre-allocated memory pool for CAN frame storage. The size is configurable
     * at compile time but defaults to 10 buffers.
     *
     * Use cases:
     * - Pre-allocating CAN message buffer pool at initialization
     * - Providing static memory allocation for embedded systems
     * - Avoiding dynamic memory allocation during runtime
     *
     * @note Array size determined by USER_DEFINED_CAN_MSG_BUFFER_DEPTH
     * @note Maximum array size is 254 buffers (0xFE)
     *
     * @see USER_DEFINED_CAN_MSG_BUFFER_DEPTH - Defines array size
     * @see can_buffer_store.c - Manages this array
     */
    typedef can_msg_t can_msg_array_t[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

    /**
     * @typedef can_main_statemachine_t
     * @struct can_main_statemachine_struct
     * @brief Structure to hold state information for the CAN main state machine
     *
     * @details This structure maintains the working context for the CAN layer's
     * main state machine, providing access to the OpenLCB worker thread that
     * handles message processing and node management.
     *
     * Use cases:
     * - Managing CAN state machine execution context
     * - Providing access to OpenLCB layer worker thread
     * - Coordinating between CAN and OpenLCB protocol layers
     */
    typedef struct can_main_statemachine_struct {
        /**<
         * @brief Pointer to the OpenLCB state machine worker thread context
         */
        openlcb_statemachine_worker_t *openlcb_worker;
    } can_main_statemachine_t;

    /**
     * @typedef can_statemachine_info_t
     * @struct can_statemachine_info_struct
     * @brief Structure to hold information needed by the CAN Statemachine as it is
     * pulling messages from can_buffer_fifo.h and then dispatching them to handlers that
     * may require a reply
     *
     * @details This structure serves as the working context for the CAN main state machine,
     * maintaining pointers to the current node being processed and managing outgoing message
     * buffers. It supports both stack-allocated login messages and heap-allocated general
     * messages, with flags to control enumeration behavior for multi-message responses.
     *
     * The state machine uses this structure to:
     * - Track which OpenLCB node is currently being processed
     * - Hold outgoing CAN messages waiting for transmission
     * - Coordinate between login sequence and normal message handling
     * - Support multi-message response sequences through enumeration flag
     *
     * Use cases:
     * - Processing incoming CAN frames and generating responses
     * - Managing login sequence message transmission
     * - Handling multi-message responses (e.g., event enumeration)
     * - Coordinating between CAN layer and OpenLCB layer state machines
     *
     * @warning Stack-allocated login buffer must not be freed
     * @warning Heap-allocated outgoing buffer must be freed after transmission
     * @warning NOT thread-safe - caller must handle synchronization
     *
     * @attention The enumerating flag prevents premature message cleanup
     * @attention Login messages use stack allocation for efficiency
     *
     * @note login_outgoing_can_msg is always available (stack-allocated)
     * @note outgoing_can_msg is NULL when no message pending
     *
     * @see can_main_statemachine.h - Uses this structure
     * @see can_login_statemachine.h - Loads login_outgoing_can_msg
     * @see can_buffer_store.h - Allocates outgoing_can_msg buffers
     */
    typedef struct can_statemachine_info_struct {
        /**<
         * @brief Pointer to the OpenLcb/LCC node that is being operated on
         */
        openlcb_node_t *openlcb_node;
        /**<
         * @brief Pointer to the CAN message that the Login Statemachine has loaded and needs to be transmitted
         * @note This buffer is allocated on the stack from the CAN state machine and is always available.  The
         * /ref login_outgoing_can_msg_valid flag marks if the current value of this struture needs to be transmitted
         */
        can_msg_t *login_outgoing_can_msg;
        /**<
         * @brief Flag to mark that \ref login_outgoing_can_msg is a valid message that needs transmitting
         */
        uint8_t login_outgoing_can_msg_valid : 1;
        /**<
         * @brief Pointer to the CAN message that needs to be sent
         * @note This buffer is a buffer allocated from can_buffer_store.h and will be freed and set to NULL after it
         * is successfully transmitted
         */
        can_msg_t *outgoing_can_msg;
        /**<
         * @brief Flag to tell the state machine that the current outgoing message is the first of N messages that this response
         * needs to transmit.
         * @note A good example of this is if a message to enumerate all Consumers is received then N message will need to be sent
         * as a response.  If the handler sets this flag then the state machine knows that is should not free the current message is its
         * handling and should continue to call the hander for that incoming message until this flag is clear.
         */
        uint8_t enumerating : 1;

    } can_statemachine_info_t;

    /**
     * @typedef alias_mapping_t
     * @struct alias_mapping_struct
     * @brief Structure to hold a shadow buffer of Node ID/Alias pairs for Nodes that have been
     * allocated and logged into the network.
     *
     * @details This structure maintains the mapping between a node's permanent 48-bit Node ID
     * and its temporary 12-bit CAN alias. The mapping is critical for CAN bus communication
     * where the compact alias is used in frame headers instead of the full Node ID.
     *
     * The structure tracks two important states:
     * - Duplicate detection: Set when another node claims the same alias
     * - Permission status: Set when node successfully completes login sequence
     *
     * This design allows interrupt/thread contexts to safely set flags while the main loop
     * handles the actual response processing using lock/unlock mechanisms.
     *
     * Use cases:
     * - Tracking alias allocation during node login
     * - Detecting alias conflicts on the CAN bus
     * - Translating between Node IDs and aliases during message processing
     * - Managing node permission state transitions
     *
     * @warning Duplicate detection requires immediate handling to prevent bus conflicts
     * @warning NOT thread-safe - use lock/unlock when accessing from multiple contexts
     *
     * @attention Flags are set in interrupt context, processed in main loop
     * @attention Valid aliases range from 0x001 to 0xFFF (0x000 is invalid)
     *
     * @note Node ID is the permanent 48-bit identifier
     * @note Alias is temporary and may change between power cycles
     * @note is_duplicate flag triggers alias conflict resolution
     * @note is_permitted indicates successful network login
     *
     * @see alias_mappings.h - Manages array of these structures
     * @see can_login_statemachine.h - Sets is_permitted flag
     * @see AliasMappings_register - Registers new mappings
     */
    typedef struct alias_mapping_struct {
        /**<
         * @brief Node ID of the mapping pair
         */
        node_id_t node_id;
        /**<
         * @brief Alias ID of the mapping pair
         */
        uint16_t alias;
        /**<
         * @brief The CAN frame receiving interrupt or thread has detected a duplicate Alias being used and sets this flag
         * so the main loop can handle it
         */
        uint8_t is_duplicate : 1;
         /**<
         * @brief Main loop Login has successfully logged the Node ID/Alias ID pair into the network
         */
        uint8_t is_permitted : 1;

    } alias_mapping_t;


    /**
     * @typedef alias_mapping_info_t
     * @struct alias_mapping_info_struct
     * @brief Structure to hold an array of \ref alias_mapping_t structures and a flag to flag the main loop
     * that at least one of the containing mappings has detected a duplicate Alias ID
     *
     * @details This structure serves as the master container for all Node ID/Alias mappings
     * in the system. It maintains an array sized to match the maximum number of nodes and
     * includes a global duplicate detection flag for efficient conflict checking.
     *
     * The has_duplicate_alias flag provides a fast check mechanism: when set, at least one
     * entry in the list has detected an alias conflict, allowing the main loop to efficiently
     * scan for and handle duplicates without checking every entry on every iteration.
     *
     * Use cases:
     * - Managing all active Node ID/Alias mappings for the system
     * - Providing fast duplicate alias detection across all nodes
     * - Supporting efficient main loop processing of alias conflicts
     * - Coordinating between interrupt context (flag setting) and main loop (handling)
     *
     * @warning Array size must match USER_DEFINED_NODE_BUFFER_DEPTH
     * @warning NOT thread-safe - use lock/unlock when accessing
     *
     * @attention has_duplicate_alias is a global flag for ANY duplicate in the array
     * @attention Must scan array when has_duplicate_alias is true to find specific entry
     *
     * @note Array size defined by ALIAS_MAPPING_BUFFER_DEPTH
     * @note has_duplicate_alias does not identify WHICH mapping has duplicate
     * @note Cleared only after all duplicates are resolved
     *
     * @see alias_mappings.h - Manages this structure
     * @see ALIAS_MAPPING_BUFFER_DEPTH - Defines array size
     * @see AliasMappings_set_has_duplicate_alias_flag - Sets the global flag
     */
    typedef struct alias_mapping_info_struct {
        alias_mapping_t list[ALIAS_MAPPING_BUFFER_DEPTH];
        bool has_duplicate_alias;

    } alias_mapping_info_t;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_COMMON_CAN_TYPES__ */
