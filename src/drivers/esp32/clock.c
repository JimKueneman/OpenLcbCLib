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

// Add any compiler specific includes

#include "../../openlcb/openlcb_types.h"


// This must be here and assigned in Driver100msClock_Initialization

_100ms_timer_callback_func_t Driver100msClock_callback_func;

void Driver100msClock_Initialization(_100ms_timer_callback_func_t _100ms_callback_func) {
    
    // Must be here.....
    Driver100msClock_callback_func = _100ms_callback_func;
    
    // Timer Initialize --------------------------------------------------------
    // -------------------------------------------------------------------------


    // -------------------------------------------------------------------------
    
    
}


// Implement a way to call the Driver100msClock_callback_func at 100ms intervals here

// Example:

//void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {
//
//    IFS0bits.T2IF = 0; // Clear T2IF
//
//    // Increment any timer counters assigned
//    if (Driver100msClock_callback_func)
//        Driver100msClock_callback_func();
//
//    return;
//}


void Driver100msClock_pause_100ms_timer() {
  
    // Implement a way to stop the timer here
    
// Example
//    T2CONbits.TON = 0; // Turn off 100ms Timer
    
}

extern void Driver100msClock_resume_100ms_timer() {
    
    // Implement a way to stop the timer here
    
// Example
//    T2CONbits.TON = 1; // Turn on 100ms Timer
    
}

