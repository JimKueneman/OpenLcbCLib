/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_tx_Test.cxx
 *
 * Unit tests for the bootloader TX engine.
 */

#include "gtest/gtest.h"

extern "C" {
#include "bootloader_tx.h"
#include "bootloader_rx.h"
#include "bootloader_can.h"
}

/* ====================================================================== */
/* Mock CAN driver                                                         */
/* ====================================================================== */

#define MOCK_TX_QUEUE_SIZE 64

static bootloader_can_frame_t _mock_tx_queue[MOCK_TX_QUEUE_SIZE];
static int _mock_tx_count = 0;
static bool _mock_tx_fail = false;
static int _mock_tx_fail_count = 0;

static bool _mock_try_send(const bootloader_can_frame_t *frame) {

    if (_mock_tx_fail) {

        _mock_tx_fail_count++;

        /* Fail a few times then succeed to test retry. */
        if (_mock_tx_fail_count <= 3) { return false; }

        _mock_tx_fail = false;

    }

    if (_mock_tx_count < MOCK_TX_QUEUE_SIZE) {

        _mock_tx_queue[_mock_tx_count++] = *frame;

    }

    return true;

}

static bool _mock_read_can_frame(bootloader_can_frame_t *frame) {

    (void) frame;
    return false;

}

static uint16_t _mock_node_alias(void) {

    return 0;

}

static const bootloader_can_driver_t _mock_can_driver = {

    .read_received_frame = _mock_read_can_frame,
    .try_send_frame = _mock_try_send,
    .get_cached_alias = _mock_node_alias

};

/* ====================================================================== */
/* Helpers                                                                 */
/* ====================================================================== */

static const uint16_t SRC_ALIAS = 0x123;
static const uint16_t DEST_ALIAS = 0x456;

static void _reset(void) {

    _mock_tx_count = 0;
    _mock_tx_fail = false;
    _mock_tx_fail_count = 0;
    BootloaderRx_init(&_mock_can_driver);
    BootloaderTx_init(&_mock_can_driver);

}

/* ====================================================================== */
/* Tests — send_frame                                                      */
/* ====================================================================== */

TEST(BootloaderTx, send_frame_success) {

    _reset();

    bootloader_can_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x12345678;
    frame.can_dlc = 3;
    frame.data[0] = 0xAA;

    EXPECT_TRUE(BootloaderTx_send_frame(&frame));
    EXPECT_EQ(_mock_tx_count, 1);
    EXPECT_EQ(_mock_tx_queue[0].can_id, 0x12345678u);

}

TEST(BootloaderTx, send_frame_failure) {

    _reset();
    _mock_tx_fail = true;

    bootloader_can_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.can_id = 0x12345678;

    EXPECT_FALSE(BootloaderTx_send_frame(&frame));
    EXPECT_EQ(_mock_tx_count, 0);

}

/* ====================================================================== */
/* Tests — send_multiframe single frame                                    */
/* ====================================================================== */

TEST(BootloaderTx, multiframe_single_frame_pip_reply) {

    _reset();

    /* PIP reply: 6 bytes fits in one frame. */
    uint8_t pip_data[6] = {0x50, 0x10, 0x00, 0x00, 0x00, 0x00};
    uint16_t mti = 0x668;  /* MTI_PROTOCOL_SUPPORT_REPLY */

    EXPECT_TRUE(BootloaderTx_send_multiframe(mti, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, pip_data, 6));
    EXPECT_EQ(_mock_tx_count, 1);

    /* Verify CAN ID. */
    uint32_t expected_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t) mti << 12) | SRC_ALIAS;
    EXPECT_EQ(_mock_tx_queue[0].can_id, expected_id);

    /* Verify framing: MULTIFRAME_ONLY (0x00) in top nibble of data[0]. */
    EXPECT_EQ(_mock_tx_queue[0].data[0] & 0xF0, MULTIFRAME_ONLY);

    /* Verify dest alias in data[0] lower nibble + data[1]. */
    uint16_t got_dest = ((_mock_tx_queue[0].data[0] & 0x0F) << 8) |
            _mock_tx_queue[0].data[1];
    EXPECT_EQ(got_dest, DEST_ALIAS);

    /* Verify DLC = 2 (header) + 6 (data) = 8. */
    EXPECT_EQ(_mock_tx_queue[0].can_dlc, 8);

}

/* ====================================================================== */
/* Tests — send_multiframe multi-frame (SNIP)                              */
/* ====================================================================== */

TEST(BootloaderTx, multiframe_snip_reply) {

    _reset();

    /* 20-byte SNIP payload: 4 frames (6 + 6 + 6 + 2). */
    uint8_t snip[20];

    for (int i = 0; i < 20; i++) { snip[i] = (uint8_t) i; }

    uint16_t mti = 0xA08;  /* MTI_SIMPLE_NODE_INFO_REPLY */

    EXPECT_TRUE(BootloaderTx_send_multiframe(mti, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, snip, 20));

    /* Should produce 4 frames: first(6) + middle(6) + middle(6) + final(2). */
    EXPECT_EQ(_mock_tx_count, 4);

    /* Frame 0: MULTIFRAME_FIRST, 6 data bytes. */
    EXPECT_EQ(_mock_tx_queue[0].data[0] & 0xF0, MULTIFRAME_FIRST);
    EXPECT_EQ(_mock_tx_queue[0].can_dlc, 8);
    EXPECT_EQ(_mock_tx_queue[0].data[2], 0);  /* snip[0] */
    EXPECT_EQ(_mock_tx_queue[0].data[7], 5);  /* snip[5] */

    /* Frame 1: MULTIFRAME_MIDDLE, 6 data bytes. */
    EXPECT_EQ(_mock_tx_queue[1].data[0] & 0xF0, MULTIFRAME_MIDDLE);
    EXPECT_EQ(_mock_tx_queue[1].can_dlc, 8);
    EXPECT_EQ(_mock_tx_queue[1].data[2], 6);  /* snip[6] */

    /* Frame 2: MULTIFRAME_MIDDLE, 6 data bytes. */
    EXPECT_EQ(_mock_tx_queue[2].data[0] & 0xF0, MULTIFRAME_MIDDLE);
    EXPECT_EQ(_mock_tx_queue[2].can_dlc, 8);
    EXPECT_EQ(_mock_tx_queue[2].data[2], 12); /* snip[12] */

    /* Frame 3: MULTIFRAME_FINAL, 2 data bytes. */
    EXPECT_EQ(_mock_tx_queue[3].data[0] & 0xF0, MULTIFRAME_FINAL);
    EXPECT_EQ(_mock_tx_queue[3].can_dlc, 4);  /* 2 header + 2 data */
    EXPECT_EQ(_mock_tx_queue[3].data[2], 18); /* snip[18] */
    EXPECT_EQ(_mock_tx_queue[3].data[3], 19); /* snip[19] */

}

TEST(BootloaderTx, multiframe_exactly_12_bytes) {

    _reset();

    /* 12 bytes = first(6) + final(6). */
    uint8_t data[12];

    for (int i = 0; i < 12; i++) { data[i] = (uint8_t) i; }

    EXPECT_TRUE(BootloaderTx_send_multiframe(0xA08, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, data, 12));
    EXPECT_EQ(_mock_tx_count, 2);

    EXPECT_EQ(_mock_tx_queue[0].data[0] & 0xF0, MULTIFRAME_FIRST);
    EXPECT_EQ(_mock_tx_queue[0].can_dlc, 8);

    EXPECT_EQ(_mock_tx_queue[1].data[0] & 0xF0, MULTIFRAME_FINAL);
    EXPECT_EQ(_mock_tx_queue[1].can_dlc, 8);

}

TEST(BootloaderTx, multiframe_exactly_7_bytes) {

    _reset();

    /* 7 bytes = first(6) + final(1). */
    uint8_t data[7] = {0, 1, 2, 3, 4, 5, 6};

    EXPECT_TRUE(BootloaderTx_send_multiframe(0xA08, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, data, 7));
    EXPECT_EQ(_mock_tx_count, 2);

    EXPECT_EQ(_mock_tx_queue[0].data[0] & 0xF0, MULTIFRAME_FIRST);
    EXPECT_EQ(_mock_tx_queue[0].can_dlc, 8);

    EXPECT_EQ(_mock_tx_queue[1].data[0] & 0xF0, MULTIFRAME_FINAL);
    EXPECT_EQ(_mock_tx_queue[1].can_dlc, 3);  /* 2 header + 1 data */

}

TEST(BootloaderTx, multiframe_zero_bytes) {

    _reset();

    /* 0 bytes = single frame with just the 2-byte header. */
    EXPECT_TRUE(BootloaderTx_send_multiframe(0x668, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, NULL, 0));
    EXPECT_EQ(_mock_tx_count, 1);
    EXPECT_EQ(_mock_tx_queue[0].data[0] & 0xF0, MULTIFRAME_ONLY);
    EXPECT_EQ(_mock_tx_queue[0].can_dlc, 2);

}

TEST(BootloaderTx, multiframe_all_frames_have_correct_can_id) {

    _reset();

    uint8_t data[20];
    memset(data, 0, 20);
    uint16_t mti = 0xA08;

    BootloaderTx_send_multiframe(mti, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, data, 20);

    uint32_t expected_id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_OPENLCB_MSG | OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t) mti << 12) | SRC_ALIAS;

    for (int i = 0; i < _mock_tx_count; i++) {

        EXPECT_EQ(_mock_tx_queue[i].can_id, expected_id)
                << "Frame " << i << " has wrong CAN ID";

    }

}

TEST(BootloaderTx, multiframe_all_frames_have_dest_alias) {

    _reset();

    uint8_t data[20];
    memset(data, 0, 20);

    BootloaderTx_send_multiframe(0xA08, SRC_ALIAS, DEST_ALIAS,
            SRC_ALIAS, 0, data, 20);

    for (int i = 0; i < _mock_tx_count; i++) {

        uint16_t got_dest = ((_mock_tx_queue[i].data[0] & 0x0F) << 8) |
                _mock_tx_queue[i].data[1];
        EXPECT_EQ(got_dest, DEST_ALIAS)
                << "Frame " << i << " has wrong dest alias";

    }

}
