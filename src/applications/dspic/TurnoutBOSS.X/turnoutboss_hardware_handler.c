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



#define INPUT_FILTER_COUNT 10

uint8_olcb_t _run_filter_inc(uint8_olcb_t filter) {

    if (filter >= INPUT_FILTER_COUNT)

        return INPUT_FILTER_COUNT;

    return filter + 1;

}

uint8_olcb_t _run_filter_dec(uint8_olcb_t filter) {

    if (filter == 0)

        return 0;

    return filter - 1;

}

void TurnoutBossHardwareHandler_scan_for_changes(signaling_state_t *signal_calculation_states) {

    // *******************
    uint8_olcb_t filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_1_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.occupany_1 = UNOCCUPIED;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.occupany_1 = OCCUPIED;

    }


    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_2_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.occupany_2 = UNOCCUPIED;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.occupany_2 = OCCUPIED;

    }


    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (OCCUPANCY_DETECT_3_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.occupany_3 = UNOCCUPIED;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.occupany_3 = OCCUPIED;

    }


    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_POSITION_NORMAL_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.turnout_feedback_normal = INACTIVE;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.turnout_feedback_normal = ACTIVE;

    }


    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_POSITION_DIVERGING_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.turnout_feedback_diverging = INACTIVE;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.turnout_feedback_diverging = ACTIVE;

    }

    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_PUSHBUTTON_NORMAL_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.turnout_pushbutton_normal = OPEN;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.turnout_pushbutton_normal = CLOSED;

    }

    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TURNOUT_PUSHBUTTON_DIVERGING_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {

        signal_calculation_states->next.hardware.turnout_pushbutton_diverging = OPEN;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.turnout_pushbutton_diverging = CLOSED;

    }
    
#ifdef BOSS2
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (LEARN_BUTTON_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {
        
        signal_calculation_states->learn_button_toggled = signal_calculation_states->next.hardware.learn_pin != OPEN;

        signal_calculation_states->next.hardware.learn_pin = OPEN;
      
    } else if (filter == INPUT_FILTER_COUNT) {
        
        signal_calculation_states->next.hardware.learn_pin = CLOSED;

    }
    
    // *******************
    filter = INPUT_FILTER_COUNT / 2;

    for (int i = 0; i < INPUT_FILTER_COUNT; i++) {

        if (TEACH_BUTTON_PIN) {

            filter = _run_filter_inc(filter);

        } else

            filter = _run_filter_dec(filter);

    }

    if (filter == 0) {
        
        signal_calculation_states->teach_button_toggled = signal_calculation_states->next.hardware.teach_pin != OPEN;

        signal_calculation_states->next.hardware.teach_pin = OPEN;

    } else if (filter == INPUT_FILTER_COUNT) {

        signal_calculation_states->next.hardware.teach_pin = CLOSED;

    }
    
#endif
    

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

void TurnoutBossHardwareHandler_write_eeprom(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    _25AA1024_Driver_write_latch_enable();

    _25AA1024_Driver_write(address, count, buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

    while (_25AA1024_Driver_write_in_progress()) {

        __delay32(1000); // 25AA08 seems to be sensitive to how fast you check the register... it will lock up  

    }

}

void TurnoutBossHardwareHandler_validate_config_mem(void) {

    configuration_memory_buffer_t buffer;

    printf("Address 0x000 in EEPROM: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));

    if (_25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS) == 0xFF) {

        for (int i = 0; i < EEPROM_PAGE_SIZE_IN_BYTES; i++)
            buffer[i] = 0x00; // Default value


        for (int i = 0; i < EEPROM_SIZE_IN_BYTES / EEPROM_PAGE_SIZE_IN_BYTES; i++)
            TurnoutBossHardwareHandler_write_eeprom((uint32_olcb_t) (i * EEPROM_PAGE_SIZE_IN_BYTES), EEPROM_PAGE_SIZE_IN_BYTES, & buffer);

        buffer[0] = 31;
        TurnoutBossHardwareHandler_write_eeprom(CONFIG_MEM_ADDRESS_DETECTOR_1_GAIN, 1, &buffer);
        TurnoutBossHardwareHandler_write_eeprom(CONFIG_MEM_ADDRESS_DETECTOR_2_GAIN, 1, &buffer);
        TurnoutBossHardwareHandler_write_eeprom(CONFIG_MEM_ADDRESS_DETECTOR_3_GAIN, 1, &buffer);
        TurnoutBossHardwareHandler_write_eeprom(CONFIG_MEM_ADDRESS_SIGNAL_LED_BRIGHTNESS_GAIN, 1, &buffer);

    }

}