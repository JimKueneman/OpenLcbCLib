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
 * \file protocol_event_transport.h
 *
 * Implementation of the Openlcb event protocol. Handlers are call from the
 * openlcb_main_statemachine.c processing when a datagram message is being processed
 * from the FIFO buffer.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

/*
 * NOTE:  All Function for all Protocols expect that the incoming CAN messages have been
 *        blocked so there is not a race on the incoming message buffer.
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __PROTOCOL_EVENT_TRANSPORT__
#define __PROTOCOL_EVENT_TRANSPORT__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct
{

    // Callback events
    void (*on_consumer_range_identified)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_consumer_identified_unknown)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_consumer_identified_set)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_consumer_identified_clear)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_consumer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);
    void (*on_producer_range_identified)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_producer_identified_unknown)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_producer_identified_set)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_producer_identified_clear)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_producer_identified_reserved)(openlcb_node_t *openlcb_node, event_id_t *event_id);
    void (*on_event_learn)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_pc_event_report)(openlcb_node_t* openlcb_node, event_id_t* event_id);
    void (*on_pc_event_report_with_payload)(openlcb_node_t* node, event_id_t* event_id, uint16_t count, event_payload_t* payload);

} interface_openlcb_protocol_event_transport_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef SUPPORT_FIRMWARE_BOOTLOADER

    extern void ProtocolEventTransport_initialize(const interface_openlcb_protocol_event_transport_t *interface_openlcb_protocol_event_transport);

    extern bool ProtocolEventTransport_handle_consumer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_consumer_range_identified(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_consumer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_consumer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_consumer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_consumer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_range_identified(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_identified_unknown(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_identified_set(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_identified_clear(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_producer_identified_reserved(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_events_identify_dest(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_events_identify(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_event_learn(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_pc_event_report(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern bool ProtocolEventTransport_handle_pc_event_report_with_payload(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg);

    extern uint16_t ProtocolEventTransport_extract_consumer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);

    extern uint16_t ProtocolEventTransport_extract_producer_event_status_mti(openlcb_node_t *openlcb_node, uint16_t event_index);


#endif // SUPPORT_FIRMWARE_BOOTLOADER

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PROTOCOL_EVENT_TRANSPORT__ */
