/** 
 * 
 * @subsection Description
 * 
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This module defines 
 * the specific handlers to respond to a CAN level requests and login requirements.  
 * Each function completes a task that is required for an associated request or login
 * state.<br><br>
 * 
 * These functions are assigned to the \ref interface_can_login_state_machine_t which 
 * are called from within the CAN login statemachine switch statement.<br>
 * 
 * @note Any handler may be overridden by assigning a custom function pointer to the 
 * \ref interface_can_login_state_machine_t field during initialization of the application.<br>
 * see: \ref CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine);
 * 
 *  
 * @subsection License
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
 * @copyright Copyright (c) 2025, Jim Kueneman
 * @author Jim Kueneman
 * @date 14 Dec 2025
 * @file can_login_message_handler.h
 *
 */


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
 * \file can_login_message_handler.h
 *
 * When a node is logging into the network on a CAN bus it must follow a specific
 * flow to allocate a unique alias ID and broadcast its events.  This is the handler 
 * that is called from the CAN main statemachine to accomplish that when a new node
 * is created.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVERS_COMMON_CAN_CAN_LOGIN_MESSAGE_HANDLER__
#define	__DRIVERS_COMMON_CAN_CAN_LOGIN_MESSAGE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @struct interface_can_login_message_handler_t
     * @brief A structure to define function dependancies that this module requires.
     */
    typedef struct {
        /** Registers an Alias Mapping pair */
        alias_mapping_t*(*alias_mapping_register)(uint16_t alias, node_id_t node_id);
        /** Finds an Alias Mapping by the Alias */
        alias_mapping_t*(*alias_mapping_find_mapping_by_alias)(uint16_t alias);
        // Callback events
        /** Callback function for an Application to be notified when an Alias has been successfully registered */
        void (*on_alias_change)(uint16_t alias, node_id_t node_id);

    } interface_can_login_message_handler_t;


    /**
     * @brief Initializes the CAN Login Message Handler module<br>
     * 
     * @param none
     * 
     * @return none
     * 
     * @note This must always be called during application initialization
     */
    extern void CanLoginMessageHandler_initialize(const interface_can_login_message_handler_t *interface);

    extern void CanLoginMessageHandler_state_init(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_generate_seed(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_generate_alias(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_load_cid07(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_load_cid06(can_statemachine_info_t *can_statemachine_infog);

    extern void CanLoginMessageHandler_state_load_cid05(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_load_cid04(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_wait_200ms(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_load_rid(can_statemachine_info_t *can_statemachine_info);

    extern void CanLoginMessageHandler_state_load_amd(can_statemachine_info_t *can_statemachine_info);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVERS_COMMON_CAN_CAN_LOGIN_MESSAGE_HANDLER__ */

