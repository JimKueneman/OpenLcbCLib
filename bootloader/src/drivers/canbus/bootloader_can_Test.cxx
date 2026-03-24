/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_can_Test.cxx
 *
 * Unit tests for bootloader_can.c — alias negotiation, CAN frame dispatch,
 * datagram assembly.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "bootloader_test_mocks.h"

extern "C" {

#include "bootloader.h"
#include "bootloader_transport.h"

}

/* Helper: initialize and run until alias negotiation completes. */
static void _run_until_initialized(void) {

    mock_reset();
    mock_request_bootloader = true;
    Bootloader_init();

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderTransport_loop();

    }

}

/* ====================================================================== */
/* Alias Initialization                                                    */
/* ====================================================================== */

TEST(BootloaderCan, init_completes_with_alias) {

    _run_until_initialized();

    EXPECT_TRUE(BootloaderTransport_is_initialized());
    EXPECT_NE(BootloaderCan_get_alias(), 0);

}

TEST(BootloaderCan, init_uses_provided_alias) {

    mock_reset();
    mock_alias = 0x456;
    mock_request_bootloader = true;
    Bootloader_init();

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderTransport_loop();

    }

    EXPECT_EQ(BootloaderCan_get_alias(), 0x456);

}

TEST(BootloaderCan, init_auto_generates_alias) {

    mock_reset();
    mock_alias = 0;  /* Force auto-generation. */
    mock_request_bootloader = true;
    Bootloader_init();

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderTransport_loop();

    }

    EXPECT_TRUE(BootloaderTransport_is_initialized());
    EXPECT_NE(BootloaderCan_get_alias(), 0);

}

TEST(BootloaderCan, init_sends_cid_rid_amd_init_complete) {

    mock_reset();
    mock_alias = 0x123;
    mock_request_bootloader = true;
    Bootloader_init();

    /* Run enough loops to complete init. */
    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderTransport_loop();

    }

    /* Should have sent at least: CID7, CID6, CID5, CID4, RID, AMD, InitComplete = 7 frames. */
    EXPECT_GE(mock_tx_count, 7);

}

/* ====================================================================== */
/* Transport send functions                                                */
/* ====================================================================== */

TEST(BootloaderCan, send_datagram_ok) {

    _run_until_initialized();
    mock_tx_count = 0;

    bool result = BootloaderTransport_send_datagram_ok(0x456);
    EXPECT_TRUE(result);
    EXPECT_EQ(mock_tx_count, 0);

    /* Frame is in the output buffer — send it on next loop. */
    BootloaderTransport_loop();
    EXPECT_GE(mock_tx_count, 1);

}

TEST(BootloaderCan, send_datagram_rejected) {

    _run_until_initialized();
    mock_tx_count = 0;

    bool result = BootloaderTransport_send_datagram_rejected(0x456, 0x1081);
    EXPECT_TRUE(result);

    BootloaderTransport_loop();
    EXPECT_GE(mock_tx_count, 1);

}
