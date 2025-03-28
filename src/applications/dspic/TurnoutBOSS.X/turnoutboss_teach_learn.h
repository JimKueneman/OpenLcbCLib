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
 * \file turnoutboss_teach_learn.c
 *
 * TurnoutBOSS main
 *
 * @author Jim Kueneman
 * @date 1 Mar 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_TEACH_LEARN__
#define	__TURNOUTBOSS_TEACH_LEARN__


#include "turnoutboss_types.h"

#define STATE_TEACH_LEARN_DEACTIVATED 0

#define STATE_TEACH_LEARN_SET_DEFAULTS_START 1
#define STATE_TEACH_LEARN_SET_DEFAULTS_WAIT_FOR_TEACH 2
#define STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE 3

#define STATE_TEACH_LEARN_START_LEARN 10
#define STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START 11
#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_LH_BOARD_START 12
#define STATE_TEACH_LEARN_START_LEARN_LH_BOARD 13
#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_RH_BOARD_START 14
#define STATE_TEACH_LEARN_START_LEARN_RH_BOARD 15
#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_LH_START 16
#define STATE_TEACH_LEARN_START_LEARN_BOARD_LH 17
#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_RH_START 18
#define STATE_TEACH_LEARN_START_LEARN_BOARD_RH 19

#define STATE_TEACH_LEARN_START_LEARN_SIGNALS 50
#define STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START_SIGNALS 51

#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_ANODE_START 52
#define STATE_TEACH_LEARN_START_SIGNALS_COMMON_ANODE 53

#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_CATHODE_START 54
#define STATE_TEACH_LEARN_START_SIGNALS_COMMON_CATHODE 55

#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_BI_DIRECTIONAL_START 56
#define STATE_TEACH_LEARN_START_SIGNALS_BI_DIRECTIONAL 57

#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_TWO_START 58
#define STATE_TEACH_LEARN_START_TURNOUT_CTRL_TWO 59

#define STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_ONE_START 60
#define STATE_TEACH_LEARN_START_TURNOUT_CTRL_ONE 61



#define STATE_TEACH_LEARN_ACTION_DONE 100



//#define STATE_TEACH_LEARN_CONFIGURE_LH_BOARD 4
//#define STATE_TEACH_LEARN_CONFIGURE_RH_BOARD 5
//#define STATE_TEACH_LEARN_CONFIGURE_BOARD 6
//#define STATE_TEACH_LEARN_SIGNAL_WIRING 7


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void TurnoutBossTeachLearn_initialize(void);
    
    extern void TurnoutBossTeachLearn_check_for_enable(void);
    
    extern void TurnoutBossTeachLearn_run(openlcb_node_t *openlcb_node);
    
    // Called from the 100ms timer so don't access any data structures or cause other problems.  Just set the IO pins
    extern void TurnoutBossTeachLearn_update_leds(uint8_olcb_t teach_learn_state);
    
    extern teach_learn_state_t TurnoutBossTeachLearn_teach_learn_state;
    
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_TEACH_LEARN__ */

