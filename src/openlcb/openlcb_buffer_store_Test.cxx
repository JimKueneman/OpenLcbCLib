#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferStore, OpenLcbBufferStore_initialize)
{

    BufferStore_initialize();
}

TEST(OpenLcbBufferStore, allocate_basic_buffer)
{

    BufferFifo_initialize();

    int count = 0;

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {
        // Should be one allocated
        count = BufferStore_basic_messages_allocated();
        EXPECT_EQ(count, 1);
        count = BufferStore_basic_messages_max_allocated();
        EXPECT_EQ(count, 1);

        // None should be allocated
        count = BufferStore_datagram_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_datagram_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // None should be allocated
        count = BufferStore_snip_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_snip_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // None should be allocated
        count = BufferStore_stream_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_stream_messages_max_allocated();
        EXPECT_EQ(count, 0);

        BufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbBufferStore, allocate_datagram_buffer)
{

    BufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {
        // None be one allocated
        count = BufferStore_basic_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_basic_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // Should be one allocated
        count = BufferStore_datagram_messages_allocated();
        EXPECT_EQ(count, 1);
        count = BufferStore_datagram_messages_max_allocated();
        EXPECT_EQ(count, 1);

        // None should be allocated
        count = BufferStore_snip_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_snip_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // None should be allocated
        count = BufferStore_stream_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_stream_messages_max_allocated();
        EXPECT_EQ(count, 0);

        BufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbBufferStore, allocate_snip_buffer)
{

    BufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(SNIP);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {
        // None be one allocated
        count = BufferStore_basic_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_basic_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // None should be allocated
        count = BufferStore_datagram_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_datagram_messages_max_allocated();
        EXPECT_EQ(count, 0);

        // Should be one allocated
        count = BufferStore_snip_messages_allocated();
        EXPECT_EQ(count, 1);
        count = BufferStore_snip_messages_max_allocated();
        EXPECT_EQ(count, 1);

        // None should be allocated
        count = BufferStore_stream_messages_allocated();
        EXPECT_EQ(count, 0);
        count = BufferStore_stream_messages_max_allocated();
        EXPECT_EQ(count, 0);

        BufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbBufferStore, allocate_stream_buffer)
{

    // Streams not implemented yet 7.29.25
}

TEST(OpenlcbBufferStore, stresstest_basic)
{
}

TEST(OpenlcbBufferStore, stresstest_datagram)
{
}

TEST(OpenlcbBufferStore, stresstest_snip)
{
}

TEST(OpenlcbBufferStore, stresstest_stream)
{

    // Streams not implemented yet 7.29.25
}