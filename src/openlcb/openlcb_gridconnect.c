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
 * \file openlcb_buffer_store.h
 *
 * Implements the core buffers for normal, snip, datagram, and stream length buffers.
 * The FIFO and List buffers are arrays of pointers to these core buffers that are
 * allocated and freed through access.  The CAN Rx and 100ms timer access these buffers
 * so care must be taken to Pause and Resume those calls if the main loop needs to
 * access the buffers.
 *
 * @author Jim Kueneman
 * @date 11 Dec 2024
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <stdlib.h>
#include <string.h>
#include "openlcb_gridconnect.h"
#include "openlcb_types.h"
#include "../drivers/common/can_types.h"

#include "stdio.h" // printf

uint8_t _current_state = GRIDCONNECT_STATE_SYNC_START;
uint8_t _receive_buffer_index = 0;

gridconnect_buffer_t _receive_buffer;

uint8_t _is_valid_hex_char(uint8_t next_byte) {

    return (((next_byte >= '0') && (next_byte <= '9')) ||
            ((next_byte >= 'A') && (next_byte <= 'F')) ||
            ((next_byte >= 'a') && (next_byte <= 'f')));
}

uint8_t OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_t next_byte, gridconnect_buffer_t *buffer) {

    switch (_current_state) {

        case GRIDCONNECT_STATE_SYNC_START:
        {

            if ((next_byte == 'X') || (next_byte == 'x')) {
                _receive_buffer_index = 0;
                _receive_buffer[_receive_buffer_index] = ':';
                _receive_buffer_index++;
                _receive_buffer[_receive_buffer_index] = 'X';
                _receive_buffer_index++;
                _current_state = GRIDCONNECT_STATE_SYNC_FIND_HEADER;
            }

            break;
        }
        case GRIDCONNECT_STATE_SYNC_FIND_HEADER:
        {

            if (_receive_buffer_index < 11) {
                if ((next_byte == 'N') || (next_byte == 'n')) {
                    if (_receive_buffer_index == 10) { // Just right number of characters for the header, all done

                        _receive_buffer[_receive_buffer_index] = 'N';
                        _receive_buffer_index++; // skip over the "N"
                        _current_state = GRIDCONNECT_STATE_SYNC_FIND_DATA;
                    } else {
                        char _header_array[8];

                        for (int_olcb_t i = 0; i < 8; i++) {

                            _header_array[i] = '0';

                        }

                        int j = 0;
                        for (int_olcb_t i = _receive_buffer_index - 1; i >= (11 - _receive_buffer_index); i--) {

                            _header_array[j] = _receive_buffer[i];
                            j--;

                        }

                        for (int_olcb_t i = 0; i < 8; i++) {

                            _receive_buffer[2 + i] = _header_array[i];

                        }

                        _receive_buffer[10] = 'N';
                        _receive_buffer_index = 11;
                        _current_state = GRIDCONNECT_STATE_SYNC_FIND_DATA;
                    }
                } else {
                    if (_is_valid_hex_char(next_byte)) {
                        _receive_buffer[_receive_buffer_index] = next_byte;
                        _receive_buffer_index++;
                    } else
                        _current_state = GRIDCONNECT_STATE_SYNC_START; // Error Start Over
                }
            } else
                _current_state = GRIDCONNECT_STATE_SYNC_START; // Error Start Over

            break;
        }
        case GRIDCONNECT_STATE_SYNC_FIND_DATA:
        {
            if (next_byte == ';') {

                if ((_receive_buffer_index + 1) % 2 == 0) {

                    _receive_buffer[_receive_buffer_index] = ';';
                    _receive_buffer[_receive_buffer_index + 1] = 0; // null
                    _current_state = GRIDCONNECT_STATE_SYNC_START;

                    for (int_olcb_t i = 0; i < MAX_GRID_CONNECT_LEN; i++) {


                        (*buffer)[i] = _receive_buffer[i];

                    }

                    return true;
                }
            } else {

                if ((_is_valid_hex_char(next_byte)) && (_receive_buffer_index < MAX_GRID_CONNECT_LEN)) {
                    _receive_buffer[_receive_buffer_index] = next_byte;
                    _receive_buffer_index++;
                } else
                    _current_state = GRIDCONNECT_STATE_SYNC_START; // Error Start Over
            }

            break;
        }
        default:
            _current_state = GRIDCONNECT_STATE_SYNC_START; // Error Start Over

            break;
    }

    return false;
}

void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg) {
    char byte_str[5]; // 2 + null
    uint8_t byte;
    char identifier_str[9]; // 8 + null

    for (int_olcb_t i = 2; i < 10; i++) {

        identifier_str[i - 2] = (*gridconnect)[i];

    }
    identifier_str[8] = 0;

    char hex_it[64] = "0x";
    strcat(hex_it, identifier_str);
    can_msg->identifier = (uint32_t) strtoul(hex_it, NULL, 0);

    unsigned long data_char_count = strlen((char *) gridconnect) - (12);
    can_msg->payload_count = (uint8_t) (data_char_count / 2);

    int payload_index = 0;
    int i = 11;
    while (i < (data_char_count + 11)) {
        byte_str[0] = '0';
        byte_str[1] = 'x';
        byte_str[2] = (*gridconnect)[i];
        byte_str[3] = (*gridconnect)[i + 1];
        byte_str[4] = 0;

        byte = (uint8_t) strtoul(byte_str, NULL, 0);
        can_msg->payload[payload_index] = byte;
        payload_index++;
        i++;
        i++;
    }
}

void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg) {

    char temp_str[30];

    (*gridconnect)[0] = 0;
    strcat((char *) gridconnect, ":");
    strcat((char *) gridconnect, "X");

    sprintf((char *) &temp_str, "%08lX", (unsigned long) can_msg->identifier);
    strcat((char *) gridconnect, (char *) &temp_str);
    strcat((char *) gridconnect, "N");
    for (int_olcb_t i = 0; i < can_msg->payload_count; i++) {

        sprintf((char *) &temp_str, "%02X", can_msg->payload[i]);
        strcat((char *) gridconnect, (char *) &temp_str);

    }

    strcat((char *) gridconnect, ";");

}
