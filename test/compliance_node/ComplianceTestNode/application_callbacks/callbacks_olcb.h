
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
 * \file callbacks_olcb.h
 *
 * Core node lifecycle callback declarations for the ComplianceTestNode.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#ifndef __CALLBACKS_OLCB__
#define __CALLBACKS_OLCB__

#include <stdbool.h>

#include "../openlcb_c_lib/openlcb/openlcb_types.h"
#include "../protocol_modes.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void CallbacksOlcb_set_active_mode(const protocol_mode_t *mode);

    extern void CallbacksOlcb_on_100ms_timer(void);

    extern bool CallbacksOlcb_on_login_complete(openlcb_node_t *openlcb_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CALLBACKS_OLCB__ */
