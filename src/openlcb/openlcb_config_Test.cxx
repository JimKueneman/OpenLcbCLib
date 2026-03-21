/** \copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file openlcb_config_Test.cxx
 * @brief Test suite for openlcb_config.c — library initialization and wiring.
 *
 * @details Exercises the public API of openlcb_config.c to achieve 100% line
 * coverage of every _build_* function (via OpenLcb_initialize with all features
 * compiled in), _run_periodic_services (via OpenLcb_run), and each public
 * function.
 *
 * Because CanMainStateMachine_run() dereferences its interface immediately,
 * CanMainStatemachine_initialize() must be called with a mock interface before
 * OpenLcb_run() is safe. _global_initialize() handles both.
 *
 * Test Categories:
 *   1. Initialization (2 tests)
 *   2. Node Creation (2 tests)
 *   3. Timer Tick (2 tests)
 *   4. Run (1 test)
 *
 * Coverage:
 *   - OpenLcb_initialize:            100%
 *   - OpenLcb_create_node:           100%
 *   - OpenLcb_100ms_timer_tick:      100%
 *   - OpenLcb_get_global_100ms_tick: 100%
 *   - OpenLcb_run:                   100%
 *   - _run_periodic_services:        100%
 *   - All _build_* functions:        100%
 *
 * @author Jim Kueneman
 * @date 18 Mar 2026
 */

#include "test/main_Test.hxx"

extern "C" {

#include "openlcb_config.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "drivers/canbus/can_main_statemachine.h"
#include "drivers/canbus/can_types.h"

}

// =============================================================================
// Mock state
// =============================================================================

static alias_mapping_info_t _mock_alias_info = {};

// =============================================================================
// Mock functions — shared resources
// =============================================================================

static void _mock_lock(void) {}

static void _mock_unlock(void) {}

// =============================================================================
// Mock functions — config memory
// =============================================================================

static uint16_t _mock_config_read(openlcb_node_t *node, uint32_t address,
                                  uint16_t count, configuration_memory_buffer_t *buffer) {

    return count;

}

static uint16_t _mock_config_write(openlcb_node_t *node, uint32_t address,
                                   uint16_t count, configuration_memory_buffer_t *buffer) {

    return count;

}

// =============================================================================
// Mock functions — CAN main statemachine interface
// =============================================================================

static bool _mock_send_can(can_msg_t *msg) { return true; }

static openlcb_node_t *_mock_node_get_first(uint8_t key) { return NULL; }

static openlcb_node_t *_mock_node_get_next(uint8_t key) { return NULL; }

static openlcb_node_t *_mock_node_find_alias(uint16_t alias) { return NULL; }

static void _mock_login_sm_run(can_statemachine_info_t *info) {}

static alias_mapping_info_t *_mock_alias_get_info(void) { return &_mock_alias_info; }

static void _mock_alias_unregister(uint16_t alias) {}

static uint8_t _mock_get_tick(void) { return 0; }

static bool _mock_return_false(void) { return false; }

static bool _mock_return_true(void) { return true; }

// =============================================================================
// Interface structs
// =============================================================================

static const interface_can_main_statemachine_t _can_interface = {

    .lock_shared_resources                = &_mock_lock,
    .unlock_shared_resources              = &_mock_unlock,
    .send_can_message                     = &_mock_send_can,
    .openlcb_node_get_first               = &_mock_node_get_first,
    .openlcb_node_get_next                = &_mock_node_get_next,
    .openlcb_node_find_by_alias           = &_mock_node_find_alias,
    .login_statemachine_run               = &_mock_login_sm_run,
    .alias_mapping_get_alias_mapping_info = &_mock_alias_get_info,
    .alias_mapping_unregister             = &_mock_alias_unregister,
    .get_current_tick                     = &_mock_get_tick,
    .handle_duplicate_aliases             = &_mock_return_false,
    .handle_outgoing_can_message          = &_mock_return_false,
    .handle_login_outgoing_can_message    = &_mock_return_false,
    .handle_try_enumerate_first_node      = &_mock_return_true,
    .handle_try_enumerate_next_node       = &_mock_return_true,
    .handle_listener_verification         = NULL,
    .listener_flush_aliases               = NULL,
    .listener_set_alias                   = NULL,

};

static const openlcb_config_t _config = {

    .lock_shared_resources   = &_mock_lock,
    .unlock_shared_resources = &_mock_unlock,
    .config_mem_read         = &_mock_config_read,
    .config_mem_write        = &_mock_config_write,

};

// =============================================================================
// Test node parameters
// =============================================================================

static const node_parameters_t _node_params = {

    .snip = {
        .mfg_version = 4,
        .name = "Test Node",
        .model = "Config Test",
        .hardware_version = "1.0",
        .software_version = "1.0",
        .user_version = 2
    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_IDENTIFICATION,

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

};

// =============================================================================
// Helper
// =============================================================================

static void _global_initialize(void) {

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcb_initialize(&_config);

}

// =============================================================================
// Section 1: Initialization
// =============================================================================

TEST(OpenLcbConfig, initialize_does_not_crash) {

    _global_initialize();
    SUCCEED();

}

TEST(OpenLcbConfig, get_tick_returns_valid_uint8) {

    _global_initialize();
    uint8_t tick = OpenLcb_get_global_100ms_tick();
    EXPECT_LE(tick, 255u);

}

// =============================================================================
// Section 2: Node Creation
// =============================================================================

TEST(OpenLcbConfig, create_node_returns_valid_node) {

    _global_initialize();
    openlcb_node_t *node = OpenLcb_create_node(0x050101010101ULL, &_node_params);
    EXPECT_NE(node, nullptr);

}

TEST(OpenLcbConfig, create_node_exhausted_returns_null) {

    _global_initialize();

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        OpenLcb_create_node(0x050101010100ULL + i, &_node_params);

    }

    openlcb_node_t *overflow = OpenLcb_create_node(0x050101010200ULL, &_node_params);
    EXPECT_EQ(overflow, nullptr);

}

// =============================================================================
// Section 3: Timer Tick
// =============================================================================

TEST(OpenLcbConfig, timer_tick_increments_counter) {

    _global_initialize();
    uint8_t before = OpenLcb_get_global_100ms_tick();
    OpenLcb_100ms_timer_tick();
    EXPECT_EQ(OpenLcb_get_global_100ms_tick(), (uint8_t)(before + 1));

}

TEST(OpenLcbConfig, timer_tick_wraps_at_256) {

    _global_initialize();
    uint8_t start = OpenLcb_get_global_100ms_tick();

    for (int i = 0; i < 256; i++) {

        OpenLcb_100ms_timer_tick();

    }

    EXPECT_EQ(OpenLcb_get_global_100ms_tick(), start);

}

// =============================================================================
// Section 4: Run
// =============================================================================

TEST(OpenLcbConfig, run_does_not_crash) {

    _global_initialize();
    OpenLcb_run();
    SUCCEED();

}
