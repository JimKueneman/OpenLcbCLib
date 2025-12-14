/** 
 * 
 * @subsection Description
 * 
 * Implements the core buffers for CAN message buffers. The CAN FIFO buffer is an 
 * array of pointers to these core buffers that are allocated and freed.  
 * 
 * @note The CAN Receive Statemachine and 100ms timer access these buffers and typically 
 * run within interrupts and/or threads. Care must be taken to Pause and Resume the 
 * interrupts or threads if the main loop needs to access the buffers for any reason.
 * 
 *  
 * @subsection License
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
 * @copyright Copyright (c) 2025, Jim Kueneman
 * @author Jim Kueneman
 * @date 14 Dec 2025
 * @file can_buffer_store.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVERS_COMMON_CAN_CAN_BUFFER_STORE__
#define	__DRIVERS_COMMON_CAN_CAN_BUFFER_STORE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the CAN Buffer Store<br>
     * 
     * @param none
     * 
     * @return none
     * 
     * @note This must always be called during application initialization
     */
    extern void CanBufferStore_initialize(void);

    
    /**
     * @brief Allocates a new CAN buffer
     * 
     * @param none
     * 
     * @return *can_msg_t - Pointer to the message buffer or NULL if it fails
     */
    extern can_msg_t* CanBufferStore_allocate_buffer(void);
 
    
    /**
     * @brief Frees the buffer so it can be reused.<br>
     * 
     * @param can_msg_t *msg [in] - Pointer to a message to be freed
     * 
     * @return none
     * 
     */
    extern void CanBufferStore_free_buffer(can_msg_t* msg);

    
    /**
     * @brief The number of CAN messages currently allocated in the buffer store.
     * 
     * @param none
     * 
     * @return uint16_t - Number of CAN messages currently allocated
     */
    extern uint16_t CanBufferStore_messages_allocated(void);

    
    /**
     * @brief The maximum number of CAN messages that have been allocated in the buffer store.
     * Useful for understanding how deep the buffer store needs to be during stress testing
     * 
     * @param none
     * 
     * @return uint16_t - Maximum number of CAN sized messages that have been allocated currently
     */
    extern uint16_t CanBufferStore_messages_max_allocated(void);

    
    /**
     * @brief Resets the running count of the Maximum number of CAN buffer that have been allocated at one time
     * 
     * @param none
     * 
     * @return none
     */
    extern void CanBufferStore_clear_max_allocated(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVERS_COMMON_CAN_CAN_BUFFER_STORE__ */

