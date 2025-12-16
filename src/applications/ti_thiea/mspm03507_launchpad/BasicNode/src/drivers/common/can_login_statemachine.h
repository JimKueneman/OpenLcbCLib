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
 * \file can_login_statemachine.h
 *
 *
 * @author Jim Kueneman
 * @date 12 Aug 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVERS_COMMON_CAN_CAN_LOGIN_STATEMACHINE__
#define	__DRIVERS_COMMON_CAN_CAN_LOGIN_STATEMACHINE__

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"


typedef struct {
    
     void (*state_init)(can_statemachine_info_t *can_statemachine_info);
     void (*state_generate_seed)(can_statemachine_info_t *can_statemachine_info);
     void (*state_generate_alias)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_cid07)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_cid06)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_cid05)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_cid04)(can_statemachine_info_t *can_statemachine_info);
     void (*state_wait_200ms)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_rid)(can_statemachine_info_t *can_statemachine_info);
     void (*state_load_amd)(can_statemachine_info_t *can_statemachine_info);
    
    
} interface_can_login_state_machine_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    extern void CanLoginStateMachine_initialize(const interface_can_login_state_machine_t *interface_can_login_state_machine);

    extern void CanLoginStateMachine_run(can_statemachine_info_t *can_statemachine_info);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVERS_COMMON_CAN_CAN_LOGIN_STATEMACHINE__ */

