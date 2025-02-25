/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * \file main.c
 *
 * TurnoutBOSS main
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include <libpic30.h>

#include "xc.h"
#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"

#if defined(MPLAB)

#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "../dsPIC_Common/ecan1_helper.h"
#include "turnoutboss_drivers.h"
#include "../TurnoutBossCommon/common_debug_helper.h"
#include "turnoutboss_uart_handler.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#include "turnoutboss_traps.h"
#include "../TurnoutBossCommon/common_loader_app.h"

#else 

#include "src/drivers/common/can_main_statemachine.h"
#include "src/drivers/common/../driver_mcu.h"
#include "src/drivers/driver_can.h"
#include "src/openlcb/openlcb_main_statemachine.h"
#include "src/openlcb/openlcb_node.h"
#include "src/openlcb/application_callbacks.h"
#include "src/openlcb/application.h"
#include "src/openlcb/openlcb_utilities.h"

#endif

#include "turnoutboss_node_parameters.h"
#include "turnoutboss_event_handler.h"
#include "turnoutboss_hardware_handler.h"
#include "turnoutboss_signal_calculations_board_left.h"
#include "turnoutboss_signal_calculations_board_right.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_board_configuration.h"
#include "turnoutboss_types.h"
#include "local_drivers/_25AA1024/25AA1024_driver.h"


// This creates an array of pointers to the handlers for the different interrupts that are at a known
// place in the program space (AppStartAddress++ResetVectorSize).  We defined the program start at AppStartAddress
// so that is a jump call to the start of the initialization.
//__prog__ const uint16_olcb_t __attribute__((space(prog), address((APPLICATION_START_ADDRESS + RESET_INSTRUCTION_SIZE)))) _VirtualIVT[9] = {
//
//    (uint16_olcb_t) & Traps_oscillator_fail_handler, // APPLICATION_START_ADDRESS + 0x0004
//    (uint16_olcb_t) & Traps_address_error_handler, // APPLICATION_START_ADDRESS + // 0x0006
//    (uint16_olcb_t) & Traps_stack_error_handler, // APPLICATION_START_ADDRESS + // 0x0008
//    (uint16_olcb_t) & Traps_math_error_handler, // APPLICATION_START_ADDRESS + 0x000A
//    (uint16_olcb_t) & Traps_dmac_error_handler, // APPLICATION_START_ADDRESS + 0x000C
//    (uint16_olcb_t) & TurnoutBossDrivers_t2_interrupt_handler, // APPLICATION_START_ADDRESS + 0x000E
//    (uint16_olcb_t) & TurnoutBossDrivers_u1_rx_interrupt_handler, // APPLICATION_START_ADDRESS + 0x0010
//    (uint16_olcb_t) & TurnoutBossDrivers_u1_tx_interrupt_handler, // APPLICATION_START_ADDRESS + 0x0012
//    (uint16_olcb_t) & Ecan1Helper_C1_interrupt_handler, // APPLICATION_START_ADDRESS + 0x0014
//
//};


board_configuration_t _board_configuration;
signaling_state_t _signal_calculation_states;
send_event_engine_t _event_engine;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

void _config_memory_freeze_firmware_update_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    CommonLoaderApp_bootloader_state.do_start = TRUE;

}

void _initialize_bootloader_state(void) {

    // The only time POR and/or BOR is set is with a true start from 0V so it is guaranteed to be the first boot.
    // if we jumped here from the bootloader it will have cleared these bits
    if (RCONbits.POR || RCONbits.BOR) {

        // the code is being used stand alone without a bootloader so initialize the states
        memset(&CommonLoaderApp_bootloader_state, 0x0000, sizeof (CommonLoaderApp_bootloader_state));

        // Clear it so the app knows the CommonLoaderApp_bootloader_state is valid
        RCONbits.POR = 0;
        RCONbits.BOR = 0;

    }

    CommonLoaderApp_bootloader_state.do_start = FALSE;

}

node_id_t _extract_node_id(void) {

    if (CommonLoaderApp_bootloader_state.started_from_bootloader) {

        if ((CommonLoaderApp_node_id == 0xFFFFFFFFFFFF) || (CommonLoaderApp_node_id == 0x000000000000)) {

            return NODE_ID_DEFAULT;

        } else {

            return CommonLoaderApp_node_id;

        }

    } else {

        // TODO: Need to add the code to allow a stand alone version read the node id from flash

        return NODE_ID_DEFAULT;

    }

}

uint8_olcb_t _calculate_yellow_led(uint8_olcb_t signal, uint8_olcb_t bi_directional) {

    uint8_olcb_t result;

    if (bi_directional) {

        if (signal != 0b00000010) { // if the green is not on turn it on

            result = 0b00000010; // turn on the green  

        } else {

            result = 0b00000100; // turn on the red      

        }

    } else {

        result = 0b00000001; // Just turn-on the yellow led   

    }

    return result;

}

void _update_signal_lamps(signaling_state_t* signal_calculation_states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    // Called from the T1 time incase yellow is a bi-directional LED and we need to toggle it to make yellow out of green/red

    uint8_olcb_t signal_a, signal_b, signal_c, signal_d;

    switch (signal_calculation_states->lamps.SaBL) {
        case DARK:
            signal_a = 0b00000000;
            break;
        case GREEN:
            signal_a = 0b00000010;
            break;
        case YELLOW:
            signal_a = _calculate_yellow_led(signal_calculation_states->leds.signal_a, (board_configuration->led_polarity == BiDirectionalYellow));
            break;
        case RED:
            signal_a = 0b00000100;
            break;
    }

    switch (signal_calculation_states->lamps.SbBL) {
        case DARK:
            signal_b = 0b00000000;
            break;
        case GREEN:
            signal_b = 0b00000010;
            break;
        case YELLOW:
            signal_b = _calculate_yellow_led(signal_calculation_states->leds.signal_b, (board_configuration->led_polarity == BiDirectionalYellow));
            break;
        case RED:
            signal_b = 0b00000100;
            break;
    }

    switch (signal_calculation_states->lamps.ScBL) {
        case DARK:
            signal_c = 0b00000000;
            break;
        case GREEN:
            signal_c = 0b00000010;
            break;
        case YELLOW:
            signal_c = _calculate_yellow_led(signal_calculation_states->leds.signal_c, (board_configuration->led_polarity == BiDirectionalYellow));
            break;
        case RED:
            signal_c = 0b00000100;
            break;
    }

    switch (signal_calculation_states->lamps.SdBL) {
        case DARK:
            signal_d = 0b00000000;
            break;
        case GREEN:
            signal_d = 0b00000010;
            break;
        case YELLOW:
            signal_d = _calculate_yellow_led(signal_calculation_states->leds.signal_d, (board_configuration->led_polarity == BiDirectionalYellow));
            break;
        case RED:
            signal_d = 0b00000100;
            break;
    }


    if (board_configuration->led_polarity == CommonAnode) {

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

void _build_interrupt_jump_table(void) {

    // After the initialization where we cleared these variables set it up the bootloader jump table
    CommonLoaderApp_jumptable.oscillatorfail_hander = &Traps_oscillator_fail_handler;
    CommonLoaderApp_jumptable.addresserror_hander = &Traps_address_error_handler;
    CommonLoaderApp_jumptable.stackerror_hander = &Traps_stack_error_handler;
    CommonLoaderApp_jumptable.matherror_hander = &Traps_math_error_handler;
    CommonLoaderApp_jumptable.dmacerror_hander = &Traps_dmac_error_handler;
    CommonLoaderApp_jumptable.timer_2_hander = &TurnoutBossDrivers_t2_interrupt_handler;
    CommonLoaderApp_jumptable.u1_rx_hander = &TurnoutBossDrivers_u1_rx_interrupt_handler;
    CommonLoaderApp_jumptable.u1_tx_hander = &TurnoutBossDrivers_u1_tx_interrupt_handler;
    CommonLoaderApp_jumptable.c1_hander = &Ecan1Helper_C1_interrupt_handler;
    CommonLoaderApp_jumptable.timer_1_hander = &TurnoutBossDrivers_t1_interrupt_handler;

}

void _signal_update_timer_1_callback(void) {

    // Timer 1 will be paused when the states are being recalculated (below) and when any configuration memory access occurs (turnoutboss_drivers.c)
    // so the SPI bus will not have a conflict

    _update_signal_lamps(&_signal_calculation_states, &_board_configuration, &_event_engine);

}

void _validate_config_mem(void) {

    uint8_olcb_t buffer[EEPROM_PAGE_SIZE_IN_BYTES];

    printf("Address 0x000 in EEPROM: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));

    if (_25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS) == 0xFF) {
       
        for (int i = 0; i < EEPROM_PAGE_SIZE_IN_BYTES; i++) 
            buffer[i] = 0;


        for (int i = 0; i < EEPROM_SIZE_IN_BYTES / EEPROM_PAGE_SIZE_IN_BYTES; i++) {
            
            _25AA1024_Driver_write_latch_enable();
            _25AA1024_Driver_write(i * EEPROM_PAGE_SIZE_IN_BYTES, EEPROM_PAGE_SIZE_IN_BYTES, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);
            while (_25AA1024_Driver_write_in_progress()) {                      
                 __delay32(1000);  // 25AA08 seems to be sensitive to how fast you check the register... it will lock up  
            }

        }
        
        
        buffer[0] = 31;
        
        _25AA1024_Driver_write_latch_enable();
         _25AA1024_Driver_write(DETECTOR_1_GAIN_ADDRESS, 1, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);
        while (_25AA1024_Driver_write_in_progress()) {
                 __delay32(1000);
            }
        _25AA1024_Driver_write_latch_enable();
         _25AA1024_Driver_write(DETECTOR_2_GAIN_ADDRESS, 1, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);
        while (_25AA1024_Driver_write_in_progress()) {
                 __delay32(1000);
            }
        _25AA1024_Driver_write_latch_enable();
         _25AA1024_Driver_write(DETECTOR_3_GAIN_ADDRESS, 1, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);
        while (_25AA1024_Driver_write_in_progress()) {
                 __delay32(1000);
            }
        _25AA1024_Driver_write_latch_enable();
         _25AA1024_Driver_write(SIGNAL_LED_BRIGHTNESS_GAIN_ADDRESS, 1, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);
        while (_25AA1024_Driver_write_in_progress()) {
                 __delay32(1000);
            }
 
    }

}

openlcb_node_t* _initialize_turnout_boss(void) {

    openlcb_node_t* result = (void*) 0;

    memset(&_board_configuration, 0x00, sizeof ( _board_configuration));
    memset(&_signal_calculation_states, 0x00, sizeof ( _signal_calculation_states));
    memset(&_event_engine, 0x00, sizeof ( _event_engine));

    CanMainStatemachine_initialize(
            &Ecan1Helper_setup,
            &Ecan1Helper_transmit_raw_can_frame,
            &Ecan1Helper_is_can_tx_buffer_clear,
            &Ecan1Helper_pause_can_rx,
            &Ecan1Helper_resume_can_rx
            );
    MainStatemachine_initialize(
            &TurnoutBossDrivers_setup,
            &TurnoutBossDrivers_reboot,
            &TurnoutBossDrivers_config_mem_read,
            &TurnoutBossDrivers_config_mem_write,
            &TurnoutBossDrivers_pause_100ms_timer,
            &TurnoutBossDrivers_resume_100ms_timer
            );

    _build_interrupt_jump_table();

    TurnoutBossDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);
    UartHandler_board_configuration = &_board_configuration;
    UartHandler_signal_calculation_states = &_signal_calculation_states;

    ApplicationCallbacks_set_alias_change(&_alias_change_callback);
    ApplicationCallbacks_set_config_mem_freeze_firmware_update(&_config_memory_freeze_firmware_update_callback);

    TurnoutBossDrivers_set_signal_update_timer_sink(&_signal_update_timer_1_callback);

    // We always boot and reallocate the alias
    result = Node_allocate(_extract_node_id(), &NodeParameters_main_node);


    // Can do this now that the SPI has been setup and need to do it before we try to load the board_configuration.
    _validate_config_mem();

    // Read in the configuration memory for how the user has the board configured and setup a callback so new changes to the board configuration are captured
    TurnoutBossBoardConfiguration_initialize(result, &_board_configuration);

    // Initialize Calculation data structures
    if (_board_configuration.board_location == BL) {
        TurnoutBossSignalCalculationsBoardLeft_initialize(&_signal_calculation_states);
    } else {
        TurnoutBossSignalCalculationsBoardRight_initialize(&_signal_calculation_states);
    }

    // Setup the event engine so when states change any outgoing events can be flagged to send
    TurnoutBossEventEngine_initialize(&_event_engine);

    // Build the dynamic events and the callback to handle incoming events
    TurnoutBossEventHandler_initialize(result, &_board_configuration, &_signal_calculation_states, &_event_engine);

    return result;

}

void _print_turnoutboss_version(void) {

#ifdef BOSS1
    printf("Application Booted: Boss 1.0.................\n");
#endif
#ifdef BOSS2 
    printf("Application Booted: Boss 2.0.................\n");
#endif

}

void _recalculate_states(signaling_state_t* signal_calculation_states, board_configuration_t* board_configuration, send_event_engine_t* event_engine) {

    if (board_configuration->board_location == BL) {

        TurnoutBossSignalCalculationsBoardLeft_run(signal_calculation_states, board_configuration, event_engine);

        TURNOUT_DRIVER_PIN = signal_calculation_states->turnout.TLC;

    } else {

        TurnoutBossSignalCalculationsBoardRight_run(signal_calculation_states, board_configuration, event_engine);

        TURNOUT_DRIVER_PIN = signal_calculation_states->turnout.TRC;

    }

}

int main(void) {

    openlcb_node_t* node;

#ifdef BOSS1
    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;
#endif

#ifdef BOSS2

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    ANSELC = 0x00;


    LED_BLUE_TRIS = 0;
    LED_GREEN_TRIS = 0;
    LED_YELLOW_TRIS = 0;

    LED_BLUE = 1;
    LED_GREEN = 1;
    LED_YELLOW = 1;
#endif


    _initialize_bootloader_state();
    node = _initialize_turnout_boss();
    _print_turnoutboss_version();

    // Point the interrupt table to the application and re-enable the interrupts
    CommonLoaderApp_bootloader_state.interrupt_redirect = TRUE;
    _GIE = 1; // Enable interrupts

    while (!CommonLoaderApp_bootloader_state.do_start) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

        TurnoutBossEventEngine_run(node, &_event_engine);

        if (TurnoutBossEventEngine_is_flushed(&_event_engine)) {

            TurnoutBossHardwareHandler_scan_for_changes(&_signal_calculation_states);

            // Pause the timer so we don't re-calculate the state of the signals and stomp on the signals being set in the Signal Update Timer
            TurnoutBossDrivers_pause_signal_calculation_timer();

            _recalculate_states(&_signal_calculation_states, &_board_configuration, &_event_engine);

            TurnoutBossDrivers_resume_signal_calculation_timer();

        }

    }

    printf("Starting Bootloader.........\n");

    _GIE = 0; // Disable Interrupts

    CommonLoaderApp_node_alias = node->alias;
    CommonLoaderApp_bootloader_state.started_from_bootloader = FALSE;
    CommonLoaderApp_bootloader_state.do_start = FALSE;
    CommonLoaderApp_bootloader_state.started_from_app = TRUE;
    CommonLoaderApp_bootloader_state.interrupt_redirect = FALSE;

    // Create a pointer to a function at the app entry point
    void (*startBootloader)() = (void*) BOOTLOADER_START_ADDRESS;

    startBootloader();

}
