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
 * @file bootloader_types.h
 *
 * Type definitions, constants, and SNIP defaults for the standalone bootloader.
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TYPES__
#define __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TYPES__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* SNIP string defaults — override with -D compiler flags per platform */
    /* ================================================================== */

    #ifndef BOOTLOADER_SNIP_MANUFACTURER
    #define BOOTLOADER_SNIP_MANUFACTURER "OpenLCB"
    #endif

    #ifndef BOOTLOADER_SNIP_MODEL
    #define BOOTLOADER_SNIP_MODEL "Standalone Bootloader"
    #endif

    #ifndef BOOTLOADER_SNIP_HW_VERSION
    #define BOOTLOADER_SNIP_HW_VERSION "1.0"
    #endif

    #ifndef BOOTLOADER_SNIP_SW_VERSION
    #define BOOTLOADER_SNIP_SW_VERSION "1.0"
    #endif

    #ifndef BOOTLOADER_SNIP_USER_NAME
    #define BOOTLOADER_SNIP_USER_NAME ""
    #endif

    #ifndef BOOTLOADER_SNIP_USER_DESCRIPTION
    #define BOOTLOADER_SNIP_USER_DESCRIPTION ""
    #endif

    /* ================================================================== */
    /* OpenLCB constants                                                   */
    /* ================================================================== */

    /** Write buffer size — one datagram payload worth of data. */
    #define BOOTLOADER_WRITE_BUFFER_SIZE 64

    /* MTI values (names from openlcb_defines.h). */
    #define MTI_INITIALIZATION_COMPLETE              0x0100
    #define MTI_VERIFY_NODE_ID_ADDRESSED             0x0488
    #define MTI_VERIFY_NODE_ID_GLOBAL                0x0490
    #define MTI_VERIFIED_NODE_ID                     0x0170
    #define MTI_PROTOCOL_SUPPORT_INQUIRY             0x0828
    #define MTI_PROTOCOL_SUPPORT_REPLY               0x0668
    #define MTI_SIMPLE_NODE_INFO_REQUEST             0x0DE8
    #define MTI_SIMPLE_NODE_INFO_REPLY               0x0A08
    #define MTI_OPTIONAL_INTERACTION_REJECTED         0x0068
    #define MTI_DATAGRAM_OK_REPLY                    0x0A28
    #define MTI_DATAGRAM_REJECTED_REPLY              0x0A48

    /* Memory configuration command bytes (names from openlcb_defines.h). */
    #define CONFIG_MEM_CONFIGURATION                 0x20
    #define CONFIG_MEM_WRITE_SPACE_IN_BYTE_6         0x00
    #define CONFIG_MEM_OPTIONS_CMD                    0x80
    #define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD     0x84
    #define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT 0x87
    #define CONFIG_MEM_FREEZE                        0xA1
    #define CONFIG_MEM_UNFREEZE                      0xA0
    #define CONFIG_MEM_RESET_REBOOT                  0xA9
    #define CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6 0x10
    #define CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6 0x18
    #define CONFIG_MEM_SPACE_FIRMWARE                 0xEF

    /* Datagram reply flags (name from openlcb_defines.h). */
    #define DATAGRAM_OK_REPLY_PENDING                0x80

    /* Protocol Support Inquiry bits (names from openlcb_defines.h). */
    #define PSI_DATAGRAM                             0x400000
    #define PSI_MEMORY_CONFIGURATION                 0x100000
    #define PSI_SIMPLE_NODE_INFORMATION              0x001000
    #define PSI_FIRMWARE_UPGRADE                     0x000020
    #define PSI_FIRMWARE_UPGRADE_ACTIVE              0x000010

    /* Multi-frame flag bits for addressed CAN messages (names from openlcb_defines.h). */
    #define MULTIFRAME_ONLY                          0x00
    #define MULTIFRAME_FIRST                         0x10
    #define MULTIFRAME_FINAL                         0x20
    #define MULTIFRAME_MIDDLE                        0x30

    /* Error codes (names from openlcb_defines.h). */
    #define ERROR_PERMANENT                          0x1000
    #define ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL 0x1043
    #define ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN 0x1081
    #define ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS 0x1082
    #define ERROR_PERMANENT_INVALID_ARGUMENTS        0x1080

    /* LED function identifiers. */
    typedef enum {

        BOOTLOADER_LED_ACTIVE     = 1,
        BOOTLOADER_LED_WRITING    = 2,
        BOOTLOADER_LED_CSUM_ERROR = 8,
        BOOTLOADER_LED_REQUEST    = 16,
        BOOTLOADER_LED_FRAME_LOST = 32

    } bootloader_led_enum;

    /* ================================================================== */
    /* Bootloader state                                                    */
    /* ================================================================== */

    /**
     *     Protocol-layer state for the bootloader.
     *
     *     This state is shared between bootloader.c and bootloader_protocol.c.
     *     Transport-layer state (CAN alias, frames, etc.) is internal to the
     *     transport and not visible here.
     */
    typedef struct {

        unsigned request_reset : 1;
        unsigned firmware_active : 1;

        /** Write buffer for accumulating firmware data from datagrams. */
        uint8_t write_buffer[BOOTLOADER_WRITE_BUFFER_SIZE];
        uint32_t write_buffer_offset;
        uint16_t write_buffer_index;

        /** Source alias of the node sending us firmware data. */
        uint16_t write_src_alias;

        /** Response datagram buffer (for Get Options, Address Space Info, SNIP). */
        uint8_t datagram_payload[72];
        uint8_t datagram_dlc;

    } bootloader_state_t;

    /** Global bootloader state. */
    extern bootloader_state_t bootloader_state;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TYPES__ */
