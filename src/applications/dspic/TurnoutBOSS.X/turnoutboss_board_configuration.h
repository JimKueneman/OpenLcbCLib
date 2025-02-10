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
 * \file turnoutboss_board_configuration.h
 *
 * Module pulls the linked board NodeID (if available) and the board type (BL/BR) from
 * the configuration memory.  It also maintains the state of the signals that are updated
 * from the TurnoutBoss_Event_Handler module for incoming events from linked Left and 
 * Right TurnoutBosses
 *
 * @author Jim Kueneman
 * @date 22 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTBOSS_BOARD_CONFIGURATION__
#define	__TURNOUTBOSS_BOARD_CONFIGURATION__

#ifdef MPLAB
#include "../../../openlcb/openlcb_types.h"
#else
#include "src/openlcb/openlcb_types.h"
#endif


#include "turnoutboss_types.h"



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    
    extern void TurnoutBossBoardConfiguration_initialize(openlcb_node_t *node, board_configuration_t* board_configuration);
    
    extern uint16_olcb_t TurnoutBossBoardConfiguration_write_eventID_to_configuration_memory(openlcb_node_t *node, event_id_t event, uint16_olcb_t address);

# define the start location of the producer/consumer array for CDI. This goes with the other definitions in turnout_board_configuration.h
#define START_OF_PRODUCER_CONSUMER_MAP                  200 // 416 bytes space for 51 producers and consumers

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTBOSS_BOARD_CONFIGURATION__ */

