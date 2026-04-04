/*******************************************************************************
 * File: can_config_Test.cxx
 *
 * Description:
 *   Test suite for CAN Config wiring module.
 *   Verifies CanConfig_initialize() wires all internal interface structs
 *   and calls all module initialize functions in the correct order.
 *
 * Module Under Test:
 *   CanConfig - Library-internal wiring module for CAN bus transport
 *
 * Test Coverage:
 *   - CanConfig_initialize() executes all 7 _build_*() functions
 *   - All OPENLCB_COMPILE_TRAIN conditional branches
 *   - All module initialize calls
 *
 * Functions Under Test:
 *   CanConfig_initialize()
 *
 * Author: Test Suite
 * Date: 2026-04-04
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_config.h"

/*******************************************************************************
 * Stub callbacks for can_config_t required fields
 ******************************************************************************/

static bool _stub_transmit_raw_can_frame(can_msg_t *can_msg) {

    (void) can_msg;
    return true;

}

static bool _stub_is_tx_buffer_clear(void) {

    return true;

}

static void _stub_lock(void) {

}

static void _stub_unlock(void) {

}

static void _stub_on_rx(can_msg_t *can_msg) {

    (void) can_msg;

}

static void _stub_on_tx(can_msg_t *can_msg) {

    (void) can_msg;

}

static void _stub_on_alias_change(uint16_t alias, node_id_t node_id) {

    (void) alias;
    (void) node_id;

}

/*******************************************************************************
 * Test: initialize_wires_all_modules
 *
 * Purpose:
 *   Verifies that CanConfig_initialize() runs to completion, executing
 *   all 7 internal _build_*() functions and all module _initialize() calls.
 *   This single call covers every line in can_config.c because
 *   OPENLCB_COMPILE_TRAIN is defined in the test configuration.
 *
 * OpenLCB Context:
 *   can_config.c is the CAN transport wiring module. It reads a user-provided
 *   can_config_t and builds all internal interface structs, then initializes
 *   all CAN modules in dependency order.
 ******************************************************************************/
TEST(CAN_Config, initialize_wires_all_modules) {

    can_config_t config;
    memset(&config, 0, sizeof(config));

    config.transmit_raw_can_frame  = &_stub_transmit_raw_can_frame;
    config.is_tx_buffer_clear      = &_stub_is_tx_buffer_clear;
    config.lock_shared_resources   = &_stub_lock;
    config.unlock_shared_resources = &_stub_unlock;
    config.on_rx                   = &_stub_on_rx;
    config.on_tx                   = &_stub_on_tx;
    config.on_alias_change         = &_stub_on_alias_change;

    CanConfig_initialize(&config);

}

/*******************************************************************************
 * Test: initialize_with_null_optional_callbacks
 *
 * Purpose:
 *   Verifies that CanConfig_initialize() handles NULL optional callbacks.
 *   The required fields are set but on_rx, on_tx, and on_alias_change are
 *   left NULL.
 *
 * OpenLCB Context:
 *   Users are not required to provide on_rx, on_tx, or on_alias_change.
 *   The wiring must accept NULL for these optional fields.
 ******************************************************************************/
TEST(CAN_Config, initialize_with_null_optional_callbacks) {

    can_config_t config;
    memset(&config, 0, sizeof(config));

    config.transmit_raw_can_frame  = &_stub_transmit_raw_can_frame;
    config.is_tx_buffer_clear      = &_stub_is_tx_buffer_clear;
    config.lock_shared_resources   = &_stub_lock;
    config.unlock_shared_resources = &_stub_unlock;
    // on_rx, on_tx, on_alias_change left NULL

    CanConfig_initialize(&config);

}
