/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_test_mocks.h
 *
 * Shared mock implementations for all BootloaderDriver_* and
 * BootloaderCanDriver_* functions used by the bootloader test suite.
 *
 * Include this file in each _Test.cxx that needs driver mocks.
 * Only one .cxx in a test executable should define
 * BOOTLOADER_TEST_MOCKS_IMPLEMENT before including this header
 * to provide the actual mock function bodies.
 */

#ifndef __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__
#define __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bootloader_types.h"
#include "bootloader_app_header.h"
#include "bootloader_can.h"
#include "bootloader_crc.h"

/* ====================================================================== */
/* Mock state — accessible from all test files                             */
/* ====================================================================== */

#define MOCK_FLASH_SIZE 65536
#define MOCK_APP_HEADER_OFFSET 0x100
#define MOCK_CAN_QUEUE_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern uint8_t mock_flash[MOCK_FLASH_SIZE];
extern uint64_t mock_node_id;
extern uint16_t mock_alias;
extern bool mock_request_bootloader;
extern uint16_t mock_flash_complete_result;
extern bool mock_application_entered;
extern bool mock_rebooted;

extern bootloader_can_frame_t mock_rx_queue[MOCK_CAN_QUEUE_SIZE];
extern int mock_rx_head;
extern int mock_rx_tail;

extern bootloader_can_frame_t mock_tx_queue[MOCK_CAN_QUEUE_SIZE];
extern int mock_tx_count;

extern uint32_t mock_led_state;
extern int mock_erase_count;
extern int mock_write_count;
extern uint8_t mock_timer_tick;

extern void mock_reset(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* ====================================================================== */
/* Implementation — define in exactly one .cxx per test executable          */
/* ====================================================================== */

#ifdef BOOTLOADER_TEST_MOCKS_IMPLEMENT

uint8_t mock_flash[MOCK_FLASH_SIZE];
uint64_t mock_node_id = 0x050101010700ULL;
uint16_t mock_alias = 0;
bool mock_request_bootloader = false;
uint16_t mock_flash_complete_result = 0;
bool mock_application_entered = false;
bool mock_rebooted = false;

bootloader_can_frame_t mock_rx_queue[MOCK_CAN_QUEUE_SIZE];
int mock_rx_head = 0;
int mock_rx_tail = 0;

bootloader_can_frame_t mock_tx_queue[MOCK_CAN_QUEUE_SIZE];
int mock_tx_count = 0;

uint32_t mock_led_state = 0;
int mock_erase_count = 0;
int mock_write_count = 0;
uint8_t mock_timer_tick = 0;

void mock_reset(void) {

    memset(mock_flash, 0xFF, sizeof(mock_flash));
    mock_node_id = 0x050101010700ULL;
    mock_alias = 0x123;
    mock_request_bootloader = false;
    mock_flash_complete_result = 0;
    mock_application_entered = false;
    mock_rebooted = false;
    mock_rx_head = 0;
    mock_rx_tail = 0;
    mock_tx_count = 0;
    mock_led_state = 0;
    mock_erase_count = 0;
    mock_write_count = 0;
    mock_timer_tick = 0;

}

/* --- BootloaderDriver_* implementations --- */

void BootloaderDriver_hw_set_to_safe(void) { }
void BootloaderDriver_hw_init(void) { }

bool BootloaderDriver_request_bootloader(void) {

    return mock_request_bootloader;

}

void BootloaderDriver_application_entry(void) {

    mock_application_entered = true;

}

void BootloaderDriver_reboot(void) {

    mock_rebooted = true;

}

void BootloaderDriver_get_flash_boundaries(
        const void **flash_min,
        const void **flash_max,
        const bootloader_app_header_t **app_header) {

    *flash_min = mock_flash;
    *flash_max = mock_flash + MOCK_FLASH_SIZE;
    *app_header = (const bootloader_app_header_t *) (mock_flash + MOCK_APP_HEADER_OFFSET);

}

void BootloaderDriver_get_flash_page_info(
        const void *address,
        const void **page_start,
        uint32_t *page_length_bytes) {

    uintptr_t addr = (uintptr_t) address;
    uintptr_t page = addr & ~((uintptr_t) 1023);
    *page_start = (const void *) page;
    *page_length_bytes = 1024;

}

void BootloaderDriver_erase_flash_page(const void *address) {

    uintptr_t offset = (uintptr_t) address - (uintptr_t) mock_flash;

    if (offset + 1024 <= MOCK_FLASH_SIZE) {

        memset(mock_flash + offset, 0xFF, 1024);

    }

    mock_erase_count++;

}

void BootloaderDriver_write_flash(
        const void *address,
        const void *data,
        uint32_t size_bytes) {

    uintptr_t offset = (uintptr_t) address - (uintptr_t) mock_flash;

    if (offset + size_bytes <= MOCK_FLASH_SIZE) {

        memcpy(mock_flash + offset, data, size_bytes);

    }

    mock_write_count++;

}

uint16_t BootloaderDriver_flash_complete(void) {

    return mock_flash_complete_result;

}

void BootloaderDriver_checksum_data(
        const void *data,
        uint32_t size,
        uint32_t *checksum) {

    memset(checksum, 0, BOOTLOADER_CHECKSUM_COUNT * sizeof(uint32_t));
    uint16_t crc_out[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(data, size, crc_out);
    checksum[0] = crc_out[0];
    checksum[1] = crc_out[1];
    checksum[2] = crc_out[2];
    checksum[3] = 0;

}

uint64_t BootloaderDriver_node_id(void) {

    return mock_node_id;

}

void BootloaderDriver_led(bootloader_led_enum led, bool value) {

    if (value) {

        mock_led_state |= (uint32_t) led;

    } else {

        mock_led_state &= ~((uint32_t) led);

    }

}

uint8_t BootloaderDriver_timer_tick(void) {

    return mock_timer_tick;

}

/* --- BootloaderCanDriver_* implementations --- */

bool BootloaderCanDriver_read_can_frame(bootloader_can_frame_t *frame) {

    if (mock_rx_head == mock_rx_tail) { return false; }

    *frame = mock_rx_queue[mock_rx_head++];
    return true;

}

bool BootloaderCanDriver_try_send_can_frame(const bootloader_can_frame_t *frame) {

    if (mock_tx_count >= MOCK_CAN_QUEUE_SIZE) { return false; }

    mock_tx_queue[mock_tx_count++] = *frame;
    return true;

}

uint16_t BootloaderCanDriver_node_alias(void) {

    return mock_alias;

}

#endif /* BOOTLOADER_TEST_MOCKS_IMPLEMENT */

#endif /* __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__ */
