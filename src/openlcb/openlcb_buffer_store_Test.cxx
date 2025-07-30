#include "test/main_Test.hxx"

#include "./openlcb_buffer_store.h"
#include "./openlcb_types.h"

// TEST(OpenLcbBufferStore, OpenLcbBufferStore_initialize)
// {

//     BufferStore_initialize();
// }

// TEST(OpenLcbBufferStore, OpenLcbBufferStore_allocateBuffer_basic)
// {

//     // BufferStore_initialize();

//     // // Allocate a basic OpenLcb Message Structure
//     // openlcb_msg_t *openlcb_msg = BufferStore_allocateBuffer(BASIC);
//     // EXPECT_NE(openlcb_msg, nullptr);

//     // // Test the getters for the current and past number of allocated buffers
//     // int count = BufferStore_basic_messages_allocated();
//     // EXPECT_EQ(count, 1);
//     // count = BufferStore_basic_messages_max_allocated();
//     // EXPECT_EQ(count, 1);

//     // BufferStore_clear_max_allocated();

//     // count = BufferStore_basic_messages_allocated();
//     // EXPECT_EQ(count, 1);
//     // count = BufferStore_basic_messages_max_allocated();
//     // EXPECT_EQ(count, 0);

//     // if (openlcb_msg)
//     // {

//     //     EXPECT_EQ(openlcb_msg->state.allocated, 1);
//     //     EXPECT_EQ(openlcb_msg->state.inprocess, 0);
//     //     EXPECT_EQ(openlcb_msg->mti, 0);
//     //     EXPECT_EQ(openlcb_msg->source_alias, 0);
//     //     EXPECT_EQ(openlcb_msg->dest_alias, 0);
//     //     EXPECT_EQ(openlcb_msg->source_id, 0);
//     //     EXPECT_EQ(openlcb_msg->dest_id, 0);
//     //     EXPECT_EQ(openlcb_msg->payload_size, BASIC);
//     //     EXPECT_EQ(openlcb_msg->payload_count, 0);
//     //     EXPECT_EQ(openlcb_msg->payload, nullptr);
//     //     EXPECT_EQ(openlcb_msg->timerticks, 0);
//     //     EXPECT_EQ(openlcb_msg->reference_count, 1);

//     //     BufferStore_freeBuffer(openlcb_msg);
//     // }
// }

// TEST(OpenLcbBufferStore, OpenLcbBufferStore_clear_message)
// {

//     BufferStore_initialize();

//     // openlcb_msg_t *can_msg = CanBufferStore_allocateBuffer();

//     // EXPECT_NE(can_msg, nullptr);

//     // if (can_msg)
//     // {

//     //     can_msg->identifier = 0xFFFFFFFF;
//     //     can_msg->payload_count = LEN_CAN_BYTE_ARRAY;

//     //     for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
//     //     {
//     //         can_msg->payload[i] = i;
//     //     }

//     //     CanBufferStore_clear_can_message(can_msg);

//     //     EXPECT_EQ(can_msg->identifier, 0);
//     //     EXPECT_EQ(can_msg->payload_count, 0);

//     //     for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
//     //     {
//     //         EXPECT_EQ(can_msg->payload[i], 0);
//     //     }

//     //     CanBufferStore_freeBuffer(can_msg);
//     // }
// }

// TEST(OpenLcbBufferStore, OpenLcbBufferStore_stress_buffer)
// {

//     BufferStore_initialize();

//     // openlcb_msg_t *can_msg_array[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];
//     // openlcb_msg_t *can_msg;

//     // for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
//     // {

//     //     can_msg_array[i] = CanBufferStore_allocateBuffer();

//     //     EXPECT_NE(can_msg_array[i], nullptr);
//     // }

//     // can_msg = CanBufferStore_allocateBuffer();

//     // EXPECT_TRUE(can_msg == nullptr);

//     // CanBufferStore_freeBuffer(can_msg_array[0]);

//     // int count = CanBufferStore_messages_allocated();

//     // EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - 1);

//     // for (int i = 1; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
//     // {

//     //     CanBufferStore_freeBuffer(can_msg_array[i]);

//     //     count = CanBufferStore_messages_allocated();

//     //     EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1);
//     // }
// }

TEST(OpenLcbBufferStore, OpenLcbBufferStore_free_null_buffer)
{

    BufferStore_initialize();

    // CanBufferStore_freeBuffer(nullptr);
}