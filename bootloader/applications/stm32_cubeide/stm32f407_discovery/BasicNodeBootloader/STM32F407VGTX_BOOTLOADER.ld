/*
******************************************************************************
**
** @file        : STM32F407VGTX_BOOTLOADER.ld
**
** @author      : Jim Kueneman
**
**  Abstract    : Linker script for the OpenLCB standalone bootloader on the
**                STM32F407G-DISC1 board (STM32F407VGTx).
**
**                Flash layout (STM32F407VG, 1MB total):
**                  Sectors 0-1  (2 x 16KB = 32KB) : BOOTLOADER  ← this binary
**                  Sectors 2-10 (32KB+64KB+768KB)  : application image
**                  Sector  11   (128KB)             : node ID storage (reserved)
**
**                RAM layout (128KB at 0x20000000):
**                  0x20000000 – 0x2001FFEF  general RAM (128KB - 16B)
**                  0x2001FFF0 – 0x2001FFFF  SHARED_NOINIT (16B)
**                    bootloader_request_flag  (4B) – magic handshake value
**                    bootloader_cached_alias  (2B) – CAN alias for fast re-entry
**                    (6B padding)
**
**                SHARED_NOINIT is mapped identically in both this linker
**                script and STM32F407VGTX_APPLICATION.ld so both binaries
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
  RAM           (xrw) : ORIGIN = 0x20000000, LENGTH = 0x1FFF0   /* 128KB - 16B */
  SHARED_NOINIT (xrw) : ORIGIN = 0x2001FFF0, LENGTH = 0x10      /* 16B handshake */
  FLASH          (rx) : ORIGIN = 0x08000000, LENGTH = 32K        /* sectors 0-1  */
}

/* ========================================================================= */
/* Sections                                                                   */
/* ========================================================================= */
SECTIONS
{
  /* Vector table */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
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
   * The variables survive a software reset so the bootloader can read
   * the magic flag and cached CAN alias written by the application. */
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
