/*******************************************************************************
 * File: alias_mapping_listener_Test.cxx
 *
 * Description:
 *   Test suite for the AliasMappingListener module (alias_mapping_listener.h/.c).
 *   Tests listener Node ID registration, alias resolution, invalidation, and
 *   table lifecycle management.
 *
 * Test Coverage:
 *   - Initialization and reset
 *   - Registration (first-fit, duplicate, capacity)
 *   - Unregistration
 *   - Alias set from AMD
 *   - Find by Node ID
 *   - Flush all aliases (global AME)
 *   - Clear alias by alias value (AMR)
 *   - Boundary and validation checks
 *   - Verification prober (round-robin, rate-limiting, stale detection)
 *
 * Author: Jim Kueneman
 * Date: 2026-03-03
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_types.h"
#include "alias_mapping_listener.h"
#include "../../openlcb/openlcb_types.h"

/*******************************************************************************
 * Test Constants
 ******************************************************************************/

#define TEST_NODE_ID_A   0x010203040506ULL
#define TEST_NODE_ID_B   0xAABBCCDDEEFFULL
#define TEST_NODE_ID_C   0x112233445566ULL

#define TEST_ALIAS_A     0x0AAA
#define TEST_ALIAS_B     0x0BBB
#define TEST_ALIAS_C     0x0CCC

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

static void setup_test(void) {

    ListenerAliasTable_initialize();

}

/**
 * Fill the table to capacity with sequential test data.
 */
static void fill_table(void) {

    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        ListenerAliasTable_register(TEST_NODE_ID_A + i);

    }

}

/*******************************************************************************
 * Initialization Tests
 ******************************************************************************/

TEST(AliasMappingListener, initialize_clears_all_entries) {

    setup_test();

    // After init, no entry should be findable
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B), nullptr);

}

TEST(AliasMappingListener, initialize_after_populated_clears_all) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    ListenerAliasTable_initialize();

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

/*******************************************************************************
 * Registration Tests
 ******************************************************************************/

TEST(AliasMappingListener, register_returns_entry_with_alias_zero) {

    setup_test();

    listener_alias_entry_t *entry = ListenerAliasTable_register(TEST_NODE_ID_A);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0);

}

TEST(AliasMappingListener, register_multiple_entries) {

    setup_test();

    listener_alias_entry_t *a = ListenerAliasTable_register(TEST_NODE_ID_A);
    listener_alias_entry_t *b = ListenerAliasTable_register(TEST_NODE_ID_B);
    listener_alias_entry_t *c = ListenerAliasTable_register(TEST_NODE_ID_C);

    EXPECT_NE(a, nullptr);
    EXPECT_NE(b, nullptr);
    EXPECT_NE(c, nullptr);

    EXPECT_EQ(a->node_id, TEST_NODE_ID_A);
    EXPECT_EQ(b->node_id, TEST_NODE_ID_B);
    EXPECT_EQ(c->node_id, TEST_NODE_ID_C);

}

TEST(AliasMappingListener, register_duplicate_returns_existing_entry) {

    setup_test();

    listener_alias_entry_t *first = ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    listener_alias_entry_t *second = ListenerAliasTable_register(TEST_NODE_ID_A);

    // Should return the same entry, alias untouched
    EXPECT_EQ(first, second);
    EXPECT_EQ(second->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, register_rejects_zero_node_id) {

    setup_test();

    listener_alias_entry_t *entry = ListenerAliasTable_register(0);

    EXPECT_EQ(entry, nullptr);

}

TEST(AliasMappingListener, register_fills_to_capacity) {

    setup_test();
    fill_table();

    // Table is full — verify all entries are findable
    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A + i);
        EXPECT_NE(entry, nullptr);

    }

}

TEST(AliasMappingListener, register_returns_null_when_full) {

    setup_test();
    fill_table();

    // One more should fail
    node_id_t overflow_id = TEST_NODE_ID_A + LISTENER_ALIAS_TABLE_DEPTH;
    listener_alias_entry_t *entry = ListenerAliasTable_register(overflow_id);

    EXPECT_EQ(entry, nullptr);

}

TEST(AliasMappingListener, register_reuses_slot_after_unregister) {

    setup_test();
    fill_table();

    // Table is full — remove one
    ListenerAliasTable_unregister(TEST_NODE_ID_A);

    // Now should be able to register a new one
    listener_alias_entry_t *entry = ListenerAliasTable_register(TEST_NODE_ID_B);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_B);

}

/*******************************************************************************
 * Unregistration Tests
 ******************************************************************************/

TEST(AliasMappingListener, unregister_removes_entry) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    ListenerAliasTable_unregister(TEST_NODE_ID_A);

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, unregister_nonexistent_is_safe) {

    setup_test();

    // Should not crash or corrupt anything
    ListenerAliasTable_unregister(TEST_NODE_ID_A);

    // Register something else and verify it works
    listener_alias_entry_t *entry = ListenerAliasTable_register(TEST_NODE_ID_B);
    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_B);

}

TEST(AliasMappingListener, unregister_zero_is_safe) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);

    // Should not crash or remove anything
    ListenerAliasTable_unregister(0);

    EXPECT_NE(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, unregister_does_not_affect_other_entries) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_register(TEST_NODE_ID_B);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    ListenerAliasTable_unregister(TEST_NODE_ID_A);

    // B should be untouched
    listener_alias_entry_t *b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);
    EXPECT_NE(b, nullptr);
    EXPECT_EQ(b->alias, TEST_ALIAS_B);

}

/*******************************************************************************
 * Set Alias Tests (AMD arrival)
 ******************************************************************************/

TEST(AliasMappingListener, set_alias_populates_registered_entry) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, set_alias_ignores_unregistered_node_id) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);

    // Set alias for a node that's NOT registered — should be a no-op
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    // B should not exist
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B), nullptr);

    // A should still have alias 0
    listener_alias_entry_t *a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_NE(a, nullptr);
    EXPECT_EQ(a->alias, 0);

}

TEST(AliasMappingListener, set_alias_rejects_zero_alias) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    // Try to set alias to 0 — should be rejected, original alias preserved
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, 0);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, set_alias_rejects_out_of_range) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);

    // 0x1000 is above 12-bit max — should be rejected
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, 0x1000);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0);

}

TEST(AliasMappingListener, set_alias_rejects_zero_node_id) {

    setup_test();

    // Should not crash or create an entry
    ListenerAliasTable_set_alias(0, TEST_ALIAS_A);

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(0), nullptr);

}

TEST(AliasMappingListener, set_alias_updates_existing_alias) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_B);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_B);

}

TEST(AliasMappingListener, set_alias_max_valid) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, 0xFFF);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0xFFF);

}

TEST(AliasMappingListener, set_alias_min_valid) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, 0x001);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0x001);

}

/*******************************************************************************
 * Find By Node ID Tests
 ******************************************************************************/

TEST(AliasMappingListener, find_returns_null_for_unregistered) {

    setup_test();

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, find_returns_null_for_zero_node_id) {

    setup_test();

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(0), nullptr);

}

TEST(AliasMappingListener, find_returns_entry_with_unresolved_alias) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0);

}

TEST(AliasMappingListener, find_returns_entry_with_resolved_alias) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, find_last_entry_in_full_table) {

    setup_test();
    fill_table();

    node_id_t last_id = TEST_NODE_ID_A + LISTENER_ALIAS_TABLE_DEPTH - 1;
    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(last_id);

    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->node_id, last_id);

}

/*******************************************************************************
 * Flush Aliases Tests (Global AME)
 ******************************************************************************/

TEST(AliasMappingListener, flush_zeros_all_aliases) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_register(TEST_NODE_ID_B);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    ListenerAliasTable_flush_aliases();

    listener_alias_entry_t *a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    listener_alias_entry_t *b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);

    EXPECT_NE(a, nullptr);
    EXPECT_NE(b, nullptr);
    EXPECT_EQ(a->alias, 0);
    EXPECT_EQ(b->alias, 0);

}

TEST(AliasMappingListener, flush_preserves_node_ids) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_register(TEST_NODE_ID_B);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    ListenerAliasTable_flush_aliases();

    listener_alias_entry_t *a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    listener_alias_entry_t *b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);

    EXPECT_NE(a, nullptr);
    EXPECT_NE(b, nullptr);
    EXPECT_EQ(a->node_id, TEST_NODE_ID_A);
    EXPECT_EQ(b->node_id, TEST_NODE_ID_B);

}

TEST(AliasMappingListener, flush_then_repopulate) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    ListenerAliasTable_flush_aliases();

    // Simulate AMD reply re-populating
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_B);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_B);

}

TEST(AliasMappingListener, flush_on_empty_table_is_safe) {

    setup_test();

    // Should not crash
    ListenerAliasTable_flush_aliases();

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, flush_full_table) {

    setup_test();
    fill_table();

    // Set aliases on all entries
    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        ListenerAliasTable_set_alias(TEST_NODE_ID_A + i, TEST_ALIAS_A + i);

    }

    ListenerAliasTable_flush_aliases();

    // All aliases should be 0, all node_ids preserved
    for (int i = 0; i < LISTENER_ALIAS_TABLE_DEPTH; i++) {

        listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A + i);
        EXPECT_NE(entry, nullptr);
        EXPECT_EQ(entry->alias, 0);
        EXPECT_EQ(entry->node_id, (node_id_t) (TEST_NODE_ID_A + i));

    }

}

/*******************************************************************************
 * Clear Alias By Alias Tests (AMR)
 ******************************************************************************/

TEST(AliasMappingListener, clear_alias_by_alias_zeros_matching_entry) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    ListenerAliasTable_clear_alias_by_alias(TEST_ALIAS_A);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->alias, 0);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_A);

}

TEST(AliasMappingListener, clear_alias_by_alias_does_not_affect_others) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_register(TEST_NODE_ID_B);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    ListenerAliasTable_clear_alias_by_alias(TEST_ALIAS_A);

    listener_alias_entry_t *a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    listener_alias_entry_t *b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);

    EXPECT_EQ(a->alias, 0);
    EXPECT_EQ(b->alias, TEST_ALIAS_B);

}

TEST(AliasMappingListener, clear_alias_nonexistent_is_safe) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    // Clear an alias that's not in the table — should be a no-op
    ListenerAliasTable_clear_alias_by_alias(TEST_ALIAS_C);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, clear_alias_zero_is_safe) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    // Should not crash or clear anything
    ListenerAliasTable_clear_alias_by_alias(0);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

/*******************************************************************************
 * Full Lifecycle Tests
 ******************************************************************************/

TEST(AliasMappingListener, lifecycle_attach_resolve_forward_detach) {

    setup_test();

    // 1. Listener attached — register node_id
    listener_alias_entry_t *entry = ListenerAliasTable_register(TEST_NODE_ID_A);
    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->alias, 0);

    // 2. AME sent, AMD reply arrives — set alias
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

    // 3. TX path resolves alias for forwarding — find succeeds
    listener_alias_entry_t *resolved = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->alias, TEST_ALIAS_A);

    // 4. Listener detached — unregister
    ListenerAliasTable_unregister(TEST_NODE_ID_A);
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, lifecycle_amr_then_amd_recovery) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    // AMR arrives — node releases alias
    ListenerAliasTable_clear_alias_by_alias(TEST_ALIAS_A);

    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, 0);

    // Node re-logs in with new alias, AMD arrives
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_B);

    entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->alias, TEST_ALIAS_B);

}

TEST(AliasMappingListener, lifecycle_global_ame_flush_and_recovery) {

    setup_test();

    ListenerAliasTable_register(TEST_NODE_ID_A);
    ListenerAliasTable_register(TEST_NODE_ID_B);
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_B);

    // Global AME received — flush all aliases
    ListenerAliasTable_flush_aliases();

    // TX path would fail (alias == 0), retry later
    listener_alias_entry_t *a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    listener_alias_entry_t *b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);
    EXPECT_EQ(a->alias, 0);
    EXPECT_EQ(b->alias, 0);

    // AMD replies arrive, re-populate
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_C);
    ListenerAliasTable_set_alias(TEST_NODE_ID_B, TEST_ALIAS_A);

    a = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    b = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B);
    EXPECT_EQ(a->alias, TEST_ALIAS_C);
    EXPECT_EQ(b->alias, TEST_ALIAS_A);

}

/*******************************************************************************
 * Table Depth Verification
 ******************************************************************************/

TEST(AliasMappingListener, table_depth_matches_user_defines) {

    // Compile-time check that table depth is computed from user defines
    EXPECT_EQ(LISTENER_ALIAS_TABLE_DEPTH,
              USER_DEFINED_MAX_LISTENERS_PER_TRAIN * USER_DEFINED_TRAIN_NODE_COUNT);

}

/*******************************************************************************
 * Verification Prober Tests
 ******************************************************************************/

/**
 * Helper: register a node, set its alias, then manually set verify_ticks
 * to simulate an entry that has been resolved for a while.
 */
static listener_alias_entry_t *setup_resolved_entry(node_id_t node_id,
                                                     uint16_t alias,
                                                     uint16_t verify_ticks) {

    listener_alias_entry_t *entry = ListenerAliasTable_register(node_id);
    ListenerAliasTable_set_alias(node_id, alias);
    entry->verify_ticks = verify_ticks;
    return entry;

}

/**
 * @brief Advance the prober's internal counter by calling check_one_verification
 *        the specified number of times on an empty table.
 *
 * @details Each call with a unique tick value increments the monotonic
 *          counter.  Call before registering entries so that entries stamped
 *          with verify_ticks = 0 appear "due" when the counter reaches
 *          PROBE_INTERVAL_TICKS.
 */
static void advance_prober_counter(uint16_t count) {

    for (uint16_t i = 0; i < count; i++) {

        uint8_t tick = (uint8_t) (i + 1);
        ListenerAliasTable_check_one_verification(tick);

    }

}

TEST(AliasMappingListener, verify_no_entries_returns_zero) {

    setup_test();

    // Empty table — nothing to probe
    EXPECT_EQ(ListenerAliasTable_check_one_verification(1), (node_id_t) 0);

}

TEST(AliasMappingListener, verify_unresolved_entry_skipped) {

    setup_test();

    // Registered but alias not yet resolved
    ListenerAliasTable_register(TEST_NODE_ID_A);

    EXPECT_EQ(ListenerAliasTable_check_one_verification(255), (node_id_t) 0);

}

TEST(AliasMappingListener, verify_resolved_entry_not_due) {

    setup_test();

    // Resolved entry with verify_ticks = 0; tick = 1 — interval not elapsed
    setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    EXPECT_EQ(ListenerAliasTable_check_one_verification(1), (node_id_t) 0);

}

TEST(AliasMappingListener, verify_resolved_entry_due_returns_node_id) {

    setup_test();

    // Advance prober counter so entries registered now will be due
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);

    setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    node_id_t result = ListenerAliasTable_check_one_verification(tick);

    EXPECT_EQ(result, TEST_NODE_ID_A);

    // Entry should now be pending
    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    EXPECT_EQ(entry->verify_pending, 1);

}

TEST(AliasMappingListener, verify_pending_entry_amd_clears_pending) {

    setup_test();

    // Advance counter, register entry, probe it to make it pending
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    listener_alias_entry_t *entry = setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(entry->verify_pending, 1);

    // Simulate AMD arrival
    ListenerAliasTable_set_alias(TEST_NODE_ID_A, TEST_ALIAS_A);

    EXPECT_EQ(entry->verify_pending, 0);
    EXPECT_EQ(entry->alias, TEST_ALIAS_A);

}

TEST(AliasMappingListener, verify_pending_timeout_clears_alias) {

    setup_test();

    // Advance counter, register entry, probe it to make it pending
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    listener_alias_entry_t *entry = setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(entry->verify_pending, 1);

    // Advance by VERIFY_TIMEOUT_TICKS — should detect stale
    advance_prober_counter(USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS);

    tick = (uint8_t) (tick + USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL +
            USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS);
    node_id_t result = ListenerAliasTable_check_one_verification(tick);

    // Stale handled internally, returns 0
    EXPECT_EQ(result, (node_id_t) 0);

    // Entry: alias cleared, verify_pending cleared, node_id preserved
    EXPECT_EQ(entry->alias, 0);
    EXPECT_EQ(entry->verify_pending, 0);
    EXPECT_EQ(entry->node_id, TEST_NODE_ID_A);

}

TEST(AliasMappingListener, verify_rate_limiting) {

    setup_test();

    // Advance counter so entry is due
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);

    // First call — returns node_id
    node_id_t result1 = ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(result1, TEST_NODE_ID_A);

    // Reset entry for another probe attempt
    listener_alias_entry_t *entry = ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A);
    entry->verify_pending = 0;
    entry->verify_ticks = 0;  // stamp as "long ago" to ensure due

    // Same tick — rate-limited, returns 0
    node_id_t result2 = ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(result2, (node_id_t) 0);

    // Advance by PROBE_TICK_INTERVAL — can probe again
    uint8_t next_tick = (uint8_t) (tick + USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL);
    node_id_t result3 = ListenerAliasTable_check_one_verification(next_tick);
    EXPECT_EQ(result3, TEST_NODE_ID_A);

}

TEST(AliasMappingListener, verify_round_robin_advances) {

    setup_test();

    // Advance counter so all entries will be due
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);

    setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);
    setup_resolved_entry(TEST_NODE_ID_B, TEST_ALIAS_B, 0);
    setup_resolved_entry(TEST_NODE_ID_C, TEST_ALIAS_C, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);

    // Three calls, advancing tick each time — should get 3 different node_ids
    node_id_t r1 = ListenerAliasTable_check_one_verification(tick);
    tick = (uint8_t) (tick + USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL);
    node_id_t r2 = ListenerAliasTable_check_one_verification(tick);
    tick = (uint8_t) (tick + USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL);
    node_id_t r3 = ListenerAliasTable_check_one_verification(tick);

    // All should be non-zero and distinct
    EXPECT_NE(r1, (node_id_t) 0);
    EXPECT_NE(r2, (node_id_t) 0);
    EXPECT_NE(r3, (node_id_t) 0);
    EXPECT_NE(r1, r2);
    EXPECT_NE(r2, r3);
    EXPECT_NE(r1, r3);

}

TEST(AliasMappingListener, verify_flush_clears_pending) {

    setup_test();

    // Advance counter, register, probe to make pending
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    listener_alias_entry_t *entry = setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(entry->verify_pending, 1);

    // Flush aliases
    ListenerAliasTable_flush_aliases();

    EXPECT_EQ(entry->alias, 0);
    EXPECT_EQ(entry->verify_pending, 0);

}

TEST(AliasMappingListener, verify_unregister_clears_pending) {

    setup_test();

    // Advance counter, register, probe to make pending
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    listener_alias_entry_t *entry = setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(entry->verify_pending, 1);

    // Unregister
    ListenerAliasTable_unregister(TEST_NODE_ID_A);

    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);

}

TEST(AliasMappingListener, verify_clear_alias_by_alias_clears_pending) {

    setup_test();

    // Advance counter, register, probe to make pending
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    listener_alias_entry_t *entry = setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(entry->verify_pending, 1);

    // Clear alias by alias value (AMR)
    ListenerAliasTable_clear_alias_by_alias(TEST_ALIAS_A);

    EXPECT_EQ(entry->alias, 0);
    EXPECT_EQ(entry->verify_pending, 0);

}

TEST(AliasMappingListener, verify_initialize_resets_cursor) {

    setup_test();

    // Advance counter and register entries
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    setup_resolved_entry(TEST_NODE_ID_A, TEST_ALIAS_A, 0);
    setup_resolved_entry(TEST_NODE_ID_B, TEST_ALIAS_B, 0);

    uint8_t tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    ListenerAliasTable_check_one_verification(tick);
    tick = (uint8_t) (tick + USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL);
    ListenerAliasTable_check_one_verification(tick);

    // Re-initialize — resets counter and cursor to 0
    ListenerAliasTable_initialize();

    // All entries should be cleared
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_A), nullptr);
    EXPECT_EQ(ListenerAliasTable_find_by_node_id(TEST_NODE_ID_B), nullptr);

    // Advance counter again, register, and verify probe works
    advance_prober_counter(USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS);
    setup_resolved_entry(TEST_NODE_ID_C, TEST_ALIAS_C, 0);
    tick = (uint8_t) (USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS + 1);
    node_id_t result = ListenerAliasTable_check_one_verification(tick);
    EXPECT_EQ(result, TEST_NODE_ID_C);

}
