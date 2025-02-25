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
 * \file turnoutboss_board_configuration.h
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
#ifndef __TURNOUTBOSS_BOARD_CONFIGURATION__
#define	__TURNOUTBOSS_BOARD_CONFIGURATION__

#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

// Identification locations (SNIP, ACDI) fill configuration space 0 - 126
// These have to align with the CDI XML file
// First segment
#define BOARD_LOCATION_CONFIG_MEM_ADDRESS               127 // Single Byte, 0 = LB; 1 = RB
#define BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS          128 // 8 Bytes for an Event ID
#define BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS         136 // 8 Bytes for an Event ID
#define BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS        144 // uses 2 pushbutton for normal/diverging or using one to toggle, 0 = two buttons; 1 = single button toggle
#define BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS  145 // uses both turnout feedbacks or just a single one, 0 = not used; 1 = single feedback sensors; 2 = two feedback sensor
#define BOARD_POINT_SIGNALHEAD_TYPE_CONFIG_MEM_ADDRESS  146 // does the point signal use a single or dual signal head
// some reserved space
// Second segment starts at 170 with 4 bytes of signal electrical configuration
#define SIGNAL_A_LED_CONFIGURATION                      170 // Is it a 3 or 2 light head
#define SIGNAL_B_LED_CONFIGURATION                      171 // Is it a 3 or 2 light head
#define SIGNAL_C_LED_CONFIGURATION                      172 // Is it a 3 or 2 light head
#define SIGNAL_D_LED_CONFIGURATION                      173 // Is it a 3 or 2 light head
#define LED_POLARITY_ADDRESS                            174 // Common Anode, Common Cathode, BiDirectional Yellow
#define DETECTOR_1_GAIN_ADDRESS                         175
#define DETECTOR_2_GAIN_ADDRESS                         176
#define DETECTOR_3_GAIN_ADDRESS                         177
#define SIGNAL_LED_BRIGHTNESS_GAIN_ADDRESS              178


#include "turnoutboss_types.h"



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    
    extern void TurnoutBossBoardConfiguration_initialize(openlcb_node_t *node, board_configuration_t* board_configuration);
    
    extern uint16_olcb_t TurnoutBossBoardConfiguration_write_eventID_to_configuration_memory(openlcb_node_t *node, event_id_t event, uint16_olcb_t address);

# define the start location of the producer/consumer array for CDI. This goes with the other definitions in turnout_board_configuration.h
#define START_OF_PRODUCER_CONSUMER_MAP                  200 // 416 bytes space for 51 producers and consumers

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_BOARD_CONFIGURATION__ */

