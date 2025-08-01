#include "test/main_Test.hxx"

#include "drivers/common/can_buffer_fifo.h"
#include "drivers/common/can_buffer_store.h"

TEST(CAN_BufferFIFO, CAN_BufferFIFO_initialize)
{

    CanBufferFifo_initialize();

    int count = CanBufferFifo_get_allocated_count();

    EXPECT_EQ(count, 0);

    count = CanBufferFifo_is_empty();

    EXPECT_NE(count, 0);
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_push_pop)
{

    CanBufferFifo_initialize();

    // allocate and check a buffer
    can_msg_t *new_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(new_msg, nullptr);

    // push and check the return
    bool pushed = CanBufferFifo_push(new_msg);
    EXPECT_TRUE(pushed);

    // checked it registered it
    int count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 1);

    // check is claims its not empty
    count = CanBufferFifo_is_empty();
    EXPECT_EQ(count, 0);

    // pop it into a different variable
    can_msg_t *popped_msg = CanBufferFifo_pop();

    // check it unregistered it
    count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // check it is not empty
    count = CanBufferFifo_is_empty();
    EXPECT_NE(count, 0);

    // Check did it pop the same message?
    EXPECT_EQ(popped_msg, new_msg);

    // free the message
    CanBufferStore_free_buffer(new_msg);
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_stress)
{

    can_msg_t *new_msg_array[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

    CanBufferFifo_initialize();

    // Allocate enough buffers to fill the FIFO
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {

        new_msg_array[i] = CanBufferStore_allocate_buffer();
        EXPECT_NE(new_msg_array[i], nullptr);
    }

    // Push all the buffers and make sure they each are registered and valid pushes
    int count = 0;
    int result = 0;
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {

        result = CanBufferFifo_push(new_msg_array[i]);
        EXPECT_EQ(result, 1);
        count = CanBufferFifo_get_allocated_count();
        EXPECT_EQ(count, i + 1);
        count = CanBufferFifo_is_empty();
        EXPECT_EQ(count, 0);
    }

    // Try to push one too many and validate result
    can_msg_t overflow_msg;
    result = CanBufferFifo_push(&overflow_msg);
    EXPECT_EQ(result, 0);

    // Pop them all and check they are what was pushed and then unregistered
    can_msg_t *popped_msg;
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        popped_msg = CanBufferFifo_pop();
        EXPECT_EQ(popped_msg, new_msg_array[i]);
        count = CanBufferFifo_get_allocated_count();
        EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1);
    }

    // Free the buffers
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        CanBufferStore_free_buffer(new_msg_array[i]);
    }
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_push_null)
{

    CanBufferFifo_initialize();

    int result = CanBufferFifo_push(nullptr);
    EXPECT_EQ(result, 0);
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_pop_empty)
{

    CanBufferFifo_initialize();

    can_msg_t *popped_msg = CanBufferFifo_pop();
    EXPECT_EQ(popped_msg, nullptr);
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_wrap_head_tail)
{

    CanBufferFifo_initialize();

    can_msg_t test_msg;
    can_msg_t *test_msg_pop;
    int count;

    // make the head/tail wrap a few times
    for (int j = 0; j < 10; j++)
    {
        for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
        {
            CanBufferFifo_push(&test_msg);
        }

        for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
        {
            test_msg_pop = CanBufferFifo_pop();
            EXPECT_EQ(test_msg_pop, &test_msg);
            count = CanBufferFifo_get_allocated_count();
            EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1);
        }
    }
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_wra)
{
}