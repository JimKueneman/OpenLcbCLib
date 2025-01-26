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
 * \file turnoutboss_signal_calculations.h
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and 
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 20 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __SIGNALING_STATES__
#define	__SIGNALING_STATES__

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


    extern void TurnoutBossSignalCalculations_initialize(signaling_state_t* states, signaling_state_t* states_next);

    extern void TurnoutBossSignalCalculations_run_board_left(
            signaling_state_t* states,
            signaling_state_t* states_next,
            hardware_input_states_t* states_hardware,
            hardware_input_states_t* states_hardware_next,
            send_event_engine_t* event_engine,
            board_configuration_t* _board_configuration
    );
    
    extern void TurnoutBossSignalCalculations_run_board_right(
            signaling_state_t* states,
            signaling_state_t* states_next,
            hardware_input_states_t* states_hardware,
            hardware_input_states_t* states_hardware_next,
            send_event_engine_t* event_engine,
            board_configuration_t* _board_configuration
    );

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __SIGNALING_STATES__ */

