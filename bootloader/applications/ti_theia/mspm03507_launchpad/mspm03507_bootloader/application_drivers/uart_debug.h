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
 * @file uart_debug.h
 *
 * Temporary UART debug output over the XDS110 backchannel.
 * Include this header and call UartDebug_putchar / UartDebug_print
 * wherever you need visibility.  Remove when done debugging.
 *
 * @author Jim Kueneman
 * @date 24 Mar 2026
 */

#ifndef __UART_DEBUG__
#define __UART_DEBUG__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     *     Transmit a single character over UART0 (blocking).
     *
     *     @param c character to send
     */
    extern void UartDebug_putchar(char c);

    /**
     *     Transmit a null-terminated string over UART0 (blocking).
     *
     *     @param s string to send
     */
    extern void UartDebug_print(const char *s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_DEBUG__ */
