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
 * @file bootloader_can.h
 *
 * CAN transport layer types and declarations for the standalone bootloader.
 * Constant names match the main OpenLcbCLib library (openlcb_defines.h).
 *
 * @author Jim Kueneman
 * @date 23 Mar 2026
 */

#ifndef __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN__
#define __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN__

#include <stdint.h>
#include <stdbool.h>

#include "../../openlcb/bootloader_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* ================================================================== */
    /* CAN ID field constants (names from openlcb_defines.h)               */
    /* ================================================================== */

    /** EFF flag in can_id (bit 31) -- not part of OpenLCB, used by driver layer. */
    #define BOOTLOADER_CAN_EFF_FLAG 0x80000000U

    /** Bit 28: reserved, shall be sent as 1 (CAN Frame Transfer Standard §4). */
    #define RESERVED_TOP_BIT 0x10000000U

    /** Bit 27: Frame Type -- 1=OpenLCB Message, 0=CAN Control Frame. */
    #define CAN_OPENLCB_MSG 0x08000000U

    /** Mask for frame type bits (26-24). */
    #define MASK_CAN_FRAME_TYPE 0x07000000U

    /** Mask for variable field (bits 23-12). */
    #define MASK_CAN_VARIABLE_FIELD 0x00FFF000U

    /** Frame type: Global or addressed OpenLCB message. */
    #define OPENLCB_MESSAGE_STANDARD_FRAME_TYPE 0x01000000U

    /** Frame type: Datagram complete in single frame. */
    #define CAN_FRAME_TYPE_DATAGRAM_ONLY 0x02000000U

    /** Frame type: First frame of multi-frame datagram. */
    #define CAN_FRAME_TYPE_DATAGRAM_FIRST 0x03000000U

    /** Frame type: Middle frame of multi-frame datagram. */
    #define CAN_FRAME_TYPE_DATAGRAM_MIDDLE 0x04000000U

    /** Frame type: Final frame of multi-frame datagram. */
    #define CAN_FRAME_TYPE_DATAGRAM_FINAL 0x05000000U

    /* ================================================================== */
    /* CAN control frame identifiers (names from openlcb_defines.h)        */
    /* ================================================================== */

    #define CAN_CONTROL_FRAME_CID7 0x07000000U
    #define CAN_CONTROL_FRAME_CID6 0x06000000U
    #define CAN_CONTROL_FRAME_CID5 0x05000000U
    #define CAN_CONTROL_FRAME_CID4 0x04000000U

    #define CAN_CONTROL_FRAME_RID 0x00700000U
    #define CAN_CONTROL_FRAME_AMD 0x00701000U
    #define CAN_CONTROL_FRAME_AME 0x00702000U
    #define CAN_CONTROL_FRAME_AMR 0x00703000U

    /* ================================================================== */
    /* Helper to get the current transport alias                           */
    /* ================================================================== */

        /**
         *     Returns the alias currently in use by the CAN transport.
         *     Available after initialization is complete.
         *
         *     @return 12-bit CAN alias
         */
    extern uint16_t BootloaderCan_get_alias(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_DRIVERS_CANBUS_BOOTLOADER_CAN__ */
