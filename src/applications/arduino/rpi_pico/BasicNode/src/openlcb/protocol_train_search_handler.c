/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * @file protocol_train_search_handler.c
 * @brief Train Search Protocol message handler implementation
 *
 * @details Decodes incoming train search Event IDs, compares the search query
 * against each train node's DCC address, and replies with a Producer
 * Identified event when a match is found.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

#include "protocol_train_search_handler.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_application_train.h"


    /** @brief Stored callback interface pointer. */
static const interface_protocol_train_search_handler_t *_interface;


    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @verbatim
     * @param interface  Populated callback table (may be NULL).
     * @endverbatim
     */
void ProtocolTrainSearch_initialize(const interface_protocol_train_search_handler_t *interface) {

    _interface = interface;

}

    /** @brief Return true if train_state matches the search address and flags. */
static bool _does_train_match(train_state_t *train_state, uint16_t search_address, uint8_t flags) {

    // Address must match
    if (train_state->dcc_address != search_address) { return false; }

    // If DCC protocol flag is set, check long address match
    if (flags & TRAIN_SEARCH_FLAG_DCC) {

        if (flags & TRAIN_SEARCH_FLAG_LONG_ADDR) {

            // Requesting long address — train must be long address
            if (!train_state->is_long_address) { 
                
                return false; 
            
            }

        } else {

            // Not requesting long address — if address < 128 and train is long, no match
            // (unless allocate bit is set, which is handled separately)
            if (search_address < 128 && train_state->is_long_address && !(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) { 
                
                return false; 
            
            }

        }

    }

    return true;

}


    /**
     * @brief Handles incoming train search events.
     *
     * @details Decodes the search query, compares against this node's DCC
     * address, and loads a Producer Identified reply if matched.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit event_id_t containing encoded search query.
     * @endverbatim
     */
void ProtocolTrainSearch_handle_search_event(
        openlcb_statemachine_info_t *statemachine_info,
        event_id_t event_id) {

    if (!statemachine_info || !statemachine_info->openlcb_node) { 
        
        return; 
    
    }

    train_state_t *train_state = statemachine_info->openlcb_node->train_state;
    if (!train_state) { 
        
        return; 
    
    }

    // Decode the search query
    uint8_t digits[6];
    OpenLcbUtilities_extract_train_search_digits(event_id, digits);
    uint16_t search_address = OpenLcbUtilities_train_search_digits_to_address(digits);
    uint8_t flags = OpenLcbUtilities_extract_train_search_flags(event_id);

    // Check if this train matches
    if (!_does_train_match(train_state, search_address, flags)) { return; }

    // Build reply: Producer Identified Set with this train's search event ID
    uint8_t reply_flags = 0;
    if (train_state->is_long_address) {

        reply_flags |= TRAIN_SEARCH_FLAG_DCC | TRAIN_SEARCH_FLAG_LONG_ADDR;

    } else {

        reply_flags |= TRAIN_SEARCH_FLAG_DCC;

    }
    reply_flags |= (train_state->speed_steps & TRAIN_SEARCH_SPEED_STEP_MASK);

    event_id_t reply_event = OpenLcbUtilities_create_train_search_event_id(train_state->dcc_address, reply_flags);

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            MTI_PRODUCER_IDENTIFIED_SET);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, reply_event);

    statemachine_info->outgoing_msg_info.valid = true;

    // Fire callback
    if (_interface && _interface->on_search_matched) {

        _interface->on_search_matched(statemachine_info->openlcb_node, search_address, flags);

    }

}
