/** @copyright
 * Copyright (c) 2026, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file bootloader_isr_redirect.h
 *
 * ISR redirect (VIVT) initialization for the dsPIC33 bootloader.
 *
 * Provides strong overrides for the MCC weak trap handlers and a proxy
 * for the TMR2 interrupt handler so that after jump_to_application() all
 * interrupts are routed through bootloader_vivt_jumptable to the application.
 *
 * Call BootloaderIsrRedirect_initialize() once during hardware init.  The
 * _C1Interrupt VIVT redirect is handled directly in can1.c (hand-edited).
 *
 * @author Jim Kueneman
 * @date 25 Mar 2026
 */

#ifndef __BOOTLOADER_ISR_REDIRECT__
#define __BOOTLOADER_ISR_REDIRECT__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * Installs the VIVT proxy as the TMR2 interrupt handler and clears
         * the interrupt_redirect flag.  Call once from initialize_hardware().
         */
    extern void BootloaderIsrRedirect_initialize(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOOTLOADER_ISR_REDIRECT__ */
