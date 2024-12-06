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
 * Test application to verify the library code
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */



#include "stdio.h"  // printf

#include "../../../drivers/common/can_buffer_store.h"
#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../openlcb/openlcb_buffer_store.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "node_parameters.h"
#include "../../../openlcb/callback_hooks.h"

uint64_t node_id_base = 0x050101010700;

void _uart_callback(uint16_t code) {

    switch (code) {
        case 'B':
        case 'b':

           

            return;

        case 'P':
        case 'p':

            if (openlcb_helper.active_msg)
             

            return;

        case 'C':
        case 'c':

            if (can_helper.active_msg) {

            
                
                return;

            }
        case 'N':
        case 'n':

            
            return;
            
        case 'L':
        case 'l':

            node_id_base++;
            Node_allocate(node_id_base, &NodeParameters_main_node);
            
            return;
            
        case 'H':
        case 'h':
            
            printf("B - Print Buffer Storage state\n");
            printf("P - Print the active message in the CanHelper\n");
            printf("C - Print the active message in the OpenLcbHelper\n");
            printf("N - Print the state of the first allocated Node\n");
            printf("L - Allocate a new Node\n");
            
            return;

    }

    return;

}

void _alias_change_callback(uint16_t new_alias, uint64_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
   
    printf("\n");

}

int main(void) {

    McuDriver_uart_rx_callback_func = &_uart_callback;
    CallbackHooks_alias_change = &_alias_change_callback;

    CanMainStatemachine_initialize();
    MainStatemachine_initialize(); 
    McuDriver_initialization();

    printf("\n\nBooted\n");

    openlcb_node_t* node = Node_allocate(node_id_base, &NodeParameters_main_node);

    printf("Node Created\n");

    while (1) {

        CanMainStateMachine_run(); // Runnning a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();)


    }

}
