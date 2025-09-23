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
 * \file application.h
 *
 * Where most of the application layer code interfaces with the library
 *
 * @author Jim Kueneman
 * @date 16 Jan 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_APPLICATION__
#define	__OPENLCB_APPLICATION__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {
    
    bool (*transmit_openlcb_message)(openlcb_msg_t* openlcb_msg);
    uint16_t (*configuration_memory_read)(uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);
    uint16_t (*configuration_memory_write) (uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

} interface_openlcb_application_t;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    extern void OpenLcbApplication_initialize(const interface_openlcb_application_t *interface_openlcb_application);

    // Event ID helpers
    extern void OpenLcbApplication_clear_consumer_eventids(openlcb_node_t* node);

    extern void OpenLcbApplication_clear_producer_eventids(openlcb_node_t* node);

    extern uint16_t OpenLcbApplication_register_consumer_eventid(openlcb_node_t* node, event_id_t event_id, event_status_enum event_status);

    extern uint16_t OpenLcbApplication_register_producer_eventid(openlcb_node_t* node, event_id_t event_id, event_status_enum event_status);

    extern bool OpenLcbApplication_send_event_pc_report(openlcb_node_t* node, event_id_t event_id);
    
    extern bool OpenLcbApplication_send_teach_event(openlcb_node_t* node, event_id_t event_id);


    // Configuration Memory helpers
    extern uint16_t OpenLcbApplication_read_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);

    extern uint16_t OpenLcbApplication_write_configuration_memory(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_APPLICATION__ */

