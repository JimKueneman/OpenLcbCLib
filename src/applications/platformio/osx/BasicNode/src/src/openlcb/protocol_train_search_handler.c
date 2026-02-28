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
#include <string.h>

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

    /** @brief Return true if the search digits match the address per TrainSearchS §6.3. */
static bool _does_address_match(uint16_t train_address, const uint8_t *digits, uint8_t flags) {

    // Convert train address to decimal digits for comparison
    uint8_t train_digits[6];
    int train_digit_count = 0;
    uint16_t temp = train_address;

    if (temp == 0) {

        train_digits[0] = 0;
        train_digit_count = 1;

    } else {

        // Extract digits in reverse
        uint8_t rev[6];
        while (temp > 0 && train_digit_count < 6) {

            rev[train_digit_count++] = (uint8_t)(temp % 10);
            temp /= 10;

        }

        // Reverse into train_digits
        for (int i = 0; i < train_digit_count; i++) {

            train_digits[i] = rev[train_digit_count - 1 - i];

        }

    }

    // Extract the search digit sequence (skip leading 0xF padding)
    uint8_t search_digits[6];
    int search_digit_count = 0;

    for (int i = 0; i < 6; i++) {

        if (digits[i] <= 9) {

            search_digits[search_digit_count++] = digits[i];

        }

    }

    if (search_digit_count == 0) { return false; }

    if (flags & TRAIN_SEARCH_FLAG_EXACT) {

        // Exact: search digits must exactly equal the address digits
        if (search_digit_count != train_digit_count) { return false; }

        for (int i = 0; i < search_digit_count; i++) {

            if (search_digits[i] != train_digits[i]) { return false; }

        }

        return true;

    } else {

        // Prefix: search digits must be a prefix of the address digits
        if (search_digit_count > train_digit_count) { return false; }

        for (int i = 0; i < search_digit_count; i++) {

            if (search_digits[i] != train_digits[i]) { return false; }

        }

        return true;

    }

}

    /** @brief Return true if the search digits match any digit sequence in name per TrainSearchS §6.3. */
static bool _does_name_match(const char *name, const uint8_t *digits, uint8_t flags) {

    if (!name || name[0] == '\0') { return false; }

    // Extract search digit sequences separated by 0xF nibbles
    // Each contiguous sequence of digit nibbles must match a digit run in the name
    int name_len = (int) strlen(name);
    int i = 0;

    while (i < 6) {

        // Skip 0xF padding/separators
        if (digits[i] > 9) {

            i++;
            continue;

        }

        // Found start of a digit sequence in the search query
        uint8_t seq[6];
        int seq_len = 0;

        while (i < 6 && digits[i] <= 9) {

            seq[seq_len++] = digits[i];
            i++;

        }

        // This digit sequence must match somewhere in the name
        bool seq_matched = false;

        for (int p = 0; p < name_len && !seq_matched; p++) {

            // Find positions where a digit run starts (no digit immediately before)
            if (name[p] < '0' || name[p] > '9') { continue; }
            if (p > 0 && name[p - 1] >= '0' && name[p - 1] <= '9') { continue; }

            // Match search digits against digit characters in name starting at p
            int si = 0;
            int np = p;

            while (si < seq_len && np < name_len) {

                // Skip non-digit characters in name
                if (name[np] < '0' || name[np] > '9') {

                    np++;
                    continue;

                }

                if ((name[np] - '0') != seq[si]) { break; }

                si++;
                np++;

            }

            if (si == seq_len) {

                if (flags & TRAIN_SEARCH_FLAG_EXACT) {

                    // Exact: no digit character immediately following the match
                    // Skip non-digits after match
                    while (np < name_len && (name[np] < '0' || name[np] > '9')) {

                        np++;

                    }

                    if (np >= name_len || name[np] < '0' || name[np] > '9') {

                        seq_matched = true;

                    }

                } else {

                    seq_matched = true;

                }

            }

        }

        if (!seq_matched) { return false; }

    }

    return true;

}

    /** @brief Return true if the train node matches the search query and flags per TrainSearchS §6.3. */
static bool _does_train_match(
            train_state_t *train_state,
            const uint8_t *digits,
            uint16_t search_address,
            uint8_t flags) {

    // Check DCC protocol match
    if (flags & TRAIN_SEARCH_FLAG_DCC) {

        if (flags & TRAIN_SEARCH_FLAG_LONG_ADDR) {

            if (!train_state->is_long_address) { return false; }

        } else {

            if (search_address < 128 && train_state->is_long_address && !(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

                return false;

            }

        }

    }

    // Address match
    if (_does_address_match(train_state->dcc_address, digits, flags)) { return true; }

    // Name match (only when Address Only flag is clear)
    if (!(flags & TRAIN_SEARCH_FLAG_ADDRESS_ONLY) && train_state->owner_node) {

        const node_parameters_t *params = train_state->owner_node->parameters;

        if (params && _does_name_match(params->snip.name, digits, flags)) { return true; }

    }

    return false;

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
    if (!_does_train_match(train_state, digits, search_address, flags)) { return; }

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
