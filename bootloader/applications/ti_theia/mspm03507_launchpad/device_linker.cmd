/*****************************************************************************

  Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
  Copyright (c) 2026, Jim Kueneman - bootloader flash partitioning

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
 * 0x00000000 - 0x00003FFF  BOOT_FLASH   (16KB)  Bootloader code
 * 0x00004000 - 0x0001FBFF  APP_FLASH    (112KB) Application image
 * 0x0001FC00 - 0x0001FFFF  NODEID_FLASH (1KB)   Node ID (production-programmed)
 */

MEMORY
{
    BOOT_FLASH      (RX)  : origin = 0x00000000, length = 0x00004000
    APP_FLASH       (RX)  : origin = 0x00004000, length = 0x0001BC00
    NODEID_FLASH    (R)   : origin = 0x0001FC00, length = 0x00000400
    SRAM            (RWX) : origin = 0x20200000, length = 0x00008000
    BCR_CONFIG      (R)   : origin = 0x41C00000, length = 0x000000FF
    BSL_CONFIG      (R)   : origin = 0x41C00100, length = 0x00000080
}

SECTIONS
{
    .intvecs:   > 0x00000000
    .text   : palign(8) {} > BOOT_FLASH
    .const  : palign(8) {} > BOOT_FLASH
    .cinit  : palign(8) {} > BOOT_FLASH
    .pinit  : palign(8) {} > BOOT_FLASH
    .rodata : palign(8) {} > BOOT_FLASH
    .ARM.exidx    : palign(8) {} > BOOT_FLASH
    .init_array   : palign(8) {} > BOOT_FLASH
    .binit        : palign(8) {} > BOOT_FLASH
    .TI.ramfunc   : load = BOOT_FLASH, palign(8), run=SRAM, table(BINIT)

    .vtable :   > SRAM
    .args   :   > SRAM
    .data   :   > SRAM
    .bss    :   > SRAM
    .sysmem :   > SRAM
    .stack  :   > SRAM (HIGH)

    .BCRConfig  : {} > BCR_CONFIG
    .BSLConfig  : {} > BSL_CONFIG
}

/*
 * Linker symbols for the bootloader to locate app and node ID regions.
 */
__app_start        = 0x00004000;
__app_end          = 0x0001FBFF;
__app_header_addr  = 0x000040C0;   /* APP_FLASH_START + 0xC0 (after vector table) */
__nodeid_addr      = 0x0001FC00;
