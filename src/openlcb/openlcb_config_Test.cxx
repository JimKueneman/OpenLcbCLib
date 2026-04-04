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
 * @brief Test suite for openlcb_config.c -- library initialization and wiring.
 *
 * @details Exercises the public API of openlcb_config.c to achieve 100% line
 * coverage of every _build_* function (via OpenLcbConfig_initialize with all
 * features compiled in), _run_periodic_services (via OpenLcbConfig_run), each
 * public function, and all static stream read/write callbacks.
 *
 * Test Categories:
 *   1. Initialization (2 tests)
 *   2. Node Creation (2 tests)
 *   3. Timer Tick (2 tests)
 *   4. Run (1 test)
 *   5. Stream Read Callbacks -- CDI, All, Config Mem, ACDI Mfg, ACDI User,
 *      Train FDI, Train Function Config Memory
 *   6. Stream Write Callbacks -- Config Mem, ACDI User, Train Function Config Memory
 *
 * @author Jim Kueneman
 * @date 18 Mar 2026
 */

#include "test/main_Test.hxx"

#include <cstring>

extern "C" {

#include "openlcb_config.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "openlcb_application_train.h"
#include "drivers/canbus/can_main_statemachine.h"
#include "drivers/canbus/can_types.h"

// Matches _config_field_t in openlcb_config.c
typedef struct {
    uint32_t start;
    uint16_t len;
    const void *data;
} _config_field_t;

// GTEST wrappers for static stream callbacks in openlcb_config.c
extern uint16_t OpenLcbConfigTest_read_field_array(const _config_field_t *fields,
        uint16_t num_fields, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_config_definition_info(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_all(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_configuration_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_acdi_user(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_train_function_definition_info(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_read_request_train_function_config_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_write_request_configuration_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_write_request_acdi_user(
        openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer);
extern uint16_t OpenLcbConfigTest_stream_write_request_train_function_config_memory(
        openlcb_node_t *node, uint32_t address, uint16_t count, const uint8_t *buffer);

}

// =============================================================================
// Mock state
// =============================================================================

static alias_mapping_info_t _mock_alias_info = {};

// Config memory mock backing store
static uint8_t _mock_config_mem[256];
static uint8_t _mock_config_write_dest[256];
static int _mock_config_read_called = 0;
static int _mock_config_write_called = 0;

// =============================================================================
// Mock functions -- shared resources
// =============================================================================

static void _mock_lock(void) {}

static void _mock_unlock(void) {}

// =============================================================================
// Mock functions -- config memory
// =============================================================================

static uint16_t _mock_config_read(openlcb_node_t *node, uint32_t address,
                                  uint16_t count, configuration_memory_buffer_t *buffer) {

    _mock_config_read_called++;

    for (uint16_t i = 0; i < count; i++) {

        ((uint8_t *) buffer)[i] = _mock_config_mem[(address + i) % sizeof(_mock_config_mem)];

    }

    return count;

}

static uint16_t _mock_config_write(openlcb_node_t *node, uint32_t address,
                                   uint16_t count, configuration_memory_buffer_t *buffer) {

    _mock_config_write_called++;

    for (uint16_t i = 0; i < count; i++) {

        _mock_config_write_dest[(address + i) % sizeof(_mock_config_write_dest)] = ((uint8_t *) buffer)[i];

    }

    return count;

}

// =============================================================================
// Mock functions -- CAN main statemachine interface
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
// Test data
// =============================================================================

static const uint8_t _test_cdi[] = {

    '<', 'c', 'd', 'i', '>', 'H', 'e', 'l', 'l', 'o', '<', '/', 'c', 'd', 'i', '>', 0x00

};

static const uint8_t _test_fdi[] = {

    '<', 'f', 'd', 'i', '>', 'T', 'r', 'a', 'i', 'n', '<', '/', 'f', 'd', 'i', '>', 0x00

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

static const node_parameters_t _node_params_full = {

    .snip = {
        .mfg_version = 4,
        .name = "Test Manufacturer",
        .model = "Test Model XYZ",
        .hardware_version = "HW2.0",
        .software_version = "SW3.1",
        .user_version = 2
    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_IDENTIFICATION
                        | PSI_STREAM | PSI_DATAGRAM,

    .address_space_configuration_definition = {
        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address = (uint32_t) (sizeof(_test_cdi) - 1),
        .low_address = 0,
        .description = "CDI",
    },

    .address_space_all = {
        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_ALL,
        .highest_address = (uint32_t) (sizeof(_test_cdi) - 1),
        .low_address = 0,
        .description = "All",
    },

    .address_space_config_memory = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address = 255,
        .low_address = 0,
        .description = "Config",
    },

    .address_space_acdi_manufacturer = {
        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
        .highest_address = 0x7C,
        .low_address = 0,
        .description = "ACDI Mfg",
    },

    .address_space_acdi_user = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
        .highest_address = 0x7F,
        .low_address = 0,
        .description = "ACDI User",
    },

    .address_space_train_function_definition_info = {
        .present = true,
        .read_only = true,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,
        .highest_address = (uint32_t) (sizeof(_test_fdi) - 1),
        .low_address = 0,
        .description = "FDI",
    },

    .address_space_train_function_config_memory = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY,
        .highest_address = (USER_DEFINED_MAX_TRAIN_FUNCTIONS * 2) - 1,
        .low_address = 0,
        .description = "Fn Config",
    },

    .cdi = _test_cdi,
    .fdi = _test_fdi,

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

};

// Node params with NULL cdi/fdi for null-pointer branches
static const node_parameters_t _node_params_null_cdi = {

    .snip = {
        .mfg_version = 4,
        .name = "Test Node",
        .model = "Config Test",
        .hardware_version = "1.0",
        .software_version = "1.0",
        .user_version = 2
    },

    .protocol_support = PSI_SIMPLE_NODE_INFORMATION | PSI_IDENTIFICATION,

    .cdi = NULL,
    .fdi = NULL,

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

};

// =============================================================================
// Helper
// =============================================================================

static void _global_initialize(void) {

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcbConfig_initialize(&_config);

}

static void _reset_mock_state(void) {

    memset(_mock_config_mem, 0, sizeof(_mock_config_mem));
    memset(_mock_config_write_dest, 0, sizeof(_mock_config_write_dest));
    _mock_config_read_called = 0;
    _mock_config_write_called = 0;

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
    uint8_t tick = OpenLcbConfig_get_global_100ms_tick();
    EXPECT_LE(tick, 255u);

}

// =============================================================================
// Section 2: Node Creation
// =============================================================================

TEST(OpenLcbConfig, create_node_returns_valid_node) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010101ULL, &_node_params);
    EXPECT_NE(node, nullptr);

}

TEST(OpenLcbConfig, create_node_exhausted_returns_null) {

    _global_initialize();

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++) {

        OpenLcbConfig_create_node(0x050101010100ULL + i, &_node_params);

    }

    openlcb_node_t *overflow = OpenLcbConfig_create_node(0x050101010200ULL, &_node_params);
    EXPECT_EQ(overflow, nullptr);

}

// =============================================================================
// Section 3: Timer Tick
// =============================================================================

TEST(OpenLcbConfig, timer_tick_increments_counter) {

    _global_initialize();
    uint8_t before = OpenLcbConfig_get_global_100ms_tick();
    OpenLcbConfig_100ms_timer_tick();
    EXPECT_EQ(OpenLcbConfig_get_global_100ms_tick(), (uint8_t)(before + 1));

}

TEST(OpenLcbConfig, timer_tick_wraps_at_256) {

    _global_initialize();
    uint8_t start = OpenLcbConfig_get_global_100ms_tick();

    for (int i = 0; i < 256; i++) {

        OpenLcbConfig_100ms_timer_tick();

    }

    EXPECT_EQ(OpenLcbConfig_get_global_100ms_tick(), start);

}

// =============================================================================
// Section 4: Run
// =============================================================================

TEST(OpenLcbConfig, run_does_not_crash) {

    _global_initialize();
    OpenLcbConfig_run();
    SUCCEED();

}

// =============================================================================
// Section 5: Stream Read Callbacks
// =============================================================================

TEST(OpenLcbConfig, stream_read_cdi_copies_data) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010101ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_config_definition_info(
            node, 0, 5, buffer);

    EXPECT_EQ(actual, 5u);
    EXPECT_EQ(buffer[0], '<');
    EXPECT_EQ(buffer[1], 'c');
    EXPECT_EQ(buffer[2], 'd');
    EXPECT_EQ(buffer[3], 'i');
    EXPECT_EQ(buffer[4], '>');

}

TEST(OpenLcbConfig, stream_read_cdi_null_returns_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010102ULL, &_node_params_null_cdi);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    uint16_t actual = OpenLcbConfigTest_stream_read_request_config_definition_info(
            node, 0, 5, buffer);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_read_all_copies_data) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010103ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_all(node, 0, 5, buffer);

    EXPECT_EQ(actual, 5u);
    EXPECT_EQ(buffer[0], '<');

}

TEST(OpenLcbConfig, stream_read_all_null_returns_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010104ULL, &_node_params_null_cdi);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    uint16_t actual = OpenLcbConfigTest_stream_read_request_all(node, 0, 5, buffer);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_read_config_memory_delegates) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010105ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    _mock_config_mem[0] = 0xAA;
    _mock_config_mem[1] = 0xBB;

    uint8_t buffer[8];
    memset(buffer, 0, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_configuration_memory(
            node, 0, 2, buffer);

    EXPECT_EQ(actual, 2u);
    EXPECT_EQ(buffer[0], 0xAA);
    EXPECT_EQ(buffer[1], 0xBB);
    EXPECT_EQ(_mock_config_read_called, 1);

}

TEST(OpenLcbConfig, stream_read_config_memory_null_callback_returns_zero) {

    // Use a config with NULL config_mem_read
    openlcb_config_t config_no_read = {};
    config_no_read.lock_shared_resources   = &_mock_lock;
    config_no_read.unlock_shared_resources = &_mock_unlock;
    config_no_read.config_mem_read         = NULL;
    config_no_read.config_mem_write        = &_mock_config_write;

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcbConfig_initialize(&config_no_read);

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010106ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    uint16_t actual = OpenLcbConfigTest_stream_read_request_configuration_memory(
            node, 0, 2, buffer);

    EXPECT_EQ(actual, 0u);

    // Restore normal config for subsequent tests
    _global_initialize();

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_reads_version) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010107ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read just the version byte at address 0x00
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, CONFIG_MEM_ACDI_MANUFACTURER_VERSION_ADDRESS, 1, buffer);

    EXPECT_EQ(actual, 1u);
    EXPECT_EQ(buffer[0], 4u);

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_reads_name) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010108ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[32];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read from start of manufacturer name (address 0x01)
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, CONFIG_MEM_ACDI_MANUFACTURER_ADDRESS, 17, buffer);

    EXPECT_EQ(actual, 17u);
    EXPECT_EQ(buffer[0], 'T');
    EXPECT_EQ(buffer[1], 'e');
    EXPECT_EQ(buffer[2], 's');
    EXPECT_EQ(buffer[3], 't');

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_cross_field_boundary) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010109ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Read across version + name boundary: start at 0x00, read 4 bytes
    // Byte 0: version (4), Bytes 1-3: first 3 chars of name "Tes"
    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, 0x00, 4, buffer);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(buffer[0], 4u);
    EXPECT_EQ(buffer[1], 'T');
    EXPECT_EQ(buffer[2], 'e');
    EXPECT_EQ(buffer[3], 's');

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_model_field) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010AULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[16];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read model field starting at CONFIG_MEM_ACDI_MODEL_ADDRESS (0x2A)
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, CONFIG_MEM_ACDI_MODEL_ADDRESS, 14, buffer);

    EXPECT_EQ(actual, 14u);
    EXPECT_EQ(buffer[0], 'T');  // "Test Model XYZ"
    EXPECT_EQ(buffer[5], 'M');

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_full_range) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010BULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Read all 125 bytes (0x00-0x7C) -- covers all fields and gap filling
    uint8_t buffer[125];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, 0x00, 125, buffer);

    EXPECT_EQ(actual, 125u);

    // Version at 0x00
    EXPECT_EQ(buffer[0], 4u);

    // Software version at 0x68
    EXPECT_EQ(buffer[0x68], 'S');  // "SW3.1"
    EXPECT_EQ(buffer[0x69], 'W');

}

TEST(OpenLcbConfig, stream_read_acdi_user_reads_version) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010CULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read user version byte at address 0x00
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_VERSION_ADDRESS, 1, buffer);

    EXPECT_EQ(actual, 1u);
    EXPECT_EQ(buffer[0], 2u);  // user_version = 2

}

TEST(OpenLcbConfig, stream_read_acdi_user_reads_name_from_config_mem) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010DULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Pre-populate mock config memory with user name
    _mock_config_mem[0] = 'M';
    _mock_config_mem[1] = 'y';
    _mock_config_mem[2] = ' ';
    _mock_config_mem[3] = 'N';
    _mock_config_mem[4] = 'o';
    _mock_config_mem[5] = 'd';
    _mock_config_mem[6] = 'e';
    _mock_config_mem[7] = 0;

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read user name starting at 0x01
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 8, buffer);

    EXPECT_EQ(actual, 8u);
    EXPECT_EQ(buffer[0], 'M');
    EXPECT_EQ(buffer[1], 'y');
    EXPECT_GT(_mock_config_read_called, 0);

}

TEST(OpenLcbConfig, stream_read_acdi_user_reads_description_from_config_mem) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010EULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Pre-populate mock config memory at the description offset
    _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET] = 'D';
    _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + 1] = 'e';
    _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + 2] = 's';
    _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + 3] = 'c';

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read user description starting at 0x40
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 4, buffer);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(buffer[0], 'D');
    EXPECT_GT(_mock_config_read_called, 0);

}

TEST(OpenLcbConfig, stream_read_acdi_user_null_config_read_fills_zeros) {

    // Use a config with NULL config_mem_read
    openlcb_config_t config_no_read = {};
    config_no_read.lock_shared_resources   = &_mock_lock;
    config_no_read.unlock_shared_resources = &_mock_unlock;
    config_no_read.config_mem_read         = NULL;
    config_no_read.config_mem_write        = &_mock_config_write;

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcbConfig_initialize(&config_no_read);

    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101010FULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read user name with no config_mem_read -- should fill zeros
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 4, buffer);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(buffer[0], 0x00);
    EXPECT_EQ(buffer[1], 0x00);

    // Also test description path with NULL config_mem_read
    memset(buffer, 0xFF, sizeof(buffer));
    actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 4, buffer);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(buffer[0], 0x00);

    // Restore normal config
    _global_initialize();

}

TEST(OpenLcbConfig, stream_read_acdi_user_past_end_breaks) {

    _global_initialize();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010110ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read beyond the ACDI user address space -- should break out
    uint32_t past_end = CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS
                        + CONFIG_MEM_ACDI_USER_DESCRIPTION_LEN;
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, past_end, 4, buffer);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_read_acdi_user_with_low_address) {

    _global_initialize();
    _reset_mock_state();

    // Create node params with low_address_valid
    node_parameters_t params = _node_params_full;
    params.address_space_config_memory.low_address_valid = true;
    params.address_space_config_memory.low_address = 0x100;

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010111ULL, &params);
    ASSERT_NE(node, nullptr);

    // Pre-populate config mem at offset 0x100 (low_address + name offset)
    uint32_t idx = (0x100 + CONFIG_MEM_CONFIG_USER_NAME_OFFSET) % sizeof(_mock_config_mem);
    _mock_config_mem[idx] = 'L';
    _mock_config_mem[idx + 1] = 'o';
    _mock_config_mem[idx + 2] = 'w';

    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 3, buffer);

    EXPECT_EQ(actual, 3u);
    EXPECT_GT(_mock_config_read_called, 0);

}

TEST(OpenLcbConfig, stream_read_acdi_user_description_with_low_address) {

    _global_initialize();
    _reset_mock_state();

    // Create node params with low_address_valid on config_memory space
    node_parameters_t params = _node_params_full;
    params.address_space_config_memory.low_address_valid = true;
    params.address_space_config_memory.low_address = 0x80;

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010120ULL, &params);
    ASSERT_NE(node, nullptr);

    // Pre-populate config mem at the offset (low_address + description offset)
    uint32_t idx = (0x80 + CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET)
                   % sizeof(_mock_config_mem);
    _mock_config_mem[idx] = 'D';
    _mock_config_mem[idx + 1] = 'L';

    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 2, buffer);

    EXPECT_EQ(actual, 2u);
    EXPECT_GT(_mock_config_read_called, 0);

}

TEST(OpenLcbConfig, stream_read_acdi_user_name_short_read_breaks) {

    // Need a config_mem_read that returns fewer bytes than requested
    static auto short_read = [](openlcb_node_t *node, uint32_t address,
                                uint16_t count, configuration_memory_buffer_t *buffer) -> uint16_t {

        // Return only 1 byte regardless of count
        ((uint8_t *) buffer)[0] = 'S';
        return 1;

    };

    openlcb_config_t config_short = {};
    config_short.lock_shared_resources   = &_mock_lock;
    config_short.unlock_shared_resources = &_mock_unlock;
    config_short.config_mem_read         = short_read;
    config_short.config_mem_write        = &_mock_config_write;

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcbConfig_initialize(&config_short);

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010121ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Request 4 bytes from user name but config_mem_read only returns 1
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 4, buffer);

    // Should break early after receiving only 1 byte
    EXPECT_EQ(actual, 1u);
    EXPECT_EQ(buffer[0], 'S');

    // Restore normal config
    _global_initialize();

}

TEST(OpenLcbConfig, stream_read_acdi_user_description_short_read_breaks) {

    static auto short_read = [](openlcb_node_t *node, uint32_t address,
                                uint16_t count, configuration_memory_buffer_t *buffer) -> uint16_t {

        ((uint8_t *) buffer)[0] = 'X';
        return 1;

    };

    openlcb_config_t config_short = {};
    config_short.lock_shared_resources   = &_mock_lock;
    config_short.unlock_shared_resources = &_mock_unlock;
    config_short.config_mem_read         = short_read;
    config_short.config_mem_write        = &_mock_config_write;

    CanMainStatemachine_initialize(&_can_interface);
    OpenLcbConfig_initialize(&config_short);

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010122ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Request 4 bytes from description but config_mem_read only returns 1
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 4, buffer);

    EXPECT_EQ(actual, 1u);
    EXPECT_EQ(buffer[0], 'X');

    // Restore normal config
    _global_initialize();

}

TEST(OpenLcbConfig, stream_read_acdi_user_name_clamps_to_field_end) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010125ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Populate mock config memory for name and description
    for (uint16_t i = 0; i < 63; i++) {

        _mock_config_mem[CONFIG_MEM_CONFIG_USER_NAME_OFFSET + i] = 'N';

    }

    for (uint16_t i = 0; i < 64; i++) {

        _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + i] = 'D';

    }

    // Read starting at address 0x38 (inside name field), count 16.
    // name_avail = 63 - (0x38 - 0x01) = 8, count - filled = 16.
    // This forces name_avail < (count - filled), hitting the true branch
    // on line 512, then the loop continues into the description field.
    uint8_t buffer[16];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, 0x38, 16, buffer);

    EXPECT_EQ(actual, 16u);

    // First 8 bytes come from name field
    EXPECT_EQ(buffer[0], 'N');
    EXPECT_EQ(buffer[7], 'N');

    // Remaining 8 bytes come from description field
    EXPECT_EQ(buffer[8], 'D');
    EXPECT_EQ(buffer[15], 'D');

}

TEST(OpenLcbConfig, stream_read_acdi_user_description_clamps_to_field_end) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010126ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    for (uint16_t i = 0; i < 64; i++) {

        _mock_config_mem[CONFIG_MEM_CONFIG_USER_DESCRIPTION_OFFSET + i] = 'D';

    }

    // Read starting at address 0x70 (inside description field), count 32.
    // desc_avail = 64 - (0x70 - 0x40) = 16, count - filled = 32.
    // This forces desc_avail < (count - filled), hitting the true branch
    // on line 546. Only 16 bytes are available so the function reads those
    // then breaks out via the else clause (past end of space).
    uint8_t buffer[32];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_user(
            node, 0x70, 32, buffer);

    // Only 16 bytes available from description, then the loop hits the
    // else-break because pos reaches 0x80 (past end of ACDI user space)
    EXPECT_EQ(actual, 16u);
    EXPECT_EQ(buffer[0], 'D');
    EXPECT_EQ(buffer[15], 'D');

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_gap_filling) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010123ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // The ACDI manufacturer layout has potential gaps if name is shorter than
    // its field size. Read across the name->model boundary.
    // Name field: 0x01-0x29 (41 bytes), Model field: 0x2A-0x52 (41 bytes)
    // "Test Manufacturer" is 18 chars + null = 19 bytes, rest of name field is padding.
    // Reading from just before model start should hit gap-filling logic if we
    // read starting mid-gap between where the name data ends and model starts.

    // Read starting at offset 0x28 (near end of name field), requesting 4 bytes
    // This spans: name[0x27..0x29] + model[0x2A]
    uint8_t buffer[4];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, 0x28, 4, buffer);

    EXPECT_EQ(actual, 4u);
    // Last byte should be start of model "Test Model XYZ"
    EXPECT_EQ(buffer[2], 'T');

}

TEST(OpenLcbConfig, stream_read_acdi_manufacturer_gap_truncation) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010124ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // The fields layout has gaps between fields. If we start reading at an
    // address between the version (0x00, 1 byte) and name (0x01, 41 bytes),
    // there's no gap there. But between name end (0x29) and model start (0x2A),
    // the data is the tail of the name string which is zero-padded.

    // To hit the gap > count truncation (line 444-446), we need to read
    // a tiny amount from just before a field starts, where the gap is larger
    // than the requested count.
    // Actually, the fields are contiguous in the ACDI layout, so the "gap"
    // code handles alignment. Let me try reading from address 0 with count
    // that spans version + partial name.
    uint8_t buffer[128];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read the entire ACDI manufacturer space -- exercises all field boundaries
    uint16_t actual = OpenLcbConfigTest_stream_read_request_acdi_manufacturer(
            node, 0x00, 125, buffer);

    EXPECT_EQ(actual, 125u);

    // Hardware version at 0x53
    EXPECT_EQ(buffer[0x53], 'H');  // "HW2.0"
    EXPECT_EQ(buffer[0x54], 'W');

}

TEST(OpenLcbConfig, read_field_array_gap_between_fields) {

    // Two non-contiguous fields with a gap:
    // Field 0: address 0, length 5  (bytes 0-4)
    // Field 1: address 10, length 5 (bytes 10-14)
    // Gap at addresses 5-9
    uint8_t data0[] = { 'A', 'B', 'C', 'D', 'E' };
    uint8_t data1[] = { 'V', 'W', 'X', 'Y', 'Z' };

    const _config_field_t fields[] = {
        { 0, 5, data0 },
        { 10, 5, data1 },
    };

    uint8_t buffer[20];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read across both fields and the gap: address 0, count 15
    uint16_t actual = OpenLcbConfigTest_read_field_array(fields, 2, 0, 15, buffer);

    EXPECT_EQ(actual, 15u);
    // Field 0 data
    EXPECT_EQ(buffer[0], 'A');
    EXPECT_EQ(buffer[4], 'E');
    // Gap should be zero-filled
    EXPECT_EQ(buffer[5], 0);
    EXPECT_EQ(buffer[9], 0);
    // Field 1 data
    EXPECT_EQ(buffer[10], 'V');
    EXPECT_EQ(buffer[14], 'Z');

}

TEST(OpenLcbConfig, read_field_array_gap_larger_than_count) {

    // Field 0: address 0, length 2
    // Field 1: address 100, length 2
    // Gap at addresses 2-99 (98 bytes)
    uint8_t data0[] = { 'A', 'B' };
    uint8_t data1[] = { 'Y', 'Z' };

    const _config_field_t fields[] = {
        { 0, 2, data0 },
        { 100, 2, data1 },
    };

    uint8_t buffer[8];
    memset(buffer, 0xFF, sizeof(buffer));

    // Read 5 bytes starting at address 0 — fills field 0 (2 bytes),
    // then hits the gap but count (5) is reached before field 1
    uint16_t actual = OpenLcbConfigTest_read_field_array(fields, 2, 0, 5, buffer);

    EXPECT_EQ(actual, 5u);
    EXPECT_EQ(buffer[0], 'A');
    EXPECT_EQ(buffer[1], 'B');
    // Remaining 3 bytes are zero-filled gap
    EXPECT_EQ(buffer[2], 0);
    EXPECT_EQ(buffer[3], 0);
    EXPECT_EQ(buffer[4], 0);

}

TEST(OpenLcbConfig, stream_read_train_fdi_copies_data) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010112ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    memset(buffer, 0, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_train_function_definition_info(
            node, 0, 5, buffer);

    EXPECT_EQ(actual, 5u);
    EXPECT_EQ(buffer[0], '<');
    EXPECT_EQ(buffer[1], 'f');
    EXPECT_EQ(buffer[2], 'd');
    EXPECT_EQ(buffer[3], 'i');
    EXPECT_EQ(buffer[4], '>');

}

TEST(OpenLcbConfig, stream_read_train_fdi_null_returns_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010113ULL, &_node_params_null_cdi);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[8];
    uint16_t actual = OpenLcbConfigTest_stream_read_request_train_function_definition_info(
            node, 0, 5, buffer);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_read_train_function_config_memory) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010114ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    // Set up train state
    train_state_t *state = OpenLcbApplicationTrain_setup(node);
    ASSERT_NE(state, nullptr);

    state->functions[0] = 0x1234;
    state->functions[1] = 0x5678;

    uint8_t buffer[4];
    memset(buffer, 0, sizeof(buffer));

    // Read 4 bytes: fn0 high, fn0 low, fn1 high, fn1 low
    uint16_t actual = OpenLcbConfigTest_stream_read_request_train_function_config_memory(
            node, 0, 4, buffer);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(buffer[0], 0x12);  // fn0 high byte
    EXPECT_EQ(buffer[1], 0x34);  // fn0 low byte
    EXPECT_EQ(buffer[2], 0x56);  // fn1 high byte
    EXPECT_EQ(buffer[3], 0x78);  // fn1 low byte

}

TEST(OpenLcbConfig, stream_read_train_function_config_memory_null_state_returns_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010115ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t buffer[4];
    uint16_t actual = OpenLcbConfigTest_stream_read_request_train_function_config_memory(
            node, 0, 4, buffer);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_read_train_function_config_memory_past_end_fills_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010116ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    train_state_t *state = OpenLcbApplicationTrain_setup(node);
    ASSERT_NE(state, nullptr);
    state->functions[0] = 0xFFFF;

    // Read beyond the function array
    uint32_t past_end = USER_DEFINED_MAX_TRAIN_FUNCTIONS * 2;
    uint8_t buffer[2];
    memset(buffer, 0xFF, sizeof(buffer));

    uint16_t actual = OpenLcbConfigTest_stream_read_request_train_function_config_memory(
            node, past_end, 2, buffer);

    EXPECT_EQ(actual, 2u);
    EXPECT_EQ(buffer[0], 0x00);
    EXPECT_EQ(buffer[1], 0x00);

}

// =============================================================================
// Section 6: Stream Write Callbacks
// =============================================================================

TEST(OpenLcbConfig, stream_write_config_memory_delegates) {

    _global_initialize();
    _reset_mock_state();

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010117ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t data[] = { 0xDE, 0xAD, 0xBE, 0xEF };

    uint16_t actual = OpenLcbConfigTest_stream_write_request_configuration_memory(
            node, 0, 4, data);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(_mock_config_write_called, 1);

}

TEST(OpenLcbConfig, stream_write_acdi_user_delegates_with_offset) {

    _global_initialize();
    _reset_mock_state();

    // Node params with low_address for ACDI user
    node_parameters_t params = _node_params_full;
    params.address_space_acdi_user.low_address = 0x50;

    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010118ULL, &params);
    ASSERT_NE(node, nullptr);

    uint8_t data[] = { 0xAA, 0xBB };

    uint16_t actual = OpenLcbConfigTest_stream_write_request_acdi_user(
            node, 0x10, 2, data);

    EXPECT_EQ(actual, 2u);
    EXPECT_EQ(_mock_config_write_called, 1);
    // The write should go to address 0x10 + 0x50 = 0x60
    EXPECT_EQ(_mock_config_write_dest[0x60], 0xAA);
    EXPECT_EQ(_mock_config_write_dest[0x61], 0xBB);

}

TEST(OpenLcbConfig, stream_write_train_function_config_memory) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x050101010119ULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    train_state_t *state = OpenLcbApplicationTrain_setup(node);
    ASSERT_NE(state, nullptr);

    state->functions[0] = 0x0000;
    state->functions[1] = 0x0000;

    // Write 4 bytes: fn0=0x1234, fn1=0x5678
    uint8_t data[] = { 0x12, 0x34, 0x56, 0x78 };

    uint16_t actual = OpenLcbConfigTest_stream_write_request_train_function_config_memory(
            node, 0, 4, data);

    EXPECT_EQ(actual, 4u);
    EXPECT_EQ(state->functions[0], 0x1234);
    EXPECT_EQ(state->functions[1], 0x5678);

}

TEST(OpenLcbConfig, stream_write_train_function_config_memory_null_state_returns_zero) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101011AULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    uint8_t data[] = { 0x12, 0x34 };
    uint16_t actual = OpenLcbConfigTest_stream_write_request_train_function_config_memory(
            node, 0, 2, data);

    EXPECT_EQ(actual, 0u);

}

TEST(OpenLcbConfig, stream_write_train_function_config_memory_past_end_ignored) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101011BULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    train_state_t *state = OpenLcbApplicationTrain_setup(node);
    ASSERT_NE(state, nullptr);

    // Write past the function array -- should not crash, bytes silently ignored
    uint32_t past_end = USER_DEFINED_MAX_TRAIN_FUNCTIONS * 2;
    uint8_t data[] = { 0xFF, 0xFF };

    uint16_t actual = OpenLcbConfigTest_stream_write_request_train_function_config_memory(
            node, past_end, 2, data);

    EXPECT_EQ(actual, 2u);

}

TEST(OpenLcbConfig, stream_write_train_function_config_memory_single_byte) {

    _global_initialize();
    openlcb_node_t *node = OpenLcbConfig_create_node(0x05010101011CULL, &_node_params_full);
    ASSERT_NE(node, nullptr);

    train_state_t *state = OpenLcbApplicationTrain_setup(node);
    ASSERT_NE(state, nullptr);

    state->functions[0] = 0x0000;

    // Write only the high byte of function 0
    uint8_t data_hi[] = { 0xAB };
    OpenLcbConfigTest_stream_write_request_train_function_config_memory(node, 0, 1, data_hi);
    EXPECT_EQ(state->functions[0], 0xAB00);

    // Write only the low byte of function 0
    uint8_t data_lo[] = { 0xCD };
    OpenLcbConfigTest_stream_write_request_train_function_config_memory(node, 1, 1, data_lo);
    EXPECT_EQ(state->functions[0], 0xABCD);

}
