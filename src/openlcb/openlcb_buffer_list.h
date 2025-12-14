/** 
 * 
 * @subsection Description
 * 
 * A buffer list that can hold openlcb message structures.  Used to hold
 * messages that are coming in using a CAN physical layer adaptation of the openlcb 
 * message where it can be sent in 1 or many frames.  This buffer can hold partially 
 * received messages until they are fully assembled 
 * 
 * @note The CAN Receive Statemachine and 100ms timer access these buffers and typically 
 * run within interrupts and/or threads. Care must be taken to Pause and Resume the 
 * interrupts or threads if the main loop needs to access the buffers for any reason.
 * 
 * @note This is an internal list and the Application should not access it
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
 * @file openlcb_buffer_list.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LIST__
#define __OPENLCB_OPENLCB_LIST__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Message Buffer List<br>
     * 
     * @param none
     * 
     * @return none
     * 
     * @note This must always be called during application initialization
     */
    extern void OpenLcbBufferList_initialize(void);

    
    /**
     * @brief Inserts a new OpenLcb message into the buffer at the first open slot 
     *   see \ref OpenLcbBufferStore_allocate_buffer() 
     * 
     * @param openlcb_msg_t *new_msg [in] - Pointer to a message allocated from the OpenLcb Buffer Pool
     * 
     * @return *openlcb_msg_t - Pointer to the message or NULL if it fails
     */
    extern openlcb_msg_t *OpenLcbBufferList_add(openlcb_msg_t *new_msg);

    
    /**
     * @brief Searches the buffer for a message that matches the passed parameters
     * 
     * @param uint16_t source_alias [in] - the CAN alias of the node sending the message frame
     * @param uint16_t dest_alias   [in] - the CAN alias of the node receiving the message frame
     * @param uint16_t mti          [in] - the CAN MTI code of the message frame type
     * 
     * @return *openlcb_msg_t - Pointer to the message or NULL if the buffer is empty
     */
    extern openlcb_msg_t *OpenLcbBufferList_find(uint16_t source_alias, uint16_t dest_alias, uint16_t mti);

    
    /**
     * @brief Removes an OpenLcb message from the buffer, the caller is responsible for freeing the message
     *   see \ref OpenLcbBufferStore_free_buffer() 
     * 
     * @param openlcb_msg_t *new_msg [in] - Pointer to a message to be released from the list
     * 
     * @return *openlcb_msg_t - Pointer to the message or NULL if the message does not exist
     */
    extern openlcb_msg_t *OpenLcbBufferList_release(openlcb_msg_t *msg);

    
    /**
     * @brief Returns a Pointer to the message at the passed index
     * 
     * @param uint16_t index [in] - Index of the message requested
     * 
     * @return *openlcb_msg_t - Pointer to the message at the passed index or NULL if the index is out of bounds or the list slot is empty
     */
    extern openlcb_msg_t *OpenLcbBufferList_index_of(uint16_t index);

    
    /**
     * @brief Test to see if there are any messages stored in the buffer list
     * 
     * @param none
     * 
     * @return bool - True if the list is empty False if there is a least one non NULL message pointer in the list
     */
    extern bool OpenLcbBufferList_is_empty(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_LIST__ */
