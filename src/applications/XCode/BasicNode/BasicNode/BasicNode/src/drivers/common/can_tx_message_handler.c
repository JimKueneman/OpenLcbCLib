/*
 * File:   can_tx_message_handler.c
 * Author: jimkueneman
 *
 * Created on August 17, 2025, 5:47 AM
 */


#include "can_tx_message_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_utilities.h"

const uint32_t OPENLCB_MESSAGE_DATAGRAM_ONLY = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_ONLY;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FIRST;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_MIDDLE;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_DATAGRAM_FINAL;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_UNADDRESSED = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED;
const uint32_t OPENLCB_MESSAGE_DATAGRAM_ADDRESSED = RESERVED_TOP_BIT | CAN_OPENLCB_MSG | CAN_FRAME_TYPE_GLOBAL_ADDRESSED;

static interface_can_tx_message_handler_t *_interface;

void CanTxMessageHandler_initialize(const interface_can_tx_message_handler_t *interface_can_tx_message_handler) {

    _interface = (interface_can_tx_message_handler_t*) interface_can_tx_message_handler;

}

static uint32_t _construct_identfier_datagram_only_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_ONLY | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

static uint32_t _construct_identfier_datagram_first_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_FIRST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

static uint32_t _construct_identfier_datagram_middle_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_MIDDLE_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

static uint32_t _construct_identfier_datagram_last_frame(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_LAST_FRAME | ((uint32_t) (openlcb_msg->dest_alias) << 12) | openlcb_msg->source_alias);

}

static uint32_t _construct_unaddressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_UNADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

static uint32_t _construct_addressed_message_identifier(openlcb_msg_t* openlcb_msg) {

    return (OPENLCB_MESSAGE_DATAGRAM_ADDRESSED | ((uint32_t) (openlcb_msg->mti & 0x0FFF) << 12) | openlcb_msg->source_alias);

}

static bool _transmit_can_frame(can_msg_t* can_msg) {
    
    bool result = _interface->transmit_can_frame(can_msg);

    if (_interface->on_transmit && result) {

        _interface->on_transmit(can_msg);

    }

    return result;

}

static bool _datagram_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_only_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

static bool _datagram_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_first_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

static bool _datagram_middle_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_middle_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

static bool _datagram_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker) {

    can_msg_worker->identifier = _construct_identfier_datagram_last_frame(openlcb_msg);
    return _transmit_can_frame(can_msg_worker);

}

static bool _addressed_message_only_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_ONLY);
    return _transmit_can_frame(can_msg);

}

static bool _addressed_message_first_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FIRST);
    return _transmit_can_frame(can_msg);

}

static bool _addressed_message_middle_frame(can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_MIDDLE);
    return _transmit_can_frame(can_msg);

}

static bool _addressed_message_last_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    OpenLcbUtilities_set_multi_frame_flag(&can_msg->payload[0], MULTIFRAME_FINAL);
    return _transmit_can_frame(can_msg);

}

static void _load_destination_address_in_payload(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg) {

    can_msg->payload[0] = (openlcb_msg->dest_alias >> 8) & 0xFF; // Setup the first two CAN data bytes with the destination address
    can_msg->payload[1] = openlcb_msg->dest_alias & 0xFF;

}

bool CanTxMessageHandler_datagram_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    bool result = false;
    uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY) {

        result = _datagram_only_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index < LEN_CAN_BYTE_ARRAY) {

        result = _datagram_first_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index + len_msg_frame < openlcb_msg->payload_count) {

        result = _datagram_middle_frame(openlcb_msg, can_msg_worker);

    } else {

        result = _datagram_last_frame(openlcb_msg, can_msg_worker);

    }

    if (result) {

        *openlcb_start_index = *openlcb_start_index + len_msg_frame;

    }

    return result;

}

bool CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    bool result = false;

    if (openlcb_msg->payload_count <= LEN_CAN_BYTE_ARRAY) { // single frame

        uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITHOUT_DEST_ADDRESS);
        can_msg_worker->identifier = _construct_unaddressed_message_identifier(openlcb_msg);

        result = _transmit_can_frame(can_msg_worker);

        if (result) {

            *openlcb_start_index = *openlcb_start_index + len_msg_frame;

        }

    } else { // multi frame

        // TODO: Is there such a thing as a unaddressed multi frame?

    }
    
    return result;

}

bool CanTxMessageHandler_addressed_msg_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    _load_destination_address_in_payload(openlcb_msg, can_msg_worker);


    can_msg_worker->identifier = _construct_addressed_message_identifier(openlcb_msg);
    uint8_t len_msg_frame = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, can_msg_worker, *openlcb_start_index, OFFSET_CAN_WITH_DEST_ADDRESS);
    bool result = false;

    if (openlcb_msg->payload_count <= 6) {// Account for 2 bytes used for dest alias

        result = _addressed_message_only_frame(openlcb_msg, can_msg_worker);

    } else if (*openlcb_start_index < 6) { // Account for 2 bytes used for dest alias

        result = _addressed_message_first_frame(openlcb_msg, can_msg_worker);

    } else if ((*openlcb_start_index + len_msg_frame) < openlcb_msg->payload_count) {

        result = _addressed_message_middle_frame(can_msg_worker);

    } else {

        result = _addressed_message_last_frame(openlcb_msg, can_msg_worker);

    }

    if (result) {

        *openlcb_start_index = *openlcb_start_index + len_msg_frame;

    }

    return result;

}

bool CanTxMessageHandler_stream_frame(openlcb_msg_t* openlcb_msg, can_msg_t* can_msg_worker, uint16_t *openlcb_start_index) {

    // ToDo: implement streams
    return true;
}

bool CanTxMessageHandler_can_frame(can_msg_t* can_msg) {

    return _transmit_can_frame(can_msg);

}
