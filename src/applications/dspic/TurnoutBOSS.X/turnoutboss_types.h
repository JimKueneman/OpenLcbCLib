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

#ifdef MPLAB
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
#define EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED 0x0106
#define EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN 0x0107
#define EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED 0x0108
#define EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN 0x0109
#define EVENT_SUFFIX_TURNOUT_OBSERVED_NORMAL 0x0110
#define EVENT_SUFFIX_TURNOUT_OBSERVED_DIVERGING 0x0111
#define EVENT_SUFFIX_TURNOUT_OBSERVED_IN_MOTION 0x0112

#define EVENT_SUFFIX_SIGNAL_A_RED 0x0200
#define EVENT_SUFFIX_SIGNAL_A_YELLOW 0x0201
#define EVENT_SUFFIX_SIGNAL_A_GREEN 0x0202
#define EVENT_SUFFIX_SIGNAL_A_DARK 0x0203

#define EVENT_SUFFIX_SIGNAL_B_RED 0x0210
#define EVENT_SUFFIX_SIGNAL_B_YELLOW 0x0211
#define EVENT_SUFFIX_SIGNAL_B_GREEN 0x0212
#define EVENT_SUFFIX_SIGNAL_B_DARK 0x0213

#define EVENT_SUFFIX_SIGNAL_C_RED 0x0220
#define EVENT_SUFFIX_SIGNAL_C_YELLOW 0x0221
#define EVENT_SUFFIX_SIGNAL_C_GREEN 0x0222
#define EVENT_SUFFIX_SIGNAL_C_DARK 0x0223

#define EVENT_SUFFIX_SIGNAL_D_RED 0x0230
#define EVENT_SUFFIX_SIGNAL_D_YELLOW 0x0231
#define EVENT_SUFFIX_SIGNAL_D_GREEN 0x0232
#define EVENT_SUFFIX_SIGNAL_D_DARK 0x0233

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
#define OFFSET_EVENT_TURNOUT_OBSERVED_NORMAL 22
#define OFFSET_EVENT_TURNOUT_OBSERVED_DIVERGING 23
#define OFFSET_EVENT_TURNOUT_OBSERVED_IN_MOTION 24

#define OFFSET_EVENT_SIGNAL_A_RED 25
#define OFFSET_EVENT_SIGNAL_A_YELLOW 26
#define OFFSET_EVENT_SIGNAL_A_GREEN 27
#define OFFSET_EVENT_SIGNAL_A_DARK 28

#define OFFSET_EVENT_SIGNAL_B_RED 29
#define OFFSET_EVENT_SIGNAL_B_YELLOW 30
#define OFFSET_EVENT_SIGNAL_B_GREEN 31
#define OFFSET_EVENT_SIGNAL_B_DARK 32

#define OFFSET_EVENT_SIGNAL_C_RED 33
#define OFFSET_EVENT_SIGNAL_C_YELLOW 34
#define OFFSET_EVENT_SIGNAL_C_GREEN 35
#define OFFSET_EVENT_SIGNAL_C_DARK 36

#define OFFSET_EVENT_SIGNAL_D_RED 37
#define OFFSET_EVENT_SIGNAL_D_YELLOW 38
#define OFFSET_EVENT_SIGNAL_D_GREEN 39
#define OFFSET_EVENT_SIGNAL_D_DARK 40

#define OFFSET_EVENT_SIGNAL_STATE_A_STOP 41
#define OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP 42
#define OFFSET_EVENT_SIGNAL_STATE_B_STOP 43
#define OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP 44
#define OFFSET_EVENT_SIGNAL_STATE_CD_STOP 45
#define OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP 46

#define OFFSET_EVENT_VITAL_LOGIC_STATE_HELD 47
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_LEFT 48
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_RIGHT 49
#define OFFSET_EVENT_VITAL_LOGIC_STATE_CLEARED_BOTH 50

#define MAX_EVENT_SUFFIX 51

typedef uint16_olcb_t event_suffix_t[MAX_EVENT_SUFFIX];

// Occupancy Detector Nomenclature
#define OCCUPIED TRUE
#define UNOCCUPIED FALSE

// Turnout Feedback Nomenclature
#define ACTIVE TRUE
#define INACTIVE FALSE

// Turnout Pushbutton Nomenclature
#define OPEN LOW
#define CLOSED HIGH


// Turnout Observed/Command State Nomenclature (TLO, TRO, TLC, TRC)
#define TURNOUT_OBSERVED_NORMAL TRUE
#define TURNOUT_OBSERVED_DIVERGING FALSE
#define TURNOUT_OBSERVED_IN_MOTION 0b10

#define RED 0      // red signal
#define YELLOW 1   // yellow signal
#define GREEN 2    // green
#define DARK 3     // none lit


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
    uint8_olcb_t OMC : 1; // Occupancy Main Center (OMC) from the Board Left (BL), consumed by BR
    uint8_olcb_t OSC : 1; // Occupancy Siding Center (OSC) from the Board Left (BL), consumed by BR
    // BR only
    uint8_olcb_t OTR : 1; // Occupancy Turnout Right (OTR) from the Board Right BR)
    uint8_olcb_t OMR : 1; // Occupancy Main Right (OMR) from the Board Right (BR) 

} occupancy_state_t;

typedef struct {
    // BL only
    uint8_olcb_t TL  : 1; // Turnout Left 
    uint8_olcb_t TLC : 1; // Turnout Left Command  (normal position = TRUE; diverging position = FALSE)
    uint8_olcb_t TLO : 2; // Turnout Left Observed (normal position = TRUE; diverging position = FALSE; In-Motion = IN_MOTION) 
    // BR only
    uint8_olcb_t TR  : 1; // Turnout Right 
    uint8_olcb_t TRC : 1; // Turnout Right Command  (normal position = TRUE; diverging position = FALSE)
    uint8_olcb_t TRO : 2; // Turnout Right Observed (normal position = TRUE; diverging position = FALSE; In-Motion = IN_MOTION)

} turnout_state_t;

typedef struct {
    // BL only
    uint16_olcb_t SaBL : 2; // Signal A from the Board Left (BL)   RED/YELLOW/GREEN/DARK
    uint16_olcb_t SbBL : 2; // Signal B from the Board Left (BL)   RED/YELLOW/GREEN/DARK
    uint16_olcb_t ScBL : 2; // Signal CD from the Board Left (BL)  RED/YELLOW/GREEN/DARK
    uint16_olcb_t SdBL : 2; // Signal CD from the Board Left (BL)  RED/YELLOW/GREEN/DARK

    // BR only
    uint16_olcb_t SaBR : 2; // Signal A from the Board Right (BR)  RED/YELLOW/GREEN/DARK
    uint16_olcb_t SbBR : 2; // Signal B from the Board Right (BR)  RED/YELLOW/GREEN/DARK
    uint16_olcb_t ScBR : 2; // Signal CD from the Board Right (BR) RED/YELLOW/GREEN/DARK
    uint16_olcb_t SdBR : 2; // Signal CD from the Board Right (BR) RED/YELLOW/GREEN/DARK

} lamp_state_t;

typedef struct {
    // BL
    uint16_olcb_t SaBLstop : 1; // when BL current state, when BR consumed by; stop ACTIVE/INACTIVE
    uint16_olcb_t SbBLstop : 1; // when BL current state, when BR consumed by; stop ACTIVE/INACTIVE
    uint16_olcb_t ScdBLstop : 1; // when BL current state, when BR not used; stop ACTIVE/INACTIVE

    uint16_olcb_t ScdBALstop : 1; // consumes when BL; stop ACTIVE/INACTIVE

    // BR
    uint16_olcb_t SaBRstop : 1; // when BR current state, when BL consumed by; stop ACTIVE/INACTIVE
    uint16_olcb_t SbBRstop : 1; // when BR current state, when BL consumed by; stop ACTIVE/INACTIVE
    uint16_olcb_t ScdBRstop : 1; // When BR current state, when BL not used; stop ACTIVE/INACTIVE

    uint16_olcb_t ScdBARstop : 1; // consumes when BR; stop ACTIVE/INACTIVE

} signal_stop_state_t;

typedef struct {
    uint8_olcb_t SHD : 1; // Signal Held - CTC forcing Signal Held in all directions ACTIVE/INACTIVE
    uint8_olcb_t SCL : 1; // Signal Cleared-Left - CTC forcing Signal Held on the Right only ACTIVE/INACTIVE
    uint8_olcb_t SCR : 1; // Signal Cleared-Right - CTC forcing Signal Held on the Left only ACTIVE/INACTIVE
    uint8_olcb_t SCB : 1; // Signal Cleared-Both - CTC not forcing any stops ACTIVE/INACTIVE

} vital_logic_state_t;

typedef struct {
    uint8_olcb_t turnout_normal : 1;
    uint8_olcb_t turnout_diverging : 1;

} remote_control_state_t;

typedef struct {
    uint8_olcb_t occupany_1 : 1;
    uint8_olcb_t occupany_2 : 1;
    uint8_olcb_t occupany_3 : 1;
    uint8_olcb_t turnout_feedback_normal : 1;
    uint8_olcb_t turnout_feedback_diverging : 1;
    uint8_olcb_t turnout_pushbutton_normal : 1;
    uint8_olcb_t turnout_pushbutton_diverging : 1;

} hardware_input_states_t;

typedef struct {
    occupancy_state_t occupancy;
    turnout_state_t turnout;
    lamp_state_t signal_lamps;
    signal_stop_state_t stop;
    vital_logic_state_t ctc_control;
    remote_control_state_t remote_control;
    hardware_input_states_t hardware;
            
} next_signaling_state_t;


typedef struct {
    occupancy_state_t occupancy;
    turnout_state_t turnout;
    lamp_state_t lamps;
    signal_stop_state_t stop;
    vital_logic_state_t ctc_control;
    next_signaling_state_t next; // collects information for calculating the next state to compare to the current state fields
    hardware_input_states_t hardware;
    uint8_olcb_t pushbutton_normal_toggled: 1;
    uint8_olcb_t pushbutton_diverging_toggled: 1;

} signaling_state_t;

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

