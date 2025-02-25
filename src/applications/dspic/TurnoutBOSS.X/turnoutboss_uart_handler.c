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

uint8_olcb_t signalA = 0x00; // All off
uint8_olcb_t signalB = 0x00;
uint8_olcb_t signalC = 0x00;
uint8_olcb_t signalD = 0x00;

uint8_olcb_t detector_gain = 0;

uint8_olcb_t track_detector_to_led = 1;

board_configuration_t *UartHandler_board_configuration;
signaling_state_t *UartHandler_signal_calculation_states;

void UartHandler_handle_rx(uint16_olcb_t code) {

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers: %d\n", CanBufferStore_messages_allocated());
            printf("\nBuffers: %d\n", BufferStore_messages_allocated());

            printf("\nMax Can Buffers: %d\n", CanBufferStore_messages_max_allocated());
            printf("\nMax Buffers: %d\n", BufferStore_messages_max_allocated());
            printf("\nMax CAN FIFO depth: %d\n", Ecan1Helper_get_max_can_fifo_depth());

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

        case '3':

            track_detector_to_led = track_detector_to_led + 1;

            if (track_detector_to_led > 3)
                track_detector_to_led = 1;

            printf("Detected track %d\n", track_detector_to_led);

            return;

        case 'S':
        case 's':

            printf("Setting the Signals\n");

            if (signalA == 0b111) {
                signalA = 0b000;
                signalB = 0b000;
                signalC = 0b000;
                signalD = 0b000;

            } else {

                signalA = signalA + 1;
                signalB = signalB + 1;
                signalC = signalC + 1;
                signalD = signalD + 1;
            }

            printf("A = %d, B = %d, C = %d, D = %d\n", signalA, signalB, signalC, signalD);


            MCP23S17Driver_set_signals(signalA, signalB, signalC, signalD);

            //     MCP23S17Driver_set_signals(1, 1, 1, 1);

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

            TURNOUT_DRIVER_PIN = !TURNOUT_DRIVER_PIN;

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



            for (int i = 0; i < EEPROM_PAGE_SIZE_IN_BYTES; i++) {

                buffer[i] = 0xFF;

            }

            for (int i = 0; i < EEPROM_SIZE_IN_BYTES / EEPROM_PAGE_SIZE_IN_BYTES; i++) {

                _25AA1024_Driver_write_latch_enable();

                _25AA1024_Driver_write(i * EEPROM_PAGE_SIZE_IN_BYTES, EEPROM_PAGE_SIZE_IN_BYTES, (configuration_memory_buffer_t*) & buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

                while (_25AA1024_Driver_write_in_progress()) {

                    // 25AA08 seems to be sensitive to how fast you check the register... it will lock up

                    __delay32(1000);

                }
   
            }

            printf("Address 0x000 in EEPROM after: %d\n", _25AA1024_Driver_read_byte(0x0000, EEPROM_ADDRESS_SIZE_IN_BITS));
            
            printf("Erased:\n");

            break;
    }

    return;

}
