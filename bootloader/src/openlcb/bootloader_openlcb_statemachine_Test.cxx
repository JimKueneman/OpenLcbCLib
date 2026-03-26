/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_openlcb_statemachine_Test.cxx
 *
 * Unit tests for bootloader_openlcb_statemachine.c — PIP, SNIP, Verify Node ID,
 * Memory Config commands (FREEZE, WRITE, UNFREEZE, RESET).
 */

#include <gtest/gtest.h>
#include <cstring>

#include "bootloader_test_mocks.h"

extern "C" {

#include "bootloader.h"
#include "bootloader_openlcb_statemachine.h"
#include "bootloader_can_statemachine.h"

}

/* Helper: init bootloader and run through alias negotiation. */
static void _init_bootloader(void) {

    mock_reset();
    mock_request_bootloader = BOOTLOADER_REQUESTED_BY_BUTTON;
    Bootloader_init(&mock_can_driver, &mock_openlcb_driver);

    for (int i = 0; i < 20; i++) {

        mock_timer_tick = (uint8_t) (i * 2);
        BootloaderCanSM_loop();

    }

    mock_tx_count = 0;  /* Clear init frames from TX queue. */

}

/*
 * CAN datagram frame type identification.
 *
 * From the Datagram Transport Standard, Section 7.1:
 *   Single: 0x1A,ddd,sss   (RESERVED | OPENLCB | 0x02)
 *   First:  0x1B,ddd,sss   (RESERVED | OPENLCB | 0x03)
 *   Middle: 0x1C,ddd,sss   (RESERVED | OPENLCB | 0x04)
 *   Last:   0x1D,ddd,sss   (RESERVED | OPENLCB | 0x05)
 *
 * All 8 CAN data bytes are payload — no framing bits in data.
 * Mask bits 28-24 of the CAN ID to identify the frame type.
 */
#define MASK_DG_FRAME_TYPE 0x1F000000U

#define DG_SINGLE 0x1A000000U  /* RESERVED_TOP_BIT | CAN_OPENLCB_MSG | 0x02 */
#define DG_FIRST  0x1B000000U  /* RESERVED_TOP_BIT | CAN_OPENLCB_MSG | 0x03 */
#define DG_MIDDLE 0x1C000000U  /* RESERVED_TOP_BIT | CAN_OPENLCB_MSG | 0x04 */
#define DG_LAST   0x1D000000U  /* RESERVED_TOP_BIT | CAN_OPENLCB_MSG | 0x05 */

/* Helper: reassemble the first datagram found in the TX queue.
 * Handles both single-frame and multi-frame (FIRST + MIDDLE* + LAST).
 * Returns total payload length, or 0 if no datagram found. */
static uint8_t _reassemble_datagram(uint8_t *out, uint8_t out_max) {

    uint8_t total = 0;
    bool collecting = false;

    for (int i = 0; i < mock_tx_count; i++) {

        uint32_t type = mock_tx_queue[i].can_id & MASK_DG_FRAME_TYPE;
        uint8_t n = mock_tx_queue[i].can_dlc;

        if (type == DG_SINGLE) {

            if (n > out_max) { n = out_max; }
            memcpy(out, mock_tx_queue[i].data, n);
            return n;

        }

        if (type == DG_FIRST) {

            collecting = true;
            total = 0;
            if (n <= out_max) {
                memcpy(out, mock_tx_queue[i].data, n);
            }
            total += n;

        } else if (collecting && type == DG_MIDDLE) {

            if (total + n <= out_max) {
                memcpy(out + total, mock_tx_queue[i].data, n);
            }
            total += n;

        } else if (collecting && type == DG_LAST) {

            if (total + n <= out_max) {
                memcpy(out + total, mock_tx_queue[i].data, n);
            }
            total += n;
            return total;

        }

    }

    return 0;

}

/* ====================================================================== */
/* Memory Config: FREEZE                                                   */
/* ====================================================================== */

TEST(BootloaderProtocol, freeze_sets_firmware_active) {

    _init_bootloader();

    uint8_t freeze_dg[] = {0x20, 0xA1, 0xEF};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, freeze_dg, 3);

    EXPECT_TRUE(bootloader_state.firmware_active);

}

TEST(BootloaderProtocol, freeze_wrong_space_rejected) {

    _init_bootloader();

    uint8_t freeze_dg[] = {0x20, 0xA1, 0xFD};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, freeze_dg, 3);

    EXPECT_FALSE(bootloader_state.firmware_active);

}

/* ====================================================================== */
/* Memory Config: WRITE                                                    */
/* ====================================================================== */

TEST(BootloaderProtocol, write_firmware_sends_write_reply_ok) {

    _init_bootloader();

    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
                          0xAA, 0xBB, 0xCC, 0xDD};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, write_dg, 11);

    EXPECT_EQ(mock_write_count, 1);

    uint8_t reply[72];
    uint8_t reply_len = _reassemble_datagram(reply, sizeof(reply));

    ASSERT_EQ(reply_len, 7);
    EXPECT_EQ(reply[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(reply[1], 0x10);  /* CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6 */
    EXPECT_EQ(reply[2], 0x00);  /* Address echoed back */
    EXPECT_EQ(reply[3], 0x00);
    EXPECT_EQ(reply[4], 0x00);
    EXPECT_EQ(reply[5], 0x00);
    EXPECT_EQ(reply[6], 0xEF);  /* CONFIG_MEM_SPACE_FIRMWARE */

}

TEST(BootloaderProtocol, write_wrong_space_rejected) {

    _init_bootloader();

    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD,
                          0xAA, 0xBB};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, write_dg, 9);

    EXPECT_EQ(mock_write_count, 0);

}

TEST(BootloaderProtocol, write_erase_fail_sends_write_reply_fail) {

    _init_bootloader();
    mock_erase_result = 0x2088;

    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
                          0xAA, 0xBB, 0xCC, 0xDD};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, write_dg, 11);

    /* Write should not have been called since erase failed. */
    EXPECT_EQ(mock_write_count, 0);

    uint8_t reply[72];
    uint8_t reply_len = _reassemble_datagram(reply, sizeof(reply));

    ASSERT_EQ(reply_len, 9);
    EXPECT_EQ(reply[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(reply[1], 0x18);  /* CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6 */
    EXPECT_EQ(reply[6], 0xEF);  /* CONFIG_MEM_SPACE_FIRMWARE */
    EXPECT_EQ(reply[7], 0x20);  /* Error code high byte */
    EXPECT_EQ(reply[8], 0x88);  /* Error code low byte */

}

TEST(BootloaderProtocol, write_flash_bytes_fail_sends_write_reply_fail) {

    _init_bootloader();
    mock_write_result = 0x1080;

    uint8_t write_dg[] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
                          0xAA, 0xBB, 0xCC, 0xDD};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, write_dg, 11);

    EXPECT_EQ(mock_write_count, 1);

    uint8_t reply[72];
    uint8_t reply_len = _reassemble_datagram(reply, sizeof(reply));

    ASSERT_EQ(reply_len, 9);
    EXPECT_EQ(reply[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(reply[1], 0x18);  /* CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6 */
    EXPECT_EQ(reply[6], 0xEF);  /* CONFIG_MEM_SPACE_FIRMWARE */
    EXPECT_EQ(reply[7], 0x10);  /* Error code high byte */
    EXPECT_EQ(reply[8], 0x80);  /* Error code low byte */

}

/* ====================================================================== */
/* Memory Config: UNFREEZE                                                 */
/* ====================================================================== */

TEST(BootloaderProtocol, unfreeze_success_requests_reset) {

    _init_bootloader();
    bootloader_state.firmware_active = 1;
    mock_flash_complete_result = 0;

    uint8_t unfreeze_dg[] = {0x20, 0xA0, 0xEF};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, unfreeze_dg, 3);

    EXPECT_TRUE(bootloader_state.request_reset);
    EXPECT_FALSE(bootloader_state.firmware_active);

}

TEST(BootloaderProtocol, unfreeze_fail_rejected) {

    _init_bootloader();
    bootloader_state.firmware_active = 1;
    mock_flash_complete_result = 0x2088;

    uint8_t unfreeze_dg[] = {0x20, 0xA0, 0xEF};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, unfreeze_dg, 3);

    EXPECT_FALSE(bootloader_state.request_reset);

}

/* ====================================================================== */
/* Memory Config: RESET                                                    */
/* ====================================================================== */

TEST(BootloaderProtocol, reset_requests_reboot) {

    _init_bootloader();

    uint8_t reset_dg[] = {0x20, 0xA9};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, reset_dg, 2);

    EXPECT_TRUE(bootloader_state.request_reset);

}

/* ====================================================================== */
/* Memory Config: GET OPTIONS                                              */
/* ====================================================================== */

TEST(BootloaderProtocol, get_options_reply_correct) {

    _init_bootloader();

    uint8_t options_dg[] = {0x20, 0x80};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, options_dg, 2);

    uint8_t dg_buf[72];
    uint8_t dg_len = _reassemble_datagram(dg_buf, sizeof(dg_buf));
    ASSERT_EQ(dg_len, 7);

    EXPECT_EQ(dg_buf[0], 0x20);  /* CONFIG_MEM_CONFIGURATION */
    EXPECT_EQ(dg_buf[1], 0x82);  /* CONFIG_MEM_OPTIONS_REPLY */
    EXPECT_EQ(dg_buf[2], 0x00);
    EXPECT_EQ(dg_buf[3], 0x00);
    EXPECT_EQ(dg_buf[4], 0xE2);  /* Write lengths: reserved bits set */
    EXPECT_EQ(dg_buf[5], 0xEF);  /* Highest space = firmware */
    EXPECT_EQ(dg_buf[6], 0xEF);  /* Lowest space = firmware */

}

/* ====================================================================== */
/* Memory Config: GET ADDRESS SPACE INFO                                   */
/* ====================================================================== */

TEST(BootloaderProtocol, get_address_space_info_firmware_present) {

    _init_bootloader();

    uint8_t info_dg[] = {0x20, 0x84, 0xEF};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, info_dg, 3);

    uint8_t dg_buf[72];
    uint8_t dg_len = _reassemble_datagram(dg_buf, sizeof(dg_buf));
    ASSERT_EQ(dg_len, 8);

    EXPECT_EQ(dg_buf[0], 0x20);
    EXPECT_EQ(dg_buf[1], 0x87);  /* Present */
    EXPECT_EQ(dg_buf[2], 0xEF);
    EXPECT_EQ(dg_buf[3], 0xFF);
    EXPECT_EQ(dg_buf[4], 0xFF);
    EXPECT_EQ(dg_buf[5], 0xFF);
    EXPECT_EQ(dg_buf[6], 0xFF);
    EXPECT_EQ(dg_buf[7], 0x00);

}

TEST(BootloaderProtocol, get_address_space_info_unknown_not_present) {

    _init_bootloader();

    uint8_t info_dg[] = {0x20, 0x84, 0xFD};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, info_dg, 3);

    uint8_t dg_buf[72];
    uint8_t dg_len = _reassemble_datagram(dg_buf, sizeof(dg_buf));
    ASSERT_EQ(dg_len, 3);

    EXPECT_EQ(dg_buf[0], 0x20);
    EXPECT_EQ(dg_buf[1], 0x86);  /* Not present */
    EXPECT_EQ(dg_buf[2], 0xFD);

}

/* ====================================================================== */
/* Non-memconfig datagram rejected                                         */
/* ====================================================================== */

TEST(BootloaderProtocol, non_memconfig_datagram_rejected) {

    _init_bootloader();

    uint8_t unknown_dg[] = {0x99, 0x00};
    BootloaderOpenlcbSM_on_datagram_received(0x456, 0, unknown_dg, 2);

    EXPECT_FALSE(bootloader_state.request_reset);

}
