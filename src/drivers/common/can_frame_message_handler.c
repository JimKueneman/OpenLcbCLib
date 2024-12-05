/*
 * File:   can_msg_handler.c
 * Author: jimkueneman
 *
 * Created on November 21, 2024, 6:57 AM
 */


#include "xc.h"
#include "can_types.h"
#include "can_buffer_store.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_utilities.h"
#include "can_tx_statemachine.h"

#include "../../applications/test/main_statemachine/main_statemachine.X/debug.h"

void _flush_alias_node_id_mappings() {

    int i = 0;

    i = i + 1;

    //   TODO:  

}

uint8_t _check_for_hard_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_message(can_msg)) {

        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | can_node->alias;
        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = TRUE;
            can_node->state.permitted = 0;
            can_node->state.initalized = 0;
            can_node->state.run_state = RUNSTATE_GENERATE_SEED;

        }

        return TRUE;

    } else

        return FALSE;

}

uint8_t _check_for_soft_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_message(can_msg)) {

        worker_msg->payload_count = 0;
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg))

            can_node->state.can_msg_handled = TRUE;

        return TRUE;

    }

    return FALSE;

}

void CanFrameMessageHandler_cid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_soft_alias_conflict(can_node, can_msg, worker_msg))

        can_node->state.can_msg_handled = TRUE;

}

void CanFrameMessageHandler_rid(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg))

        can_node->state.can_msg_handled = TRUE;

}

void CanFrameMessageHandler_amd(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg, worker_msg))

        can_node->state.can_msg_handled = TRUE;

}

void CanFrameMessageHandler_ame(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {


    // Someone is requesting we reply with Alias Mapping Definitions for our Node(s)

    if (can_msg->payload_count == 0)
        _flush_alias_node_id_mappings;

    if (_check_for_hard_alias_conflict(can_node, can_msg, worker_msg))

        return;

    if ((can_msg->payload_count == 0) || (can_node->id == CanUtilities_extract_can_payload_as_node_id(can_msg))) {

        CanUtilities_copy_node_id_to_payload(worker_msg, can_node->id, 0);
        worker_msg->identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(worker_msg)) {

            can_node->state.can_msg_handled = TRUE;

        }

    } else {

        can_node->state.can_msg_handled = TRUE;

    }

}

void CanFrameMessageHandler_amr(openlcb_node_t* can_node, can_msg_t* can_msg, can_msg_t* worker_msg) {

    // printf("AMR\n");

    can_node->state.can_msg_handled = TRUE;

}