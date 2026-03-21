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
 * \file callbacks_can.cpp
 *
 * CAN bus callback implementations for the BasicNode application.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#define PRINT_RX_TX_MESSAGE

#include "callbacks_can.h"

#include "Arduino.h"

#include "../openlcb_c_lib/openlcb/openlcb_gridconnect.h"

void CallbacksCan_on_rx(can_msg_t *can_msg) {

#ifdef PRINT_RX_TX_MESSAGE
    gridconnect_buffer_t gridconnect;

    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);

    unsigned int i = 0;
    Serial.print("Rx: ");
    while ((gridconnect[i] != 0x00) && (i < sizeof(gridconnect_buffer_t))) {

        Serial.print((char)gridconnect[i]);
        i++;
    }
    Serial.println();

#endif

    // TODO: Turn on Rx LED
}

void CallbacksCan_on_tx(can_msg_t *can_msg) {

#ifdef PRINT_RX_TX_MESSAGE
    gridconnect_buffer_t gridconnect;

    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);

    unsigned int i = 0;
    Serial.print("Tx: ");
    while ((gridconnect[i] != 0x00) && (i < sizeof(gridconnect_buffer_t))) {

        Serial.print((char)gridconnect[i]);
        i++;
    }
    Serial.println();

#endif

    // TODO: Turn on Tx LED
}

void CallbacksCan_on_alias_change(uint16_t new_alias, node_id_t node_id) {

    // Print out the allocated Alias
    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%06llX\n\n", node_id);
}
