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
 * \file callbacks_olcb.c
 *
 * Core node lifecycle callback implementations for the BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_olcb.h"

#include "xc.h"

#define LED_BLUE_TRIS _TRISA7
#define LED_BLUE _LATA7

#define LED_YELLOW_TRIS _TRISC5
#define LED_YELLOW _LATC5

void CallbacksOlcb_initialize(void) {

    LED_BLUE = 0;
    LED_YELLOW = 0;

    LED_BLUE_TRIS = 0;
    LED_YELLOW_TRIS = 0;

}

void CallbacksOlcb_on_100ms_timer(void) {

    static uint16_t _100ms_ticks = 0;

    if (_100ms_ticks > 5) {

        LED_BLUE = 0;
        LED_YELLOW = 0;

        _100ms_ticks = 0;

    }

    _100ms_ticks++;

}
