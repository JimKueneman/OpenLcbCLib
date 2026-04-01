/**
 * @file    app_header.c
 * @author  Jim Kueneman
 * @date    29 Mar 2026
 *
 * @brief   Application image header for OpenLCB standalone bootloader CRC validation.
 *
 * @details Places one @c app_header_image_t instance into the
 *          @c .rodata_custom_desc linker section.  ESP-IDF places this section
 *          immediately after @c esp_app_desc_t in the DROM segment, at a fixed
 *          file offset of 0x120 in the firmware.bin image:
 *
 *            0x00   esp_image_header_t          (24 bytes)
 *            0x18   esp_image_segment_header_t  (8 bytes)
 *            0x20   esp_app_desc_t              (256 bytes)
 *            0x120  .rodata_custom_desc         <-- this struct
 *
 *          Because the struct is placed by the linker, it is included in
 *          the build-time XOR checksum and SHA256 hash automatically.
 *          The post-link tool (hex2bin.py --arch esp32) patches the CRC
 *          fields and then recomputes the ESP-IDF image checksum and hash.
 *
 *          On every cold boot the bootloader reads this struct from the
 *          inactive OTA partition via esp_partition_read() and verifies:
 *          -# @c app_size is within the available flash region.
 *          -# @c checksum_pre matches the CRC-16-IBM (triple variant) of all
 *             bytes from partition start to the byte before this struct.
 *          -# @c checksum_post matches the CRC-16-IBM (triple variant) of all
 *             bytes immediately after this struct to the end of the image.
 *
 *          All fields are zero-initialised and @b must be patched by the
 *          post-link CRC tool before the image is uploaded.
 *
 *          @par Development bypass
 *          Define @c NO_CHECKSUM in bootloader_types.h to skip CRC
 *          validation at boot time.
 *
 *          @par Struct compatibility
 *          The local @c app_header_image_t mirrors @c bootloader_app_header_t
 *          from bootloader/src/openlcb/bootloader_types.h.  If that library
 *          struct changes, update this file to match.
 *
 *          @par C file requirement
 *          This must be a .c file, not .cpp.  There is a known ESP-IDF bug
 *          where __attribute__((section(".rodata_custom_desc"))) is ignored
 *          in C++ translation units.
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
 * | Field          | Size   | Description                                    |
 * |----------------|--------|------------------------------------------------|
 * | app_size       | 4 B    | Total image bytes from partition start to end  |
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

/** Size guard: fail at compile time if this drifts from the library struct. */
_Static_assert(sizeof(app_header_image_t) == 36U,
               "app_header_image_t size mismatch vs bootloader_app_header_t -- "
               "update APP_HEADER_CHECKSUM_COUNT or field list");

/* -------------------------------------------------------------------------
 * Image header instance
 *
 * Placed by the ESP-IDF linker at file offset 0x120 in the firmware.bin.
 * All checksum fields are zero; the post-link tool must patch them.
 * ------------------------------------------------------------------------- */

/**
 * @brief Application image header in the .rodata_custom_desc section.
 *
 * @details All fields are zero-initialised.  The post-link CRC tool
 *          (hex2bin.py --arch esp32) computes and writes the correct
 *          @c app_size, @c checksum_pre, and @c checksum_post values.
 */
__attribute__((section(".rodata_custom_desc")))
__attribute__((used))
const app_header_image_t app_header = { 0U };
