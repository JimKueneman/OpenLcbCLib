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
 * @file protocol_datagram_handler.h
 * @brief Datagram protocol handler implementation
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__
#define    __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

typedef struct {

    // Required function assignments
    void (*lock_shared_resources)(void);
    void (*unlock_shared_resources)(void);

    // Optional functions to implement Address Space access to Read Address Spaces, these are general functions that call the request functions defined in interface_protocol_config_mem_read_handler_t
    void (*memory_read_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space read replies (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    void (*memory_read_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space read replies (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    void (*memory_read_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space access to Read Address Spaces through a Stream
    void (*memory_read_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space read replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    void (*memory_read_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space read replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    void (*memory_read_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_read_stream_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space access to Write Address Spaces, these are general functions that call the request functions defined in interface_protocol_config_mem_write_handler_t
    void (*memory_write_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);           // Typically NULL as this a a read only space
    void (*memory_write_space_all)(openlcb_statemachine_info_t *statemachine_info);                               // Typically NULL as this a a read only space
    void (*memory_write_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);                 // Typically NULL as this a a read only space
    void (*memory_write_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info); // Typically NULL as this a a read only space
    void (*memory_write_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space write replies (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    void (*memory_write_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);  // Typically never called as this a a read only space
    void (*memory_write_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);                      // Typically never called as this a a read only space
    void (*memory_write_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);        // Typically never called as this a a read only space
    void (*memory_write_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info); // Typically never called as this a a read only space
    void (*memory_write_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space write replies (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    void (*memory_write_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space write under a mask
    void (*memory_write_under_mask_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);           // Typically NULL as this a a read only space
    void (*memory_write_under_mask_space_all)(openlcb_statemachine_info_t *statemachine_info);                               // Typically NULL as this a a read only space
    void (*memory_write_under_mask_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);                 // Typically NULL as this a a read only space
    void (*memory_write_under_mask_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info); // Typically NULL as this a a read only space
    void (*memory_write_under_mask_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_under_mask_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space access to Write Address Spaces through a Stream
    void (*memory_write_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);              // Typically NULL as this a a read only space
    void (*memory_write_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);                                  // Typically NULL as this a a read only space
    void (*memory_write_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);                    // Typically NULL as this a a read only space
    void (*memory_write_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);    // Typically NULL as this a a read only space
    void (*memory_write_stream_space_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space write replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the OK reply)
    void (*memory_write_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);    // Typically never called as this a a read only space
    void (*memory_write_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);                        // Typically never called as this a a read only space
    void (*memory_write_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);          // Typically never called as this a a read only space
    void (*memory_write_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info); // Typically never called as this a a read only space
    void (*memory_write_stream_space_traction_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement Address Space write replies through a Stream (Only required if the node is requesting a datagram from some other node and this is the FAIL reply)
    void (*memory_write_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_write_stream_space_traction_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

    // Optional functions to implement the commands in the Configuration Memory Operations, typically assigned the handlers in interface_protocol_config_mem_operations_handler_t.
    void (*memory_options_cmd)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_options_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reserve_lock)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_unique_id)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_unfreeze)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_freeze)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_update_complete)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_reset_reboot)(openlcb_statemachine_info_t *statemachine_info);
    void (*memory_factory_reset)(openlcb_statemachine_info_t *statemachine_info);

} interface_protocol_datagram_handler_t;


typedef void(*memory_handler_t)(openlcb_statemachine_info_t *statemachine_info);

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the Protocol Datagram Handler module
     * @param interface_protocol_datagram_handler Pointer to the interface structure containing required callbacks
     * @return None
     */
    extern void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler);

    /**
     * @brief Loads a datagram received OK acknowledgment message
     * @param statemachine_info Pointer to the statemachine information structure
     * @param reply_pending_time_in_seconds Time in seconds (as 2^N) until reply will be sent
     * @return None
     */
    extern void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

    /**
     * @brief Loads a datagram rejected message with error code
     * @param statemachine_info Pointer to the statemachine information structure
     * @param return_code Error code indicating reason for rejection
     * @return None
     */
    extern void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

    /**
     * @brief Processes an incoming datagram message
     * @param statemachine_info Pointer to the statemachine information structure
     * @return None
     */
    extern void ProtocolDatagramHandler_datagram(openlcb_statemachine_info_t * statemachine_info);

    /**
     * @brief Handles datagram received OK acknowledgment
     * @param statemachine_info Pointer to the statemachine information structure
     * @return None
     */
    extern void ProtocolDatagramHandler_datagram_received_ok(openlcb_statemachine_info_t * statemachine_info);

    /**
     * @brief Handles datagram rejected response
     * @param statemachine_info Pointer to the statemachine information structure
     * @return None
     */
    extern void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t * statemachine_info);

    /**
     * @brief Clears the resend datagram message flag for the specified node
     * @param openlcb_node Pointer to the OpenLcb node
     * @return None
     */
    extern void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t * openlcb_node);

    /**
     * @brief 100ms timer tick handler for datagram protocol timeouts
     * @param None
     * @return None
     */
    extern void ProtocolDatagramHandler_100ms_timer_tick(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__ */

