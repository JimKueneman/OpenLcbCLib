
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
 * \file debug.h
 *
 *
 * @author Jim Kueneman
 * @date 3 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __Debug__
#define	__Debug__

#include "../../../openlcb/openlcb_defines.h"
#include "../../../openlcb/openlcb_types.h"
#include "../../../drivers/common/can_types.h"

extern void PrintCAN1Registers(void);

extern void PrintDMA0Registers(void);

extern void PrintDMA1Registers(void);

extern void PrintDMA2Registers(void);

extern void PrintDMA3Registers(void);

extern void PrintMtiName(uint16_olcb_t mti);

extern void PrintBufferStats(void);

//extern void ForceFlushAndFreeFIFO(openlcb_msg_buffer_t* fifo);
//extern void PrintContentsFIFO(openlcb_msg_buffer_t* fifo);

extern void PrintAliasAndNodeID(uint16_olcb_t alias, uint64_olcb_t node_id);

extern void PrintCanIdentifier(uint32_olcb_t identifier);

extern void PrintCanFrameIdentifierName(uint32_olcb_t identifier);

extern void PrintAlias(uint16_olcb_t alias);

extern void PrintNodeID(uint64_olcb_t node_id);

extern void PrintEventID(event_id_t event_id) ;

extern void PrintOpenLcbMsg(openlcb_msg_t* openlcb_msg);

extern void PrintInt64(uint64_olcb_t n);

extern void PrintDWord(uint32_olcb_t dword);

extern void PrintCanMsg(can_msg_t* can_msg);

extern void PrintNode(openlcb_node_t* node);
    

extern uint8_olcb_t print_msg;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __Debug__ */

