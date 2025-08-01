#include "test/main_Test.hxx"

#include "openlcb_buffer_list.h"
#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

TEST(OpenLcbBufferList, initialize)
{

    BufferList_initialize();

    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, allocate_release)
{

    BufferList_initialize();

    openlcb_msg_t *msg = BufferStore_allocate_buffer(BASIC);

    EXPECT_NE(msg, nullptr);

    EXPECT_TRUE(BufferList_is_empty());
    openlcb_msg_t *added_msg = BufferList_add(msg);
    EXPECT_EQ(added_msg, msg);
    EXPECT_FALSE(BufferList_is_empty());

    msg = BufferList_release(msg);

    EXPECT_EQ(added_msg, msg);

    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, allocate_free)
{

    BufferList_initialize();

    openlcb_msg_t *msg = BufferStore_allocate_buffer(BASIC);
    EXPECT_NE(msg, nullptr);

    EXPECT_TRUE(BufferList_is_empty());
    openlcb_msg_t *added_msg = BufferList_add(msg);
    EXPECT_EQ(added_msg, msg);
    EXPECT_FALSE(BufferList_is_empty());

    EXPECT_TRUE(BufferList_free(msg));
    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, free_invalid)
{

    openlcb_msg_t msg;

    BufferList_initialize();

    EXPECT_TRUE(BufferList_is_empty());
    openlcb_msg_t *added_msg = BufferList_add(&msg);
    EXPECT_EQ(added_msg, &msg);
    EXPECT_FALSE(BufferList_is_empty());

    EXPECT_FALSE(BufferList_free(&msg + 10));

    EXPECT_TRUE(BufferList_release(&msg));
    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, release_invalid)
{

    openlcb_msg_t msg;

    BufferList_initialize();

    EXPECT_TRUE(BufferList_is_empty());
    openlcb_msg_t *added_msg = BufferList_add(&msg);
    EXPECT_EQ(added_msg, &msg);
    EXPECT_FALSE(BufferList_is_empty());

    EXPECT_EQ(BufferList_release(&msg + 10), nullptr);

    EXPECT_TRUE(BufferList_release(&msg));
    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, allocate_index_of)
{

#define LOOP_COUNT 4

    openlcb_msg_t *msg_array[LOOP_COUNT];

    BufferList_initialize();

    EXPECT_TRUE(BufferList_is_empty());

    for (int i = 0; i < LOOP_COUNT; i++)
    {

        msg_array[i] = BufferStore_allocate_buffer(BASIC);
        EXPECT_NE(msg_array[i], nullptr);
        BufferList_add(msg_array[i]);
    }

    EXPECT_FALSE(BufferList_is_empty());

    for (int i = 0; i < LOOP_COUNT; i++)
    {
        EXPECT_EQ(BufferList_index_of(i), msg_array[i]);
    }

    for (int i = 0; i < LOOP_COUNT; i++)
    {
        BufferList_free(msg_array[i]);
    }

    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, release_null)
{

    BufferList_initialize();

    openlcb_msg_t *msg = nullptr;
    EXPECT_EQ(BufferList_release(msg), nullptr);
}

TEST(OpenLcbBufferList, add_null)
{

    BufferList_initialize();

    openlcb_msg_t *msg = nullptr;
    EXPECT_EQ(BufferList_add(msg), nullptr);
}

TEST(OpenLcbBufferList, release_stress)
{

    openlcb_msg_t msg;
    openlcb_msg_t *msg_ptr;

    BufferList_initialize();

    msg_ptr = &msg;

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        EXPECT_EQ(BufferList_add(msg_ptr), msg_ptr);
        msg_ptr++;
    }

    // Should not work
    EXPECT_EQ(BufferList_add(msg_ptr), nullptr);

    msg_ptr = &msg;
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {

        EXPECT_EQ(BufferList_release(msg_ptr), msg_ptr);
        msg_ptr++;
    }

    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, release_find)
{

    openlcb_msg_t msg;
    openlcb_msg_t msg1;
    openlcb_msg_t msg2;
    openlcb_msg_t msg3;

    BufferList_initialize();

    msg.source_alias = 0x0568;
    msg.dest_alias = 0x0567;
    msg.mti = 0x0222;
    EXPECT_EQ(BufferList_add(&msg), &msg);

    msg1.source_alias = 0x0568;
    msg1.dest_alias = 0x0567;
    msg1.mti = 0x0333;
    EXPECT_EQ(BufferList_add(&msg1), &msg1);

    msg2.source_alias = 0x0568;
    msg2.dest_alias = 0x0777;
    msg2.mti = 0x0222;
    EXPECT_EQ(BufferList_add(&msg2), &msg2);

    msg3.source_alias = 0x0999;
    msg3.dest_alias = 0x0567;
    msg3.mti = 0x0222;
    EXPECT_EQ(BufferList_add(&msg3), &msg3);

    EXPECT_FALSE(BufferList_is_empty());

    EXPECT_EQ(BufferList_find(0x0568, 0x0567, 0x0AAA), nullptr);
    EXPECT_EQ(BufferList_find(0x0568, 0x0777, 0x0333), nullptr);
    EXPECT_EQ(BufferList_find(0x0568, 0x0777, 0x0222), &msg2);
    EXPECT_EQ(BufferList_find(0x0999, 0x0777, 0x0222), nullptr);

    EXPECT_EQ(BufferList_find(0x0568, 0x0567, 0x0222), &msg);
    EXPECT_EQ(BufferList_find(0x0568, 0x0567, 0x0333), &msg1);
    EXPECT_EQ(BufferList_find(0x0568, 0x0777, 0x0333), nullptr);
    EXPECT_EQ(BufferList_find(0x0999, 0x0567, 0x0222), &msg3);

    BufferList_release(&msg);
    BufferList_release(&msg1);
    BufferList_release(&msg2);
    BufferList_release(&msg3);

    EXPECT_TRUE(BufferList_is_empty());
}

TEST(OpenLcbBufferList, invalid_index_of)
{

    BufferList_initialize();

    openlcb_msg_t msg;
    openlcb_msg_t msg1;
    openlcb_msg_t msg2;
    openlcb_msg_t msg3;

    EXPECT_EQ(BufferList_add(&msg), &msg);
    EXPECT_EQ(BufferList_add(&msg1), &msg1);
    EXPECT_EQ(BufferList_add(&msg2), &msg2);
    EXPECT_EQ(BufferList_add(&msg3), &msg3);

    // out of range
    EXPECT_EQ(BufferList_index_of(LEN_MESSAGE_BUFFER + 1), nullptr);

    // in range
    EXPECT_EQ(BufferList_index_of(LEN_MESSAGE_BUFFER - 1), nullptr);

    BufferList_release(&msg);
    BufferList_release(&msg1);
    BufferList_release(&msg2);
    BufferList_release(&msg3);

    EXPECT_TRUE(BufferList_is_empty());
}