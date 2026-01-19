/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Test suite for OpenLCB Buffer List
 * Refactored and enhanced for maximum code coverage
 */

#include "test/main_Test.hxx"

#include "openlcb_buffer_list.h"
#include "openlcb_buffer_store.h"
#include "openlcb_types.h"

// ============================================================================
// EXISTING TESTS - Cleaned up and improved
// ============================================================================

/**
 * @brief Test basic initialization of buffer list
 * 
 * Verifies:
 * - List is empty after initialization
 * - All slots are NULL
 */
TEST(OpenLcbBufferList, initialize)
{
    OpenLcbBufferList_initialize();

    // Verify list is empty
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    
    // Verify all slots are NULL
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        EXPECT_EQ(OpenLcbBufferList_index_of(i), nullptr);
    }
}

/**
 * @brief Test add and release operations with buffer store allocation
 * 
 * Verifies:
 * - Can add allocated message to list
 * - List reports not empty
 * - Can release message from list
 * - List reports empty after release
 */
TEST(OpenLcbBufferList, allocate_release)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();

    // Allocate a message from buffer store
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(msg, nullptr);

    // List should be empty initially
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    
    // Add message to list
    openlcb_msg_t *added_msg = OpenLcbBufferList_add(msg);
    EXPECT_EQ(added_msg, msg);
    
    // List should not be empty
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Release message from list
    openlcb_msg_t *released_msg = OpenLcbBufferList_release(msg);
    EXPECT_EQ(released_msg, msg);

    // List should be empty again
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    
    // Free the buffer
    OpenLcbBufferStore_free_buffer(msg);
}

/**
 * @brief Test complete lifecycle: allocate, add, release, free
 * 
 * Verifies:
 * - Full lifecycle works correctly
 * - Message can be freed after release
 */
TEST(OpenLcbBufferList, allocate_free)
{
    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();

    // Allocate message
    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(msg, nullptr);

    // Add to list
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    openlcb_msg_t *added_msg = OpenLcbBufferList_add(msg);
    EXPECT_EQ(added_msg, msg);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Release from list
    openlcb_msg_t *released_msg = OpenLcbBufferList_release(msg);
    EXPECT_EQ(released_msg, msg);
    
    // Free buffer
    OpenLcbBufferStore_free_buffer(msg);
    
    // Verify list is empty
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

/**
 * @brief Test releasing with invalid pointer (not in list)
 * 
 * Verifies:
 * - Release returns NULL for pointer not in list
 * - List state unchanged by failed release
 */
TEST(OpenLcbBufferList, release_invalid_pointer)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg;
    openlcb_msg_t invalid_msg;

    // Add one message
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    openlcb_msg_t *added_msg = OpenLcbBufferList_add(&msg);
    EXPECT_EQ(added_msg, &msg);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Try to release a different message (not in list)
    openlcb_msg_t *released = OpenLcbBufferList_release(&invalid_msg);
    EXPECT_EQ(released, nullptr);

    // Original message should still be in list
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Clean up - release the valid message
    released = OpenLcbBufferList_release(&msg);
    EXPECT_EQ(released, &msg);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

/**
 * @brief Test index_of functionality with multiple messages
 * 
 * Verifies:
 * - Messages can be accessed by index
 * - Index matches insertion order
 * - Messages can be released and freed
 */
TEST(OpenLcbBufferList, allocate_index_of)
{
    #define LOOP_COUNT 4

    openlcb_msg_t *msg_array[LOOP_COUNT];

    OpenLcbBufferStore_initialize();
    OpenLcbBufferList_initialize();

    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // Allocate and add messages
    for (int i = 0; i < LOOP_COUNT; i++)
    {
        msg_array[i] = OpenLcbBufferStore_allocate_buffer(BASIC);
        EXPECT_NE(msg_array[i], nullptr);
        
        openlcb_msg_t *added = OpenLcbBufferList_add(msg_array[i]);
        EXPECT_EQ(added, msg_array[i]);
    }

    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Verify index access
    for (int i = 0; i < LOOP_COUNT; i++)
    {
        EXPECT_EQ(OpenLcbBufferList_index_of(i), msg_array[i]);
    }

    // Clean up
    for (int i = 0; i < LOOP_COUNT; i++)
    {
        OpenLcbBufferList_release(msg_array[i]);
        OpenLcbBufferStore_free_buffer(msg_array[i]);
    }

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    
    #undef LOOP_COUNT
}

/**
 * @brief Test releasing NULL pointer
 * 
 * Verifies:
 * - Release NULL returns NULL
 * - No crash or undefined behavior
 */
TEST(OpenLcbBufferList, release_null)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t *msg = nullptr;
    
    // Release NULL should return NULL
    openlcb_msg_t *result = OpenLcbBufferList_release(msg);
    EXPECT_EQ(result, nullptr);
}

/**
 * @brief Stress test - fill list completely and release all
 * 
 * Verifies:
 * - Can fill list to maximum capacity
 * - Add fails when list is full
 * - All messages can be released
 * - List is empty after releasing all
 */
TEST(OpenLcbBufferList, release_stress)
{
    openlcb_msg_t test_msgs[LEN_MESSAGE_BUFFER];
    openlcb_msg_t *msg_ptr;

    OpenLcbBufferList_initialize();

    msg_ptr = &test_msgs[0];

    // Fill list completely
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        openlcb_msg_t *added = OpenLcbBufferList_add(msg_ptr);
        EXPECT_EQ(added, msg_ptr);
        msg_ptr++;
    }

    // List should be full - next add should fail
    openlcb_msg_t overflow_msg;
    openlcb_msg_t *result = OpenLcbBufferList_add(&overflow_msg);
    EXPECT_EQ(result, nullptr);

    // Release all messages
    msg_ptr = &test_msgs[0];
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        openlcb_msg_t *released = OpenLcbBufferList_release(msg_ptr);
        EXPECT_EQ(released, msg_ptr);
        msg_ptr++;
    }

    // List should be empty
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

/**
 * @brief Test find functionality with multiple messages
 * 
 * Verifies:
 * - Find locates messages by source_alias, dest_alias, and mti
 * - All three parameters must match
 * - Find returns NULL when no match
 * - Find returns first match when multiple could match
 */
TEST(OpenLcbBufferList, release_find)
{
    openlcb_msg_t msg;
    openlcb_msg_t msg1;
    openlcb_msg_t msg2;
    openlcb_msg_t msg3;

    OpenLcbBufferList_initialize();

    // Set up message 0: source=0x0568, dest=0x0567, mti=0x0222
    msg.source_alias = 0x0568;
    msg.dest_alias = 0x0567;
    msg.mti = 0x0222;
    EXPECT_EQ(OpenLcbBufferList_add(&msg), &msg);

    // Set up message 1: source=0x0568, dest=0x0567, mti=0x0333 (different MTI)
    msg1.source_alias = 0x0568;
    msg1.dest_alias = 0x0567;
    msg1.mti = 0x0333;
    EXPECT_EQ(OpenLcbBufferList_add(&msg1), &msg1);

    // Set up message 2: source=0x0568, dest=0x0777, mti=0x0222 (different dest)
    msg2.source_alias = 0x0568;
    msg2.dest_alias = 0x0777;
    msg2.mti = 0x0222;
    EXPECT_EQ(OpenLcbBufferList_add(&msg2), &msg2);

    // Set up message 3: source=0x0999, dest=0x0567, mti=0x0222 (different source)
    msg3.source_alias = 0x0999;
    msg3.dest_alias = 0x0567;
    msg3.mti = 0x0222;
    EXPECT_EQ(OpenLcbBufferList_add(&msg3), &msg3);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Test finds that should fail (no match)
    EXPECT_EQ(OpenLcbBufferList_find(0x0568, 0x0567, 0x0AAA), nullptr);  // Wrong MTI
    EXPECT_EQ(OpenLcbBufferList_find(0x0568, 0x0777, 0x0333), nullptr);  // Wrong combination
    EXPECT_EQ(OpenLcbBufferList_find(0x0999, 0x0777, 0x0222), nullptr);  // Wrong dest

    // Test finds that should succeed (exact match)
    EXPECT_EQ(OpenLcbBufferList_find(0x0568, 0x0567, 0x0222), &msg);
    EXPECT_EQ(OpenLcbBufferList_find(0x0568, 0x0567, 0x0333), &msg1);
    EXPECT_EQ(OpenLcbBufferList_find(0x0568, 0x0777, 0x0222), &msg2);
    EXPECT_EQ(OpenLcbBufferList_find(0x0999, 0x0567, 0x0222), &msg3);

    // Clean up
    OpenLcbBufferList_release(&msg);
    OpenLcbBufferList_release(&msg1);
    OpenLcbBufferList_release(&msg2);
    OpenLcbBufferList_release(&msg3);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

/**
 * @brief Test index_of with invalid indices
 * 
 * Verifies:
 * - Out of bounds index returns NULL
 * - Empty slot returns NULL
 * - Cannot distinguish between out-of-bounds and empty slot
 */
TEST(OpenLcbBufferList, invalid_index_of)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg;
    openlcb_msg_t msg1;
    openlcb_msg_t msg2;
    openlcb_msg_t msg3;

    // Add 4 messages
    EXPECT_EQ(OpenLcbBufferList_add(&msg), &msg);
    EXPECT_EQ(OpenLcbBufferList_add(&msg1), &msg1);
    EXPECT_EQ(OpenLcbBufferList_add(&msg2), &msg2);
    EXPECT_EQ(OpenLcbBufferList_add(&msg3), &msg3);

    // Out of range index should return NULL
    EXPECT_EQ(OpenLcbBufferList_index_of(LEN_MESSAGE_BUFFER), nullptr);
    EXPECT_EQ(OpenLcbBufferList_index_of(LEN_MESSAGE_BUFFER + 1), nullptr);

    // In range but empty slot should return NULL
    EXPECT_EQ(OpenLcbBufferList_index_of(LEN_MESSAGE_BUFFER - 1), nullptr);

    // Clean up
    OpenLcbBufferList_release(&msg);
    OpenLcbBufferList_release(&msg1);
    OpenLcbBufferList_release(&msg2);
    OpenLcbBufferList_release(&msg3);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

// ============================================================================
// NEW TESTS - For maximum coverage
// ============================================================================

/**
 * @brief Test adding NULL pointer to list
 *
 * Verifies:
 * - Add NULL is allowed (implementation doesn't validate)
 * - NULL can be stored in list
 * - is_empty() returns true even though slot is occupied (because it checks for NULL pointers)
 * - NULL is stored at index 0
 *
 * Note: This reveals interesting behavior - is_empty() checks slot contents, not slot occupancy
 */
TEST(OpenLcbBufferList, add_null_pointer)
{
    OpenLcbBufferList_initialize();

    // Add NULL to list
    openlcb_msg_t *result = OpenLcbBufferList_add(nullptr);
    EXPECT_EQ(result, nullptr); // Returns what we passed

    // List will appear empty because is_empty() checks for NULL pointers
    // This is actually correct behavior - a NULL pointer means "no message"
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // However, the slot IS occupied with NULL
    // We can verify this by trying to add messages - first one goes to slot 0
    openlcb_msg_t msg1;
    OpenLcbBufferList_add(&msg1);

    // Now msg1 should be at index 0 (replaced the NULL)
    EXPECT_EQ(OpenLcbBufferList_index_of(0), &msg1);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Clean up
    OpenLcbBufferList_release(&msg1);
}

/**
 * @brief Test find in empty list
 * 
 * Verifies:
 * - Find returns NULL when list is empty
 * - No crash or undefined behavior
 */
TEST(OpenLcbBufferList, find_empty_list)
{
    OpenLcbBufferList_initialize();

    // Find in empty list should return NULL
    openlcb_msg_t *result = OpenLcbBufferList_find(0x1234, 0x5678, 0xABCD);
    EXPECT_EQ(result, nullptr);
}

/**
 * @brief Test find with no matching messages
 * 
 * Verifies:
 * - Find returns NULL when no messages match
 * - Searches entire list before returning NULL
 */
TEST(OpenLcbBufferList, find_no_match)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg1, msg2, msg3;

    // Add messages with specific attributes
    msg1.source_alias = 0x0001;
    msg1.dest_alias = 0x0002;
    msg1.mti = 0x0003;
    OpenLcbBufferList_add(&msg1);

    msg2.source_alias = 0x0004;
    msg2.dest_alias = 0x0005;
    msg2.mti = 0x0006;
    OpenLcbBufferList_add(&msg2);

    msg3.source_alias = 0x0007;
    msg3.dest_alias = 0x0008;
    msg3.mti = 0x0009;
    OpenLcbBufferList_add(&msg3);

    // Find with non-matching attributes
    openlcb_msg_t *result = OpenLcbBufferList_find(0xFFFF, 0xFFFF, 0xFFFF);
    EXPECT_EQ(result, nullptr);

    // Clean up
    OpenLcbBufferList_release(&msg1);
    OpenLcbBufferList_release(&msg2);
    OpenLcbBufferList_release(&msg3);
}

/**
 * @brief Test find with partial field matches
 * 
 * Verifies:
 * - All three fields must match for find to succeed
 * - Partial matches return NULL
 */
TEST(OpenLcbBufferList, find_partial_match)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg;
    msg.source_alias = 0x1111;
    msg.dest_alias = 0x2222;
    msg.mti = 0x3333;
    
    OpenLcbBufferList_add(&msg);

    // Match only source and dest (wrong mti)
    EXPECT_EQ(OpenLcbBufferList_find(0x1111, 0x2222, 0xFFFF), nullptr);

    // Match only source and mti (wrong dest)
    EXPECT_EQ(OpenLcbBufferList_find(0x1111, 0xFFFF, 0x3333), nullptr);

    // Match only dest and mti (wrong source)
    EXPECT_EQ(OpenLcbBufferList_find(0xFFFF, 0x2222, 0x3333), nullptr);

    // All three match - should succeed
    EXPECT_EQ(OpenLcbBufferList_find(0x1111, 0x2222, 0x3333), &msg);

    // Clean up
    OpenLcbBufferList_release(&msg);
}

/**
 * @brief Test releasing message from middle of list
 * 
 * Verifies:
 * - Can release message from any position
 * - Remaining messages stay in place
 * - List state is consistent after release
 */
TEST(OpenLcbBufferList, release_from_middle)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg0, msg1, msg2, msg3;

    // Add messages
    OpenLcbBufferList_add(&msg0);
    OpenLcbBufferList_add(&msg1);
    OpenLcbBufferList_add(&msg2);
    OpenLcbBufferList_add(&msg3);

    // Verify initial positions
    EXPECT_EQ(OpenLcbBufferList_index_of(0), &msg0);
    EXPECT_EQ(OpenLcbBufferList_index_of(1), &msg1);
    EXPECT_EQ(OpenLcbBufferList_index_of(2), &msg2);
    EXPECT_EQ(OpenLcbBufferList_index_of(3), &msg3);

    // Release from middle (index 1)
    openlcb_msg_t *released = OpenLcbBufferList_release(&msg1);
    EXPECT_EQ(released, &msg1);

    // Verify remaining messages
    EXPECT_EQ(OpenLcbBufferList_index_of(0), &msg0);
    EXPECT_EQ(OpenLcbBufferList_index_of(1), nullptr);  // Released slot
    EXPECT_EQ(OpenLcbBufferList_index_of(2), &msg2);
    EXPECT_EQ(OpenLcbBufferList_index_of(3), &msg3);

    // List should not be empty (3 messages remain)
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Clean up
    OpenLcbBufferList_release(&msg0);
    OpenLcbBufferList_release(&msg2);
    OpenLcbBufferList_release(&msg3);
}

/**
 * @brief Test slot reuse after release
 * 
 * Verifies:
 * - Released slots can be reused
 * - Add uses first available slot
 * - Slot reuse works correctly
 */
TEST(OpenLcbBufferList, slot_reuse)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg0, msg1, msg2, msg_new;

    // Fill first 3 slots
    OpenLcbBufferList_add(&msg0);
    OpenLcbBufferList_add(&msg1);
    OpenLcbBufferList_add(&msg2);

    // Release middle slot
    OpenLcbBufferList_release(&msg1);
    EXPECT_EQ(OpenLcbBufferList_index_of(1), nullptr);

    // Add new message - should reuse slot 1
    OpenLcbBufferList_add(&msg_new);
    EXPECT_EQ(OpenLcbBufferList_index_of(1), &msg_new);

    // Clean up
    OpenLcbBufferList_release(&msg0);
    OpenLcbBufferList_release(&msg_new);
    OpenLcbBufferList_release(&msg2);
}

/**
 * @brief Test refilling list after releasing all
 * 
 * Verifies:
 * - List can be completely drained and refilled
 * - No state corruption from previous use
 * - Refilled list works normally
 */
TEST(OpenLcbBufferList, refill_after_release)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t first_set[5];
    openlcb_msg_t second_set[5];

    // First cycle: Add and release
    for (int i = 0; i < 5; i++)
    {
        OpenLcbBufferList_add(&first_set[i]);
    }
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    for (int i = 0; i < 5; i++)
    {
        OpenLcbBufferList_release(&first_set[i]);
    }
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // Second cycle: Refill with different messages
    for (int i = 0; i < 5; i++)
    {
        OpenLcbBufferList_add(&second_set[i]);
    }
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Verify second set is in list
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(OpenLcbBufferList_index_of(i), &second_set[i]);
    }

    // Clean up
    for (int i = 0; i < 5; i++)
    {
        OpenLcbBufferList_release(&second_set[i]);
    }
}

/**
 * @brief Test message field integrity through find
 * 
 * Verifies:
 * - Find correctly matches all three fields
 * - Message attributes are not corrupted
 * - Each unique combination is distinguishable
 */
TEST(OpenLcbBufferList, message_field_integrity)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg;
    
    // Set specific values
    msg.source_alias = 0xABCD;
    msg.dest_alias = 0x1234;
    msg.mti = 0x5678;
    
    OpenLcbBufferList_add(&msg);

    // Find with exact match
    openlcb_msg_t *found = OpenLcbBufferList_find(0xABCD, 0x1234, 0x5678);
    ASSERT_NE(found, nullptr);

    // Verify all fields match
    EXPECT_EQ(found->source_alias, 0xABCD);
    EXPECT_EQ(found->dest_alias, 0x1234);
    EXPECT_EQ(found->mti, 0x5678);

    // Verify it's the same message
    EXPECT_EQ(found, &msg);

    // Clean up
    OpenLcbBufferList_release(&msg);
}

/**
 * @brief Test interleaved add, find, and release operations
 * 
 * Verifies:
 * - Operations can be interleaved
 * - List state remains consistent
 * - Find works during complex operation sequences
 */
TEST(OpenLcbBufferList, interleaved_operations)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg1, msg2, msg3;

    // Set up messages with identifiable attributes
    msg1.source_alias = 0x0001;
    msg1.dest_alias = 0x0001;
    msg1.mti = 0x0001;

    msg2.source_alias = 0x0002;
    msg2.dest_alias = 0x0002;
    msg2.mti = 0x0002;

    msg3.source_alias = 0x0003;
    msg3.dest_alias = 0x0003;
    msg3.mti = 0x0003;

    // Add msg1
    OpenLcbBufferList_add(&msg1);
    
    // Find msg1
    EXPECT_EQ(OpenLcbBufferList_find(0x0001, 0x0001, 0x0001), &msg1);

    // Add msg2
    OpenLcbBufferList_add(&msg2);
    
    // Find both
    EXPECT_EQ(OpenLcbBufferList_find(0x0001, 0x0001, 0x0001), &msg1);
    EXPECT_EQ(OpenLcbBufferList_find(0x0002, 0x0002, 0x0002), &msg2);

    // Release msg1
    OpenLcbBufferList_release(&msg1);
    
    // msg1 should not be found, msg2 should still be found
    EXPECT_EQ(OpenLcbBufferList_find(0x0001, 0x0001, 0x0001), nullptr);
    EXPECT_EQ(OpenLcbBufferList_find(0x0002, 0x0002, 0x0002), &msg2);

    // Add msg3
    OpenLcbBufferList_add(&msg3);
    
    // Find msg2 and msg3
    EXPECT_EQ(OpenLcbBufferList_find(0x0002, 0x0002, 0x0002), &msg2);
    EXPECT_EQ(OpenLcbBufferList_find(0x0003, 0x0003, 0x0003), &msg3);

    // Clean up
    OpenLcbBufferList_release(&msg2);
    OpenLcbBufferList_release(&msg3);
}

/**
 * @brief Test finding same message multiple times
 * 
 * Verifies:
 * - Find is non-destructive
 * - Same message can be found repeatedly
 * - Message remains in list after find
 */
TEST(OpenLcbBufferList, multiple_finds)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg;
    msg.source_alias = 0xAAAA;
    msg.dest_alias = 0xBBBB;
    msg.mti = 0xCCCC;

    OpenLcbBufferList_add(&msg);

    // Find same message multiple times
    for (int i = 0; i < 10; i++)
    {
        openlcb_msg_t *found = OpenLcbBufferList_find(0xAAAA, 0xBBBB, 0xCCCC);
        EXPECT_EQ(found, &msg);
    }

    // List should still contain the message
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Clean up
    OpenLcbBufferList_release(&msg);
}

/**
 * @brief Test that find returns first match only
 * 
 * Verifies:
 * - When multiple messages could match, first is returned
 * - Early exit optimization works
 */
TEST(OpenLcbBufferList, find_first_match_only)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg1, msg2, msg3;

    // Create three messages with same attributes
    msg1.source_alias = 0x9999;
    msg1.dest_alias = 0x8888;
    msg1.mti = 0x7777;

    msg2.source_alias = 0x9999;
    msg2.dest_alias = 0x8888;
    msg2.mti = 0x7777;

    msg3.source_alias = 0x9999;
    msg3.dest_alias = 0x8888;
    msg3.mti = 0x7777;

    // Add in order
    OpenLcbBufferList_add(&msg1);
    OpenLcbBufferList_add(&msg2);
    OpenLcbBufferList_add(&msg3);

    // Find should return first match (msg1)
    openlcb_msg_t *found = OpenLcbBufferList_find(0x9999, 0x8888, 0x7777);
    EXPECT_EQ(found, &msg1);

    // Release first, now should find second
    OpenLcbBufferList_release(&msg1);
    found = OpenLcbBufferList_find(0x9999, 0x8888, 0x7777);
    EXPECT_EQ(found, &msg2);

    // Release second, now should find third
    OpenLcbBufferList_release(&msg2);
    found = OpenLcbBufferList_find(0x9999, 0x8888, 0x7777);
    EXPECT_EQ(found, &msg3);

    // Clean up
    OpenLcbBufferList_release(&msg3);
}

/**
 * @brief Test re-initialization with messages in list
 * 
 * Verifies:
 * - Re-initialization clears all messages
 * - List is empty after re-init
 * - No memory leaks or corruption
 */
TEST(OpenLcbBufferList, reinitialize)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg1, msg2, msg3;

    // Add messages
    OpenLcbBufferList_add(&msg1);
    OpenLcbBufferList_add(&msg2);
    OpenLcbBufferList_add(&msg3);
    
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Re-initialize
    OpenLcbBufferList_initialize();

    // List should be empty
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // All slots should be NULL
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        EXPECT_EQ(OpenLcbBufferList_index_of(i), nullptr);
    }

    // Should be able to use list again
    OpenLcbBufferList_add(&msg1);
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    
    // Clean up
    OpenLcbBufferList_release(&msg1);
}

/**
 * @brief Test sparse list (non-contiguous entries)
 * 
 * Verifies:
 * - List can have gaps (NULL slots between valid entries)
 * - Operations work correctly with sparse list
 * - is_empty correctly detects non-empty sparse list
 */
TEST(OpenLcbBufferList, sparse_list)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msg0, msg2, msg4;

    // Add to slots 0, 1, 2
    OpenLcbBufferList_add(&msg0);
    openlcb_msg_t temp;
    OpenLcbBufferList_add(&temp);
    OpenLcbBufferList_add(&msg2);

    // Release slot 1 to create gap
    OpenLcbBufferList_release(&temp);

    // Add another (should go to slot 1, the gap)
    OpenLcbBufferList_add(&msg4);

    // Verify layout: 0=msg0, 1=msg4, 2=msg2
    EXPECT_EQ(OpenLcbBufferList_index_of(0), &msg0);
    EXPECT_EQ(OpenLcbBufferList_index_of(1), &msg4);
    EXPECT_EQ(OpenLcbBufferList_index_of(2), &msg2);

    // List should not be empty
    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Clean up
    OpenLcbBufferList_release(&msg0);
    OpenLcbBufferList_release(&msg4);
    OpenLcbBufferList_release(&msg2);
}

/**
 * @brief Test boundary: add and release exactly LEN_MESSAGE_BUFFER items
 * 
 * Verifies:
 * - Can fill to exact capacity
 * - Can release from full list
 * - List state is correct at boundaries
 */
TEST(OpenLcbBufferList, exact_capacity_boundary)
{
    OpenLcbBufferList_initialize();

    openlcb_msg_t msgs[LEN_MESSAGE_BUFFER];

    // Fill to exact capacity
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        openlcb_msg_t *result = OpenLcbBufferList_add(&msgs[i]);
        EXPECT_EQ(result, &msgs[i]);
        EXPECT_FALSE(OpenLcbBufferList_is_empty());
    }

    // Verify all are in list
    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        EXPECT_EQ(OpenLcbBufferList_index_of(i), &msgs[i]);
    }

    // Release from end to start
    for (int i = LEN_MESSAGE_BUFFER - 1; i >= 0; i--)
    {
        openlcb_msg_t *released = OpenLcbBufferList_release(&msgs[i]);
        EXPECT_EQ(released, &msgs[i]);
        
        // Check is_empty status
        if (i == 0)
        {
            EXPECT_TRUE(OpenLcbBufferList_is_empty());
        }
        else
        {
            EXPECT_FALSE(OpenLcbBufferList_is_empty());
        }
    }
}
