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
 * \uart_handler.c
 *
 * Handles the UART receive code to process what is asked.
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include <libpic30.h>

#include "uart_handler.h"

#include "xc.h"
#include "stdio.h"  // printf
#include "../../../openlcb/openlcb_defines.h"
#include "../../../drivers/common/can_buffer_store.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "debug.h"
#include "../dsPIC_Common/ecan1_helper.h"

#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#include "local_drivers/_MCP4014/MCP4014_driver.h"
#include "turnoutboss_drivers.h"

uint8_olcb_t signalA = 0x00; // All off
uint8_olcb_t signalB = 0x00;
uint8_olcb_t signalC = 0x00;
uint8_olcb_t signalD = 0x00;

uint8_olcb_t detector_gain = 0;

uint8_olcb_t track_detector_to_led = 1;

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

            MCP4014Driver_set_gain(0, 0, 0);

            return;

        case 'Z':
        case 'z':

            printf("Setting the Detector Gain\n");

            MCP4014Driver_set_gain(63, 64, 64);

            return;

        case '2':

            if (detector_gain + 1 < 64)
                detector_gain = detector_gain + 1;

            printf("Increasing the Detector Gain: %d\n", detector_gain);

            MCP4014Driver_set_gain(detector_gain, detector_gain, detector_gain);

            return;

        case '1':

            if (detector_gain - 1 >= 0)
                detector_gain = detector_gain - 1;

            printf("Decreasing the Detector Gain: %d\n", detector_gain);

            MCP4014Driver_set_gain(detector_gain, detector_gain, detector_gain);

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

            if (signalA == 0) {
                signalA = 0b111;
                signalB = 0b111;
                signalC = 0b111;
                signalD = 0b111;
            } else {
                signalA = 0b000;
                signalB = 0b000;
                signalC = 0b000;
                signalD = 0b000;
            }

            MCP23S17Driver_set_signals(signalA, signalB, signalC, signalD);

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

            TURNOUT_DRIVER = !TURNOUT_DRIVER;

            return;

    }

    return;


}
