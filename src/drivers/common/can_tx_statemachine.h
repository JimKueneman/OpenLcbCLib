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
 * \file can_tx_statemachine.h
 *
 * Takes an OpenLcb message structure and splits it into CAN frames to transmit if 
 * necessary, else it packs up the CAN frame from the message struture and send it
 * to the CAN Driver to transmit on the physical layer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_TX_STATEMACHINE__
#define	__CAN_TX_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */


extern void CanTxStatemachine_initialize(void);


extern bool CanTxStatemachine_try_transmit_openlcb_message(can_msg_t* can_msg_worker, openlcb_msg_t* openlcb_msg, uint16_t openlcb_payload_start_index);

extern bool CanTxStatemachine_try_transmit_can_message(can_msg_t* can_msg);
 
#ifdef	__cplusplus
}
#endif /* __cplusplus */


#endif	/* __CAN_TX_STATEMACHINE__ */

