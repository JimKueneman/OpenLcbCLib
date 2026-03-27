/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_can_Test.cxx
 *
 * Unit tests for bootloader_can_statemachine.c -- alias negotiation,
 * CAN frame dispatch, send helpers.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "bootloader_test_mocks.h"

extern "C" {

#include "bootloader.h"
#include "bootloader_can_statemachine.h"

}

/* Helper: initialize and run until alias negotiation completes. */
static void _run_until_initialized(void) {

    mock_reset();
    mock_request_bootloader = BOOTLOADER_REQUESTED_BY_BUTTON;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver, mock_request_bootloader);

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderCanSM_loop();

    }

}

/* ====================================================================== */
/* Alias Initialization                                                    */
/* ====================================================================== */

TEST(BootloaderCan, init_completes_with_alias) {

    _run_until_initialized();

    EXPECT_TRUE(BootloaderCanSM_is_initialized());
    EXPECT_NE(BootloaderCanSM_alias(), 0);

}

TEST(BootloaderCan, init_uses_provided_alias) {

    mock_reset();
    mock_alias = 0x456;
    mock_request_bootloader = BOOTLOADER_REQUESTED_BY_BUTTON;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver, mock_request_bootloader);

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderCanSM_loop();

    }

    EXPECT_EQ(BootloaderCanSM_alias(), 0x456);

}

TEST(BootloaderCan, init_auto_generates_alias) {

    mock_reset();
    mock_alias = 0;  /* Force auto-generation. */
    mock_request_bootloader = BOOTLOADER_REQUESTED_BY_BUTTON;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver, mock_request_bootloader);

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderCanSM_loop();

    }

    EXPECT_TRUE(BootloaderCanSM_is_initialized());
    EXPECT_NE(BootloaderCanSM_alias(), 0);

}

TEST(BootloaderCan, init_sends_cid_rid_amd_init_complete) {

    mock_reset();
    mock_alias = 0;
    mock_request_bootloader = BOOTLOADER_REQUESTED_BY_BUTTON;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver, mock_request_bootloader);

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderCanSM_loop();

    }

    /* Should have sent at least: CID7, CID6, CID5, CID4, RID, AMD, InitComplete = 7 frames. */
    EXPECT_GE(mock_tx_count, 7);

}

/* ====================================================================== */
/* Send helpers                                                            */
/* ====================================================================== */

TEST(BootloaderCan, send_datagram_ok) {

    _run_until_initialized();
    mock_tx_count = 0;

    /* New architecture: sends inline (blocking). */
    bool result = BootloaderCanSM_send_datagram_ok(0x456, 0);
    EXPECT_TRUE(result);
    EXPECT_GE(mock_tx_count, 1);

}

TEST(BootloaderCan, send_datagram_rejected) {

    _run_until_initialized();
    mock_tx_count = 0;

    bool result = BootloaderCanSM_send_datagram_rejected(0x456, 0, 0x1081);
    EXPECT_TRUE(result);
    EXPECT_GE(mock_tx_count, 1);

}
