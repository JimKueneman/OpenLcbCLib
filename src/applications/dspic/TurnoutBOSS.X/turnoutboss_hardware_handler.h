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
 * \file turnoutboss_hardware_handler.h
 *
 * Scans the TurnoutBoss Hardware States and updates the TurnoutBoss_Signaling_States
 * 
 *
 * @author Jim Kueneman
 * @date 19 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_HARDWARE_SIGNALING_HANDLER__
#define	__TURNOUTBOSS_HARDWARE_SIGNALING_HANDLER__

#ifndef PLATFORMIO
#include "../../../openlcb/openlcb_types.h"
#include "../../../openlcb/openlcb_node.h"
#else
#include "src/openlcb/openlcb_types.h"
#include "src/openlcb/openlcb_node.h"
#endif 



typedef struct {
    uint8_olcb_t state : 1; // State is set or not
    uint8_olcb_t filter : 4; // counter for the digital filter while reading IO pins

} input_filter_t;

typedef uint8_olcb_t (*filter_func_t)(input_filter_t* filter);

typedef struct {
    input_filter_t turnout_normal;
    input_filter_t turnout_diverging;
    input_filter_t pushbutton_normal;
    input_filter_t pushbutton_diverging;
    input_filter_t occupancy_1;
    input_filter_t occupancy_2;
    input_filter_t occupancy_3;
    
    filter_func_t inc_filter;
    filter_func_t dec_filter;

} input_filters_t;

typedef struct {
    uint8_olcb_t turnout : 1;
    uint8_olcb_t signal_a;
    uint8_olcb_t signal_b;
    uint8_olcb_t signal_c;
    uint8_olcb_t signal_d;

} outputs_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    

    extern void TurnoutBossHardwareHandler_initalize(void);
    
    extern void TurnoutBossHardwareHandler_scan_for_changes(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_HARDWARE_SIGNALING_HANDLER__ */

