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
    /* CAN frame type                                                      */
    /* ================================================================== */

        /** Minimal CAN frame structure. Bit 31 of can_id = EFF flag. */
    typedef struct {

        uint32_t can_id;
        uint8_t can_dlc;
        uint8_t data[8];

    } bootloader_can_frame_t;

    /* ================================================================== */
    /* Application header (embedded in firmware image at linker address)    */
    /* ================================================================== */

    /** Number of 32-bit words in one checksum block. */
    #define BOOTLOADER_CHECKSUM_COUNT 4

        /**
         *     Application header embedded in the firmware image.
         *
         *     The linker places this struct at a known address in the application
         *     flash region. A post-link tool computes the checksums and patches
         *     them into the binary.
         *
         *     checksum_pre covers bytes from flash_min to the start of this struct.
         *     checksum_post covers bytes from end of this struct to app_size.
         */
    typedef struct bootloader_app_header {

        uint32_t app_size;
        uint32_t checksum_pre[BOOTLOADER_CHECKSUM_COUNT];
        uint32_t checksum_post[BOOTLOADER_CHECKSUM_COUNT];

    } bootloader_app_header_t;

    /* ================================================================== */
    /* LED function identifiers (must precede DI struct that references it) */
    /* ================================================================== */

        /** LED function identifiers. */
    typedef enum {

        BOOTLOADER_LED_ACTIVE     = 1,
        BOOTLOADER_LED_WRITING    = 2,
        BOOTLOADER_LED_CSUM_ERROR = 8,
        BOOTLOADER_LED_REQUEST    = 16,
        BOOTLOADER_LED_FRAME_LOST = 32

    } bootloader_led_enum;

    /* ================================================================== */
    /* Bootloader request reason (returned by is_bootloader_requested)     */
    /* ================================================================== */

        /**
         *     Why the bootloader should stay in bootloader mode (or not).
         *
         *     NOT_REQUESTED     -- normal boot, jump to application if valid.
         *     REQUESTED_BY_APP  -- the application set the magic value and reset.
         *                         The CT already sent Freeze, so the bootloader
         *                         starts with firmware_active = 1 (transfer in
         *                         progress) and the CT does not need to send
         *                         Freeze again.
         *     REQUESTED_BY_BUTTON -- the user held a hardware button at power-on.
         *                         The bootloader starts as a normal node with
         *                         firmware_active = 0.  The CT must send Freeze
         *                         before it can transfer firmware.
         */
    typedef enum {

        BOOTLOADER_NOT_REQUESTED      = 0,
        BOOTLOADER_REQUESTED_BY_APP   = 1,
        BOOTLOADER_REQUESTED_BY_BUTTON = 2

    } bootloader_request_t;

    /* ================================================================== */
    /* Forward declarations and typedefs for DI struct parameters          */
    /* ================================================================== */

    struct bootloader_app_header;

        /** Function pointer type for computing a checksum over a flash region.
         *  flash_addr is a 32-bit flash address (PC units on dsPIC33, byte
         *  address on Von Neumann targets).  size has the same units.
         *  Using uint32_t avoids truncation on Harvard targets where
         *  uintptr_t / void* are 16-bit (e.g. XC16 / dsPIC33). */
    typedef void (*compute_checksum_func_t)(uint32_t flash_addr, uint32_t size, uint32_t *checksum);

    /* ================================================================== */
    /* DI structs                                                          */
    /* ================================================================== */

        /** CAN physical layer driver -- used by RX and TX engines. */
    typedef struct {

        bool (*read_received_frame)(bootloader_can_frame_t *frame);
        bool (*try_send_frame)(const bootloader_can_frame_t *frame);
        uint16_t (*get_cached_alias)(void);

    } bootloader_can_driver_t;

        /** OpenLCB driver -- used by CAN SM, OpenLCB SM, bootloader.c. */
    typedef struct {

        uint64_t (*get_persistent_node_id)(void);
        uint8_t (*get_100ms_timer_tick)(void);
        void (*set_status_led)(bootloader_led_enum led, bool state);
        bootloader_request_t (*is_bootloader_requested)(void);
        void (*jump_to_application)(void);
        void (*reboot)(void);
        void (*initialize_hardware)(bootloader_request_t request);
        /* Flash address parameters are always uint32_t to avoid silent
         * truncation on Harvard targets (e.g. dsPIC33/XC16) where
         * uintptr_t and void* are only 16-bit.  Von Neumann platforms
         * cast to (const void *)(uintptr_t) inside their implementations. */
        void (*get_flash_boundaries)(uint32_t *flash_min, uint32_t *flash_max, uint32_t *app_header);
        void (*get_flash_page_info)(uint32_t address, uint32_t *page_start, uint32_t *page_length_bytes);
        uint16_t (*erase_flash_page)(uint32_t address);
        uint16_t (*write_flash_bytes)(uint32_t address, const uint8_t *data, uint32_t size_bytes);
        uint16_t (*finalize_flash)(compute_checksum_func_t compute_checksum_helper);
        compute_checksum_func_t compute_checksum;

            /**
             *     Copy bytes from flash into a RAM buffer.
             *
             *     Von Neumann (STM32, MSPM0, most ARM): flash is memory-mapped
             *     and readable via a normal data pointer.  Implement as:
             *         memcpy(dest_ram, (const void *)(uintptr_t)flash_addr, size_bytes);
             *
             *     Harvard (dsPIC33): program flash and data SRAM are SEPARATE
             *     address spaces.  A data pointer cannot legally address flash --
             *     dereferencing one reads unrelated SRAM instead.  Implement
             *     using the TBLRDL/TBLRDH table-read instructions, exposed by
             *     MCC as FLASH_ReadWord24().  See bootloader_drivers_openlcb.c
             *     in the dsPIC demo for the full implementation.
             *
             *     This DI slot is why the library never calls memcpy() directly
             *     on flash pointers and never dereferences a flash address as a
             *     data pointer.  Routing all flash reads through this function
             *     pointer lets a single bootloader.c compile and run correctly
             *     on both Von Neumann and Harvard targets with no #ifdefs in the
             *     library source.
             *
             *     @param flash_addr  source address in program flash (always
             *                        32-bit, even on platforms where pointers
             *                        are narrower, to avoid truncation)
             *     @param dest_ram    destination buffer in data SRAM
             *     @param size_bytes  number of bytes to copy
             */
        void (*read_flash_bytes)(uint32_t flash_addr, void *dest_ram, uint32_t size_bytes);

            /** @brief Tear down all peripherals and core state before handing off to the other binary. */
        void (*cleanup_before_handoff)(void);

    } bootloader_openlcb_driver_t;

    /* ================================================================== */
    /* Feature flags -- uncomment to disable optional features             */
    /* ================================================================== */

    /**
     *     Uncomment NO_CHECKSUM to bypass application image checksum
     *     validation -- both at boot time and after a firmware write.
     *
     *     WARNING: The bootloader will jump to any image in flash without
     *     verifying it is valid.  Use only during development or on
     *     platforms where the post-link checksum tool has not yet been
     *     integrated.
     */
    /* #define NO_CHECKSUM */

    /* ================================================================== */
    /* SNIP string defaults -- override with -D compiler flags per platform */
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

        /** Write buffer size -- one datagram payload worth of data. */
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
    #define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT 0x86
    #define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT 0x87
    #define CONFIG_MEM_OPTIONS_REPLY                  0x82
    #define CONFIG_OPTIONS_WRITE_LENGTH_RESERVED      (0x80 | 0x40 | 0x20 | 0x02)
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

    /* Temporary error codes (names from openlcb_defines.h). */
    #define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    #define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END    0x2042

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

        /** Write buffer for accumulating firmware data from datagrams.
         * Must be 8-byte aligned because the flash programming API
         * (DL_FlashCTL_programMemoryFromRAM64WithECCGenerated) reads
         * the source data as 64-bit words via a uint32_t* pointer. */
        uint8_t write_buffer[BOOTLOADER_WRITE_BUFFER_SIZE] __attribute__((aligned(8)));
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

    /* ================================================================== */
    /* Application-to-bootloader drop-back ("magic value" handshake)       */
    /* ================================================================== */

    /*
     * The shared RAM definitions (BOOTLOADER_REQUEST_MAGIC, bootloader_request_flag,
     * bootloader_cached_alias) live in a standalone header/source pair:
     *
     *   shared/bootloader_shared_ram.h  -- #define and extern declarations
     *   shared/bootloader_shared_ram.c  -- variable definitions with .noinit attr
     *
     * The application includes bootloader_shared_ram.h directly (no need to
     * pull in all of bootloader_types.h).  The bootloader project should also
     * add the shared/ folder to its include path so that any source file that
     * needs the magic value or shared variables can include it.
     *
     * Both projects compile bootloader_shared_ram.c and both linker scripts
     * map .noinit to the same fixed SRAM address (SHARED_NOINIT) so the two
     * binaries see the same physical memory.
     *
     * Test builds define these symbols in bootloader_test_mocks.c instead.
     */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_LIB_OPENLCB_BOOTLOADER_TYPES__ */
