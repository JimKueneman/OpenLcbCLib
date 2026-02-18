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
     * @file protocol_config_mem_operations_handler.c
     * @brief Implementation of configuration memory operations protocol handler
     * @author Jim Kueneman
     * @date 17 Jan 2026
     */

#include "protocol_config_mem_operations_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"


static interface_protocol_config_mem_operations_handler_t* _interface;

    /**
    * @brief Initializes the configuration memory operations protocol handler
    *
    * @details Algorithm:
    * -# Store pointer to interface structure in static variable
    * -# Interface callbacks are now available for use by handler functions
    *
    * The interface structure must remain valid for the lifetime of the application
    * as the handler stores a pointer to it rather than copying its contents.
    *
    * Use cases:
    * - Called once during application startup
    * - Must be called before processing any configuration datagrams
    *
    * @verbatim
    * @param interface_protocol_config_mem_operations_handler Pointer to interface structure with callback functions
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Interface structure must remain valid throughout application lifetime
    * @warning Required callbacks (load_datagram_received_ok_message, load_datagram_received_rejected_message) must be set
    * @attention Call during initialization before enabling datagram reception
    *
    * @see interface_protocol_config_mem_operations_handler_t
    */
void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler) {

    _interface = (interface_protocol_config_mem_operations_handler_t*) interface_protocol_config_mem_operations_handler;

}

    /**
    * @brief Decodes address space identifier to space definition structure
    *
    * @details Algorithm:
    * -# Extract space identifier byte from payload at specified offset
    * -# Use switch statement to map space ID to corresponding address space structure
    * -# Return pointer to appropriate space definition in node parameters
    * -# Return NULL if space identifier is unrecognized
    *
    * This function maps OpenLCB standard address space identifiers to the node's
    * internal address space definition structures. The space definitions contain
    * metadata like size, flags, and description.
    *
    * Standard address spaces:
    * - 0xFF: Configuration Definition Info (CDI)
    * - 0xFE: All Memory
    * - 0xFD: Configuration Memory
    * - 0xFC: ACDI Manufacturer
    * - 0xFB: ACDI User
    * - 0xFA: Train Function Definition Info
    * - 0xF9: Train Function Configuration Memory
    * - 0xEF: Firmware
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    * @verbatim
    * @param space_offset Byte offset in payload where space identifier is located
    * @endverbatim
    *
    * @return Pointer to address space definition structure, or NULL if space not recognized
    *
    * @warning Pointer must not be NULL
    * @warning statemachine_info->incoming_msg_info.msg_ptr must be valid
    * @warning Caller must check for NULL return value
    *
    * @see user_address_space_info_t
    */
static const user_address_space_info_t* _decode_to_space_definition(openlcb_statemachine_info_t *statemachine_info, uint8_t space_offset) {

    switch (*statemachine_info->incoming_msg_info.msg_ptr->payload[space_offset]) {

        case CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO:

            return (&statemachine_info->openlcb_node->parameters->address_space_configuration_definition);

        case CONFIG_MEM_SPACE_ALL:

            return &statemachine_info->openlcb_node->parameters->address_space_all;

        case CONFIG_MEM_SPACE_CONFIGURATION_MEMORY:

            return &statemachine_info->openlcb_node->parameters->address_space_config_memory;

        case CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS:

            return &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

        case CONFIG_MEM_SPACE_ACDI_USER_ACCESS:

            return &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_DEFINITION_INFO:

            return &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

        case CONFIG_MEM_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY:

            return &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory;

        case CONFIG_MEM_SPACE_FIRMWARE:

            return &statemachine_info->openlcb_node->parameters->address_space_firmware;

        default:

            return NULL;

    }

}

    /**
    * @brief Loads the common header for configuration memory operation reply messages
    *
    * @details Algorithm:
    * -# Reset outgoing message payload count to 0
    * -# Load OpenLCB message header with source/destination addressing
    * -# Set MTI to DATAGRAM
    * -# Copy CONFIG_MEM_CONFIGURATION command byte to payload position 0
    * -# Set outgoing message valid flag to false (caller will set true when complete)
    *
    * This helper function sets up the common portions of all configuration memory
    * operation reply messages. The caller is responsible for adding operation-specific
    * payload bytes and setting the valid flag.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request information (used for addressing)
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning outgoing_msg_info.msg_ptr must be allocated
    * @attention Caller must add operation-specific payload and set valid flag
    *
    * @see OpenLcbUtilities_load_openlcb_message
    * @see OpenLcbUtilities_copy_byte_to_openlcb_payload
    */
static void _load_config_mem_reply_message_header(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_read_request_info) {

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 0;

    OpenLcbUtilities_load_openlcb_message(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->alias,
            statemachine_info->openlcb_node->id,
            statemachine_info->incoming_msg_info.msg_ptr->source_alias,
            statemachine_info->incoming_msg_info.msg_ptr->source_id,
            MTI_DATAGRAM);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_CONFIGURATION,
            0);

    statemachine_info->outgoing_msg_info.valid = false; // Assume there is not a message to send by default

}

    /**
    * @brief Builds the available write length flags for configuration options
    *
    * @details Algorithm:
    * -# Initialize flags with reserved bits set
    * -# Check if stream read/write is supported in node parameters
    * -# If supported, OR in the stream read/write flag
    * -# Return combined flags byte
    *
    * The write length flags indicate which write operations are supported:
    * - Bit 6-7: Reserved (always set)
    * - Bit 5: Stream read/write support
    * - Bits 0-4: Reserved for future use
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing node parameters
    * @endverbatim
    *
    * @return Byte containing write length capability flags
    *
    * @warning Pointer must not be NULL
    * @warning statemachine_info->openlcb_node->parameters must be valid
    *
    * @see _available_commands_flags
    * @see CONFIG_OPTIONS_WRITE_LENGTH_STREAM_READ_WRITE
    */
static uint8_t _available_write_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint8_t write_lengths = CONFIG_OPTIONS_WRITE_LENGTH_RESERVED;

    if (statemachine_info->openlcb_node->parameters->configuration_options.stream_read_write_supported) {

        write_lengths = write_lengths | CONFIG_OPTIONS_WRITE_LENGTH_STREAM_READ_WRITE;

    }

    return write_lengths;

}

    /**
    * @brief Builds the available command flags for configuration options
    *
    * @details Algorithm:
    * -# Initialize result to 0x0000
    * -# Check each supported command in node configuration options
    * -# For each supported command, OR in the corresponding flag bit:
    *    - Write under mask
    *    - Unaligned reads
    *    - Unaligned writes
    *    - ACDI manufacturer read (0xFC)
    *    - ACDI user read (0xFB)
    *    - ACDI user write (0xFB)
    * -# Return combined flags word
    *
    * The command flags indicate which optional configuration commands are supported
    * by this node. These flags are returned in response to Get Configuration Options
    * queries.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing node parameters
    * @endverbatim
    *
    * @return 16-bit word containing command capability flags
    *
    * @warning Pointer must not be NULL
    * @warning statemachine_info->openlcb_node->parameters must be valid
    *
    * @see _available_write_flags
    * @see CONFIG_OPTIONS_COMMANDS_WRITE_UNDER_MASK
    */
static uint16_t _available_commands_flags(openlcb_statemachine_info_t *statemachine_info) {

    uint16_t result = 0x0000;

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_under_mask_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_WRITE_UNDER_MASK;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_reads_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_UNALIGNED_READS;

    }
    if (statemachine_info->openlcb_node->parameters->configuration_options.unaligned_writes_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_UNALIGNED_WRITES;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_manufacturer_space_0xfc_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_MANUFACTURER_READ;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.read_from_user_space_0xfb_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_USER_READ;

    }

    if (statemachine_info->openlcb_node->parameters->configuration_options.write_to_user_space_0xfb_supported) {

        result = result | CONFIG_OPTIONS_COMMANDS_ACDI_USER_WRITE;

    }

    return result;

}

    /**
    * @brief Builds the flags byte for address space information
    *
    * @details Algorithm:
    * -# Initialize flags to 0x0
    * -# Check if address space is read-only
    * -# If read-only, OR in the read-only flag
    * -# Check if low address is valid for this space
    * -# If valid, OR in the low address valid flag
    * -# Return combined flags byte
    *
    * Address space info flags indicate characteristics of the space:
    * - Bit 0: Read-only flag
    * - Bit 1: Low address valid flag
    * - Bits 2-7: Reserved
    *
    * @verbatim
    * @param config_mem_operations_request_info Pointer to request info containing space definition
    * @endverbatim
    *
    * @return Byte containing address space characteristic flags
    *
    * @warning Pointer must not be NULL
    * @warning config_mem_operations_request_info->space_info must be valid
    *
    * @see user_address_space_info_t
    * @see CONFIG_OPTIONS_SPACE_INFO_FLAG_READ_ONLY
    */
static uint8_t _available_address_space_info_flags(config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t flags = 0x0;

    if (config_mem_operations_request_info->space_info->read_only) {

        flags = flags | CONFIG_OPTIONS_SPACE_INFO_FLAG_READ_ONLY;

    }

    if (config_mem_operations_request_info->space_info->low_address_valid) {

        flags = flags | CONFIG_OPTIONS_SPACE_INFO_FLAG_USE_LOW_ADDRESS;

    }

    return flags;
}

    /**
    * @brief Loads a datagram received OK acknowledgment message
    *
    * @details Algorithm:
    * -# Call interface callback to load datagram OK message with 0x00 delay
    * -# Set openlcb_datagram_ack_sent flag to true
    * -# Set enumerate flag to true to re-invoke handler for data processing
    *
    * This function sends a positive acknowledgment for the received datagram,
    * indicating it was accepted and will be processed. The enumerate flag causes
    * the handler to be called again to complete the actual operation.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning _interface->load_datagram_received_ok_message must be set
    * @attention Sets state flags that affect subsequent handler invocations
    *
    * @see _load_datagram_reject_message
    * @see _interface->load_datagram_received_ok_message
    */
static void _load_datagram_ok_message(openlcb_statemachine_info_t *statemachine_info) {

    _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
    statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data

}

    /**
    * @brief Loads a datagram received rejected acknowledgment message
    *
    * @details Algorithm:
    * -# Call interface callback to load datagram rejected message with error code
    * -# Set openlcb_datagram_ack_sent flag to false (operation complete)
    * -# Set enumerate flag to false (stop processing this message)
    *
    * This function sends a negative acknowledgment for the received datagram,
    * indicating it was rejected due to an error. The flags are set to terminate
    * processing of this message.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param error_code OpenLCB error code indicating reason for rejection
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning _interface->load_datagram_received_rejected_message must be set
    * @attention Terminates processing of current message
    *
    * @see _load_datagram_ok_message
    * @see _interface->load_datagram_received_rejected_message
    */
static void _load_datagram_reject_message(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code) {

    _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // done
    statemachine_info->incoming_msg_info.enumerate = false; // done

}

    /**
    * @brief Central dispatcher for configuration memory operations requests
    *
    * @details Algorithm:
    * -# Check if datagram acknowledgment has been sent
    * -# If not sent yet (first call):
    *    - Check if operation callback is registered
    *    - If registered, send datagram OK and return
    *    - If not registered, send datagram reject with NOT_IMPLEMENTED error
    * -# If ACK already sent (second call):
    *    - Invoke the operation-specific callback to process the command
    *    - Reset openlcb_datagram_ack_sent flag to false
    *    - Reset enumerate flag to false
    *
    * This function implements a two-phase processing pattern:
    * - Phase 1: Validate request and send datagram acknowledgment
    * - Phase 2: Execute the actual operation via registered callback
    *
    * The two-phase approach allows the datagram ACK to be sent quickly while
    * potentially time-consuming operations are deferred to the second invocation.
    *
    * Use cases:
    * - All configuration memory operation command processing
    * - Coordinating datagram ACK with operation execution
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_operations_request_info Pointer to request information including operation callback
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Operation callback in request_info will be invoked if not NULL
    * @attention Uses state flags to implement two-phase processing
    *
    * @see _load_datagram_ok_message
    * @see _load_datagram_reject_message
    */
static void _handle_operations_request(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        if (config_mem_operations_request_info->operations_func) {

            _load_datagram_ok_message(statemachine_info);

        } else {

            _load_datagram_reject_message(statemachine_info, ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);
        }

        return;
    }

    // Complete Command Request, if it was null the first pass with the datagram ACK check would have return NACK and with won't get called with a null
    config_mem_operations_request_info->operations_func(statemachine_info, config_mem_operations_request_info);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // reset
    statemachine_info->incoming_msg_info.enumerate = false; // done
}

    /**
    * @brief Processes a Get Configuration Options command request
    *
    * @details Algorithm:
    * -# Load reply message header
    * -# Copy CONFIG_MEM_OPTIONS_REPLY command byte to payload
    * -# Build and copy available write flags to payload
    * -# Build and copy available commands flags (high byte then low byte) to payload
    * -# Copy write lengths byte to payload (hardcoded to 0x00)
    * -# Set outgoing message as valid
    *
    * This function generates a response to a Get Configuration Options query,
    * advertising the node's configuration capabilities including supported commands,
    * write operations, and address spaces.
    *
    * Use cases:
    * - Responding to configuration tool capability queries
    * - Advertising node features during configuration discovery
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_operations_request_info Pointer to request information
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning outgoing_msg_info.msg_ptr must be allocated
    *
    * @see _available_write_flags
    * @see _available_commands_flags
    * @see ProtocolConfigMemOperationsHandler_options_cmd
    */
void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_OPTIONS_REPLY,
            1);

    OpenLcbUtilities_copy_word_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            _available_commands_flags(statemachine_info),
            2);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            _available_write_flags(statemachine_info),
            4);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->parameters->configuration_options.high_address_space,
            5);


    // elect to always send this optional byte
    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->parameters->configuration_options.low_address_space,
            6);

    if (strlen(statemachine_info->openlcb_node->parameters->configuration_options.description) > 0x00) {

        OpenLcbUtilities_copy_string_to_openlcb_payload(
                statemachine_info->outgoing_msg_info.msg_ptr,
                statemachine_info->openlcb_node->parameters->configuration_options.description,
                statemachine_info->outgoing_msg_info.msg_ptr->payload_count);

    }

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Processes a Get Address Space Information command request
    *
    * @details Algorithm:
    * -# Initialize description offset to 8 (default position)
    * -# Load reply message header
    * -# Check if requested space definition exists
    * -# If space exists and is present:
    *    - Copy CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT command byte
    *    - Copy requested space identifier from incoming message
    *    - Copy highest address (4 bytes) to payload
    *    - Build and copy space info flags byte
    *    - If low address is valid:
    *      * Copy low address (4 bytes) to payload
    *      * Update description offset to 12
    *    - If description string exists (non-empty):
    *      * Copy description string to payload at description offset
    *    - Set outgoing message as valid
    *    - Return
    * -# If space not present or doesn't exist:
    *    - Copy CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT command byte
    *    - Copy requested space identifier from incoming message
    *    - Set payload count to 8 (minimum size expected by OpenLcbChecker)
    *    - Set outgoing message as valid
    *
    * This function responds to queries about specific address spaces, providing
    * information about size, characteristics (read-only, low address), and description.
    *
    * Use cases:
    * - Responding to address space capability queries
    * - Providing memory layout information to configuration tools
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_operations_request_info Pointer to request information containing space definition
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning outgoing_msg_info.msg_ptr must be allocated
    *
    * @see _available_address_space_info_flags
    * @see _decode_to_space_definition
    */
void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    uint8_t description_offset = 8;

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    if (config_mem_operations_request_info->space_info) {

        if (config_mem_operations_request_info->space_info->present) {

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT,
                    1);

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[2],
                    2);

            OpenLcbUtilities_copy_dword_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    config_mem_operations_request_info->space_info->highest_address,
                    3);

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    _available_address_space_info_flags(config_mem_operations_request_info),
                    7);

            if (config_mem_operations_request_info->space_info->low_address_valid) {

                OpenLcbUtilities_copy_dword_to_openlcb_payload(
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_operations_request_info->space_info->low_address,
                        8);

                description_offset = 12;

            }

            if (strlen(config_mem_operations_request_info->space_info->description) > 0) {

                OpenLcbUtilities_copy_string_to_openlcb_payload(
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_operations_request_info->space_info->description,
                        description_offset);

            }

            statemachine_info->outgoing_msg_info.valid = true;

            return;

        }

    }

    // default reply

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT,
            1);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
    *statemachine_info->incoming_msg_info.msg_ptr->payload[2],
            2);

    statemachine_info->outgoing_msg_info.msg_ptr->payload_count = 8; // OpenLcbChecker needs 8

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Processes a Lock/Reserve command request
    *
    * @details Algorithm:
    * -# Load reply message header
    * -# Extract requested Node ID from incoming message payload (6 bytes starting at position 2)
    * -# Check current lock state:
    *    - If node is unlocked (owner_node == 0):
    *      * Set owner_node to requested Node ID (grant lock)
    *    - If node is already locked:
    *      * If requested Node ID is 0:
    *        - Release lock (set owner_node to 0)
    *      * Otherwise keep existing lock holder
    * -# Load reply message header again (ensure clean state)
    * -# Copy CONFIG_MEM_RESERVE_LOCK_REPLY command byte to payload
    * -# Copy current owner_node (6 bytes) to payload at position 2
    * -# Set outgoing message as valid
    *
    * Lock behavior:
    * - Unlocked node: Grant lock to any requester
    * - Locked node: Only Node ID 0 can release lock
    * - Lock holder is always returned in the reply
    *
    * Use cases:
    * - Granting exclusive configuration access
    * - Releasing configuration lock
    * - Checking current lock holder
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_operations_request_info Pointer to request information
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning incoming_msg_info.msg_ptr must contain valid Lock/Reserve command with Node ID
    * @attention Lock state persists until explicitly released or node resets
    *
    * @see OpenLcbUtilities_extract_node_id_from_openlcb_payload
    * @see OpenLcbUtilities_copy_node_id_to_openlcb_payload
    */
void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    node_id_t new_node_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(
            statemachine_info->incoming_msg_info.msg_ptr,
            2);

    if (statemachine_info->openlcb_node->owner_node == 0) {

        statemachine_info->openlcb_node->owner_node = new_node_id;

    } else {

        if (new_node_id == 0) {

            statemachine_info->openlcb_node->owner_node = 0;

        }

    }

    _load_config_mem_reply_message_header(statemachine_info, config_mem_operations_request_info);

    OpenLcbUtilities_copy_byte_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            CONFIG_MEM_RESERVE_LOCK_REPLY,
            1);

    OpenLcbUtilities_copy_node_id_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            statemachine_info->openlcb_node->owner_node,
            2);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Entry point for processing Get Configuration Options command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for options command
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * This wrapper function sets up the request context and delegates to the
    * central handler which manages the two-phase processing (ACK then execute).
    *
    * Use cases:
    * - Called by datagram handler when Get Configuration Options command is received
    * - Entry point for options query processing
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid options command
    *
    * @see _handle_operations_request
    * @see ProtocolConfigMemOperationsHandler_request_options_cmd
    */
void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);
}

    /**
    * @brief Entry point for processing Get Configuration Options reply
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for options reply
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * This wrapper processes incoming options replies when acting as a configuration tool.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_options_cmd_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Get Address Space Information command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for address space info
    * -# Decode space identifier from payload to get space_info
    * -# Call central _handle_operations_request dispatcher
    *
    * The space identifier at payload offset 2 is decoded to find the appropriate
    * address space definition before processing.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Payload must contain valid space identifier at offset 2
    *
    * @see _handle_operations_request
    * @see _decode_to_space_definition
    */
void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Address Space Information Not Present reply
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for not present reply
    * -# Decode space identifier from payload to get space_info
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes replies indicating requested address space does not exist.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    * @see _decode_to_space_definition
    */
void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_not_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Address Space Information Present reply
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for present reply
    * -# Decode space identifier from payload to get space_info
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes replies containing address space information (size, flags, description).
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    * @see _decode_to_space_definition
    */
void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_address_space_info_reply_present;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Lock/Reserve command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for reserve lock
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming lock/reserve requests to grant or release exclusive configuration access.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    * @see ProtocolConfigMemOperationsHandler_request_reserve_lock
    */
void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Lock/Reserve reply
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for reserve lock reply
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming lock/reserve replies when acting as a configuration tool.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reserve_lock_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);


}

    /**
    * @brief Entry point for processing Get Unique ID command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for get unique ID
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming unique ID requests.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Get Unique ID reply
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for get unique ID reply
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming unique ID replies when acting as a configuration tool.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_get_unique_id_reply;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Unfreeze command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for unfreeze
    * -# Decode space identifier from payload to get space_info
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming unfreeze commands to resume operations in a specific address space.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    * @see ProtocolConfigMemOperationsHandler_freeze
    */
void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_unfreeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Freeze command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for freeze
    * -# Decode space identifier from payload to get space_info
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming freeze commands to suspend operations in a specific address space.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_operations_request
    * @see ProtocolConfigMemOperationsHandler_unfreeze
    */
void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_freeze;
    config_mem_operations_request_info.space_info = _decode_to_space_definition(statemachine_info, 2);

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Update Complete command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for update complete
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming update complete notifications indicating configuration changes are finished.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @attention Implementation may trigger configuration reload or node reset
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_update_complete;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Reset/Reboot command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for reset/reboot
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming reset/reboot commands to restart the node.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Implementation must handle safe shutdown before reset
    * @attention Node will become temporarily unavailable during reset
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_reset_reboot;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}

    /**
    * @brief Entry point for processing Factory Reset command
    *
    * @details Algorithm:
    * -# Create local config_mem_operations_request_info structure
    * -# Set operations_func to interface callback for factory reset
    * -# Set space_info to NULL (not space-specific operation)
    * -# Call central _handle_operations_request dispatcher
    *
    * Processes incoming factory reset commands to restore node to factory defaults.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Implementation must safely erase user configuration
    * @warning This is a destructive operation - all user settings will be lost
    * @attention Consider requiring confirmation before executing
    *
    * @see _handle_operations_request
    */
void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_operations_request_info_t config_mem_operations_request_info;

    config_mem_operations_request_info.operations_func = _interface->operations_request_factory_reset;
    config_mem_operations_request_info.space_info = NULL;

    _handle_operations_request(statemachine_info, &config_mem_operations_request_info);

}



