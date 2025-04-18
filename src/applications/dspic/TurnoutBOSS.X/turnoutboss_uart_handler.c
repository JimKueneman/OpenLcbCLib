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
 * \turnoutboss_uart_handler.c
 *
 * Handles the UART receive code to process what is asked.
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include <libpic30.h>

#include "turnoutboss_uart_handler.h"

#include "xc.h"
#include "stdio.h"  // printf
#include "../../../openlcb/openlcb_defines.h"
#include "../../../drivers/common/can_buffer_store.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../TurnoutBossCommon/common_debug_helper.h"
#include "../dsPIC_Common/ecan1_helper.h"

#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#include "local_drivers/_MCP4014/MCP4014_driver.h"
#include "turnoutboss_types.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#include "turnoutboss_drivers.h"

uint8_olcb_t UartHandler_pause_calculations = FALSE;
uint8_olcb_t _signal_a = 0x00; // All off
uint8_olcb_t _signal_b = 0x00;
uint8_olcb_t _signal_c = 0x00;
uint8_olcb_t _signal_d = 0x00;
uint8_olcb_t detector_gain = 0;

board_configuration_t *UartHandler_board_configuration;
signaling_state_t *UartHandler_signal_calculation_states;

void UartHandler_handle_rx(uint16_olcb_t code) {

    T3CONbits.TON = 0; // Turn off Timer  don't count these dumps in the timing 

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers: %d\n", CanBufferStore_messages_allocated());
            printf("Buffers: %d\n", BufferStore_messages_allocated());
            printf("Max Can Buffers: %d\n", CanBufferStore_messages_max_allocated());
            printf("Max Buffers: %d\n", BufferStore_messages_max_allocated());
            printf("Max CAN FIFO depth: %d\n", Ecan1Helper_get_max_can_fifo_depth());

            return;

        case 'P':
        case 'p':

            if (openlcb_helper.active_msg)
                PrintOpenLcbMsg(openlcb_helper.active_msg);

            return;

        case 'C':
        case 'c':

            if (can_helper.active_msg) {

                PrintCanMsg(can_helper.active_msg);
                printf("\n");
                PrintCanFrameIdentifierName(can_helper.active_msg->identifier);

                return;

            }
        case 'N':
        case 'n':

            if (Node_get_first(0))
                PrintNode(Node_get_first(0));

            return;

        case 'H':
        case 'h':

            printf("B - Print Buffer Storage state\n");
            printf("P - Print the active message in the CanHelper\n");
            printf("C - Print the active message in the OpenLcbHelper\n");
            printf("N - Print the state of the first allocated Node\n");
            printf("G - Set Detector Gain to 0\n");
            printf("Z - Set Detector Gain to 64\n");
            printf("1 - Decrease Detector Gain by 1\n");
            printf("2 - Increase Detector Gain by 1\n");
            printf("S - Cycle through the signal LED lights (Use 'Y' to pause the calculations)\n");
            printf("I - Reinitialize the MCP23S17\n");
            printf("R - Reset the MCP23S17\n");
            printf("M - Toggle the Turnout Driver (Use 'Y' to pause the calculations)\n");
            printf("L - List out all of the TurnoutBoss internal Signal States\n");
            printf("E - Erase the EEPROM\n");
            printf("Y - Pause the internal calculations to allow overriding\n");
            printf("Q - List the current state of the inputs\n");
            printf("T - Print out the maximum timing to complete the OpenlcbCLib loop and the Application Loop\n");
            printf("O - Reset the maximum timing counters\n");
            printf("U - Ignore/Measure Configuration Memory read/writes in the maximum timing results\n");

            return;

        case 'Q':
        case 'q':

            printf("Turnout Feedback Normal: %d\n", UartHandler_signal_calculation_states->hardware.turnout_feedback_normal);
            printf("Turnout Feedback Diverging: %d\n", UartHandler_signal_calculation_states->hardware.turnout_feedback_diverging);
            printf("Turnout Pushbutton Normal: %d\n", UartHandler_signal_calculation_states->hardware.turnout_pushbutton_normal);
            printf("Turnout Pushbutton Diverging: %d\n", UartHandler_signal_calculation_states->hardware.turnout_pushbutton_diverging);
            printf("Occupancy 1: %d\n", UartHandler_signal_calculation_states->hardware.occupany_1);
            printf("Occupancy 2: %d\n", UartHandler_signal_calculation_states->hardware.occupany_2);
            printf("Occupancy 3: %d\n", UartHandler_signal_calculation_states->hardware.occupany_3);

#ifdef BOSS2
            printf("Teach: %d\n", UartHandler_signal_calculation_states->hardware.teach_pin);
            printf("Learn: %d\n", UartHandler_signal_calculation_states->hardware.learn_pin);
#endif           
            
#ifdef BOSS3
            printf("Teach: %d\n", UartHandler_signal_calculation_states->hardware.teach_pin);
            printf("Learn: %d\n", UartHandler_signal_calculation_states->hardware.learn_pin);
#endif
            return;

        case 'Y':
        case 'y':

            if (UartHandler_pause_calculations) {

                UartHandler_pause_calculations = FALSE;
                printf("Internal Calculations Enabled....\n");

            } else {

                UartHandler_pause_calculations = TRUE;
                printf("Internal Calculations Disabled....\n");

            }

            return;
        case 'G':
        case 'g':

            printf("Setting the Detector Gain\n");

            MCP4014Driver_set_gain(0, 0, 0, 0);

            return;

        case 'Z':
        case 'z':

            printf("Setting the Detector Gain\n");

            MCP4014Driver_set_gain(63, 64, 64, 64);

            return;

        case '2':

            if (detector_gain + 1 < 64)
                detector_gain = detector_gain + 1;

            printf("Increasing the Detector Gain: %d\n", detector_gain);

            MCP4014Driver_set_gain(detector_gain, detector_gain, detector_gain, detector_gain);

            return;

        case '1':

            if (detector_gain - 1 >= 0)
                detector_gain = detector_gain - 1;

            printf("Decreasing the Detector Gain: %d\n", detector_gain);

            MCP4014Driver_set_gain(detector_gain, detector_gain, detector_gain, detector_gain);

            return;

        case 'S':
        case 's':

            printf("Setting the Signals\n");

            if (_signal_a == 0b111) {
                _signal_a = 0b000;
                _signal_b = 0b000;
                _signal_c = 0b000;
                _signal_d = 0b000;

            } else {

                _signal_a = _signal_a + 1;
                _signal_b = _signal_b + 1;
                _signal_c = _signal_c + 1;
                _signal_d = _signal_d + 1;
            }

            printf("A = %d, B = %d, C = %d, D = %d\n", _signal_a, _signal_b, _signal_c, _signal_d);


            MCP23S17Driver_set_signals(_signal_a, _signal_b, _signal_c, _signal_d);

            return;

        case 'I':
        case 'i':

            printf("Reinitializing the MCP23S17\n");
            MCP23S17Driver_initialize();

            return;

        case 'R':
        case 'r':

            printf("Resetting the MCP23S17\n");
            _MCP23S17_RESET = 0;
            __delay32(100); // 1us min setup and hold
            _MCP23S17_RESET = 1;

            return;

        case 'M':
        case 'm':

            if (TURNOUT_DRIVER_PIN) {

                TURNOUT_DRIVER_PIN = FALSE;
                printf("TURNOUT_DRIVER_PIN = FALSE\n");

            } else {

                TURNOUT_DRIVER_PIN = TRUE;
                printf("TURNOUT_DRIVER_PIN = TRUE\n");

            }

            return;

        case 'l':
        case 'L':

            if (UartHandler_signal_calculation_states->hardware.occupany_1 == OCCUPIED) {
                printf("Track 1 (BL = OTL, BR = OTR) is OCCUPIED\n");
            } else {
                printf("Track 1 (BL = OTL, BR = OTR) is UNOCCUPIED\n");
            }
            if (UartHandler_signal_calculation_states->hardware.occupany_2 == OCCUPIED) {
                printf("Track 2 (BL = OMC, BR = OMR) is OCCUPIED\n");
            } else {
                printf("Track 2 (BL = OMC, BR = OMR)  is UNOCCUPIED\n");
            }
            if (UartHandler_signal_calculation_states->hardware.occupany_3 == OCCUPIED) {
                printf("Track 3 (BL = OSC, BR = unused) is OCCUPIED\n");
            } else {
                printf("Track 3 (BL = OSC, BR = unused) is UNOCCUPIED\n");
            }

            if (UartHandler_signal_calculation_states->hardware.turnout_pushbutton_normal == CLOSED) {
                printf("Pushbutton Normal is CLOSED\n");
            } else {
                printf("Pushbutton Normal is OPEN\n");
            }
            if (UartHandler_signal_calculation_states->hardware.turnout_pushbutton_diverging == CLOSED) {
                printf("Pushbutton Diverging is CLOSED\n");
            } else {
                printf("Pushbutton Diverging is OPEN\n");
            }

            if (UartHandler_signal_calculation_states->hardware.turnout_feedback_normal == ACTIVE) {
                printf("Turnout Feedback Normal is ACTIVE\n");
            } else {
                printf("Turnout Feedback Normal is INACTIVE\n");
            }
            if (UartHandler_signal_calculation_states->hardware.turnout_feedback_diverging == ACTIVE) {
                printf("Turnout Feedback Diverging is ACTIVE\n");
            } else {
                printf("Turnout Feedback Diverging is INACTIVE\n");
            }

            if (UartHandler_board_configuration->board_location == BL) {
                if (UartHandler_signal_calculation_states->turnout.TLC == ACTIVE) {
                    printf("Turnout Control = NORMAL\n");
                } else {
                    printf("Turnout Control = DIVERGING\n");
                }
            } else {
                if (UartHandler_signal_calculation_states->turnout.TRC == ACTIVE) {
                    printf("Turnout Control = NORMAL\n");
                } else {
                    printf("Turnout Control = DIVERGING\n");
                }
            }

            if (UartHandler_board_configuration->board_location == BL) {

                switch (UartHandler_signal_calculation_states->lamps.SaBL) {

                    case RED:
                        printf("Signal A = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal A = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal A = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal A = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.SbBL) {

                    case RED:
                        printf("Signal B = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal B = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal B = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal B = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.ScBL) {

                    case RED:
                        printf("Signal C = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal C = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal C = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal C = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.SdBL) {

                    case RED:
                        printf("Signal D = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal D = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal D = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal D = DARK\n");
                        break;
                }
            } else {
                switch (UartHandler_signal_calculation_states->lamps.SaBR) {

                    case RED:
                        printf("Signal A = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal A = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal A = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal A = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.SbBR) {

                    case RED:
                        printf("Signal B = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal B = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal B = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal B = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.ScBR) {

                    case RED:
                        printf("Signal C = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal C = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal C = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal C = DARK\n");
                        break;
                }

                switch (UartHandler_signal_calculation_states->lamps.SdBR) {

                    case RED:
                        printf("Signal D = RED\n");
                        break;
                    case YELLOW:
                        printf("Signal D = YELLOW\n");
                        break;
                    case GREEN:
                        printf("Signal D = GREEN\n");
                        break;
                    case DARK:
                        printf("Signal D = DARK\n");
                        break;
                }
            }

            if (UartHandler_board_configuration->board_location == BL) {

                if (UartHandler_signal_calculation_states->stop.SaBLstop == ACTIVE) {
                    printf("Signal A at STOP\n");
                } else {
                    printf("Signal A at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.SbBLstop == ACTIVE) {
                    printf("Signal B at STOP\n");
                } else {
                    printf("Signal B at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.ScdBLstop == ACTIVE) {
                    printf("Signal CD at STOP\n");
                } else {
                    printf("Signal CD at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.ScdBALstop == ACTIVE) {
                    printf("Signal BAL CD at STOP\n");
                } else {
                    printf("Signal BAL CD at NONSTOP\n");
                }
            } else {

                if (UartHandler_signal_calculation_states->stop.SaBRstop == ACTIVE) {
                    printf("Signal A at STOP\n");
                } else {
                    printf("Signal A at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.SbBRstop == ACTIVE) {
                    printf("Signal B at STOP\n");
                } else {
                    printf("Signal B at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.ScdBRstop == ACTIVE) {
                    printf("Signal CD at STOP\n");
                } else {
                    printf("Signal CD at NONSTOP\n");
                }

                if (UartHandler_signal_calculation_states->stop.ScdBARstop == ACTIVE) {
                    printf("Signal BAR CD at STOP\n");
                } else {
                    printf("Signal BAR CD at NONSTOP\n");
                }
            }

            break;

        case 'E':
        case 'e':

            // The 25AA08 does not have an erase instruction

            printf("Erasing EEPROM\n");

            uint8_olcb_t buffer[EEPROM_PAGE_SIZE_IN_BYTES];

            printf("Address 0x000 in EEPROM before: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));

            for (int i = 0; i < EEPROM_PAGE_SIZE_IN_BYTES; i++)
                buffer[i] = 0xFF;


            for (uint16_olcb_t i = 0; i < EEPROM_SIZE_IN_BYTES / EEPROM_PAGE_SIZE_IN_BYTES; i++)
                TurnoutBossDrivers_config_mem_write((uint32_olcb_t) (i * EEPROM_PAGE_SIZE_IN_BYTES), EEPROM_PAGE_SIZE_IN_BYTES, (configuration_memory_buffer_t*) & buffer);

            printf("Address 0x000 in EEPROM after: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));
            printf("Erased:\n");

            break;

        case 'T':
        case 't':

            printf("Max Application Delay Detected: %.2f us\n", (double) (CommonLoaderApp_max_application_loop_timer * 1.6));
            printf("Max OpenlcbCLib Delay Detected: %.2f us\n", (double) (CommonLoaderApp_max_openlcb_c_lib_loop_timer * 1.6));

            break;

        case 'O':
        case 'o':

            printf("Reseting the timing counters....\n");
            CommonLoaderApp_max_application_loop_timer = 0;
            CommonLoaderApp_max_openlcb_c_lib_loop_timer = 0;

            break;

        case 'U':
        case 'u':

            printf("Reseting the timing counters....\n");

            if (CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes) {

                printf("Timing counters are including the config memory writes....\n");

                CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes = FALSE;

            } else {

                printf("Timing counters are not including the config memory writes....\n");

                CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes = TRUE;

            }



            break;



    }

    T3CONbits.TON = 1; // Turn on Timer 

    return;

}

