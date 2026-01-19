/*******************************************************************************
 * File: can_buffer_fifo_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for the CAN Buffer FIFO module.
 *   Tests FIFO operations, wraparound behavior, and edge conditions.
 *
 * Module Under Test:
 *   CanBufferFifo - Circular FIFO queue for CAN message buffer pointers
 *
 * Test Coverage:
 *   - Module initialization
 *   - Push and pop operations
 *   - FIFO full/empty detection
 *   - Allocation counting
 *   - Buffer overflow handling
 *   - Circular buffer wraparound
 *   - Pop from empty FIFO
 *   - NULL pointer handling
 *   - Stress testing with multiple iterations
 *
 * Design Notes:
 *   The CAN Buffer FIFO implements a circular queue using head/tail pointers.
 *   Size is LEN_CAN_FIFO_BUFFER = USER_DEFINED_CAN_MSG_BUFFER_DEPTH + 1.
 *   The extra slot is needed to distinguish full from empty.
 *   
 *   The FIFO stores pointers to can_msg_t structures, not the messages
 *   themselves. Actual message storage is managed by CanBufferStore.
 *   
 *   FIFO State:
 *   - Empty: head == tail
 *   - Full:  (head + 1) % size == tail
 *   - Count: (head - tail + size) % size
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "drivers/canbus/can_buffer_fifo.h"
#include "drivers/canbus/can_buffer_store.h"

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Initialize both FIFO and buffer store for each test.
 * The FIFO depends on the buffer store for actual message allocation.
 */
void setup_fifo_test(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
}

/*******************************************************************************
 * Basic Functionality Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies that the FIFO initializes to empty state.
 * 
 * Tests:
 * - Allocation count is zero after initialization
 * - FIFO reports as empty
 */
TEST(CAN_BufferFIFO, initialize)
{
    setup_fifo_test();

    // Verify FIFO is empty
    int count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // Verify empty flag is set (non-zero means empty)
    count = CanBufferFifo_is_empty();
    EXPECT_NE(count, 0);
}

/**
 * Test: Basic push and pop operations
 * Verifies fundamental FIFO operations work correctly.
 * 
 * Tests:
 * - Push operation returns success
 * - Count increments after push
 * - FIFO no longer reports empty
 * - Pop returns the same message that was pushed
 * - Count decrements after pop
 * - FIFO reports empty after popping last message
 * - FIFO maintains FIFO ordering (first in, first out)
 */
TEST(CAN_BufferFIFO, push_pop)
{
    setup_fifo_test();

    // Allocate a buffer from the store
    can_msg_t *new_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(new_msg, nullptr);

    // Push the buffer onto the FIFO
    bool pushed = CanBufferFifo_push(new_msg);
    EXPECT_TRUE(pushed);

    // Verify count incremented
    int count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 1);

    // Verify FIFO no longer empty (zero means not empty)
    count = CanBufferFifo_is_empty();
    EXPECT_EQ(count, 0);

    // Pop the buffer
    can_msg_t *popped_msg = CanBufferFifo_pop();

    // Verify count decremented
    count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 0);

    // Verify FIFO is empty again (non-zero means empty)
    count = CanBufferFifo_is_empty();
    EXPECT_NE(count, 0);

    // Verify we got the same message back (FIFO ordering)
    EXPECT_EQ(popped_msg, new_msg);

    // Free the buffer back to the store
    CanBufferStore_free_buffer(new_msg);
}

/**
 * Test: Pop from empty FIFO
 * Verifies that popping from an empty FIFO returns NULL.
 * 
 * This is important error handling - the FIFO should safely
 * handle underflow conditions.
 */
TEST(CAN_BufferFIFO, pop_empty)
{
    setup_fifo_test();

    // Try to pop from empty FIFO
    can_msg_t *popped_msg = CanBufferFifo_pop();
    
    EXPECT_EQ(popped_msg, nullptr) << "Pop from empty FIFO should return NULL";
}

/*******************************************************************************
 * Stress and Boundary Tests
 ******************************************************************************/

/**
 * Test: FIFO capacity and overflow handling
 * Verifies correct behavior when FIFO is filled to capacity.
 * 
 * Tests:
 * - Can push USER_DEFINED_CAN_MSG_BUFFER_DEPTH messages
 * - Count increments correctly for each push
 * - Push fails when FIFO is full
 * - Messages are retrieved in FIFO order (first in, first out)
 * - Count decrements correctly for each pop
 * - All messages can be popped successfully
 */
TEST(CAN_BufferFIFO, stress)
{
    can_msg_t *new_msg_array[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];

    setup_fifo_test();

    // Allocate enough buffers to fill the FIFO
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        new_msg_array[i] = CanBufferStore_allocate_buffer();
        EXPECT_NE(new_msg_array[i], nullptr) << "Failed to allocate buffer " << i;
    }

    // Push all buffers and verify each operation
    int count = 0;
    int result = 0;
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        result = CanBufferFifo_push(new_msg_array[i]);
        EXPECT_EQ(result, 1) << "Push failed at index " << i;
        
        count = CanBufferFifo_get_allocated_count();
        EXPECT_EQ(count, i + 1) << "Count mismatch at index " << i;
        
        count = CanBufferFifo_is_empty();
        EXPECT_EQ(count, 0) << "FIFO should not be empty";
    }
    
    // Try to push one more - should fail (FIFO full)
    can_msg_t overflow_msg;
    result = CanBufferFifo_push(&overflow_msg);
    EXPECT_EQ(result, 0) << "Push should fail when FIFO is full";

    // Pop all messages and verify FIFO ordering
    can_msg_t *popped_msg;
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        popped_msg = CanBufferFifo_pop();
        
        // Verify FIFO ordering (first in, first out)
        EXPECT_EQ(popped_msg, new_msg_array[i]) << "FIFO ordering violated at index " << i;
        
        count = CanBufferFifo_get_allocated_count();
        EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1) 
            << "Count mismatch at pop index " << i;
    }

    // Free all buffers back to the store
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        CanBufferStore_free_buffer(new_msg_array[i]);
    }
}

/**
 * Test: Circular buffer wraparound
 * Verifies that head/tail pointers wrap correctly at buffer boundaries.
 * 
 * This tests the circular buffer implementation by filling and draining
 * the FIFO multiple times, forcing the head and tail pointers to wrap
 * around the buffer boundaries. This is critical for long-running systems
 * where the pointers will eventually overflow.
 * 
 * Tests:
 * - Multiple fill/drain cycles work correctly
 * - Pointers wrap without corruption
 * - FIFO maintains correctness across wraparound
 * - Counts remain accurate through wraparound
 */
TEST(CAN_BufferFIFO, wrap_head_tail)
{
    setup_fifo_test();

    can_msg_t test_msg;
    can_msg_t *test_msg_pop;
    int count;

    // Perform 10 complete fill/drain cycles to force wraparound
    for (int j = 0; j < 10; j++)
    {
        // Fill the FIFO
        for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
        {
            bool pushed = CanBufferFifo_push(&test_msg);
            EXPECT_TRUE(pushed) << "Push failed at cycle " << j << ", index " << i;
        }

        // Drain the FIFO
        for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
        {
            test_msg_pop = CanBufferFifo_pop();
            
            // Verify we got the message we pushed
            EXPECT_EQ(test_msg_pop, &test_msg) 
                << "Message mismatch at cycle " << j << ", index " << i;
            
            count = CanBufferFifo_get_allocated_count();
            EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1)
                << "Count error at cycle " << j << ", index " << i;
        }
        
        // Verify FIFO is empty after each cycle
        EXPECT_NE(CanBufferFifo_is_empty(), 0) << "FIFO should be empty at end of cycle " << j;
    }
}

/*******************************************************************************
 * Error Handling Tests
 ******************************************************************************/

/**
 * Test: Push NULL pointer
 * Verifies behavior when attempting to push a NULL pointer onto the FIFO.
 * 
 * Design Decision Note:
 * The implementation does not validate the pointer before queuing it.
 * This test documents current behavior - NULL is a valid pointer to queue.
 * The receiving code (pop caller) is responsible for handling NULL pointers.
 * 
 * This is intentional for performance - no validation overhead on push.
 */
TEST(CAN_BufferFIFO, push_null_pointer)
{
    setup_fifo_test();
    
    // Push NULL pointer - implementation allows this
    bool result = CanBufferFifo_push(nullptr);
    EXPECT_TRUE(result) << "Push of NULL pointer succeeds (no validation)";
    
    // Verify count incremented
    int count = CanBufferFifo_get_allocated_count();
    EXPECT_EQ(count, 1);
    
    // Pop should return the NULL we pushed
    can_msg_t *popped = CanBufferFifo_pop();
    EXPECT_EQ(popped, nullptr) << "FIFO returns the NULL pointer we pushed";
    
    // FIFO should be empty again
    EXPECT_NE(CanBufferFifo_is_empty(), 0);
}

/*******************************************************************************
 * End of Test Suite
 ******************************************************************************/
