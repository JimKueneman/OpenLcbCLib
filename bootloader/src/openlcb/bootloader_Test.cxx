/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_Test.cxx
 *
 * Unit tests for bootloader.c — boot decision, checksum validation, main loop.
 */

#include <gtest/gtest.h>
#include <cstring>

#include "bootloader_test_mocks.h"

extern "C" {

#include "bootloader.h"
#include "bootloader_crc.h"

}

TEST(BootloaderInit, bootloader_requested_enters_bootloader) {

    mock_reset();
    mock_request_bootloader = true;

    bool result = Bootloader_init(&mock_can_driver, &mock_openlcb_driver);
    EXPECT_FALSE(result);
    EXPECT_FALSE(mock_application_entered);

}

TEST(BootloaderInit, valid_checksum_enters_application) {

    mock_reset();
    mock_request_bootloader = false;

    /* Set up a valid app_header with correct checksums. */
    bootloader_app_header_t *header =
            (bootloader_app_header_t *) (mock_flash + MOCK_APP_HEADER_OFFSET);
    header->app_size = MOCK_FLASH_SIZE;

    /* Compute pre-checksum (flash[0] to flash[0x100]). */
    uint16_t pre_crc[3];
    BootloaderCrc_crc3_crc16_ibm(mock_flash, MOCK_APP_HEADER_OFFSET, pre_crc);
    header->checksum_pre[0] = pre_crc[0];
    header->checksum_pre[1] = pre_crc[1];
    header->checksum_pre[2] = pre_crc[2];
    header->checksum_pre[3] = 0;

    /* Compute post-checksum (after header to end). */
    uint32_t post_offset = MOCK_APP_HEADER_OFFSET + sizeof(bootloader_app_header_t);
    uint32_t post_size = MOCK_FLASH_SIZE - post_offset;
    uint16_t post_crc[3];
    BootloaderCrc_crc3_crc16_ibm(mock_flash + post_offset, post_size, post_crc);
    header->checksum_post[0] = post_crc[0];
    header->checksum_post[1] = post_crc[1];
    header->checksum_post[2] = post_crc[2];
    header->checksum_post[3] = 0;

    bool result = Bootloader_init(&mock_can_driver, &mock_openlcb_driver);
    EXPECT_TRUE(result);
    EXPECT_TRUE(mock_application_entered);

}

TEST(BootloaderInit, invalid_checksum_enters_bootloader) {

    mock_reset();
    mock_request_bootloader = false;

    /* Flash is all 0xFF — app_header has wrong checksums. */
    bool result = Bootloader_init(&mock_can_driver, &mock_openlcb_driver);
    EXPECT_FALSE(result);
    EXPECT_FALSE(mock_application_entered);
    EXPECT_TRUE(mock_led_state & BOOTLOADER_LED_CSUM_ERROR);

}

TEST(BootloaderLoop, reset_request_causes_reboot) {

    mock_reset();
    mock_request_bootloader = true;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver);

    /* Run loops until initialized (alias negotiation).
     * Advance mock timer to satisfy the 200 ms wait after CID4. */
    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        Bootloader_loop();

    }

    /* Set reset request. */
    bootloader_state.request_reset = 1;
    bool result = Bootloader_loop();

    EXPECT_TRUE(result);
    EXPECT_TRUE(mock_rebooted);

}
