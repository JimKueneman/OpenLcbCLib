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
#define LEN_CAN_FIFO_BUFFER USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1


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
     * @brief Creates a type this is an array of 8 bytes to carry the CAN frame data
     */
    typedef uint8_t payload_bytes_can_t[LEN_CAN_BYTE_ARRAY];

    /**
     * @typedef can_msg_state_t
     * @struct can_msg_state_struct
     * @brief Structure to hold information needed to track the state of a CAN message/frame
     */
    typedef struct can_msg_state_struct {
        uint8_t allocated : 1; /**< @brief Flag to define if the can message buffer is allocated or not. */

    } can_msg_state_t;

    /**
     * @typedef can_msg_t
     * @struct can_msg_struct
     * @brief Structure to hold information needed by a CAN message/frame
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
     */
    typedef can_msg_t can_msg_array_t[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

    typedef struct can_main_statemachine_struct {
        openlcb_statemachine_worker_t *openlcb_worker;
    } can_main_statemachine_t;

    /**
     * @typedef can_statemachine_info_t
     * @struct can_statemachine_info_struct
     * @brief Structure to hold information needed by the CAN Statemachine as it is
     * pulling messages from can_buffer_fifo.h and then dispatching them to handlers that
     * may require a reply
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
         * is successfull transmitted
         */
        can_msg_t *outgoing_can_msg;
        /**<
         * @brief Flag to tell the state machine that the current outgoing message is the first of N messages that this response
         * needs to transmit.
         * @note A good example of this is if a message to enumumerate all Consumers is recieved then N message will need to be sent
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
     * If during a message/frame reception it is found that some other node is using an Alias that is
     * associated with one of our nodes the \ref is_duplicate flag is set.  Likewise once the node
     * has become Permitted on the network the \ref is_permitted flag is set.  This allows the interrupt or
     * thread on receiving the frame can use this structure to make these checks/flags and the main loop can access this
     * structure (using the lock/unlock calls) to safely peek in and see if something needs to be handled in the main loops context.
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
         * @brief The CAN frame receiveing interrupt or thread has detected a duplicate Alias being used and sets this flag
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
     * If during a message/frame reception it is found that some other node is using an Alias that is
     * associated with one of our nodes the the \ref has_duplicate_alias flag is set.  This mean at least
     * one mapping in the array is a duplicate.
     */
    typedef struct alias_mapping_info_struct {
        alias_mapping_t list[ALIAS_MAPPING_BUFFER_DEPTH];
        bool has_duplicate_alias;

    } alias_mapping_info_t;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_COMMON_CAN_TYPES__ */
