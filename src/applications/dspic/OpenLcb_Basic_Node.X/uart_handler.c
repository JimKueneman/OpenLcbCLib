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

#include "xc.h"
#include "stdio.h"  // printf

#include "../../../drivers/common/can_types.h"
#include "../../../drivers/common/can_utilities.h"
#include "../../../drivers/common/can_buffer_store.h"
#include "../../../drivers/common/can_buffer_fifo.h"
#include "../../../drivers/common/can_login_message_handler.h"
#include "../../../drivers/common/can_login_statemachine.h"
#include "../../../drivers/common/can_rx_message_handler.h"
#include "../../../drivers/common/can_rx_statemachine.h"
#include "../../../drivers/common/can_tx_message_handler.h"
#include "../../../drivers/common/can_tx_statemachine.h"
#include "../../../drivers/common/can_main_statemachine_handler.h"
#include "../../../drivers/common/can_main_statemachine.h"

#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../openlcb/openlcb_buffer_list.h"
#include "../../../openlcb/openlcb_buffer_fifo.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/protocol_message_network.h"
#include "../../../openlcb/protocol_event_transport.h"
#include "../../../openlcb/protocol_snip.h"
#include "../../../openlcb/openlcb_main_statemachine.h"

#include "../dsPIC_Common/ecan1_helper.h"
#include "common_debug_helper.h"


void UartHandler_handle_rx(uint16_t code) {

    T3CONbits.TON = 0; // Turn off Timer  don't count these dumps in the timing 

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers Allocated: %d\n", CanBufferStore_messages_allocated());
            printf("Basic Buffers Allocated: %d\n", OpenLcbBufferStore_basic_messages_allocated());
            printf("SNIP Buffers Allocated: %d\n", OpenLcbBufferStore_snip_messages_allocated());
            printf("Datagram Buffers Allocated: %d\n", OpenLcbBufferStore_datagram_messages_allocated());
            printf("Stream Buffers Allocated: %d\n", OpenLcbBufferStore_stream_messages_allocated());
            
            printf("Max Can Buffers: %d\n", CanBufferStore_messages_max_allocated());
            printf("Max Basic Buffers Allocated: %d\n", OpenLcbBufferStore_basic_messages_max_allocated());
            printf("Max SNIP Buffers Allocated: %d\n", OpenLcbBufferStore_snip_messages_max_allocated());
            printf("Max Datagram Buffers Allocated: %d\n", OpenLcbBufferStore_datagram_messages_max_allocated());
            printf("Max Stream Buffers Allocated: %d\n", OpenLcbBufferStore_stream_messages_max_allocated());
            
            printf("Max CAN FIFO depth: %d\n", Ecan1Helper_get_max_can_fifo_depth());

            return;

        case 'N':
        case 'n':

            if (OpenLcbNode_get_first(2))
                PrintNode(OpenLcbNode_get_first(2));

            return;

        case 'H':
        case 'h':

            printf("B - Print Buffer Storage state\n");     
            printf("N - Print the state of the first allocated Node\n");

            return;

    }

    T3CONbits.TON = 1; // Turn on Timer 

    return;

}

