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


// DSPIC33EP64GP502 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <libpic30.h>

#include "xc.h"
#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"

#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/openlcb_utilities.h"

#include "debug.h"
#include "uart_handler.h"
#include "turnoutboss_drivers.h"
#include "node_parameters.h"
#include "turnoutboss_event_handler.h"
#include "turnoutboss_hardware_handler.h"
#include "turnoutboss_signal_calculations.h"
#include "turnoutboss_event_engine.h"
#include "turnoutboss_board_configuration.h"
#include "turnoutboss_types.h"
#include "../dsPIC_Common/ecan1_helper.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"



uint64_olcb_t node_id_base = 0x0507010100AA;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}



// #define  _SIMULATOR_

int button_latch = 0;

board_configuration_t _turnoutboss_board_configuration;

signaling_state_t _turnoutboss_signal_calculation_states;
signaling_state_t _turnoutboss_signal_calculation_states_next;

hardware_input_states_t _turnoutboss_signal_calculation_hardware_states;
hardware_input_states_t _turnoutboss_signal_calculation_hardware_states_next;

send_event_engine_t _turnoutboss_event_engine;

int main(void) {

    memset(&_turnoutboss_board_configuration, 0x00, sizeof ( _turnoutboss_board_configuration));
    memset(&_turnoutboss_signal_calculation_states, 0x00, sizeof ( _turnoutboss_signal_calculation_states));
    memset(&_turnoutboss_signal_calculation_states_next, 0x00, sizeof ( _turnoutboss_signal_calculation_states_next));
    memset(&_turnoutboss_signal_calculation_hardware_states, 0x00, sizeof ( _turnoutboss_signal_calculation_hardware_states));
    memset(&_turnoutboss_signal_calculation_hardware_states_next, 0x00, sizeof ( _turnoutboss_signal_calculation_hardware_states_next));
    memset(&_turnoutboss_event_engine, 0x00, sizeof ( _turnoutboss_event_engine));


    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;


#ifdef _SIMULATOR_

    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

#else

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

    Application_Callbacks_set_alias_change(&_alias_change_callback);


#endif

    printf("\nBooted\n");
    openlcb_node_t* node = Node_allocate(node_id_base, &NodeParameters_main_node);
    printf("Node Created\n");

    // Read in the configuration memory for how the user has the board configured and setup a callback so new changes to the board configuration are captured
    TurnoutBossBoardConfiguration_initialize(
            node,
            &_turnoutboss_board_configuration
            );

    // Initialize Calculation data structures
    TurnoutBossSignalCalculations_initialize(
            &_turnoutboss_signal_calculation_states,
            &_turnoutboss_signal_calculation_states_next
            );

    // Setup the event engine so when states change any outgoing events can be flags to send
    TurnoutBossEventEngine_initialize(
            &_turnoutboss_event_engine
            );

    // Build the dynamic events and the callback to handle incoming events
    TurnoutBossEventHandler_initialize(
            node,
            &_turnoutboss_board_configuration,
            &_turnoutboss_signal_calculation_states,
            &_turnoutboss_event_engine
            );


#ifdef _SIMULATOR_


#endif


    while (1) {



        switch (track_detector_to_led) {
            case 1:

                LED = OCCUPANCY_DETECT_1_PIN;
                break;

            case 2:

                LED = OCCUPANCY_DETECT_2_PIN;
                break;

            case 3:

                LED = OCCUPANCY_DETECT_3_PIN;
                break;
        }



        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

        TurnoutBossHardwareHandler_scan_for_changes(&_turnoutboss_signal_calculation_hardware_states_next);
        
        // Note that that turnoutboss_event_handler.c is capturing PCER events in the background and updating
        //  _turnoutboss_signal_calculation_states_next where necessary

        if (_turnoutboss_board_configuration.board_location == BL) {

            TurnoutBossSignalCalculations_run_board_left(
                    &_turnoutboss_signal_calculation_states,
                    &_turnoutboss_signal_calculation_states_next,
                    &_turnoutboss_signal_calculation_hardware_states,
                    &_turnoutboss_signal_calculation_hardware_states_next,
                    &_turnoutboss_event_engine,
                    &_turnoutboss_board_configuration
                    );

        } else {

            TurnoutBossSignalCalculations_run_board_right(
                    &_turnoutboss_signal_calculation_states,
                    &_turnoutboss_signal_calculation_states_next,
                    &_turnoutboss_signal_calculation_hardware_states,
                    &_turnoutboss_signal_calculation_hardware_states_next,
                    &_turnoutboss_event_engine,
                    &_turnoutboss_board_configuration
                    );

        }

        TurnoutBossEventEngine_run(
                node,
                &_turnoutboss_event_engine);

    }

}
