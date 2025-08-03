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
 * \file callback_hooks.c
 *
 * Where most of the application layer code will hook into in order to respond to
 * messages that an app will need.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include "openlcb_application_callbacks.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"


callback_alias_change_t callback_alias_change_func = NULL;
callback_event_identified_t callback_event_consumer_identified_unknown_func = NULL;
callback_event_identified_t callback_event_consumer_identified_clear_func = NULL;
callback_event_identified_t callback_event_consumer_identified_set_func = NULL;
callback_event_identified_t callback_event_producer_identified_unknown_func = NULL;
callback_event_identified_t callback_event_producer_identified_clear_func = NULL;
callback_event_identified_t callback_event_producer_identified_set_func = NULL;
callback_event_pc_report_t callback_event_pc_report_func = NULL;
callback_event_pc_report_with_payload_t callback_event_pc_report_with_payload_func = NULL;
callback_event_identified_t callback_event_learn_func = NULL;
callback_config_mem_write_t callback_config_mem_write_func = NULL;
callback_config_mem_freeze_t callback_config_mem_freeze_firmware_update_func = NULL;
callback_config_mem_freeze_t callback_config_mem_unfreeze_firmware_update_func = NULL;
parameterless_callback_t callback_can_rx_func = NULL;
parameterless_callback_t callback_can_tx_func = NULL;
parameterless_callback_t callback_100ms_timer_func = NULL;


void OpenLcbApplicationCallbacks_set_100ms_timer(parameterless_callback_t callback_100ms_timer) {
    
   callback_100ms_timer_func = callback_100ms_timer;
   
}

parameterless_callback_t OpenLcbApplicationCallbacks_get_100ms_timer(void) {
    
    return callback_100ms_timer_func;
    
}

void OpenLcbApplicationCallbacks_set_can_tx(parameterless_callback_t callback_can_tx) {
    
   callback_can_tx_func = callback_can_tx;
   
}

parameterless_callback_t OpenLcbApplicationCallbacks_get_can_tx(void) {
    
    return callback_can_tx_func;
    
}

void OpenLcbApplicationCallbacks_set_can_rx(parameterless_callback_t callback_can_rx) {
    
    callback_can_rx_func = callback_can_rx;
    
}

parameterless_callback_t OpenLcbApplicationCallbacks_get_can_rx(void) {
    
    return callback_can_rx_func;
    
}


void OpenLcbApplicationCallbacks_set_alias_change(callback_alias_change_t alias_change_callback) {

    callback_alias_change_func = alias_change_callback;

}

callback_alias_change_t OpenLcbApplicationCallbacks_get_alias_change(void) {

    return callback_alias_change_func;

}

void OpenLcbApplicationCallbacks_set_event_consumer_identified_unknown(callback_event_identified_t event_identified_callback) {

    callback_event_consumer_identified_unknown_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_unknown(void) {

    return callback_event_consumer_identified_unknown_func;

}

void OpenLcbApplicationCallbacks_set_event_consumer_identified_clear(callback_event_identified_t event_identified_callback) {

    callback_event_consumer_identified_clear_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_clear(void) {

    return callback_event_consumer_identified_clear_func;

}

void OpenLcbApplicationCallbacks_set_event_consumer_identified_set(callback_event_identified_t event_identified_callback) {

    callback_event_consumer_identified_set_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_consumer_identified_set(void) {

    return callback_event_consumer_identified_set_func;

}

void OpenLcbApplicationCallbacks_set_event_producer_identified_unknown(callback_event_identified_t event_identified_callback) {

    callback_event_producer_identified_unknown_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_unknown(void) {

    return callback_event_producer_identified_unknown_func;

}

void OpenLcbApplicationCallbacks_set_event_producer_identified_clear(callback_event_identified_t event_identified_callback) {

    callback_event_producer_identified_clear_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_clear(void) {

    return callback_event_producer_identified_clear_func;

}

void OpenLcbApplicationCallbacks_set_event_producer_identified_set(callback_event_identified_t event_identified_callback) {

    callback_event_producer_identified_set_func = event_identified_callback;

}

callback_event_identified_t OpenLcbApplicationCallbacks_get_producer_identified_set(void) {

    return callback_event_producer_identified_set_func;

}

void OpenLcbApplicationCallbacks_set_event_pc_report(callback_event_pc_report_t event_pc_report_callback) {

    callback_event_pc_report_func = event_pc_report_callback;

}

callback_event_pc_report_t OpenLcbApplicationCallbacks_get_event_pc_report(void) {

    return callback_event_pc_report_func;

}

void OpenLcbApplicationCallbacks_set_event_pc_report_with_payload(callback_event_pc_report_with_payload_t event_pc_report_callback) {

    callback_event_pc_report_with_payload_func = event_pc_report_callback;

}

callback_event_pc_report_with_payload_t OpenLcbApplicationCallbacks_get_event_pc_report_with_payload(void) {

    return callback_event_pc_report_with_payload_func;

}

void OpenLcbApplicationCallbacks_set_event_learn(callback_event_pc_report_t event_learn_callback) {

    callback_event_learn_func = event_learn_callback;

}

callback_event_pc_report_t OpenLcbApplicationCallbacks_get_event_learn(void) {

    return callback_event_learn_func;

}

void OpenLcbApplication_Callbacks_set_config_mem_write(callback_config_mem_write_t config_mem_write_callback) {
    
    callback_config_mem_write_func = config_mem_write_callback;
    
}

callback_config_mem_write_t OpenLcbApplicationCallbacks_get_config_mem_write(void) {
    
    return callback_config_mem_write_func;
    
}


void OpenLcbApplicationCallbacks_set_config_mem_freeze_firmware_update(callback_config_mem_freeze_t callback_config_mem_freeze) {
  
    callback_config_mem_freeze_firmware_update_func = callback_config_mem_freeze;
    
}

callback_config_mem_freeze_t OpenLcbApplicationCallbacks_get_config_mem_freeze_firmware_update(void) {
    
   return callback_config_mem_freeze_firmware_update_func; 
    
}

void OpenLcbApplicationCallbacks_set_config_mem_unfreeze_firmware_update(callback_config_mem_freeze_t callback_config_mem_unfreeze) {
  
    callback_config_mem_unfreeze_firmware_update_func = callback_config_mem_unfreeze;
    
}

callback_config_mem_freeze_t OpenLcbApplicationCallbacks_get_config_mem_unfreeze_firmware_update(void) {
    
   return callback_config_mem_unfreeze_firmware_update_func; 
    
}