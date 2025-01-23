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
 * \file turnoutboss_hardware_handler_pushbuttons.h
 *
 * Scans the TurnoutBoss Hardware States and updates the TurnoutBoss_Signaling_States
 * 
 *
 * @author Jim Kueneman
 * @date 23 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_HARDWARE_HANDLER_PUSHBUTTONS__
#define	__TURNOUTBOSS_HARDWARE_HANDLER_PUSHBUTTONS__

#include "turnoutboss_hardware_handler.h"
#include "turnoutboss_signaling_states.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_board_configuration.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    void TurnoutBossHardwareHandler_Pushbuttons_check_state_changes(input_filters_t* filters, signaling_state_t* signaling_state, send_event_engine_t* event_engine, board_type_enum_t board_location, pushbutton_type_enum_t pushbutton_type);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_HARDWARE_HANDLER_PUSHBUTTONS__ */

