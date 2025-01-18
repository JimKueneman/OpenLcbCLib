/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file turnoutboss_event_handler.h
 *
 * Typedefs and various constants for the library.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __TURNOUTOBOSS_EVENT_HANDLER__
#define	__TURNOUTOBOSS_EVENT_HANDLER__

#ifdef PLATFORMIO
#include "src/openlcb/openlcb_types.h"
#else
#include "../../../openlcb/openlcb_types.h"
#endif

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    extern void TurnoutBoss_Event_Handler_register_events(openlcb_node_t *_node);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __TURNOUTOBOSS_EVENT_HANDLER__ */

