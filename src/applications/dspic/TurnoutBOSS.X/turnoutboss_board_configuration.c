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

#include "turnoutboss_types.h"

#define BOARD_LOCATION_CONFIG_MEM_ADDRESS 0x7F // Single Byte, 0 = LB; 1 = RB
#define BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS 0x80  // 8 Bytes for an Event ID
#define BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS 0x88 // 8 Bytes for an Event ID
#define BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS 0x90  // uses 2 pushbutton for normal/diverging or using one to toggle, 0 = two buttons; 1 = single button toggle
#define BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS 0x91 // uses both turnout feedbacks or just a single one, 0 = not used; 1 = single feedback sensors; 2 = two feedback sensor
#define BOARD_POINT_SIGNALHEAD_TYPE_CONFIG_MEM_ADDRESS 0x100 // do the points use a single or dual signal head 
#define DETECTOR_A_GAIN_ADDRESS 0x180
#define DETECTOR_B_GAIN_ADDRESS 0x181
#define DETECTOR_C_GAIN_ADDRESS 0x182
#define LED_BRIGHTNESS_GAIN_ADDRESS 0x183

board_configuration_t* _board_configuration;

void _set_detector_gains(void) {

    // TODO LED BRIGHTNESS GAIN

    MCP4014Driver_set_gain(_board_configuration->detector_gain_a, _board_configuration->detector_gain_b, _board_configuration->detector_gain_c);

}

void _config_mem_write_callback(uint32_olcb_t address, uint8_olcb_t data_count, configuration_memory_buffer_t* config_mem_buffer) {

    switch (address) {

        case BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS:

            _board_configuration->board_to_the_left = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS:

            _board_configuration->board_to_the_right = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case BOARD_LOCATION_CONFIG_MEM_ADDRESS:

            if (*config_mem_buffer[0] == 1) {

                _board_configuration->board_location = BR;

            } else {

                _board_configuration->board_location = BL;

            }

            return;

        case BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS:

            if (*config_mem_buffer[0] == 1) {

                _board_configuration->pushbutton_type = Pushbutton_Single;

            } else {

                _board_configuration->pushbutton_type = Pushbutton_Dual;

            }

            return;

        case BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS:

            switch (*config_mem_buffer[0]) {

                case 1:

                    _board_configuration->turnout_feedback_type = TurnoutFeedbackSingle;

                    return;
                case 2:

                    _board_configuration->turnout_feedback_type = TurnoutFeedbackDual;

                    return;
                default:

                    _board_configuration->turnout_feedback_type = TurnoutFeedbackUnused;

                    return;

            }

        case DETECTOR_A_GAIN_ADDRESS:

            _board_configuration->detector_gain_a = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case DETECTOR_B_GAIN_ADDRESS:

            _board_configuration->detector_gain_b = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case DETECTOR_C_GAIN_ADDRESS:

            _board_configuration->detector_gain_c = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case LED_BRIGHTNESS_GAIN_ADDRESS:

            _board_configuration->led_brightness_gain = *config_mem_buffer[0];
            _set_detector_gains();

            return;


    }

}

pushbutton_type_enum_t _extract_pushbutton_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    pushbutton_type_enum_t result = Pushbutton_Dual;

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        if (*config_mem_buffer[0] == 1)

            result = Pushbutton_Single;

    }

    return result;

}

board_type_enum_t _extract_boardtype_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        switch (*config_mem_buffer[0]) {

            case 1:
                return BR;

        }

    }

    return BL; // default

}

turnout_feedback_type_enum_t _extract_turnoutfeedback_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        switch (*config_mem_buffer[0]) {

            case 1:
                return TurnoutFeedbackSingle;
            case 2:
                return TurnoutFeedbackDual;
        }

    }

    return TurnoutFeedbackUnused; // default

}

point_signalhead_type_enum_t _extract_point_signalhead_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1) {

        switch (*config_mem_buffer[0]) {

            case 1:
                return PointSignalHeadSingle;

        }

    }

    return PointSignalHeadDual; // default

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

void TurnoutBossBoardConfiguration_initialize(openlcb_node_t *node, board_configuration_t* board_configuration) {

    Application_Callbacks_set_config_mem_write(&_config_mem_write_callback);

    configuration_memory_buffer_t config_mem_buffer;
    _board_configuration = board_configuration;

    _board_configuration->board_location = _extract_boardtype_from_config_mem(node, BOARD_LOCATION_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->board_to_the_left = _extract_event_id_from_config_mem(node, BOARD_ADJACENT_LEFT_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->board_to_the_right = _extract_event_id_from_config_mem(node, BOARD_ADJACENT_RIGHT_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->turnout_feedback_type = _extract_turnoutfeedback_type_from_config_mem(node, BOARD_TURNOUT_FEEDBACK_TYPE_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->point_signalhead_type = _extract_point_signalhead_type_from_config_mem(node, BOARD_POINT_SIGNALHEAD_TYPE_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->pushbutton_type = _extract_pushbutton_type_from_config_mem(node, BOARD_PUSHBUTTON_TYPE_CONFIG_MEM_ADDRESS, &config_mem_buffer);
    _board_configuration->detector_gain_a = _extract_detector_gain_from_config_mem(node, DETECTOR_A_GAIN_ADDRESS, &config_mem_buffer);
    _board_configuration->detector_gain_b = _extract_detector_gain_from_config_mem(node, DETECTOR_B_GAIN_ADDRESS, &config_mem_buffer);
    _board_configuration->detector_gain_c = _extract_detector_gain_from_config_mem(node, DETECTOR_C_GAIN_ADDRESS, &config_mem_buffer);
    _board_configuration->led_brightness_gain = _extract_detector_gain_from_config_mem(node, LED_BRIGHTNESS_GAIN_ADDRESS, &config_mem_buffer);

    _set_detector_gains();

}