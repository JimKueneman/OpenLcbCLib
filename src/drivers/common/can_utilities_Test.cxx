#include "test/main_Test.hxx"

#include "can_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_utilities.h"

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
    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
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

TEST(CAN_Utilities, copy_can_payload_to_openlcb_payload)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg = nullptr;
    uint16_t result = 0;

    OpenLcbBufferStore_initialize();

    memset(&can_msg, 0x00, sizeof(can_msg));

    // Load up a CAN message
    CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);

    // Copy the all 8 bytes in the can payload to the openlcb payload.  This will wipe out
    // any payload in the openlcb structure if it is successful, else nothing is changed
    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 4);

        result = CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 0);

        EXPECT_EQ(result, 8);
        EXPECT_EQ(openlcb_msg->payload_count, 8);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i], i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    // Copy the all upper 4 bytes in the can payload to the openlcb payload.  This will wipe out
    // any payload in the openlcb structure if it is successful, else nothing is changed
    openlcb_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 8);

        result = CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 4);

        EXPECT_EQ(result, 4);
        EXPECT_EQ(openlcb_msg->payload_count, 4);

        EXPECT_EQ(*openlcb_msg->payload[0], 5);
        EXPECT_EQ(*openlcb_msg->payload[1], 6);
        EXPECT_EQ(*openlcb_msg->payload[2], 7);
        EXPECT_EQ(*openlcb_msg->payload[3], 8);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    // Can buffer offset is beyond the size of a can message, should return nothing writen an
    // nothing changed
    openlcb_msg = OpenLcbBufferStore_allocate_buffer(SNIP);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 8);

        result = CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 8);

        // Nothing written and nothing changed
        EXPECT_EQ(result, 0);
        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i], 0x00);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

}

TEST(CAN_Utilities, copy_openlcb_payload_to_can_payload)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg = nullptr;
    uint16_t result = 0;

    OpenLcbBufferStore_initialize();

    memset(&can_msg, 0x00, sizeof(can_msg));

    // Copy the all 8 bytes in the can payload to the openlcb payload.  This will wipe out
    // any payload in the openlcb structure if it is successful, else nothing is changed
    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 4);
        *openlcb_msg->payload[0] = 0xAA;
        *openlcb_msg->payload[1] = 0xBB;
        *openlcb_msg->payload[2] = 0xCC;
        *openlcb_msg->payload[3] = 0xDD;

        result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, &can_msg, 0, 0);

        EXPECT_EQ(result, 4);
        EXPECT_EQ(can_msg.payload_count, 4);

        // Copied to can buffer
        EXPECT_EQ(can_msg.payload[0], 0xAA);
        EXPECT_EQ(can_msg.payload[1], 0xBB);
        EXPECT_EQ(can_msg.payload[2], 0xCC);
        EXPECT_EQ(can_msg.payload[3], 0xDD);

        // openlcb buffer is unchanged
        EXPECT_EQ(openlcb_msg->payload_count, 4);
        EXPECT_EQ(*openlcb_msg->payload[0], 0xAA);
        EXPECT_EQ(*openlcb_msg->payload[1], 0xBB);
        EXPECT_EQ(*openlcb_msg->payload[2], 0xCC);
        EXPECT_EQ(*openlcb_msg->payload[3], 0xDD);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 4);
        *openlcb_msg->payload[0] = 0xAA;
        *openlcb_msg->payload[1] = 0xBB;
        *openlcb_msg->payload[2] = 0xCC;
        *openlcb_msg->payload[3] = 0xDD;

        // both offset by 2 so the can message should retain its first 2 bytes, the openlcb message
        // is only 4 bytes long so it should only copy the remaining 2 to the can message and will
        // have clipped off the remaining bytes in the can message for a size of 4
        result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, &can_msg, 2, 2);

        EXPECT_EQ(result, 2);
        EXPECT_EQ(can_msg.payload_count, 4);

        // Copied to can buffer
        EXPECT_EQ(can_msg.payload[0], 0x01);
        EXPECT_EQ(can_msg.payload[1], 0x02);
        EXPECT_EQ(can_msg.payload[2], 0xCC);
        EXPECT_EQ(can_msg.payload[3], 0xDD);

        // openlcb buffer is unchanged
        EXPECT_EQ(openlcb_msg->payload_count, 4);
        EXPECT_EQ(*openlcb_msg->payload[0], 0xAA);
        EXPECT_EQ(*openlcb_msg->payload[1], 0xBB);
        EXPECT_EQ(*openlcb_msg->payload[2], 0xCC);
        EXPECT_EQ(*openlcb_msg->payload[3], 0xDD);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 16);
        for (int i = 0; i < 16; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        // both offset by 2 so the can message should retain its first 2 bytes, the openlcb message
        // is only 4 bytes long so it should only copy the remaining 2 to the can message and will
        // have clipped off the remaining bytes in the can message for a size of 4
        result = CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, &can_msg, 0, 0);

        EXPECT_EQ(result, 8);
        EXPECT_EQ(can_msg.payload_count, 8);

        // Copied to can buffer
        for (int i = 0; i < 8; i++)
        {
            EXPECT_EQ(can_msg.payload[i], i + 1);
        }

        // openlcb buffer is unchanged
        EXPECT_EQ(openlcb_msg->payload_count, 16);
        for (int i = 0; i < 16; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i], i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

}

TEST(CAN_Utilities, append_can_payload_to_openlcb_payload)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg = nullptr;
    uint16_t result;

    OpenLcbBufferStore_initialize();

    memset(&can_msg, 0x00, sizeof(can_msg));

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 0);

        // append an empty openlcb message
        result = CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 0);

        EXPECT_EQ(result, 8);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i], i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 4);
        *openlcb_msg->payload[0] = 0xAA;
        *openlcb_msg->payload[1] = 0xBB;
        *openlcb_msg->payload[2] = 0xCC;
        *openlcb_msg->payload[3] = 0xDD;

        // append 8 bytes from the can message to a openlcb message containing 4 existing items
        result = CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 0);

        EXPECT_EQ(result, 8);

        EXPECT_EQ(openlcb_msg->payload_count, 12);

        EXPECT_EQ(*openlcb_msg->payload[0], 0xAA);
        EXPECT_EQ(*openlcb_msg->payload[1], 0xBB);
        EXPECT_EQ(*openlcb_msg->payload[2], 0xCC);
        EXPECT_EQ(*openlcb_msg->payload[3], 0xDD);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i + 4], i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 14);
        for (int i = 0; i < 14; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        // attempt to append 8 bytes from the can message to a openlcb message containing 2 open items
        result = CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, 0);

        // Should only be able to copy 2 as the olcb buffer is almost full
        EXPECT_EQ(result, 2);

        // should be full
        EXPECT_EQ(openlcb_msg->payload_count, 16);

        for (int i = 0; i < 14; i++)
        {

            EXPECT_EQ(*openlcb_msg->payload[i], i + 1);
        }

        // two that were copied
        EXPECT_EQ(*openlcb_msg->payload[14], 0x01);
        EXPECT_EQ(*openlcb_msg->payload[15], 0x02);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(*openlcb_msg->payload[i], i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

}

TEST(CAN_Utilities, copy_64_bit_to_can_message)
{

    can_msg_t can_msg;
    uint8_t result;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    CanUtilties_load_can_message(&can_msg, 0xFFFF, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);

    result = CanUtilities_copy_64_bit_to_can_message(&can_msg, 0x5566778899AABBCC);

    EXPECT_EQ(result, true);
    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.identifier, 0xFFFF);
    EXPECT_EQ(can_msg.payload_count, 8);

    EXPECT_EQ(can_msg.payload[0], 0x55);
    EXPECT_EQ(can_msg.payload[1], 0x66);
    EXPECT_EQ(can_msg.payload[2], 0x77);
    EXPECT_EQ(can_msg.payload[3], 0x88);
    EXPECT_EQ(can_msg.payload[4], 0x99);
    EXPECT_EQ(can_msg.payload[5], 0xAA);
    EXPECT_EQ(can_msg.payload[6], 0xBB);
    EXPECT_EQ(can_msg.payload[7], 0xCC);

}

TEST(CAN_Utilities, copy_can_message)
{

    can_msg_t can_msg;
    can_msg_t can_msg_target;
    uint8_t result;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    CanUtilties_load_can_message(&can_msg, 0xFFFF, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);

    can_msg_target.state.allocated = 0;
    can_msg_target.state.addressed_direct_tx = 0;
    CanUtilties_load_can_message(&can_msg_target, 0xAAAA, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    result = CanUtilities_copy_can_message(&can_msg, &can_msg_target);

    EXPECT_EQ(result, true);
    // No changes in first message
    EXPECT_EQ(can_msg.state.allocated, 1);
    EXPECT_EQ(can_msg.state.addressed_direct_tx, 1);
    EXPECT_EQ(can_msg.identifier, 0xFFFF);
    EXPECT_EQ(can_msg.payload_count, 8);

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
    {

        EXPECT_EQ(can_msg.payload[i], i + 1);
    }

    // no changes in the state of the target but copied the rest
    EXPECT_EQ(can_msg_target.state.allocated, 0);
    EXPECT_EQ(can_msg_target.state.addressed_direct_tx, 0);
    EXPECT_EQ(can_msg_target.identifier, 0xFFFF);
    EXPECT_EQ(can_msg_target.payload_count, 8);

    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
    {

        EXPECT_EQ(can_msg_target.payload[i], i + 1);
    }
}

TEST(CAN_Utilities, extract_can_payload_as_node_id)
{

    can_msg_t can_msg;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    CanUtilties_load_can_message(&can_msg, 0xFFFF, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);

    uint64_t node_id = CanUtilities_extract_can_payload_as_node_id(&can_msg);

    EXPECT_EQ(node_id, 0x010203040506);

}

TEST(CAN_Utilities, extract_source_alias_from_can_message)
{

    can_msg_t can_msg;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 1;
    CanUtilties_load_can_message(&can_msg, 0xFFFFFFFF, 8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08);

    uint16_t alias = CanUtilities_extract_source_alias_from_can_identifier(&can_msg);

    EXPECT_EQ(alias, 0x0FFF);

}

TEST(CAN_Utilities, extract_dest_alias_from_can_message)
{

    can_msg_t can_msg;
    uint16_t alias;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 0;

    // initialize complete, no dest address
    CanUtilties_load_can_message(&can_msg, 0x19100AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0x000);

    // Verify Node ID Number Addressed , dest address in first byte of data
    CanUtilties_load_can_message(&can_msg, 0x19488AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // Protocol Support Addressed , dest address in first byte of data
    CanUtilties_load_can_message(&can_msg, 0x19828AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // CAN-Datagram Content (one frame)  , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x1AAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // CAN-Datagram Content (first frame)  , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x1BAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // CAN-Datagram Content (middle frame)  , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x1CAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // CAN-Datagram Content (last frame)  , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x1DAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // Stream Initiate Request , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x19CC8AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // Stream Data Proceed  , dest address in identifier
    CanUtilties_load_can_message(&can_msg, 0x19888AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0xAAA);

    // Reserved
    uint32_t reserved_identifier = (0x19888AAA & !0x19488AAA) | CAN_FRAME_TYPE_RESERVED;
    CanUtilties_load_can_message(&can_msg, reserved_identifier, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    EXPECT_EQ(alias, 0x00);

}

TEST(CAN_Utilities, extract_can_mti_from_can_identifier)
{

    can_msg_t can_msg;
    uint16_t mti;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 0;

    // initialize complete, mti
    CanUtilties_load_can_message(&can_msg, 0x19100AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x100);

    // CAN-Datagram Content (one frame)  , mti
    CanUtilties_load_can_message(&can_msg, 0x1AAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x1C48);

    // CAN-Datagram Content (first frame)   ,mti
    CanUtilties_load_can_message(&can_msg, 0x1BAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x1C48);

    // CAN-Datagram Content (middle frame)  , mti
    CanUtilties_load_can_message(&can_msg, 0x1CAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x1C48);

    // CAN-Datagram Content (last frame)  , mti
    CanUtilties_load_can_message(&can_msg, 0x1DAAABBB, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x1C48);

    // Stream Initiate Request , mti
    CanUtilties_load_can_message(&can_msg, 0x19CC8AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0xCC8);

    uint32_t reserved_identifier = (0x19888AAA & !0x19488AAA) | CAN_FRAME_TYPE_RESERVED;
    CanUtilties_load_can_message(&can_msg, reserved_identifier, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    mti = CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);
    EXPECT_EQ(mti, 0x0000);

}

TEST(CAN_Utilities, count_nulls_in_payloads)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg = nullptr;
    uint16_t result = 0;

    OpenLcbBufferStore_initialize();

    memset(&can_msg, 0x00, sizeof(can_msg));

    // Copy the all 8 bytes in the can payload to the openlcb payload.  This will wipe out
    // any payload in the openlcb structure if it is successful, else nothing is changed
    openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Load up a CAN message
        CanUtilties_load_can_message(&can_msg, 0xABAB, 8, 0x01, 0x02, 0x00, 0x04, 0x05, 0x00, 0x07, 0x08);
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0x0AAA, 0xBBBBBBBBBBBB, 0x0CCC, 0xDDDDDDDDDDDD, 0x0999, 4);
        *openlcb_msg->payload[0] = 0xAA;
        *openlcb_msg->payload[1] = 0x00;
        *openlcb_msg->payload[2] = 0xCC;
        *openlcb_msg->payload[3] = 0x00;

        result = CanUtilities_count_nulls_in_payloads(openlcb_msg, &can_msg);
        EXPECT_EQ(result, 4);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }
}

TEST(CAN_Utilities, is_openlcb_message)
{

    can_msg_t can_msg;

    can_msg.state.allocated = 1;
    can_msg.state.addressed_direct_tx = 0;

    CanUtilties_load_can_message(&can_msg, 0x19100AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    EXPECT_TRUE(CanUtilities_is_openlcb_message(&can_msg));

    CanUtilties_load_can_message(&can_msg, 0x17050AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    EXPECT_FALSE(CanUtilities_is_openlcb_message(&can_msg));

    CanUtilties_load_can_message(&can_msg, 0x10701AAA, 8, 0x1A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    EXPECT_FALSE(CanUtilities_is_openlcb_message(&can_msg));
}