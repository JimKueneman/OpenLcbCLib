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
* @file protocol_event_transport.h
* @brief Event Transport protocol — producer/consumer identification, event
*        reports, and learn/teach operations.  All callbacks are optional.
*
* @author Jim Kueneman
* @date 28 Feb 2026
*/

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */

#ifndef __OPENLCB_PROTOCOL_EVENT_TRANSPORT__
#define __OPENLCB_PROTOCOL_EVENT_TRANSPORT__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Application callbacks for event transport notifications.  All optional (NULL = ignored). */
typedef struct
{

        /** @brief Consumer event identified with status.  Optional. */
    void (*on_consumed_event_identified)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload);

        /** @brief Consumer event report (PCER) received.  Optional. */
    void (*on_consumed_event_pcer)(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload);

        /** @brief Consumer Range Identified from a remote node.  Optional. */
    void (*on_consumer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Unknown from a remote node.  Optional. */
    void (*on_consumer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Set from a remote node.  Optional. */
    void (*on_consumer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Clear from a remote node.  Optional. */
    void (*on_consumer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Consumer Identified Reserved from a remote node.  Optional. */
    void (*on_consumer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Range Identified from a remote node.  Optional. */
    void (*on_producer_range_identified)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Unknown from a remote node.  Optional. */
    void (*on_producer_identified_unknown)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Set from a remote node.  Optional. */
    void (*on_producer_identified_set)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Clear from a remote node.  Optional. */
    void (*on_producer_identified_clear)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Producer Identified Reserved from a remote node.  Optional. */
    void (*on_producer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief Learn Event received — app must store the event ID if in learn mode.  Optional. */
    void (*on_event_learn)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief PC Event Report — an event occurred on the network.  Optional. */
    void (*on_pc_event_report)(openlcb_node_t *openlcb_node, event_id_t *event_id);

        /** @brief PC Event Report with payload — event plus additional data bytes.  Optional. */
    void (*on_pc_event_report_with_payload)(openlcb_node_t *openlcb_node, event_id_t *event_id, uint16_t count, event_payload_t *payload);

} interface_openlcb_protocol_event_transport_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup.
         *
         * @param interface_openlcb_protocol_event_transport  Pointer to @ref interface_openlcb_protocol_event_transport_t (must remain valid for application lifetime).
         */
    extern void ProtocolEventTransport_initialize(const interface_openlcb_protocol_event_transport_t *interface_openlcb_protocol_event_transport);

        /**
         * @brief Responds with Consumer Identified if this node consumes the requested event.
         *
         * @details No response if the event is not in this node's consumer list.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Consumer Range Identified to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_range_identified(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Consumer Identified Unknown to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Consumer Identified Set to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_identified_set(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Consumer Identified Clear to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_identified_clear(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Consumer Identified Reserved to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Responds with Producer Identified if this node produces the requested event.
         *
         * @details No response if the event is not in this node's producer list.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Producer Range Identified to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_range_identified(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Producer Identified Unknown to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_identified_unknown(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Producer Identified Set to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_identified_set(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Producer Identified Clear to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_identified_clear(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Producer Identified Reserved to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_producer_identified_reserved(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Checks addressing then delegates to handle_events_identify for addressed variant.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_events_identify_dest(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Enumerates all producer then consumer events, responding with Identified
         *        messages.  Uses the enumerate flag for multi-message sequencing.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_events_identify(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards Learn Event to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_event_learn(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards PC Event Report to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_pc_event_report(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Forwards PC Event Report with payload to the app callback.  No automatic response.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Returns the Consumer Identified MTI (Unknown/Set/Clear) for consumers.list[event_index].
         *
         * @param openlcb_node   Pointer to @ref openlcb_node_t with the consumer list.
         * @param event_index    Index into consumers.list.
         * @return Appropriate MTI_CONSUMER_IDENTIFIED_xxx value.
         */
    extern uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);

        /**
         * @brief Returns the Producer Identified MTI (Unknown/Set/Clear) for producers.list[event_index].
         *
         * @param openlcb_node   Pointer to @ref openlcb_node_t with the producer list.
         * @param event_index    Index into producers.list.
         * @return Appropriate MTI_PRODUCER_IDENTIFIED_xxx value.
         */
    extern uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_PROTOCOL_EVENT_TRANSPORT__ */
