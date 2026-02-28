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
 * @file protocol_train_search_handler.h
 * @brief Train Search Protocol message handler
 *
 * @details Handles incoming train search Event IDs from the network.
 * Decodes the search query (DCC address, flags) and compares against each
 * train node's address. Matching nodes reply with a Producer Identified
 * event containing their own address.
 *
 * Called from the main statemachine when a train search event is detected.
 * Unlike broadcast time (node index 0 only), train search is called for
 * every train node so each can check for a match.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see openlcb_application_train.h - Train state with DCC address
 * @see openlcb_utilities.h - Event ID encoding/decoding functions
 */

#ifndef __OPENLCB_PROTOCOL_TRAIN_SEARCH_HANDLER__
#define __OPENLCB_PROTOCOL_TRAIN_SEARCH_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @struct interface_protocol_train_search_handler_t
     * @brief Application callbacks for train search events.
     *
     * All callbacks are optional (can be NULL).
     */
    typedef struct {

            /** @brief Called when a search matches this train node. */
        void (*on_search_matched)(openlcb_node_t *openlcb_node, uint16_t search_address, uint8_t flags);

            /** @brief Called when no train node matches (allocate case, deferred). */
        openlcb_node_t* (*on_search_no_match)(uint16_t search_address, uint8_t flags);

    } interface_protocol_train_search_handler_t;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the Train Search Protocol handler.
     *
     * @param interface  Pointer to @ref interface_protocol_train_search_handler_t callbacks (may be NULL).
     */
    extern void ProtocolTrainSearch_initialize(const interface_protocol_train_search_handler_t *interface);

    /**
     * @brief Handles incoming train search events.
     *
     * @details Called from the main statemachine MTI_PC_EVENT_REPORT case
     * for each train node. Decodes the search query, compares against this
     * node's DCC address, and if matching, loads a Producer Identified reply.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit @ref event_id_t containing encoded search query.
     */
    extern void ProtocolTrainSearch_handle_search_event(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_TRAIN_SEARCH_HANDLER__ */
