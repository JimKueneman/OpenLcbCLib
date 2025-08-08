#include "test/main_Test.hxx"

#include "can_types.h"
#include "can_utilities.h"
#include "can_frame_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"

#define MAPPING_COUNT 6

alias_mapping_t mapping_array[MAPPING_COUNT] = {

    {.alias = 0x0555, .node_id = 0x010203040500},
    {.alias = 0x0666, .node_id = 0x010203040501},
    {.alias = 0x0777, .node_id = 0x010203040502},
    {.alias = 0x0888, .node_id = 0x010203040503},
    {.alias = 0x0999, .node_id = 0x010203040504},
    {.alias = 0x0AAA, .node_id = 0x010203040505}};

uint16_t duplicate_alias = 0;
interface_can_frame_message_handler_t can_frame_message_handler_interface;

alias_mapping_t *find_alias_mapping(node_id_t node_id, uint16_t alias)
{

    if (alias != 0)
    {

        for (int i = 0; i < MAPPING_COUNT; i++)
        {

            if (mapping_array[i].alias == alias)
            {
                return (&mapping_array[i]);
            }
        }
    }
    if (node_id != 0)
    {

        for (int i = 0; i < MAPPING_COUNT; i++)
        {

            if (mapping_array[i].node_id == node_id)
            {
                return (&mapping_array[i]);
            }
        }
    }

    return nullptr;
}

void set_mapping_duplicate_alias_detected(uint16_t node_alias)
{

    duplicate_alias = node_alias;
}

uint16_t mapping_count(void)
{

    return MAPPING_COUNT;
}

alias_mapping_t *alias_mapping(uint16_t index)
{

    return (&mapping_array[index]);
}

void global_initialize(void)
{
    can_frame_message_handler_interface.find_alias_mapping = &find_alias_mapping;
    can_frame_message_handler_interface.alias_mapping = &alias_mapping;
    can_frame_message_handler_interface.set_mapping_duplicate_alias_detected = &set_mapping_duplicate_alias_detected;
    can_frame_message_handler_interface.mapping_count = &mapping_count;

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbNode_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();

    CanFrameMessageHandler_initialize(&can_frame_message_handler_interface);

    duplicate_alias = 0;
}

bool _is_cid_frame(can_msg_t *can_msg)
{

    return ((can_msg->identifier & 0x0F000000) != 0x00000000);
}

bool _is_rid_frame(can_msg_t *can_msg)
{

    return ((can_msg->identifier & 0x0FFFF000) == 0x00700000);
}

bool _is_amd_frame(can_msg_t *can_msg)
{

    return ((can_msg->identifier & 0x0FFFF000) == 0x00701000);
}

bool _is_ame_frame(can_msg_t *can_msg)
{

    return ((can_msg->identifier & 0x0FFFF000) == 0x00702000);
}

bool _is_amr_frame(can_msg_t *can_msg)
{

    return ((can_msg->identifier & 0x0FFFF000) == 0x00703000);
}

bool _is_error_report_frame(can_msg_t *can_msg)
{

    return (

        ((can_msg->identifier & 0x0FFFF000) == 0x00710000) ||
        ((can_msg->identifier & 0x0FFFF000) == 0x00711000) ||
        ((can_msg->identifier & 0x0FFFF000) == 0x00712000) ||
        ((can_msg->identifier & 0x0FFFF000) == 0x00713000));
}

TEST(CanFrameMessageHandler, initialize)
{

    global_initialize();
}

TEST(CanFrameMessageHandler, cid)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    duplicate_alias = 0;

    // No conflict
    can_msg.identifier = 0x17050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // No conflict
    can_msg.identifier = 0x16050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // No conflict
    can_msg.identifier = 0x15050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // No conflict
    can_msg.identifier = 0x17040AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // Cause a conflict
    can_msg.identifier = 0x17050AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_rid_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0);

    // Cause a conflict
    can_msg.identifier = 0x17040777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_rid_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0);
}

TEST(CanFrameMessageHandler, rid)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // No conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10700AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // Cause a conflict
    can_msg.identifier = 0x10700AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0AAA);

    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10700777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0777);
}

TEST(CanFrameMessageHandler, amd)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // No conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10701AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amd(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);

    // Cause a conflict
    can_msg.identifier = 0x10701AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amd(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0AAA);

    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10701777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0777);
}

TEST(CanFrameMessageHandler, ame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10702AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_ame(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0AAA);

    // No conflict
    // Request one of our nodes 0x010203040503
    duplicate_alias = 0;
    can_msg.identifier = 0x10702AAB;
    can_msg.payload_count = 6;
    can_msg.payload[0] = 0x01;
    can_msg.payload[1] = 0x02;
    can_msg.payload[2] = 0x03;
    can_msg.payload[3] = 0x04;
    can_msg.payload[4] = 0x05;
    can_msg.payload[5] = 0x03;
    CanFrameMessageHandler_ame(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x03);
    EXPECT_EQ(duplicate_alias, 0);

    // No conflict
    // Request all of our nodes
    duplicate_alias = 0;
    can_msg.identifier = 0x10702AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_ame(&can_msg);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), MAPPING_COUNT);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x00);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x01);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x02);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x03);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x04);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amd_frame(outgoing_can_msg));
    EXPECT_EQ(outgoing_can_msg->payload_count, 6);
    EXPECT_EQ(outgoing_can_msg->payload[0], 0x01);
    EXPECT_EQ(outgoing_can_msg->payload[1], 0x02);
    EXPECT_EQ(outgoing_can_msg->payload[2], 0x03);
    EXPECT_EQ(outgoing_can_msg->payload[3], 0x04);
    EXPECT_EQ(outgoing_can_msg->payload[4], 0x05);
    EXPECT_EQ(outgoing_can_msg->payload[5], 0x05);

    EXPECT_EQ(duplicate_alias, 0);
}

TEST(CanFrameMessageHandler, amr)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10703AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amr(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(_is_amr_frame(outgoing_can_msg));
    EXPECT_EQ(duplicate_alias, 0x0AAA);

    // Non  conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10703AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amr(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
}

TEST(CanFrameMessageHandler, handle_single_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *outgoing_openlcb_msg;

    global_initialize();

    // Addressed message to us
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    duplicate_alias = 0;
    can_msg.identifier = 0x196686BE;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x0A;
    can_msg.payload[1] = 0xAA;
    can_msg.payload[2] = 0x54;
    can_msg.payload[3] = 0x58;
    can_msg.payload[4] = 0x20;
    can_msg.payload[5] = 0x00;
    can_msg.payload[6] = 0x00;
    can_msg.payload[7] = 0x00;
    CanFrameMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);
    EXPECT_EQ(outgoing_openlcb_msg->mti, 0x668);
    EXPECT_EQ(outgoing_openlcb_msg->source_alias, 0x06BE);
    EXPECT_EQ(outgoing_openlcb_msg->dest_alias, 0x0AAA);
    EXPECT_EQ(outgoing_openlcb_msg->source_id, 0x0000);
    EXPECT_EQ(outgoing_openlcb_msg->dest_id, 0x0000);
    EXPECT_EQ(outgoing_openlcb_msg->payload_count, 6);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[0], 0x54);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[1], 0x58);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[2], 0x20);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[3], 0x00);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[4], 0x00);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[5], 0x00);

    // Addressed message to someone else
    // [[196686be] 0A AB 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    duplicate_alias = 0;
    can_msg.identifier = 0x196686BE;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x0A;
    can_msg.payload[1] = 0xAB;
    can_msg.payload[2] = 0x54;
    can_msg.payload[3] = 0x58;
    can_msg.payload[4] = 0x20;
    can_msg.payload[5] = 0x00;
    can_msg.payload[6] = 0x00;
    can_msg.payload[7] = 0x00;
    CanFrameMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_EQ(outgoing_openlcb_msg, nullptr);

    // Unaddressed message to us
    // [[195476be]  Producer Identified Unknown for EventID:05.01.01.01.07.FF.00.25
    duplicate_alias = 0;
    can_msg.identifier = 0x195476BE;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x05;
    can_msg.payload[1] = 0x01;
    can_msg.payload[2] = 0x01;
    can_msg.payload[3] = 0x01;
    can_msg.payload[4] = 0x07;
    can_msg.payload[5] = 0xFF;
    can_msg.payload[6] = 0x00;
    can_msg.payload[7] = 0x25;
    CanFrameMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);
    EXPECT_EQ(outgoing_openlcb_msg->mti, 0x547);
    EXPECT_EQ(outgoing_openlcb_msg->source_alias, 0x06BE);
    EXPECT_EQ(outgoing_openlcb_msg->dest_alias, 0x0000);
    EXPECT_EQ(outgoing_openlcb_msg->source_id, 0x0000);
    EXPECT_EQ(outgoing_openlcb_msg->dest_id, 0x0000);
    EXPECT_EQ(outgoing_openlcb_msg->payload_count, 8);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[0], 0x05);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[1], 0x01);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[2], 0x01);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[3], 0x01);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[4], 0x07);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[5], 0xFF);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[6], 0x00);
    EXPECT_EQ(*outgoing_openlcb_msg->payload[7], 0x25);
}