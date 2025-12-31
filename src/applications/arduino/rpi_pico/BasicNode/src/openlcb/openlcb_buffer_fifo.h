/*
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
 * 22 Dec 2025
 * Copyright (c) 2025, Jim Kueneman
 */

/**
 *
 * @brief Implements a FIFO buffer for OpenLcb/Lcc message structures.
 *
 * OpenLcb/LCC messages as allocated from the buffer store (\ref openlcb_buffer_store.h) then placed into the FIFO by the
 * physical layer receive state machine and then the main OpenLcb/LCC loop (\ref openlcb_main_statemachine.h) pulls
 * them out one at a time and dispatches them to the handlers.
 *
 * @note Applications typically only need to access the Initialize function in this module.
 * 
 * @warning The physical layer receive state machines and 100ms timer access these buffers and typically
 * run within interrupts and/or threads. Care must be taken to Pause and Resume the
 * interrupts or threads if the main loop needs to access the buffers for any reason.
 *
 * @file openlcb_buffer_fifo.h
 *
 */


// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_FIFO__
#define __OPENLCB_OPENLCB_FIFO__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
    /**
     * @brief Initializes the OpenLcb Message Buffer FIFO.
     * 
     * @param none
     * 
     * @return none
     * 
     * @attention This must always be called during application initialization.
     */
    extern void OpenLcbBufferFifo_initialize(void);
    
    
     /**
     * @brief Pushes a new OpenLcb message into the FIFO buffer that was allocated 
     * see \ref OpenLcbBufferStore_allocate_buffer().
     * 
     * @param openlcb_msg_t *new_msg - Pointer to a message allocated from the OpenLcb Buffer Pool.
     * 
     * @return Pointer to the message or NULL if it fails.
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg);
 
    
    /**
     * @brief Pops a OpenLcb message off the FIFO buffer.  The caller is responsible to free the message when done 
     *   see \ref OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg).
     * 
     * @param none
     * 
     * @return Pointer to the message or NULL if the buffer is empty.
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_pop(void);
    

    /**
     * @brief Tests if there is a message in the FIFO buffer.
     * 
     * @param none
     * 
     * @return True if there is at least one message, false if the list is empty.
     */
    extern bool OpenLcbBufferFifo_is_empty(void);
    
    
    /**
     * @brief Returns the number of messages in the FIFO buffer.
     * 
     * @param none
     * 
     * @return The number of messages in the buffer.
     */
    extern uint16_t OpenLcbBufferFifo_get_allocated_count(void);
    

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_FIFO__ */
