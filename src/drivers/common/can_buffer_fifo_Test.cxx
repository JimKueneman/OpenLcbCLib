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

    can_msg_t *new_msg = CanBufferFifo_push();

    EXPECT_NE(new_msg, nullptr);

    int count = CanBufferFifo_get_allocated_count();

    EXPECT_EQ(count, 1);

    count = CanBufferFifo_is_empty();

    EXPECT_EQ(count, 0);

    new_msg = nullptr;

    new_msg = CanBufferFifo_pop();

    count = CanBufferFifo_get_allocated_count();

    EXPECT_EQ(count, 0);

    count = CanBufferFifo_is_empty();

    EXPECT_NE(count, 0);
}

TEST(CAN_BufferFIFO, CAN_BufferFIFO_stress)
{

    can_msg_t *new_msg_array[LEN_CAN_FIFO_BUFFER];

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++)
    {

        new_msg_array[i] = CanBufferStore_allocateBuffer();
        EXPECT_NE(new_msg_array[i], nullptr);
    }

    int count = 0;

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++)
    {

        CanBufferFifo_push(new_msg_array[i]);
        count = CanBufferFifo_get_allocated_count();
        EXPECT_EQ(count, i + 1);
    }

    can_msg_t *popped_msg;

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++)
    {
        popped_msg = CanBufferFifo_pop();

        int count = CanBufferFifo_get_allocated_count();

        EXPECT_EQ(count, LEN_CAN_FIFO_BUFFER - i);
    }

    for (int i = 0; i < LEN_CAN_FIFO_BUFFER; i++)
    {
        CanBufferStore_freeBuffer(new_msg_array[i]);
    }
}
