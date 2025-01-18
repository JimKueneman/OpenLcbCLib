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
 * \turnoutboss_event_hander.c
 *
 * @author Jim Kueneman
 * @date 17 Jan 2025
 */

#include "turnoutboss_event_handler.h"

#ifdef PLATFORMIO
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#else
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#endif

#define BOARD_USAGE_CONFIG_MEM_ADDRESS 0x7F
#define BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS 0x80
#define BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS 0x88

#define EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED 0x0000
#define EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED 0x0001
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_OCCUPIED 0x0002
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED 0x0003
#define EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED 0x0004
#define EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED 0x0005
#define EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED 0x0006
#define EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED 0x0007
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED 0x0008
#define EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED 0x0009
//#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_OCCUPIED 0x0000
//#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED 0x0001

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

#define EVENT_SUFFIX_SIGNAL_A_RED 0x2000
#define EVENT_SUFFIX_SIGNAL_A_YELLOW 0x2001
#define EVENT_SUFFIX_SIGNAL_A_GREEN 0x2002
#define EVENT_SUFFIX_SIGNAL_A_DARK 0x2003

#define EVENT_SUFFIX_SIGNAL_B_RED 0x2004
#define EVENT_SUFFIX_SIGNAL_B_YELLOW 0x2005
#define EVENT_SUFFIX_SIGNAL_B_GREEN 0x2006
#define EVENT_SUFFIX_SIGNAL_B_DARK 0x2007

#define EVENT_SUFFIX_SIGNAL_C_RED 0x2008
#define EVENT_SUFFIX_SIGNAL_C_YELLOW 0x2009
#define EVENT_SUFFIX_SIGNAL_C_GREEN 0x200A
#define EVENT_SUFFIX_SIGNAL_C_DARK 0x200B

#define EVENT_SUFFIX_SIGNAL_D_RED 0x200C
#define EVENT_SUFFIX_SIGNAL_D_YELLOW 0x200D
#define EVENT_SUFFIX_SIGNAL_D_GREEN 0x200E
#define EVENT_SUFFIX_SIGNAL_D_DARK 0x200F

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



node_id_t _extract_board_adjecent_node_id_from_config_mem(openlcb_node_t *_node, configuration_memory_buffer_t *_eventid_buffer, uint32_olcb_t _config_mem_address) {

    node_id_t board_adjacent = NULL_NODE_ID;

    // Read configuration memory to see if the Adjacent Left Board is available, it so create these Events
    if (Application_read_configuration_memory(_node, _config_mem_address, LEN_EVENT_ID, _eventid_buffer) == LEN_EVENT_ID) {
         
        board_adjacent = Utilities_extract_node_id_from_config_mem_buffer(_eventid_buffer, 0);
                
        if ((board_adjacent > 0x00) && (board_adjacent < 0x1000000000000)) {

            return board_adjacent;
        }
    }

    return NULL_NODE_ID;
}

void _register_common_local_board_events(openlcb_node_t *_node) {

    event_id_t _event_id_base = _node->id << 16;
    
   
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL); // Turnout Command Normal
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING); // Turnout Command Diverging

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_ACTIVE); // Turnout Feedback Normal Active
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_FEEDBACK_NORMAL_INACTIVE); // Turnout Feedback Normal Inactive
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_ACTIVE); // Turnout Feedback Diverging Active
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_FEEDBACK_DIVERGING_INACTIVE); // Turnout Feedback Diverging Inactive

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN); // Turnout Button Normal Closed
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED); // Turnout Button Normal Open
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN); // Turnout Button Diverging Closed
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED); // Turnout Button Diverging Open
  
    
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_RED); // Signal A Red
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_YELLOW); // Signal A Yellow
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_GREEN); // Signal A Green
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_DARK); // Signal A Dark

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_RED); // Signal B Red
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_YELLOW); // Signal B Yellow
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_GREEN); // Signal B Green
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_DARK); // Signal B Dark

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_RED); // Signal C Red
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_YELLOW); // Signal C Yellow
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_GREEN); // Signal C Green
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_DARK); // Signal C Dark

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_RED); // Signal D Red
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_YELLOW); // Signal D Yellow
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_GREEN); // Signal D Green
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_DARK); // Signal D Dark
    

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP); // Signal A Stop
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP); // Signal A NonStop
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP); // Signal B Stop
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP); // Signal B NonStop
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP); // Signal CD Stop
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP); // Signal CD NonStop

    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_VITAL_LOGIC_STATE_HELD); // Held
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_LEFT); // Cleared Left
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_RIGHT); // Cleared Right
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_VITAL_LOGIC_STATE_CLEARED_BOTH); // Cleared Both
}

void _register_as_board_usage_left_events(openlcb_node_t *_node, node_id_t board_left, node_id_t board_right) {

    event_id_t _event_id_base;

    if (board_left) {
        
        _event_id_base = board_left << 16;
        
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED); // Mainline Occupied
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED); // Mainline Unoccupied
        
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL); // Turnout Command Normal
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING); // Turnout Command Diverging
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN); // Button Normal Closed
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED); // Button Normal Open
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN); // Button Diverging Closed
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED); // Button Diverging Open
        
      
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_RED); // Signal A Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_YELLOW); // Signal A Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_GREEN); // Signal A Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_DARK); // Signal A Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_RED); // Signal B Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_YELLOW); // Signal B Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_GREEN); // Signal B Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_DARK); // Signal B Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_RED); // Signal C Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_YELLOW); // Signal C Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_GREEN); // Signal C Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_DARK); // Signal C Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_RED); // Signal D Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_YELLOW); // Signal D Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_GREEN); // Signal D Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_DARK); // Signal D Dark
    }

    // LH Defined Node specific Producers
    _event_id_base = _node->id << 16;
    
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_OCCUPIED); // Turnout Occupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_TURNOUT_LEFT_UNOCCUPIED); // Turnout UnOccupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED); // Center Main Occupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED); // Center Main UnOccupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED); // Center Siding Occupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED); // Center Siding UnOccupied

    _register_common_local_board_events(_node);
}

void _register_as_board_usage_right_events(openlcb_node_t *_node, node_id_t board_left, node_id_t board_right) {

    event_id_t _event_id_base;

    if (board_left) {

        _event_id_base = board_left << 16;
        

        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED); // Center Main Occupied
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED); // Center Unoccupied
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED); // Center Siding Occupied
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED); // Center Siding Unoccupied
        
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_NORMAL); // Turnout Command Normal
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_COMMAND_DIVERGING); // Turnout Command Diverging
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_OPEN); // Button Normal Closed
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_NORMAL_CLOSED); // Button Normal Open
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_OPEN); // Button Diverging Closed
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_TURNOUT_BUTTON_DIVERGING_CLOSED); // Button Diverging Open
        
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_RED); // Signal A Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_YELLOW); // Signal A Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_GREEN); // Signal A Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_A_DARK); // Signal A Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_RED); // Signal B Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_YELLOW); // Signal B Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_GREEN); // Signal B Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_B_DARK); // Signal B Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_RED); // Signal C Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_YELLOW); // Signal C Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_GREEN); // Signal C Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_C_DARK); // Signal C Dark
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_RED); // Signal D Red
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_YELLOW); // Signal D Yellow
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_GREEN); // Signal D Green
        Application_register_consumer_eventid(_node, _event_id_base + EVENT_SUFFIX_SIGNAL_D_DARK); // Signal D Dark
    }

    // RH Defined Node specific Producers
    _event_id_base = _node->id << 16;
    
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED); // Turnout Occupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED); // Turnout UnOccupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_OCCUPIED); // Mainline Occupied
    Application_register_producer_eventid(_node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_TURNOUT_RIGHT_UNOCCUPIED); // Mainline UnOccupied

    _register_common_local_board_events(_node);
}

void TurnoutBoss_Event_Handler_register_events(openlcb_node_t *_node) {

    configuration_memory_buffer_t _eventid_buffer;
    
    node_id_t board_adjacent_left = _extract_board_adjecent_node_id_from_config_mem(_node, &_eventid_buffer, BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS);
    node_id_t board_adjacent_right = _extract_board_adjecent_node_id_from_config_mem(_node, &_eventid_buffer, BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS);
    
    // Clear the events just in case
    Application_clear_consumer_eventids(_node);
    Application_clear_producer_eventids(_node);

    if (Application_read_configuration_memory(_node, BOARD_USAGE_CONFIG_MEM_ADDRESS, 1, &_eventid_buffer) == 1) {
        switch (_eventid_buffer[0]) {
            case 0:
            {
                _register_as_board_usage_left_events(_node, board_adjacent_left, board_adjacent_right);
                break;
            }
            case 1:
            {
                _register_as_board_usage_right_events(_node, board_adjacent_left, board_adjacent_right);
                break;
            }
        }
    }
}
