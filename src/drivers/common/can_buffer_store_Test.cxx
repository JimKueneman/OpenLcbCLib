#include "test/main_Test.hxx"

#include "drivers/common/can_buffer_store.h"
#include "drivers/common/can_types.h"

TEST(CAN_BufferStore, CanBufferStore_initialize)
{
    CanBufferStore_initialize();
}

TEST(CAN_BufferStore, CanBufferStore_allocate_buffer)
{

    CanBufferStore_initialize();

    can_msg_t *can_msg = CanBufferStore_allocate_buffer();

    EXPECT_NE(can_msg, nullptr);

    // Test the getters for the current and past number of allocated buffers
    int count = CanBufferStore_messages_allocated();
    EXPECT_EQ(count, 1);
    count = CanBufferStore_messages_max_allocated();
    EXPECT_EQ(count, 1);

    CanBufferStore_clear_max_allocated();

    count = CanBufferStore_messages_allocated();
    EXPECT_EQ(count, 1);
    count = CanBufferStore_messages_max_allocated();
    EXPECT_EQ(count, 0);

    if (can_msg)
    {

        EXPECT_EQ(can_msg->state.allocated, 1);
        EXPECT_EQ(can_msg->state.addressed_direct_tx, 0);

        EXPECT_EQ(can_msg->identifier, 0);
        EXPECT_EQ(can_msg->payload_count, 0);

        CanBufferStore_free_buffer(can_msg);
    }
}

TEST(CAN_BufferStore, CanBufferStore_clear_message)
{

    CanBufferStore_initialize();

    can_msg_t *can_msg = CanBufferStore_allocate_buffer();

    EXPECT_NE(can_msg, nullptr);

    if (can_msg)
    {

        can_msg->identifier = 0xFFFFFFFF;
        can_msg->payload_count = LEN_CAN_BYTE_ARRAY;

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            can_msg->payload[i] = i;
        }

        CanBufferStore_clear_can_message(can_msg);

        EXPECT_EQ(can_msg->identifier, 0);
        EXPECT_EQ(can_msg->payload_count, 0);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(can_msg->payload[i], 0);
        }

        CanBufferStore_free_buffer(can_msg);
    }
}

TEST(CAN_BufferStore, CanBufferStore_stress_buffer)
{
    can_msg_t *can_msg_array[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];
    can_msg_t *can_msg;

    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {

        can_msg_array[i] = CanBufferStore_allocate_buffer();

        EXPECT_NE(can_msg_array[i], nullptr);
    }

    can_msg = CanBufferStore_allocate_buffer();

    EXPECT_TRUE(can_msg == nullptr);

    CanBufferStore_free_buffer(can_msg_array[0]);

    int count = CanBufferStore_messages_allocated();

    EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - 1);

    for (int i = 1; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {

        CanBufferStore_free_buffer(can_msg_array[i]);

        count = CanBufferStore_messages_allocated();

        EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1);
    }
}

TEST(CAN_BufferStore, CanBufferStore_free_null_buffer)
{

    CanBufferStore_free_buffer(nullptr);
}