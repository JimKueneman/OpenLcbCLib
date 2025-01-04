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
 * \file MCP4014_driver.c
 *
 * Driver for the MicroChip MPC4014 electronically adjusted resistor.
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

// define FCY=40000000UL in the compiler macros so the delay, UART, etc calculations are correct  

#include "xc.h"
#include <libpic30.h>

#include "MCP4014_driver.h"

#include "../../../../../openlcb/openlcb_types.h"
#include "../../io_pinout.h"

void MCP4014Driver_set_gain(uint8_olcb_t detector_id, uint8_olcb_t gain) {

    switch (detector_id) {
        case 0:
            TRACK_DETECT_GAIN = 0; // set to 0 before CS is lower = decrement mode
            __delay_us(1);
            TRACK_DETECT_GAIN_1_CS = 0;
            __delay_us(1);

            // Countdown to 0;
            for (int i = 0; i < 64; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 1;
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;  // Wiper decrements on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_1_CS = 1;
            __delay_us(1);

            // Now set the gain from 0
            TRACK_DETECT_GAIN = 1; // set to 0 before CS is lower = increment mode
            __delay_us(1);
            TRACK_DETECT_GAIN_1_CS = 0;
            __delay_us(1);

            // Count up to gain
            for (int i = 0; i < gain; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;
                __delay_us(1);
                TRACK_DETECT_GAIN = 1; // Wiper increments on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_1_CS = 1;

            return;

        case 1:
            TRACK_DETECT_GAIN = 0; // set to 0 before CS is lower = decrement mode
            __delay_us(1);
            TRACK_DETECT_GAIN_2_CS = 0;
            __delay_us(1);

            // Countdown to 0;
            for (int i = 0; i < 64; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 1;
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;  // Wiper decrements on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_2_CS = 1;
            __delay_us(1);

            // Now set the gain from 0
            TRACK_DETECT_GAIN = 1; // set to 0 before CS is lower = increment mode
            __delay_us(1);
            TRACK_DETECT_GAIN_2_CS = 0;
            __delay_us(1);

            // Count up to gain
            for (int i = 0; i < gain; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;
                __delay_us(1);
                TRACK_DETECT_GAIN = 1; // Wiper increments on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_2_CS = 1;

            return;


        case 2:
            TRACK_DETECT_GAIN = 0; // set to 0 before CS is lower = decrement mode
            __delay_us(1);
            TRACK_DETECT_GAIN_3_CS = 0;
            __delay_us(1);

            // Countdown to 0;
            for (int i = 0; i < 64; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 1;
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;  // Wiper decrements on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_3_CS = 1;
            __delay_us(1);

            // Now set the gain from 0
            TRACK_DETECT_GAIN = 1; // set to 0 before CS is lower = increment mode
            __delay_us(1);
            TRACK_DETECT_GAIN_3_CS = 0;
            __delay_us(1);

            // Count up to gain
            for (int i = 0; i < gain; i++) {
                __delay_us(1);
                TRACK_DETECT_GAIN = 0;
                __delay_us(1);
                TRACK_DETECT_GAIN = 1; // Wiper increments on this edge

            }
            __delay_us(1);
            TRACK_DETECT_GAIN_3_CS = 1;

            return;



    }

}



