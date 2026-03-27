/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_test_mocks.c
 *
 * Mock implementations for bootloader test suite. Provides DI struct
 * instances populated with mock functions.
 */

#include <string.h>

#include "bootloader_test_mocks.h"
#include "bootloader_crc.h"

/* ====================================================================== */
/* Mock state                                                              */
/* ====================================================================== */

/* Satisfies the extern in bootloader_types.h.  In real hardware this lives
 * in a .noinit section; in tests we just define it as a normal variable. */
volatile uint32_t bootloader_request_flag = 0;
volatile uint16_t bootloader_cached_alias = 0;

uint8_t mock_flash[MOCK_FLASH_SIZE];
uint64_t mock_node_id = 0x050101010700ULL;
uint16_t mock_alias = 0;
bootloader_request_t mock_request_bootloader = BOOTLOADER_NOT_REQUESTED;
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
uint16_t mock_erase_result = 0;
int mock_write_count = 0;
uint16_t mock_write_result = 0;
uint8_t mock_timer_tick = 0;

void mock_reset(void) {

    memset(mock_flash, 0xFF, sizeof(mock_flash));
    mock_node_id = 0x050101010700ULL;
    mock_alias = 0x123;
    mock_request_bootloader = BOOTLOADER_NOT_REQUESTED;
    bootloader_request_flag = 0;
    bootloader_cached_alias = 0;
    mock_flash_complete_result = 0;
    mock_application_entered = false;
    mock_rebooted = false;
    mock_rx_head = 0;
    mock_rx_tail = 0;
    mock_tx_count = 0;
    mock_led_state = 0;
    mock_erase_count = 0;
    mock_erase_result = 0;
    mock_write_count = 0;
    mock_write_result = 0;
    mock_timer_tick = 0;

}

/* ====================================================================== */
/* Mock CAN driver functions                                               */
/* ====================================================================== */

static bool _mock_read_can_frame(bootloader_can_frame_t *frame) {

    if (mock_rx_head == mock_rx_tail) { return false; }

    *frame = mock_rx_queue[mock_rx_head++];
    return true;

}

static bool _mock_try_send_can_frame(const bootloader_can_frame_t *frame) {

    if (mock_tx_count >= MOCK_CAN_QUEUE_SIZE) { return false; }

    mock_tx_queue[mock_tx_count++] = *frame;
    return true;

}

static uint16_t _mock_node_alias(void) {

    return mock_alias;

}

/* ====================================================================== */
/* Mock OpenLCB driver functions                                                */
/* ====================================================================== */

static uint64_t _mock_node_id(void) {

    return mock_node_id;

}

static uint8_t _mock_timer_tick(void) {

    return mock_timer_tick;

}

static void _mock_led(bootloader_led_enum led, bool value) {

    if (value) {

        mock_led_state |= (uint32_t) led;

    } else {

        mock_led_state &= ~((uint32_t) led);

    }

}

static bootloader_request_t _mock_request_bootloader(void) {

    return mock_request_bootloader;

}

static void _mock_application_entry(void) {

    mock_application_entered = true;

}

static void _mock_cleanup_before_handoff(void) { }

static void _mock_reboot(void) {

    mock_rebooted = true;

}

static void _mock_openlcb_init(bootloader_request_t request) { (void)request; }

static void _mock_get_flash_boundaries(const void **flash_min, const void **flash_max, const struct bootloader_app_header **app_header) {

    *flash_min = mock_flash;
    *flash_max = mock_flash + MOCK_FLASH_SIZE;
    *app_header = (const struct bootloader_app_header *)
            (mock_flash + MOCK_APP_HEADER_OFFSET);

}

static void _mock_get_flash_page_info(const void *address, const void **page_start, uint32_t *page_length_bytes) {

    uintptr_t addr = (uintptr_t) address;
    uintptr_t page = addr & ~((uintptr_t) 1023);
    *page_start = (const void *) page;
    *page_length_bytes = 1024;

}

static uint16_t _mock_erase_flash_page(const void *address) {

    uintptr_t offset = (uintptr_t) address - (uintptr_t) mock_flash;

    if (offset + 1024 <= MOCK_FLASH_SIZE) {

        memset(mock_flash + offset, 0xFF, 1024);

    }

    mock_erase_count++;
    return mock_erase_result;

}

static uint16_t _mock_write_flash_bytes(const void *address, const void *data, uint32_t size_bytes) {

    uintptr_t offset = (uintptr_t) address - (uintptr_t) mock_flash;

    if (offset + size_bytes <= MOCK_FLASH_SIZE) {

        memcpy(mock_flash + offset, data, size_bytes);

    }

    mock_write_count++;
    return mock_write_result;

}

static uint16_t _mock_flash_complete(compute_checksum_func_t compute_checksum_helper) {

    (void) compute_checksum_helper;
    return mock_flash_complete_result;

}

static void _mock_checksum_data(const void *data, uint32_t size, uint32_t *checksum) {

    memset(checksum, 0, BOOTLOADER_CHECKSUM_COUNT * sizeof(uint32_t));
    uint16_t crc_out[3] = {0, 0, 0};
    BootloaderCrc_crc3_crc16_ibm(data, size, crc_out);
    checksum[0] = crc_out[0];
    checksum[1] = crc_out[1];
    checksum[2] = crc_out[2];
    checksum[3] = 0;

}

static void _mock_read_flash_bytes(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes) {

    /* On 64-bit test hosts the caller truncates the real pointer to uint32_t.
     * Recover the correct source address by computing the byte offset from
     * the low-32-bit representation of mock_flash[0]; modular subtraction is
     * exact as long as the array never crosses a 4 GB boundary. */
    uint32_t base32 = (uint32_t)(uintptr_t) mock_flash;
    uint32_t offset = flash_addr - base32;
    memcpy(dest_ram, mock_flash + offset, size_bytes);

}

/* ====================================================================== */
/* DI struct instances                                                     */
/* ====================================================================== */

const bootloader_can_driver_t mock_can_driver = {

    .read_received_frame = _mock_read_can_frame,
    .try_send_frame = _mock_try_send_can_frame,
    .get_cached_alias = _mock_node_alias

};

const bootloader_openlcb_driver_t mock_openlcb_driver = {

    .get_persistent_node_id = _mock_node_id,
    .get_100ms_timer_tick = _mock_timer_tick,
    .set_status_led = _mock_led,
    .is_bootloader_requested = _mock_request_bootloader,
    .jump_to_application = _mock_application_entry,
    .reboot = _mock_reboot,
    .initialize_hardware = _mock_openlcb_init,
    .get_flash_boundaries = _mock_get_flash_boundaries,
    .get_flash_page_info = _mock_get_flash_page_info,
    .erase_flash_page = _mock_erase_flash_page,
    .write_flash_bytes = _mock_write_flash_bytes,
    .finalize_flash = _mock_flash_complete,
    .compute_checksum = _mock_checksum_data,
    .read_flash_bytes = _mock_read_flash_bytes,
    .cleanup_before_handoff = _mock_cleanup_before_handoff

};
