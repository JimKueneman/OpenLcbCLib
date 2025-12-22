/*
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
 * 14 Dec 2025
 * Copyright (c) 2025, Jim Kueneman
 */

/**
 *
 * @brief Implements handlers for use by the CAN Receive (Rx) statemachine \ref can_rx_statemachine.h.
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN receive (Rx)
 * state machine to accomplish that.
 *
 * @note Applications typically only need to access the Initialize function in this module.
 * 
 * @note Any handler may be overridden by assigning a custom function pointer to the
 * \ref interface_can_rx_message_handler_t field during initialization of the application.
 * see: \ref CanRxMessageHandler_initialize().
 *
 * @file can_rx_message_handler.h
 *
 */


// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__
#define __DRIVERS_CANBUS_CAN_RX_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_node.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief A structure to hold pointers to functions for dependencies this module requires, \ref can_rx_message_handler.h.
     *
     * @details OpenLcbCLib uses dependency injection to allow for writing full coverage tests as the
     * functions that are used can be modeled in the test and return valid OR invalid results to fully
     * test all program flows in the module.  It also allows for reducing the program size. If a particular
     * protocol does not need to be implemented simply filling in the dependency for that handler with a NULL
     * will strip out code for that protocols handlers and minimize the application size (bootloader is an example).
     * The library will automatically reply with the correct error/reply codes if the handler is defined as NULL.
     */
    typedef struct {
        /*@{*/

        // REQUIRED FUNCTIONS

        /** @brief Pointer to a function for access into the \ref openlcb_buffer_store.h functions to allocate a new CAN message buffer.
         * @warning <b>Required</b> assignment.  Defaults to \ref CanBufferStore_allocate_buffer(). */
        can_msg_t *(*can_buffer_store_allocate_buffer)(void);

        /** @brief Pointer to a function for access into the \ref openlcb_buffer_store.h functions to allocate a new Openlcb message buffer.
         * @warning <b>Required</b> assignment.  Defaults to \ref OpenLcbBufferStore_allocate_buffer(). */
        openlcb_msg_t *(*openlcb_buffer_store_allocate_buffer)(payload_type_enum payload_type);

        /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to access the mapping pairs.
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_find_mapping_by_alias(). */
        alias_mapping_t *(*alias_mapping_find_mapping_by_alias)(uint16_t alias);

        /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to access the mapping pairs.
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_find_mapping_by_node_id(). */
        alias_mapping_t *(*alias_mapping_find_mapping_by_node_id)(node_id_t node_id);

        /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to access the mapping pairs.
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_get_alias_mapping_info(). */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /** @brief Pointer to a function for access into the \ref alias_mappings.h functions to set the flag to signal at least one of the pairs was found to be a duplicate Alias.
         * @warning <b>Required</b> assignment.  Defaults to \ref AliasMappings_set_has_duplicate_alias_flag(). */
        void (*alias_mapping_set_has_duplicate_alias_flag)(void);

        // OPTIONAL FUNCTION

        // CALLBACK FUNCTIONS

        /*@}*/

    } interface_can_rx_message_handler_t;


    /**
     * @brief Initializes the CAN Receive (Rx) Message Handlers
     *
     * @param const interface_can_rx_message_handler_t *interface_can_frame_message_handler - Pointer to a
     * interface_can_rx_message_handler_t struct containing the functions that this module requires.
     *
     * @return none
     *
     * @attention This must always be called during application initialization.
     */
    extern void CanRxMessageHandler_initialize(const interface_can_rx_message_handler_t *interface_can_frame_message_handler);


    /**
     * @brief Handles a first frame of an OpenLcb/LCC message that is sent multiple CAN frames.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     * @param payload_type_enum data_type - Defines what data type will be required for this OpenLcb/LCC message.  
     * Required to allow the handler to allocate an OpenLcb/LCC buffer to start assembling the message on subsequent received frames.
     *
     * @return none
     */
    extern void CanRxMessageHandler_first_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a middle frame of an OpenLcb/LCC message that is sent multiple CAN frames.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     *
     * @return none
     */
    extern void CanRxMessageHandler_middle_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles the last frame of an OpenLcb/LCC message that is sent multiple CAN frames.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     *
     * @return none
     */
    extern void CanRxMessageHandler_last_frame(can_msg_t *can_msg, uint8_t offset);


    /**
     * @brief Handles a frame of an OpenLcb/LCC message that is a complete message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     * @param payload_type_enum data_type - Defines what data type will be required for this OpenLcb/LCC message.  
     * Required to allow the handler to allocate an OpenLcb/LCC buffer to start assembling the message on subsequent received frames.
     *
     * @return none
     */
    extern void CanRxMessageHandler_single_frame(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a frame of a multiple CAN frame OpenLcb/LCC SNIP message that is being sent in the original
     * Protocol format.
     * 
     * The original SNIP format used the number of NULLs in the string to find the end of the message, later the 
     * protocol was updated when a mechanism for encoding a bits in the upper nibble of the 2 Byte target Alias passed
     * in the first 2 bytes of the CAN data.  This encoding allowed this message to be handled using the first/middle/last 
     * handlers above.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     * @param payload_type_enum data_type - Defines what data type will be required for this OpenLcb/LCC message.  
     * Required to allow the handler to allocate an OpenLcb/LCC buffer to start assembling the message on subsequent received frames.
     *
     * @return none
     */
    extern void CanRxMessageHandler_can_legacy_snip(can_msg_t *can_msg, uint8_t offset, payload_type_enum data_type);


    /**
     * @brief Handles a stream frame of an OpenLcb/LCC message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     * @param uint8_t offset - Offset in the Data array where the actual data begins.  Some 
     * messages place a target Alias in the first 2 bytes and some do not.
     * @param payload_type_enum data_type - Defines what data type will be required for this OpenLcb/LCC message.  
     * Required to allow the handler to allocate an OpenLcb/LCC buffer to start assembling the message on subsequent received frames.
     * @return none
     */
    extern void CanRxMessageHandler_stream_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type);


    /**
     * @brief Handles the reply if the received message is a RID CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received
     *
     * @return none
     */
    extern void CanRxMessageHandler_rid_frame(can_msg_t *can_msg);


    /**
     * @brief Handles the reply if the received message is an AMD CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     *
     * @return none
     */
    extern void CanRxMessageHandler_amd_frame(can_msg_t *can_msg);


    /**
     * @brief Handles the reply if the received message is an AME CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     *
     * @return none
     */
    extern void CanRxMessageHandler_ame_frame(can_msg_t *can_msg);


    /**
     * @brief Handles the reply if the received message is an AMR CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     *
     * @return none
     */
    extern void CanRxMessageHandler_amr_frame(can_msg_t *can_msg);


    /**
     * @brief Handles the reply if the received message is an Error Info Report CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     *
     * @return none
     */
    extern void CanRxMessageHandler_error_info_report_frame(can_msg_t *can_msg);


    /**
     * @brief Handles the reply if the received message is a CID CAN message.
     *
     * @param can_msg_t *can_msg - Pointer to the CAN frame buffer that was received.
     *
     * @return none
     */
    extern void CanRxMessageHandler_cid_frame(can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_FRAME_MESSAGE_HANDLER__ */
