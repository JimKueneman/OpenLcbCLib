/*******************************************************************************
 * File: alias_mappings_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for the AliasMappings module.
 *   Tests alias registration, lookup, validation, and buffer management.
 *
 * Test Coverage:
 *   - Initialization and reset
 *   - Alias registration and unregistration
 *   - Lookup by alias and node ID
 *   - Boundary value validation
 *   - Duplicate alias handling
 *   - Buffer overflow conditions
 *   - Flag management (duplicate alias detection)
 *   - Flush operations
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_types.h"
#include "alias_mappings.h"
#include "../../openlcb/openlcb_types.h"

/*******************************************************************************
 * Test Constants
 ******************************************************************************/
#define NODE_ID 0x010203040506
#define NODE_ALIAS 0x0666

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Initialize the alias mappings module for each test.
 * Called at the beginning of each test to ensure clean state.
 */
void setup_test(void)
{
    AliasMappings_initialize();
}

/**
 * Fill the entire mapping table with sequential test data.
 * Used to test buffer full conditions and exhaustive searches.
 */
void fill_mapping_table(void)
{
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
    }
}

/**
 * Verify that a range of mappings exist in the table.
 * 
 * @param count Number of sequential mappings to verify
 * @return true if all mappings exist, false otherwise
 */
bool verify_mappings_exist(int count)
{
    for (int i = 0; i < count; i++)
    {
        if (AliasMappings_find_mapping_by_alias(NODE_ALIAS + i) == nullptr)
            return false;
        if (AliasMappings_find_mapping_by_node_id(NODE_ID + i) == nullptr)
            return false;
    }
    return true;
}

/*******************************************************************************
 * Basic Functionality Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies that the module can be initialized without errors.
 */
TEST(AliasMapping, initialize)
{
    setup_test();
    // Test passes if no crash occurs
}

/**
 * Test: Get alias mapping info pointer
 * Verifies that the info structure pointer is valid.
 */
TEST(AliasMapping, get_alias_mapping_info_returns_valid_pointer)
{
    setup_test();
    
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    
    EXPECT_NE(info, nullptr);
}

/**
 * Test: Set duplicate alias flag
 * Verifies that the global duplicate alias flag can be set.
 */
TEST(AliasMapping, set_has_duplicate_alias_flag_sets_flag)
{
    setup_test();
    
    AliasMappings_set_has_duplicate_alias_flag();
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    
    EXPECT_TRUE(info->has_duplicate_alias);
}

/**
 * Test: Clear duplicate alias flag
 * Verifies that the global duplicate alias flag can be cleared.
 */
TEST(AliasMapping, clear_has_duplicate_alias_flag_clears_flag)
{
    setup_test();
    
    // Set the flag first
    AliasMappings_set_has_duplicate_alias_flag();
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    EXPECT_TRUE(info->has_duplicate_alias);
    
    // Clear the flag
    AliasMappings_clear_has_duplicate_alias_flag();
    EXPECT_FALSE(info->has_duplicate_alias);
}

/**
 * Test: Initialize clears duplicate flag
 * Verifies that initialization clears the duplicate alias flag.
 */
TEST(AliasMapping, initialize_clears_has_duplicate_alias_flag)
{
    setup_test();
    
    // Set the flag
    AliasMappings_set_has_duplicate_alias_flag();
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    EXPECT_TRUE(info->has_duplicate_alias);
    
    // Re-initialize should clear it
    AliasMappings_initialize();
    EXPECT_FALSE(info->has_duplicate_alias);
}

/*******************************************************************************
 * Registration Tests
 ******************************************************************************/

/**
 * Test: Register fills table to capacity
 * Verifies that mappings can be registered up to buffer capacity,
 * and that overflow attempts are rejected.
 */
TEST(AliasMapping, register_fills_table_to_capacity)
{
    setup_test();
    
    // Fill the table
    fill_mapping_table();
    
    // Verify all mappings exist
    EXPECT_TRUE(verify_mappings_exist(ALIAS_MAPPING_BUFFER_DEPTH));
    
    // Table is full, next registration should fail
    alias_mapping_t *overflow = AliasMappings_register(NODE_ALIAS - 1, NODE_ID - 1);
    EXPECT_EQ(overflow, nullptr);
}

/**
 * Test: Register initializes mapping fields correctly
 * Verifies that newly registered mappings have correct initial values.
 */
TEST(AliasMapping, register_initializes_mapping_fields_correctly)
{
    setup_test();
    
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);
    
    EXPECT_NE(mapping, nullptr);
    EXPECT_EQ(mapping->alias, NODE_ALIAS);
    EXPECT_EQ(mapping->node_id, NODE_ID);
    EXPECT_FALSE(mapping->is_duplicate);
    EXPECT_FALSE(mapping->is_permitted);
}

/**
 * Test: Register updates alias for existing node ID
 * Verifies that registering a new alias for an existing node ID
 * updates the mapping rather than creating a duplicate.
 */
TEST(AliasMapping, register_updates_alias_for_existing_node_id)
{
    setup_test();
    
    // Register with first alias
    AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS), nullptr);
    
    // Register same Node ID with different alias (should update, not add new)
    AliasMappings_register(NODE_ALIAS + 1, NODE_ID);
    
    // Old alias should be gone
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS), nullptr);
    
    // New alias should exist
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS + 1), nullptr);
}

/**
 * Test: Register update preserves structure pointer
 * Verifies that updating an alias uses the same buffer slot.
 */
TEST(AliasMapping, register_update_preserves_structure_pointer)
{
    setup_test();
    
    alias_mapping_t *first_mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(first_mapping, nullptr);
    
    // Register same node_id with different alias (should update same slot)
    alias_mapping_t *second_mapping = AliasMappings_register(NODE_ALIAS + 1, NODE_ID);
    
    // Should return the same structure pointer (same slot in array)
    EXPECT_EQ(first_mapping, second_mapping);
    
    // But with updated alias
    EXPECT_EQ(second_mapping->alias, NODE_ALIAS + 1);
    EXPECT_EQ(second_mapping->node_id, NODE_ID);
}

/*******************************************************************************
 * Unregistration Tests
 ******************************************************************************/

/**
 * Test: Unregister removes all mappings
 * Verifies that unregister correctly removes mappings from the table.
 */
TEST(AliasMapping, unregister_removes_all_mappings)
{
    setup_test();
    fill_mapping_table();
    
    // Unregister all mappings
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        AliasMappings_unregister(NODE_ALIAS + i);
    }
    
    // Verify all mappings are gone
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS + i), nullptr);
    }
}

/**
 * Test: Unregister clears all mapping fields
 * Verifies that unregister clears all fields including flags.
 */
TEST(AliasMapping, unregister_clears_all_mapping_fields)
{
    setup_test();
    
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(mapping, nullptr);
    
    // Manually set flags to verify they get cleared
    mapping->is_duplicate = true;
    mapping->is_permitted = true;
    
    AliasMappings_unregister(NODE_ALIAS);
    
    // After unregister, the mapping should be completely cleared
    EXPECT_EQ(mapping->alias, 0);
    EXPECT_EQ(mapping->node_id, 0);
    EXPECT_FALSE(mapping->is_duplicate);
    EXPECT_FALSE(mapping->is_permitted);
}

/**
 * Test: Unregister ignores nonexistent alias
 * Verifies that unregistering a non-existent alias doesn't cause errors.
 */
TEST(AliasMapping, unregister_ignores_nonexistent_alias)
{
    setup_test();
    
    AliasMappings_register(NODE_ALIAS, NODE_ID);
    
    // Unregistering non-existent alias should not crash
    AliasMappings_unregister(NODE_ALIAS + 1);
    
    // Original mapping should still exist
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS), nullptr);
}

/**
 * Test: Unregister finds and removes middle slot
 * Verifies that unregister can find and remove entries in the middle of the buffer.
 */
TEST(AliasMapping, unregister_finds_and_removes_middle_slot)
{
    setup_test();
    
    // Register several mappings
    for (int i = 0; i < 5; i++)
    {
        AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
    }
    
    // Get pointer to middle entry before unregister
    alias_mapping_t *middle = AliasMappings_find_mapping_by_alias(NODE_ALIAS + 2);
    EXPECT_NE(middle, nullptr);
    
    // Unregister the middle one - exercises loop finding it
    AliasMappings_unregister(NODE_ALIAS + 2);
    
    // Verify it's gone
    EXPECT_EQ(middle->alias, 0);
    EXPECT_EQ(middle->node_id, 0);
    
    // Verify others still exist
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS + 1), nullptr);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS + 3), nullptr);
}

/*******************************************************************************
 * Lookup Tests - Find by Alias
 ******************************************************************************/

/**
 * Test: Find by alias finds first slot
 * Verifies that lookup can find a mapping in the first buffer slot.
 */
TEST(AliasMapping, find_by_alias_finds_first_slot)
{
    setup_test();
    
    // Register a mapping in the first slot
    alias_mapping_t *registered = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(registered, nullptr);
    
    // Search for it - this will cause the loop to find it in slot 0
    alias_mapping_t *found = AliasMappings_find_mapping_by_alias(NODE_ALIAS);
    
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found, registered);  // Should be the same pointer
    EXPECT_EQ(found->alias, NODE_ALIAS);
    EXPECT_EQ(found->node_id, NODE_ID);
}

/**
 * Test: Find by alias finds middle slot
 * Verifies that lookup can find a mapping in the middle of the buffer.
 */
TEST(AliasMapping, find_by_alias_finds_middle_slot)
{
    setup_test();
    
    // Fill first few slots
    for (int i = 0; i < 5; i++)
    {
        AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
    }
    
    // Search for one in the middle - this exercises the loop continuing
    alias_mapping_t *found = AliasMappings_find_mapping_by_alias(NODE_ALIAS + 3);
    
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->alias, NODE_ALIAS + 3);
    EXPECT_EQ(found->node_id, NODE_ID + 3);
}

/**
 * Test: Find by alias finds last slot
 * Verifies that lookup can find a mapping in the last buffer slot.
 */
TEST(AliasMapping, find_by_alias_finds_last_slot)
{
    setup_test();
    
    // Register entries one by one and verify each one
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        alias_mapping_t *registered = AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
        EXPECT_NE(registered, nullptr) << "Failed to register at index " << i;
    }
    
    // Verify the table is actually full by checking we can't add more
    alias_mapping_t *overflow = AliasMappings_register(NODE_ALIAS - 1, NODE_ID - 1);
    EXPECT_EQ(overflow, nullptr) << "Table should be full";
    
    // Now search for each entry to verify they all exist
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        alias_mapping_t *found = AliasMappings_find_mapping_by_alias(NODE_ALIAS + i);
        EXPECT_NE(found, nullptr) << "Failed to find alias at index " << i 
                                   << " (alias=0x" << std::hex << (NODE_ALIAS + i) << ")";
        if (found) {
            EXPECT_EQ(found->alias, NODE_ALIAS + i) << "Wrong alias at index " << i;
            EXPECT_EQ(found->node_id, NODE_ID + i) << "Wrong node_id at index " << i;
        }
    }
}

/**
 * Test: Find by alias returns NULL for nonexistent mappings
 * Verifies that searching for a non-existent alias returns NULL.
 */
TEST(AliasMapping, find_returns_null_for_nonexistent_mappings)
{
    setup_test();
    
    AliasMappings_register(NODE_ALIAS, NODE_ID);
    
    // Search for different alias and node_id should return NULL
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS + 1), nullptr);
    EXPECT_EQ(AliasMappings_find_mapping_by_node_id(NODE_ID + 1), nullptr);
}

/*******************************************************************************
 * Lookup Tests - Find by Node ID
 ******************************************************************************/

/**
 * Test: Find by node ID finds first slot
 * Verifies that lookup by node ID can find a mapping in the first buffer slot.
 */
TEST(AliasMapping, find_by_node_id_finds_first_slot)
{
    setup_test();
    
    alias_mapping_t *registered = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(registered, nullptr);
    
    alias_mapping_t *found = AliasMappings_find_mapping_by_node_id(NODE_ID);
    
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found, registered);
    EXPECT_EQ(found->node_id, NODE_ID);
}

/**
 * Test: Find by node ID finds middle slot
 * Verifies that lookup by node ID can find a mapping in the middle of the buffer.
 */
TEST(AliasMapping, find_by_node_id_finds_middle_slot)
{
    setup_test();
    
    for (int i = 0; i < 5; i++)
    {
        AliasMappings_register(NODE_ALIAS + i, NODE_ID + i);
    }
    
    alias_mapping_t *found = AliasMappings_find_mapping_by_node_id(NODE_ID + 3);
    
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->node_id, NODE_ID + 3);
    EXPECT_EQ(found->alias, NODE_ALIAS + 3);
}

/*******************************************************************************
 * Boundary Value Tests - Alias Validation
 ******************************************************************************/

/**
 * Test: Register rejects zero alias
 * Per OpenLCB spec, alias must be 1-0xFFF (12-bit, non-zero).
 */
TEST(AliasMapping, register_rejects_zero_alias)
{
    setup_test();
    
    alias_mapping_t *mapping = AliasMappings_register(0, NODE_ID);
    
    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Register rejects alias above maximum
 * Alias must fit in 12 bits (0x001-0xFFF).
 */
TEST(AliasMapping, register_rejects_alias_above_max)
{
    setup_test();
    
    // 0xFFF is max valid 12-bit alias, 0x1000 is over
    alias_mapping_t *mapping = AliasMappings_register(0x1000, NODE_ID);
    
    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Register accepts maximum valid alias
 * Verifies that the maximum valid 12-bit alias (0xFFF) is accepted.
 */
TEST(AliasMapping, register_accepts_max_valid_alias)
{
    setup_test();
    
    // 0xFFF is the maximum valid 12-bit alias
    alias_mapping_t *mapping = AliasMappings_register(0xFFF, NODE_ID);
    
    EXPECT_NE(mapping, nullptr);
}

/**
 * Test: Find by alias rejects zero alias
 * Verifies input validation for zero alias.
 */
TEST(AliasMapping, find_by_alias_rejects_zero_alias)
{
    setup_test();

    alias_mapping_t *mapping = AliasMappings_find_mapping_by_alias(0);

    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Find by alias rejects alias above maximum
 * Verifies input validation for out-of-range alias values.
 */
TEST(AliasMapping, find_by_alias_rejects_alias_above_max)
{
    setup_test();

    alias_mapping_t *mapping = AliasMappings_find_mapping_by_alias(0x1000);

    EXPECT_EQ(mapping, nullptr);
}

/*******************************************************************************
 * Boundary Value Tests - Node ID Validation
 ******************************************************************************/

/**
 * Test: Register rejects zero node ID
 * Per OpenLCB spec, node ID must be non-zero.
 */
TEST(AliasMapping, register_rejects_zero_node_id)
{
    setup_test();
    
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, 0);
    
    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Register rejects node ID above maximum
 * Node ID must fit in 48 bits (0x000000000001-0xFFFFFFFFFFFF).
 */
TEST(AliasMapping, register_rejects_node_id_above_max)
{
    setup_test();
    
    // Max is 0xFFFFFFFFFFFF (48 bits), test with higher value
    node_id_t invalid_node_id = 0x1000000000000ULL;
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, invalid_node_id);
    
    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Register accepts maximum valid node ID
 * Verifies that the maximum valid 48-bit node ID is accepted.
 */
TEST(AliasMapping, register_accepts_max_valid_node_id)
{
    setup_test();
    
    // 0xFFFFFFFFFFFF is the maximum valid 48-bit Node ID
    node_id_t max_node_id = 0xFFFFFFFFFFFFULL;
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, max_node_id);
    
    EXPECT_NE(mapping, nullptr);
    EXPECT_EQ(mapping->node_id, max_node_id);
    EXPECT_EQ(mapping->alias, NODE_ALIAS);
}

/**
 * Test: Find by node ID rejects zero node ID
 * Verifies input validation for zero node ID.
 */
TEST(AliasMapping, find_by_node_id_rejects_zero_node_id)
{
    setup_test();

    alias_mapping_t *mapping = AliasMappings_find_mapping_by_node_id(0);

    EXPECT_EQ(mapping, nullptr);
}

/**
 * Test: Find by node ID rejects node ID above maximum
 * Verifies input validation for out-of-range node ID values.
 */
TEST(AliasMapping, find_by_node_id_rejects_node_id_above_max)
{
    setup_test();

    node_id_t invalid_node_id = 0x1000000000000ULL;
    alias_mapping_t *mapping = AliasMappings_find_mapping_by_node_id(invalid_node_id);

    EXPECT_EQ(mapping, nullptr);
}

/*******************************************************************************
 * Flush Tests
 ******************************************************************************/

/**
 * Test: Flush clears all mappings
 * Verifies that flush removes all registered mappings.
 */
TEST(AliasMapping, flush_clears_all_mappings)
{
    setup_test();
    fill_mapping_table();
    
    // Verify mappings exist
    EXPECT_TRUE(verify_mappings_exist(ALIAS_MAPPING_BUFFER_DEPTH));
    
    // Flush all mappings
    AliasMappings_flush();
    
    // Verify all mappings are gone
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS + i), nullptr);
        EXPECT_EQ(AliasMappings_find_mapping_by_node_id(NODE_ID + i), nullptr);
    }
    
    // Re-initialize to ensure clean state for next test
    AliasMappings_initialize();
}

/**
 * Test: Flush clears duplicate alias flag
 * Verifies that flush clears the global duplicate alias flag.
 */
TEST(AliasMapping, flush_clears_has_duplicate_alias_flag)
{
    setup_test();
    
    AliasMappings_set_has_duplicate_alias_flag();
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    EXPECT_TRUE(info->has_duplicate_alias);
    
    AliasMappings_flush();
    EXPECT_FALSE(info->has_duplicate_alias);
    
    // Re-initialize to ensure clean state for next test
    AliasMappings_initialize();
}

/**
 * Test: Flush clears all mapping fields
 * Verifies that flush clears all fields in all mapping entries.
 */
TEST(AliasMapping, flush_clears_all_mapping_fields)
{
    setup_test();
    
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(mapping, nullptr);
    
    // Set flags
    mapping->is_duplicate = true;
    mapping->is_permitted = true;
    
    AliasMappings_flush();
    
    // Verify all fields cleared
    alias_mapping_info_t *info = AliasMappings_get_alias_mapping_info();
    for (int i = 0; i < ALIAS_MAPPING_BUFFER_DEPTH; i++)
    {
        EXPECT_EQ(info->list[i].alias, 0);
        EXPECT_EQ(info->list[i].node_id, 0);
        EXPECT_FALSE(info->list[i].is_duplicate);
        EXPECT_FALSE(info->list[i].is_permitted);
    }
    
    // Re-initialize to ensure clean state for next test
    AliasMappings_initialize();
}

/*******************************************************************************
 * Edge Case Tests
 ******************************************************************************/

/**
 * Test: Multiple initializations work correctly
 * Verifies that the module can be re-initialized multiple times.
 */
TEST(AliasMapping, multiple_initializations_work_correctly)
{
    setup_test();
    
    AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS), nullptr);
    
    // Re-initialize should clear everything
    AliasMappings_initialize();
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS), nullptr);
    
    // Should be able to register again
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS, NODE_ID);
    EXPECT_NE(mapping, nullptr);
}

/*******************************************************************************
 * End of Test Suite
 ******************************************************************************/
