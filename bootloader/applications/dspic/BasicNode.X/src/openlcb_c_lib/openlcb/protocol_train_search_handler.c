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
 * @date 20 Mar 2026
 */

#include "protocol_train_search_handler.h"

#include "openlcb_config.h"

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

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


    /** @brief Return true if the search event contains reserved nibbles (0xA-0xE) or reserved flag bits. */
static bool _has_reserved_values(const uint8_t *digits, uint8_t flags) {

    // Check for reserved nibbles (0xA-0xE) in the address field
    for (int i = 0; i < 6; i++) {

        if (digits[i] >= 0x0A && digits[i] <= 0x0E) {

            return true;

        }

    }

    // Check reserved flag bit 4 (0x10)
    if (flags & 0x10) {

        return true;

    }

    // If DCC flag is not set, lower bits (0-2) must be zero
    if (!(flags & TRAIN_SEARCH_FLAG_DCC) && (flags & 0x07)) {

        return true;

    }

    return false;

}


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

    if (search_digit_count == 0) {

        return false;

    }

    if (flags & TRAIN_SEARCH_FLAG_EXACT) {

        // Exact: search digits must exactly equal the address digits
        if (search_digit_count != train_digit_count) {

            return false;

        }

        for (int i = 0; i < search_digit_count; i++) {

            if (search_digits[i] != train_digits[i]) {

                return false;

            }

        }

        return true;

    } else {

        // Prefix: search digits must be a prefix of the address digits
        if (search_digit_count > train_digit_count) {

            return false;

        }

        for (int i = 0; i < search_digit_count; i++) {

            if (search_digits[i] != train_digits[i]) {

                return false;

            }

        }

        return true;

    }

}

    /** @brief Return true if the search digits match any digit sequence in name per TrainSearchS §6.3. */
static bool _does_name_match(const char *name, const uint8_t *digits, uint8_t flags) {

    if (!name || name[0] == '\0') {

        return false;

    }

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
            if (name[p] < '0' || name[p] > '9') {

                continue;

            }
            if (p > 0 && name[p - 1] >= '0' && name[p - 1] <= '9') {

                continue;

            }

            // Match search digits against digit characters in name starting at p
            int si = 0;
            int np = p;

            while (si < seq_len && np < name_len) {

                // Skip non-digit characters in name
                if (name[np] < '0' || name[np] > '9') {

                    np++;
                    continue;

                }

                if ((name[np] - '0') != seq[si]) {

                    break;

                }

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

        if (!seq_matched) {

            return false;

        }

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

            if (!train_state->is_long_address) {

                return false;

            }

        } else {

            if (!(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

                if (search_address < TRAIN_MAX_DCC_SHORT_ADDRESS && train_state->is_long_address) {

                    return false;

                } else if (search_address >= TRAIN_MAX_DCC_SHORT_ADDRESS && !train_state->is_long_address) {

                    return false;

                }

            }

        }

    }

    // Address match
    if (_does_address_match(train_state->dcc_address, digits, flags)) {

        return true;

    }

    // Name match (only when Address Only flag is clear)
    if (!(flags & TRAIN_SEARCH_FLAG_ADDRESS_ONLY) && train_state->owner_node) {

        const node_parameters_t *params = train_state->owner_node->parameters;

        if (params && _does_name_match(params->snip.name, digits, flags)) {

            return true;

        }

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
    ProtocolTrainSearch_extract_digits(event_id, digits);
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    // Reject searches with reserved nibbles or flag bits
    if (_has_reserved_values(digits, flags)) {

        return;

    }

    uint16_t search_address = ProtocolTrainSearch_digits_to_address(digits);

    // Check if this train matches
    if (!_does_train_match(train_state, digits, search_address, flags)) {

        return;

    }

    // Build reply: Producer Identified Set echoing the queried search event ID
    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            0,
            0,
            MTI_PRODUCER_IDENTIFIED_SET);

    OpenLcbUtilities_copy_event_id_to_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, event_id);

    statemachine_info->outgoing_msg_info.valid = true;

    // Fire callback
    if (_interface && _interface->on_search_matched) {

        _interface->on_search_matched(statemachine_info->openlcb_node, search_address, flags);

    }

}

    /**
     * @brief Handles the no-match case after all train nodes have been checked.
     *
     * @details Called by the main statemachine when the last node in the
     * enumeration has been reached and no train node matched the search query.
     * If the ALLOCATE flag is set in the search event and the on_search_no_match
     * callback is registered, invokes it so the application can create a new
     * virtual train node.
     *
     * @verbatim
     * @param statemachine_info  Pointer to openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit event_id_t containing encoded search query.
     * @endverbatim
     */
void ProtocolTrainSearch_handle_search_no_match(
        openlcb_statemachine_info_t *statemachine_info,
        event_id_t event_id) {

    if (!statemachine_info) {

        return;

    }

    uint8_t digits[6];
    ProtocolTrainSearch_extract_digits(event_id, digits);
    uint8_t flags = ProtocolTrainSearch_extract_flags(event_id);

    // Reject searches with reserved nibbles or flag bits
    if (_has_reserved_values(digits, flags)) {

        return;

    }

    if (!(flags & TRAIN_SEARCH_FLAG_ALLOCATE)) {

        return;

    }

    if (!_interface || !_interface->on_search_no_match) {

        return;

    }

    uint16_t search_address = ProtocolTrainSearch_digits_to_address(digits);

    openlcb_node_t *new_node = _interface->on_search_no_match(search_address, flags);

    if (new_node && new_node->train_state) {

        // Build Producer Identified reply echoing the queried search event ID
        OpenLcbUtilities_load_openlcb_message(
                statemachine_info->outgoing_msg_info.msg_ptr,
                new_node->alias,
                new_node->id,
                0,
                0,
                MTI_PRODUCER_IDENTIFIED_SET);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr, event_id);

        statemachine_info->outgoing_msg_info.valid = true;

    }

}

// =============================================================================
// Train Search Event ID Utilities
//
// Moved from openlcb_utilities.c so the linker only pulls in train-search
// code when OPENLCB_COMPILE_TRAIN and OPENLCB_COMPILE_TRAIN_SEARCH are defined.
// =============================================================================

    /** @brief Returns true if the event ID belongs to the train search space. */
bool ProtocolTrainSearch_is_search_event(event_id_t event_id) {

    return (event_id & TRAIN_SEARCH_MASK) == EVENT_TRAIN_SEARCH_SPACE;

}

    /** @brief Extracts 6 search-query nibbles from a train search event ID into digits[]. */
void ProtocolTrainSearch_extract_digits(event_id_t event_id, uint8_t *digits) {

    if (!digits) {

        return;

    }

    // Bytes 4-6 contain 6 nibbles (bits 31-8)
    // Byte 4 = bits 31-24: nibbles 0 and 1
    // Byte 5 = bits 23-16: nibbles 2 and 3
    // Byte 6 = bits 15-8:  nibbles 4 and 5

    uint32_t lower = (uint32_t)(event_id & 0xFFFFFFFF);

    digits[0] = (uint8_t)((lower >> 28) & 0x0F);
    digits[1] = (uint8_t)((lower >> 24) & 0x0F);
    digits[2] = (uint8_t)((lower >> 20) & 0x0F);
    digits[3] = (uint8_t)((lower >> 16) & 0x0F);
    digits[4] = (uint8_t)((lower >> 12) & 0x0F);
    digits[5] = (uint8_t)((lower >> 8) & 0x0F);

}

    /** @brief Extracts the flags byte (byte 7) from a train search event ID. */
uint8_t ProtocolTrainSearch_extract_flags(event_id_t event_id) {

    return (uint8_t)(event_id & 0xFF);

}

    /** @brief Converts a 6-nibble digit array to a numeric DCC address, skipping leading 0xF nibbles. */
uint16_t ProtocolTrainSearch_digits_to_address(const uint8_t *digits) {

    if (!digits) {

        return 0;

    }

    uint16_t address = 0;

    for (int i = 0; i < 6; i++) {

        if (digits[i] <= 9) {

            address = address * 10 + digits[i];

        }

    }

    return address;

}

    /** @brief Creates a train search event ID from a DCC address and flags byte. */
event_id_t ProtocolTrainSearch_create_event_id(uint16_t address, uint8_t flags) {

    // Encode address as decimal digits into 6 nibbles, right-justified, padded with 0xF
    uint8_t digits[6];
    int i;

    for (i = 0; i < 6; i++) {

        digits[i] = 0x0F;

    }

    // Fill from right to left with decimal digits
    i = 5;
    if (address == 0) {

        digits[i] = 0;

    } else {

        while (address > 0 && i >= 0) {

            digits[i] = (uint8_t)(address % 10);
            address /= 10;
            i--;

        }

    }

    // Build the lower 4 bytes: 6 nibbles + flags byte
    uint32_t lower = 0;
    lower |= ((uint32_t)digits[0] << 28);
    lower |= ((uint32_t)digits[1] << 24);
    lower |= ((uint32_t)digits[2] << 20);
    lower |= ((uint32_t)digits[3] << 16);
    lower |= ((uint32_t)digits[4] << 12);
    lower |= ((uint32_t)digits[5] << 8);
    lower |= (uint32_t)flags;

    return EVENT_TRAIN_SEARCH_SPACE | (event_id_t)lower;

}

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */
