/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Test suite for OpenLCB Buffer FIFO
 * Refactored and enhanced for maximum code coverage
 */

#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_types.h"

// ============================================================================
// EXISTING TESTS - Cleaned up and improved
// ============================================================================

/**
 * @brief Test basic initialization of FIFO
 * 
 * Verifies:
 * - FIFO is empty after initialization
 * - Count is zero
 * - is_empty returns true
 */
TEST(OpenLcbBufferFIFO, initialize)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Verify FIFO starts empty
    int count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // Verify is_empty returns non-zero (true)
    bool is_empty = OpenLcbBufferFifo_is_empty();
    EXPECT_TRUE(is_empty);
}

/**
 * @brief Test allocation and basic FIFO operations
 * 
 * Verifies:
 * - Can allocate a buffer from buffer store
 * - Can push buffer into FIFO
 * - Count increments correctly
 * - Pop returns same buffer
 * - Buffer can be freed
 */
TEST(OpenLcbBufferFIFO, allocate_basic_buffer)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Allocate a basic buffer
    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);

    if (openlcb_msg)
    {
        // Push it into FIFO
        openlcb_msg_t *push_result = OpenLcbBufferFifo_push(openlcb_msg);
        EXPECT_NE(push_result, nullptr);
        EXPECT_EQ(push_result, openlcb_msg);

        // Verify count is now 1
        int count = OpenLcbBufferFifo_get_allocated_count();
        EXPECT_EQ(count, 1);

        // Pop it and verify it's the same object
        openlcb_msg_t *popped_openlcb_msg = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped_openlcb_msg, openlcb_msg);

        // Free the buffer
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

/**
 * @brief Test basic push and pop operations
 * 
 * Verifies:
 * - Push returns non-NULL on success
 * - Count increments on push
 * - is_empty returns false when FIFO has items
 * - Pop returns correct message
 * - Count decrements on pop
 * - is_empty returns true after popping all items
 */
TEST(OpenLcbBufferFIFO, push_pop)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Allocate and verify a buffer
    openlcb_msg_t *new_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(new_msg, nullptr);

    // Push and check the return value
    openlcb_msg_t *pushed = OpenLcbBufferFifo_push(new_msg);
    EXPECT_NE(pushed, nullptr);
    EXPECT_EQ(pushed, new_msg);

    // Verify it registered (count = 1)
    int count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 1);

    // Verify FIFO claims it's not empty
    bool is_empty = OpenLcbBufferFifo_is_empty();
    EXPECT_FALSE(is_empty);

    // Pop it into a different variable
    openlcb_msg_t *popped_msg = OpenLcbBufferFifo_pop();

    // Verify it unregistered (count = 0)
    count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // Verify FIFO is now empty
    is_empty = OpenLcbBufferFifo_is_empty();
    EXPECT_TRUE(is_empty);

    // Verify we popped the same message
    EXPECT_EQ(popped_msg, new_msg);

    // Free the message
    OpenLcbBufferStore_free_buffer(new_msg);
}

/**
 * @brief Stress test - fill FIFO completely with all buffer types
 * 
 * Verifies:
 * - Can fill FIFO to maximum capacity
 * - Correctly rejects overflow push
 * - All messages are popped in FIFO order
 * - Count decrements correctly during drain
 * - Tests with mix of BASIC, DATAGRAM, SNIP, and STREAM buffers
 */
TEST(OpenLcbBufferFIFO, stress_test)
{
    openlcb_msg_t *new_msg_array[LEN_MESSAGE_BUFFER];
    int array_index = 0;

    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Allocate BASIC buffers
    for (int i = array_index; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {
        new_msg_array[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_NE(new_msg_array[i], nullptr);
        array_index++;
    }

    // Allocate DATAGRAM buffers
    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH); i++)
    {
        new_msg_array[i] = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        EXPECT_NE(new_msg_array[i], nullptr);
        array_index++;
    }

    // Allocate SNIP buffers
    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH); i++)
    {
        new_msg_array[i] = OpenLcbBufferStore_allocate_buffer(SNIP);
        EXPECT_NE(new_msg_array[i], nullptr);
        array_index++;
    }

    // Allocate STREAM buffers
    for (int i = array_index; i < (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH); i++)
    {
        new_msg_array[i] = OpenLcbBufferStore_allocate_buffer(STREAM);
        EXPECT_NE(new_msg_array[i], nullptr);
        array_index++;
    }

    // Push all buffers and verify each push succeeds and count increments
    int count = 0;
    openlcb_msg_t *result = NULL;
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        result = OpenLcbBufferFifo_push(new_msg_array[i]);
        EXPECT_NE(result, nullptr);
        
        count = OpenLcbBufferFifo_get_allocated_count();
        EXPECT_EQ(count, i + 1);
        
        bool is_empty = OpenLcbBufferFifo_is_empty();
        EXPECT_FALSE(is_empty);
    }

    // Try to push one too many - should fail (FIFO full)
    openlcb_msg_t overflow_msg;
    result = OpenLcbBufferFifo_push(&overflow_msg);
    EXPECT_EQ(result, nullptr);

    // Pop all messages and verify FIFO order and count
    openlcb_msg_t *popped_msg;
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        popped_msg = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped_msg, new_msg_array[i]);
        
        count = OpenLcbBufferFifo_get_allocated_count();
        EXPECT_EQ(count, LEN_MESSAGE_BUFFER - i - 1);
    }

    // Free all the buffers
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        OpenLcbBufferStore_free_buffer(new_msg_array[i]);
    }
}

/**
 * @brief Test popping from empty FIFO
 * 
 * Verifies:
 * - Popping from empty FIFO returns NULL
 * - No crash or undefined behavior
 */
TEST(OpenLcbBufferFIFO, pop_empty)
{
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *popped_msg = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped_msg, nullptr);
}

/**
 * @brief Test head and tail wraparound behavior
 * 
 * Verifies:
 * - FIFO correctly wraps head and tail pointers
 * - Messages are retrieved in correct order after wraparound
 * - Count calculation is correct during wraparound
 */
TEST(OpenLcbBufferFIFO, wrap_head_tail)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t test_msg[LEN_MESSAGE_BUFFER];
    openlcb_msg_t *next;
    openlcb_msg_t *test_msg_pop;
    int count;

    // Make the head/tail wrap a few times
    for (int j = 0; j < 3; j++)
    {
        next = &test_msg[0];

        // Fill the FIFO
        for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
        {
            OpenLcbBufferFifo_push(next);
            next++;
        }

        // Drain the FIFO and verify order
        next = &test_msg[0];
        for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
        {
            test_msg_pop = OpenLcbBufferFifo_pop();
            EXPECT_EQ(test_msg_pop, next);
            
            count = OpenLcbBufferFifo_get_allocated_count();
            EXPECT_EQ(count, LEN_MESSAGE_BUFFER - i - 1);
            next++;
        }
    }
}

// ============================================================================
// NEW TESTS - For maximum coverage
// ============================================================================

/**
 * @brief Test pushing NULL pointer
 *
 * Verifies:
 * - FIFO accepts NULL pointer (as per implementation - no validation)
 * - NULL can be pushed and popped
 * - Count still increments/decrements correctly
 *
 * Note: Current implementation doesn't validate NULL, so this tests actual behavior
 * The push function returns the pointer you passed (including NULL) on success
 */
TEST(OpenLcbBufferFIFO, push_null_pointer)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Push NULL - implementation allows this and returns NULL (the value we passed)
    openlcb_msg_t *result = OpenLcbBufferFifo_push(NULL);
    EXPECT_EQ(result, nullptr);  // Push succeeds, returns the NULL we passed

    // Verify count incremented
    int count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 1);

    // Verify FIFO is not empty even though it contains NULL
    bool is_empty = OpenLcbBufferFifo_is_empty();
    EXPECT_FALSE(is_empty);

    // Pop the NULL back
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, nullptr);

    // Verify count decremented
    count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // Verify FIFO is now empty
    is_empty = OpenLcbBufferFifo_is_empty();
    EXPECT_TRUE(is_empty);
}

/**
 * @brief Test FIFO with exactly one element
 * 
 * Verifies:
 * - Boundary condition with single element
 * - is_empty transitions correctly
 * - Count is accurate
 */
TEST(OpenLcbBufferFIFO, single_element)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Start empty
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    // Push one element
    openlcb_msg_t *result = OpenLcbBufferFifo_push(msg);
    EXPECT_EQ(result, msg);

    // Verify not empty, count = 1
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    // Pop it
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, msg);

    // Verify empty again
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    OpenLcbBufferStore_free_buffer(msg);
}

TEST(OpenLcbBufferFIFO, nearly_full)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t test_msgs[LEN_MESSAGE_BUFFER];

    // Push all but one (LEN_MESSAGE_BUFFER - 1) using stack-allocated test messages
    for (int i = 0; i < LEN_MESSAGE_BUFFER - 1; i++)
    {
        openlcb_msg_t *result = OpenLcbBufferFifo_push(&test_msgs[i]);
        EXPECT_EQ(result, &test_msgs[i]);
    }

    // Verify count is exactly capacity - 1
    int count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, LEN_MESSAGE_BUFFER - 1);

    // Should be able to push one more
    openlcb_msg_t *result = OpenLcbBufferFifo_push(&test_msgs[LEN_MESSAGE_BUFFER - 1]);
    EXPECT_EQ(result, &test_msgs[LEN_MESSAGE_BUFFER - 1]);

    // Now FIFO is full
    count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, LEN_MESSAGE_BUFFER);

    // Try to push one more - should fail
    openlcb_msg_t overflow;
    result = OpenLcbBufferFifo_push(&overflow);
    EXPECT_EQ(result, nullptr);

    // Clean up - just pop them all (no need to free since they're stack-allocated)
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        OpenLcbBufferFifo_pop();
    }
}

/**
 * @brief Test interleaved push and pop operations
 * 
 * Verifies:
 * - Can push and pop in mixed sequence
 * - FIFO order maintained
 * - Count accurate throughout
 */
TEST(OpenLcbBufferFIFO, interleaved_operations)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(BASIC);
    
    ASSERT_NE(msg1, nullptr);
    ASSERT_NE(msg2, nullptr);
    ASSERT_NE(msg3, nullptr);

    // Push msg1
    OpenLcbBufferFifo_push(msg1);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    // Push msg2
    OpenLcbBufferFifo_push(msg2);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 2);

    // Pop msg1 (should be first in)
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, msg1);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    // Push msg3
    OpenLcbBufferFifo_push(msg3);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 2);

    // Pop msg2 (should be next)
    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, msg2);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    // Pop msg3 (should be last)
    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, msg3);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    // Verify empty
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());

    // Clean up
    OpenLcbBufferStore_free_buffer(msg1);
    OpenLcbBufferStore_free_buffer(msg2);
    OpenLcbBufferStore_free_buffer(msg3);
}

/**
 * @brief Test refilling FIFO after complete drain
 * 
 * Verifies:
 * - FIFO can be reused after draining
 * - Second fill works same as first
 * - No state corruption from previous use
 */
TEST(OpenLcbBufferFIFO, refill_after_drain)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    const int TEST_COUNT = 5;
    openlcb_msg_t *msgs[TEST_COUNT];

    // Allocate messages
    for (int i = 0; i < TEST_COUNT; i++)
    {
        msgs[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msgs[i], nullptr);
    }

    // First cycle: Fill and drain
    for (int i = 0; i < TEST_COUNT; i++)
    {
        OpenLcbBufferFifo_push(msgs[i]);
    }
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), TEST_COUNT);

    for (int i = 0; i < TEST_COUNT; i++)
    {
        openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped, msgs[i]);
    }
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());

    // Second cycle: Refill and drain (different order to test)
    for (int i = TEST_COUNT - 1; i >= 0; i--)
    {
        OpenLcbBufferFifo_push(msgs[i]);
    }
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), TEST_COUNT);

    for (int i = TEST_COUNT - 1; i >= 0; i--)
    {
        openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped, msgs[i]);
    }
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());

    // Clean up
    for (int i = 0; i < TEST_COUNT; i++)
    {
        OpenLcbBufferStore_free_buffer(msgs[i]);
    }
}

/**
 * @brief Test count accuracy during wraparound
 * 
 * Verifies:
 * - Count calculation handles wraparound correctly
 * - Both wraparound cases tested (tail > head and head > tail)
 */
TEST(OpenLcbBufferFIFO, count_accuracy_wraparound)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t msgs[LEN_MESSAGE_BUFFER];

    // Fill completely to move pointers
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        OpenLcbBufferFifo_push(&msgs[i]);
    }

    // Drain half
    int half = LEN_MESSAGE_BUFFER / 2;
    for (int i = 0; i < half; i++)
    {
        OpenLcbBufferFifo_pop();
    }

    // Verify count
    int count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, LEN_MESSAGE_BUFFER - half);

    // Refill what we drained to cause wraparound
    for (int i = 0; i < half; i++)
    {
        OpenLcbBufferFifo_push(&msgs[i]);
    }

    // Count should be back to full
    count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, LEN_MESSAGE_BUFFER);

    // Drain a few more
    for (int i = 0; i < 3; i++)
    {
        OpenLcbBufferFifo_pop();
    }

    // Verify count after wraparound drain
    count = OpenLcbBufferFifo_get_allocated_count();
    EXPECT_EQ(count, LEN_MESSAGE_BUFFER - 3);
}

/**
 * @brief Test that FIFO doesn't modify message contents
 * 
 * Verifies:
 * - Messages retrieved are identical to what was pushed
 * - FIFO only stores pointers, doesn't copy data
 * - Message integrity maintained
 */
TEST(OpenLcbBufferFIFO, message_integrity)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Set some identifiable values in the message
    msg->mti = 0x1234;
    msg->source_alias = 0xABCD;
    msg->dest_alias = 0x5678;
    msg->payload_count = 5;

    // Push it
    OpenLcbBufferFifo_push(msg);

    // Pop it
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();

    // Verify it's the exact same pointer
    EXPECT_EQ(popped, msg);

    // Verify contents unchanged
    EXPECT_EQ(popped->mti, 0x1234);
    EXPECT_EQ(popped->source_alias, 0xABCD);
    EXPECT_EQ(popped->dest_alias, 0x5678);
    EXPECT_EQ(popped->payload_count, 5);

    OpenLcbBufferStore_free_buffer(msg);
}

/**
 * @brief Test multiple re-initialization
 * 
 * Verifies:
 * - FIFO can be re-initialized
 * - Re-initialization clears the FIFO
 * - No memory leaks or corruption from re-init
 */
TEST(OpenLcbBufferFIFO, reinitialize)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    
    ASSERT_NE(msg1, nullptr);
    ASSERT_NE(msg2, nullptr);

    // Push some messages
    OpenLcbBufferFifo_push(msg1);
    OpenLcbBufferFifo_push(msg2);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 2);

    // Re-initialize
    OpenLcbBufferFifo_initialize();

    // FIFO should be empty
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    // Should be able to use FIFO again
    OpenLcbBufferFifo_push(msg1);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, msg1);

    // Clean up
    OpenLcbBufferStore_free_buffer(msg1);
    OpenLcbBufferStore_free_buffer(msg2);
}

 /**
 * @brief Test alternating push/pop pattern
 * 
 * Verifies:
 * - FIFO handles repeated single push/pop cycles
 * - No state accumulation or drift
 * - Pointers wrap correctly with this pattern
 */
TEST(OpenLcbBufferFIFO, alternating_push_pop)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Do many cycles of push-one, pop-one
    for (int i = 0; i < LEN_MESSAGE_BUFFER * 3; i++)
    {
        // Push
        openlcb_msg_t *pushed = OpenLcbBufferFifo_push(msg);
        EXPECT_EQ(pushed, msg);
        EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
        EXPECT_FALSE(OpenLcbBufferFifo_is_empty());

        // Pop
        openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped, msg);
        EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    }

    OpenLcbBufferStore_free_buffer(msg);
}

/**
 * @brief Test partial fill cycles
 * 
 * Verifies:
 * - FIFO handles partial fill/drain cycles
 * - Count remains accurate
 * - No state corruption
 */
TEST(OpenLcbBufferFIFO, partial_fill_cycles)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    const int PARTIAL_SIZE = LEN_MESSAGE_BUFFER / 3;
    openlcb_msg_t *msgs[PARTIAL_SIZE];

    // Allocate messages
    for (int i = 0; i < PARTIAL_SIZE; i++)
    {
        msgs[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msgs[i], nullptr);
    }

    // Do several partial fill/drain cycles
    for (int cycle = 0; cycle < 5; cycle++)
    {
        // Partial fill
        for (int i = 0; i < PARTIAL_SIZE; i++)
        {
            OpenLcbBufferFifo_push(msgs[i]);
        }
        EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), PARTIAL_SIZE);

        // Partial drain (drain half)
        for (int i = 0; i < PARTIAL_SIZE / 2; i++)
        {
            openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
            EXPECT_EQ(popped, msgs[i]);
        }
        EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), PARTIAL_SIZE - PARTIAL_SIZE / 2);

        // Drain rest
        for (int i = PARTIAL_SIZE / 2; i < PARTIAL_SIZE; i++)
        {
            openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
            EXPECT_EQ(popped, msgs[i]);
        }
        EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
    }

    // Clean up
    for (int i = 0; i < PARTIAL_SIZE; i++)
    {
        OpenLcbBufferStore_free_buffer(msgs[i]);
    }
}

/**
 * @brief Test mixed buffer type operations
 * 
 * Verifies:
 * - FIFO can handle different buffer types in same queue
 * - Order preserved regardless of buffer type
 * - Each type maintains its identity
 */
TEST(OpenLcbBufferFIFO, mixed_buffer_types)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    // Allocate different types
    openlcb_msg_t *basic = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *datagram = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t *snip = OpenLcbBufferStore_allocate_buffer(SNIP);
    openlcb_msg_t *stream = OpenLcbBufferStore_allocate_buffer(STREAM);

    ASSERT_NE(basic, nullptr);
    ASSERT_NE(datagram, nullptr);
    ASSERT_NE(snip, nullptr);
    ASSERT_NE(stream, nullptr);

    // Push in specific order
    OpenLcbBufferFifo_push(basic);
    OpenLcbBufferFifo_push(datagram);
    OpenLcbBufferFifo_push(snip);
    OpenLcbBufferFifo_push(stream);

    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 4);

    // Pop and verify order and type
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, basic);
    EXPECT_EQ(popped->payload_type, BASIC);

    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, datagram);
    EXPECT_EQ(popped->payload_type, DATAGRAM);

    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, snip);
    EXPECT_EQ(popped->payload_type, SNIP);

    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, stream);
    EXPECT_EQ(popped->payload_type, STREAM);

    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());

    // Clean up
    OpenLcbBufferStore_free_buffer(basic);
    OpenLcbBufferStore_free_buffer(datagram);
    OpenLcbBufferStore_free_buffer(snip);
    OpenLcbBufferStore_free_buffer(stream);
}

/**
 * @brief Test FIFO behavior at exact wraparound boundary
 * 
 * Verifies:
 * - Head and tail wrap correctly at buffer boundary
 * - No off-by-one errors at wraparound point
 */
TEST(OpenLcbBufferFIFO, wraparound_boundary)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();

    openlcb_msg_t msgs[LEN_MESSAGE_BUFFER + 2];

    // Fill to capacity
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        OpenLcbBufferFifo_push(&msgs[i]);
    }

    // At this point, head should be at position 0 (wrapped)
    // and tail should be at position 0
    // This is the "full" condition

    // Pop one to make room
    openlcb_msg_t *popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, &msgs[0]);

    // Now tail is at 1, head is at 0 (after wrap)
    // Push one more - this tests wraparound insertion
    openlcb_msg_t *result = OpenLcbBufferFifo_push(&msgs[LEN_MESSAGE_BUFFER]);
    EXPECT_EQ(result, &msgs[LEN_MESSAGE_BUFFER]);

    // Count should be full again
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), LEN_MESSAGE_BUFFER);

    // Pop and verify we get the expected order
    for (int i = 1; i < LEN_MESSAGE_BUFFER; i++)
    {
        popped = OpenLcbBufferFifo_pop();
        EXPECT_EQ(popped, &msgs[i]);
    }

    // Last pop should get the wrapped message
    popped = OpenLcbBufferFifo_pop();
    EXPECT_EQ(popped, &msgs[LEN_MESSAGE_BUFFER]);

    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
}
