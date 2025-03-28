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


#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"


#ifdef MPLAB
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/openlcb_tx_driver.h"
#include "local_drivers/_MCP4014/MCP4014_driver.h"
#include "turnoutboss_drivers.h"
#else
#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/application.h"
#include "src/openlcb/application_callbacks.h"
#include "src/openlcb/openlcb_tx_driver.h"
#endif

#include "turnoutboss_types.h"

// some reserved space for other brightness gains, etc
// The starting location for the event ID map is defined by START_OF_PRODUCER_CONSUMER_MAP in the .h file: 200
// configuration space ends at 632, see .address_space_config_memory.highest_address in node_parameters.c

board_configuration_t* _turnoutboss_board_configuration;


void _set_detector_gains(void) {
#ifdef MPLAB
    // Since we multiplex the port expander don't let the timer update the signal LEDs while we are writing to the LED brightness gain adjustment (use a port expander pin for CS)
    TurnoutBossDrivers_pause_signal_calculation_timer();
    MCP4014Driver_set_gain(_turnoutboss_board_configuration->detector_gain_1, _turnoutboss_board_configuration->detector_gain_2, _turnoutboss_board_configuration->detector_gain_3, _turnoutboss_board_configuration->signal_led_brightness_gain);
    TurnoutBossDrivers_resume_signal_calculation_timer();
#endif
}

//
// Whenever anything is written to the configuration memory we may need to react to it with a different
// state calulation so we hook into the OpenlcbCLib Configuration Memory Write system to see what it being changed
//
void _config_mem_write_callback(uint32_olcb_t address, uint8_olcb_t data_count, configuration_memory_buffer_t* config_mem_buffer) {

    switch (address) {

        case CONFIG_MEM_ADDRESS_BOARD_ADJACENT_LEFT:

            _turnoutboss_board_configuration->board_to_the_left = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case CONFIG_MEM_ADDRESS_BOARD_ADJACENT_RIGHT:

            _turnoutboss_board_configuration->board_to_the_right = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);
            return;

        case CONFIG_MEM_ADDRESS_BOARD_LOCATION:

            if (*config_mem_buffer[0] == 1) {

                _turnoutboss_board_configuration->board_location = BR;

            } else {

                _turnoutboss_board_configuration->board_location = BL;

            }

            return;

        case CONFIG_MEM_ADDRESS_BOARD_PUSHBUTTON_TYPE:

            if (*config_mem_buffer[0] == TURNOUT_CONTROL_ONE_BUTTON) {

                _turnoutboss_board_configuration->pushbutton_type = Pushbutton_Single;

            } else {
                // TURNOUT_CONTROL_TWO_BUTTON case
                _turnoutboss_board_configuration->pushbutton_type = Pushbutton_Dual;

            }

            return;

        case CONFIG_MEM_ADDRESS_BOARD_TURNOUT_FEEDBACK_TYPE:

            switch (*config_mem_buffer[0]) {

                case 1:

                    _turnoutboss_board_configuration->turnout_feedback_type = TurnoutFeedbackSingle;

                    return;
                case 2:

                    _turnoutboss_board_configuration->turnout_feedback_type = TurnoutFeedbackDual;

                    return;
                default:

                    _turnoutboss_board_configuration->turnout_feedback_type = TurnoutFeedbackUnused;

                    return;

            }

        case CONFIG_MEM_ADDRESS_BOARD_POINT_SIGNALHEAD_TYPE:
            
            if (*config_mem_buffer[0] == 1) {
                
                _turnoutboss_board_configuration->point_signalhead_type = PointSignalHeadSingle;
                
            } else {
                
                _turnoutboss_board_configuration->point_signalhead_type = PointSignalHeadDual;
                
            }
            
            return;

        case CONFIG_MEM_ADDRESS_DETECTOR_1_GAIN:

            _turnoutboss_board_configuration->detector_gain_1 = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case CONFIG_MEM_ADDRESS_DETECTOR_2_GAIN:

            _turnoutboss_board_configuration->detector_gain_2 = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case CONFIG_MEM_ADDRESS_DETECTOR_3_GAIN:

            _turnoutboss_board_configuration->detector_gain_3 = *config_mem_buffer[0];
            _set_detector_gains();

            return;

        case CONFIG_MEM_ADDRESS_SIGNAL_LED_BRIGHTNESS_GAIN:
            
            _turnoutboss_board_configuration->signal_led_brightness_gain = *config_mem_buffer[0];
            _set_detector_gains();
            
            return;
            
        case CONFIG_MEM_ADDRESS_LED_POLARITY: 
           
            _turnoutboss_board_configuration->led_polarity = *config_mem_buffer[0];
        
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

signal_led_polarity_enum_t _extract_led_polarity_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    signal_led_polarity_enum_t result;

    result = CommonAnode;
    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1)
        result = (signal_led_polarity_enum_t) *config_mem_buffer[0];
   
    return result;

}

signalhead_type_enum_t _extract_signal_type_from_config_mem(openlcb_node_t *node, uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    signalhead_type_enum_t result;

    result = ThreeLEDOutputs;
    if (Application_read_configuration_memory(node, config_mem_address, 1, config_mem_buffer) == 1)
        result = (signalhead_type_enum_t) *config_mem_buffer[0];
   
    return result;

}

uint16_olcb_t TurnoutBossBoardConfiguration_write_event_id_to_configuration_memory(openlcb_node_t *node, event_id_t event, uint16_olcb_t address) {
    
    configuration_memory_buffer_t buffer;
    
    Application_read_configuration_memory(node, address, 8, &buffer);
    // no need to write if the buffer contains the requested event ID
    event_id_t read_event_id = Utilities_copy_config_mem_buffer_to_event_id(&buffer, 0);
    
    if (event == read_event_id) {
        // skipping write
        return 8;  // as if the write succeeded
    }

    Utilities_copy_event_id_to_config_mem_buffer(&buffer, event, 0);
    return (Application_write_configuration_memory(node, address, 8, &buffer));
       
}

void TurnoutBossBoardConfiguration_reset_to_defaults(openlcb_node_t *node) {
    
 configuration_memory_buffer_t buffer;  
    
 memset(&buffer, 0x00, sizeof(buffer));
 
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_LOCATION, 1, &buffer);
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_ADJACENT_LEFT, 8, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_ADJACENT_RIGHT, 8, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_TURNOUT_FEEDBACK_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_POINT_SIGNALHEAD_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_A_LED_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_B_LED_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_C_LED_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_D_LED_TYPE, 1, &buffer);  
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_LED_POLARITY, 1, &buffer); 
 
 buffer[0] = TURNOUT_CONTROL_ONE_BUTTON;
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_PUSHBUTTON_TYPE, 1, &buffer);  
 
 buffer[0] = 32;
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_1_GAIN, 1, &buffer); 
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_2_GAIN, 1, &buffer); 
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_3_GAIN, 1, &buffer); 
 TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_LED_BRIGHTNESS_GAIN, 1, &buffer); 
 
}

void TurnoutBossBoardConfiguration_initialize(openlcb_node_t *node, board_configuration_t *board_configuration) {

    Application_Callbacks_set_config_mem_write(&_config_mem_write_callback);

    configuration_memory_buffer_t config_mem_buffer;
    
    _turnoutboss_board_configuration = board_configuration;

    _turnoutboss_board_configuration->board_location = _extract_boardtype_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_LOCATION, &config_mem_buffer);
    _turnoutboss_board_configuration->board_to_the_left = _extract_event_id_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_ADJACENT_LEFT, &config_mem_buffer);
    _turnoutboss_board_configuration->board_to_the_right = _extract_event_id_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_ADJACENT_RIGHT, &config_mem_buffer);
    _turnoutboss_board_configuration->turnout_feedback_type = _extract_turnoutfeedback_type_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_TURNOUT_FEEDBACK_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->point_signalhead_type = _extract_point_signalhead_type_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_POINT_SIGNALHEAD_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->pushbutton_type = _extract_pushbutton_type_from_config_mem(node, CONFIG_MEM_ADDRESS_BOARD_PUSHBUTTON_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->signal_a_type = _extract_signal_type_from_config_mem(node, CONFIG_MEM_ADDRESS_SIGNAL_A_LED_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->signal_b_type = _extract_signal_type_from_config_mem(node, CONFIG_MEM_ADDRESS_SIGNAL_B_LED_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->signal_c_type = _extract_signal_type_from_config_mem(node, CONFIG_MEM_ADDRESS_SIGNAL_C_LED_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->signal_d_type = _extract_signal_type_from_config_mem(node, CONFIG_MEM_ADDRESS_SIGNAL_D_LED_TYPE, &config_mem_buffer);
    _turnoutboss_board_configuration->led_polarity = _extract_led_polarity_from_config_mem(node, CONFIG_MEM_ADDRESS_LED_POLARITY, &config_mem_buffer); 
    
    _turnoutboss_board_configuration->detector_gain_1 = _extract_detector_gain_from_config_mem(node, CONFIG_MEM_ADDRESS_DETECTOR_1_GAIN, &config_mem_buffer);
    _turnoutboss_board_configuration->detector_gain_2 = _extract_detector_gain_from_config_mem(node, CONFIG_MEM_ADDRESS_DETECTOR_2_GAIN, &config_mem_buffer);
    _turnoutboss_board_configuration->detector_gain_3 = _extract_detector_gain_from_config_mem(node, CONFIG_MEM_ADDRESS_DETECTOR_3_GAIN, &config_mem_buffer);
    _turnoutboss_board_configuration->signal_led_brightness_gain = _extract_detector_gain_from_config_mem(node, CONFIG_MEM_ADDRESS_SIGNAL_LED_BRIGHTNESS_GAIN, &config_mem_buffer);
    
    _set_detector_gains();

}

