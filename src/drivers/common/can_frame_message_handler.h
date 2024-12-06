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
 * \file can_frame_message_hander.h
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN main
 * statemachine to accomplish that.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */
// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_MSG_HANDLER__
#define	__CAN_MSG_HANDLER__

#include <xc.h> // include processor files - each processor file is guarded.
#include "can_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"

extern void CanFrameMessageHandler_cid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) ;

extern void CanFrameMessageHandler_rid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg); 

extern void CanFrameMessageHandler_amd(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg); 

extern void CanFrameMessageHandler_ame(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg); 

extern void CanFrameMessageHandler_amr(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg); 


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_MSG_HANDLER__ */

