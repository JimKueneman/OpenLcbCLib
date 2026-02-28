/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * @file protocol_broadcast_time_handler.h
 * @brief Broadcast Time Protocol message handler
 *
 * @details Handles incoming broadcast time Event IDs from the network.
 * Decodes time/date/year/rate/command data and updates the singleton clock
 * state in openlcb_application_broadcast_time module.
 *
 * Called from the main statemachine when a broadcast time event is detected.
 * Only processes events for node index 0 (broadcast time events are global).
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see openlcb_application_broadcast_time.h - Singleton clock state and API
 * @see openlcb_utilities.h - Event ID encoding/decoding functions
 */

#ifndef __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__
#define __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @struct interface_openlcb_protocol_broadcast_time_handler_t
     * @brief Application callbacks for broadcast time events.
     *
     * All callbacks are optional (can be NULL).
     */
    typedef struct {

            /** @brief Time-of-day updated.  Optional. */
        void (*on_time_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Date updated.  Optional. */
        void (*on_date_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Year updated.  Optional. */
        void (*on_year_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock rate changed.  Optional. */
        void (*on_rate_received)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock started.  Optional. */
        void (*on_clock_started)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Clock stopped.  Optional. */
        void (*on_clock_stopped)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);
            /** @brief Date rollover occurred.  Optional. */
        void (*on_date_rollover)(openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state);

    } interface_openlcb_protocol_broadcast_time_handler_t;

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the Broadcast Time Protocol handler.
     *
     * @param interface_openlcb_protocol_broadcast_time_handler  Pointer to @ref interface_openlcb_protocol_broadcast_time_handler_t (must remain valid for application lifetime).
     */
    extern void ProtocolBroadcastTime_initialize(const interface_openlcb_protocol_broadcast_time_handler_t *interface_openlcb_protocol_broadcast_time_handler);

    /**
     * @brief Handles incoming broadcast time events.
     *
     * @details Decodes the Event ID and updates the singleton clock state.
     * Only processes if the node has index == 0 and a matching clock is
     * registered in the application broadcast time module.
     *
     * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
     * @param event_id           Full 64-bit @ref event_id_t containing encoded time data.
     */
    extern void ProtocolBroadcastTime_handle_time_event(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

#ifdef __cplusplus
}
#endif

#endif /* __OPENLCB_PROTOCOL_BROADCAST_TIME_HANDLER__ */
