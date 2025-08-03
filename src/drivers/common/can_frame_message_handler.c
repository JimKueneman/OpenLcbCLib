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
 * \file can_frame_message_hander.c
 *
 * As CAN only frame messages come in they need to be processed by the node(s) to see
 * if there is a response required.  These are the handlers called by the CAN main
 * statemachine to accomplish that.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

#include "can_frame_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_buffer_store.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_utilities.h"
#include "can_tx_statemachine.h"

void _flush_alias_node_id_mappings(void) {

    int i = 0;

    i = i + 1;

    //   TODO:  

}

uint8_t _check_for_hard_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_identifier(can_msg)) {

        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | can_node->alias;
        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = true;
            can_node->state.permitted = 0;
            can_node->state.initalized = 0;
            can_node->state.run_state = RUNSTATE_GENERATE_SEED;

        }

        return true;

    } else

        return false;

}

uint8_t _check_for_soft_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_identifier(can_msg)) {

        worker_msg->payload_count = 0;
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg))

            can_node->state.can_msg_handled = true;

        return true;

    }

    return false;

}

void CanFrameMessageHandler_cid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_soft_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_rid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_amd(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_ame(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {


    // Someone is requesting we reply with Alias Mapping Definitions for our Node(s)

    if (can_msg->payload_count == 0) {

        _flush_alias_node_id_mappings();

    }

    if (_check_for_hard_alias_conflict(can_node, can_msg, worker_msg)) {

        return;

    }

    if ((can_msg->payload_count == 0) || (can_node->id == CanUtilities_extract_can_payload_as_node_id(can_msg))) {

        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = true;

        }

    } else {

        can_node->state.can_msg_handled = true;

    }

}

void CanFrameMessageHandler_amr(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    // printf("AMR\n");

    can_node->state.can_msg_handled = true;

}