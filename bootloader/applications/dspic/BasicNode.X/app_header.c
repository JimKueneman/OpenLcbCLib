/**
 * @file    app_header.c
 * @author  Jim Kueneman
 * @date    27 Mar 2026
 *
 * @brief   Application image header for OpenLCB standalone bootloader CRC validation.
 *
 * @details Places one @c app_header_image_t instance into the @c .app_header
 *          linker section, which @c p33EP512MC506_BasicNode.gld forces to
 *          PC address 0x4004 (= @c __CODE_BASE + 0x4, immediately after the
 *          GOTO instruction at the application entry point).
 *
 *          On every cold boot the bootloader reads this struct and verifies:
 *          -# @c app_size is within the available flash region.
 *          -# @c checksum_pre matches the CRC-16-IBM (triple variant) of all
 *             flash bytes from @c APP_FLASH_START to the byte immediately
 *             before this struct (the pre-header region).
 *          -# @c checksum_post matches the CRC-16-IBM (triple variant) of all
 *             flash bytes immediately after this struct to the end of the image
 *             (the post-header region).
 *
 *          All three checksum fields are initialised to zero here and
 *          @b must be patched by a post-link CRC tool before the image is
 *          programmed into production flash.
 *
 *          @par Development bypass
 *          Define @c NO_CHECKSUM in the @c BasicNodeBootloader project
 *          (uncomment the line in
 *          @c openlcb_bootloader_src/openlcb/bootloader_types.h) to skip
 *          CRC validation at boot time.
 *
 *          @par Struct compatibility
 *          The local @c app_header_image_t type mirrors
 *          @c bootloader_app_header_t from
 *          @c bootloader/src/openlcb/bootloader_types.h.
 *          If that library struct changes (field order, array size, or
 *          @c BOOTLOADER_CHECKSUM_COUNT), update this file to match.
 *
 *          @par dsPIC Harvard note
 *          The @c space(prog) attribute is required on dsPIC33 (Harvard
 *          architecture) to place const data in program flash rather than
 *          the PSV (Program Space Visibility) window in data memory.
 */

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Local mirror of bootloader_app_header_t.
 *
 * Must match bootloader/src/openlcb/bootloader_types.h exactly:
 *   BOOTLOADER_CHECKSUM_COUNT = 4
 *   struct bootloader_app_header { app_size, checksum_pre[4], checksum_post[4] }
 * ------------------------------------------------------------------------- */

/** Number of CRC words per checksum array -- must equal BOOTLOADER_CHECKSUM_COUNT. */
#define APP_HEADER_CHECKSUM_COUNT  4U

/**
 * @brief Local mirror of @c bootloader_app_header_t.
 *
 * @details
 * | Field          | Size   | Description                                    |
 * |----------------|--------|------------------------------------------------|
 * | app_size       | 4 B    | Total image bytes from flash_min to image end  |
 * | checksum_pre   | 16 B   | Triple CRC-16-IBM of the pre-header region     |
 * | checksum_post  | 16 B   | Triple CRC-16-IBM of the post-header region    |
 *
 * Total: 36 bytes.
 */
typedef struct {
    uint32_t app_size;
    uint32_t checksum_pre[APP_HEADER_CHECKSUM_COUNT];
    uint32_t checksum_post[APP_HEADER_CHECKSUM_COUNT];
} app_header_image_t;

/** Size guard: fail at compile time if this drifts from the library struct.
 *  XC16 does not support C11 _Static_assert, so use the negative-array-size trick. */
typedef char _check_app_header_size[(sizeof(app_header_image_t) == 36U) ? 1 : -1];

/* -------------------------------------------------------------------------
 * Image header instance
 *
 * Placed by the linker at PC 0x4004 (binary-byte address 0x8008).
 * All checksum fields are zero; a post-link tool must patch them
 * before production programming.
 * ------------------------------------------------------------------------- */

/**
 * @brief Application image header placed at PC 0x4004.
 *
 * @details All fields are zero-initialised.  The post-link CRC tool must
 *          compute and write the correct @c app_size, @c checksum_pre, and
 *          @c checksum_post values before the image is flashed.
 */
__attribute__((section(".app_header"), space(prog)))
__attribute__((used))
const app_header_image_t app_header = { 0U };
