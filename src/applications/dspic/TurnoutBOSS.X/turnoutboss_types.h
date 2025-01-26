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
 * \file turnoutboss_types.h
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and 
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 22 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_TYPES__
#define	__TURNOUTBOSS_TYPES__

#include <xc.h> // include processor files - each processor file is guarded.  

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif


#define EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED 0x0010   // OML
#define EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED 0x0011 // OML
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_OCCUPIED 0x0012 // OTL
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED 0x0013 // OTL
#define EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED 0x0014 // OMC
#define EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED 0x0015 // OMC
#define EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED 0x0016 // OSC
#define EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED 0x0017 // OSC
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED 0x0018 // OTR
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED 0x0019 // OTR
#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_OCCUPIED 0x0010 // OMR
#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED 0x0011 // OMR

#define EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL 0x0100
#define EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING 0x0101
#define EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_ACTIVE 0x0102
#define EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_INACTIVE 0x0103
#define EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_ACTIVE 0x0104
#define EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_INACTIVE 0x0105
#define EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN 0x0106
#define EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED 0x0107
#define EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN 0x0108
#define EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED 0x0109
#define EVENT_SUFFIX_TURNOUT_IS_SET_NORMAL 0x0110
#define EVENT_SUFFIX_TURNOUT_IS_SET_DIVERGING 0x0111
#define EVENT_SUFFIX_TURNOUT_IS_SET_IN_MOTION 0x0112

#define EVENT_SUFFIX_SIGNAL_A_RED 0x2000
#define EVENT_SUFFIX_SIGNAL_A_YELLOW 0x2001
#define EVENT_SUFFIX_SIGNAL_A_GREEN 0x2002
#define EVENT_SUFFIX_SIGNAL_A_DARK 0x2003
#define EVENT_SUFFIX_SIGNAL_A_LIT 0x2006
#define EVENT_SUFFIX_SIGNAL_A_NOT_LIT 0x2007
#define EVENT_SUFFIX_SIGNAL_A_HELD 0x2008
#define EVENT_SUFFIX_SIGNAL_A_NOT_HELD 0x2009

#define EVENT_SUFFIX_SIGNAL_B_RED 0x2010
#define EVENT_SUFFIX_SIGNAL_B_YELLOW 0x2011
#define EVENT_SUFFIX_SIGNAL_B_GREEN 0x2012
#define EVENT_SUFFIX_SIGNAL_B_DARK 0x2013
#define EVENT_SUFFIX_SIGNAL_B_LIT 0x2016
#define EVENT_SUFFIX_SIGNAL_B_NOT_LIT 0x2017
#define EVENT_SUFFIX_SIGNAL_B_HELD 0x2018
#define EVENT_SUFFIX_SIGNAL_B_NOT_HELD 0x2019

#define EVENT_SUFFIX_SIGNAL_C_RED 0x2020
#define EVENT_SUFFIX_SIGNAL_C_YELLOW 0x2021
#define EVENT_SUFFIX_SIGNAL_C_GREEN 0x2022
#define EVENT_SUFFIX_SIGNAL_C_DARK 0x2023
#define EVENT_SUFFIX_SIGNAL_C_LIT 0x2026
#define EVENT_SUFFIX_SIGNAL_C_NOT_LIT 0x2027
#define EVENT_SUFFIX_SIGNAL_C_HELD 0x2028
#define EVENT_SUFFIX_SIGNAL_C_NOT_HELD 0x2029

#define EVENT_SUFFIX_SIGNAL_D_RED 0x2030
#define EVENT_SUFFIX_SIGNAL_D_YELLOW 0x2031
#define EVENT_SUFFIX_SIGNAL_D_GREEN 0x2032
#define EVENT_SUFFIX_SIGNAL_D_DARK 0x2033
#define EVENT_SUFFIX_SIGNAL_D_LIT 0x2036
#define EVENT_SUFFIX_SIGNAL_D_NOT_LIT 0x2037
#define EVENT_SUFFIX_SIGNAL_D_HELD 0x2038
#define EVENT_SUFFIX_SIGNAL_D_NOT_HELD 0x2039

#define EVENT_SUFFIX_SIGNAL_STATE_A_STOP 0x0300
#define EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP 0x0301
#define EVENT_SUFFIX_SIGNAL_STATE_B_STOP 0x0304
#define EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP 0x0305
#define EVENT_SUFFIX_SIGNAL_STATE_CD_STOP 0x0308
#define EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP 0x0309

#define EVENT_SUFFIX_VITAL_LOGIC_STATE_HELD 0x0500
#define EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_LEFT 0x0501
#define EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_RIGHT 0x0502
#define EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_BOTH 0x0503

#define OFFSET_EVENT_OCCUPANCY_MAIN_LEFT_OCCUPIED 0
#define OFFSET_EVENT_OCCUPANCY_MAIN_LEFT_UNOCCUPIED 1
#define OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_OCCUPIED 2
#define OFFSET_EVENT_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED 3
#define OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED 4
#define OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED 5
#define OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED 6
#define OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED 7
#define OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED 8
#define OFFSET_EVENT_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED 9
#define OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_OCCUPIED 10
#define OFFSET_EVENT_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED 11

#define OFFSET_EVENT_TURNOUT_COMMAND_NORMAL 12
#define OFFSET_EVENT_TURNOUT_COMMAND_DIVERGING 13
#define OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_ACTIVE 14
#define OFFSET_EVENT_TURNOUT_FEEDBACK_NORMAL_INACTIVE 15
#define OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_ACTIVE 16
#define OFFSET_EVENT_TURNOUT_FEEDBACK_DIVERGING_INACTIVE 17
#define OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_OPEN 18
#define OFFSET_EVENT_TURNOUT_BUTTON_NORMAL_CLOSED 19
#define OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_OPEN 20
#define OFFSET_EVENT_TURNOUT_BUTTON_DIVERGING_CLOSED 21
#define OFFSET_EVENT_TURNOUT_IS_SET_NORMAL 22
#define OFFSET_EVENT_TURNOUT_IS_SET_DIVERGING 23
#define OFFSET_EVENT_TURNOUT_IS_SET_IN_MOTION 24

#define OFFSET_EVENT_SIGNAL_A_RED 25
#define OFFSET_EVENT_SIGNAL_A_YELLOW 26
#define OFFSET_EVENT_SIGNAL_A_GREEN 27
#define OFFSET_EVENT_SIGNAL_A_DARK 28
#define OFFSET_EVENT_SIGNAL_A_LIT 29
#define OFFSET_EVENT_SIGNAL_A_NOT_LIT 30
#define OFFSET_EVENT_SIGNAL_A_HELD 31
#define OFFSET_EVENT_SIGNAL_A_NOT_HELD 32

#define OFFSET_EVENT_SIGNAL_B_RED 33
#define OFFSET_EVENT_SIGNAL_B_YELLOW 34
#define OFFSET_EVENT_SIGNAL_B_GREEN 35
#define OFFSET_EVENT_SIGNAL_B_DARK 36
#define OFFSET_EVENT_SIGNAL_B_LIT 37
#define OFFSET_EVENT_SIGNAL_B_NOT_LIT 38
#define OFFSET_EVENT_SIGNAL_B_HELD 39
#define OFFSET_EVENT_SIGNAL_B_NOT_HELD 40

#define OFFSET_EVENT_SIGNAL_C_RED 41
#define OFFSET_EVENT_SIGNAL_C_YELLOW 42
#define OFFSET_EVENT_SIGNAL_C_GREEN 43
#define OFFSET_EVENT_SIGNAL_C_DARK 44
#define OFFSET_EVENT_SIGNAL_C_LIT 45
#define OFFSET_EVENT_SIGNAL_C_NOT_LIT 46
#define OFFSET_EVENT_SIGNAL_C_HELD 47
#define OFFSET_EVENT_SIGNAL_C_NOT_HELD 48

#define OFFSET_EVENT_SIGNAL_D_RED 49
#define OFFSET_EVENT_SIGNAL_D_YELLOW 50
#define OFFSET_EVENT_SIGNAL_D_GREEN 51
#define OFFSET_EVENT_SIGNAL_D_DARK 52
#define OFFSET_EVENT_SIGNAL_D_LIT 53
#define OFFSET_EVENT_SIGNAL_D_NOT_LIT 54
#define OFFSET_EVENT_SIGNAL_D_HELD 55
#define OFFSET_EVENT_SIGNAL_D_NOT_HELD 56

#define OFFSET_EVENT_SIGNAL_STATE_A_STOP 57
#define OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP 58
#define OFFSET_EVENT_SIGNAL_STATE_B_STOP 59
#define OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP 60
#define OFFSET_EVENT_SIGNAL_STATE_CD_STOP 61
#define OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP 62

#define OFFSET_EVENT_VITAL_LOGIC_STATE_HELD 63
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_LEFT 64
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_RIGHT 65
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_BOTH 66

#define MAX_EVENT_SUFFIX 67

typedef uint16_olcb_t event_suffix_t[MAX_EVENT_SUFFIX];


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
    uint8_olcb_t TLO : 2; // Turnout Left Observed (normal position = TRUE; diverging position = FALSE; In-Motion = IN_MOTION) 
    // BR only
    uint8_olcb_t TR : 1; // Turnout Right 
    uint8_olcb_t TRC : 1; // Turnout Right Command  (normal position = TRUE; diverging position = FALSE)
    uint8_olcb_t TRO : 2; // Turnout Right Observed (normal position = TRUE; diverging position = FALSE; In-Motion = IN_MOTION)

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

typedef struct {
    uint8_olcb_t occupany_1 : 1;
    uint8_olcb_t occupany_2 : 1;
    uint8_olcb_t occupany_3 : 1;
    uint8_olcb_t turnout_normal_feedback : 1;
    uint8_olcb_t turnout_diverging_feedback : 1;
    uint8_olcb_t turnout_pushbutton_normal : 1;
    uint8_olcb_t turnout_pushbutton_diverging : 1;

} hardware_input_states_t;

typedef struct {
    uint8_olcb_t send : 1; // marked to send this event
    uint8_olcb_t valid_producer : 1; // true if this is a valid Event to create a producer from
    uint8_olcb_t valid_consumer : 1; // true if this is a valid Event to accept a consumer from
    uint8_olcb_t core_signaling : 1; // true if this is a core signaling event (allows filtering of events for debug)

} event_engine_struct_state_t;

typedef struct {
    uint16_olcb_t suffix;
    event_engine_struct_state_t state;

} event_engine_struct_t;

typedef struct {
    event_engine_struct_t events[MAX_EVENT_SUFFIX];
    uint8_olcb_t current_index;

} send_event_engine_t;

typedef enum {
    BL,
    BR

} board_type_enum_t;

typedef enum {
    Pushbutton_Dual,
    Pushbutton_Single

} pushbutton_type_enum_t;

typedef enum {
    TurnoutFeedbackUnused,
    TurnoutFeedbackSingle,
    TurnoutFeedbackDual

} turnout_feedback_type_enum_t;

typedef enum {
    PointSignalHeadSingle,
    PointSignalHeadDual
            
} point_signalhead_type_enum_t;

typedef struct {
    board_type_enum_t board_location;
    pushbutton_type_enum_t pushbutton_type;
    turnout_feedback_type_enum_t turnout_feedback_type;
    point_signalhead_type_enum_t point_signalhead_type;

    node_id_t board_to_the_left;
    node_id_t board_to_the_right;
    uint8_olcb_t detector_gain_a;
    uint8_olcb_t detector_gain_b;
    uint8_olcb_t detector_gain_c;
    uint8_olcb_t led_brightness_gain;

} board_configuration_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_TYPES__ */

