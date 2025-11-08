/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file openlcb_login_statemachine.h
 *
 *
 * @author Jim Kueneman
 * @date 11 Oct 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_LOGIN_STATEMACHINE__
#define	__OPENLCB_LOGIN_STATEMACHINE__

#include "openlcb_types.h"

typedef struct {
    void (*load_initialization_complete)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);
    void (*load_producer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);
    void (*load_consumer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);
    void (*process_login_statemachine)(openlcb_login_statemachine_info_t *statemachine_info);
    
    // For test injection
    bool (*handle_outgoing_openlcb_message)(void);
    bool (*handle_try_reenumerate)(void);
    bool (*handle_try_enumerate_first_node)(void);
    bool (*handle_try_enumerate_next_node)(void);

} interface_openlcb_login_state_machine_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine);

    extern void OpenLcbLoginMainStatemachine_run(void);
    
    // For test coverage

    extern void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info);
    
    extern bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void);
 
    extern bool OpenLcbLoginStatemachine_handle_try_reenumerate(void);

    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void);

    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void);
    
    extern openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_LOGIN_STATEMACHINE__ */