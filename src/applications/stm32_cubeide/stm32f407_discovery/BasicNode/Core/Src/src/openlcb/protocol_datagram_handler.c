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
* @file protocol_datagram_handler.c
* @brief Implementation of OpenLCB datagram protocol handler
*
* @details This file implements the core datagram protocol handling for
* OpenLCB, supporting reliable transfer of 0-72 bytes between nodes.
* The implementation handles:
* - Configuration memory operations (read/write)
* - Multiple address space types (CDI, ACDI, Configuration Memory, etc.)
* - Datagram and stream-based transfers
* - Write-under-mask operations
* - Acknowledgment and rejection handling
* - Resource locking for thread safety
*
* The handler uses a callback-based architecture where the application
* provides implementations for specific memory operations through the
* interface_protocol_datagram_handler_t structure.
*
* @author Jim Kueneman
* @date 17 Jan 2026
*
* @see protocol_datagram_handler.h - Public interface
* @see OpenLCB Datagram Transport Specification
* @see OpenLCB Configuration Memory Operations Specification
*/

#include "protocol_datagram_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_defines.h"
#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


    /**
    * @brief Internal storage for callback interface pointer
    *
    * @details Stores the pointer to the application-provided interface
    * structure containing all callback functions for datagram operations.
    * Set during initialization and used throughout datagram processing.
    *
    * @note Module-level static variable
    * @note Must be initialized before any datagram operations
    *
    * @see ProtocolDatagramHandler_initialize - Sets this pointer
    */
static interface_protocol_datagram_handler_t *_interface;

    /**
    * @brief Initializes the Protocol Datagram Handler module
    *
    * @details Algorithm:
    * -# Store the interface pointer in module-level static variable
    * -# Cast away const qualifier for internal storage
    * -# Interface structure must remain valid for application lifetime
    *
    * The stored interface pointer is used by all subsequent datagram
    * operations to invoke appropriate callback handlers.
    *
    * Use cases:
    * - Called once during application initialization
    * - Must be called before processing any datagrams
    * - Typically called after node initialization
    *
    * @verbatim
    * @param interface_protocol_datagram_handler Pointer to initialized interface
    * @endverbatim
    * structure containing all required and optional callback functions
    *
    * @warning The interface structure must remain valid after this call
    * @warning The locking callbacks are required
    *
    * @attention Call before any datagram processing begins
    * @attention Interface pointer is stored but structure is not copied
    *
    * @see interface_protocol_datagram_handler_t - Interface structure definition
    * @see ProtocolDatagramHandler_datagram - Main datagram processor
    */
void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface_protocol_datagram_handler) {

    _interface = (interface_protocol_datagram_handler_t *) interface_protocol_datagram_handler;

}

    /**
    * @brief Internal helper to execute or reject a subcommand handler
    *
    * @details Algorithm:
    * -# Check if handler pointer is NULL
    * -# If NULL:
    *    - Load datagram rejection with "not implemented" error
    *    - Return immediately
    * -# If valid:
    *    - Invoke the handler with statemachine context
    *
    * This function centralizes the null-check logic used throughout
    * all address space dispatch functions, providing consistent error
    * handling for unimplemented operations.
    *
    * Use cases:
    * - Called by all address space handler dispatch functions
    * - Provides consistent null-handler rejection behavior
    * - Reduces code duplication across switch statements
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param handler_ptr Function pointer to memory operation handler,
    * @endverbatim
    * or NULL if operation not implemented
    *
    * @note Automatically generates rejection response for NULL handlers
    * @note Does not validate statemachine_info (caller responsibility)
    *
    * @attention Handler is responsible for generating response message
    *
    * @see ProtocolDatagramHandler_load_datagram_rejected_message - Error response generator
    */
static void _handle_subcommand(openlcb_statemachine_info_t *statemachine_info, memory_handler_t handler_ptr) {

    if (!handler_ptr) {

        ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

        return;

    }

    handler_ptr(statemachine_info);

}

    /**
    * @brief Dispatches datagram read requests based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding read handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * Supported address spaces:
    * - Configuration Definition Info (CDI)
    * - All Address Space (composite view)
    * - Configuration Memory
    * - ACDI Manufacturer
    * - ACDI User
    * - Traction Function Definition Info
    * - Traction Function Configuration Memory
    *
    * Use cases:
    * - Processing read commands with space ID in byte 6
    * - Routing to appropriate address space handler
    * - Central dispatch for datagram-based reads
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming datagram with address space in payload[6]
    *
    * @note Payload byte 6 must contain valid address space identifier
    * @note Generates automatic rejection for unknown spaces
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see CONFIG_MEM_SPACE_* - Address space identifiers
    */
static void _handle_read_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches read reply OK responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding reply handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles successful read reply datagrams when this node
    * initiated a read request to another node. The reply confirms successful
    * read and contains the requested data.
    *
    * Use cases:
    * - Processing read replies when node is the requestor
    * - Completing client-side read operations
    * - Receiving configuration data from remote nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming read reply datagram with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Generates automatic rejection for unknown spaces
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_read_reply_fail_address_space_at_offset_6 - Handles failures
    */
static void _handle_read_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches read reply fail responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding failure handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles failed read reply datagrams when this node
    * initiated a read request to another node. The reply indicates the
    * read operation could not be completed, with error details in payload.
    *
    * Use cases:
    * - Processing read failure replies when node is the requestor
    * - Handling errors from client-side read operations
    * - Detecting access restrictions or invalid addresses
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming read failure datagram with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Error details are in datagram payload
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_read_reply_ok_address_space_at_offset_6 - Handles successes
    */
static void _handle_read_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_traction_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream-based read requests based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding stream read handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * Stream-based reads are used for large data transfers that exceed the
    * 72-byte datagram limit. The stream protocol allows efficient transfer
    * of arbitrarily large address space contents.
    *
    * Use cases:
    * - Processing stream read commands with space ID in byte 6
    * - Transferring large CDI or configuration data
    * - Efficient bulk read operations
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream read request with address space in payload[6]
    *
    * @note Used for large data transfers (>72 bytes)
    * @note Generates automatic rejection for unknown spaces
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_read_address_space_at_offset_6 - Datagram-based equivalent
    */
static void _handle_read_stream_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream read reply OK responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding stream reply handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles successful stream read reply datagrams when this
    * node initiated a stream read request to another node. The reply confirms
    * stream setup and data transfer will follow via stream protocol.
    *
    * Use cases:
    * - Processing stream read replies when node is the requestor
    * - Completing client-side stream read setup
    * - Receiving large configuration data from remote nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream read reply with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Actual data follows via stream protocol
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_read_stream_reply_fail_address_space_at_offset_6 - Handles failures
    */
static void _handle_read_stream_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream read reply fail responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding failure handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles failed stream read reply datagrams when this node
    * initiated a stream read request to another node. The reply indicates the
    * stream read operation could not be set up, with error details in payload.
    *
    * Use cases:
    * - Processing stream read failure replies when node is the requestor
    * - Handling errors from client-side stream read operations
    * - Detecting access restrictions or resource limitations
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream read failure with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Error details are in datagram payload
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_read_stream_reply_ok_address_space_at_offset_6 - Handles successes
    */
static void _handle_read_stream_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_traction_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches datagram write requests based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding write handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * Supported address spaces include both read-write spaces (Configuration
    * Memory, ACDI User, Traction Config, Firmware) and read-only spaces
    * (which should have NULL handlers and will auto-reject).
    *
    * Use cases:
    * - Processing write commands with space ID in byte 6
    * - Updating configuration memory
    * - Firmware upgrade operations
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming write request with address space in payload[6]
    *
    * @note Read-only spaces should have NULL handlers
    * @note Generates automatic rejection for unknown spaces
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see CONFIG_MEM_SPACE_* - Address space identifiers
    */
static void _handle_write_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all);

            break;
        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory);

            break;

        case CONFIG_MEM_SPACE_FIRMWARE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_firmware_upgrade);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches write reply OK responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding reply handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles successful write reply datagrams when this node
    * initiated a write request to another node. The reply confirms successful
    * write completion.
    *
    * Use cases:
    * - Processing write replies when node is the requestor
    * - Completing client-side write operations
    * - Confirming configuration updates to remote nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming write reply with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Confirms write was completed successfully
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_reply_fail_address_space_at_offset_6 - Handles failures
    */
static void _handle_write_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches write reply fail responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding failure handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles failed write reply datagrams when this node
    * initiated a write request to another node. The reply indicates the
    * write operation could not be completed, with error details in payload.
    *
    * Use cases:
    * - Processing write failure replies when node is the requestor
    * - Handling errors from client-side write operations
    * - Detecting write protection or invalid addresses
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming write failure with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Error details are in datagram payload
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_reply_ok_address_space_at_offset_6 - Handles successes
    */
static void _handle_write_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_traction_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream-based write requests based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding stream write handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * Stream-based writes are used for large data transfers that exceed the
    * 72-byte datagram limit. The stream protocol allows efficient transfer
    * of arbitrarily large amounts of data to address spaces.
    *
    * Use cases:
    * - Processing stream write commands with space ID in byte 6
    * - Transferring large firmware images
    * - Efficient bulk write operations
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream write request with address space in payload[6]
    *
    * @note Used for large data transfers (>72 bytes)
    * @note Read-only spaces should have NULL handlers
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_address_space_at_offset_6 - Datagram-based equivalent
    */
static void _handle_write_stream_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all);

            break;
        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream write reply OK responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding stream reply handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles successful stream write reply datagrams when this
    * node initiated a stream write request to another node. The reply confirms
    * stream setup and data transfer can begin via stream protocol.
    *
    * Use cases:
    * - Processing stream write replies when node is the requestor
    * - Completing client-side stream write setup
    * - Preparing for large data transfer to remote nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream write reply with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Actual data transfer follows via stream protocol
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_stream_reply_fail_address_space_at_offset_6 - Handles failures
    */
static void _handle_write_stream_reply_ok_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer_reply_ok);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_definition_info_reply_ok);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_config_memory_reply_ok);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches stream write reply fail responses based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding failure handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * This function handles failed stream write reply datagrams when this node
    * initiated a stream write request to another node. The reply indicates the
    * stream write operation could not be set up, with error details in payload.
    *
    * Use cases:
    * - Processing stream write failure replies when node is the requestor
    * - Handling errors from client-side stream write operations
    * - Detecting write protection or resource limitations
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming stream write failure with address space in payload[6]
    *
    * @note Only used when node is acting as client (requestor)
    * @note Error details are in datagram payload
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_stream_reply_ok_address_space_at_offset_6 - Handles successes
    */
static void _handle_write_stream_reply_fail_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_manufacturer_reply_fail);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_acdi_user_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_definition_info_reply_fail);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_traction_function_config_memory_reply_fail);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Dispatches write-under-mask requests based on address space
    *
    * @details Algorithm:
    * -# Extract address space identifier from payload byte 6
    * -# Match against supported CONFIG_MEM_SPACE_* constants
    * -# For each recognized space:
    *    - Dispatch to corresponding write-under-mask handler via _handle_subcommand
    * -# For unrecognized space:
    *    - Generate "unknown subcommand" rejection
    *
    * Write-under-mask operations allow selective modification of bytes using
    * a mask to specify which bits should be updated. This is useful for
    * updating individual configuration fields without affecting adjacent data.
    *
    * Use cases:
    * - Processing write-under-mask commands with space ID in byte 6
    * - Selective bit/byte updates in configuration
    * - Atomic field modifications
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming write-under-mask request with address space in payload[6]
    *
    * @note Mask specifies which bits to modify
    * @note Read-only spaces should have NULL handlers
    *
    * @attention Assumes payload is at least 7 bytes long
    * @attention Handler callbacks may be NULL (handled by _handle_subcommand)
    *
    * @see _handle_subcommand - Executes or rejects handler
    * @see _handle_write_address_space_at_offset_6 - Standard write variant
    */
static void _handle_write_under_mask_address_space_at_offset_6(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[6]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info);

            break;

        case CONFIG_MEM_SPACE_ALL:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all);

            break;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory);

            break;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_manufacturer);

            break;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_acdi_user);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_traction_function_definition_info);

            break;

        case CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_traction_function_config_memory);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    }

}

    /**
    * @brief Main dispatcher for configuration memory commands
    *
    * @details Algorithm:
    * -# Extract subcommand from payload byte 1
    * -# Match against CONFIG_MEM_* subcommand constants
    * -# For read/write operations:
    *    - Dispatch to address-space-specific handler
    * -# For configuration operations:
    *    - Dispatch to operation-specific handler
    * -# For unrecognized subcommand:
    *    - Generate "unknown subcommand" rejection
    *
    * This is the top-level dispatcher for all configuration memory protocol
    * operations including reads, writes, streams, and control commands.
    *
    * Supported subcommands:
    * - Read operations (datagram and stream-based, with space in byte 6 or shorthand)
    * - Write operations (datagram, stream, and under-mask, with space in byte 6 or shorthand)
    * - Reply handling (OK and FAIL for reads/writes)
    * - Configuration commands (options, lock, freeze, reset, etc.)
    *
    * Use cases:
    * - Processing all CONFIG_MEM_CONFIGURATION (0x20) datagrams
    * - Routing to appropriate subcommand handler
    * - Central dispatch for memory protocol
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming configuration memory command with subcommand in payload[1]
    *
    * @note Payload byte 1 contains subcommand identifier
    * @note Generates automatic rejection for unknown subcommands
    *
    * @attention Assumes payload is at least 2 bytes long
    * @attention Handler callbacks may be NULL (handled by dispatched functions)
    *
    * @see CONFIG_MEM_* - Subcommand identifiers
    * @see ProtocolDatagramHandler_datagram - Calls this for 0x20 commands
    */
static void _handle_datagram_memory_configuration_command(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[1]) { // which space?

        case CONFIG_MEM_READ_SPACE_IN_BYTE_6:

            _handle_read_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory);

            break;

        case CONFIG_MEM_READ_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all);

            break;

        case CONFIG_MEM_READ_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_read_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_read_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_all_reply_fail);

            break;

        case CONFIG_MEM_READ_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6:

            _handle_read_stream_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all);

            break;

        case CONFIG_MEM_READ_STREAM_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_read_stream_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_read_stream_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_read_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_WRITE_SPACE_IN_BYTE_6:

            _handle_write_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all);

            break;

        case CONFIG_MEM_WRITE_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_write_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_write_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_all_reply_fail);

            break;

        case CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_IN_BYTE_6:

            _handle_write_under_mask_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_all);

            break;

        case CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_under_mask_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6:

            _handle_write_stream_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all);

            break;

        case CONFIG_MEM_WRITE_STREAM_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6:

            _handle_write_stream_reply_ok_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_ok);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6:

            _handle_write_stream_reply_fail_address_space_at_offset_6(statemachine_info);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FD:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_configuration_memory_reply_fail);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FE:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_all_reply_fail);

            break;

        case CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FF:

            _handle_subcommand(statemachine_info, _interface->memory_write_stream_space_config_description_info_reply_fail);

            break;

        case CONFIG_MEM_OPTIONS_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_options_cmd);

            break;

        case CONFIG_MEM_OPTIONS_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_options_reply);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_not_present);

            break;

        case CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT:

            _handle_subcommand(statemachine_info, _interface->memory_get_address_space_info_reply_present);

            break;

        case CONFIG_MEM_RESERVE_LOCK:

            _handle_subcommand(statemachine_info, _interface->memory_reserve_lock);

            break;

        case CONFIG_MEM_RESERVE_LOCK_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_reserve_lock_reply);

            break;

        case CONFIG_MEM_GET_UNIQUE_ID:

            _handle_subcommand(statemachine_info, _interface->memory_get_unique_id);

            break;


        case CONFIG_MEM_GET_UNIQUE_ID_REPLY:

            _handle_subcommand(statemachine_info, _interface->memory_get_unique_id_reply);

            break;


        case CONFIG_MEM_UNFREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_unfreeze);

            break;

        case CONFIG_MEM_FREEZE:

            _handle_subcommand(statemachine_info, _interface->memory_freeze);

            break;

        case CONFIG_MEM_UPDATE_COMPLETE:

            _handle_subcommand(statemachine_info, _interface->memory_update_complete);

            break;

        case CONFIG_MEM_RESET_REBOOT:

            _handle_subcommand(statemachine_info, _interface->memory_reset_reboot);

            break;

        case CONFIG_MEM_FACTORY_RESET:

            _handle_subcommand(statemachine_info, _interface->memory_factory_reset);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);

            break;

    } // switch sub-command

}

    /**
    * @brief Processes an incoming datagram message
    *
    * @details Algorithm:
    * -# Extract command type from payload byte 0
    * -# Match against supported command constants
    * -# For CONFIG_MEM_CONFIGURATION (0x20):
    *    - Dispatch to memory configuration handler
    * -# For unrecognized commands:
    *    - Generate "unknown command" rejection
    *
    * This is the main entry point for all datagram processing. It examines
    * the datagram content type and routes to the appropriate protocol handler.
    *
    * Use cases:
    * - Called when MTI_DATAGRAM (0x1C48) message received
    * - Processing configuration memory requests
    * - Routing to appropriate protocol handlers
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
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
    * @see _handle_datagram_memory_configuration_command - Memory config dispatcher
    */
void ProtocolDatagramHandler_datagram(openlcb_statemachine_info_t *statemachine_info) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[0]) { // commands

        case CONFIG_MEM_CONFIGURATION: // are we 0x20?

            _handle_datagram_memory_configuration_command(statemachine_info);

            break;

        default:

            ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN);

            break;

    } // switch command


}

    /**
    * @brief Loads a datagram received OK acknowledgment message
    *
    * @details Algorithm:
    * -# Initialize exponent to 0 (no timeout)
    * -# If reply_pending_time_in_seconds > 0:
    *    - Calculate power-of-2 exponent for timeout value
    *    - Test against threshold values (2, 4, 8, 16, 32, ...)
    *    - Select appropriate exponent (1-15)
    * -# Build Datagram Received OK message (MTI 0x0A28)
    * -# Set source/destination from incoming message
    * -# Add flags byte with reply-pending bit and timeout exponent
    * -# Mark outgoing message as valid
    *
    * The timeout encoding uses 4 bits (0-15) representing 2^N seconds:
    * - 0 = no reply pending
    * - 1 = 2^1 = 2 seconds
    * - 2 = 2^2 = 4 seconds
    * - ...
    * - 15 = 2^15 = 32768 seconds
    *
    * Use cases:
    * - Acknowledging successful datagram reception
    * - Indicating deferred processing with reply-pending timeout
    * - Normal datagram protocol flow completion
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming message details and outgoing message buffer
    * @verbatim
    * @param reply_pending_time_in_seconds Time in seconds until reply will
    * @endverbatim
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
void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds) {

    uint8_t exponent = 0;

    if (reply_pending_time_in_seconds > 0) {

            if (reply_pending_time_in_seconds <= 2) {

                exponent = 1;

            } else if (reply_pending_time_in_seconds <= 4) {

                exponent = 2;

            } else if (reply_pending_time_in_seconds <= 8) {

                exponent = 3;

            } else if (reply_pending_time_in_seconds <= 16) {

                exponent = 4;

            } else if (reply_pending_time_in_seconds <= 32) {

                exponent = 5;

            } else if (reply_pending_time_in_seconds <= 64) {

                exponent = 6;

            } else if (reply_pending_time_in_seconds <= 128) {

                exponent = 7;

            } else if (reply_pending_time_in_seconds <= 256) {

                exponent = 8;

            } else if (reply_pending_time_in_seconds <= 512) {

                exponent = 9;

            } else if (reply_pending_time_in_seconds <= 1024) {

                exponent = 0x0A;

            } else if (reply_pending_time_in_seconds <= 2048) {

                exponent = 0x0B;

            } else if (reply_pending_time_in_seconds <= 4096) {

                exponent = 0x0C;

            } else if (reply_pending_time_in_seconds <= 8192) {

                exponent = 0x0D;

            } else if (reply_pending_time_in_seconds <= 16384) {

                exponent = 0x0E;

            } else {

                exponent = 0x0F;

            }

        }

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM_OK_REPLY);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            DATAGRAM_OK_REPLY_PENDING | exponent,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Loads a datagram rejected message with error code
    *
    * @details Algorithm:
    * -# Build Datagram Rejected message (MTI 0x0A48)
    * -# Set source/destination from incoming message
    * -# Add 16-bit error code to payload
    * -# Mark outgoing message as valid
    *
    * The error code indicates why the datagram was rejected. The high bit
    * (0x8000) indicates temporary vs permanent error:
    * - Temporary (0x8xxx): Sender may retry
    * - Permanent (0x0xxx): Sender must abort
    *
    * Common error codes:
    * - ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN
    * - ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN
    * - ERROR_TEMPORARY_BUFFER_UNAVAILABLE
    *
    * Use cases:
    * - Rejecting unsupported datagram commands
    * - Rejecting datagrams when resources unavailable
    * - Indicating protocol violations or errors
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
    * incoming message details and outgoing message buffer
    * @verbatim
    * @param return_code 16-bit OpenLCB error code indicating rejection
    * @endverbatim
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
void ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code) {

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM_REJECTED_REPLY);

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            return_code,
            0);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Handles datagram received OK acknowledgment
    *
    * @details Algorithm:
    * -# Clear resend datagram message flag and free buffer
    * -# Set outgoing_msg_info.valid to false (no transmission needed)
    *
    * This function processes an incoming Datagram Received OK reply
    * (MTI 0x0A28), which confirms that a previously sent datagram was
    * successfully received by the destination node.
    *
    * Use cases:
    * - Completing successful datagram transmission
    * - Freeing resources after acknowledged send
    * - Normal datagram protocol completion
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
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
void ProtocolDatagramHandler_datagram_received_ok(openlcb_statemachine_info_t *statemachine_info) {

    ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Handles datagram rejected response
    *
    * @details Algorithm:
    * -# Extract error code from payload
    * -# Test for ERROR_TEMPORARY bit (0x8000)
    * -# If temporary error:
    *    - Check if last_received_datagram exists
    *    - Set resend_datagram flag if buffer exists
    * -# If permanent error:
    *    - Clear resend flag and free buffer
    * -# Set outgoing_msg_info.valid to false (no transmission needed)
    *
    * This function processes an incoming Datagram Rejected reply (MTI 0x0A48),
    * examining the error code to determine if retry is possible. Temporary
    * errors (ERROR_TEMPORARY bit set) allow retry by setting the resend
    * flag; permanent errors clear all retry state.
    *
    * Use cases:
    * - Handling temporary resource unavailability
    * - Detecting permanent protocol failures
    * - Managing datagram retry logic
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing
    * @endverbatim
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
void ProtocolDatagramHandler_datagram_rejected(openlcb_statemachine_info_t *statemachine_info) {

    if ((OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0) & ERROR_TEMPORARY) == ERROR_TEMPORARY) {

        if (statemachine_info->openlcb_node->last_received_datagram) {

            statemachine_info->openlcb_node->state.resend_datagram = true;

        }

    } else {

        ProtocolDatagramHandler_clear_resend_datagram_message(statemachine_info->openlcb_node);

    }

    statemachine_info->outgoing_msg_info.valid = false;

}

    /**
    * @brief Clears the resend datagram message flag for the specified node
    *
    * @details Algorithm:
    * -# Check if last_received_datagram buffer exists
    * -# If buffer exists:
    *    - Acquire shared resource lock
    *    - Free the buffer via OpenLcbBufferStore_free_buffer
    *    - Release shared resource lock
    *    - Set last_received_datagram pointer to NULL
    * -# Clear resend_datagram flag
    *
    * This function frees the stored datagram buffer and clears the resend flag
    * for a node, preventing further retry attempts. Thread-safe buffer
    * management is ensured through resource locking callbacks.
    *
    * Use cases:
    * - Completing datagram transmission (success or permanent failure)
    * - Aborting retry attempts
    * - Cleaning up node state
    *
    * @verbatim
    * @param openlcb_node Pointer to the OpenLCB node structure to clear
    * @endverbatim
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
void ProtocolDatagramHandler_clear_resend_datagram_message(openlcb_node_t *openlcb_node) {

    if (openlcb_node->last_received_datagram) {

        _interface->lock_shared_resources();
        OpenLcbBufferStore_free_buffer(openlcb_node->last_received_datagram);
        _interface->unlock_shared_resources();

        openlcb_node->last_received_datagram = NULL;

    }

    openlcb_node->state.resend_datagram = false;

}

    /**
    * @brief 100ms timer tick handler for datagram protocol timeouts
    *
    * @details Algorithm:
    * -# Placeholder for future implementation
    * -# Will handle datagram acknowledgment timeouts
    * -# Will manage retry timing
    *
    * This function is a periodic timer callback for managing datagram protocol
    * timeouts and retry logic. Currently a placeholder for future timeout
    * management features.
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
void ProtocolDatagramHandler_100ms_timer_tick(void) {


}






