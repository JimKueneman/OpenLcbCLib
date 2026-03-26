/*
******************************************************************************
**
** @file        : STM32F407VGTX_APPLICATION.ld
**
** @author      : Jim Kueneman
**
**  Abstract    : Linker script for the OpenLCB BasicNode application on the
**                STM32F407G-DISC1 board (STM32F407VGTx).
**
**                Flash layout (STM32F407VG, 1MB total):
**                  Sectors 0-1  (32KB)              : bootloader (not used here)
**                  Sector  2    (0x08008000)         : APPLICATION START ← this binary
**                  0x08008200                        : bootloader_app_header_t
**                  Sectors 2-10 (32KB+64KB+768KB)    : application code/data (864KB)
**                  Sector  11   (128KB, 0x080E0000)  : node ID storage (excluded)
**
**                bootloader_app_header_t is placed at APP_FLASH_START + 0x200
**                (= 0x08008200), immediately after the Cortex-M4 vector table
**                (98 entries x 4 = 392 bytes).  A post-link tool must patch the
**                checksum fields before the image is programmed so that the
**                bootloader can validate the image on every power-up.
**
**                RAM layout (128KB at 0x20000000):
**                  0x20000000 – 0x2001FFEF  general RAM (128KB - 16B)
**                  0x2001FFF0 – 0x2001FFFF  SHARED_NOINIT (16B)
**                    bootloader_request_flag  (4B) – magic handshake value
**                    bootloader_cached_alias  (2B) – CAN alias for fast re-entry
**                    (6B padding)
**
**                SHARED_NOINIT is mapped identically in both this linker
**                script and STM32F407VGTX_BOOTLOADER.ld so both binaries
**                see the same physical SRAM addresses after a soft reset.
**
**                DO NOT use STM32F407VGTX_FLASH.ld for this project.
**                CubeIDE regenerates that file; this custom script is
**                preserved across code-generation runs.
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Initial stack pointer: top of general RAM, just below SHARED_NOINIT */
_estack = ORIGIN(RAM) + LENGTH(RAM);

_Min_Heap_Size  = 0x200;   /* 512 B heap  */
_Min_Stack_Size = 0x400;   /* 1 KB stack  */

/* ========================================================================= */
/* Memory regions                                                             */
/* ========================================================================= */
MEMORY
{
  CCMRAM        (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
  RAM           (xrw) : ORIGIN = 0x20000000, LENGTH = 0x1FFF0   /* 128KB - 16B  */
  SHARED_NOINIT (xrw) : ORIGIN = 0x2001FFF0, LENGTH = 0x10      /* 16B handshake */
  FLASH          (rx) : ORIGIN = 0x08008000, LENGTH = 864K       /* sectors 2-10  */
}

/* ========================================================================= */
/* Sections                                                                   */
/* ========================================================================= */
SECTIONS
{
  /* Vector table — the bootloader sets SCB->VTOR = 0x08008000 before
   * jumping here so the CPU finds these vectors after the handoff. */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Application header for bootloader CRC validation.
   *
   * Fixed at APP_FLASH_START + 0x200 = 0x08008200, which is 512 bytes
   * past the application start.  The Cortex-M4 vector table occupies
   * at most 98 entries x 4 bytes = 392 bytes (0x188), so there is no
   * overlap.  The 120-byte gap between the end of the vector table and
   * this section is left as 0xFF (erased flash) and is included in the
   * pre-checksum region.
   *
   * The bootloader_app_header_t struct must be defined in the application
   * with __attribute__((section(".app_header"))).  The post-link tool
   * patches the checksum fields before programming. */
  . = 0x08008200;
  .app_header :
  {
    KEEP(*(.app_header))
    . = ALIGN(4);
  } >FLASH

  /* Code */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  /* Read-only data */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) :
  {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM (READONLY) :
  {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array (READONLY) :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Initialized data: load from FLASH, run in RAM */
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> FLASH

  _siccmram = LOADADDR(.ccmram);
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;
    *(.ccmram)
    *(.ccmram*)
    . = ALIGN(4);
    _eccmram = .;
  } >CCMRAM AT> FLASH

  /* Zero-initialised data */
  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* Heap and stack size guard */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Bootloader <-> application handshake variables.
   * NOLOAD: the startup code must NOT zero-initialise this region.
   * The application writes bootloader_request_flag and
   * bootloader_cached_alias here before calling HAL_NVIC_SystemReset(),
   * and the bootloader reads them on the next startup. */
  .noinit (NOLOAD) :
  {
    KEEP(*(.noinit))
  } >SHARED_NOINIT

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
