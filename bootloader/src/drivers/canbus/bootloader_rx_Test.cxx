/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_rx_Test.cxx
 *
 * Unit tests for the bootloader RX engine -- frame filtering, datagram
 * assembly, error flagging, and collision detection.
 */

#include "gtest/gtest.h"

extern "C" {
#include "bootloader_rx.h"
#include "bootloader_can.h"
}

/* ====================================================================== */
/* Mock CAN driver                                                         */
/* ====================================================================== */

#define MOCK_RX_QUEUE_SIZE 128

static bootloader_can_frame_t _mock_rx_queue[MOCK_RX_QUEUE_SIZE];
static int _mock_rx_head = 0;
static int _mock_rx_tail = 0;

static bool _mock_read_can_frame(bootloader_can_frame_t *frame) {

    if (_mock_rx_head == _mock_rx_tail) { return false; }

    *frame = _mock_rx_queue[_mock_rx_tail];
    _mock_rx_tail = (_mock_rx_tail + 1) % MOCK_RX_QUEUE_SIZE;
    return true;

}

static bool _mock_try_send(const bootloader_can_frame_t *frame) {

    (void) frame;
    return true;

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

static const uint16_t OUR_ALIAS = 0x123;
static const uint16_t OTHER_ALIAS = 0x456;
static const uint16_t SENDER_ALIAS = 0x789;

static void _reset(void) {

    _mock_rx_head = 0;
    _mock_rx_tail = 0;
    BootloaderRx_init(&_mock_can_driver);

}

static void _enqueue_frame(uint32_t can_id, uint8_t dlc, const uint8_t *data) {

    bootloader_can_frame_t f;
    memset(&f, 0, sizeof(f));
    f.can_id = can_id;
    f.can_dlc = dlc;

    if (data && dlc > 0) {

        memcpy(f.data, data, dlc);

    }

    _mock_rx_queue[_mock_rx_head] = f;
    _mock_rx_head = (_mock_rx_head + 1) % MOCK_RX_QUEUE_SIZE;

}

static uint32_t _make_addressed_id(uint16_t mti, uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            OPENLCB_MESSAGE_STANDARD_FRAME_TYPE |
            ((uint32_t)(mti & 0xFFF) << 12) | src;

}

static uint32_t _make_datagram_only_id(uint16_t dest, uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            CAN_FRAME_TYPE_DATAGRAM_ONLY |
            ((uint32_t) dest << 12) | src;

}

static uint32_t _make_datagram_first_id(uint16_t dest, uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            CAN_FRAME_TYPE_DATAGRAM_FIRST |
            ((uint32_t) dest << 12) | src;

}

static uint32_t _make_datagram_middle_id(uint16_t dest, uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            CAN_FRAME_TYPE_DATAGRAM_MIDDLE |
            ((uint32_t) dest << 12) | src;

}

static uint32_t _make_datagram_final_id(uint16_t dest, uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT | CAN_OPENLCB_MSG |
            CAN_FRAME_TYPE_DATAGRAM_FINAL |
            ((uint32_t) dest << 12) | src;

}

static uint32_t _make_ame_id(uint16_t src) {

    return BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_CONTROL_FRAME_AME | src;

}

/* ====================================================================== */
/* Frame filtering tests                                                   */
/* ====================================================================== */

TEST(BootloaderRx, init_clears_state) {

    _reset();
    EXPECT_FALSE(BootloaderRx_has_frame());
    EXPECT_FALSE(BootloaderRx_has_datagram());
    EXPECT_FALSE(BootloaderRx_has_error());
    EXPECT_FALSE(BootloaderRx_collision_cid());
    EXPECT_FALSE(BootloaderRx_collision_non_cid());

}

TEST(BootloaderRx, discards_non_eff_frame) {

    _reset();

    /* Frame without EFF flag. */
    _enqueue_frame(RESERVED_TOP_BIT | CAN_OPENLCB_MSG | 0x123, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);
    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, discards_frame_without_reserved_top_bit) {

    _reset();

    /* EFF flag set but no reserved top bit. */
    _enqueue_frame(BOOTLOADER_CAN_EFF_FLAG | CAN_OPENLCB_MSG | 0x123, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);
    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, collision_cid_sets_flag) {

    _reset();

    /* CID7 frame with our alias as source. */
    uint32_t cid7 = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_CONTROL_FRAME_CID7 | OUR_ALIAS;
    _enqueue_frame(cid7, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_collision_cid());
    EXPECT_FALSE(BootloaderRx_collision_non_cid());
    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, collision_non_cid_sets_flag) {

    _reset();

    /* OpenLCB message with our alias as source (non-CID collision). */
    uint32_t id = _make_addressed_id(0x100, OUR_ALIAS);
    uint8_t data[2] = {0x04, 0x56};  /* Addressed to some other node. */
    _enqueue_frame(id, 2, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_collision_cid());
    EXPECT_TRUE(BootloaderRx_collision_non_cid());
    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, clear_collision_flags) {

    _reset();

    uint32_t cid7 = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_CONTROL_FRAME_CID7 | OUR_ALIAS;
    _enqueue_frame(cid7, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_collision_cid());
    BootloaderRx_clear_collision_flags();
    EXPECT_FALSE(BootloaderRx_collision_cid());
    EXPECT_FALSE(BootloaderRx_collision_non_cid());

}

TEST(BootloaderRx, discards_datagram_for_other_node) {

    _reset();

    uint32_t id = _make_datagram_only_id(OTHER_ALIAS, SENDER_ALIAS);
    uint8_t data[4] = {0x20, 0x80, 0x00, 0x00};
    _enqueue_frame(id, 4, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_frame());
    EXPECT_FALSE(BootloaderRx_has_datagram());

}

TEST(BootloaderRx, buffers_addressed_message_for_us) {

    _reset();

    /* PIP request addressed to us. */
    uint32_t id = _make_addressed_id(0x828, OTHER_ALIAS);
    uint8_t data[2] = {

        (uint8_t)((OUR_ALIAS >> 8) & 0x0F),
        (uint8_t)(OUR_ALIAS & 0xFF)

    };
    _enqueue_frame(id, 2, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, discards_addressed_message_for_other_node) {

    _reset();

    /* PIP request addressed to another node. */
    uint32_t id = _make_addressed_id(0x828, SENDER_ALIAS);
    uint8_t data[2] = {

        (uint8_t)((OTHER_ALIAS >> 8) & 0x0F),
        (uint8_t)(OTHER_ALIAS & 0xFF)

    };
    _enqueue_frame(id, 2, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, buffers_verify_node_id_global) {

    _reset();

    /* Verify Node ID Global (MTI 0x0490, global so Address Present bit = 0). */
    uint32_t id = _make_addressed_id(0x490, OTHER_ALIAS);
    _enqueue_frame(id, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, discards_other_global_messages) {

    _reset();

    /* Initialization Complete (MTI 0x0100, global). */
    uint32_t id = _make_addressed_id(0x100, OTHER_ALIAS);
    uint8_t data[6] = {0x05, 0x01, 0x01, 0x01, 0x22, 0x00};
    _enqueue_frame(id, 6, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, buffers_ame_control_frame) {

    _reset();

    uint32_t id = _make_ame_id(OTHER_ALIAS);
    _enqueue_frame(id, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, discards_other_control_frames) {

    _reset();

    /* RID from another node. */
    uint32_t id = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_CONTROL_FRAME_RID | OTHER_ALIAS;
    _enqueue_frame(id, 0, NULL);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, circular_buffer_wraps) {

    _reset();

    /* Fill buffer with AME frames (non-datagram). */
    for (int i = 0; i < BOOTLOADER_RX_BUFFER_SIZE; i++) {

        uint32_t id = _make_ame_id((uint16_t)(0x200 + i));
        uint8_t data[1] = {(uint8_t) i};
        _enqueue_frame(id, 1, data);

    }

    BootloaderRx_poll(OUR_ALIAS, 0);

    /* Read all frames. */
    for (int i = 0; i < BOOTLOADER_RX_BUFFER_SIZE; i++) {

        bootloader_can_frame_t out;
        EXPECT_TRUE(BootloaderRx_get_frame(&out));
        EXPECT_EQ(out.data[0], (uint8_t) i);

    }

    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRx, buffer_full_drops_new_frames) {

    _reset();

    /* Fill buffer completely with AME frames. */
    for (int i = 0; i < BOOTLOADER_RX_BUFFER_SIZE; i++) {

        uint32_t id = _make_ame_id((uint16_t)(0x200 + i));
        uint8_t data[1] = {(uint8_t) i};
        _enqueue_frame(id, 1, data);

    }

    /* One more that should be dropped. */
    uint32_t id = _make_ame_id(0x999);
    uint8_t data[1] = {0xFF};
    _enqueue_frame(id, 1, data);

    BootloaderRx_poll(OUR_ALIAS, 0);

    /* Should have exactly BOOTLOADER_RX_BUFFER_SIZE frames, no 0xFF. */
    int count = 0;

    while (BootloaderRx_has_frame()) {

        bootloader_can_frame_t out;
        BootloaderRx_get_frame(&out);
        EXPECT_NE(out.data[0], 0xFF);
        count++;

    }

    EXPECT_EQ(count, BOOTLOADER_RX_BUFFER_SIZE);

}

TEST(BootloaderRx, no_collision_when_alias_is_zero) {

    _reset();

    /* CID with alias 0 -- should not trigger collision. */
    uint32_t cid7 = BOOTLOADER_CAN_EFF_FLAG | RESERVED_TOP_BIT |
            CAN_CONTROL_FRAME_CID7 | 0x000;
    _enqueue_frame(cid7, 0, NULL);
    BootloaderRx_poll(0, 0);

    EXPECT_FALSE(BootloaderRx_collision_cid());
    EXPECT_FALSE(BootloaderRx_collision_non_cid());

}

/* ====================================================================== */
/* Datagram assembly tests                                                 */
/* ====================================================================== */

TEST(BootloaderRxDatagram, datagram_only_delivers_complete) {

    _reset();

    uint8_t payload[4] = {0x20, 0x80, 0x00, 0x00};
    _enqueue_frame(_make_datagram_only_id(OUR_ALIAS, SENDER_ALIAS), 4, payload);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_datagram());
    EXPECT_FALSE(BootloaderRx_has_frame());

    uint16_t src;
    uint8_t buf[72];
    uint8_t len;
    EXPECT_TRUE(BootloaderRx_get_datagram(&src, buf, &len));
    EXPECT_EQ(src, SENDER_ALIAS);
    EXPECT_EQ(len, 4);
    EXPECT_EQ(buf[0], 0x20);
    EXPECT_EQ(buf[1], 0x80);

}

TEST(BootloaderRxDatagram, multi_frame_datagram_assembles) {

    _reset();

    uint8_t first_data[8] = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0xAA};
    uint8_t middle_data[8] = {0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33};
    uint8_t final_data[4] = {0x44, 0x55, 0x66, 0x77};

    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 8, first_data);
    _enqueue_frame(_make_datagram_middle_id(OUR_ALIAS, SENDER_ALIAS), 8, middle_data);
    _enqueue_frame(_make_datagram_final_id(OUR_ALIAS, SENDER_ALIAS), 4, final_data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_datagram());
    EXPECT_FALSE(BootloaderRx_has_error());

    uint16_t src;
    uint8_t buf[72];
    uint8_t len;
    EXPECT_TRUE(BootloaderRx_get_datagram(&src, buf, &len));
    EXPECT_EQ(src, SENDER_ALIAS);
    EXPECT_EQ(len, 20);
    EXPECT_EQ(buf[0], 0x20);
    EXPECT_EQ(buf[7], 0xAA);
    EXPECT_EQ(buf[8], 0xBB);
    EXPECT_EQ(buf[16], 0x44);
    EXPECT_EQ(buf[19], 0x77);

}

TEST(BootloaderRxDatagram, first_during_assembly_flags_error) {

    _reset();

    uint8_t data1[4] = {0x20, 0x00, 0x00, 0x00};
    uint8_t data2[4] = {0x20, 0x80, 0x00, 0x00};

    /* First datagram starts assembly. */
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 4, data1);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_datagram());
    EXPECT_FALSE(BootloaderRx_has_error());

    /* Second FIRST interrupts -- flags error for the old src. */
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, OTHER_ALIAS), 4, data2);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_error());
    bootloader_rx_error_t err = BootloaderRx_get_error();
    EXPECT_EQ(err.error_code, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);
    EXPECT_EQ(err.error_src_alias, SENDER_ALIAS);

}

TEST(BootloaderRxDatagram, middle_without_first_flags_error) {

    _reset();

    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    _enqueue_frame(_make_datagram_middle_id(OUR_ALIAS, SENDER_ALIAS), 4, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_error());
    bootloader_rx_error_t err = BootloaderRx_get_error();
    EXPECT_EQ(err.error_code, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);
    EXPECT_EQ(err.error_src_alias, SENDER_ALIAS);

}

TEST(BootloaderRxDatagram, final_without_first_flags_error) {

    _reset();

    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    _enqueue_frame(_make_datagram_final_id(OUR_ALIAS, SENDER_ALIAS), 4, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_error());
    bootloader_rx_error_t err = BootloaderRx_get_error();
    EXPECT_EQ(err.error_code, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);
    EXPECT_EQ(err.error_src_alias, SENDER_ALIAS);

}

TEST(BootloaderRxDatagram, assembly_timeout_flags_error) {

    _reset();

    uint8_t first_data[4] = {0x20, 0x00, 0x00, 0x00};
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 4, first_data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_error());

    /* Middle arrives 30 ticks (3 seconds) later -- timeout. */
    uint8_t middle_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    _enqueue_frame(_make_datagram_middle_id(OUR_ALIAS, SENDER_ALIAS), 4, middle_data);
    BootloaderRx_poll(OUR_ALIAS, 30);

    EXPECT_TRUE(BootloaderRx_has_error());
    bootloader_rx_error_t err = BootloaderRx_get_error();
    EXPECT_EQ(err.error_code, ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START);

}

TEST(BootloaderRxDatagram, assembly_timeout_boundary_no_error) {

    _reset();

    uint8_t first_data[4] = {0x20, 0x00, 0x00, 0x00};
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 4, first_data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    /* Middle arrives 29 ticks later -- just under timeout. */
    uint8_t middle_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    _enqueue_frame(_make_datagram_middle_id(OUR_ALIAS, SENDER_ALIAS), 4, middle_data);
    BootloaderRx_poll(OUR_ALIAS, 29);

    EXPECT_FALSE(BootloaderRx_has_error());

}

TEST(BootloaderRxDatagram, datagram_only_during_assembly_flags_error_then_delivers) {

    _reset();

    uint8_t first_data[4] = {0x20, 0x00, 0x00, 0x00};
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 4, first_data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    /* DATAGRAM_ONLY from different sender interrupts assembly. */
    uint8_t only_data[3] = {0x20, 0x80, 0xEF};
    _enqueue_frame(_make_datagram_only_id(OUR_ALIAS, OTHER_ALIAS), 3, only_data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    /* Error for old assembly. */
    EXPECT_TRUE(BootloaderRx_has_error());
    bootloader_rx_error_t err = BootloaderRx_get_error();
    EXPECT_EQ(err.error_code, ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END);
    EXPECT_EQ(err.error_src_alias, SENDER_ALIAS);

    /* New datagram delivered. */
    EXPECT_TRUE(BootloaderRx_has_datagram());
    uint16_t src;
    uint8_t buf[72];
    uint8_t len;
    EXPECT_TRUE(BootloaderRx_get_datagram(&src, buf, &len));
    EXPECT_EQ(src, OTHER_ALIAS);
    EXPECT_EQ(len, 3);

}

TEST(BootloaderRxDatagram, datagram_does_not_enter_fifo) {

    _reset();

    uint8_t data[4] = {0x20, 0x80, 0x00, 0x00};
    _enqueue_frame(_make_datagram_first_id(OUR_ALIAS, SENDER_ALIAS), 4, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_FALSE(BootloaderRx_has_frame());

}

TEST(BootloaderRxDatagram, get_datagram_clears_flag) {

    _reset();

    uint8_t data[4] = {0x20, 0x80, 0x00, 0x00};
    _enqueue_frame(_make_datagram_only_id(OUR_ALIAS, SENDER_ALIAS), 4, data);
    BootloaderRx_poll(OUR_ALIAS, 0);

    EXPECT_TRUE(BootloaderRx_has_datagram());

    uint16_t src;
    uint8_t buf[72];
    uint8_t len;
    BootloaderRx_get_datagram(&src, buf, &len);

    EXPECT_FALSE(BootloaderRx_has_datagram());

}
