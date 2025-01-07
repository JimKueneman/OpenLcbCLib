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
 * \file clock.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * of a 100ms clock.  A new supported MCU/PC will create a file that handles the 
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "../openlcb/openlcb_types.h"
#include "../openlcb/openlcb_node.h"
#include "../openlcb/protocol_datagram.h"


parameterless_callback_t _pause_timer_callback_func = (void*) 0;
parameterless_callback_t _resume_timer_callback_func = (void*) 0;


void Driver100msClock_initialization(parameterless_callback_t pause_timer_callback, parameterless_callback_t resume_timer_callback) {
    
    _pause_timer_callback_func = pause_timer_callback;
    _resume_timer_callback_func = resume_timer_callback;
       
}

void _100ms_clock_sink() {
    
   
    Node_100ms_timer_tick();
    DatagramProtocol_100ms_time_tick();
    
    
}

parameterless_callback_t Driver100msClock_get_sink(void) {
    
    return &_100ms_clock_sink;
    
}

void Driver100msClock_pause_100ms_timer(void) {
  
    if (_pause_timer_callback_func)
        _pause_timer_callback_func();
   
}

extern void Driver100msClock_resume_100ms_timer(void) {
    
    if (_resume_timer_callback_func)
        _resume_timer_callback_func();
    
}

