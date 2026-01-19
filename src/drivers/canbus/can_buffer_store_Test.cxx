/*******************************************************************************
 * File: can_buffer_store_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for the CAN Buffer Store module.
 *   Tests buffer allocation, deallocation, initialization, and stress conditions.
 *
 * Module Under Test:
 *   CanBufferStore - Fixed-size pool allocator for CAN message buffers
 *
 * Test Coverage:
 *   - Module initialization and reset
 *   - Buffer allocation and deallocation
 *   - Allocation tracking (current and peak usage)
 *   - Buffer overflow conditions
 *   - NULL pointer handling
 *   - Double-free protection
 *   - Message clearing utilities
 *   - Stress testing (fill and drain buffer pool)
 *
 * Design Notes:
 *   The CAN Buffer Store implements a simple fixed-size pool allocator.
 *   It maintains USER_DEFINED_CAN_MSG_BUFFER_DEPTH buffers (typically 10).
 *   Each buffer is a can_msg_t structure containing:
 *   - 29-bit CAN identifier
 *   - 0-8 payload bytes
 *   - Allocation state flag
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_buffer_store.h"
#include "can_utilities.h"
#include "can_types.h"

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Initialize the buffer store for each test.
 * Ensures clean state with all buffers freed.
 */
void setup_buffer_store_test(void)
{
    CanBufferStore_initialize();
}

/*******************************************************************************
 * Basic Functionality Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies that the buffer store can be initialized without errors.
 * All buffers should be freed and counters reset.
 */
TEST(CAN_BufferStore, initialize)
{
    setup_buffer_store_test();
    
    // Verify all buffers are free
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
    EXPECT_EQ(CanBufferStore_messages_max_allocated(), 0);
}

/**
 * Test: Allocate and free single buffer
 * Verifies basic allocation/deallocation cycle and counter tracking.
 * 
 * Tests:
 * - Successful allocation returns non-NULL pointer
 * - Allocated buffer has correct initial state
 * - Allocation counters increment correctly
 * - Deallocation decrements counters
 * - Max allocation tracking works
 * - Clear max resets peak counter
 */
TEST(CAN_BufferStore, allocate_buffer)
{
    setup_buffer_store_test();

    // Allocate a buffer
    can_msg_t *can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);

    // Verify allocation counters
    int count = CanBufferStore_messages_allocated();
    EXPECT_EQ(count, 1);
    
    count = CanBufferStore_messages_max_allocated();
    EXPECT_EQ(count, 1);

    // Clear max counter and verify
    CanBufferStore_clear_max_allocated();
    
    count = CanBufferStore_messages_allocated();
    EXPECT_EQ(count, 1);  // Current count unchanged
    
    count = CanBufferStore_messages_max_allocated();
    EXPECT_EQ(count, 0);  // Max counter reset

    // Verify buffer initial state
    if (can_msg)
    {
        EXPECT_EQ(can_msg->state.allocated, 1);
        EXPECT_EQ(can_msg->identifier, 0);
        EXPECT_EQ(can_msg->payload_count, 0);

        // Free buffer and verify counter
        CanBufferStore_free_buffer(can_msg);
        EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
    }
}

/**
 * Test: Clear message utility
 * Verifies that CanUtilities_clear_can_message() properly zeros all fields.
 * 
 * This tests the utility function that clears a CAN message structure,
 * ensuring all fields are reset to zero/empty state.
 */
TEST(CAN_BufferStore, clear_message)
{
    setup_buffer_store_test();

    can_msg_t *can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);

    if (can_msg)
    {
        // Fill message with non-zero data
        can_msg->identifier = 0xFFFFFFFF;
        can_msg->payload_count = LEN_CAN_BYTE_ARRAY;

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            can_msg->payload[i] = i;
        }

        // Clear the message
        CanUtilities_clear_can_message(can_msg);

        // Verify all fields are zeroed
        EXPECT_EQ(can_msg->identifier, 0);
        EXPECT_EQ(can_msg->payload_count, 0);

        for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        {
            EXPECT_EQ(can_msg->payload[i], 0);
        }

        CanBufferStore_free_buffer(can_msg);
    }
}

/*******************************************************************************
 * Stress and Boundary Tests
 ******************************************************************************/

/**
 * Test: Buffer pool exhaustion
 * Verifies correct behavior when buffer pool is completely filled.
 * 
 * Tests:
 * - Can allocate all USER_DEFINED_CAN_MSG_BUFFER_DEPTH buffers
 * - Allocation fails (returns NULL) when pool is exhausted
 * - Freeing a buffer makes space for new allocation
 * - Counters accurately track allocation/deallocation
 * - All buffers can be freed without issues
 */
TEST(CAN_BufferStore, stress_buffer)
{
    setup_buffer_store_test();

    can_msg_t *can_msg_array[USER_DEFINED_CAN_MSG_BUFFER_DEPTH];
    can_msg_t *can_msg;

    // Fill the entire buffer pool
    for (int i = 0; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        can_msg_array[i] = CanBufferStore_allocate_buffer();
        EXPECT_NE(can_msg_array[i], nullptr) << "Failed to allocate buffer " << i;
    }

    // Try to allocate one more - should fail
    can_msg = CanBufferStore_allocate_buffer();
    EXPECT_EQ(can_msg, nullptr) << "Should fail when buffer pool is full";

    // Free one buffer
    CanBufferStore_free_buffer(can_msg_array[0]);

    // Verify counter decremented
    int count = CanBufferStore_messages_allocated();
    EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - 1);

    // Free remaining buffers and verify counts
    for (int i = 1; i < USER_DEFINED_CAN_MSG_BUFFER_DEPTH; i++)
    {
        CanBufferStore_free_buffer(can_msg_array[i]);

        count = CanBufferStore_messages_allocated();
        EXPECT_EQ(count, USER_DEFINED_CAN_MSG_BUFFER_DEPTH - i - 1);
    }
    
    // Verify all buffers freed
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
}

/*******************************************************************************
 * Error Handling Tests
 ******************************************************************************/

/**
 * Test: NULL pointer handling
 * Verifies that freeing a NULL pointer doesn't cause crashes or errors.
 * 
 * This is important for defensive programming - the function should
 * gracefully handle NULL input without side effects.
 */
TEST(CAN_BufferStore, null_input)
{
    setup_buffer_store_test();

    // Should not crash or cause errors
    CanBufferStore_free_buffer(nullptr);
    
    // Verify counters unchanged
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
}

/**
 * Test: Double free protection
 * Verifies behavior when freeing the same buffer twice.
 * 
 * Implementation Note:
 * The free function checks the allocated flag. After the first free,
 * the flag is cleared, so a second free is a no-op (safe but incorrect usage).
 * 
 * This test documents the defensive behavior - double free doesn't
 * corrupt counters or cause crashes, though it represents a logic error
 * in the calling code.
 */
TEST(CAN_BufferStore, double_free_protection)
{
    setup_buffer_store_test();
    
    can_msg_t *msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(msg, nullptr);
    EXPECT_EQ(CanBufferStore_messages_allocated(), 1);
    
    // First free - normal operation
    CanBufferStore_free_buffer(msg);
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
    EXPECT_EQ(msg->state.allocated, 0);
    
    // Second free - should be safe (allocated flag already clear)
    // The implementation checks allocated flag before decrementing
    CanBufferStore_free_buffer(msg);
    
    // Verify counter didn't underflow
    // The implementation checks: if (!msg) return; then decrements
    // Since allocated flag is 0, this is actually unsafe - it WILL decrement
    int count = CanBufferStore_messages_allocated();
    
    // This documents a potential bug: counter can go negative on double-free
    // The test will fail if implementation doesn't guard against this
    EXPECT_GE(count, 0) << "Counter should not go negative on double-free";
}

/*******************************************************************************
 * End of Test Suite
 ******************************************************************************/
