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
 * \file callbacks.c
 *
 *
 * @author Jim Kueneman
 * @date 11 Nov 2024
 */


#include "callbacks.h"

#include "src/openlcb/openlcb_utilities.h"
#include "src/openlcb/openlcb_gridconnect.h"
#include "src/drivers/canbus/can_types.h"
#include "src/openlcb/openlcb_application_broadcast_time.h"


#define LED_PIN 2

static uint16_t _100ms_ticks = 0;

void Callbacks_initialize(void) {
    


}

void Callbacks_on_100ms_timer_callback(void)
{
    
    OpenLcbApplicationBroadcastTime_100ms_time_tick();

    if (_100ms_ticks > 10) {

        broadcast_clock_state_t* broadcast_state = OpenLcbApplicationBroadcastTime_get_clock(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
        
        if (broadcast_state) {
            
            if (broadcast_state->is_running) {
                
                printf("Time is Running\n");
                printf("Time: %2d:%d, rate: %d\n", broadcast_state->time.hour, broadcast_state->time.minute, broadcast_state->rate.rate);
                
            } else {
                
                printf("Time is Stopped\n");
            }
            
        }
        
        _100ms_ticks = 0;
    }

    _100ms_ticks++;
}

void Callbacks_on_can_rx_callback(can_msg_t *can_msg)
{
    gridconnect_buffer_t gridconnect;

    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);
    printf("[R] %s\n", (char*)&gridconnect);

}

 void Callbacks_on_can_tx_callback(can_msg_t *can_msg)
{

    gridconnect_buffer_t gridconnect;

    OpenLcbGridConnect_from_can_msg(&gridconnect, can_msg);
    printf("[S] %s\n", (char *)&gridconnect);

}

 void Callbacks_alias_change_callback(uint16_t new_alias, node_id_t node_id)
{

    printf("Alias Allocation: 0x%02X  ", new_alias);
    printf("NodeID: 0x%06llX\n\n", node_id);
}

void Callbacks_operations_request_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info)
{

    printf("Factory Reset: NodeID = 0x%06llX\n", OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0));
}

void Callbacks_on_consumed_event_identified(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload) {
    
    printf("Received a produced event identified that we are register as a consumer of: EventID = 0x%08llX\n", *event_id);
    
    if (index == 0xFFFF) {
        
        printf("Within registered Consumer Range\n");
        
    } else {
        
        printf("at index: %d in Node.Consumers.List[]\n", index);
        
    }
    
}

void Callbacks_on_consumed_event_pcer(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload) {
    
    printf("Received a PECR event that we are regstered as a consumer of: EventID = 0x%08llX\n", *event_id);
    
    if (index == 0xFFFF) {
        
        printf("Within registered Consumer Range\n");
        
    } else {
        
        printf("at index: %d in Node.Consumers.List[]\n", index);
        
    }
    
}

void Callbacks_on_event_learn(openlcb_node_t *openlcb_node, event_id_t *event_id) {
    
    printf("Received Event Learn: EventID = 0x%08llX\n", *event_id);
    
}

bool Callbacks_on_login_complete(openlcb_node_t *openlcb_node) {
    
    OpenLcbApplicationBroadcastTime_start(BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
    
    return OpenLcbApplicationBroadcastTime_send_query(openlcb_node);
    
}
