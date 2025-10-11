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
 * \file can_main_statemachine.h
 *
 * Where the real work in dispatching the incoming OpenLcb messages to the various
 * handlers to process.  It will call the OpenLcb main statemachine when needed.  
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_MAIN_STATEMACHINE__
#define	__CAN_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef struct {
        void (*lock_shared_resources)(void);
        void (*unlock_shared_resources)(void);
        bool (*send_can_message)(can_msg_t *msg);
        bool (*send_openlcb_message)(openlcb_msg_t *openlcb_msg);
        openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);
        openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);
        openlcb_node_t *(*openlcb_node_find_by_alias)(uint16_t alias);
        void (*login_statemachine_run)(can_statemachine_info_t *can_statemachine_info);
        alias_mapping_info_t*(*alias_mapping_get_alias_mapping_info)(void);
        void (*alias_mapping_unregister)(uint16_t alias);

        bool (*handle_duplicate_aliases)(void);
        bool (*handle_outgoing_can_message)(void);
        bool (*handle_login_outgoing_can_message)(void);
        bool (*handle_login_outgoing_openlcb_message)(void);
        bool (*handle_reenumerate_openlcb_message)(void);
        bool (*handle_try_enumerate_first_node)(void);
        bool (*handle_try_enumerate_next_node)(void);



    } interface_can_main_statemachine_t;

    extern void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine);

    extern void CanMainStateMachine_run(void);

    extern can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void);

    extern bool CanMainStatemachine_handle_duplicate_aliases(void);

    extern bool CanMainStatemachine_handle_login_outgoing_can_message(void);
    
    extern bool CanMainStatemachine_handle_login_outgoing_openlcb_message(void);

    extern bool CanMainStatemachine_handle_outgoing_can_message(void);

    extern bool CanMainStatemachine_handle_reenumerate_openlcb_message(void);

    extern bool CanMainStatemachine_handle_try_enumerate_first_node(void);

    extern bool CanMainStatemachine_handle_try_enumerate_next_node(void);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_MAIN_STATEMACHINE__ */

