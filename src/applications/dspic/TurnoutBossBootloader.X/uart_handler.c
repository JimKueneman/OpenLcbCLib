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
#include "ecan1_bootloader_helper.h"
#include "debug.h"
#include "local_drivers/_25AA1024/25AA1024_driver.h"

#include "turnoutboss_drivers.h"

void (*startApplication)() = (void*) 0xB000;

void UartHandler_handle_rx(uint16_olcb_t code) {

    uint16_t applicationISRAddress = 0;

    switch (code) {

        case '0':

            _RB8 = 1;
            __delay32(10);

            applicationISRAddress = __builtin_tblrdl(0xB00E); // Where the Timer 2 Interrupt Handler is in the Application

            _RB8 = 0;
            __delay32(10);

            printf("0x%04X\n", applicationISRAddress);

            void (*app_t1_interrupt_func)() = (void*) applicationISRAddress;
            app_t1_interrupt_func();

            _RB8 = 1;
            __delay32(10);

            return;

        case '1':

            _RB8 = 1;
            __delay32(10);

            applicationISRAddress = __builtin_tblrdl(0xB010); // Where the UART RX Interrupt Handler is in the Application

            _RB8 = 0;
            __delay32(10);
            
            printf("0x%04X\n", applicationISRAddress);

            void (*app_u1_rx_interrupt_func)() = (void*) applicationISRAddress;

            app_u1_rx_interrupt_func();

            _RB8 = 1;
            __delay32(10);

            return;

        case '2':

            _RB8 = 1;
            __delay32(10);

            applicationISRAddress = __builtin_tblrdl(0xB012); // Where the UART TX Interrupt Handler is in the Application

            _RB8 = 0;
            __delay32(10);
            
            printf("0x%04X\n", applicationISRAddress);

            void (*app_u1_tx_interrupt_func)() = (void*) applicationISRAddress;
            app_u1_tx_interrupt_func();

            _RB8 = 1;
            __delay32(10);

            return;

        case '3':

            _RB8 = 1;
            __delay32(10);

            applicationISRAddress = __builtin_tblrdl(0xB014); // Where the CAN C1 Interrupt Handler is in the Application

            _RB8 = 0;
            __delay32(10);
            
            printf("0x%04X\n", applicationISRAddress);

            void (*app_c1_tx_interrupt_func)() = (void*) applicationISRAddress;
            app_c1_tx_interrupt_func();

            _RB8 = 1;
            __delay32(10);

            return;



        case 'L':
        case 'l':

            printf("Jumping to App... I hope!");

            // Disable the Interrupt
            _GIE = 0;
            TurnoutBossDrivers_app_running = TRUE;
            startApplication();

            return;
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

    }

    return;


}
