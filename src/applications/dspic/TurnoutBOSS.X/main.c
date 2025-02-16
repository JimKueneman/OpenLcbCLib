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
#include "debug.h"
#include "turnoutboss_uart_handler.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#include "turnoutboss_traps.h"
#include "common_loader_app.h"

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

extern int main(void);

// This creates an array of pointers to the handlers for the different interrupts that are at a known
// place in the program space (AppStartAddress++ResetVectorSize).  We defined the program start at AppStartAddress
// so that is a jump call to the start of the initialization.
__prog__ const uint16_olcb_t __attribute__((space(prog), address((APPLICATION_START_ADDRESS + RESET_INSTRUCTION_SIZE)))) _VirtualIVT[9] = {

    (uint16_olcb_t) & Traps_oscillator_fail_handler, // 0x0004
    (uint16_olcb_t) & Traps_address_error_handler, // 0x0006
    (uint16_olcb_t) & Traps_stack_error_handler, // 0x0008
    (uint16_olcb_t) & Traps_math_error_handler, // 0x000A
    (uint16_olcb_t) & Traps_dmac_error_handler, // 0x000C
    (uint16_olcb_t) & TurnoutBossDrivers_t2_interrupt_handler, // 0x000E
    (uint16_olcb_t) & TurnoutBossDrivers_u1_rx_interrupt_handler, // 0x0010
    (uint16_olcb_t) & TurnoutBossDrivers_u1_tx_interrupt_handler, // 0x0012
    (uint16_olcb_t) & Ecan1Helper_C1_interrupt_handler, // 0x0014

};

board_configuration_t _board_configuration;
signaling_state_t _signal_calculation_states;
send_event_engine_t _event_engine;

uint64_olcb_t node_id_base = 0x0507010100AA;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

int main(void) {

    memset(&_board_configuration, 0x00, sizeof ( _board_configuration));
    memset(&_signal_calculation_states, 0x00, sizeof ( _signal_calculation_states));
    memset(&_event_engine, 0x00, sizeof ( _event_engine));


    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;

    uint8_olcb_t signal_a, signal_b, signal_c, signal_d = 0;
    uint8_olcb_t last_signal_a, last_signal_b, last_signal_c, last_signal_d = 0;

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


    TurnoutBossDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);
    UartHandler_board_configuration = &_board_configuration;
    UartHandler_signal_calculation_states = &_signal_calculation_states;

    Application_Callbacks_set_alias_change(&_alias_change_callback);


    printf("\nApplication Booted\n");
    openlcb_node_t* node = Node_allocate(node_id_base, &NodeParameters_main_node);
    printf("Node Created\n");

    // Need to check CommonLoaderApp_node_id and CommonLoaderApp_node_alias to see if the bootloader has already logged us in

    printf("Bootloaders Openlcb credentials:\n");
    PrintNodeID(CommonLoaderApp_node_id);
    PrintAlias(CommonLoaderApp_node_alias);

    if ((CommonLoaderApp_node_id != NULL_NODE_ID) && (CommonLoaderApp_node_alias != 0)) {
        
        printf("Using the bootloaders credentials");
        node->alias = CommonLoaderApp_node_alias;
        node->id = CommonLoaderApp_node_id;
        
    }
    
    // Read in the configuration memory for how the user has the board configured and setup a callback so new changes to the board configuration are captured
    TurnoutBossBoardConfiguration_initialize(node, &_board_configuration);

    // Initialize Calculation data structures
    if (_board_configuration.board_location == BL) {
        TurnoutBossSignalCalculationsBoardLeft_initialize(&_signal_calculation_states);
    } else {
        TurnoutBossSignalCalculationsBoardRight_initialize(&_signal_calculation_states);
    }

    // Setup the event engine so when states change any outgoing events can be flags to send
    TurnoutBossEventEngine_initialize(&_event_engine);

    // Build the dynamic events and the callback to handle incoming events
    TurnoutBossEventHandler_initialize(node, &_board_configuration, &_signal_calculation_states, &_event_engine);

    // Lets rock
    _GIE = TRUE;
   
    while (1) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

        if (TurnoutBossEventEngine_is_flushed(&_event_engine)) {

            TurnoutBossHardwareHandler_scan_for_changes(&_signal_calculation_states);

            if (_board_configuration.board_location == BL) {

                TurnoutBossSignalCalculationsBoardLeft_run(&_signal_calculation_states, &_board_configuration, &_event_engine);

            } else {

                TurnoutBossSignalCalculationsBoardRight_run(&_signal_calculation_states, &_board_configuration, &_event_engine);

            }

            switch (_signal_calculation_states.lamps.SaBL) {
                case DARK:
                    signal_a = 0b00000000;
                    break;
                case GREEN:
                    signal_a = 0b00000010;
                    break;
                case YELLOW:
                    signal_a = 0b00000001;
                    break;
                case RED:
                    signal_a = 0b00000100;
                    break;
            }

            switch (_signal_calculation_states.lamps.SbBL) {
                case DARK:
                    signal_b = 0b00000000;
                    break;
                case GREEN:
                    signal_b = 0b00000010;
                    break;
                case YELLOW:
                    signal_b = 0b00000001;
                    break;
                case RED:
                    signal_b = 0b00000100;
                    break;
            }

            switch (_signal_calculation_states.lamps.ScBL) {
                case DARK:
                    signal_c = 0b00000000;
                    break;
                case GREEN:
                    signal_c = 0b00000010;
                    break;
                case YELLOW:
                    signal_c = 0b00000001;
                    break;
                case RED:
                    signal_c = 0b00000100;
                    break;
            }

            switch (_signal_calculation_states.lamps.SdBL) {
                case DARK:
                    signal_d = 0b00000000;
                    break;
                case GREEN:
                    signal_d = 0b00000010;
                    break;
                case YELLOW:
                    signal_d = 0b00000001;
                    break;
                case RED:
                    signal_d = 0b00000100;
                    break;
            }

            if ((last_signal_a != signal_a) || (last_signal_b != signal_b) || (last_signal_c != signal_c) || (last_signal_d != signal_d)) {

                MCP23S17Driver_set_signals(signal_a, signal_b, signal_c, signal_d);

                last_signal_a = signal_a;
                last_signal_b = signal_b;
                last_signal_c = signal_c;
                last_signal_d = signal_d;

            }

            if (_board_configuration.board_location == BL)
                TURNOUT_DRIVER_PIN = _signal_calculation_states.turnout.TLC;
            else
                TURNOUT_DRIVER_PIN = _signal_calculation_states.turnout.TRC;

        }

        TurnoutBossEventEngine_run(node, &_event_engine);

    }

}
