#include "test/main_Test.hxx"

#include "can_utilities.h"

TEST(CAN_Utilities, clear_can_message)
{

    can_msg_t can_msg;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    can_msg.identifier = 0xFFFF;
    can_msg.payload_count = 8;

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
    {
        can_msg.payload[i] = 0xFF;
    }

    CanUtilities_clear_can_message(&can_msg);

    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.identifier, 0);
    EXPECT_EQ(can_msg.payload_count, 0);
    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
    {
        EXPECT_EQ(can_msg.payload[i], 0);
    }
}

TEST(CAN_Utilities, load_can_message)
{

    can_msg_t can_msg;

    memset(&can_msg, 0x00, sizeof(can_msg));

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.identifier, 0xABAB);
    EXPECT_EQ(can_msg.payload_count, 8);
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(can_msg.payload[i], 0xFF);
    }
}

TEST(CAN_Utilities, copy_node_id_to_payload)
{

    can_msg_t can_msg;

    memset(&can_msg, 0x00, sizeof(can_msg));

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    EXPECT_EQ(CanUtilities_copy_node_id_to_payload(&can_msg, 0xAABBCCDDEEFF, 0), 6);

    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.payload_count, 6);
    EXPECT_EQ(can_msg.payload[0], 0xAA);
    EXPECT_EQ(can_msg.payload[1], 0xBB);
    EXPECT_EQ(can_msg.payload[2], 0xCC);
    EXPECT_EQ(can_msg.payload[3], 0xDD);
    EXPECT_EQ(can_msg.payload[4], 0xEE);
    EXPECT_EQ(can_msg.payload[5], 0xFF);

    memset(&can_msg, 0x00, sizeof(can_msg));

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    EXPECT_EQ(CanUtilities_copy_node_id_to_payload(&can_msg, 0xAABBCCDDEEFF, 2), 8);

    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.payload_count, 8);
    EXPECT_EQ(can_msg.payload[2], 0xAA);
    EXPECT_EQ(can_msg.payload[3], 0xBB);
    EXPECT_EQ(can_msg.payload[4], 0xCC);
    EXPECT_EQ(can_msg.payload[5], 0xDD);
    EXPECT_EQ(can_msg.payload[6], 0xEE);
    EXPECT_EQ(can_msg.payload[7], 0xFF);

    // Out of range on offset
    memset(&can_msg, 0x00, sizeof(can_msg));

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    EXPECT_EQ(CanUtilities_copy_node_id_to_payload(&can_msg, 0xAABBCCDDEEFF, 3), 0);

    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.payload_count, 0);
}

TEST(CAN_Utilities, append_can_payload_to_openlcb_payload)
{

    can_msg_t can_msg;
    openlcb_msg_t openlcb_msg;
    uint8_t can_start_index = 0;

    memset(&can_msg, 0x00, sizeof(can_msg));
    memset(&openlcb_msg, 0x00, sizeof(openlcb_msg));

    CanUtilities_append_can_payload_to_openlcb_payload(&openlcb_msg, &can_msg, can_start_index);
}

TEST(CAN_Utilities, copy_can_payload_to_openlcb_payload)
{

    can_msg_t can_msg;
    openlcb_msg_t openlcb_msg;
    uint8_t can_start_index = 0;

    memset(&can_msg, 0x00, sizeof(can_msg));
    memset(&openlcb_msg, 0x00, sizeof(openlcb_msg));

    CanUtilities_copy_can_payload_to_openlcb_payload(&openlcb_msg, &can_msg, can_start_index);
}
