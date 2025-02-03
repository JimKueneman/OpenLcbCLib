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
#ifndef __CALLBACK_HOOKS__
#define	__CALLBACK_HOOKS__

#include "../openlcb/openlcb_types.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    

// Node Alias and Node ID
typedef void (*callback_alias_change_t) (uint16_olcb_t, uint64_olcb_t);

// Incoming Event
typedef void (*callback_event_identified_t)(openlcb_node_t*, event_id_t*);

// Incoming PCR Event
typedef void (*callback_event_pc_report_t)(openlcb_node_t*, event_id_t*);

// Incoming PCR Event with Payload
typedef void (*callback_event_pc_report_with_payload_t)(openlcb_node_t*, event_id_t*, uint8_olcb_t count, event_payload_t* payload);

// Configuration Memory Write occured
typedef void (*callback_config_mem_write_t)(uint32_olcb_t address, uint8_olcb_t data_count, configuration_memory_buffer_t* config_mem_buffer);

/**
* When a node is created and it allocates and alias ID (or if it detects a collision and 
*        allocates a new alias ID this callback is called with the new value)
*/

extern void Application_Callbacks_set_alias_change(callback_alias_change_t alias_change_callback);
extern callback_alias_change_t Application_Callbacks_get_alias_change(void);

// Event Consumer Identified
extern void Application_Callbacks_set_event_consumer_identified_unknown(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_consumer_identified_unknown(void);

extern void Application_Callbacks_set_event_consumer_identified_clear(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_consumer_identified_clear(void);

extern void Application_Callbacks_set_event_consumer_identified_set(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_consumer_identified_set(void);

// Event Producer Identified
extern void Application_Callbacks_set_event_producer_identified_unknown(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_producer_identified_unknown(void);

extern void Application_Callbacks_set_event_producer_identified_clear(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_producer_identified_clear(void);

extern void Application_Callbacks_set_event_producer_identified_set(callback_event_identified_t event_identified_callback);
extern callback_event_identified_t Application_Callbacks_get_producer_identified_set(void);

extern void Application_Callbacks_set_event_pc_report(callback_event_pc_report_t event_pc_report_callback);
extern callback_event_pc_report_t Application_Callbacks_get_event_pc_report(void);

extern void Application_Callbacks_set_event_pc_report_with_payload(callback_event_pc_report_with_payload_t event_pc_report_callback);
extern callback_event_pc_report_with_payload_t Application_Callbacks_get_event_pc_report_with_payload(void);



extern void Application_Callbacks_set_config_mem_write(callback_config_mem_write_t config_mem_write_callback);
extern callback_config_mem_write_t Application_Callbacks_get_config_mem_write(void);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CALLBACK_HOOKS__ */

