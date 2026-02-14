/** \copyright
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
 * \file mustangpeak_endian_helper.h
 *
 * Implements the core buffers for normal, snip, datagram, and stream length buffers.
 * The FIFO and List buffers are arrays of pointers to these core buffers that are
 * allocated and freed through access.  The CAN Rx and 100ms timer access these buffers
 * so care must be taken to Pause and Resume those calls if the main loop needs to
 * access the buffers.
 *
 * @author Jim Kueneman
 * @date 9 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.

#ifndef __MUSTANGPEAK_ENDIAN_HELPER__
#define __MUSTANGPEAK_ENDIAN_HELPER__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern uint8_t swap_endian8(uint8_t x);

    extern uint16_t swap_endian16(uint16_t x);

    extern uint32_t swap_endian32(uint32_t x);

    extern uint64_t swap_endian64(uint64_t x);
    

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MUSTANGPEAK_ENDIAN_HELPER__ */