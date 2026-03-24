/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * @file bootloader_test_mocks.h
 *
 * Shared mock state and DI struct instances for the bootloader test suite.
 *
 * Include this file in each _Test.cxx that needs driver mocks.
 * The mock function implementations and DI struct instances are provided
 * by bootloader_test_mocks.c (compiled as a library).
 */

#ifndef __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__
#define __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bootloader_types.h"
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
extern uint16_t mock_erase_result;
extern int mock_write_count;
extern uint16_t mock_write_result;
extern uint8_t mock_timer_tick;

extern void mock_reset(void);

/** Pre-built DI struct instances for tests. */
extern const bootloader_can_driver_t mock_can_driver;
extern const bootloader_openlcb_driver_t mock_openlcb_driver;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_TEST_BOOTLOADER_TEST_MOCKS__ */
