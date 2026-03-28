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
 *          @par Struct sizing
 *          The local @c app_header_image_t is a 27-byte opaque array that
 *          produces exactly 9 dsPIC instruction words in @c space(prog).
 *          This matches the 36 binary-byte footprint of
 *          @c bootloader_app_header_t (9 instructions x 4 binary bytes).
 *          If the library struct changes size, update the array size here
 *          to @c ceil(new_size / 4) * 3.
 *
 *          @par dsPIC Harvard note
 *          The @c space(prog) attribute is required on dsPIC33 (Harvard
 *          architecture) to place const data in program flash rather than
 *          the PSV (Program Space Visibility) window in data memory.
 */

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Flash reservation for bootloader_app_header_t.
 *
 * On dsPIC with space(prog), XC16 packs 3 data bytes per instruction word.
 * The bootloader_app_header_t is 36 bytes and occupies 36 binary bytes in
 * the firmware image (9 instructions x 4 binary bytes each).
 *
 * Each uint32_t field aligns with one instruction.  The phantom byte (4th
 * byte of each instruction, always 0x00) lands on the MSB of each uint32_t,
 * which is naturally 0x00 for all header fields (app_size < 16 MB, CRC-16
 * values fit in 16 bits, reserved slots are zero).
 *
 * To produce exactly 9 instructions, this struct is declared as a 27-byte
 * opaque array (9 instructions x 3 data bytes).  The post-link tool
 * (hex2bin.py) patches the 36 flat binary bytes at the correct offset;
 * write_flash_bytes discards the phantom bytes, leaving the correct 27
 * data bytes in flash.
 *
 * The bootloader reads back with _read_flash_image_bytes (4 bytes per
 * instruction) and reconstructs the original 36-byte struct.
 * ------------------------------------------------------------------------- */

/**
 * @brief Flash reservation sized to produce exactly 9 instruction words.
 *
 * @details 9 instructions x 3 data bytes = 27 bytes.  The phantom byte
 *          per instruction is implicit in space(prog) encoding.
 */
typedef struct {

    uint8_t raw[27];

} app_header_image_t;

/** Size guard: must produce exactly 9 instructions (27 data bytes). */
typedef char _check_app_header_size[(sizeof(app_header_image_t) == 27U) ? 1 : -1];

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
const app_header_image_t app_header = {{ 0U }};
