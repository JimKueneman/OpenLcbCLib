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
 * \file dependency_injectors.c
 *
 *
 * @author Jim Kueneman
 * @date 31 Dec 2025
 */

 #define PRINT_RX_TX_MESSAGE

#include "callbacks.h"

#include "src/openlcb/openlcb_utilities.h"

static uint16_t _100ms_ticks = 0;

void Callbacks_initialize(void)
{

    // TODO: Initialize any librarys needed in this module
}

void Callbacks_on_100ms_timer_callback(void)
{

    if (_100ms_ticks > 5)
    {

        // Turn off LED

        _100ms_ticks = 0;
    }

    _100ms_ticks++;
}

void Callbacks_on_can_rx_callback(can_msg_t *can_msg)
{
    #ifdef PRINT_RX_TX_MESSAGE
    gridconnect_buffer_t gridconnect;

    // convert message to a Grid Connect String and print
    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);
    printf("[R] %s\n", (char *)&gridconnect);
    #endif

    // TODO: Turn on Rx LED
   
}

void Callbacks_on_can_tx_callback(can_msg_t *can_msg)
{

     #ifdef PRINT_RX_TX_MESSAGE
    gridconnect_buffer_t gridconnect;

    // convert message to a Grid Connect String and print
    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);
    printf("[R] %s\n", (char *)&gridconnect);
    #endif

    // TODO: Turn on Tx LED
}

void Callbacks_alias_change_callback(uint16_t new_alias, node_id_t node_id)
{

    // Print out the allocated Alias
    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%06llX\n\n", node_id);
}

void Callbacks_operations_request_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info)
{

    // TODO: reset passsed node to factory defauts

    printf("Factory Reset: NodeID = 0x%06llX\n", OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0));
}