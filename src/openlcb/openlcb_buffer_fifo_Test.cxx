#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_types.h"

//TEST(OpenLcbBufferFIFO, initialize)
//{
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    int count = BufferFifo_get_allocated_count();
//    EXPECT_EQ(count, 0);
//
//    count = BufferFifo_is_empty();
//    EXPECT_NE(count, 0);
//}
//
//TEST(OpenLcbBufferFIFO, allocate_basic_buffer)
//{
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    // Allocate a basic buffer
//    openlcb_msg_t *openlcb_msg = BufferStore_allocate_buffer(BASIC);
//    EXPECT_NE(openlcb_msg, nullptr);
//
//    if (openlcb_msg)
//    {
//
//        // Push it
//        openlcb_msg = BufferFifo_push(openlcb_msg);
//        EXPECT_NE(openlcb_msg, nullptr);
//
//        // Should be one allocated
//        int count = BufferFifo_get_allocated_count();
//        EXPECT_EQ(count, 1);
//
//        // Pop it and see if it is the same object
//        openlcb_msg_t *popped_openlcb_msg = BufferFifo_pop();
//        EXPECT_EQ(popped_openlcb_msg, openlcb_msg);
//
//        BufferStore_free_buffer(openlcb_msg);
//    }
//}
//
//TEST(OpenLcbBufferFIFO, push_pop)
//{
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    // allocate and check a buffer
//    openlcb_msg_t *new_msg = BufferStore_allocate_buffer(BASIC);
//    EXPECT_NE(new_msg, nullptr);
//
//    // push and check the return
//    bool pushed = BufferFifo_push(new_msg);
//    EXPECT_TRUE(pushed);
//
//    // checked it registered it
//    int count = BufferFifo_get_allocated_count();
//    EXPECT_EQ(count, 1);
//
//    // check is claims its not empty
//    count = BufferFifo_is_empty();
//    EXPECT_EQ(count, 0);
//
//    // pop it into a different variable
//    openlcb_msg_t *popped_msg = BufferFifo_pop();
//
//    // check it unregistered it
//    count = BufferFifo_get_allocated_count();
//    EXPECT_EQ(count, 0);
//
//    // check it is not empty
//    count = BufferFifo_is_empty();
//    EXPECT_NE(count, 0);
//
//    // Check did it pop the same message?
//    EXPECT_EQ(popped_msg, new_msg);
//
//    // free the message
//    BufferStore_free_buffer(new_msg);
//}
//
//TEST(OpenLcbBufferFIFO, stress_test)
//{
//    openlcb_msg_t *new_msg_array[LEN_MESSAGE_BUFFER];
//    int array_index = 0;
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    // Allocate enough buffers to fill the FIFO
//    for (int i = array_index; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
//    {
//        new_msg_array[i] = BufferStore_allocate_buffer(BASIC);
//
//        if (new_msg_array[i] == nullptr)
//            EXPECT_NE(new_msg_array[i], nullptr);
//
//        array_index++;
//    }
//
//    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH); i++)
//    {
//
//        new_msg_array[i] = BufferStore_allocate_buffer(DATAGRAM);
//        EXPECT_NE(new_msg_array[i], nullptr);
//
//        array_index++;
//    }
//
//    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH); i++)
//    {
//
//        new_msg_array[i] = BufferStore_allocate_buffer(SNIP);
//        EXPECT_NE(new_msg_array[i], nullptr);
//
//        array_index++;
//    }
//
//    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH); i++)
//    {
//
//        new_msg_array[i] = BufferStore_allocate_buffer(STREAM);
//        EXPECT_NE(new_msg_array[i], nullptr);
//
//        array_index++;
//    }
//
//    // Push all the buffers and make sure they each are registered and valid pushes
//    int count = 0;
//    openlcb_msg_t *result = 0;
//    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//    {
//
//        result = BufferFifo_push(new_msg_array[i]);
//        EXPECT_NE(result, nullptr);
//        count = BufferFifo_get_allocated_count();
//        EXPECT_EQ(count, i + 1);
//        count = BufferFifo_is_empty();
//        EXPECT_EQ(count, 0);
//    }
//
//    // Try to push one too many and validate result
//    openlcb_msg_t overflow_msg;
//    result = BufferFifo_push(&overflow_msg);
//    EXPECT_EQ(result, nullptr);
//
//    // Pop them all and check they are what was pushed and then unregistered
//    openlcb_msg_t *popped_msg;
//    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//    {
//        popped_msg = BufferFifo_pop();
//        EXPECT_EQ(popped_msg, new_msg_array[i]);
//        count = BufferFifo_get_allocated_count();
//        EXPECT_EQ(count, LEN_MESSAGE_BUFFER - i - 1);
//    }
//
//    // Free the buffers
//    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//    {
//        BufferStore_free_buffer(new_msg_array[i]);
//    }
//}
//
//TEST(OpenLcbBufferFIFO, push_null)
//{
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    openlcb_msg_t *result = BufferFifo_push(nullptr);
//    EXPECT_EQ(result, nullptr);
//}
//
//TEST(OpenLcbBufferFIFO, pop_empty)
//{
//
//    BufferFifo_initialize();
//
//    openlcb_msg_t *popped_msg = BufferFifo_pop();
//    EXPECT_EQ(popped_msg, nullptr);
//}
//
//TEST(OpenLcbBufferFIFO, wrap_head_tail)
//{
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    openlcb_msg_t test_msg;
//    openlcb_msg_t *next;
//    openlcb_msg_t *test_msg_pop;
//    int count;
//
//    // make the head/tail wrap a few times
//    for (int j = 0; j < 1; j++)
//    {
//        next = &test_msg;
//
//        for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//        {
//            BufferFifo_push(next);
//            next++;
//        }
//
//        next = &test_msg;
//        for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//        {
//            test_msg_pop = BufferFifo_pop();
//            EXPECT_EQ(test_msg_pop, next);
//            count = BufferFifo_get_allocated_count();
//            EXPECT_EQ(count, LEN_MESSAGE_BUFFER - i - 1);
//            next++;
//        }
//    }
//}
//
//TEST(OpenLcbBufferFIFO, push_exisiting)
//{
//    openlcb_msg_t openlcb_msg;
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    openlcb_msg_t *pushed_msg = BufferFifo_push_existing(&openlcb_msg);
//
//    EXPECT_EQ(pushed_msg, &openlcb_msg);
//
//    int count = BufferFifo_get_allocated_count();
//    EXPECT_EQ(count, 1);
//
//    pushed_msg = BufferFifo_pop();
//    EXPECT_EQ(pushed_msg, &openlcb_msg);
//}
//
//TEST(OpenLcbBufferFIFO, push_exisiting_null_rollover)
//{
//    openlcb_msg_t openlcb_msg;
//    openlcb_msg_t *pushed_msg;
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//    {
//        pushed_msg = BufferFifo_push_existing(&openlcb_msg);
//        EXPECT_EQ(pushed_msg, &openlcb_msg);
//    }
//
//    for (int i = 0; i < LEN_MESSAGE_BUFFER / 2; i++)
//    {
//        pushed_msg = BufferFifo_pop();
//    }
//
//    for (int i = 0; i < LEN_MESSAGE_BUFFER / 2; i++)
//    {
//        pushed_msg = BufferFifo_push_existing(&openlcb_msg);
//        EXPECT_EQ(pushed_msg, &openlcb_msg);
//    }
//
//    while (BufferFifo_get_allocated_count() > 0)
//    {
//
//        BufferFifo_pop();
//    }
//}
//
//TEST(OpenLcbBufferFIFO, push_exisiting_full)
//{
//
//    openlcb_msg_t openlcb_msg;
//    openlcb_msg_t *pushed_msg;
//
//    BufferStore_initialize();
//    BufferFifo_initialize();
//
//    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
//    {
//        pushed_msg = BufferFifo_push_existing(&openlcb_msg);
//        EXPECT_EQ(pushed_msg, &openlcb_msg);
//    }
//
//    pushed_msg = BufferFifo_push_existing(&openlcb_msg);
//    EXPECT_EQ(pushed_msg, nullptr);
//
//    while (BufferFifo_get_allocated_count() > 0)
//    {
//
//        BufferFifo_pop();
//    }
//}
