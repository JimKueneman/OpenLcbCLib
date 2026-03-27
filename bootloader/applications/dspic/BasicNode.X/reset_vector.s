; reset_vector.s
;
; Places a GOTO __reset at the application entry point (0x8000) so the
; bootloader can jump here and reach the C runtime startup.

    .section resetvector, address(0x8000), code
    goto __reset
