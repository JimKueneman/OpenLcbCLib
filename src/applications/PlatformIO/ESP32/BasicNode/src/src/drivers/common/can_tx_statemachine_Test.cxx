#include "test/main_Test.hxx"

#include "can_tx_statemachine.h"

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"

bool _handle_addressed_msg_frame_called = false;
bool _handle_unaddressed_msg_frame_called = false;
bool _handle_datagram_frame_called = false;
bool _handle_stream_frame_called = false;
bool _handle_can_frame_frame_called = false;
bool _is_can_tx_buffer_empty_called = false;
bool _is_can_tx_buffer_empty_diabled = false;
bool _fail_handle_stream_frame = false;

can_msg_t sent_can_msg;

bool _handle_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index)
{

    _handle_addressed_msg_frame_called = true;

    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {

        *openlcb_start_index = openlcb_msg->payload_count;
    }

    return true;
}

bool _handle_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index)
{

    _handle_unaddressed_msg_frame_called = true;

    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {

        *openlcb_start_index = openlcb_msg->payload_count;
    }

    return true;
}

bool _handle_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index)
{

    _handle_datagram_frame_called = true;

    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {

        *openlcb_start_index = openlcb_msg->payload_count;
    }

    return true;
}

bool _handle_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, uint16_t *openlcb_start_index)
{

    _handle_stream_frame_called = true;

    if (_fail_handle_stream_frame)
    {

        return false;
    }

    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {

        *openlcb_start_index = openlcb_msg->payload_count;
    }

    return true;
}

bool _handle_can_frame(can_msg_t *can_msg)
{

    _handle_can_frame_frame_called = true;

    CanUtilities_copy_can_message(can_msg, &sent_can_msg);

    return true;
}

bool _is_can_tx_buffer_empty(void)
{

    _is_can_tx_buffer_empty_called = true;

    if (_is_can_tx_buffer_empty_diabled)
    {

        return false;
    }

    return true;
}

bool compare_can_msg(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t bytes[])
{

    bool result = (can_msg->identifier == identifier) &&
                  (can_msg->payload_count == payload_size);

    if (!result)
    {

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (can_msg->payload[i] != bytes[i])
        {

            return false;
        }
    }

    return true;
}

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {

    .is_tx_buffer_empty = &_is_can_tx_buffer_empty,
    .handle_addressed_msg_frame = &_handle_addressed_msg_frame,
    .handle_can_frame = &_handle_can_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame

};

void _reset_variables(void)
{

    _handle_addressed_msg_frame_called = false;
    _handle_unaddressed_msg_frame_called = false;
    _handle_datagram_frame_called = false;
    _handle_stream_frame_called = false;
    _handle_can_frame_frame_called = false;
    _is_can_tx_buffer_empty_called = false;
    _is_can_tx_buffer_empty_diabled = false;
    _fail_handle_stream_frame = false;
}

void _initialize(void)
{

    OpenLcbBufferStore_initialize();
    CanTxStatemachine_initialize(&interface_can_tx_statemachine);
}

TEST(CanTxStatemachine, initialize)
{

    _reset_variables();
    _initialize();
}

TEST(CanTxStatemachine, send_can_message)
{

    can_msg_t can_msg;

    _reset_variables();
    _initialize();

    CanUtilities_load_can_message(&can_msg, 0x170506be, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    EXPECT_TRUE(CanTxStatemachine_send_can_message(&can_msg));
    EXPECT_TRUE(_handle_can_frame_frame_called);
    EXPECT_TRUE(compare_can_msg(&can_msg, sent_can_msg.identifier, sent_can_msg.payload_count, sent_can_msg.payload));
    _handle_addressed_msg_frame_called = false;
}

TEST(CanTxStatemachine, send_openlcb_message)
{
    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {
        // ********************************************************************
        // Verify node ID global is unaddressed and no data
        // ********************************************************************
        _reset_variables();

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_VERIFY_NODE_ID_GLOBAL);
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_TRUE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        // ********************************************************************
        // Verify Node ID addressed and data
        // ********************************************************************
        _reset_variables();

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_VERIFY_NODE_ID_ADDRESSED);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_TRUE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        // ********************************************************************
        // Single Datagram frame addressed and data
        // ********************************************************************
        _reset_variables();

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_DATAGRAM);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_TRUE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        // ********************************************************************
        // Multiframe Datagram frame addressed and data
        // ********************************************************************
        _reset_variables();

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_DATAGRAM);
        openlcb_msg->payload_count = 24;
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_TRUE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);

        // ********************************************************************

        // ********************************************************************
        // Stream frame
        // ********************************************************************
        _reset_variables();

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_STREAM_PROCEED);
        openlcb_msg->payload_count = 8;
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_TRUE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        // ********************************************************************
        // Verify node ID but buffer is not empty
        // ********************************************************************
        _reset_variables();
        _is_can_tx_buffer_empty_diabled = true;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_VERIFY_NODE_ID_GLOBAL);
        EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        // ********************************************************************
        // Stream frame but send failed
        // ********************************************************************
        _reset_variables();
        _fail_handle_stream_frame = true;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x060504030201, MTI_STREAM_PROCEED);
        openlcb_msg->payload_count = 8;
        EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_TRUE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        // ********************************************************************

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}