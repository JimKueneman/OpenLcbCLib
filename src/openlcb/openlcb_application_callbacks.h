/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * \file callback_hooks.h
 *
 * Where most of the application layer code will hook into in order to respond to
 * messages that an app will need.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_APPLICATION_CALLBACKS__
#define	__OPENLCB_APPLICATION_CALLBACKS__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    


/**
* When a node is created and it allocates and alias ID (or if it detects a collision and 
*        allocates a new alias ID this callback is called with the new value)
*/

extern void OpenLcbApplicationCallbacks_set_alias_change(callback_alias_change_t alias_change_callback);
extern callback_alias_change_t OpenLcbApplicationCallbacks_get_alias_change(void);

// Event Consumer Identified
extern void OpenLcbApplicationCallbacks_set_event_consumer_identified_unknown(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_unknown(void);

extern void OpenLcbApplicationCallbacks_set_event_consumer_identified_clear(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_clear(void);

extern void OpenLcbApplicationCallbacks_set_event_consumer_identified_set(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_set(void);

// Event Producer Identified
extern void OpenLcbApplicationCallbacks_set_event_producer_identified_unknown(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_unknown(void);

extern void OpenLcbApplicationCallbacks_set_event_producer_identified_clear(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_clear(void);

extern void OpenLcbApplicationCallbacks_set_event_producer_identified_set(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_set(void);

extern void OpenLcbApplicationCallbacks_set_event_pc_report(callback_event_pc_report_t event_pc_report_callback);
extern callback_event_pc_report_t OpenLcbApplicationCallbacks_get_event_pc_report(void);

extern void OpenLcbApplicationCallbacks_set_event_pc_report_with_payload(callback_event_pc_report_with_payload_t event_pc_report_callback);
extern callback_event_pc_report_with_payload_t OpenLcbApplicationCallbacks_get_event_pc_report_with_payload(void);

extern void OpenLcbApplicationCallbacks_set_event_learn(callback_event_pc_report_t event_learn_callback);
extern callback_event_pc_report_t OpenLcbApplicationCallbacks_get_event_learn(void);

extern void OpenLcbApplication_Callbacks_set_config_mem_write(callback_config_mem_write_t config_mem_write_callback);
extern callback_config_mem_write_t OpenLcbApplicationCallbacks_get_config_mem_write(void);

extern void OpenLcbApplicationCallbacks_set_config_mem_freeze_firmware_update(callback_config_mem_freeze_t callback_config_mem_freeze);
extern callback_config_mem_freeze_t OpenLcbApplicationCallbacks_get_config_mem_freeze_firmware_update(void);

extern void OpenLcbApplicationCallbacks_set_config_mem_unfreeze_firmware_update(callback_config_mem_freeze_t callback_config_mem_freeze);
extern callback_config_mem_freeze_t OpenLcbApplicationCallbacks_get_config_mem_unfreeze_firmware_update(void);

extern void OpenLcbApplicationCallbacks_set_can_tx(parameterless_callback_t callback_can_tx);
extern parameterless_callback_t OpenLcbApplicationCallbacks_get_can_tx(void);

extern void OpenLcbApplicationCallbacks_set_can_rx(parameterless_callback_t callback_can_rx);
extern parameterless_callback_t OpenLcbApplicationCallbacks_get_can_rx(void);

extern void OpenLcbApplicationCallbacks_set_100ms_timer(parameterless_callback_t callback_100ms_timer);
extern parameterless_callback_t OpenLcbApplicationCallbacks_get_100ms_timer(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_APPLICATION_CALLBACKS__ */

