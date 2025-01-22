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
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory and then registers event IDs required by the board type.
 * It also registers an Event callback and updates the TurnoutBoss_Signaling_States with
 * any received events from nodes of interest (BAL/BAR/BL/BR).
 *
 * @author Jim Kueneman
 * @date 17 Jan 2025
 */

#include <string.h>

#include "turnoutboss_event_handler.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_signaling_states.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#else
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application_callbacks.h"
#endif


board_type_enum_t _board_location = BL;
node_id_t _board_to_the_left = NULL_NODE_ID;
node_id_t _board_to_the_right = NULL_EVENT_ID;

void _handle_event_from_board_adjecent_left_for_bl(uint16_olcb_t suffix) {

    switch (suffix) {
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OML = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OML = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            TurnoutBoss_Signaling_States.stop.ScdBALstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.ScdBALstop = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_to_the_right_for_bl(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            TurnoutBoss_Signaling_States.stop.SaBRstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.SaBRstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {

            TurnoutBoss_Signaling_States.stop.SbBRstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.SbBRstop = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_left_for_rb(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            TurnoutBoss_Signaling_States.stop.SaBLstop = ACTIVE;
            break;


        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.SaBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {
            TurnoutBoss_Signaling_States.stop.SbBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.SbBLstop = INACTIVE;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OMC = ACTIVE;
            break;

        }


        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OMC = INACTIVE;
            break;

        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OSC = ACTIVE;
            break;
        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED:
        {

            TurnoutBoss_Signaling_States.occupancy.OSC = INACTIVE;
            break;

        }
    }

}

void _handle_event_from_board_adjecent_right_for_rb(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            TurnoutBoss_Signaling_States.stop.ScdBARstop = ACTIVE;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            TurnoutBoss_Signaling_States.stop.ScdBARstop = INACTIVE;
            break;

        }

    }

}

void _event_pc_report_callback(openlcb_node_t* node, event_id_t* event_id) {

    // Start the filtering of the events to find one we care about

    node_id_t local_node_id = (node_id_t) (*event_id >> 16);
    uint16_olcb_t event_id_suffix = (uint16_olcb_t) (*event_id);

    if (local_node_id == _board_to_the_left) {

        if (_board_location == BL) {

            _handle_event_from_board_adjecent_left_for_bl(event_id_suffix);

        } else {

            _handle_event_from_board_left_for_rb(event_id_suffix);

        }

    } else if (local_node_id == _board_to_the_right) {

        if (_board_location == BL) {

            _handle_event_from_board_to_the_right_for_bl(event_id_suffix);

        } else {

            _handle_event_from_board_adjecent_right_for_rb(event_id_suffix);


        }

    }

}


void _register_producer(openlcb_node_t *node, event_id_t event, uint8_olcb_t offset, uint8_olcb_t core_signaling_event) {
    
    Application_register_producer_eventid(node, event);
    TurnoutBoss_Event_Engine.events[offset].state.valid_producer = TRUE;
    TurnoutBoss_Event_Engine.events[offset].state.core_signaling = core_signaling_event;
    
}

void _register_consumer(openlcb_node_t *node, event_id_t event, uint8_olcb_t offset, uint8_olcb_t core_signaling_event) {
  
    Application_register_consumer_eventid(node, event);
    TurnoutBoss_Event_Engine.events[offset].state.valid_consumer = TRUE;
    TurnoutBoss_Event_Engine.events[offset].state.core_signaling = core_signaling_event;
    
}

void _board_left_register_core_signaling_events(openlcb_node_t *node, node_id_t board_adjacent_left, node_id_t board_right) {

        
    if (board_adjacent_left) {

        event_id_t _event_id_base_board_adjacent_left = board_adjacent_left << 16;

        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE);
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE);   
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP, OFFSET_EVENT_SIGNAL_STATE_CD_STOP, TRUE);
        _register_consumer(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP, TRUE);
       
    }

    if (board_right) {

        event_id_t _event_id_base_board_right = board_right << 16;

        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE); 
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE);       
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE);     
        _register_consumer(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE);

    }

    // LH Defined Node Only specific Producers
    event_id_t _event_id_base = node->id << 16;

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, TRUE);
 
}

void _board_right_register_core_signaling_events(openlcb_node_t *node, node_id_t board_left, node_id_t board_adjacent_right) {



    if (board_adjacent_right) {

        event_id_t _event_id_base_board_adjacent_right = board_adjacent_right << 16;

        _register_consumer(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP, OFFSET_EVENT_SIGNAL_STATE_CD_STOP, TRUE);
        _register_consumer(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_CD_NONSTOP, TRUE);

    }

    if (board_left) {

        event_id_t _event_id_base_board_left = board_left << 16;

        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_OCCUPIED, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_MAIN_CENTER_UNOCCUPIED, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_OCCUPIED, TRUE);
        _register_consumer(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, OFFSET_EVENT_OCCUPANCY_SIDING_CENTER_UNOCCUPIED, TRUE);

    }

    // RH Defined Node ONLY specific Producers

    event_id_t _event_id_base = node->id << 16;

    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP, OFFSET_EVENT_SIGNAL_STATE_A_STOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_A_NONSTOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP, OFFSET_EVENT_SIGNAL_STATE_B_STOP, TRUE);
    _register_producer(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP, OFFSET_EVENT_SIGNAL_STATE_B_NONSTOP, TRUE);

}

void _board_register_general_events(openlcb_node_t *node) {
    
    
}

void TurnoutBoss_Event_Handler_initialize(openlcb_node_t *node, board_type_enum_t board_location, node_id_t bl, node_id_t br) {

    _board_location = board_location;
    _board_to_the_left = bl;
    _board_to_the_right = br;
    
    // Clear the events just in case
    Application_clear_consumer_eventids(node);
    Application_clear_producer_eventids(node);

    if (_board_location == BL) {

        _board_left_register_core_signaling_events(node, _board_to_the_left, _board_to_the_right);

    } else {

        _board_right_register_core_signaling_events(node, _board_to_the_left, _board_to_the_right);

    }
    
    _board_register_general_events(node);

    Application_Callbacks_set_event_pc_report(&_event_pc_report_callback);

}

