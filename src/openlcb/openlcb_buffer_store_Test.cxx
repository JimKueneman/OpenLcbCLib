#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferStore, OpenLcbBufferStore_initialize)
{

    BufferStore_initialize();
}

TEST(OpenLcbBufferStore, allocate_basic_buffer)
{

    int count = 0;

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        BufferStore_initialize();

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

            // Max should be one allocated
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 0);
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

            // Make sure clear clears the max value
            BufferStore_clear_max_allocated();
            count = BufferStore_basic_messages_max_allocated();
            EXPECT_EQ(count, 0);
        }
    }
}

TEST(OpenLcbBufferStore, allocate_datagram_buffer)
{

    int count = 0;

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {

        BufferStore_initialize();

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

            // None be one allocated
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_basic_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // Max should still be one allocated
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 0);
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

            // Make sure clear clears the max value
            BufferStore_clear_max_allocated();
            count = BufferStore_datagram_messages_max_allocated();
            EXPECT_EQ(count, 0);
        }
    }
}

TEST(OpenLcbBufferStore, allocate_snip_buffer)
{

    int count = 0;

    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {

        BufferStore_initialize();

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

            // None should be allocated
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_basic_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // None should be allocated
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_datagram_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // Max should still be one
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_snip_messages_max_allocated();
            EXPECT_EQ(count, 1);

            // None should be allocated
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_stream_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // Make sure clear clears the max value
            BufferStore_clear_max_allocated();
            count = BufferStore_snip_messages_max_allocated();
            EXPECT_EQ(count, 0);
        }
    }
}

TEST(OpenLcbBufferStore, allocate_stream_buffer)
{
    int count = 0;

    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {

        BufferStore_initialize();

        // Allocate a basic buffer
        openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(STREAM);
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
            EXPECT_EQ(count, 0);
            count = BufferStore_snip_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // None should be allocated
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 1);
            count = BufferStore_stream_messages_max_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);

            // None should be allocated
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_basic_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // None should be allocated
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_datagram_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // Max should still be one
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_snip_messages_max_allocated();
            EXPECT_EQ(count, 0);

            // None should be allocated
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 0);
            count = BufferStore_stream_messages_max_allocated();
            EXPECT_EQ(count, 1);

            // Make sure clear clears the max value
            BufferStore_clear_max_allocated();
            count = BufferStore_stream_messages_max_allocated();
            EXPECT_EQ(count, 0);
        }
    }
}

TEST(OpenlcbBufferStore, stresstest_basic)
{
    openlcb_msg_t *openlcb_msg_array[USER_DEFINED_BASIC_BUFFER_DEPTH];
    openlcb_msg_t *openlcb_msg;
    int count = 0;

    if (USER_DEFINED_BASIC_BUFFER_DEPTH == 0)
    {

        return;
    }

    BufferStore_initialize();

    // allocate all possible buffers
    for (int i = 0; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {

        // check for success in allocating
        openlcb_msg_array[i] = BufferStore_allocate_buffer(BASIC);
        EXPECT_NE(openlcb_msg_array[i], nullptr);
    }

    // All should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH);

    // None should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // Max should still be one
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_snip_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // try one more, should fail and not increase the count
    openlcb_msg = BufferStore_allocate_buffer(BASIC);
    EXPECT_TRUE(openlcb_msg == nullptr);
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH);

    // Free one and make sure the count goes down
    BufferStore_free_buffer(openlcb_msg_array[0]);
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH - 1);

    for (int i = 1; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {

        BufferStore_free_buffer(openlcb_msg_array[i]);
        count = BufferStore_basic_messages_allocated();
        EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH - i - 1);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_BASIC_BUFFER_DEPTH);

    // None should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // Max should still be one
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_snip_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, 0);
}

TEST(OpenlcbBufferStore, stresstest_datagram)
{

    openlcb_msg_t *openlcb_msg_array[USER_DEFINED_DATAGRAM_BUFFER_DEPTH];
    openlcb_msg_t *openlcb_msg;
    int count = 0;

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH == 0)
    {

        return;
    }

    BufferStore_initialize();

    // allocate all possible buffers
    for (int i = 0; i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH; i++)
    {

        // check for success in allocating
        openlcb_msg_array[i] = BufferStore_allocate_buffer(DATAGRAM);
        EXPECT_NE(openlcb_msg_array[i], nullptr);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // All should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH);

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

    // try one more, should fail and not increase the count
    openlcb_msg = BufferStore_allocate_buffer(DATAGRAM);
    EXPECT_TRUE(openlcb_msg == nullptr);
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH);

    // Free one and make sure the count goes down
    BufferStore_free_buffer(openlcb_msg_array[0]);
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH - 1);

    for (int i = 1; i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH; i++)
    {

        BufferStore_free_buffer(openlcb_msg_array[i]);
        count = BufferStore_datagram_messages_allocated();
        EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH - i - 1);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // Max should still be one
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_DATAGRAM_BUFFER_DEPTH);

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
}

TEST(OpenlcbBufferStore, stresstest_snip)
{

    openlcb_msg_t *openlcb_msg_array[USER_DEFINED_SNIP_BUFFER_DEPTH];
    openlcb_msg_t *openlcb_msg;
    int count = 0;

    if (USER_DEFINED_SNIP_BUFFER_DEPTH == 0)
    {

        return;
    }

    BufferStore_initialize();

    // allocate all possible buffers
    for (int i = 0; i < USER_DEFINED_SNIP_BUFFER_DEPTH; i++)
    {

        // check for success in allocating
        openlcb_msg_array[i] = BufferStore_allocate_buffer(SNIP);
        EXPECT_NE(openlcb_msg_array[i], nullptr);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // All should be allocated
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH);
    count = BufferStore_snip_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH);

    // None should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // try one more, should fail and not increase the count
    openlcb_msg = BufferStore_allocate_buffer(SNIP);
    EXPECT_TRUE(openlcb_msg == nullptr);
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH);

    // Free one and make sure the count goes down
    BufferStore_free_buffer(openlcb_msg_array[0]);
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH - 1);

    for (int i = 1; i < USER_DEFINED_SNIP_BUFFER_DEPTH; i++)
    {

        BufferStore_free_buffer(openlcb_msg_array[i]);
        count = BufferStore_snip_messages_allocated();
        EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH - i - 1);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // Max should still be one
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_snip_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_SNIP_BUFFER_DEPTH);

    // None should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, 0);
}

TEST(OpenlcbBufferStore, stresstest_stream)
{
    openlcb_msg_t *openlcb_msg_array[USER_DEFINED_STREAM_BUFFER_DEPTH];
    openlcb_msg_t *openlcb_msg;
    int count = 0;

    if (USER_DEFINED_STREAM_BUFFER_DEPTH == 0)
    {

        return;
    }

    BufferStore_initialize();

    // allocate all possible buffers
    for (int i = 0; i < USER_DEFINED_STREAM_BUFFER_DEPTH; i++)
    {

        // check for success in allocating
        openlcb_msg_array[i] = BufferStore_allocate_buffer(STREAM);
        EXPECT_NE(openlcb_msg_array[i], nullptr);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

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

    // All should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH);

    // try one more, should fail and not increase the count
    openlcb_msg = BufferStore_allocate_buffer(STREAM);
    EXPECT_TRUE(openlcb_msg == nullptr);
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH);

    // Free one and make sure the count goes down
    BufferStore_free_buffer(openlcb_msg_array[0]);
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH - 1);

    for (int i = 1; i < USER_DEFINED_STREAM_BUFFER_DEPTH; i++)
    {
        BufferStore_free_buffer(openlcb_msg_array[i]);
        count = BufferStore_stream_messages_allocated();
        EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH - i - 1);
    }

    // None should be allocated
    count = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_basic_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_datagram_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // Max should still be one
    count = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_snip_messages_max_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
    count = BufferStore_stream_messages_max_allocated();
    EXPECT_EQ(count, USER_DEFINED_STREAM_BUFFER_DEPTH);
}

TEST(OpenlcbBufferStore, reference_counting)
{

    openlcb_msg_t *openlcb_msg;
    int count = 0;

    BufferStore_initialize();

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {

        openlcb_msg = BufferStore_allocate_buffer(BASIC);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            EXPECT_EQ(openlcb_msg->reference_count, 1);

            BufferStore_inc_reference_count(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 2);
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 1);
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 0);
        }
    }

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(DATAGRAM);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            EXPECT_EQ(openlcb_msg->reference_count, 1);

            BufferStore_inc_reference_count(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 2);
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 1);
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 0);
        }
    }

    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(SNIP);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            EXPECT_EQ(openlcb_msg->reference_count, 1);

            BufferStore_inc_reference_count(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 2);
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 1);
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 0);
        }
    }

    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(STREAM);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            EXPECT_EQ(openlcb_msg->reference_count, 1);

            BufferStore_inc_reference_count(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 2);
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            EXPECT_EQ(openlcb_msg->reference_count, 1);
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 1);

            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 0);
        }
    }
}

TEST(OpenlcbBufferStore, bad_allocation)
{

    openlcb_msg_t *openlcb_msg;

    openlcb_msg = BufferStore_allocate_buffer((payload_type_enum_t)0xFFFC);
    EXPECT_EQ(openlcb_msg, nullptr);
}

TEST(OpenlcbBufferStore, bad_release)
{

    int count_basic = 0;
    int count_datagram = 0;
    int count_snip = 0;
    int count_stream = 0;
    int count = 0;
    openlcb_msg_t *openlcb_msg = nullptr;

    BufferStore_initialize();

    // None should be allocated
    count_basic = BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count_datagram = BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);

    // All should be allocated
    count_snip = BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);

    // None should be allocated
    count_stream = BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);

    BufferStore_free_buffer(openlcb_msg);

    // make sure nothing has changed
    count = count_basic - BufferStore_basic_messages_allocated();
    EXPECT_EQ(count, count_basic);

    // make sure nothing has changed
    count = count_datagram - BufferStore_datagram_messages_allocated();
    EXPECT_EQ(count, 0);

    // make sure nothing has changed
    count = count_snip - BufferStore_snip_messages_allocated();
    EXPECT_EQ(count, 0);

    // make sure nothing has changed
    count = count_stream - BufferStore_stream_messages_allocated();
    EXPECT_EQ(count, 0);
}

TEST(OpenlcbBufferStore, corrupted_msg)
{

    int count = 0;
    openlcb_msg_t *openlcb_msg = nullptr;

    BufferStore_initialize();

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {

        openlcb_msg = BufferStore_allocate_buffer(BASIC);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {

            openlcb_msg->payload_type = (payload_type_enum_t)0xFFFF;
            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_basic_messages_allocated();
            EXPECT_EQ(count, 1);
        }
    }

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(DATAGRAM);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {

            openlcb_msg->payload_type = (payload_type_enum_t)0xFFFF;
            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_datagram_messages_allocated();
            EXPECT_EQ(count, 1);
        }
    }

    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(SNIP);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            openlcb_msg->payload_type = (payload_type_enum_t)0xFFFF;
            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_snip_messages_allocated();
            EXPECT_EQ(count, 1);
        }
    }

    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg = BufferStore_allocate_buffer(STREAM);
        EXPECT_NE(openlcb_msg, nullptr);

        if (openlcb_msg)
        {
            openlcb_msg->payload_type = (payload_type_enum_t)0xFFFF;
            BufferStore_free_buffer(openlcb_msg);
            count = BufferStore_stream_messages_allocated();
            EXPECT_EQ(count, 1);
        }
    }
}

TEST(OpenlcbBufferStore, max_messaages)
{

    openlcb_msg_t *openlcb_msg1;
    openlcb_msg_t *openlcb_msg2;
    openlcb_msg_t *openlcb_msg3;

    BufferStore_initialize();

    openlcb_msg1 = BufferStore_allocate_buffer(BASIC);
    openlcb_msg2 = BufferStore_allocate_buffer(BASIC);
    openlcb_msg3 = BufferStore_allocate_buffer(BASIC);

    BufferStore_free_buffer(openlcb_msg1);

    openlcb_msg1 = BufferStore_allocate_buffer(BASIC);

    BufferStore_free_buffer(openlcb_msg1);
    BufferStore_free_buffer(openlcb_msg2);
    BufferStore_free_buffer(openlcb_msg3);

    // Datagram

    openlcb_msg1 = BufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg2 = BufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg3 = BufferStore_allocate_buffer(DATAGRAM);

    BufferStore_free_buffer(openlcb_msg1);

    openlcb_msg1 = BufferStore_allocate_buffer(DATAGRAM);

    BufferStore_free_buffer(openlcb_msg1);
    BufferStore_free_buffer(openlcb_msg2);
    BufferStore_free_buffer(openlcb_msg3);

    // SNIP

    openlcb_msg1 = BufferStore_allocate_buffer(SNIP);
    openlcb_msg2 = BufferStore_allocate_buffer(SNIP);
    openlcb_msg3 = BufferStore_allocate_buffer(SNIP);

    BufferStore_free_buffer(openlcb_msg1);

    openlcb_msg1 = BufferStore_allocate_buffer(SNIP);

    BufferStore_free_buffer(openlcb_msg1);
    BufferStore_free_buffer(openlcb_msg2);
    BufferStore_free_buffer(openlcb_msg3);

    // Stream

    openlcb_msg1 = BufferStore_allocate_buffer(STREAM);
    openlcb_msg2 = BufferStore_allocate_buffer(STREAM);
    openlcb_msg3 = BufferStore_allocate_buffer(STREAM);

    BufferStore_free_buffer(openlcb_msg1);

    openlcb_msg1 = BufferStore_allocate_buffer(STREAM);

    BufferStore_free_buffer(openlcb_msg1);
    BufferStore_free_buffer(openlcb_msg2);
    BufferStore_free_buffer(openlcb_msg3);
}
