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

#include "openlcb_gridconnect.h"

#include "openlcb_types.h"


#include "stdio.h"  // printf


uint8_olcb_t _current_state = GRIDCONNECT_STATE_SYNC_START;
uint8_olcb_t _receive_buffer_index = 0;

gridconnect_buffer_t _receive_buffer;

uint8_olcb_t _is_valid_hex_char(uint8_olcb_t next_byte) {

    return ( ((next_byte >= '0') && (next_byte <= '9')) ||
            ((next_byte >= 'A') && (next_byte <= 'F')) ||
            ((next_byte >= 'a') && (next_byte <= 'f')));

}

uint8_olcb_t OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_olcb_t next_byte, gridconnect_buffer_t* buffer) {

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

                        for (int i = 0; i < 8; i++)
                            _header_array[i] = '0';

                        int j = 0;
                        for (int i = _receive_buffer_index - 1; i >= (11 - _receive_buffer_index); i--) {
                            _header_array[j] = _receive_buffer[i];
                            j--;
                        }

                        for (int i = 0; i < 8; i++)
                            _receive_buffer[2 + i] = _header_array[i];

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
                        
                    for (int i = 0; i < MAX_GRID_CONNECT_LEN; i++)
                      (*buffer)[i] = _receive_buffer[i];

                    return TRUE;
                    
                } 
            } else {
                    
                    if ( (_is_valid_hex_char(next_byte)) && (_receive_buffer_index < MAX_GRID_CONNECT_LEN) ) {
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

    return FALSE;
}
