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
 * \file turnoutboss_signaling_states.h
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


#define ACTIVE 1
#define INACTIVE 0

#define OPEN 1
#define CLOSED 0

// These are the states that the equations utilize to calculate what signals to light
// in the mast heads.  They are updated from the turnoutboss_event_handler.h (deals with
// the states of the nodes to the right and left of this one).  The other place they are
// updated is from the turnout_boss_hardware_handler.h and the current state of the 
// switches/occupancy detectors/feedback sensors of this node.

typedef struct {
    // BL only
    uint8_olcb_t OML : 1; // Occupancy Main Line (OML) from the Board Adjacent Left (BAL) main line 
    uint8_olcb_t OTL : 1; // Occupancy Turnout Left (OTL) from the Board Left (BL)
    // BL/BR
    uint8_olcb_t OMC : 1; // Occupancy Main Center (OMC) from the Board Left (BL)
    uint8_olcb_t OSC : 1; // Occupancy Siding Center (OSC) from the Board Left (BL)
    // BR only
    uint8_olcb_t OTR : 1; // Occupancy Turnout Right (OTR) from the Board Right BR)
    uint8_olcb_t OMR : 1; // Occupancy Main Right (OMR) from the Board Right (BR) 

} occupancy_state_t;


typedef struct {
    // BL only
    uint8_olcb_t TL : 1; // Turnout Left 
    uint8_olcb_t TLC : 1; // Turnout Left Command  (normal position = TRUE; diverging position = FALSE)
    uint8_olcb_t TLO : 1; // Turnout Left Observed (normal position = TRUE; diverging position = FALSE) 
    // BR only
    uint8_olcb_t TR : 1; // Turnout Right 
    uint8_olcb_t TRC : 1; // Turnout Right Command  (normal position = TRUE; diverging position = FALSE)
    uint8_olcb_t TRO : 1; // Turnout Right Observed (normal position = TRUE; diverging position = FALSE)

} turnout_state_t;


typedef struct {
    // BL only
    uint8_olcb_t ScdBAL : 1; // Signal CD from the Board Adjacent Left (BAL) showing stop
    uint8_olcb_t ScdBL : 1; // Signal CD from the Board Left (BL) showing stop
    uint8_olcb_t SaBL : 1; // Signal A from the Board Left (BL) showing stop
    uint8_olcb_t SbBL : 1; // Signal B from the Board Left (BL) showing stop
    // BR only
    uint8_olcb_t SaBR : 1; // Signal A from the Board Right (BR) showing stop
    uint8_olcb_t SbBR : 1; // Signal B from the Board Right (BR) showing stop
    uint8_olcb_t ScdBR : 1; // Signal CD from the Board Right (BR) showing stop
    uint8_olcb_t ScdBAR : 1; // Signal A from the Board Adjacent Right (BAR) showing stop

} signal_state_t;

typedef struct {
    // BL only
    uint8_olcb_t ScdBALstop : 1; // Signal head CD on Board Adjacent Left (BAL) shows stop
    uint8_olcb_t SaBRstop : 1; // Signal head A on Board Left (BL) shows stop
    uint8_olcb_t SbBRstop : 1; // Signal head B on Board Left (BL) shows stop
    // BR only
    uint8_olcb_t ScdBARstop : 1; // Signal head CD on Board Adjacent Right (BAR) shows stop
    uint8_olcb_t SaBLstop : 1; // Signal head A on Board Left (BL) shows stop
    uint8_olcb_t SbBLstop : 1; // Signal head B on Board Left (BL) shows stop

} signal_stop_state_t;

typedef struct {
    uint8_olcb_t SHD : 1; // Signal Held - CTC forcing Signal Held in all directions
    uint8_olcb_t SCL : 1; // Signal Cleared-Left - CTC forcing Signal Held on the Right only
    uint8_olcb_t SCR : 1; // Signal Cleared-Right - CTC forcing Signal Held on the Left only
    uint8_olcb_t SCB : 1; // Signal Cleared-Both - CTC not forcing any stops

} vital_logic_state_t;

typedef struct {
    occupancy_state_t occupancy;
    turnout_state_t turnout;
    signal_state_t signal;
    signal_stop_state_t stop;
    vital_logic_state_t ctc_control;

} signaling_state_t;



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern signaling_state_t TurnoutBoss_Signaling_States;
    
    extern void TurnoutBoss_Signaling_States_initialize(openlcb_node_t *node);
    
    

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __SIGNALING_STATES__ */

