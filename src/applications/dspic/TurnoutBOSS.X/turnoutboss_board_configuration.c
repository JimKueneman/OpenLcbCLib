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
 * \file turnoutboss_board_configuration.c
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and 
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 22 Jan 2025
 */


#include "turnoutboss_board_configuration.h"

#include "local_drivers/_MCP4014/MCP4014_driver.h"

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/openlcb_tx_driver.h"
#else
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application_callbacks.h"
#include "src/openlcb/openlcb_tx_driver.h"
#endif

#define BOARD_LOCATION_CONFIG_MEM_ADDRESS 0x7F // Single Byte, 0 = LB; 1 = RB
#define BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS 0x80  // 8 Bytes for an Event ID
#define BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS 0x88 // 8 Bytes for an Event ID
#define BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS 0x90  // uses 2 pushbutton for normal/diverging or using one to toggle, 0 = two buttons; 1 = single button toggle
#define BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS 0x91 // uses both turnout feedbacks or just a single one, 0 = not used; 1 = single feedback sensors; 2 = two feedback sensor
#define DETECTOR_A_GAIN_ADDRESS 0x180
#define DETECTOR_B_GAIN_ADDRESS 0x181
#define DETECTOR_C_GAIN_ADDRESS 0x182


//#define PRINT_DEBUG

node_id_t TurnoutBossBoardConfiguration_board_to_the_left = NULL_NODE_ID;
node_id_t TurnoutBossBoardConfiguration_board_to_the_right = NULL_NODE_ID;
board_type_enum_t TurnoutBossBoardConfiguration_board_location = BL;
pushbutton_type_enum_t TurnoutBossBoardConfiguration_pushbutton_type = dualPushbuttons; // 2 pushbuttons
turnout_feedback_type_enum_t TurnoutBossBoardConfiguration_feedback_type = singleTurnoutFeedback; // 

uint8_olcb_t _detector_gain_a = 0;
uint8_olcb_t _detector_gain_b = 0;
uint8_olcb_t _detector_gain_c = 0;


void _config_mem_write_callback(uint32_olcb_t address, uint8_olcb_t data_count, configuration_memory_buffer_t* config_mem_buffer) {

    switch (address) {

        case BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS:

#ifdef PRINT_DEBUG
            printf("TurnoutBoss_Signaling_States_board_to_the_left\n");
#endif
            TurnoutBossBoardConfiguration_board_to_the_left = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS:

#ifdef PRINT_DEBUG
            printf("TurnoutBoss_Signaling_States_board_to_the_right\n");
#endif         
            TurnoutBossBoardConfiguration_board_to_the_right = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case BOARD_LOCATION_CONFIG_MEM_ADDRESS:

            if (*config_mem_buffer[0] == 1) {
#ifdef PRINT_DEBUG
                printf("BOARD_LOCATION_CONFIG_MEM_ADDRESS: BR\n");
#endif
                TurnoutBossBoardConfiguration_board_location = BR;

            } else {
#ifdef PRINT_DEBUG
                printf("BOARD_LOCATION_CONFIG_MEM_ADDRESS: BL\n");
#endif
                TurnoutBossBoardConfiguration_board_location = BL;

            }

            return;

        case BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS:


            if (*config_mem_buffer[0] == 1) {
#ifdef PRINT_DEBUG
                printf("BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS: single\n");
#endif
                TurnoutBossBoardConfiguration_pushbutton_type = singlePushbutton;

            } else {
#ifdef PRINT_DEBUG
                printf("BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS: dual\n");
#endif
                TurnoutBossBoardConfiguration_pushbutton_type = dualPushbuttons;

            }

            return;

        case BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS:

            switch (*config_mem_buffer[0]) {

                case 1:
#ifdef PRINT_DEBUG                    
                    printf("BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS: single\n");
#endif
                    TurnoutBossBoardConfiguration_feedback_type = singleTurnoutFeedback;

                    return;
                case 2:
#ifdef PRINT_DEBUG
                    printf("BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS: dual\n");
#endif
                    TurnoutBossBoardConfiguration_feedback_type = dualTurnoutFeedback;

                    return;
                default:
#ifdef PRINT_DEBUG
                    printf("BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS: unused\n");
#endif
                    TurnoutBossBoardConfiguration_feedback_type = unusedTurnoutFeedback;

                    return;

            }

        case DETECTOR_A_GAIN_ADDRESS:

            _detector_gain_a = *config_mem_buffer[0];
            MCP4014Driver_set_gain(_detector_gain_a, _detector_gain_b, _detector_gain_c);
#ifdef PRINT_DEBUG
            printf("DETECTOR_A_GAIN_ADDRESS: %d\n", _detector_gain_a);
#endif

            return;

        case DETECTOR_B_GAIN_ADDRESS:

            _detector_gain_b = *config_mem_buffer[0];
            MCP4014Driver_set_gain(_detector_gain_a, _detector_gain_b, _detector_gain_c);
#ifdef PRINT_DEBUG
            printf("DETECTOR_B_GAIN_ADDRESS: %d\n", _detector_gain_b);
#endif

            return;

        case DETECTOR_C_GAIN_ADDRESS:

            _detector_gain_c = *config_mem_buffer[0];
            MCP4014Driver_set_gain(_detector_gain_a, _detector_gain_b, _detector_gain_c);
#ifdef PRINT_DEBUG
            printf("DETECTOR_C_GAIN_ADDRESS: %d\n", _detector_gain_c);
#endif

            return;


    }

}

pushbutton_type_enum_t _extract_pushbutton_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    pushbutton_type_enum_t result = dualPushbuttons;

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        if (*config_mem_buffer[0] == 1)

            result = singlePushbutton;

    }

    return result;

}

board_type_enum_t _extract_boardtype_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    board_type_enum_t result = BL;

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        if (*config_mem_buffer[0] == 1)

            result = BR;

    }

    return result;

}

turnout_feedback_type_enum_t _extract_turnoutfeedback_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    turnout_feedback_type_enum_t result = singleTurnoutFeedback;

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        if (*config_mem_buffer[0] == 1)

            result = dualTurnoutFeedback;

    }

    return result;

}

node_id_t _extract_event_id_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

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

uint8_olcb_t _extract_detector_gain_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {
    
    uint8_olcb_t result;
    
    result = 0xFF;
    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1)
        result = *config_mem_buffer[0];
    if (result == 0xFF)
        result = 31;
    
    return result;
    
}


void TurnoutBoss_Board_Configuration_initialize(openlcb_node_t *node) {
    
    Application_Callbacks_set_config_mem_write(&_config_mem_write_callback);

    configuration_memory_buffer_t config_mem_buffer;

    TurnoutBossBoardConfiguration_board_to_the_left = _extract_event_id_from_config_mem(node, BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    TurnoutBossBoardConfiguration_board_to_the_right = _extract_event_id_from_config_mem(node, BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS, &config_mem_buffer);

    TurnoutBossBoardConfiguration_board_location = _extract_boardtype_from_config_mem(node, BOARD_LOCATION_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    TurnoutBossBoardConfiguration_pushbutton_type = _extract_pushbutton_type_from_config_mem(node, BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    TurnoutBossBoardConfiguration_feedback_type = _extract_turnoutfeedback_type_from_config_mem(node, BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS, &config_mem_buffer);

    _detector_gain_a = _extract_detector_gain_from_config_mem(node, DETECTOR_A_GAIN_ADDRESS, &config_mem_buffer);
    _detector_gain_b = _extract_detector_gain_from_config_mem(node, DETECTOR_B_GAIN_ADDRESS, &config_mem_buffer);
    _detector_gain_c = _extract_detector_gain_from_config_mem(node, DETECTOR_C_GAIN_ADDRESS, &config_mem_buffer);
    MCP4014Driver_set_gain(_detector_gain_a, _detector_gain_b, _detector_gain_c);

}