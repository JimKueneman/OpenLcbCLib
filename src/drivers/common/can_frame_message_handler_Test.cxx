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
#include "../../openlcb/openlcb_defines.h"

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

bool fail_buffer = 0;

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

bool set_mapping_duplicate_alias_detected(uint16_t node_alias)
{

    duplicate_alias = node_alias;

    return true;
}

uint16_t mapping_count(void)
{

    return MAPPING_COUNT;
}

alias_mapping_t *alias_mapping(uint16_t index)
{

    return (&mapping_array[index]);
}

openlcb_msg_t *openlcb_buffer_store_allocate_buffer(payload_type_enum_t payload_type)
{

    if (!fail_buffer)
    {

        return OpenLcbBufferStore_allocate_buffer(payload_type);
    }
    else
    {

        return nullptr;
    }
}

void global_initialize(void)
{
    can_frame_message_handler_interface.find_alias_mapping = &find_alias_mapping;
    can_frame_message_handler_interface.alias_mapping = &alias_mapping;
    can_frame_message_handler_interface.set_mapping_duplicate_alias_detected = &set_mapping_duplicate_alias_detected;
    can_frame_message_handler_interface.mapping_count = &mapping_count;
    can_frame_message_handler_interface.openlcb_buffer_store_allocate_buffer = &openlcb_buffer_store_allocate_buffer;

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbNode_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();

    CanFrameMessageHandler_initialize(&can_frame_message_handler_interface);

    duplicate_alias = 0;
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

bool compare_openlcb_msg(openlcb_msg_t *openlcb_msg, uint16_t mti, node_id_t source_id, uint16_t source_alias, node_id_t dest_id, uint16_t dest_alias, uint16_t payload_size, uint8_t bytes[])
{

    bool result = (openlcb_msg->mti == mti) &&
                  (openlcb_msg->source_id == source_id) &&
                  (openlcb_msg->source_alias == source_alias) &&
                  (openlcb_msg->dest_id == dest_id) &&
                  (openlcb_msg->dest_alias == dest_alias) &&
                  (openlcb_msg->payload_count == payload_size);

    if (!result)
    {

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (*openlcb_msg->payload[i] != bytes[i])
        {

            return false;
        }
    }

    return true;
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

    // **************************************************************************************************
    // No conflict
    can_msg.identifier = 0x17050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // No conflict
    can_msg.identifier = 0x16050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // No conflict
    can_msg.identifier = 0x15050AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // No conflict
    can_msg.identifier = 0x17040AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    can_msg.identifier = 0x17050AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10700AAA, 0, nullptr));
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    can_msg.identifier = 0x17040777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_cid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10700777, 0, nullptr));
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, rid)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // No conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10700AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    can_msg.identifier = 0x10700AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));
    EXPECT_EQ(duplicate_alias, 0x0AAA);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10700777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x02};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703777, 6, bytes1));
    EXPECT_EQ(duplicate_alias, 0x0777);
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, amd)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // No conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10701AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amd(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    can_msg.identifier = 0x10701AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amd(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));
    EXPECT_EQ(duplicate_alias, 0x0AAA);
    // **************************************************************************************************

    // **************************************************************************************************
    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10701777;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_rid(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x02};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703777, 6, bytes1));
    EXPECT_EQ(duplicate_alias, 0x0777);
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, ame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10702AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_ame(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));
    EXPECT_EQ(duplicate_alias, 0x0AAA);
    // **************************************************************************************************

    // **************************************************************************************************
    // No conflict
    // Request one of our nodes 0x010203040503
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x10702AAB, 6, 0x01, 0x02, 0x03, 0x04, 0x05, 0x03, 0x00, 0x00);
    CanFrameMessageHandler_ame(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes10[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x03};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701888, 6, bytes10));
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************

    // **************************************************************************************************
    // No conflict
    // Request all of our nodes
    duplicate_alias = 0;
    can_msg.identifier = 0x10702AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_ame(&can_msg);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), MAPPING_COUNT);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x00};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701555, 6, bytes1));

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes2[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x01};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701666, 6, bytes2));

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes3[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x02};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701777, 6, bytes3));

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes4[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x03};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701888, 6, bytes4));

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes5[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x04};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701999, 6, bytes5));

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes6[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701AAA, 6, bytes6));

    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, amr)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // Cause a conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10703AAA;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amr(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes6[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x05};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes6));
    EXPECT_EQ(duplicate_alias, 0x0AAA);
    // **************************************************************************************************

    // **************************************************************************************************
    // Non  conflict
    duplicate_alias = 0;
    can_msg.identifier = 0x10703AAB;
    can_msg.payload_count = 0;
    CanFrameMessageHandler_amr(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_EQ(outgoing_can_msg, nullptr);
    EXPECT_EQ(duplicate_alias, 0);
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, handle_single_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *outgoing_openlcb_msg;

    global_initialize();

    // **************************************************************************************************
    // Addressed to us but can't allocate a buffer
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAA, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    fail_buffer = true;
    CanFrameMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
    fail_buffer = false;
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // Addressed message to us
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAA, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    CanFrameMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);
    uint8_t bytes[6] = {0x54, 0x58, 0x20, 0x00, 0x00, 0x00};
    EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x668, 0x0000, 0x06BE, 0x0000, 0x0AAA, 6, bytes));
    // **************************************************************************************************

    // **************************************************************************************************
    // Addressed message to someone else
    // [[196686be] 0A AB 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAB, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    CanFrameMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_EQ(outgoing_openlcb_msg, nullptr);
    // **************************************************************************************************

    // **************************************************************************************************
    // Unaddressed message to everyone
    // [[195476be]  Producer Identified Unknown for EventID:05.01.01.01.07.FF.00.25
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x195476BE, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25);
    CanFrameMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    uint8_t bytes1[8] = {0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25};
    EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x547, 0x0000, 0x06BE, 0x0000, 0x0000, 8, bytes1));
    // **************************************************************************************************

    // **************************************************************************************************
    // Addressed Datagram message to us
    // [1a5556eb]f 20 61 00 00 00 00 08   ]  Datagram: (7) 20.61.0.0.0.0.8
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1A5556BE, 7, 0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00);
    CanFrameMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    uint8_t bytes2[7] = {0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08};
    EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x1C48, 0x0000, 0x06BE, 0x0000, 0x0555, 7, bytes2));
    // **************************************************************************************************

    // **************************************************************************************************
    // Addressed Datagram message not to us
    // Addressed Datagram message to us
    // [1a5546eb]f 20 61 00 00 00 00 08   ]  Datagram: (7) 20.61.0.0.0.0.8
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1a5546BE, 7, 0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00);
    CanFrameMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, handle_first_frame)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    global_initialize();

    // **************************************************************************************************
    // SNIP addressed to somone else
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x4F, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    //  SNIP.......
    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_TRUE(CanBufferFifo_is_empty());

    uint8_t bytes[6] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61};
    EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 6, bytes));
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us but there is one already sent
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    can_msg_t *outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
    uint8_t bytes9[4] = {0x06, 0xbe, 0x20, 0x42};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes9));

    OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));
    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us but we can't allocate a buffer
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;

    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    fail_buffer = true;
    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    fail_buffer = false;

    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
    uint8_t bytes10[4] = {0x06, 0xbe, 0x20, 0x20};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes10));

    CanBufferStore_free_buffer(outgoing_can_msg);

    OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    //  Datagram.......
    // **************************************************************************************************

    // **************************************************************************************************
    // datagram addressed to somone else
    // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1bf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanFrameMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // Datagram addressed to us
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);

    CanFrameMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x06be, 0x0666, MTI_DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes1[8] = {0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F};
    EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x1C48, 0x0000, 0x6be, 0x0000, 0x666, 8, bytes1));
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);

    CanFrameMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());

    CanFrameMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
    uint8_t bytes2[4] = {0x06, 0xbe, 0x20, 0x42};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes2));

    OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0666, MTI_DATAGRAM));
    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    // DATAGRAM addressed to us but we can't allocate a buffer
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)

    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);

    fail_buffer = true;
    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, DATAGRAM);
    fail_buffer = false;

    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
    uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x20};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));

    CanBufferStore_free_buffer(outgoing_can_msg);

    OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, handle_middle_frame)
{
    can_msg_t can_msg;
    //  openlcb_msg_t *openlcb_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // SNIP addressed to somone else
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xCF, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us but have not received the start frame yet
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    EXPECT_EQ(duplicate_alias, 0x00);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes[4] = {0x06, 0xbe, 0x20, 0x41};

    fprintf(stderr, "identifier: 0x%06X\n", outgoing_can_msg->identifier);

    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes));

    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us but there is one already sent
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes9[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes9));

    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************

    // **************************************************************************************************
    //  Datagram.......
    // **************************************************************************************************

    // **************************************************************************************************
    // datagram addressed to somone else
    // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1cf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 0);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1c6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);

    CanFrameMessageHandler_handle_middle_frame(&can_msg, 0);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));

    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, handle_last_frame)
{
    can_msg_t can_msg;
    //  openlcb_msg_t *openlcb_msg;
    can_msg_t *outgoing_can_msg;

    global_initialize();

    // **************************************************************************************************
    // SNIP addressed to somone else
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x8F, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanFrameMessageHandler_handle_last_frame(&can_msg, 2);
    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    // SNIP addressed to us but have not received the start frame yet
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x89, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);

    CanFrameMessageHandler_handle_last_frame(&can_msg, 2);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    EXPECT_EQ(duplicate_alias, 0x00);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes));

    CanBufferStore_free_buffer(outgoing_can_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    //  Datagram.......
    // **************************************************************************************************

    // **************************************************************************************************
    // datagram addressed to somone else
    // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1cf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanFrameMessageHandler_handle_last_frame(&can_msg, 0);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // **************************************************************************************************

    // **************************************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1c6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);

    CanFrameMessageHandler_handle_last_frame(&can_msg, 0);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));

    CanBufferStore_free_buffer(outgoing_can_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // **************************************************************************************************
}

TEST(CanFrameMessageHandler, multi_frame_sequence_snip)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    // **************************************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanFrameMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x65, 0x65, 0x72, 0x69, 0x6E, 0x67);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x30, 0x32, 0x35, 0x30, 0x38, 0x30);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x37, 0x2E, 0x30, 0x37, 0x35, 0x37);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x89, 0x99, 0x31, 0x38, 0x00, 0x02, 0x00, 0x00);
    CanFrameMessageHandler_handle_last_frame(&can_msg, 2);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[66] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61,
                         0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B,
                         0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E,
                         0x65, 0x65, 0x72, 0x69, 0x6E, 0x67,
                         0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F,
                         0x75, 0x74, 0x42, 0x6F, 0x73, 0x73,
                         0x00, 0x56, 0x65, 0x72, 0x73, 0x69,
                         0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32,
                         0x30, 0x32, 0x35, 0x30, 0x38, 0x30,
                         0x37, 0x2E, 0x30, 0x37, 0x35, 0x37,
                         0x31, 0x38, 0x00, 0x02, 0x00, 0x00};
    EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 66, bytes));
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanFrameMessageHandler, multi_frame_sequence_datagram)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    // **************************************************************************************************
    // Datagram addressed to us
    //[[1b7776be] 20 53 00 00 00 00 3C 3F] R : (Start of Datagram)
    //[[1c7776be] 78 6D 6C 20 76 65 72 73] R : (Middle of Datagram)
    //[[1c7776be] 69 6F 6E 3D 22 31 2E 30] R : (Middle of Datagram)
    //[[1d7776be] 22 20                  ]  R: 05.01.01.01.07.FF - 02.01.12.FE.83.2E Datagram: (26) 20.53.0.0.0.0.3C.3F.78.6D.6C.20.76.65.72.73.69.6F.6E.3D.22.31.2E.30.22.20
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x1b7776be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanFrameMessageHandler_handle_first_frame(&can_msg, 0, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x1c7776be, 8, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 0);
    CanUtilities_load_can_message(&can_msg, 0x1c7776be, 8, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30);
    CanFrameMessageHandler_handle_middle_frame(&can_msg, 0);
    CanUtilities_load_can_message(&can_msg, 0x1d7776be, 2, 0x22, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanFrameMessageHandler_handle_last_frame(&can_msg, 0);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x077, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[26] = {0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F,
                         0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73,
                         0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30,
                         0x22, 0x20};
    EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x1C48, 0x0000, 0x6be, 0x0000, 0x777, 26, bytes));
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanFrameMessageHandler, multi_frame_sequence_legacy_snip)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    // **************************************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    duplicate_alias = 0;
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x65, 0x65, 0x72, 0x69, 0x6E, 0x67);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x30, 0x32, 0x35, 0x30, 0x38, 0x30);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x37, 0x2E, 0x30, 0x37, 0x35, 0x37);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x31, 0x38, 0x00, 0x02, 0x00, 0x00);
    CanFrameMessageHandler_handle_can_legacy_snip(&can_msg, 2, SNIP);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[66] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61,
                         0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B,
                         0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E,
                         0x65, 0x65, 0x72, 0x69, 0x6E, 0x67,
                         0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F,
                         0x75, 0x74, 0x42, 0x6F, 0x73, 0x73,
                         0x00, 0x56, 0x65, 0x72, 0x73, 0x69,
                         0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32,
                         0x30, 0x32, 0x35, 0x30, 0x38, 0x30,
                         0x37, 0x2E, 0x30, 0x37, 0x35, 0x37,
                         0x31, 0x38, 0x00, 0x02, 0x00, 0x00};
    EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 66, bytes));
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_EQ(duplicate_alias, 0x00);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}