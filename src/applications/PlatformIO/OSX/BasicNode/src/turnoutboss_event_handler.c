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

#include <string.h>

#include "turnoutboss_event_handler.h"

#ifdef PLATFORMIO
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application_callbacks.h"
#else
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#endif

#define BOARD_LOCATION_CONFIG_MEM_ADDRESS 0x7F
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
#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_OCCUPIED 0x0000
#define EVENT_SUFFIX_OCCUPANCY_MAIN_RIGHT_UNOCCUPIED 0x0001

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

#define BOARD_IS_LEFTHAND  0
#define BOARD_IS_RIGHTHAND  1

typedef struct {
    uint8_olcb_t board_adjecent_left_occupancy_main : 1;
    uint8_olcb_t board_adjecent_left_signal_cd_stop : 1;
    uint8_olcb_t board_adjecent_left_signal_a_stop : 1;
    uint8_olcb_t board_adjecent_left_signal_b_stop : 1;
    uint8_olcb_t board_right_signal_a_stop : 1;
    uint8_olcb_t board_right_signal_b_stop : 1;

} board_left_consumer_states_t;

typedef struct {
    board_left_consumer_states_t consumers;

} board_left_states_t;

typedef struct {
    uint8_olcb_t board_left_signal_a_stop : 1;
    uint8_olcb_t board_left_signal_b_stop : 1;
    uint8_olcb_t board_left_occupancy_main : 1;
    uint8_olcb_t board_left_occupancy_siding : 1;
    uint8_olcb_t board_adjecent_right_signal_cd_stop : 1;

} board_right_consumer_states_t;

typedef struct {
    board_right_consumer_states_t consumers;

} board_right_states_t;

node_id_t _board_to_the_left = NULL_NODE_ID;
node_id_t _board_to_the_right = NULL_NODE_ID;
uint8_olcb_t _board_location = BOARD_IS_LEFTHAND;

board_left_states_t _board_left_states;
board_right_states_t _board_right_states;

void _handle_event_from_board_adjecent_left_on_lh_board(uint16_olcb_t suffix) {

    switch (suffix) {
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_OCCUPIED:
        {

            _board_left_states.consumers.board_adjecent_left_occupancy_main = 1;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_LEFT_UNOCCUPIED:
        {

            _board_left_states.consumers.board_adjecent_left_occupancy_main = 0;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            _board_left_states.consumers.board_adjecent_left_signal_cd_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            _board_left_states.consumers.board_adjecent_left_signal_cd_stop = 0;
            break;

        }
    }

}

void _handle_event_from_board_to_the_right_on_lh_board(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            _board_left_states.consumers.board_right_signal_a_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            _board_left_states.consumers.board_right_signal_a_stop = 0;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {

            _board_left_states.consumers.board_right_signal_b_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {

            _board_left_states.consumers.board_right_signal_b_stop = 0;
            break;

        }
    }

}

void _handle_event_from_board_left_on_rh_board(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_A_STOP:
        {

            _board_right_states.consumers.board_left_signal_a_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP:
        {

            _board_right_states.consumers.board_left_signal_a_stop = 0;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_STOP:
        {
            _board_right_states.consumers.board_left_signal_b_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP:
        {
            _board_right_states.consumers.board_left_signal_b_stop = 0;
            break;

        }
        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED:
        {

            _board_right_states.consumers.board_left_occupancy_main = 1;
            break;
        }


        case EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED:
        {

            _board_right_states.consumers.board_left_occupancy_main = 0;
            break;
        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED:
        {

            _board_right_states.consumers.board_left_occupancy_siding = 1;
            break;
        }

        case EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED:
        {

            _board_right_states.consumers.board_left_occupancy_siding = 0;
            break;
        }
    }

}

void _handle_event_from_board_adjecent_right_on_rh_board(uint16_olcb_t suffix) {

    switch (suffix) {

        case EVENT_SUFFIX_SIGNAL_STATE_CD_STOP:
        {

            _board_right_states.consumers.board_adjecent_right_signal_cd_stop = 1;
            break;

        }
        case EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP:
        {

            _board_right_states.consumers.board_adjecent_right_signal_cd_stop = 0;
            break;

        }

    }

}

void _event_pc_report_callback(openlcb_node_t* node, event_id_t* event_id) {

    // Start the filtering of the events to find one we care about

    node_id_t local_node_id = (node_id_t) (*event_id >> 16);
    uint16_olcb_t event_id_suffix = (uint16_olcb_t) (*event_id);

    if (local_node_id == _board_to_the_left) {

        if (_board_location == BOARD_IS_LEFTHAND) {

            _handle_event_from_board_adjecent_left_on_lh_board(event_id_suffix);

        } else {

            _handle_event_from_board_left_on_rh_board(event_id_suffix);

        }

    } else if (local_node_id == _board_to_the_right) {

        if (_board_location == BOARD_IS_LEFTHAND) {

            _handle_event_from_board_to_the_right_on_lh_board(event_id_suffix);

        } else {

            _handle_event_from_board_adjecent_right_on_rh_board(event_id_suffix);


        }

    }

}

uint8_olcb_t _extract_board_position(openlcb_node_t *node, configuration_memory_buffer_t *config_mem_buffer, uint32_olcb_t config_mem_address) {

    uint8_olcb_t result = BOARD_IS_LEFTHAND;

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        result = BOARD_IS_RIGHTHAND;

    }

    return result;

}

node_id_t _extract_board_adjecent_node_id_from_config_mem(openlcb_node_t *node, configuration_memory_buffer_t *config_mem_buffer, uint32_olcb_t config_mem_address) {

    node_id_t board_adjacent = NULL_NODE_ID;

    // Read configuration memory to see if the Adjacent Left Board is available, it so create these Events
    if (Application_read_configuration_memory(node, config_mem_address, LEN_EVENT_ID, config_mem_buffer) == LEN_EVENT_ID) {

        board_adjacent = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);

        if ((board_adjacent > 0x00) && (board_adjacent < 0x1000000000000)) {

            return board_adjacent;
        }
    }

    return NULL_NODE_ID;
}

void _register_as_board_left_events(openlcb_node_t *node, node_id_t board_adjacent_left, node_id_t board_right) {

    if (board_adjacent_left) {

        event_id_t _event_id_base_board_adjacent_left = board_adjacent_left << 16;

        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED);
        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED);
        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP);
        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_left + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP);

    }

    if (board_right) {

        event_id_t _event_id_base_board_right = board_right << 16;

        Application_register_consumer_eventid(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_STOP);
        Application_register_consumer_eventid(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP);
        Application_register_consumer_eventid(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_STOP);
        Application_register_consumer_eventid(node, _event_id_base_board_right + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP);


    }

    // LH Defined Node Only specific Producers
    event_id_t _event_id_base = node->id << 16;

    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED);


}

void _register_as_board_right_events(openlcb_node_t *node, node_id_t board_left, node_id_t board_adjacent_right) {



    if (board_adjacent_right) {

        event_id_t _event_id_base_board_adjacent_right = board_adjacent_right << 16;

        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_NONSTOP);
        Application_register_consumer_eventid(node, _event_id_base_board_adjacent_right + EVENT_SUFFIX_SIGNAL_STATE_CD_STOP);

    }

    if (board_left) {

        event_id_t _event_id_base_board_left = board_left << 16;

        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_STOP);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_STOP);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_OCCUPIED);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_MAIN_CENTER_UNOCCUPIED);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_OCCUPIED);
        Application_register_consumer_eventid(node, _event_id_base_board_left + EVENT_SUFFIX_OCCUPANCY_SIDING_CENTER_UNOCCUPIED);

    }

    // RH Defined Node ONLY specific Producers

    event_id_t _event_id_base = node->id << 16;

    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_STOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_A_NONSTOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_STOP);
    Application_register_producer_eventid(node, _event_id_base + EVENT_SUFFIX_SIGNAL_STATE_B_NONSTOP);

}

void TurnoutBoss_Event_Handler_initialize(openlcb_node_t * node) {

    configuration_memory_buffer_t config_mem_buffer;

    memset(&_board_left_states, 0x00, sizeof (_board_left_states));
    memset(&_board_right_states, 0x00, sizeof (_board_right_states));

    _board_to_the_left = _extract_board_adjecent_node_id_from_config_mem(node, &config_mem_buffer, BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS);
    _board_to_the_right = _extract_board_adjecent_node_id_from_config_mem(node, &config_mem_buffer, BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS);
    _board_location = _extract_board_position(node, &config_mem_buffer, BOARD_LOCATION_CONFIG_MEM_ADDRESS);

    // Clear the events just in case
    Application_clear_consumer_eventids(node);
    Application_clear_producer_eventids(node);

    if (_board_location == BOARD_IS_LEFTHAND) {

        _register_as_board_left_events(node, _board_to_the_left, _board_to_the_right);

    } else {

        _register_as_board_right_events(node, _board_to_the_left, _board_to_the_right);

    }

    Application_Callbacks_set_event_pc_report(&_event_pc_report_callback);

}