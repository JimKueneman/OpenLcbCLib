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
 * \file nmra_dcc_driver.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to write/read configuration memory.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "nmra_dcc_driver.h"

#include "nmra_dcc_types.h"
#include "nmra_dcc_buffer_store.h"

nmra_dcc_message_t* _nmra_dcc_current_message;
nmra_dcc_priority_buffers_t _nmra_dcc_priority_buffers;

nmra_dcc_timing_callback_t _one_bit_hi = (void *) 0;
nmra_dcc_timing_callback_t _one_bit_lo = (void *) 0;
nmra_dcc_timing_callback_t _zero_bit_hi = (void *) 0;
nmra_dcc_timing_callback_t _zero_bit_lo = (void *) 0;
nmra_dcc_timing_callback_t _high_z_cutout = (void *) 0;

nmra_dcc_message_t* _load_idle_message(void) {
    
    nmra_dcc_message_t* result = NmraDccBufferStore_allocateBuffer();
    
    result->buffer_size = 2;
    result->buffer[0] = 0b11111111;
    result->buffer[1] = 0b00000000;
    
    return result;

}

nmra_dcc_message_t* _next_nmra_dcc_message(void) {

    nmra_dcc_message_t* result;

    if (_nmra_dcc_priority_buffers.high.head != _nmra_dcc_priority_buffers.high.tail) {

        result = _nmra_dcc_priority_buffers.high.buffer[_nmra_dcc_priority_buffers.high.tail];

        _nmra_dcc_priority_buffers.high.tail = _nmra_dcc_priority_buffers.high.tail + 1;

        if (_nmra_dcc_priority_buffers.high.tail >= LEN_NMRA_DCC_MESSAGE_BUFFER) {

            _nmra_dcc_priority_buffers.high.tail = 0;

        }

    } else if (_nmra_dcc_priority_buffers.medium.head != _nmra_dcc_priority_buffers.medium.tail) {

        result = _nmra_dcc_priority_buffers.medium.buffer[_nmra_dcc_priority_buffers.medium.tail];

        _nmra_dcc_priority_buffers.medium.tail = _nmra_dcc_priority_buffers.medium.tail + 1;

        if (_nmra_dcc_priority_buffers.medium.tail >= LEN_NMRA_DCC_MESSAGE_BUFFER) {

            _nmra_dcc_priority_buffers.medium.tail = 0;

        }

    } else if (_nmra_dcc_priority_buffers.low.head != _nmra_dcc_priority_buffers.low.tail) {

        result = _nmra_dcc_priority_buffers.low.buffer[_nmra_dcc_priority_buffers.low.tail];

        _nmra_dcc_priority_buffers.low.tail = _nmra_dcc_priority_buffers.low.tail + 1;

        if (_nmra_dcc_priority_buffers.low.tail >= LEN_NMRA_DCC_MESSAGE_BUFFER) {

            _nmra_dcc_priority_buffers.low.tail = 0;

        }

    } else {

        result = _load_idle_message();

    }

    return result;
}

void NmraDccDriver_initalize(nmra_dcc_timing_callback_t one_bit_hi, nmra_dcc_timing_callback_t one_bit_lo, nmra_dcc_timing_callback_t zero_bit_hi, nmra_dcc_timing_callback_t zero_bit_lo, nmra_dcc_timing_callback_t high_z_cutout) {

    NmraDccBufferStore_initialize();

    _nmra_dcc_priority_buffers.high.head = 0;
    _nmra_dcc_priority_buffers.high.tail = 0;

    _nmra_dcc_priority_buffers.medium.head = 0;
    _nmra_dcc_priority_buffers.medium.tail = 0;

    _nmra_dcc_priority_buffers.low.head = 0;
    _nmra_dcc_priority_buffers.low.tail = 0;

    _nmra_dcc_current_message = (void *) 0;

    _one_bit_hi = one_bit_hi;
    _one_bit_lo = one_bit_lo;
    _zero_bit_hi = zero_bit_hi;
    _zero_bit_lo = zero_bit_lo;
    _high_z_cutout = high_z_cutout;
    
    // Load the system with a message or idle message
    _nmra_dcc_current_message = _next_nmra_dcc_message();

}

void NmraDccDriver_56us_timer(void) {
    
    if (!_nmra_dcc_current_message) 
        
        return;

    // this keeps a consistent dead-time in calling the callbacks by being the first thing that happens

    if (_nmra_dcc_current_message->next_callback) {

        _nmra_dcc_current_message->next_callback(_nmra_dcc_current_message->next_callback_parameter);

    }

    // Calculate the next one

    switch (_nmra_dcc_current_message->state) {

        case STATE_NMRA_DCC_PREAMBLE:

            if (_nmra_dcc_current_message->next_callback == _one_bit_hi) {

                // just finished the high bit, do the low bit
                _nmra_dcc_current_message->next_callback = _one_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

            } else {

                // just finished the low bit, what is next?

                // move to the next preamble bit
                _nmra_dcc_current_message->preamble_position = _nmra_dcc_current_message->preamble_position + 1;

                // have we sent all the preamble bits?
                if (_nmra_dcc_current_message->preamble_position < _nmra_dcc_current_message->preamble_size) {

                    // nope send another hi/lo pair
                    _nmra_dcc_current_message->next_callback = _one_bit_hi;
                    _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                } else {

                    // yup, done move on the the start bit
                    _nmra_dcc_current_message->state = STATE_NMRA_DCC_START_BIT;
                    _nmra_dcc_current_message->buffer_byte_bit_index = 0;
                    _nmra_dcc_current_message->buffer_byte_index = 0;
                    _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                    _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                }

            }

            break;
        case STATE_NMRA_DCC_START_BIT:

            // is it the first high half?
            if (_nmra_dcc_current_message->next_callback == _zero_bit_hi) {

                // send the low half of the bit
                _nmra_dcc_current_message->next_callback = _zero_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

            } else {

                // have all the buffer data bytes been sent?
                if (_nmra_dcc_current_message->buffer_byte_index < _nmra_dcc_current_message->buffer_size) {

                    // more data bytes to send
                    _nmra_dcc_current_message->buffer_byte_bit_index = 0; // reset the byte bits
                    _nmra_dcc_current_message->state = STATE_NMRA_DCC_DATA; // send the next data byte

                    // test the first bit to see if that is what needs to be set
                    if (_nmra_dcc_current_message->buffer[_nmra_dcc_current_message->buffer_byte_index] && 0b00000001) {

                        _nmra_dcc_current_message->next_callback = _one_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                    } else {

                        _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                    }

                    // keep the running copy of the XOR byte to send
                    _nmra_dcc_current_message->xor_byte = _nmra_dcc_current_message->xor_byte ^ _nmra_dcc_current_message->buffer[_nmra_dcc_current_message->buffer_byte_index];

                } else {

                    // XOR time
                    _nmra_dcc_current_message->buffer_byte_bit_index = 0; // reset the byte bits
                    _nmra_dcc_current_message->state = STATE_NMRA_DCC_XOR_BYTE; // send the next data byte

                    // test the first bit to see if that is what needs to be set
                    if (_nmra_dcc_current_message->xor_byte && 0b00000001) {

                        _nmra_dcc_current_message->next_callback = _one_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                    } else {

                        _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                    }

                }

            }

            break;
        case STATE_NMRA_DCC_DATA:

            if (_nmra_dcc_current_message->next_callback == _one_bit_hi) {

                // send the second half of the one bit
                _nmra_dcc_current_message->next_callback = _one_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

            } else if (_nmra_dcc_current_message->next_callback == _zero_bit_hi) {

                // send the second half of the zero bit
                _nmra_dcc_current_message->next_callback = _zero_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

            } else {

                // pick up the next bit to send from the data buffer's current byte
                _nmra_dcc_current_message->buffer_byte_bit_index = _nmra_dcc_current_message->buffer_byte_bit_index + 1;

                // have we sent all 8 of the bits?
                if (_nmra_dcc_current_message->buffer_byte_bit_index < 8) {

                    // nope not done with the bits yet, decode if the bit is a one or zero
                    if ((_nmra_dcc_current_message->buffer[_nmra_dcc_current_message->buffer_byte_index] >> _nmra_dcc_current_message->buffer_byte_bit_index) && 0b00000001) {

                        _nmra_dcc_current_message->next_callback = _one_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                    } else {

                        _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                    }

                } else {

                    // yup all sent, move on to the next byte
                    _nmra_dcc_current_message->buffer_byte_index = _nmra_dcc_current_message->buffer_byte_index + 1;

                    // send the start bit and let it decide if we are done or if it is time to send the XOR byte
                    _nmra_dcc_current_message->state = STATE_NMRA_DCC_START_BIT;
                    _nmra_dcc_current_message->buffer_byte_bit_index = 0; // reset the bit index
                    _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                    _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                }


            }

            break;
        case STATE_NMRA_DCC_XOR_BYTE:

            if (_nmra_dcc_current_message->next_callback == _one_bit_hi) {

                // send the second half of the one bit
                _nmra_dcc_current_message->next_callback = _one_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

            } else if (_nmra_dcc_current_message->next_callback == _zero_bit_hi) {

                // send the second half of the zero bit
                _nmra_dcc_current_message->next_callback = _zero_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

            } else {

                // pick up the next bit to send from the data buffer's current byte
                _nmra_dcc_current_message->buffer_byte_bit_index = _nmra_dcc_current_message->buffer_byte_bit_index + 1;

                // have we sent all 8 of the bits?
                if (_nmra_dcc_current_message->buffer_byte_bit_index < 8) {

                    // nope not done with the bits yet, decode if the bit is a one or zero
                    if ((_nmra_dcc_current_message->xor_byte >> _nmra_dcc_current_message->buffer_byte_bit_index) && 0b00000001) {

                        _nmra_dcc_current_message->next_callback = _one_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                    } else {

                        _nmra_dcc_current_message->next_callback = _zero_bit_hi;
                        _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ZERO_BIT_WIDTH_IN_MICROSECONDS;

                    }

                } else {

                    // send the start bit and let it decide if we are done or if it is time to send the XOR byte
                    _nmra_dcc_current_message->state = STATE_NMRA_DCC_END_BIT;
                    _nmra_dcc_current_message->buffer_byte_bit_index = 0; // reset the bit index
                    _nmra_dcc_current_message->buffer_byte_index = 0;
                    _nmra_dcc_current_message->next_callback = _one_bit_hi;
                    _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

                }


            }

            break;
        case STATE_NMRA_DCC_END_BIT:

            if (_nmra_dcc_current_message->next_callback == _one_bit_hi) {

                // send the second half of the one bit
                _nmra_dcc_current_message->next_callback = _one_bit_lo;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

            } else {

                // all done...
                NmraDccBufferStore_freeBuffer(_nmra_dcc_current_message);
                
                // if there is not a waiting message this will return an idle message
                _nmra_dcc_current_message = _next_nmra_dcc_message();
 
                _nmra_dcc_current_message->state = STATE_NMRA_DCC_PREAMBLE;
                _nmra_dcc_current_message->buffer_byte_bit_index = 0; // reset the bit index
                _nmra_dcc_current_message->buffer_byte_index = 0;
                _nmra_dcc_current_message->next_callback = _one_bit_hi;
                _nmra_dcc_current_message->next_callback_parameter = NMRA_DCC_ONE_BIT_WIDTH_IN_MICROSECONDS;

            }

            break;

    }

}
