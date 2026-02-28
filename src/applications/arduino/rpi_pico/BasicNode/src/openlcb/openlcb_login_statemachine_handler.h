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
* @brief Message construction handlers for the login sequence: Initialization
*        Complete, Producer Identified, and Consumer Identified messages.
*
* @author Jim Kueneman
* @date 28 Feb 2026
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__
#define    __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Callbacks that map event state to the correct Identified MTI.  Both REQUIRED. */
typedef struct {

        /** @brief Return the Producer Identified MTI (Valid/Invalid/Unknown) for producers.list[event_index]. */
    uint16_t(*extract_producer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

        /** @brief Return the Consumer Identified MTI (Valid/Invalid/Unknown) for consumers.list[event_index]. */
    uint16_t(*extract_consumer_event_state_mti)(openlcb_node_t *openlcb_node, uint16_t event_index);

} interface_openlcb_login_message_handler_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup before login processing.
         *
         * @param interface  Pointer to @ref interface_openlcb_login_message_handler_t.  Must remain valid for application lifetime.
         */
    extern void OpenLcbLoginMessageHandler_initialize(const interface_openlcb_login_message_handler_t *interface);

        /**
         * @brief Builds the Initialization Complete message and transitions to producer
         *        event enumeration.
         *
         * @param statemachine_info  Pointer to @ref openlcb_login_statemachine_info_t context.
         */
    extern void OpenLcbLoginMessageHandler_load_initialization_complete(openlcb_login_statemachine_info_t *statemachine_info);

        /**
         * @brief Builds one Producer Identified message; sets enumerate flag if more remain.
         *
         * @details Skips to consumer enumeration when all producers are done or count is 0.
         *
         * @param statemachine_info  Pointer to @ref openlcb_login_statemachine_info_t context.
         */
    extern void OpenLcbLoginMessageHandler_load_producer_event(openlcb_login_statemachine_info_t *statemachine_info);

        /**
         * @brief Builds one Consumer Identified message; sets enumerate flag if more remain.
         *
         * @details Final login step — transitions to RUNSTATE_LOGIN_COMPLETE when all
         *          consumers are done or count is 0.
         *
         * @param statemachine_info  Pointer to @ref openlcb_login_statemachine_info_t context.
         */
    extern void OpenLcbLoginMessageHandler_load_consumer_event(openlcb_login_statemachine_info_t *statemachine_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE_HANDLER__ */
