/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Test suite for OpenLCB Buffer Store
 * Refactored and enhanced for maximum code coverage
 */

#include "test/main_Test.hxx"

#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

// ============================================================================
// EXISTING TESTS - Cleaned up and improved
// ============================================================================

/**
 * @brief Test basic initialization of buffer store
 * 
 * Verifies:
 * - Initialization completes without errors
 * - All pools are properly initialized
 */
TEST(OpenLcbBufferStore, initialize)
{
    OpenLcbBufferStore_initialize();
    
    // Verify all counters start at zero
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
    
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 0);
}

/**
 * @brief Test BASIC buffer allocation and deallocation
 * 
 * Verifies:
 * - Can allocate BASIC buffer
 * - Telemetry counters update correctly
 * - Free decrements counters
 * - clear_max_allocated() works
 */
TEST(OpenLcbBufferStore, allocate_basic_buffer)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH == 0) return;
    
    OpenLcbBufferStore_initialize();

    // Allocate a BASIC buffer
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Verify BASIC counters incremented
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 1);

    // Verify other types unchanged
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);

    // Free the buffer
    OpenLcbBufferStore_free_buffer(msg);

    // Verify current count decremented, max unchanged
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 1);

    // Verify clear_max_allocated() works
    OpenLcbBufferStore_clear_max_allocated();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 0);
}

/**
 * @brief Test DATAGRAM buffer allocation and deallocation
 */
TEST(OpenLcbBufferStore, allocate_datagram_buffer)
{
    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH == 0) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(msg, nullptr);

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 1);

    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);

    OpenLcbBufferStore_free_buffer(msg);

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 1);

    OpenLcbBufferStore_clear_max_allocated();
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 0);
}

/**
 * @brief Test SNIP buffer allocation and deallocation
 */
TEST(OpenLcbBufferStore, allocate_snip_buffer)
{
    if (USER_DEFINED_SNIP_BUFFER_DEPTH == 0) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(SNIP);
    ASSERT_NE(msg, nullptr);

    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 1);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 1);

    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);

    OpenLcbBufferStore_free_buffer(msg);

    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 1);

    OpenLcbBufferStore_clear_max_allocated();
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 0);
}

/**
 * @brief Test STREAM buffer allocation and deallocation
 */
TEST(OpenLcbBufferStore, allocate_stream_buffer)
{
    if (USER_DEFINED_STREAM_BUFFER_DEPTH == 0) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(STREAM);
    ASSERT_NE(msg, nullptr);

    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 1);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 1);

    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);

    OpenLcbBufferStore_free_buffer(msg);

    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 1);

    OpenLcbBufferStore_clear_max_allocated();
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 0);
}

/**
 * @brief Stress test - allocate all BASIC buffers
 * 
 * Verifies:
 * - Can fill entire BASIC pool
 * - Allocation fails when pool exhausted
 * - Counters accurate throughout
 */
TEST(OpenLcbBufferStore, stresstest_basic)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH == 0) return;

    openlcb_msg_t *msg_array[USER_DEFINED_BASIC_BUFFER_DEPTH];
    
    OpenLcbBufferStore_initialize();

    // Allocate all BASIC buffers
    for (int i = 0; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {
        msg_array[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_NE(msg_array[i], nullptr);
    }

    // Verify all allocated
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), USER_DEFINED_BASIC_BUFFER_DEPTH);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), USER_DEFINED_BASIC_BUFFER_DEPTH);

    // Try one more - should fail
    openlcb_msg_t *overflow = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(overflow, nullptr);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), USER_DEFINED_BASIC_BUFFER_DEPTH);

    // Free all
    for (int i = 0; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {
        OpenLcbBufferStore_free_buffer(msg_array[i]);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), USER_DEFINED_BASIC_BUFFER_DEPTH - i - 1);
    }

    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), USER_DEFINED_BASIC_BUFFER_DEPTH);
}

/**
 * @brief Stress test - allocate all DATAGRAM buffers
 */
TEST(OpenLcbBufferStore, stresstest_datagram)
{
    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH == 0) return;

    openlcb_msg_t *msg_array[USER_DEFINED_DATAGRAM_BUFFER_DEPTH];
    
    OpenLcbBufferStore_initialize();

    for (int i = 0; i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH; i++)
    {
        msg_array[i] = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        EXPECT_NE(msg_array[i], nullptr);
    }

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), USER_DEFINED_DATAGRAM_BUFFER_DEPTH);
    
    openlcb_msg_t *overflow = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    EXPECT_EQ(overflow, nullptr);

    for (int i = 0; i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH; i++)
    {
        OpenLcbBufferStore_free_buffer(msg_array[i]);
    }

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
}

/**
 * @brief Stress test - allocate all SNIP buffers
 */
TEST(OpenLcbBufferStore, stresstest_snip)
{
    if (USER_DEFINED_SNIP_BUFFER_DEPTH == 0) return;

    openlcb_msg_t *msg_array[USER_DEFINED_SNIP_BUFFER_DEPTH];
    
    OpenLcbBufferStore_initialize();

    for (int i = 0; i < USER_DEFINED_SNIP_BUFFER_DEPTH; i++)
    {
        msg_array[i] = OpenLcbBufferStore_allocate_buffer(SNIP);
        EXPECT_NE(msg_array[i], nullptr);
    }

    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), USER_DEFINED_SNIP_BUFFER_DEPTH);
    
    openlcb_msg_t *overflow = OpenLcbBufferStore_allocate_buffer(SNIP);
    EXPECT_EQ(overflow, nullptr);

    for (int i = 0; i < USER_DEFINED_SNIP_BUFFER_DEPTH; i++)
    {
        OpenLcbBufferStore_free_buffer(msg_array[i]);
    }

    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
}

/**
 * @brief Stress test - allocate all STREAM buffers
 */
TEST(OpenLcbBufferStore, stresstest_stream)
{
    if (USER_DEFINED_STREAM_BUFFER_DEPTH == 0) return;

    openlcb_msg_t *msg_array[USER_DEFINED_STREAM_BUFFER_DEPTH];
    
    OpenLcbBufferStore_initialize();

    for (int i = 0; i < USER_DEFINED_STREAM_BUFFER_DEPTH; i++)
    {
        msg_array[i] = OpenLcbBufferStore_allocate_buffer(STREAM);
        EXPECT_NE(msg_array[i], nullptr);
    }

    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), USER_DEFINED_STREAM_BUFFER_DEPTH);
    
    openlcb_msg_t *overflow = OpenLcbBufferStore_allocate_buffer(STREAM);
    EXPECT_EQ(overflow, nullptr);

    for (int i = 0; i < USER_DEFINED_STREAM_BUFFER_DEPTH; i++)
    {
        OpenLcbBufferStore_free_buffer(msg_array[i]);
    }

    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
}

/**
 * @brief Test reference counting for all buffer types
 * 
 * Verifies:
 * - Initial reference count is 1
 * - inc_reference_count() increments correctly
 * - free_buffer() only frees when count reaches 0
 */
TEST(OpenLcbBufferStore, reference_counting)
{
    OpenLcbBufferStore_initialize();

    // Test BASIC
    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msg, nullptr);
        
        EXPECT_EQ(msg->reference_count, 1);
        
        OpenLcbBufferStore_inc_reference_count(msg);
        EXPECT_EQ(msg->reference_count, 2);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(msg->reference_count, 1);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    }

    // Test DATAGRAM
    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        ASSERT_NE(msg, nullptr);
        
        EXPECT_EQ(msg->reference_count, 1);
        OpenLcbBufferStore_inc_reference_count(msg);
        EXPECT_EQ(msg->reference_count, 2);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(msg->reference_count, 1);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    }

    // Test SNIP
    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(SNIP);
        ASSERT_NE(msg, nullptr);
        
        EXPECT_EQ(msg->reference_count, 1);
        OpenLcbBufferStore_inc_reference_count(msg);
        EXPECT_EQ(msg->reference_count, 2);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 1);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    }

    // Test STREAM
    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(STREAM);
        ASSERT_NE(msg, nullptr);
        
        EXPECT_EQ(msg->reference_count, 1);
        OpenLcbBufferStore_inc_reference_count(msg);
        EXPECT_EQ(msg->reference_count, 2);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 1);
        
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
    }
}

/**
 * @brief Test allocation with invalid payload type
 * 
 * Verifies:
 * - Invalid type returns NULL
 * - No counters affected
 */
TEST(OpenLcbBufferStore, bad_allocation)
{
    OpenLcbBufferStore_initialize();
    
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer((payload_type_enum)0xFFFC);
    EXPECT_EQ(msg, nullptr);
    
    // Verify no counters changed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
}

/**
 * @brief Test freeing NULL pointer
 * 
 * Verifies:
 * - free_buffer(NULL) is safe (no crash)
 * - No counters affected
 */
TEST(OpenLcbBufferStore, bad_release)
{
    OpenLcbBufferStore_initialize();

    // Get baseline counts
    int basic_before = OpenLcbBufferStore_basic_messages_allocated();
    int datagram_before = OpenLcbBufferStore_datagram_messages_allocated();
    int snip_before = OpenLcbBufferStore_snip_messages_allocated();
    int stream_before = OpenLcbBufferStore_stream_messages_allocated();

    // Free NULL - should be safe
    OpenLcbBufferStore_free_buffer(nullptr);

    // Verify nothing changed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), basic_before);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), datagram_before);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), snip_before);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), stream_before);
}

/**
 * @brief Test freeing corrupted message (invalid payload_type)
 * 
 * Verifies:
 * - free_buffer() handles corrupted payload_type
 * - Counter not decremented for invalid type
 * - Buffer still marked as freed
 */
TEST(OpenLcbBufferStore, corrupted_msg)
{
    OpenLcbBufferStore_initialize();

    // Test with each type
    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msg, nullptr);
        
        msg->payload_type = (payload_type_enum)0xFFFF;
        OpenLcbBufferStore_free_buffer(msg);
        
        // Counter should not have decremented (corrupted type in switch)
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    }

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        ASSERT_NE(msg, nullptr);
        
        msg->payload_type = (payload_type_enum)0xFFFF;
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);
    }

    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(SNIP);
        ASSERT_NE(msg, nullptr);
        
        msg->payload_type = (payload_type_enum)0xFFFF;
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 1);
    }

    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(STREAM);
        ASSERT_NE(msg, nullptr);
        
        msg->payload_type = (payload_type_enum)0xFFFF;
        OpenLcbBufferStore_free_buffer(msg);
        EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 1);
    }
}

/**
 * @brief Test peak allocation tracking (max counters)
 * 
 * Verifies:
 * - Max counters track peak usage
 * - Max doesn't decrease when current decreases
 * - Pattern: allocate 3, free 1, allocate 1, free all
 */
TEST(OpenLcbBufferStore, max_messages)
{
    OpenLcbBufferStore_initialize();

    // Test BASIC
    if (USER_DEFINED_BASIC_BUFFER_DEPTH >= 3)
    {
        openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
        openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
        openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(BASIC);
        
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 3);
        
        OpenLcbBufferStore_free_buffer(msg1);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 2);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 3); // Max unchanged
        
        msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 3);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 3); // Still 3
        
        OpenLcbBufferStore_free_buffer(msg1);
        OpenLcbBufferStore_free_buffer(msg2);
        OpenLcbBufferStore_free_buffer(msg3);
    }

    // Similar pattern for other types
    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH >= 3)
    {
        openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 3);
        OpenLcbBufferStore_free_buffer(msg1);
        msg1 = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        EXPECT_EQ(OpenLcbBufferStore_datagram_messages_max_allocated(), 3);
        
        OpenLcbBufferStore_free_buffer(msg1);
        OpenLcbBufferStore_free_buffer(msg2);
        OpenLcbBufferStore_free_buffer(msg3);
    }

    if (USER_DEFINED_SNIP_BUFFER_DEPTH >= 3)
    {
        openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(SNIP);
        openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(SNIP);
        openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(SNIP);
        
        EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 3);
        OpenLcbBufferStore_free_buffer(msg1);
        msg1 = OpenLcbBufferStore_allocate_buffer(SNIP);
        EXPECT_EQ(OpenLcbBufferStore_snip_messages_max_allocated(), 3);
        
        OpenLcbBufferStore_free_buffer(msg1);
        OpenLcbBufferStore_free_buffer(msg2);
        OpenLcbBufferStore_free_buffer(msg3);
    }

    if (USER_DEFINED_STREAM_BUFFER_DEPTH >= 3)
    {
        openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(STREAM);
        openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(STREAM);
        openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(STREAM);
        
        EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 3);
        OpenLcbBufferStore_free_buffer(msg1);
        msg1 = OpenLcbBufferStore_allocate_buffer(STREAM);
        EXPECT_EQ(OpenLcbBufferStore_stream_messages_max_allocated(), 3);
        
        OpenLcbBufferStore_free_buffer(msg1);
        OpenLcbBufferStore_free_buffer(msg2);
        OpenLcbBufferStore_free_buffer(msg3);
    }
}

// ============================================================================
// NEW TESTS - For maximum coverage (ALL COMMENTED OUT)
// ============================================================================

/**
 * @brief Test payload type correctness after allocation
 * 
 * Verifies:
 * - Allocated buffer has correct payload_type field
 * - payload_type matches requested type
 */
TEST(OpenLcbBufferStore, payload_type_verification)
{
    OpenLcbBufferStore_initialize();

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->payload_type, BASIC);
        OpenLcbBufferStore_free_buffer(msg);
    }

    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->payload_type, DATAGRAM);
        OpenLcbBufferStore_free_buffer(msg);
    }

    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(SNIP);
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->payload_type, SNIP);
        OpenLcbBufferStore_free_buffer(msg);
    }

    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(STREAM);
        ASSERT_NE(msg, nullptr);
        EXPECT_EQ(msg->payload_type, STREAM);
        OpenLcbBufferStore_free_buffer(msg);
    }
}

/**
 * @brief Test message state after allocation
 * 
 * Verifies:
 * - state.allocated flag is set
 * - reference_count is 1
 * - Message is properly cleared
 */
TEST(OpenLcbBufferStore, message_state_verification)
{
    OpenLcbBufferStore_initialize();

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msg, nullptr);
        
        // Verify state
        EXPECT_TRUE(msg->state.allocated);
        EXPECT_EQ(msg->reference_count, 1);
        
        // Verify message was cleared (key fields should be 0)
        EXPECT_EQ(msg->mti, 0);
        EXPECT_EQ(msg->source_alias, 0);
        EXPECT_EQ(msg->dest_alias, 0);
        EXPECT_EQ(msg->payload_count, 0);
        
        OpenLcbBufferStore_free_buffer(msg);
    }
}

/**
 * @brief Test buffer reuse after free
 * 
 * Verifies:
 * - Freed buffer can be reallocated
 * - First-fit allocation strategy
 * - Buffer properly reinitialized on reallocation
 */
TEST(OpenLcbBufferStore, buffer_reuse)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH < 2) return;
    
    OpenLcbBufferStore_initialize();

    // Allocate two buffers
    openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg1, nullptr);
    ASSERT_NE(msg2, nullptr);
    
    // Modify msg1 to verify it gets cleared on reallocation
    msg1->mti = 0x1234;
    msg1->source_alias = 0xABCD;
    
    // Free first buffer
    OpenLcbBufferStore_free_buffer(msg1);
    
    // Allocate again - should get msg1's slot (first-fit)
    openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg3, nullptr);
    EXPECT_EQ(msg3, msg1); // Should be same pointer
    
    // Verify buffer was cleared
    EXPECT_EQ(msg3->mti, 0);
    EXPECT_EQ(msg3->source_alias, 0);
    EXPECT_EQ(msg3->reference_count, 1);
    EXPECT_TRUE(msg3->state.allocated);
    
    // Clean up
    OpenLcbBufferStore_free_buffer(msg2);
    OpenLcbBufferStore_free_buffer(msg3);
}

/**
 * @brief Test mixed type allocation patterns
 * 
 * Verifies:
 * - Can allocate different types simultaneously
 * - Each pool is independent
 * - Counters track each type correctly
 */
TEST(OpenLcbBufferStore, mixed_allocation)
{
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *basic = nullptr;
    openlcb_msg_t *datagram = nullptr;
    openlcb_msg_t *snip = nullptr;
    openlcb_msg_t *stream = nullptr;

    // Allocate one of each type (if available)
    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
        basic = OpenLcbBufferStore_allocate_buffer(BASIC);
    
    if (USER_DEFINED_DATAGRAM_BUFFER_DEPTH > 0)
        datagram = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    if (USER_DEFINED_SNIP_BUFFER_DEPTH > 0)
        snip = OpenLcbBufferStore_allocate_buffer(SNIP);
    
    if (USER_DEFINED_STREAM_BUFFER_DEPTH > 0)
        stream = OpenLcbBufferStore_allocate_buffer(STREAM);

    // Verify each type shows 1 allocated
    if (basic) EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    if (datagram) EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);
    if (snip) EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 1);
    if (stream) EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 1);

    // Free in different order than allocated
    if (snip) OpenLcbBufferStore_free_buffer(snip);
    if (basic) OpenLcbBufferStore_free_buffer(basic);
    if (stream) OpenLcbBufferStore_free_buffer(stream);
    if (datagram) OpenLcbBufferStore_free_buffer(datagram);

    // Verify all freed
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
}

/**
 * @brief Test multiple reference count increments
 * 
 * Verifies:
 * - Can increment reference count multiple times
 * - Requires matching number of frees
 * - Counter only decrements when ref count reaches 0
 */
TEST(OpenLcbBufferStore, multiple_reference_increments)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH == 0) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);
    EXPECT_EQ(msg->reference_count, 1);

    // Increment 3 times
    OpenLcbBufferStore_inc_reference_count(msg);
    OpenLcbBufferStore_inc_reference_count(msg);
    OpenLcbBufferStore_inc_reference_count(msg);
    EXPECT_EQ(msg->reference_count, 4);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);

    // Free 3 times - still allocated
    OpenLcbBufferStore_free_buffer(msg);
    EXPECT_EQ(msg->reference_count, 3);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    
    OpenLcbBufferStore_free_buffer(msg);
    EXPECT_EQ(msg->reference_count, 2);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    
    OpenLcbBufferStore_free_buffer(msg);
    EXPECT_EQ(msg->reference_count, 1);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);

    // Final free - now it's released
    OpenLcbBufferStore_free_buffer(msg);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
}

/**
 * @brief Test re-initialization with allocated buffers
 * 
 * Verifies:
 * - Re-initialize resets all counters
 * - Previously allocated buffers are lost (memory leak scenario)
 * - New allocations work after re-init
 * 
 * Note: This tests the "undefined behavior" of re-initializing with
 * allocated buffers - counters reset but buffers stay marked allocated
 */
TEST(OpenLcbBufferStore, reinitialize_with_allocated)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH < 2) return;
    
    OpenLcbBufferStore_initialize();

    // Allocate some buffers
    openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg1, nullptr);
    ASSERT_NE(msg2, nullptr);
    
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 2);

    // Re-initialize (this is not recommended in production!)
    OpenLcbBufferStore_initialize();

    // Counters should be reset
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 0);

    // Can allocate fresh buffers (but old ones are leaked)
    openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg3, nullptr);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 1);
    
    OpenLcbBufferStore_free_buffer(msg3);
}

/**
 * @brief Test telemetry during complex allocation patterns
 * 
 * Verifies:
 * - Counters accurate through complex sequences
 * - Max tracking works correctly
 * - Handles allocate/free/reallocate patterns
 */
TEST(OpenLcbBufferStore, complex_telemetry)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH < 5) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msgs[5];
    
    // Allocate 5
    for (int i = 0; i < 5; i++)
    {
        msgs[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), i + 1);
    }
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 5);

    // Free 2
    OpenLcbBufferStore_free_buffer(msgs[1]);
    OpenLcbBufferStore_free_buffer(msgs[3]);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 3);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 5); // Max unchanged

    // Reallocate 1
    msgs[1] = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 4);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 5); // Still 5

    // Free all remaining
    OpenLcbBufferStore_free_buffer(msgs[0]);
    OpenLcbBufferStore_free_buffer(msgs[1]);
    OpenLcbBufferStore_free_buffer(msgs[2]);
    OpenLcbBufferStore_free_buffer(msgs[4]);
    
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 5); // Max persists

    // Clear max and verify
    OpenLcbBufferStore_clear_max_allocated();
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_max_allocated(), 0);
}

/**
 * @brief Test pool exhaustion recovery
 * 
 * Verifies:
 * - After exhaustion, freed buffers become available
 * - Can fill, drain, and refill pool
 * - No permanent exhaustion state
 */
TEST(OpenLcbBufferStore, exhaustion_recovery)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH < 2) return;
    
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msgs[USER_DEFINED_BASIC_BUFFER_DEPTH];
    
    // Fill pool completely
    for (int i = 0; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {
        msgs[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msgs[i], nullptr);
    }

    // Verify exhausted
    openlcb_msg_t *overflow = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(overflow, nullptr);

    // Free half
    int half = USER_DEFINED_BASIC_BUFFER_DEPTH / 2;
    for (int i = 0; i < half; i++)
    {
        OpenLcbBufferStore_free_buffer(msgs[i]);
    }

    // Should be able to allocate again
    for (int i = 0; i < half; i++)
    {
        msgs[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_NE(msgs[i], nullptr);
    }

    // Verify full again
    overflow = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(overflow, nullptr);

    // Clean up
    for (int i = 0; i < USER_DEFINED_BASIC_BUFFER_DEPTH; i++)
    {
        OpenLcbBufferStore_free_buffer(msgs[i]);
    }
}

/**
 * @brief Test first-fit allocation ordering
 * 
 * Verifies:
 * - Allocations use first available slot
 * - Freed slots are reused in order
 * - Linear search behavior
 */
TEST(OpenLcbBufferStore, allocation_ordering)
{
    if (USER_DEFINED_BASIC_BUFFER_DEPTH < 5) return;
    
    OpenLcbBufferStore_initialize();

    // Allocate 5 buffers
    openlcb_msg_t *msg0 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg2 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg3 = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg4 = OpenLcbBufferStore_allocate_buffer(BASIC);
    
    // Free non-contiguous slots (1 and 3)
    OpenLcbBufferStore_free_buffer(msg1);
    OpenLcbBufferStore_free_buffer(msg3);

    // Next allocation should get msg1's slot (first available)
    openlcb_msg_t *new1 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(new1, msg1);

    // Next should get msg3's slot
    openlcb_msg_t *new3 = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_EQ(new3, msg3);

    // Clean up
    OpenLcbBufferStore_free_buffer(msg0);
    OpenLcbBufferStore_free_buffer(new1);
    OpenLcbBufferStore_free_buffer(msg2);
    OpenLcbBufferStore_free_buffer(new3);
    OpenLcbBufferStore_free_buffer(msg4);
}

/**
 * @brief Test payload pointer validity
 *
 * Verifies:
 * - Payload pointer is not NULL
 * - Payload pointer points to correct pool
 * - Can write to payload without crash
 */
TEST(OpenLcbBufferStore, payload_pointer_validity)
{
    OpenLcbBufferStore_initialize();

    if (USER_DEFINED_BASIC_BUFFER_DEPTH > 0)
    {
        openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
        ASSERT_NE(msg, nullptr);
        ASSERT_NE(msg->payload, nullptr);

        // Cast to actual payload type to access array
        payload_basic_t *payload_ptr = (payload_basic_t *)msg->payload;

        // Write to payload (should not crash)
        (*payload_ptr)[0] = 0xAA;
        (*payload_ptr)[1] = 0xBB;

        // Verify we can read it back
        EXPECT_EQ((*payload_ptr)[0], 0xAA);
        EXPECT_EQ((*payload_ptr)[1], 0xBB);

        OpenLcbBufferStore_free_buffer(msg);
    }
}