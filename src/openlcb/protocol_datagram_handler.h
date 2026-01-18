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
* @brief OpenLCB datagram protocol handler interface
*
* @details This module provides the implementation for the OpenLCB datagram
* transport protocol, which enables reliable transfer of 0-72 bytes of data
* between two nodes. It handles datagram reception, acknowledgment, rejection,
* and timeout management according to the OpenLCB specification.
*
* The datagram handler supports:
* - Configuration memory operations (read/write)
* - Address space access for various memory regions
* - Stream-based memory operations
* - Write-under-mask operations
* - Firmware upgrade operations
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see OpenLCB Datagram Transport Specification
* @see OpenLCB Configuration Memory Operations Specification
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__
#define __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
    * @brief Interface structure for datagram protocol handler callbacks
    *
    * @details This structure contains function pointers for all supported
    * datagram operations. Required callbacks must be implemented; optional
    * callbacks may be set to NULL if the functionality is not needed.
    *
    * The structure organizes callbacks into functional groups:
    * - Resource locking (required)
    * - Memory read operations (datagram-based)
    * - Memory read operations (stream-based)
    * - Memory write operations (datagram-based)
    * - Memory write operations (stream-based)
    * - Write-under-mask operations
    * - Configuration memory commands
    * - Reply handlers (for client operations)
    *
    * @note Setting a callback to NULL will result in datagram rejection
    * with ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN
    *
    * @warning lock_shared_resources and unlock_shared_resources are REQUIRED
    *
    * @attention Callbacks are invoked from datagram receive context
    *
    * @see ProtocolDatagramHandler_initialize - Registers this interface
    * @see openlcb_statemachine_info_t - Context passed to all callbacks
    */
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


    /**
    * @brief Type definition for memory operation handler function pointer
    *
    * @details Defines a function pointer type for callbacks that handle
    * specific memory operations within datagram processing. All memory
    * operation handlers use this signature.
    *
    * @note Used throughout the datagram handler interface structure
    *
    * @see interface_protocol_datagram_handler_t - Structure using this type
    */
typedef void (*memory_handler_t)(openlcb_statemachine_info_t *statemachine_info);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
        * @brief Initializes the Protocol Datagram Handler module
        *
        * @details Registers the callback interface that the datagram handler
        * will use to process incoming datagrams and generate responses. This
        * function must be called once during system startup before any datagram
        * processing can occur.
        *
        * The interface structure pointer is stored internally and used for all
        * subsequent datagram operations. The structure must remain valid for
        * the lifetime of the application.
        *
        * Use cases:
        * - Called once during application initialization
        * - Must be called before processing any datagrams
        * - Typically called after node initialization
        *
        * @param interface_protocol_datagram_handler Pointer to initialized interface
        * structure containing all required and optional callback functions
        *
        * @warning The interface structure must remain valid after this call
        * @warning lock_shared_resources and unlock_shared_resources callbacks are required
        *
        * @attention Call before any datagram processing begins
        * @attention Interface pointer is stored but structure is not copied
        *
        * @see interface_protocol_datagram_handler_t - Interface structure definition
        * @see ProtocolDatagramHandler_datagram - Main datagram processor
        */
    extern void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler);

        /**
        * @brief Loads a datagram received OK acknowledgment message
        *
        * @details Constructs an OpenLCB Datagram Received OK reply message
        * (MTI 0x0A28) and places it in the outgoing message buffer. The reply
        * indicates successful datagram reception and optionally specifies a
        * reply-pending timeout value.
        *
        * The timeout value is encoded as an exponent (2^N seconds) according
        * to the OpenLCB datagram specification. A value of 0 indicates no
        * timeout (immediate response expected).
        *
        * Use cases:
        * - Acknowledging successful datagram reception
        * - Indicating deferred processing with reply-pending timeout
        * - Normal datagram protocol flow completion
        *
        * @param statemachine_info Pointer to state machine context containing
        * incoming message details and outgoing message buffer
        * @param reply_pending_time_in_seconds Time in seconds until reply will
        * be sent, encoded as 2^N (range: 0 to 16384 seconds). Value of 0 means
        * no reply pending.
        *
        * @note Outgoing message valid flag is set automatically
        * @note Reply is addressed to the source of the incoming datagram
        * @note Timeout values are rounded up to nearest power of 2
        *
        * @attention statemachine_info must contain valid incoming message
        *
        * @see ProtocolDatagramHandler_load_datagram_rejected_message - For rejecting datagrams
        * @see ProtocolDatagramHandler_datagram_received_ok - Handles received OK replies
        */
    extern void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /**
        * @brief Loads a datagram rejected message with error code
        *
        * @details Constructs an OpenLCB Datagram Rejected reply message
        * (MTI 0x0A48) and places it in the outgoing message buffer. The
        * rejection includes a 16-bit error code indicating the reason for
        * rejection, which may be either temporary or permanent.
        *
        * Temporary errors (bit 0x8000 set) may allow retry, while permanent
        * errors require the sender to abandon the transmission.
        *
        * Use cases:
        * - Rejecting unsupported datagram commands
        * - Rejecting datagrams when resources unavailable
        * - Indicating protocol violations or errors
        *
        * @param statemachine_info Pointer to state machine context containing
        * incoming message details and outgoing message buffer
        * @param return_code 16-bit OpenLCB error code indicating rejection
        * reason (see OpenLCB Error Codes specification)
        *
        * @note Outgoing message valid flag is set automatically
        * @note Reply is addressed to the source of the incoming datagram
        * @note Common error codes defined in openlcb_types.h
        *
        * @attention statemachine_info must contain valid incoming message
        *
        * @see ProtocolDatagramHandler_load_datagram_received_ok_message - For accepting datagrams
        * @see ProtocolDatagramHandler_datagram_rejected - Handles received rejected replies
        */
    extern void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /**
        * @brief Processes an incoming datagram message
        *
        * @details Main entry point for datagram processing. Examines the
        * datagram content type (first payload byte) and dispatches to the
        * appropriate handler based on the command code.
        *
        * Currently supports configuration memory operations (command 0x20).
        * Unsupported commands result in automatic datagram rejection with
        * ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN.
        *
        * Use cases:
        * - Called when MTI_DATAGRAM (0x1C48) message received
        * - Processing configuration memory requests
        * - Routing to appropriate protocol handlers
        *
        * @param statemachine_info Pointer to state machine context containing
        * the received datagram message and node information
        *
        * @note Sets outgoing_msg_info.valid if response generated
        * @note Automatically rejects unsupported command types
        *
        * @attention Must be called from main message processing loop
        * @attention statemachine_info must contain valid incoming datagram
        *
        * @see ProtocolDatagramHandler_load_datagram_received_ok_message - Response generator
        * @see ProtocolDatagramHandler_load_datagram_rejected_message - Rejection generator
        */
    extern void ProtocolDatagramHandler_datagram(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles datagram received OK acknowledgment
        *
        * @details Processes an incoming Datagram Received OK reply
        * (MTI 0x0A28), which confirms that a previously sent datagram was
        * successfully received by the destination node.
        *
        * This function clears the resend flag and frees any stored datagram
        * awaiting acknowledgment. The outgoing message valid flag is cleared
        * to prevent retransmission.
        *
        * Use cases:
        * - Completing successful datagram transmission
        * - Freeing resources after acknowledged send
        * - Normal datagram protocol completion
        *
        * @param statemachine_info Pointer to state machine context containing
        * the received OK reply and node information
        *
        * @note Clears outgoing_msg_info.valid flag
        * @note Frees last_received_datagram buffer if present
        * @note Uses locking for thread-safe buffer management
        *
        * @attention Called only when node is datagram sender
        *
        * @see ProtocolDatagramHandler_clear_resend_datagram_message - Cleanup function
        * @see ProtocolDatagramHandler_datagram_rejected - Handles rejection replies
        */
    extern void ProtocolDatagramHandler_datagram_received_ok(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Handles datagram rejected response
        *
        * @details Processes an incoming Datagram Rejected reply (MTI 0x0A48),
        * examining the error code to determine if retry is possible. Temporary
        * errors (ERROR_TEMPORARY bit set) allow retry by setting the resend
        * flag; permanent errors clear all retry state.
        *
        * Use cases:
        * - Handling temporary resource unavailability
        * - Detecting permanent protocol failures
        * - Managing datagram retry logic
        *
        * @param statemachine_info Pointer to state machine context containing
        * the received rejection reply and node information
        *
        * @note Sets resend_datagram flag for temporary errors
        * @note Clears resend state for permanent errors
        * @note Preserves last datagram buffer for potential retry
        *
        * @attention Error code examination determines retry behavior
        * @attention Permanent errors free datagram buffer
        *
        * @see ProtocolDatagramHandler_clear_resend_datagram_message - Cleanup for permanent errors
        * @see ProtocolDatagramHandler_datagram_received_ok - Handles success replies
        */
    extern void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info);

        /**
        * @brief Clears the resend datagram message flag for the specified node
        *
        * @details Frees the stored datagram buffer and clears the resend flag
        * for a node, preventing further retry attempts. This function uses
        * resource locking to ensure thread-safe buffer management.
        *
        * Use cases:
        * - Completing datagram transmission (success or permanent failure)
        * - Aborting retry attempts
        * - Cleaning up node state
        *
        * @param openlcb_node Pointer to the OpenLCB node structure to clear
        *
        * @note Uses lock_shared_resources/unlock_shared_resources for safety
        * @note Safe to call even if no datagram is stored
        * @note Sets last_received_datagram to NULL after freeing
        *
        * @warning Requires valid openlcb_node pointer
        *
        * @attention Clears both buffer and retry flag
        *
        * @see ProtocolDatagramHandler_datagram_received_ok - Uses this for cleanup
        * @see ProtocolDatagramHandler_datagram_rejected - Uses this for permanent errors
        */
    extern void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t *openlcb_node);

        /**
        * @brief 100ms timer tick handler for datagram protocol timeouts
        *
        * @details Periodic timer callback for managing datagram protocol
        * timeouts and retry logic. Currently a placeholder for future
        * timeout management features.
        *
        * Use cases:
        * - Called every 100ms from system timer
        * - Managing datagram acknowledgment timeouts
        * - Implementing retry delays
        *
        * @note Currently not implemented (placeholder)
        * @note Should be called from 100ms periodic timer interrupt or task
        *
        * @attention Reserved for future timeout implementation
        */
    extern void ProtocolDatagramHandler_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_PROTOCOL_DATAGRAM_HANDLER__ */
