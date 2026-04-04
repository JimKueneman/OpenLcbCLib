/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_crc_Test.cxx
 *
 * Unit tests for the CRC-16-IBM implementation.
 */

#include <gtest/gtest.h>
#include <cstring>

extern "C" {

#include "bootloader_crc.h"

}

TEST(BootloaderCrc, crc16_ibm_known_vector) {

    const char *test_data = "123456789";
    uint16_t result = BootloaderCrc_crc16_ibm(test_data, 9);
    EXPECT_EQ(result, 0xBB3D);

}

TEST(BootloaderCrc, crc16_ibm_empty) {

    uint16_t result = BootloaderCrc_crc16_ibm(NULL, 0);
    EXPECT_EQ(result, 0x0000);

}

TEST(BootloaderCrc, crc16_ibm_single_byte_0x42) {

    uint8_t data = 0x42;
    uint16_t result = BootloaderCrc_crc16_ibm(&data, 1);
    EXPECT_NE(result, 0x0000);

}

TEST(BootloaderCrc, crc3_triple_overall_matches_single) {

    const char *test_data = "123456789";
    uint16_t checksum[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(test_data, 9, checksum);

    EXPECT_EQ(checksum[0], 0xBB3D);

}

TEST(BootloaderCrc, crc3_odd_even_differ) {

    const char *test_data = "123456789";
    uint16_t checksum[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(test_data, 9, checksum);

    EXPECT_NE(checksum[1], 0);
    EXPECT_NE(checksum[2], 0);
    EXPECT_NE(checksum[1], checksum[2]);

}

TEST(BootloaderCrc, crc3_single_byte_odd_only) {

    uint8_t data = 0x42;
    uint16_t checksum[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(&data, 1, checksum);

    /* 1 byte = odd index only. Overall == odd, even == 0. */
    EXPECT_EQ(checksum[0], checksum[1]);
    EXPECT_EQ(checksum[2], 0x0000);

}

TEST(BootloaderCrc, crc3_two_bytes) {

    uint8_t data[2] = {0xAA, 0x55};
    uint16_t checksum[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(data, 2, checksum);

    /* 2 bytes: one odd, one even. All three should be non-zero. */
    EXPECT_NE(checksum[0], 0);
    EXPECT_NE(checksum[1], 0);
    EXPECT_NE(checksum[2], 0);

}
