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
 * @file dspic33_can_drivers.h
 *
 * CAN hardware interface for dsPIC33 ECAN module.  Provides the platform-
 * specific transmit, receive, and initialization functions that the
 * OpenLcbCLib CAN state machine calls through function pointers.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#ifndef __DSPIC33_CAN_DRIVERS__
#define __DSPIC33_CAN_DRIVERS__

#include <stdbool.h>
#include <stdint.h>

#include "../openlcb_c_lib/openlcb/openlcb_types.h"
#include "../openlcb_c_lib/drivers/canbus/can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void Dspic33CanDriver_initialize(void);

    extern bool Dspic33CanDriver_is_can_tx_buffer_clear(void);

    extern void Dspic33CanDriver_pause_can_rx(void);

    extern void Dspic33CanDriver_resume_can_rx(void);

    extern bool Dspic33CanDriver_transmit_can_frame(can_msg_t *msg);

    extern void Dspic33CanDriver_c1_interrupt_handler(void);

    extern uint8_t Dspic33CanDriver_get_max_can_fifo_depth(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DSPIC33_CAN_DRIVERS__ */
