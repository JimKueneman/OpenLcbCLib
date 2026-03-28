; reset_vector.s
;
; Places a GOTO __reset at the application entry point (__CODE_BASE, typically
; 0x4000) so the bootloader can jump here and reach the C runtime startup.
;
; The linker script maps this .resetvector input section to the .app_reset
; output section at __CODE_BASE.  Using the linker script for placement
; (rather than the assembler address() attribute) ensures the data appears
; in the hex file output.

    .section .resetvector, code
    goto __reset
