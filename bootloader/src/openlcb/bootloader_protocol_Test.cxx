/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_protocol_Test.cxx
 *
 * Unit tests for bootloader_protocol.c — PIP, SNIP, Verify Node ID,
 * Memory Config commands (FREEZE, WRITE, UNFREEZE, RESET).
 */

#include <gtest/gtest.h>
#include <cstring>

#include "bootloader_test_mocks.h"

extern "C" {

#include "bootloader.h"
#include "bootloader_protocol.h"
#include "bootloader_transport.h"

}

/* Helper: run enough transport loops to get through alias init. */
static void _init_bootloader(void) {

    mock_reset();
    mock_request_bootloader = true;
    Bootloader_init();

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderTransport_loop();

    }

    mock_tx_count = 0;  /* Clear init frames from TX queue. */

}

/* ====================================================================== */
/* Memory Config: FREEZE                                                   */
/* ====================================================================== */

TEST(BootloaderProtocol, freeze_sets_firmware_active) {

    _init_bootloader();

    uint8_t freeze_dg[] = {0x20, 0xA1, 0xEF};
    BootloaderProtocol_on_datagram_received(0x456, freeze_dg, 3);

    EXPECT_TRUE(bootloader_state.firmware_active);

}

TEST(BootloaderProtocol, freeze_wrong_space_rejected) {

    _init_bootloader();

    uint8_t freeze_dg[] = {0x20, 0xA1, 0xFD};
    BootloaderProtocol_on_datagram_received(0x456, freeze_dg, 3);

    EXPECT_FALSE(bootloader_state.firmware_active);

}

/* ====================================================================== */
/* Memory Config: WRITE                                                    */
/* ====================================================================== */

TEST(BootloaderProtocol, write_firmware_writes_flash) {

    _init_bootloader();

    /* Write datagram: cmd=0x20, subcmd=0x00, addr=0x00000000, space=0xEF, data */
    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
                          0xAA, 0xBB, 0xCC, 0xDD};
    BootloaderProtocol_on_datagram_received(0x456, write_dg, 11);

    EXPECT_EQ(mock_write_count, 1);

}

TEST(BootloaderProtocol, write_wrong_space_rejected) {

    _init_bootloader();

    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD,
                          0xAA, 0xBB};
    BootloaderProtocol_on_datagram_received(0x456, write_dg, 9);

    EXPECT_EQ(mock_write_count, 0);

}

/* ====================================================================== */
/* Memory Config: UNFREEZE                                                 */
/* ====================================================================== */

TEST(BootloaderProtocol, unfreeze_success_requests_reset) {

    _init_bootloader();
    bootloader_state.firmware_active = 1;
    mock_flash_complete_result = 0;

    uint8_t unfreeze_dg[] = {0x20, 0xA0, 0xEF};
    BootloaderProtocol_on_datagram_received(0x456, unfreeze_dg, 3);

    EXPECT_TRUE(bootloader_state.request_reset);
    EXPECT_FALSE(bootloader_state.firmware_active);

}

TEST(BootloaderProtocol, unfreeze_fail_rejected) {

    _init_bootloader();
    bootloader_state.firmware_active = 1;
    mock_flash_complete_result = 0x2088;

    uint8_t unfreeze_dg[] = {0x20, 0xA0, 0xEF};
    BootloaderProtocol_on_datagram_received(0x456, unfreeze_dg, 3);

    EXPECT_FALSE(bootloader_state.request_reset);

}

/* ====================================================================== */
/* Memory Config: RESET                                                    */
/* ====================================================================== */

TEST(BootloaderProtocol, reset_requests_reboot) {

    _init_bootloader();

    uint8_t reset_dg[] = {0x20, 0xA9};
    BootloaderProtocol_on_datagram_received(0x456, reset_dg, 2);

    EXPECT_TRUE(bootloader_state.request_reset);

}

/* Helper: run transport loops to flush pending datagram, then find
 * the datagram-only frame in mock_tx_queue. Returns pointer to
 * the frame or NULL. */
static bootloader_can_frame_t *_flush_and_find_datagram(void) {

    for (int i = 0; i < 10; i++) {

        BootloaderTransport_loop();

    }

    for (int i = 0; i < mock_tx_count; i++) {

        uint32_t frame_type = mock_tx_queue[i].can_id & 0x1E000000U;

        if (frame_type == 0x1A000000U ||   /* DATAGRAM_ONLY */
                frame_type == 0x1C000000U) {   /* DATAGRAM_FIRST (shouldn't happen for <=8) */

            return &mock_tx_queue[i];

        }

    }

    return NULL;

}

/* ====================================================================== */
/* Memory Config: GET OPTIONS                                              */
/* ====================================================================== */

TEST(BootloaderProtocol, get_options_reply_correct) {

    _init_bootloader();

    uint8_t options_dg[] = {0x20, 0x80};
    BootloaderProtocol_on_datagram_received(0x456, options_dg, 2);

    bootloader_can_frame_t *dg = _flush_and_find_datagram();
    ASSERT_NE(dg, nullptr);

    EXPECT_EQ(dg->can_dlc, 7);
    EXPECT_EQ(dg->data[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(dg->data[1], 0x82);  /* CONFIG_MEM_OPTIONS_REPLY */
    EXPECT_EQ(dg->data[2], 0x00);  /* Available commands high */
    EXPECT_EQ(dg->data[3], 0x00);  /* Available commands low */
    EXPECT_EQ(dg->data[4], 0xE2);  /* Write lengths: reserved bits set */
    EXPECT_EQ(dg->data[5], 0xEF);  /* Highest space = firmware */
    EXPECT_EQ(dg->data[6], 0xEF);  /* Lowest space = firmware */

}

/* ====================================================================== */
/* Memory Config: GET ADDRESS SPACE INFO                                   */
/* ====================================================================== */

TEST(BootloaderProtocol, get_address_space_info_firmware_present) {

    _init_bootloader();

    uint8_t info_dg[] = {0x20, 0x84, 0xEF};
    BootloaderProtocol_on_datagram_received(0x456, info_dg, 3);

    bootloader_can_frame_t *dg = _flush_and_find_datagram();
    ASSERT_NE(dg, nullptr);

    EXPECT_EQ(dg->can_dlc, 8);
    EXPECT_EQ(dg->data[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(dg->data[1], 0x87);  /* Present */
    EXPECT_EQ(dg->data[2], 0xEF);  /* Space */
    EXPECT_EQ(dg->data[3], 0xFF);  /* Highest address = 0xFFFFFFFF */
    EXPECT_EQ(dg->data[4], 0xFF);
    EXPECT_EQ(dg->data[5], 0xFF);
    EXPECT_EQ(dg->data[6], 0xFF);
    EXPECT_EQ(dg->data[7], 0x00);  /* Flags: writable, no low addr */

}

TEST(BootloaderProtocol, get_address_space_info_unknown_not_present) {

    _init_bootloader();

    uint8_t info_dg[] = {0x20, 0x84, 0xFD};
    BootloaderProtocol_on_datagram_received(0x456, info_dg, 3);

    bootloader_can_frame_t *dg = _flush_and_find_datagram();
    ASSERT_NE(dg, nullptr);

    EXPECT_EQ(dg->data[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(dg->data[1], 0x86);  /* Not present */
    EXPECT_EQ(dg->data[2], 0xFD);  /* Echoed space */
    EXPECT_EQ(dg->can_dlc, 3);

}

/* ====================================================================== */
/* Non-memconfig datagram rejected                                         */
/* ====================================================================== */

TEST(BootloaderProtocol, non_memconfig_datagram_rejected) {

    _init_bootloader();

    uint8_t unknown_dg[] = {0x99, 0x00};
    BootloaderProtocol_on_datagram_received(0x456, unknown_dg, 2);

    /* Should not crash. Protocol should send reject via transport. */
    EXPECT_FALSE(bootloader_state.request_reset);

}
