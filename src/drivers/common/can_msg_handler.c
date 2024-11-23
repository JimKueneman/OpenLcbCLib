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

void _flush_alias_node_id_mappings() {

    int i = 0;

    i = i + 1;

    //   TODO:  

}

uint8_t _check_for_hard_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg) {

    can_msg_t out_msg;

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_message(can_msg)) {

        out_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMR | can_node->alias;
        CanUtilities_copy_node_id_to_payload(&out_msg, can_node->id, 0);

        if (CanTxStatemachine_try_transmit_can_message(&out_msg)) {

            can_node->state.can_msg_handled = TRUE;
            can_node->state.permitted = 0;
            can_node->state.initalized = 0;
            can_node->state.run_state = RUNSTATE_GENERATE_SEED;

        }

        return TRUE;

    } else

        return FALSE;

}

uint8_t _check_for_soft_alias_conflict(openlcb_node_t* can_node, can_msg_t* can_msg) {


    can_msg_t out_msg;

    if (can_node->alias == CanUtilities_extract_source_alias_from_can_message(can_msg)) {

        out_msg.payload_count = 0;
        out_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(&out_msg))

            can_node->state.can_msg_handled = TRUE;

        return TRUE;

    }

    return FALSE;

}

void Handle_Incoming_CID(openlcb_node_t* can_node, can_msg_t* can_msg) {

    if (!_check_for_soft_alias_conflict(can_node, can_msg))

        can_node->state.can_msg_handled = TRUE;

}

void Handle_AME_Message(openlcb_node_t* can_node, can_msg_t* msg) {

    can_node->state.can_msg_handled = TRUE;

}

void Handle_Incoming_RID(openlcb_node_t* can_node, can_msg_t* can_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg))

        can_node->state.can_msg_handled = TRUE;

}

void Handle_Incoming_AMD(openlcb_node_t* can_node, can_msg_t* can_msg) {

    if (!_check_for_hard_alias_conflict(can_node, can_msg))

        can_node->state.can_msg_handled = TRUE;

}

void Handle_Incoming_AME(openlcb_node_t* can_node, can_msg_t* can_msg) {


    // Someone is requesting we reply with Alias Mapping Definitions for our Node(s)

    if (can_msg->payload_count == 0)
        _flush_alias_node_id_mappings;

    if (_check_for_hard_alias_conflict(can_node, can_msg))

        return;

    can_msg_t out_msg;

    if ((can_msg->payload_count == 0) || (can_node->id == CanUtilities_extract_can_payload_as_node_id(can_msg))) {

        CanUtilities_copy_node_id_to_payload(&out_msg, can_node->id, 0);
        out_msg.identifier = RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | can_node->alias;

        if (CanTxStatemachine_try_transmit_can_message(&out_msg)) {

            can_node->state.can_msg_handled = TRUE;

        }

    } else {

        can_node->state.can_msg_handled = TRUE;

    }

}

void Handle_Incoming_AMR(openlcb_node_t* can_node, can_msg_t* can_msg) {

    // printf("AMR\n");

    can_node->state.can_msg_handled = TRUE;

}

void CanMessageHandler_process(openlcb_node_t* can_node, can_msg_t* can_msg) {

    if (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

        switch (can_msg->identifier & MASK_CAN_FRAME_SEQUENCE_NUMBER) {

            case CAN_CONTROL_FRAME_CID7:
            case CAN_CONTROL_FRAME_CID6:
            case CAN_CONTROL_FRAME_CID5:
            case CAN_CONTROL_FRAME_CID4:

                Handle_Incoming_CID(can_node, can_msg);

                return;

            case CAN_CONTROL_FRAME_CID3:
            case CAN_CONTROL_FRAME_CID2:
            case CAN_CONTROL_FRAME_CID1:

                can_node->state.can_msg_handled = TRUE;

                return;

            default:

                can_node->state.can_msg_handled = TRUE;

                return;

        }

    } else {

        switch (can_msg->identifier & MASK_CAN_VARIABLE_FIELD) {

            case CAN_CONTROL_FRAME_RID: // Reserve ID

                Handle_Incoming_RID(can_node, can_msg);

                return;

            case CAN_CONTROL_FRAME_AMD: // Alias Map Definition

                Handle_Incoming_AMD(can_node, can_msg);

                return;

            case CAN_CONTROL_FRAME_AME:

                Handle_Incoming_AME(can_node, can_msg);

                return;

            case CAN_CONTROL_FRAME_AMR:

                Handle_Incoming_AMR(can_node, can_msg);

                return;

            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2:
            case CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3:
                // Advanced feature for gateways/routers/etc.

                can_node->state.can_msg_handled = TRUE;

                return;

            default:

                can_node->state.can_msg_handled = TRUE;

                return;

        }

    }

}
