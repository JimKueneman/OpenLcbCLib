/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
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
 *
 * @file openlcb_login_statemachine_handler.h
 * @brief Login state machine message handler for OpenLCB initialization and event broadcasting
 *
 * @details This module provides message construction handlers for the OpenLCB login sequence.
 * It handles Initialization Complete message generation, Producer Event Identified messages,
 * and Consumer Event Identified messages with appropriate state indicators.
 *
 * The handlers work in conjunction with openlcb_login_statemachine to orchestrate the
 * complete login sequence per OpenLCB Message Network Standard.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 *
 * @see openlcb_login_statemachine.h - Main state machine dispatcher
 * @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
 * @see OpenLCB Event Transport Standard S-9.7.4 - Event Transport Protocol
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__
#define    __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

/**
 * @brief Interface structure for login message handler callback functions
 *
 * @details This structure defines the function pointers that must be provided
 * during initialization to extract event state information from nodes.
 *
 * Required callbacks allow the handler to query application-specific event states
 * and generate appropriate Producer/Consumer Identified messages with Valid,
 * Invalid, or Unknown states.
 */
typedef struct {

        /**
         * @brief Callback to extract Producer Event Identified MTI for a given event
         *
         * @details Returns the appropriate MTI based on the current state of the producer event.
         * The returned MTI indicates whether the event is Valid, Invalid, or Unknown.
         *
         * @param openlcb_node Pointer to node containing the producer event
         * @param event_index Index into the node's producers.list array
         * @return MTI value for Producer Identified message (0x0914, 0x0915, or 0x0917)
         *
         * @warning openlcb_node must NOT be NULL
         * @warning event_index must be less than openlcb_node->producers.count
         *
         * @see MTI_PRODUCER_IDENTIFIED_VALID in openlcb_defines.h
         */
        uint16_t(*extract_producer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

        /**
         * @brief Callback to extract Consumer Event Identified MTI for a given event
         *
         * @details Returns the appropriate MTI based on the current state of the consumer event.
         * The returned MTI indicates whether the event is Valid, Invalid, or Unknown.
         *
         * @param openlcb_node Pointer to node containing the consumer event
         * @param event_index Index into the node's consumers.list array
         * @return MTI value for Consumer Identified message (0x04C4, 0x04C5, or 0x04C7)
         *
         * @warning openlcb_node must NOT be NULL
         * @warning event_index must be less than openlcb_node->consumers.count
         *
         * @see MTI_CONSUMER_IDENTIFIED_VALID in openlcb_defines.h
         */
        uint16_t(*extract_consumer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

    } interface_openlcb_login_message_handler_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the login message handler with callback interface
     *
     * @details Stores the interface function pointers for event state extraction.
     * This allows the handler to query application-specific event states when
     * generating Producer and Consumer Identified messages.
     *
     * Use cases:
     * - Called once during application startup
     * - Must be called before openlcb_login_statemachine begins processing nodes
     *
     * @param interface Pointer to interface structure containing callback functions
     *
     * @warning MUST be called exactly once during initialization
     * @warning The interface pointer is stored in static memory - the pointed-to
     *          structure must remain valid for the lifetime of the program
     * @warning NOT thread-safe
     *
     * @attention Call during single-threaded initialization phase
     * @attention Both callback functions in the interface must be valid (non-NULL)
     *
     * @see interface_openlcb_login_message_handler_t - Interface structure definition
     * @see OpenLcbLoginMessageHandler_load_producer_event - Uses extract_producer callback
     * @see OpenLcbLoginMessageHandler_load_consumer_event - Uses extract_consumer callback
     */
    extern void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface);

    /**
     * @brief Loads an Initialization Complete message into the outgoing message buffer
     *
     * @details Constructs an Initialization Complete message per OpenLCB Message Network Standard.
     * The message contains the node's 48-bit Node ID and uses either Simple or Full protocol MTI
     * based on the node's configuration. After loading the message, marks the node as initialized
     * and transitions state to begin producer event enumeration.
     *
     * Use cases:
     * - Called during node login sequence after alias reservation
     * - Announces node's presence on the network
     *
     * @param statemachine_info Pointer to state machine info containing node and message buffer
     *
     * @warning statemachine_info must NOT be NULL
     * @warning statemachine_info->openlcb_node must be valid
     * @warning statemachine_info->outgoing_msg_info.msg_ptr must be allocated
     *
     * @attention This message must be sent before any event messages
     * @attention The node's alias must already be allocated and validated
     *
     * @note Sets outgoing_msg_info.valid to true to trigger transmission
     * @note Protocol type (Simple vs Full) determined by node's PSI_SIMPLE flag
     *
     * @see OpenLCB Message Network Standard S-9.7.3.1 - Initialization Complete
     * @see MTI_INITIALIZATION_COMPLETE in openlcb_defines.h
     * @see OpenLcbLoginMessageHandler_load_producer_event - Next step in sequence
     */
    extern void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_login_statemachine_info_t *statemachine_info);

    /**
     * @brief Loads a Producer Event Identified message for the current producer event
     *
     * @details Constructs a Producer Identified message for one event in the node's
     * producer list. The message indicates whether the producer event is currently
     * valid, invalid, or in an unknown state. May be called multiple times via
     * re-enumeration to send messages for each produced event.
     *
     * Use cases:
     * - Called repeatedly during login to announce each produced event
     * - May be called later in response to Identify Producer messages
     *
     * @param statemachine_info Pointer to state machine info containing node and message buffer
     *
     * @warning statemachine_info must NOT be NULL
     * @warning statemachine_info->openlcb_node must be valid
     * @warning enum_index must be less than producers.count when enumerate flag is set
     *
     * @attention This function may be called multiple times (once per producer event)
     * @attention The enumerate flag controls re-entry until all events are sent
     *
     * @note If producers.count is 0, no messages are generated
     * @note Sets outgoing_msg_info.valid to false if no producers exist
     * @note Sets outgoing_msg_info.enumerate to true to process next event
     *
     * @see OpenLCB Event Transport Standard S-9.7.4.1 - Producer Identified
     * @see MTI_PRODUCER_IDENTIFIED_VALID in openlcb_defines.h
     * @see OpenLcbLoginMessageHandler_load_consumer_event - Next step after all producers
     */
    extern void OpenLcbLoginMessageHandler_load_producer_event(openlcb_login_statemachine_info_t *statemachine_info);

    /**
     * @brief Loads a Consumer Event Identified message for the current consumer event
     *
     * @details Constructs a Consumer Identified message for one event in the node's
     * consumer list. The message indicates whether the consumer event is currently
     * valid, invalid, or in an unknown state. May be called multiple times via
     * re-enumeration to send messages for each consumed event. This is the final
     * step of the login sequence.
     *
     * Use cases:
     * - Called repeatedly during login to announce each consumed event
     * - May be called later in response to Identify Consumer messages
     *
     * @param statemachine_info Pointer to state machine info containing node and message buffer
     *
     * @warning statemachine_info must NOT be NULL
     * @warning statemachine_info->openlcb_node must be valid
     * @warning enum_index must be less than consumers.count when enumerate flag is set
     *
     * @attention This function may be called multiple times (once per consumer event)
     * @attention The enumerate flag controls re-entry until all events are sent
     * @attention This is the final step of the login sequence
     *
     * @note If consumers.count is 0, no messages are generated
     * @note Sets outgoing_msg_info.valid to false if no consumers exist
     * @note Sets outgoing_msg_info.enumerate to true to process next event
     * @note Transitions to RUNSTATE_RUN when complete (node fully initialized)
     *
     * @see OpenLCB Event Transport Standard S-9.7.4.2 - Consumer Identified
     * @see MTI_CONSUMER_IDENTIFIED_VALID in openlcb_defines.h
     * @see RUNSTATE_RUN in openlcb_defines.h - Final operational state
     */
    extern void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__ */
