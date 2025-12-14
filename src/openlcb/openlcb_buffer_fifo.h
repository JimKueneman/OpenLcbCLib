/** \copyright
 * 
 * **NOTE - Applications rarely need to access these functions directly**
 * 
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
 * \file openlcb_buffer_fifo.h
 *
 * Implementation of the FIFO where the OpenLcb messages are placed by the receive
 * module.  The main loop pulls them out one at a time and dispatched them to the handlers.
 * Caution is required as the receive module may be putting new messages in within the
 * context of an interrupt or thread so when the main loop access this buffer it must
 * use the lock resources Can Rx and 100ms timer modules with the Pause and Resume functions
 * exported by them respectively.
 * 
 * @author Jim Kueneman
 * @date 14 Dec 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_FIFO__
#define __OPENLCB_FIFO__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
    /**
     * @brief Initializes the OpenLcb Message Buffer FIFO
     * @brief This must always be called at the beginning of the application execution
     * 
     * @param none
     * 
     * @return none
     */
    extern void OpenLcbBufferFifo_initialize(void);
    
     /**
     * @brief Pushes a new OpenLcb message into the FIFO buffer that was allocated 
     *   see \ref OpenLcbBufferStore_allocate_buffer()
     * 
     * @param openlcb_msg_t *new_msg [in] - Pointer to a message allocated from the OpenLcb Buffer Pool
     * 
     * @return *openlcb_msg_t - Pointer to the message or NULL if it fails
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_push(openlcb_msg_t *new_msg);
 
    
    /**
     * @brief Pops a OpenLcb message off the FIFO buffer.  The caller is responsible to free the message when done 
     *   see \ref OpenLcbBufferStore_free_buffer(openlcb_msg_t *openlcb_msg)
     * 
     * @param none
     * 
     * @return *openlcb_msg_t - Pointer to the message or NULL if the buffer is empty
     */
    extern openlcb_msg_t *OpenLcbBufferFifo_pop(void);
    

    /**
     * @brief Tests if there is a message in the FIFO buffer.
     * 
     * @param none
     * 
     * @return bool true if there is at least one message, false if the list is empty
     */
    extern bool OpenLcbBufferFifo_is_empty(void);
    
    
    /**
     * @brief Returns the number of messages in the FIFO buffer.
     * 
     * @param none
     * 
     * @return uint16_t the number of messages in the buffer
     */
    extern uint16_t OpenLcbBufferFifo_get_allocated_count(void);
    

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_FIFO__ */
