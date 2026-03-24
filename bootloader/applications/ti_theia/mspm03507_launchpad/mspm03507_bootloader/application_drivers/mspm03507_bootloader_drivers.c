/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file mspm03507_bootloader_drivers.c
 *
 * Platform driver implementation for the MSPM0G3507 LaunchPad bootloader.
 * Implements all extern functions from bootloader_driver.h and
 * bootloader_can_driver.h.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#include <string.h>

#include "../Debug/ti_msp_dl_config.h"
#include "mspm03507_bootloader_drivers.h"

#include "../src/openlcb/bootloader_driver.h"
#include "../src/drivers/canbus/bootloader_can_driver.h"
#include "../src/crc/bootloader_crc.h"

/* ====================================================================== */
/* Lifecycle                                                               */
/* ====================================================================== */

void BootloaderDriver_hw_set_to_safe(void) {

    __disable_irq();

}

void BootloaderDriver_hw_init(void) {

    SYSCFG_DL_init();
    __enable_irq();

}

bool BootloaderDriver_request_bootloader(void) {

    /* Check S2 button (PB21) — negative logic with pull-up.
     * Low = pressed = request bootloader. */
    uint32_t pin_state = DL_GPIO_readPins(BOOTLOADER_BUTTON_PORT,
            BOOTLOADER_BUTTON_PIN);

    if (pin_state == 0) {

        return true;

    }

    return false;

}

void BootloaderDriver_application_entry(void) {

    __disable_irq();

    /* The application vector table starts at APP_FLASH_START.
     * Entry 0 = initial MSP, Entry 1 = reset handler. */
    uint32_t *app_vectors = (uint32_t *) APP_FLASH_START;

    /* Relocate the vector table to the application's base. */
    SCB->VTOR = APP_FLASH_START;

    /* Set the main stack pointer to the app's initial MSP value. */
    __set_MSP(app_vectors[0]);

    /* Jump to the application reset handler. Never returns. */
    void (*app_reset)(void) = (void (*)(void)) app_vectors[1];
    app_reset();

    /* Should never reach here. */
    while (1) {
    }

}

void BootloaderDriver_reboot(void) {

    NVIC_SystemReset();

    /* Should never reach here. */
    while (1) {
    }

}

/* ====================================================================== */
/* Flash operations                                                        */
/* ====================================================================== */

void BootloaderDriver_get_flash_boundaries(
        const void **flash_min,
        const void **flash_max,
        const bootloader_app_header_t **app_header) {

    *flash_min = (const void *) APP_FLASH_START;
    *flash_max = (const void *) APP_FLASH_END;
    *app_header = (const bootloader_app_header_t *) APP_HEADER_ADDRESS;

}

void BootloaderDriver_get_flash_page_info(
        const void *address,
        const void **page_start,
        uint32_t *page_length_bytes) {

    /* MSPM0G3507 has uniform 1KB sectors. */
    uintptr_t addr = (uintptr_t) address;
    *page_start = (const void *) (addr & ~(FLASH_SECTOR_SIZE - 1));
    *page_length_bytes = FLASH_SECTOR_SIZE;

}

void BootloaderDriver_erase_flash_page(const void *address) {

    /* Clear flash status before operation (required at CPUCLK >= 32 MHz). */
    DL_FlashCTL_executeClearStatus(FLASHCTL);

    /* On MSPM0G3507, flash erase must execute from RAM.
     * DL_FlashCTL_eraseMemoryFromRAM handles this. */
    DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, (uint32_t) address,
            DL_FLASHCTL_COMMAND_SIZE_SECTOR);

}

void BootloaderDriver_write_flash(
        const void *address,
        const void *data,
        uint32_t size_bytes) {

    /* On MSPM0G3507, flash write must execute from RAM.
     * Writes 64-bit words (8 bytes at a time).
     * DL_FlashCTL_programMemoryFromRAM handles the RAM execution. */

    /* Clear flash status before operation (required at CPUCLK >= 32 MHz). */
    DL_FlashCTL_executeClearStatus(FLASHCTL);

    uint32_t dest = (uint32_t) address;
    const uint8_t *src = (const uint8_t *) data;
    uint32_t remaining = size_bytes;

    while (remaining >= FLASH_WRITE_ALIGN) {

        DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
                FLASHCTL, dest, (uint32_t *) src);

        dest += FLASH_WRITE_ALIGN;
        src += FLASH_WRITE_ALIGN;
        remaining -= FLASH_WRITE_ALIGN;

    }

    /* Handle any remaining bytes (pad with 0xFF to 8-byte boundary). */
    if (remaining > 0) {

        uint8_t pad_buffer[FLASH_WRITE_ALIGN];
        memset(pad_buffer, 0xFF, sizeof(pad_buffer));
        memcpy(pad_buffer, src, remaining);

        DL_FlashCTL_programMemoryFromRAM64WithECCGenerated(
                FLASHCTL, dest, (uint32_t *) pad_buffer);

    }

}

uint16_t BootloaderDriver_flash_complete(void) {

    /* No additional validation in standalone cold boot mode.
     * The checksum is verified at next boot by Bootloader_check_application_checksum(). */
    return 0;

}

/* ====================================================================== */
/* Checksum                                                                */
/* ====================================================================== */

void BootloaderDriver_checksum_data(
        const void *data,
        uint32_t size,
        uint32_t *checksum) {

    /* Use software triple CRC-16-IBM.
     * BootloaderCrc_crc3_crc16_ibm produces 3 uint16_t values.
     * Map them into the 4 uint32_t checksum slots (4th = 0). */
    uint16_t crc3[3];
    BootloaderCrc_crc3_crc16_ibm(data, size, crc3);

    checksum[0] = (uint32_t) crc3[0];
    checksum[1] = (uint32_t) crc3[1];
    checksum[2] = (uint32_t) crc3[2];
    checksum[3] = 0;

}

/* ====================================================================== */
/* Timer                                                                   */
/* ====================================================================== */

static volatile uint8_t _tick_counter = 0;

void SysTick_Handler(void) {

    _tick_counter++;

}

uint8_t BootloaderDriver_timer_tick(void) {

    return _tick_counter;

}

/* ====================================================================== */
/* Identity                                                                */
/* ====================================================================== */

uint64_t BootloaderDriver_node_id(void) {

    /* TODO: Read from protected flash sector at NODEID_FLASH_ADDRESS
     * once production programming is in place. Hardcoded for bringup. */
    return 0x050101012200ULL;

}

/* ====================================================================== */
/* LED                                                                     */
/* ====================================================================== */

void BootloaderDriver_led(bootloader_led_enum led, bool value) {

    /* Only BOOTLOADER_LED_ACTIVE and BOOTLOADER_LED_CSUM_ERROR are used
     * in bootloader.c. Map both to LED1 (PA0, negative logic). */
    (void) led;

    if (value) {

        /* LED on = clear pin (negative logic). */
        DL_GPIO_clearPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    } else {

        /* LED off = set pin (negative logic). */
        DL_GPIO_setPins(BOOTLOADER_LED_PORT, BOOTLOADER_LED_PIN);

    }

}

/* ====================================================================== */
/* CAN driver                                                              */
/* ====================================================================== */

bool BootloaderCanDriver_read_can_frame(bootloader_can_frame_t *frame) {

    /* Check if there is a message in RX FIFO 1.
     * The working BasicNode config routes all extended ID frames to FIFO 1.
     * Read the RXF1S register directly — the DriverLib getRxFIFOStatus()
     * only supports FIFO 0 in this SDK version. */
    uint32_t rxf1s = CANFD0->MCANSS.MCAN.MCAN_RXF1S;
    uint32_t fill_lvl = (rxf1s & MCAN_RXF1S_F1FL_MASK) >> MCAN_RXF1S_F1FL_OFS;

    if (fill_lvl == 0) {

        return false;

    }

    uint32_t get_idx = (rxf1s & MCAN_RXF1S_F1GI_MASK) >> MCAN_RXF1S_F1GI_OFS;

    DL_MCAN_RxBufElement rx_elem;
    DL_MCAN_readMsgRam(CANFD0, DL_MCAN_MEM_TYPE_FIFO, get_idx,
            DL_MCAN_RX_FIFO_NUM_1, &rx_elem);

    DL_MCAN_writeRxFIFOAck(CANFD0, DL_MCAN_RX_FIFO_NUM_1, get_idx);

    /* Convert MCAN element to bootloader_can_frame_t. */
    if (rx_elem.xtd) {

        frame->can_id = (rx_elem.id & 0x1FFFFFFFU) | BOOTLOADER_CAN_EFF_FLAG;

    } else {

        frame->can_id = (rx_elem.id >> 18) & 0x7FFU;

    }

    frame->can_dlc = rx_elem.dlc;

    if (frame->can_dlc > 8) {

        frame->can_dlc = 8;

    }

    memcpy(frame->data, rx_elem.data, frame->can_dlc);

    return true;

}

bool BootloaderCanDriver_try_send_can_frame(const bootloader_can_frame_t *frame) {

    /* The working BasicNode config uses 2 dedicated TX buffers (not TX FIFO).
     * Check if TX buffer 0 is free by checking the transmission request
     * pending register. If buffer 0 is busy, try buffer 1. */
    uint32_t buf_idx = 0;
    uint32_t tx_req_pending = DL_MCAN_getTxBufReqPend(CANFD0);

    if (tx_req_pending & (1U << 0)) {

        if (tx_req_pending & (1U << 1)) {

            return false;

        }

        buf_idx = 1;

    }

    /* Build MCAN TX element. */
    DL_MCAN_TxBufElement tx_elem;
    memset(&tx_elem, 0, sizeof(tx_elem));

    if (frame->can_id & BOOTLOADER_CAN_EFF_FLAG) {

        tx_elem.id = frame->can_id & 0x1FFFFFFFU;
        tx_elem.xtd = 1;

    } else {

        tx_elem.id = (frame->can_id & 0x7FFU) << 18;
        tx_elem.xtd = 0;

    }

    tx_elem.dlc = frame->can_dlc;
    tx_elem.fdf = 0;   /* Classic CAN, not CAN-FD. */
    tx_elem.brs = 0;

    memcpy(tx_elem.data, frame->data, frame->can_dlc);

    /* Write to dedicated TX buffer and request transmission. */
    DL_MCAN_writeMsgRam(CANFD0, DL_MCAN_MEM_TYPE_BUF, buf_idx, &tx_elem);
    DL_MCAN_TXBufAddReq(CANFD0, buf_idx);

    return true;

}

uint16_t BootloaderCanDriver_node_alias(void) {

    /* Cold boot: auto-generate alias from node ID. */
    return 0;

}
