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
 * \file turnoutboss_hardware_handler.c
 *
 * Scans the TurnoutBoss Hardware States and updates the TurnoutBoss_Signaling_States
 *
 * @author Jim Kueneman
 * @date 19 Jan 2025
 */

#include "xc.h"
#include <libpic30.h> // delay32
#include <string.h>
#include "stdio.h"  // printf

#include "turnoutboss_hardware_handler.h"


#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif

#include "turnoutboss_types.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#include "turnoutboss_board_configuration.h"
#include "turnoutboss_drivers.h"


#define INPUT_FILTER_COUNT 10
#define INPUT_FILTER_COUNT_MIDPOINT 5


#define INPUT_OCCUPANCY1 0
#define INPUT_OCCUPANCY2 1
#define INPUT_OCCUPANCY3 2
#define INPUT_TURNOUT_FEEDBACK_NORMAL 3
#define INPUT_TURNOUT_FEEDBACK_DIVERGING 4
#define INPUT_TURNOUT_PUSHBUTTON_NORMAL 5
#define INPUT_TURNOUT_PUSHBUTTON_DIVERGING 6
#define INPUT_LEARN_BUTTON 7
#define INPUT_TEACH_BUTTON 8

#define INPUT_COUNT 9

typedef struct {
    uint8_olcb_t counter;
    int filter_array[INPUT_COUNT];

} hardware_filter_t;


hardware_filter_t _hardware_filter;

void _run_filter(void) {

    if (OCCUPANCY_DETECT_1_PIN) {

        _hardware_filter.filter_array[INPUT_OCCUPANCY1] = _hardware_filter.filter_array[INPUT_OCCUPANCY1] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_OCCUPANCY1] = _hardware_filter.filter_array[INPUT_OCCUPANCY1] - 1;

    }

    if (OCCUPANCY_DETECT_2_PIN) {

        _hardware_filter.filter_array[INPUT_OCCUPANCY2] = _hardware_filter.filter_array[INPUT_OCCUPANCY2] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_OCCUPANCY2] = _hardware_filter.filter_array[INPUT_OCCUPANCY2] - 1;

    }

    if (OCCUPANCY_DETECT_3_PIN) {

        _hardware_filter.filter_array[INPUT_OCCUPANCY3] = _hardware_filter.filter_array[INPUT_OCCUPANCY3] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_OCCUPANCY3] = _hardware_filter.filter_array[INPUT_OCCUPANCY3] - 1;

    }

    if (TURNOUT_POSITION_NORMAL_PIN) {

        _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_NORMAL] = _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_NORMAL] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_NORMAL] = _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_NORMAL] - 1;

    }


    if (TURNOUT_POSITION_DIVERGING_PIN) {

        _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_DIVERGING] = _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_DIVERGING] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_DIVERGING] = _hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_DIVERGING] - 1;

    }

    if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

        _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_NORMAL] = _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_NORMAL] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_NORMAL] = _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_NORMAL] - 1;

    }


    if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

        _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_DIVERGING] = _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_DIVERGING] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_DIVERGING] = _hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_DIVERGING] - 1;

    }


    if (LEARN_BUTTON_PIN) {

        _hardware_filter.filter_array[INPUT_LEARN_BUTTON] = _hardware_filter.filter_array[INPUT_LEARN_BUTTON] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_LEARN_BUTTON] = _hardware_filter.filter_array[INPUT_LEARN_BUTTON] - 1;

    }
    
    if (TEACH_BUTTON_PIN) {

        _hardware_filter.filter_array[INPUT_TEACH_BUTTON] = _hardware_filter.filter_array[INPUT_TEACH_BUTTON] + 1;

    } else {

        _hardware_filter.filter_array[INPUT_TEACH_BUTTON] = _hardware_filter.filter_array[INPUT_TEACH_BUTTON] - 1;

    }

    _hardware_filter.counter = _hardware_filter.counter + 1;

}

void _update_hardware_state(signaling_state_t *signal_calculation_states) {

    if (_hardware_filter.filter_array[INPUT_OCCUPANCY1] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.occupany_1 = UNOCCUPIED;

    } else {

        signal_calculation_states->next.hardware.occupany_1 = OCCUPIED;

    }

    if (_hardware_filter.filter_array[INPUT_OCCUPANCY2] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.occupany_2 = UNOCCUPIED;

    } else {

        signal_calculation_states->next.hardware.occupany_2 = OCCUPIED;

    }


    if (_hardware_filter.filter_array[INPUT_OCCUPANCY3] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.occupany_3 = UNOCCUPIED;

    } else {

        signal_calculation_states->next.hardware.occupany_3 = OCCUPIED;

    }

    if (_hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_NORMAL] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.turnout_feedback_normal = INACTIVE;

    } else {

        signal_calculation_states->next.hardware.turnout_feedback_normal = ACTIVE;

    }


    if (_hardware_filter.filter_array[INPUT_TURNOUT_FEEDBACK_DIVERGING] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.turnout_feedback_diverging = INACTIVE;

    } else {

        signal_calculation_states->next.hardware.turnout_feedback_diverging = ACTIVE;

    }


    if (_hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_NORMAL] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.turnout_pushbutton_normal = INACTIVE;

    } else {

        signal_calculation_states->next.hardware.turnout_pushbutton_normal = ACTIVE;

    }


    if (_hardware_filter.filter_array[INPUT_TURNOUT_PUSHBUTTON_DIVERGING] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->next.hardware.turnout_pushbutton_diverging = INACTIVE;

    } else {

        signal_calculation_states->next.hardware.turnout_pushbutton_diverging = ACTIVE;

    }


    if (_hardware_filter.filter_array[INPUT_LEARN_BUTTON] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->learn_button_toggled = signal_calculation_states->next.hardware.learn_pin != OPEN;
        signal_calculation_states->next.hardware.learn_pin = OPEN;

    } else {

        signal_calculation_states->learn_button_toggled = signal_calculation_states->next.hardware.learn_pin != OPEN;
        signal_calculation_states->next.hardware.learn_pin = CLOSED;

    }

    if (_hardware_filter.filter_array[INPUT_TEACH_BUTTON] < INPUT_FILTER_COUNT_MIDPOINT) {

        signal_calculation_states->teach_button_toggled = signal_calculation_states->next.hardware.teach_pin != OPEN;
        signal_calculation_states->next.hardware.teach_pin = OPEN;

    } else {

        signal_calculation_states->teach_button_toggled = signal_calculation_states->next.hardware.teach_pin != CLOSED;
        signal_calculation_states->next.hardware.teach_pin = CLOSED;

    }
    
}

void _reset_filter_bank(void) {

    // Reset and start again
    _hardware_filter.counter = 0;

    for (int i = 0; i < INPUT_COUNT; i++) {

        _hardware_filter.filter_array[i] = INPUT_FILTER_COUNT_MIDPOINT;

    }

}

void TurnoutBossHardwareHandler_initialize(void) {

    _reset_filter_bank();
    
}

void TurnoutBossHardwareHandler_scan_for_changes(signaling_state_t *signal_calculation_states) {


    if (_hardware_filter.counter < INPUT_FILTER_COUNT) {

        _run_filter();

    } else {

        _update_hardware_state(signal_calculation_states);
        _reset_filter_bank();

    }

}

uint8_olcb_t _calculate_yellow_led(uint8_olcb_t signal, uint8_olcb_t bi_directional) {

    // Yellow may be an alternating Green/Red LED

    uint8_olcb_t result;

    if (bi_directional) {

        if (signal != 0b00000010) { // if the green is not on turn it on

            result = 0b00000010; // turn on the red  

        } else {

            result = 0b00000100; // turn on the green      

        }

    } else {

        result = 0b00000001; // Just turn-on the yellow led   

    }

    return result;

}

void TurnoutBossHardwareHandler_update_signal_lamps(signaling_state_t* signal_calculation_states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    // Called from the T1 time incase yellow is a bi-directional LED and we need to toggle it to make yellow out of green/red

    uint8_olcb_t signal_a, signal_b, signal_c, signal_d;

    if (board_configuration->board_location == BL) {

        switch (signal_calculation_states->lamps.SaBL) {

            case DARK:
                signal_a = 0b00000000;
                break;
            case GREEN:
                signal_a = 0b00000100;
                break;
            case YELLOW:
                signal_a = _calculate_yellow_led(signal_calculation_states->leds.signal_a, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_a = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.SbBL) {
            case DARK:
                signal_b = 0b00000000;
                break;
            case GREEN:
                signal_b = 0b00000100;
                break;
            case YELLOW:
                signal_b = _calculate_yellow_led(signal_calculation_states->leds.signal_b, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_b = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.ScBL) {
            case DARK:
                signal_c = 0b00000000;
                break;
            case GREEN:
                signal_c = 0b00000100;
                break;
            case YELLOW:
                signal_c = _calculate_yellow_led(signal_calculation_states->leds.signal_c, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_c = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.SdBL) {
            case DARK:
                signal_d = 0b00000000;
                break;
            case GREEN:
                signal_d = 0b00000100;
                break;
            case YELLOW:
                signal_d = _calculate_yellow_led(signal_calculation_states->leds.signal_d, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_d = 0b00000010;
                break;
        }

    } else {

        switch (signal_calculation_states->lamps.SaBR) {
            case DARK:
                signal_a = 0b00000000;
                break;
            case GREEN:
                signal_a = 0b00000100;
                break;
            case YELLOW:
                signal_a = _calculate_yellow_led(signal_calculation_states->leds.signal_a, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_a = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.SbBR) {
            case DARK:
                signal_b = 0b00000000;
                break;
            case GREEN:
                signal_b = 0b00000100;
                break;
            case YELLOW:
                signal_b = _calculate_yellow_led(signal_calculation_states->leds.signal_b, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_b = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.ScBR) {
            case DARK:
                signal_c = 0b00000000;
                break;
            case GREEN:
                signal_c = 0b00000100;
                break;
            case YELLOW:
                signal_c = _calculate_yellow_led(signal_calculation_states->leds.signal_c, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_c = 0b00000010;
                break;
        }

        switch (signal_calculation_states->lamps.SdBR) {
            case DARK:
                signal_d = 0b00000000;
                break;
            case GREEN:
                signal_d = 0b00000100;
                break;
            case YELLOW:
                signal_d = _calculate_yellow_led(signal_calculation_states->leds.signal_d, (board_configuration->led_polarity == BiDirectionalYellow));
                break;
            case RED:
                signal_d = 0b00000010;
                break;
        }

    }


    if (board_configuration->led_polarity == CommonCathode) {

        signal_a = ~signal_a;
        signal_b = ~signal_b;
        signal_c = ~signal_c;
        signal_d = ~signal_d;

    }

    if ((signal_calculation_states->leds.signal_a != signal_a) || (signal_calculation_states->leds.signal_b != signal_b) || (signal_calculation_states->leds.signal_c != signal_c) || (signal_calculation_states->leds.signal_d != signal_d)) {

        MCP23S17Driver_set_signals(signal_a, signal_b, signal_c, signal_d); // 0b00000RGY

        signal_calculation_states->leds.signal_a = signal_a;
        signal_calculation_states->leds.signal_b = signal_b;
        signal_calculation_states->leds.signal_c = signal_c;
        signal_calculation_states->leds.signal_d = signal_d;

    }

}

void TurnoutBossHardwareHandler_validate_config_mem(void) {

    configuration_memory_buffer_t buffer;

    printf("Address 0x000 in EEPROM: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));

    if (_25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS) == 0xFF) {

        for (int i = 0; i < EEPROM_PAGE_SIZE_IN_BYTES; i++)
            buffer[i] = 0x00; // Default value


        for (int i = 0; i < EEPROM_SIZE_IN_BYTES / EEPROM_PAGE_SIZE_IN_BYTES; i++)
            TurnoutBossDrivers_config_mem_write((uint32_olcb_t) (i * EEPROM_PAGE_SIZE_IN_BYTES), EEPROM_PAGE_SIZE_IN_BYTES, & buffer);

        buffer[0] = 31;
        TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_1_GAIN, 1, &buffer);
        TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_2_GAIN, 1, &buffer);
        TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_DETECTOR_3_GAIN, 1, &buffer);
        TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_SIGNAL_LED_BRIGHTNESS_GAIN, 1, &buffer);

    }

}