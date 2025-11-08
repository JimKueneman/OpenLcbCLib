#include "test/main_Test.hxx"

#include "can_tx_message_handler.h"

#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"

bool transmit_can_frame_called = false;
bool application_callback_tx_called = false;
can_msg_t transmitted_can_msg[1024];
bool transmit_can_frame_enabled = true;
uint16_t transmit_can_frame_index = 0;

bool _transmit_can_frame(can_msg_t *can_msg)
{

    transmit_can_frame_called = true;

    if (transmit_can_frame_enabled)
    {

        CanUtilities_copy_can_message(can_msg, &transmitted_can_msg[transmit_can_frame_index]);

        transmit_can_frame_index++;

        return true;
    }

    return false;
}

void _application_callback_tx(can_msg_t *can_msg)
{

    application_callback_tx_called = true;
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

const interface_can_tx_message_handler_t interface_can_tx_message_handler{
    .transmit_can_frame = &_transmit_can_frame,
    .on_transmit = &_application_callback_tx};

const interface_can_tx_message_handler_t interface_can_tx_message_handler_no_callback{
    .transmit_can_frame = &_transmit_can_frame,
    .on_transmit = nullptr};

void _reset_variables(void)
{

    transmit_can_frame_called = false;
    application_callback_tx_called = false;
    transmit_can_frame_enabled = true;
    transmit_can_frame_index = 0;
}

void _global_initalize(void)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();

    CanTxMessageHandler_initialize(&interface_can_tx_message_handler);
}

void _global_initalize_no_callback(void)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();

    CanTxMessageHandler_initialize(&interface_can_tx_message_handler_no_callback);
}

TEST(CanTxMessageHander, initialize)
{
    _reset_variables();
    _global_initalize();
}

TEST(CanTxMessageHandler, handle_can_frame)
{

    can_msg_t can_msg;

    _reset_variables();
    _global_initalize();

    CanUtilities_load_can_message(&can_msg, 0x107016BE, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);

    EXPECT_TRUE(CanTxMessageHandler_can_frame(&can_msg));
    uint8_t bytes[6] = {0x05, 0x01, 0x01, 0x01, 0x07, 0xFF};
    EXPECT_TRUE(compare_can_msg(&can_msg, 0x107016BE, 6, bytes));

    _reset_variables();

    transmit_can_frame_enabled = false;
    CanUtilities_clear_can_message(&can_msg);
    EXPECT_FALSE(CanTxMessageHandler_can_frame(&can_msg));
    uint8_t bytes1[6] = {0x05, 0x01, 0x01, 0x01, 0x07, 0xFF};
    EXPECT_FALSE(compare_can_msg(&can_msg, 0x107016BE, 6, bytes1));
}

TEST(CanTxMessageHandler, handle_stream_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _reset_variables();
    _global_initalize();

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(STREAM);
    // ToDo: implement streams s
    EXPECT_TRUE(CanTxMessageHandler_stream_frame(openlcb_msg, &can_msg, 0));
    OpenLcbBufferStore_free_buffer(openlcb_msg);
}

TEST(CanTxMessageHandler, handle_datagram_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    uint16_t offset = 0;

    _reset_variables();
    _global_initalize();

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    EXPECT_NE(openlcb_msg, nullptr);

    // ************************************************************************
    // Last frame has one short (7 vs 8 payload bytes)
    // ************************************************************************
    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_DATAGRAM;
    openlcb_msg->payload_count = 31;
    for (int i = 0; i < 31; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    while (offset < openlcb_msg->payload_count)
    {

        EXPECT_TRUE(CanTxMessageHandler_datagram_frame(openlcb_msg, &can_msg, &offset));
        EXPECT_TRUE(application_callback_tx_called);
        application_callback_tx_called = false;
    }

    EXPECT_EQ(offset, 31);
    EXPECT_EQ(transmit_can_frame_index, 4);

    uint8_t bytes[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x1BBBBAAA, 8, bytes));

    uint8_t bytes1[8] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[1], 0x1CBBBAAA, 8, bytes1));

    uint8_t bytes2[8] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[2], 0x1CBBBAAA, 8, bytes2));

    uint8_t bytes3[7] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[3], 0x1DBBBAAA, 7, bytes3));
    // ************************************************************************

    // ************************************************************************
    // Exact fit of data in frames with no callback
    // ************************************************************************

    _global_initalize_no_callback();
    _reset_variables();
    offset = 0;

    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_DATAGRAM;
    openlcb_msg->payload_count = 32;
    for (int i = 0; i < 32; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    while (offset < openlcb_msg->payload_count)
    {

        EXPECT_TRUE(CanTxMessageHandler_datagram_frame(openlcb_msg, &can_msg, &offset));
        EXPECT_FALSE(application_callback_tx_called);
    }

    EXPECT_EQ(offset, 32);
    EXPECT_EQ(transmit_can_frame_index, 4);

    uint8_t bytes4[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x1BBBBAAA, 8, bytes4));

    uint8_t bytes5[8] = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[1], 0x1CBBBAAA, 8, bytes5));

    uint8_t bytes6[8] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[2], 0x1CBBBAAA, 8, bytes6));

    uint8_t bytes7[8] = {0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[3], 0x1DBBBAAA, 8, bytes7));
    // ************************************************************************

    // ************************************************************************
    // Single Frame with temporary transmit fail
    // ************************************************************************

    _global_initalize();
    _reset_variables();
    offset = 0;

    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_DATAGRAM;
    openlcb_msg->payload_count = 3;
    for (int i = 0; i < 3; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    transmit_can_frame_enabled = false;
    int counter = 0;
    while (offset < openlcb_msg->payload_count)
    {

        if (counter > 10)
        {
            transmit_can_frame_enabled = true;
            EXPECT_TRUE(CanTxMessageHandler_datagram_frame(openlcb_msg, &can_msg, &offset));
        }
        else
        {
            counter++;
            EXPECT_FALSE(CanTxMessageHandler_datagram_frame(openlcb_msg, &can_msg, &offset));
        }
    }

    EXPECT_EQ(offset, 3);
    EXPECT_EQ(transmit_can_frame_index, 1);

    uint8_t bytes20[3] = {0x00, 0x01, 0x02};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x1ABBBAAA, 3, bytes20));

    OpenLcbBufferStore_free_buffer(openlcb_msg);
}

TEST(CanTxMessageHandler, handle_addressed_msg_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    uint16_t offset = 0;

    _reset_variables();
    _global_initalize();

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(openlcb_msg, nullptr);

    // ************************************************************************
    // Last frame has one data bytes in the last frame
    // ************************************************************************
    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;
    openlcb_msg->payload_count = 19;
    for (int i = 0; i < 19; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    while (offset < openlcb_msg->payload_count)
    {

        EXPECT_TRUE(CanTxMessageHandler_addressed_msg_frame(openlcb_msg, &can_msg, &offset));
        EXPECT_TRUE(application_callback_tx_called);
        application_callback_tx_called = false;
    }

    EXPECT_EQ(offset, 19);
    EXPECT_EQ(transmit_can_frame_index, 4);

    uint8_t bytes[8] = {0x4B, 0xBB, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x19A08AAA, 8, bytes));

    uint8_t bytes1[8] = {0xCB, 0xBB, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[1], 0x19A08AAA, 8, bytes1));

    uint8_t bytes2[8] = {0xCB, 0xBB, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[2], 0x19A08AAA, 8, bytes2));

    uint8_t bytes3[3] = {0x8B, 0xBB, 0x12};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[3], 0x19A08AAA, 3, bytes3));
    // ************************************************************************

    // ************************************************************************
    // Exact fit of data in frames and no callback
    // ************************************************************************

    _global_initalize_no_callback();
    _reset_variables();
    offset = 0;

    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;
    openlcb_msg->payload_count = 24;
    for (int i = 0; i < 24; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    while (offset < openlcb_msg->payload_count)
    {

        EXPECT_TRUE(CanTxMessageHandler_addressed_msg_frame(openlcb_msg, &can_msg, &offset));
        EXPECT_FALSE(application_callback_tx_called);
    }

    EXPECT_EQ(offset, 24);
    EXPECT_EQ(transmit_can_frame_index, 4);

    uint8_t bytes4[8] = {0x4B, 0xBB, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x19A08AAA, 8, bytes4));

    uint8_t bytes5[8] = {0xCB, 0xBB, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[1], 0x19A08AAA, 8, bytes5));

    uint8_t bytes6[8] = {0xCB, 0xBB, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[2], 0x19A08AAA, 8, bytes6));

    uint8_t bytes7[8] = {0x8B, 0xBB, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[3], 0x19A08AAA, 8, bytes7));

    // ************************************************************************

    // ************************************************************************
    // Single Frame with temporary transmit fail
    // ************************************************************************

    _global_initalize();
    _reset_variables();
    offset = 0;

    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;
    openlcb_msg->payload_count = 3;
    for (int i = 0; i < 3; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    transmit_can_frame_enabled = false;
    int counter = 0;
    while (offset < openlcb_msg->payload_count)
    {

        if (counter > 10)
        {
            transmit_can_frame_enabled = true;
            EXPECT_TRUE(CanTxMessageHandler_addressed_msg_frame(openlcb_msg, &can_msg, &offset));
        }
        else
        {
            counter++;
            EXPECT_FALSE(CanTxMessageHandler_addressed_msg_frame(openlcb_msg, &can_msg, &offset));
        }
    }

    EXPECT_EQ(offset, 3);
    EXPECT_EQ(transmit_can_frame_index, 1);

    uint8_t bytes20[5] = {0x0B, 0xBB, 0x00, 0x01, 0x02};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x19A08AAA, 5, bytes20));

    OpenLcbBufferStore_free_buffer(openlcb_msg);
}

TEST(CanTxMessageHandler, unaddressed_msg_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    uint16_t offset = 0;

    _reset_variables();
    _global_initalize();

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    EXPECT_NE(openlcb_msg, nullptr);

    // ************************************************************************
    // Frame with full payload
    // ************************************************************************
    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_CONSUMER_IDENTIFIED_UNKNOWN;
    openlcb_msg->payload_count = 8;
    for (int i = 0; i < 8; i++)
    {

        *openlcb_msg->payload[i] = i;
    }

    while (offset < openlcb_msg->payload_count)
    {

        EXPECT_TRUE(CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg, &can_msg, &offset));
        EXPECT_TRUE(application_callback_tx_called);
        application_callback_tx_called = false;
    }

    EXPECT_EQ(offset, 8);
    EXPECT_EQ(transmit_can_frame_index, 1);

    uint8_t bytes[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x194C7AAA, 8, bytes));

    // ************************************************************************

    // ************************************************************************
    // Frame with no payload
    // ************************************************************************

    _reset_variables();
    _global_initalize();
    offset = 0;

    openlcb_msg->source_alias = 0xAAA;
    openlcb_msg->source_id = 0x010203040506;
    openlcb_msg->dest_alias = 0xBBB;
    openlcb_msg->dest_id = 0x060504030201;
    openlcb_msg->mti = MTI_VERIFY_NODE_ID_GLOBAL;
    openlcb_msg->payload_count = 0;

    EXPECT_TRUE(CanTxMessageHandler_unaddressed_msg_frame(openlcb_msg, &can_msg, &offset));
    EXPECT_TRUE(application_callback_tx_called);
    application_callback_tx_called = false;

    EXPECT_EQ(offset, 0);
    EXPECT_EQ(transmit_can_frame_index, 1);

    uint8_t bytes1[0];
    EXPECT_TRUE(compare_can_msg(&transmitted_can_msg[0], 0x19490AAA, 0, bytes1));

    // ************************************************************************

    OpenLcbBufferStore_free_buffer(openlcb_msg);
}