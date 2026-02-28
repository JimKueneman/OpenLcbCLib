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
* @file openlcb_main_statemachine.h
* @brief Central MTI-based message dispatcher.  Pops messages from the FIFO,
*        enumerates all nodes, and routes to the correct protocol handler via
*        function pointers.  NULL optional handlers trigger Interaction Rejected
*        automatically.
*
* @author Jim Kueneman
* @date 28 Feb 2026
*/

#ifndef __OPENLCB_OPENLCB_MAIN_STATEMACHINE__
#define __OPENLCB_OPENLCB_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /** @brief Dependency-injection interface for the main state machine.  Required
     *         pointers must be non-NULL; optional ones may be NULL (causes automatic
     *         Interaction Rejected).  Internal pointers are exposed for unit testing. */
typedef struct {

    // =========================================================================
    // Resource Management (all REQUIRED)
    // =========================================================================

        /** @brief Disable interrupts / acquire mutex.  Keep short.  REQUIRED. */
    void (*lock_shared_resources)(void);

        /** @brief Re-enable interrupts / release mutex.  REQUIRED. */
    void (*unlock_shared_resources)(void);

        /** @brief Queue a message for transmission.  Return false if buffer full.  REQUIRED. */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);

    // =========================================================================
    // Node Enumeration (all REQUIRED)
    // =========================================================================

        /** @brief Return first node (NULL if none).  key separates concurrent iterations.  REQUIRED. */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /** @brief Return next node (NULL at end).  REQUIRED. */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

    // =========================================================================
    // Core Handlers (all REQUIRED)
    // =========================================================================

        /** @brief Build Optional Interaction Rejected for unhandled MTIs.  REQUIRED. */
    void (*load_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Required Message Network Protocol Handlers
    // =========================================================================

        /** @brief MTI 0x0100 — Initialization Complete.  REQUIRED. */
    void (*message_network_initialization_complete)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0101 — Initialization Complete Simple.  REQUIRED. */
    void (*message_network_initialization_complete_simple)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0488 — Verify Node ID Addressed.  REQUIRED. */
    void (*message_network_verify_node_id_addressed)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0490 — Verify Node ID Global.  REQUIRED. */
    void (*message_network_verify_node_id_global)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0170/0x0171 — Verified Node ID.  REQUIRED. */
    void (*message_network_verified_node_id)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0068 — Optional Interaction Rejected (received).  REQUIRED. */
    void (*message_network_optional_interaction_rejected)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x00A8 — Terminate Due to Error.  REQUIRED. */
    void (*message_network_terminate_due_to_error)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Required Protocol Support (PIP) Handlers
    // =========================================================================

        /** @brief MTI 0x0828 — Protocol Support Inquiry.  REQUIRED. */
    void (*message_network_protocol_support_inquiry)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0668 — Protocol Support Reply (received).  REQUIRED. */
    void (*message_network_protocol_support_reply)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Internal functions (exposed for unit testing)
    // =========================================================================

        /** @brief MTI dispatcher — routes incoming message to the correct handler. */
    void (*process_main_statemachine)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Address filter — returns true if node should process this message. */
    bool (*does_node_process_msg)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief Try to send the pending outgoing message; returns true if one was pending. */
    bool (*handle_outgoing_openlcb_message)(void);

        /** @brief Re-enter the state processor if the enumerate flag is set. */
    bool (*handle_try_reenumerate)(void);

        /** @brief Pop next incoming message from the FIFO (thread-safe). */
    bool (*handle_try_pop_next_incoming_openlcb_message)(void);

        /** @brief Start enumeration from the first node. */
    bool (*handle_try_enumerate_first_node)(void);

        /** @brief Advance to the next node; frees message when enumeration completes. */
    bool (*handle_try_enumerate_next_node)(void);

    // =========================================================================
    // Optional SNIP Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x0DE8 — Simple Node Info Request.  Optional. */
    void (*snip_simple_node_info_request)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0A08 — Simple Node Info Reply (received).  Optional. */
    void (*snip_simple_node_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Event Transport Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x08F4 — Identify Consumer.  Optional. */
    void (*event_transport_consumer_identify)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x04A4 — Consumer Range Identified.  Optional. */
    void (*event_transport_consumer_range_identified)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x04C7 — Consumer Identified Unknown.  Optional. */
    void (*event_transport_consumer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x04C4 — Consumer Identified Set.  Optional. */
    void (*event_transport_consumer_identified_set)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x04C5 — Consumer Identified Clear.  Optional. */
    void (*event_transport_consumer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x04C6 — Consumer Identified Reserved.  Optional. */
    void (*event_transport_consumer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0914 — Identify Producer.  Optional. */
    void (*event_transport_producer_identify)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0524 — Producer Range Identified.  Optional. */
    void (*event_transport_producer_range_identified)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0547 — Producer Identified Unknown.  Optional. */
    void (*event_transport_producer_identified_unknown)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0544 — Producer Identified Set.  Optional. */
    void (*event_transport_producer_identified_set)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0545 — Producer Identified Clear.  Optional. */
    void (*event_transport_producer_identified_clear)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0546 — Producer Identified Reserved.  Optional. */
    void (*event_transport_producer_identified_reserved)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0968 — Identify Events Addressed.  Optional. */
    void (*event_transport_identify_dest)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0970 — Identify Events Global.  Optional. */
    void (*event_transport_identify)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0594 — Learn Event.  Optional. */
    void (*event_transport_learn)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x05B4 — PC Event Report.  Optional. */
    void (*event_transport_pc_report)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x05F4 — PC Event Report with Payload.  Optional. */
    void (*event_transport_pc_report_with_payload)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Train Protocol Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x05EB — Train Control Command.  Optional. */
    void (*train_control_command)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x01E9 — Train Control Reply (received).  Optional. */
    void (*train_control_reply)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Train SNIP Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x0DA8 — Simple Train Node Ident Info Request.  Optional. */
    void (*simple_train_node_ident_info_request)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0A48 — Simple Train Node Ident Info Reply (received).  Optional. */
    void (*simple_train_node_ident_info_reply)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Datagram Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x1C48 — Datagram.  Must reply OK or Rejected.  Optional. */
    void (*datagram)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0A28 — Datagram Received OK (received).  Optional. */
    void (*datagram_ok_reply)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0A48 — Datagram Rejected (received).  Optional. */
    void (*datagram_rejected_reply)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Stream Handlers (NULL = Interaction Rejected)
    // =========================================================================

        /** @brief MTI 0x0CC8 — Stream Initiate Request.  Optional. */
    void (*stream_initiate_request)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0868 — Stream Initiate Reply (received).  Optional. */
    void (*stream_initiate_reply)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x1F88 — Stream Data Send.  Optional. */
    void (*stream_send_data)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x0888 — Stream Data Proceed (received).  Optional. */
    void (*stream_data_proceed)(openlcb_statemachine_info_t *statemachine_info);

        /** @brief MTI 0x08A8 — Stream Data Complete (received).  Optional. */
    void (*stream_data_complete)(openlcb_statemachine_info_t *statemachine_info);

    // =========================================================================
    // Optional Broadcast Time Handler (NULL = not implemented)
    // =========================================================================

        /** @brief Called by the event transport handler for broadcast time Event IDs.  Optional. */
    void (*broadcast_time_event_handler)(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

    // =========================================================================
    // Optional Train Search Handler (NULL = not implemented)
    // =========================================================================

        /** @brief Called for train search events; dispatched to every train node.  Optional. */
    void (*train_search_event_handler)(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

    // =========================================================================
    // Optional Train Emergency Event Handler (NULL = not implemented)
    // =========================================================================

        /** @brief Called for well-known emergency events; dispatched to every train node.  Optional. */
    void (*train_emergency_event_handler)(openlcb_statemachine_info_t *statemachine_info, event_id_t event_id);

} interface_openlcb_main_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface and prepares internal state.
         *
         * @details Call once at startup after buffer/FIFO initialization and before
         *          OpenLcbMainStatemachine_run().
         *
         * @param interface_openlcb_main_statemachine  Pointer to @ref interface_openlcb_main_statemachine_t.  Must remain valid for application lifetime.
         */
    extern void OpenLcbMainStatemachine_initialize(
            const interface_openlcb_main_statemachine_t *interface_openlcb_main_statemachine);

        /**
         * @brief Runs one non-blocking step of protocol processing.
         *
         * @details Priority order: send pending → re-enumerate → pop FIFO → enumerate nodes.
         *          Call as fast as possible from your main loop.
         */
    extern void OpenLcbMainStatemachine_run(void);

        /**
         * @brief Builds an Interaction Rejected response for the current incoming message.  Internal use.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void OpenLcbMainStatemachine_load_interaction_rejected(
            openlcb_statemachine_info_t *statemachine_info);

    // ---- Internal functions exposed for unit testing ----

        /** @brief Tries to send the pending message.  Returns true if one was pending. */
    extern bool OpenLcbMainStatemachine_handle_outgoing_openlcb_message(void);

        /** @brief Re-enters the state processor if the enumerate flag is set. */
    extern bool OpenLcbMainStatemachine_handle_try_reenumerate(void);

        /** @brief Pops the next message from the FIFO (thread-safe).  Returns true if popped. */
    extern bool OpenLcbMainStatemachine_handle_try_pop_next_incoming_openlcb_message(void);

        /** @brief Starts node enumeration from the first node.  Returns true if processed. */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_first_node(void);

        /** @brief Advances to the next node; frees message at end.  Returns true if processed. */
    extern bool OpenLcbMainStatemachine_handle_try_enumerate_next_node(void);

        /**
         * @brief MTI dispatcher.  Routes incoming message to the correct handler.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern void OpenLcbMainStatemachine_process_main_statemachine(
            openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Address filter.  Returns true if the node should process this message.
         *
         * @param statemachine_info  Pointer to @ref openlcb_statemachine_info_t context.
         */
    extern bool OpenLcbMainStatemachine_does_node_process_msg(
            openlcb_statemachine_info_t *statemachine_info);

        /** @brief Returns pointer to internal static state machine info.  For unit testing only — do not modify. */
    extern openlcb_statemachine_info_t *OpenLcbMainStatemachine_get_statemachine_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_MAIN_STATEMACHINE__ */
