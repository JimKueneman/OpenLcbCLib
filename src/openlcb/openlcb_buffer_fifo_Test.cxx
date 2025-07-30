#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferFIFO, initialize)
{

    BufferFifo_initialize();
}

TEST(OpenLcbBufferFIFO, allocate_basic_buffer)
{

    BufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Push it
        openlcb_msg = BufferFifo_push(openlcb_msg);
        EXPECT_NE(openlcb_msg, nullptr);

        // Should be one allocated
        int count = BufferFifo_get_allocated_count();
        EXPECT_EQ(count, 1);

        // Pop it and see if it is the same object
        openlcb_msg_t *popped_openlcb_msg = BufferFifo_pop();
        EXPECT_EQ(popped_openlcb_msg, openlcb_msg);
    }
}

TEST(OpenLcbBufferFIFO, allocate_datagram_buffer)
{

    BufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Push it
        openlcb_msg = BufferFifo_push(openlcb_msg);
        EXPECT_NE(openlcb_msg, nullptr);

        // Should be one allocated
        int count = BufferFifo_get_allocated_count();
        EXPECT_EQ(count, 1);

        // Pop it and see if it is the same object
        openlcb_msg_t *popped_openlcb_msg = BufferFifo_pop();
        EXPECT_EQ(popped_openlcb_msg, openlcb_msg);
    }
}

TEST(OpenLcbBufferFIFO, allocate_snip_buffer)
{

    BufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(SNIP);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {

        // Push it
        openlcb_msg = BufferFifo_push(openlcb_msg);
        EXPECT_NE(openlcb_msg, nullptr);

        // Should be one allocated
        int count = BufferFifo_get_allocated_count();
        EXPECT_EQ(count, 1);

        // Pop it and see if it is the same object
        openlcb_msg_t *popped_openlcb_msg = BufferFifo_pop();
        EXPECT_EQ(popped_openlcb_msg, openlcb_msg);
    }
}

TEST(OpenLcbBufferFIFO, allocate_stream_buffer)
{

    // Streams not implemented yet 7.29.25
}