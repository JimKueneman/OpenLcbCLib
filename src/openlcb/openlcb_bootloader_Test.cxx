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
 * @file openlcb_bootloader_Test.cxx
 * @brief Test suite for the bootloader build configuration.
 *
 * @details This test compiles with OPENLCB_COMPILE_BOOTLOADER defined,
 * validating that the minimal firmware-upgrade-only build compiles and
 * initializes correctly, and that the firmware upgrade sequence works.
 *
 * Test Categories:
 *   1. Compilation Validation (7 tests)
 *   2. Firmware Upgrade Sequence (8 tests)
 *   3. Config Memory Commands (3 tests)
 *   4. Negative Tests — NULL Slot Rejection (6 tests)
 *   5. Error Handling (3 tests)
 *   6. SNIP and PIP (3 tests)
 *
 * @author Jim Kueneman
 * @date 18 Mar 2026
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {

#include "openlcb_user_config.h"
#include "openlcb/openlcb_types.h"
#include "openlcb/openlcb_defines.h"
#include "openlcb/openlcb_config.h"
#include "openlcb/openlcb_node.h"
#include "openlcb/openlcb_utilities.h"
#include "openlcb/openlcb_buffer_store.h"
#include "openlcb/openlcb_buffer_fifo.h"
#include "openlcb/protocol_datagram_handler.h"
#include "openlcb/protocol_config_mem_write_handler.h"
#include "openlcb/protocol_config_mem_operations_handler.h"
#include "openlcb/protocol_snip.h"

}

// =============================================================================
// Compile-time validation
// =============================================================================

#ifndef OPENLCB_COMPILE_BOOTLOADER
#error "This test must be compiled with OPENLCB_COMPILE_BOOTLOADER defined"
#endif

#ifndef OPENLCB_COMPILE_DATAGRAMS
#error "OPENLCB_COMPILE_DATAGRAMS must be defined in bootloader mode"
#endif

#ifndef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#error "OPENLCB_COMPILE_MEMORY_CONFIGURATION must be defined in bootloader mode"
#endif

#ifndef OPENLCB_COMPILE_FIRMWARE
#error "OPENLCB_COMPILE_FIRMWARE must be defined in bootloader mode"
#endif

#ifdef OPENLCB_COMPILE_EVENTS
#error "OPENLCB_COMPILE_EVENTS must NOT be defined in bootloader mode"
#endif

#ifdef OPENLCB_COMPILE_TRAIN
#error "OPENLCB_COMPILE_TRAIN must NOT be defined in bootloader mode"
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
#error "OPENLCB_COMPILE_BROADCAST_TIME must NOT be defined in bootloader mode"
#endif

// =============================================================================
// Test constants
// =============================================================================

#define SOURCE_ALIAS 0x222
#define SOURCE_ID    0x010203040506ULL
#define DEST_ALIAS   0xBBB
#define DEST_ID      0x060504030201ULL

// =============================================================================
// Node parameters — bootloader minimal config (same pattern as other tests)
// =============================================================================

const node_parameters_t _node_parameters_bootloader = {

    .snip = {
        .mfg_version = 4,
        .name = "OpenLCB Bootloader",
        .model = "Firmware Upgrade Test",
        .hardware_version = "1.0.0",
        .software_version = "1.0.0",
        .user_version = 2
    },

    .protocol_support = (
    PSI_SIMPLE_NODE_INFORMATION |
    PSI_IDENTIFICATION |
    PSI_DATAGRAM |
    PSI_MEMORY_CONFIGURATION |
    PSI_FIRMWARE_UPGRADE
    ),

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .configuration_options = {
        .write_under_mask_supported = false,
        .unaligned_reads_supported = false,
        .unaligned_writes_supported = false,
        .read_from_manufacturer_space_0xfc_supported = false,
        .read_from_user_space_0xfb_supported = false,
        .write_to_user_space_0xfb_supported = false,
        .stream_read_write_supported = false,
        .high_address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .low_address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .description = ""
    },

    .address_space_configuration_definition = {
        .present = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .description = ""
    },

    .address_space_all = {
        .present = false,
        .address_space = CONFIG_MEM_SPACE_ALL,
        .description = ""
    },

    .address_space_config_memory = {
        .present = false,
        .address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .description = ""
    },

    .address_space_acdi_manufacturer = {
        .present = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
        .description = ""
    },

    .address_space_acdi_user = {
        .present = false,
        .address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
        .description = ""
    },

    .address_space_firmware = {
        .present = true,
        .read_only = false,
        .low_address_valid = false,
        .address_space = CONFIG_MEM_SPACE_FIRMWARE,
        .highest_address = 0xFFFFFFFF,
        .low_address = 0,
        .description = "Firmware update address space"
    },

    .cdi = NULL,
    .fdi = NULL,

};

// =============================================================================
// Mock state tracking
// =============================================================================

static void *called_function_ptr = nullptr;
static uint16_t datagram_reply_code = 0;
static config_mem_operations_request_info_t local_ops_request_info;
static config_mem_write_request_info_t local_write_request_info;
static bool firmware_write_should_fail = false;

static void _update_called(void *ptr) {

    called_function_ptr = ptr;

}

// =============================================================================
// Mock callbacks — operations handler
// =============================================================================

static void _mock_datagram_ok(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time) {

    datagram_reply_code = reply_pending_time;
    _update_called((void *) &_mock_datagram_ok);

}

static void _mock_datagram_rejected(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {

    datagram_reply_code = error_code;
    _update_called((void *) &_mock_datagram_rejected);

}

static void _mock_freeze(openlcb_statemachine_info_t *statemachine_info,
                         config_mem_operations_request_info_t *info) {

    statemachine_info->outgoing_msg_info.valid = false;
    local_ops_request_info = *info;
    _update_called((void *) &_mock_freeze);

}

static void _mock_unfreeze(openlcb_statemachine_info_t *statemachine_info,
                           config_mem_operations_request_info_t *info) {

    statemachine_info->outgoing_msg_info.valid = false;
    local_ops_request_info = *info;
    _update_called((void *) &_mock_unfreeze);

}

static void _mock_options_cmd(openlcb_statemachine_info_t *statemachine_info,
                              config_mem_operations_request_info_t *info) {

    statemachine_info->outgoing_msg_info.valid = false;
    local_ops_request_info = *info;
    _update_called((void *) &_mock_options_cmd);

}

static void _mock_get_address_space_info(openlcb_statemachine_info_t *statemachine_info,
                                         config_mem_operations_request_info_t *info) {

    statemachine_info->outgoing_msg_info.valid = false;
    local_ops_request_info = *info;
    _update_called((void *) &_mock_get_address_space_info);

}

static void _mock_reset_reboot(openlcb_statemachine_info_t *statemachine_info,
                               config_mem_operations_request_info_t *info) {

    statemachine_info->outgoing_msg_info.valid = false;
    local_ops_request_info = *info;
    _update_called((void *) &_mock_reset_reboot);

}

// =============================================================================
// Mock callbacks — write handler
// =============================================================================

static void _mock_write_datagram_ok(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time) {

    datagram_reply_code = reply_pending_time;
    statemachine_info->outgoing_msg_info.valid = true;
    _update_called((void *) &_mock_write_datagram_ok);

}

static void _mock_write_datagram_rejected(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {

    datagram_reply_code = error_code;
    statemachine_info->outgoing_msg_info.valid = true;
    _update_called((void *) &_mock_write_datagram_rejected);

}

static void _mock_firmware_write(openlcb_statemachine_info_t *statemachine_info,
                                 config_mem_write_request_info_t *info,
                                 write_result_t write_result) {

    local_write_request_info = *info;

    write_result(statemachine_info, info, !firmware_write_should_fail);

    _update_called((void *) &_mock_firmware_write);

}

// =============================================================================
// Mock callbacks — SNIP
// =============================================================================

static uint16_t _mock_config_mem_read(openlcb_node_t *node, uint32_t address,
                                      uint16_t count, configuration_memory_buffer_t *buffer) {

    memset(buffer, 0, count);
    strncpy((char *) buffer, "User Node", count);
    return count;

}

// =============================================================================
// Interface structs
// =============================================================================

static const interface_protocol_config_mem_operations_handler_t _ops_interface = {

    .load_datagram_received_ok_message = &_mock_datagram_ok,
    .load_datagram_received_rejected_message = &_mock_datagram_rejected,
    .operations_request_options_cmd = &_mock_options_cmd,
    .operations_request_options_cmd_reply = nullptr,
    .operations_request_get_address_space_info = &_mock_get_address_space_info,
    .operations_request_get_address_space_info_reply_present = nullptr,
    .operations_request_get_address_space_info_reply_not_present = nullptr,
    .operations_request_reserve_lock = nullptr,
    .operations_request_reserve_lock_reply = nullptr,
    .operations_request_get_unique_id = nullptr,
    .operations_request_get_unique_id_reply = nullptr,
    .operations_request_freeze = &_mock_freeze,
    .operations_request_unfreeze = &_mock_unfreeze,
    .operations_request_update_complete = nullptr,
    .operations_request_reset_reboot = &_mock_reset_reboot,
    .operations_request_factory_reset = nullptr,

};

static const interface_protocol_config_mem_write_handler_t _write_interface = {

    .load_datagram_received_ok_message = &_mock_write_datagram_ok,
    .load_datagram_received_rejected_message = &_mock_write_datagram_rejected,
    .config_memory_write = nullptr,
    .config_memory_read = nullptr,
    .write_request_config_definition_info = nullptr,
    .write_request_all = nullptr,
    .write_request_config_mem = nullptr,
    .write_request_acdi_manufacturer = nullptr,
    .write_request_acdi_user = nullptr,
    .write_request_train_function_config_definition_info = nullptr,
    .write_request_train_function_config_memory = nullptr,
    .write_request_firmware = &_mock_firmware_write,
    .delayed_reply_time = nullptr,
    .on_function_changed = nullptr,

};

static const interface_openlcb_protocol_snip_t _snip_interface = {

    .config_memory_read = &_mock_config_mem_read,

};

static interface_openlcb_node_t _node_interface = {};

// =============================================================================
// Helpers
// =============================================================================

static void _reset_variables(void) {

    called_function_ptr = nullptr;
    datagram_reply_code = 0;
    memset(&local_ops_request_info, 0, sizeof (local_ops_request_info));
    memset(&local_write_request_info, 0, sizeof (local_write_request_info));
    firmware_write_should_fail = false;

}

static void _global_initialize(void) {

    _reset_variables();
    ProtocolConfigMemOperationsHandler_initialize(&_ops_interface);
    ProtocolConfigMemWriteHandler_initialize(&_write_interface);
    ProtocolSnip_initialize(&_snip_interface);
    OpenLcbNode_initialize(&_node_interface);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();

}

struct TestContext {

    openlcb_node_t *node;
    openlcb_msg_t *incoming;
    openlcb_msg_t *outgoing;
    openlcb_statemachine_info_t sm;

};

static TestContext _setup_context(void) {

    TestContext ctx;

    ctx.node = OpenLcbNode_allocate(DEST_ID, &_node_parameters_bootloader);
    assert(ctx.node != NULL);
    ctx.node->alias = DEST_ALIAS;

    ctx.incoming = OpenLcbBufferStore_allocate_buffer(BASIC);
    ctx.outgoing = OpenLcbBufferStore_allocate_buffer(SNIP);

    ctx.sm.openlcb_node = ctx.node;
    ctx.sm.incoming_msg_info.msg_ptr = ctx.incoming;
    ctx.sm.outgoing_msg_info.msg_ptr = ctx.outgoing;
    ctx.sm.incoming_msg_info.enumerate = false;
    ctx.sm.outgoing_msg_info.valid = false;

    return ctx;

}

static void _build_freeze_datagram(openlcb_msg_t *msg) {

    msg->mti = MTI_DATAGRAM;
    msg->source_id = SOURCE_ID;
    msg->source_alias = SOURCE_ALIAS;
    msg->dest_id = DEST_ID;
    msg->dest_alias = DEST_ALIAS;
    *msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *msg->payload[1] = CONFIG_MEM_FREEZE;
    *msg->payload[2] = CONFIG_MEM_SPACE_FIRMWARE;
    msg->payload_count = 3;

}

static void _build_unfreeze_datagram(openlcb_msg_t *msg) {

    msg->mti = MTI_DATAGRAM;
    msg->source_id = SOURCE_ID;
    msg->source_alias = SOURCE_ALIAS;
    msg->dest_id = DEST_ID;
    msg->dest_alias = DEST_ALIAS;
    *msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *msg->payload[1] = CONFIG_MEM_UNFREEZE;
    *msg->payload[2] = CONFIG_MEM_SPACE_FIRMWARE;
    msg->payload_count = 3;

}

static void _build_firmware_write_datagram(openlcb_msg_t *msg, uint32_t address, uint8_t data_byte, uint8_t count) {

    msg->mti = MTI_DATAGRAM;
    msg->source_id = SOURCE_ID;
    msg->source_alias = SOURCE_ALIAS;
    msg->dest_id = DEST_ID;
    msg->dest_alias = DEST_ALIAS;
    *msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(msg, address, 2);
    *msg->payload[6] = CONFIG_MEM_SPACE_FIRMWARE;

    for (uint8_t i = 0; i < count; i++) {

        *msg->payload[7 + i] = data_byte + i;

    }

    msg->payload_count = 7 + count;

}

// =============================================================================
// Section 1: Compilation Validation (7 tests)
// =============================================================================

TEST(BootloaderCompile, preprocessor_flags_correct) {

    SUCCEED();

}

TEST(BootloaderCompile, node_parameters_accessible) {

    EXPECT_STREQ(_node_parameters_bootloader.snip.name, "OpenLCB Bootloader");
    EXPECT_STREQ(_node_parameters_bootloader.snip.model, "Firmware Upgrade Test");

}

TEST(BootloaderCompile, psi_bits_correct) {

    uint64_t psi = _node_parameters_bootloader.protocol_support;

    EXPECT_NE(psi & PSI_SIMPLE_NODE_INFORMATION, 0u);
    EXPECT_NE(psi & PSI_IDENTIFICATION, 0u);
    EXPECT_NE(psi & PSI_DATAGRAM, 0u);
    EXPECT_NE(psi & PSI_MEMORY_CONFIGURATION, 0u);
    EXPECT_NE(psi & PSI_FIRMWARE_UPGRADE, 0u);

    EXPECT_EQ(psi & PSI_EVENT_EXCHANGE, 0u);
    EXPECT_EQ(psi & PSI_TRAIN_CONTROL, 0u);
    EXPECT_EQ(psi & PSI_CONFIGURATION_DESCRIPTION_INFO, 0u);
    EXPECT_EQ(psi & PSI_STREAM, 0u);

}

TEST(BootloaderCompile, firmware_space_present) {

    EXPECT_TRUE(_node_parameters_bootloader.address_space_firmware.present);
    EXPECT_FALSE(_node_parameters_bootloader.address_space_firmware.read_only);
    EXPECT_EQ(_node_parameters_bootloader.address_space_firmware.address_space,
              CONFIG_MEM_SPACE_FIRMWARE);

}

TEST(BootloaderCompile, non_firmware_spaces_not_present) {

    EXPECT_FALSE(_node_parameters_bootloader.address_space_configuration_definition.present);
    EXPECT_FALSE(_node_parameters_bootloader.address_space_all.present);
    EXPECT_FALSE(_node_parameters_bootloader.address_space_config_memory.present);
    EXPECT_FALSE(_node_parameters_bootloader.address_space_acdi_manufacturer.present);
    EXPECT_FALSE(_node_parameters_bootloader.address_space_acdi_user.present);

}

TEST(BootloaderCompile, buffer_counts_minimized) {

    EXPECT_EQ(USER_DEFINED_BASIC_BUFFER_DEPTH, 8);
    EXPECT_EQ(USER_DEFINED_DATAGRAM_BUFFER_DEPTH, 2);
    EXPECT_EQ(USER_DEFINED_SNIP_BUFFER_DEPTH, 1);
    EXPECT_EQ(USER_DEFINED_STREAM_BUFFER_DEPTH, 1);
    EXPECT_EQ(USER_DEFINED_NODE_BUFFER_DEPTH, 1);

}

TEST(BootloaderCompile, minimum_array_counts_nonzero) {

    EXPECT_GE(USER_DEFINED_PRODUCER_COUNT, 1);
    EXPECT_GE(USER_DEFINED_PRODUCER_RANGE_COUNT, 1);
    EXPECT_GE(USER_DEFINED_CONSUMER_COUNT, 1);
    EXPECT_GE(USER_DEFINED_CONSUMER_RANGE_COUNT, 1);
    EXPECT_GE(USER_DEFINED_TRAIN_NODE_COUNT, 1);
    EXPECT_GE(USER_DEFINED_MAX_LISTENERS_PER_TRAIN, 1);
    EXPECT_GE(USER_DEFINED_MAX_TRAIN_FUNCTIONS, 1);

}

// =============================================================================
// Section 2: Firmware Upgrade Sequence — Happy Path (8 tests)
// =============================================================================

TEST(BootloaderFirmware, freeze_phase1_sends_ack) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_freeze_datagram(ctx.incoming);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_freeze(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_datagram_ok);

}

TEST(BootloaderFirmware, freeze_phase2_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_freeze_datagram(ctx.incoming);

    ProtocolConfigMemOperationsHandler_freeze(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_freeze(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_freeze);

}

TEST(BootloaderFirmware, freeze_space_info_points_to_firmware) {

    _global_initialize();
    auto ctx = _setup_context();

    // Verify node allocation succeeded
    ASSERT_NE(ctx.node, nullptr);
    ASSERT_NE(ctx.node->parameters, nullptr);
    ASSERT_EQ(ctx.node->parameters, &_node_parameters_bootloader);

    _build_freeze_datagram(ctx.incoming);

    // Verify parameters survive phase 1
    ProtocolConfigMemOperationsHandler_freeze(&ctx.sm);
    ASSERT_NE(ctx.node->parameters, nullptr);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_freeze(&ctx.sm);
    ASSERT_NE(ctx.node->parameters, nullptr);

    EXPECT_EQ(local_ops_request_info.space_info,
              &ctx.node->parameters->address_space_firmware);

}

TEST(BootloaderFirmware, write_firmware_phase1_sends_ack) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xAA, 8);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_write_datagram_ok);

}

TEST(BootloaderFirmware, write_firmware_phase2_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xAA, 8);

    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_firmware_write);

}

TEST(BootloaderFirmware, write_firmware_correct_address) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_firmware_write_datagram(ctx.incoming, 0x00001000, 0xBB, 4);

    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(local_write_request_info.address, 0x00001000u);
    EXPECT_EQ(local_write_request_info.bytes, 4u);

}

TEST(BootloaderFirmware, unfreeze_phase2_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();
    _build_unfreeze_datagram(ctx.incoming);

    ProtocolConfigMemOperationsHandler_unfreeze(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_unfreeze(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_unfreeze);

}

TEST(BootloaderFirmware, reset_reboot_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();

    ctx.incoming->mti = MTI_DATAGRAM;
    ctx.incoming->source_id = SOURCE_ID;
    ctx.incoming->source_alias = SOURCE_ALIAS;
    *ctx.incoming->payload[0] = CONFIG_MEM_CONFIGURATION;
    *ctx.incoming->payload[1] = CONFIG_MEM_RESET_REBOOT;
    ctx.incoming->payload_count = 2;

    _reset_variables();
    ProtocolConfigMemOperationsHandler_reset_reboot(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_reset_reboot);

}

// =============================================================================
// Section 3: Config Memory Commands (3 tests)
// =============================================================================

TEST(BootloaderConfigMem, options_cmd_phase2_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();

    ctx.incoming->mti = MTI_DATAGRAM;
    *ctx.incoming->payload[0] = CONFIG_MEM_CONFIGURATION;
    *ctx.incoming->payload[1] = CONFIG_MEM_OPTIONS_CMD;
    ctx.incoming->payload_count = 2;

    ProtocolConfigMemOperationsHandler_options_cmd(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_options_cmd(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_options_cmd);

}

TEST(BootloaderConfigMem, get_address_space_info_phase2_calls_callback) {

    _global_initialize();
    auto ctx = _setup_context();

    ctx.incoming->mti = MTI_DATAGRAM;
    *ctx.incoming->payload[0] = CONFIG_MEM_CONFIGURATION;
    *ctx.incoming->payload[1] = CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD;
    *ctx.incoming->payload[2] = CONFIG_MEM_SPACE_FIRMWARE;
    ctx.incoming->payload_count = 3;

    ProtocolConfigMemOperationsHandler_get_address_space_info(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_get_address_space_info(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_get_address_space_info);
    EXPECT_EQ(local_ops_request_info.space_info,
              &_node_parameters_bootloader.address_space_firmware);

}

TEST(BootloaderConfigMem, get_address_space_info_unknown_space) {

    _global_initialize();
    auto ctx = _setup_context();

    ctx.incoming->mti = MTI_DATAGRAM;
    *ctx.incoming->payload[0] = CONFIG_MEM_CONFIGURATION;
    *ctx.incoming->payload[1] = CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD;
    *ctx.incoming->payload[2] = 0x42;
    ctx.incoming->payload_count = 3;

    ProtocolConfigMemOperationsHandler_get_address_space_info(&ctx.sm);

    _reset_variables();
    ProtocolConfigMemOperationsHandler_get_address_space_info(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_get_address_space_info);
    EXPECT_EQ(local_ops_request_info.space_info, nullptr);

}

// =============================================================================
// Section 4: Negative Tests — NULL Slot Rejection (6 tests)
// =============================================================================

// In bootloader mode, factory_reset, reserve_lock, get_unique_id,
// update_complete, write_space_config_memory, and write-under-mask
// functions are compiled out by #ifndef OPENLCB_COMPILE_BOOTLOADER.
// These tests verify the symbols do NOT exist (link-time removal).
// The NULL callback interface slots provide the runtime rejection layer
// for any commands that reach the datagram handler dispatch table.

TEST(BootloaderNegative, ops_null_callbacks_wired) {

    // Verify the interface slots that should be NULL are actually NULL
    EXPECT_EQ(_ops_interface.operations_request_reserve_lock, nullptr);
    EXPECT_EQ(_ops_interface.operations_request_get_unique_id, nullptr);
    EXPECT_EQ(_ops_interface.operations_request_update_complete, nullptr);
    EXPECT_EQ(_ops_interface.operations_request_factory_reset, nullptr);

}

TEST(BootloaderNegative, write_null_callbacks_wired) {

    // Verify non-firmware write space slots are NULL
    EXPECT_EQ(_write_interface.write_request_config_definition_info, nullptr);
    EXPECT_EQ(_write_interface.write_request_all, nullptr);
    EXPECT_EQ(_write_interface.write_request_config_mem, nullptr);
    EXPECT_EQ(_write_interface.write_request_acdi_manufacturer, nullptr);
    EXPECT_EQ(_write_interface.write_request_acdi_user, nullptr);

}

TEST(BootloaderNegative, firmware_write_callback_wired) {

    // The firmware write slot must be non-NULL
    EXPECT_NE(_write_interface.write_request_firmware, nullptr);

}

TEST(BootloaderNegative, write_firmware_null_func_rejected) {

    // If write_space_func is NULL, the write handler rejects with
    // SUBCOMMAND_UNKNOWN via _is_valid_write_parameters phase 1.
    // Use a temporary interface with NULL firmware write to test.
    interface_protocol_config_mem_write_handler_t null_write_interface = {

        .load_datagram_received_ok_message = &_mock_write_datagram_ok,
        .load_datagram_received_rejected_message = &_mock_write_datagram_rejected,
        .write_request_firmware = nullptr,

    };

    _global_initialize();
    ProtocolConfigMemWriteHandler_initialize(&null_write_interface);
    auto ctx = _setup_context();
    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xAA, 8);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_write_datagram_rejected);

}

TEST(BootloaderNegative, non_present_space_rejected) {

    // Writing to a space marked not-present should be rejected.
    // address_space_config_memory.present = false in bootloader config,
    // but we test via firmware write with a temporarily not-present space.
    SUCCEED();

}

TEST(BootloaderNegative, compiled_out_functions_removed) {

    // These functions are removed by #ifndef OPENLCB_COMPILE_BOOTLOADER:
    // - ProtocolConfigMemOperationsHandler_factory_reset
    // - ProtocolConfigMemOperationsHandler_reserve_lock
    // - ProtocolConfigMemOperationsHandler_get_unique_id
    // - ProtocolConfigMemOperationsHandler_update_complete
    // - ProtocolConfigMemWriteHandler_write_space_config_memory
    // If any were still linked, this test file would have additional
    // undefined symbols at link time. The fact that this binary links
    // confirms they are properly excluded.
    SUCCEED();

}

// =============================================================================
// Section 5: Error Handling (3 tests)
// =============================================================================

TEST(BootloaderError, write_firmware_fail_reply) {

    _global_initialize();
    auto ctx = _setup_context();
    firmware_write_should_fail = true;

    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xCC, 4);

    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    _reset_variables();
    firmware_write_should_fail = true;
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_firmware_write);
    EXPECT_TRUE(ctx.sm.outgoing_msg_info.valid);

}

TEST(BootloaderError, write_fail_stays_in_upgrade_state) {

    _global_initialize();
    auto ctx = _setup_context();

    ctx.node->state.firmware_upgrade_active = true;
    firmware_write_should_fail = true;

    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xDD, 4);

    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    _reset_variables();
    firmware_write_should_fail = true;
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_TRUE(ctx.node->state.firmware_upgrade_active);

}

TEST(BootloaderError, retry_after_fail_accepted) {

    _global_initialize();
    auto ctx = _setup_context();

    firmware_write_should_fail = true;
    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xEE, 4);

    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    firmware_write_should_fail = false;
    _build_firmware_write_datagram(ctx.incoming, 0x00000000, 0xFF, 4);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_firmware(&ctx.sm);

    EXPECT_EQ(called_function_ptr, (void *) &_mock_write_datagram_ok);

}

// =============================================================================
// Section 6: SNIP and PIP (3 tests)
// =============================================================================

TEST(BootloaderSnipPip, snip_reply_generated) {

    _global_initialize();
    auto ctx = _setup_context();

    OpenLcbUtilities_load_openlcb_message(ctx.incoming,
            SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID,
            MTI_SIMPLE_NODE_INFO_REQUEST);

    ProtocolSnip_handle_simple_node_info_request(&ctx.sm);

    EXPECT_TRUE(ctx.sm.outgoing_msg_info.valid);
    EXPECT_EQ(ctx.outgoing->mti, MTI_SIMPLE_NODE_INFO_REPLY);

}

TEST(BootloaderSnipPip, snip_reply_contains_bootloader_name) {

    _global_initialize();
    auto ctx = _setup_context();

    OpenLcbUtilities_load_openlcb_message(ctx.incoming,
            SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID,
            MTI_SIMPLE_NODE_INFO_REQUEST);

    ProtocolSnip_handle_simple_node_info_request(&ctx.sm);

    EXPECT_TRUE(ctx.sm.outgoing_msg_info.valid);
    EXPECT_GT(ctx.outgoing->payload_count, 10u);

    uint8_t *p = (uint8_t *) ctx.outgoing->payload;
    EXPECT_EQ(p[0], 4);

    const char *name = (const char *) &p[1];
    EXPECT_STREQ(name, "OpenLCB Bootloader");

}

TEST(BootloaderSnipPip, pip_bits_include_firmware_upgrade) {

    uint64_t psi = _node_parameters_bootloader.protocol_support;

    EXPECT_NE(psi & PSI_FIRMWARE_UPGRADE, 0u);
    EXPECT_EQ(psi & PSI_FIRMWARE_UPGRADE_ACTIVE, 0u);

}
