/**
*   12 bytes pushed to stack after the trap happens including the handler and where_was_i call.
*   substracting 12 from the current stack pointer will give the source of trap.
*
*   current SP                        +12
*                 push w1             +10
*   where_was_i   push w0             +8
*   rcall in      PC 23->16           +6
*   handler       PC 15->0            +4
*                 PC 23->16           +2
*   TRAP!         PC 15->0            +0
*/
    .section *,bss,near
    .global _trapSrcAddr
_trapSrcAddr:   .space 4
    .text
    .global _where_was_i

_where_was_i:
    push.d w0
    sub w15,#12,w1
    mov [w1++], w0
    mov w0, _trapSrcAddr
    mov [w1], w0
    mov.b WREG, _trapSrcAddr+2
    pop.d w0
    return
