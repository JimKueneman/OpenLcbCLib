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
#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "turnoutboss_teach_learn.h"

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


board_configuration_t _board_configuration;
signaling_state_t _signal_calculation_states;
send_event_engine_t _event_engine;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

void _config_memory_freeze_firmware_update_callback(openlcb_node_t* openlcb_node, openlcb_msg_t* openlcb_msg, openlcb_msg_t * worker_msg) {

    // User want to drop into booloader mode via the OpenLcb/LCC Firmware Update Protocol
    CommonLoaderApp_bootloader_state.do_start = TRUE;

}

void _signal_update_timer_1_callback(void) {

    // Timer 1 will be paused when the states are being recalculated (below) and when any configuration memory access occurs (turnoutboss_drivers.c)
    // so the SPI bus will not have a conflict

    if (!UartHandler_pause_calculations)
        TurnoutBossHardwareHandler_update_signal_lamps(&_signal_calculation_states, &_board_configuration, &_event_engine);

}

node_id_t _extract_node_id_from_eeprom(uint32_olcb_t config_mem_address, configuration_memory_buffer_t *config_mem_buffer) {

    if (_25AA1024_Driver_read(config_mem_address, 6, config_mem_buffer, EEPROM_ADDRESS_SIZE_IN_BITS) == 6) {

        node_id_t result = Utilities_extract_node_id_from_config_mem_buffer(config_mem_buffer, 0);

        if ((result != NULL_NODE_ID) && (result != 0xFFFFFFFFFFFF)) {

            return result;

        }
    }

    return NODE_ID_DEFAULT;

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

node_id_t _extract_node_id() {

    configuration_memory_buffer_t config_mem_buffer;

    if (CommonLoaderApp_bootloader_state.started_from_bootloader) {

        if ((CommonLoaderApp_node_id == 0xFFFFFFFFFFFF) || (CommonLoaderApp_node_id == 0x000000000000)) {

            return NODE_ID_DEFAULT;

        } else {

            return CommonLoaderApp_node_id;

        }

    } else {

        return _extract_node_id_from_eeprom(NODE_ID_ADDRESS, &config_mem_buffer);

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

void _initialize_callbacks(void) {

    TurnoutBossDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);
    ApplicationCallbacks_set_alias_change(&_alias_change_callback);
    ApplicationCallbacks_set_config_mem_freeze_firmware_update(&_config_memory_freeze_firmware_update_callback);
    TurnoutBossDrivers_set_signal_update_timer_sink(&_signal_update_timer_1_callback);

}

openlcb_node_t* _initialize_turnout_boss(void) {

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

    _initialize_callbacks();

    // We always boot and reallocate the alias
    openlcb_node_t* result = Node_allocate(_extract_node_id(), &NodeParameters_main_node);

    // Can do this now that the SPI has been setup and need to do it before we try to load the board_configuration.
    TurnoutBossHardwareHandler_validate_config_mem();

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

    TurnoutBossTeachLearn_initialize();

    UartHandler_board_configuration = &_board_configuration;
    UartHandler_signal_calculation_states = &_signal_calculation_states;

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

void _handle_learn_event(openlcb_node_t *node) {

    if (_signal_calculation_states.teach_button_toggled) {

        _signal_calculation_states.teach_button_toggled = FALSE;

        Application_send_teach_event(node, node->id);

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

    // Need the timers running for this
    TurnoutBossTeachLearn_check_for_enable();

    while (!CommonLoaderApp_bootloader_state.do_start) {

        // Run the main Openlcb/LCC engine
        CanMainStateMachine_run();

        // Need to wait for the node to log in before doing anything that may try to send and event/message
        if (node->state.initalized) {
            
            if (TurnoutBossTeachLearn_teach_learn_state.state != STATE_TEACH_LEARN_DEACTIVATED) {

                TurnoutBossDrivers_pause_100ms_timer();
                TurnoutBossTeachLearn_run(node);
                TurnoutBossDrivers_resume_100ms_timer();

            } else {

                // Send any events that have been flagged
                TurnoutBossEventEngine_run(node, &_event_engine);

                // Are all the events in the Event Engine sent?
                if (TurnoutBossEventEngine_is_flushed(&_event_engine)) {

                    if (!UartHandler_pause_calculations) {

                        // Scan for any hardware changes (feedback sensors, pushbuttons, etc)
                        TurnoutBossHardwareHandler_scan_for_changes(&_signal_calculation_states);

                        TurnoutBossSignalCalculations_recalculate_states(&_signal_calculation_states, &_board_configuration, &_event_engine);

                    }

                }
                
                _handle_learn_event(node);

                LED_BLUE = OCCUPANCY_DETECT_1_PIN;
                LED_GREEN = OCCUPANCY_DETECT_2_PIN;
                LED_YELLOW = OCCUPANCY_DETECT_3_PIN;

            }

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
