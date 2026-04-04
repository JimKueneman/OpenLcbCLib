/*****************************************************************************

  Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
  Copyright (c) 2026, Jim Kueneman - application flash partitioning

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

   Neither the name of Texas Instruments Incorporated nor the names of
   its contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/
-uinterruptVectors
--stack_size=512

/*
 * Flash Memory Layout (128KB total):
 *
 * 0x00000000 - 0x00003BFF  (reserved)   Bootloader code — DO NOT TOUCH
 * 0x00003C00 - 0x0001FBFF  APP_FLASH    (113KB) Application image (this binary)
 * 0x0001FC00 - 0x0001FFFF  NODEID_FLASH (1KB)   Node ID (production-programmed)
 *
 * The bootloader occupies the first 15KB of flash and owns the reset
 * vector at 0x00000000.  This application starts at 0x00003C00 with its
 * own vector table.  The bootloader sets VTOR to 0x00003C00 before
 * jumping here.
 *
 * The application header (bootloader_app_header_t) is placed at
 * APP_FLASH + 0xC0 = 0x00003CC0, just after the vector table.  The
 * bootloader reads this to validate the checksum before jumping.
 */

MEMORY
{
    APP_FLASH       (RX)  : origin = 0x00003C00, length = 0x0001C000
    NODEID_FLASH    (R)   : origin = 0x0001FC00, length = 0x00000400
    SRAM            (RWX) : origin = 0x20200000, length = 0x00007FF0
    SHARED_NOINIT   (RW)  : origin = 0x20207FF0, length = 0x00000010  /* bootloader <-> app handshake (magic flag + alias) */
}

SECTIONS
{
    .intvecs:   > 0x00003C00

    /* Application header for bootloader CRC validation.
     *
     * Fixed at APP_FLASH_START + 0xC0 = 0x00003CC0, immediately after the
     * Cortex-M0+ vector table (48 entries x 4 bytes = 192 bytes = 0xC0).
     *
     * The bootloader_app_header_t struct must be defined in the application
     * with __attribute__((section(".app_header"))).  The post-link tool
     * patches the checksum fields before programming. */
    .app_header : fill = 0x00000000 {} > 0x00003CC0

    .text   : palign(8) {} > APP_FLASH
    .const  : palign(8) {} > APP_FLASH
    .cinit  : palign(8) {} > APP_FLASH
    .pinit  : palign(8) {} > APP_FLASH
    .rodata : palign(8) {} > APP_FLASH
    .ARM.exidx    : palign(8) {} > APP_FLASH
    .init_array   : palign(8) {} > APP_FLASH
    .binit        : palign(8) {} > APP_FLASH
    .TI.ramfunc   : load = APP_FLASH, palign(8), run=SRAM, table(BINIT)

    .vtable :   > SRAM
    .args   :   > SRAM
    .data   :   > SRAM
    .bss    :   > SRAM
    .noinit : type = NOINIT {} > SHARED_NOINIT  /* NOT zeroed by C startup — shared with the bootloader at a fixed address */
    .sysmem :   > SRAM
    .stack  :   > SRAM (HIGH)
}

/*
 * Linker symbols for application code to locate flash regions.
 */
__app_start        = 0x00003C00;
__app_header_addr  = 0x00003CC0;   /* APP_FLASH_START + 0xC0 (after vector table) */
__nodeid_addr      = 0x0001FC00;
