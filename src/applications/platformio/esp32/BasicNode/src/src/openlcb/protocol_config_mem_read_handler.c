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
     * @file protocol_config_mem_read_handler.c
     * @brief Implementation of configuration memory read protocol handler
     * @author Jim Kueneman
     * @date 17 Jan 2026
     */

#include "protocol_config_mem_read_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_application_train.h"

static interface_protocol_config_mem_read_handler_t *_interface;

    /**
    * @brief Initializes the configuration memory read protocol handler
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
    * - Must be called before processing any configuration read datagrams
    *
    * @verbatim
    * @param interface_protocol_config_mem_read_handler Pointer to interface structure with callback functions
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Interface structure must remain valid throughout application lifetime
    * @warning Required callbacks must be set (load_datagram_received_ok_message, load_datagram_received_rejected_message, config_memory_read)
    * @attention Call during initialization before enabling datagram reception
    *
    * @see interface_protocol_config_mem_read_handler_t
    */
void ProtocolConfigMemReadHandler_initialize(const interface_protocol_config_mem_read_handler_t *interface_protocol_config_mem_read_handler) {

    _interface = (interface_protocol_config_mem_read_handler_t *) interface_protocol_config_mem_read_handler;
}

    /**
    * @brief Extracts read command parameters from incoming datagram payload
    *
    * @details Algorithm:
    * -# Extract address (4 bytes) from payload starting at position 2
    * -# Check command format by examining payload byte 1
    * -# If format is CONFIG_MEM_READ_SPACE_IN_BYTE_6:
    *    - Set encoding to ADDRESS_SPACE_IN_BYTE_6
    *    - Extract byte count from payload position 7
    *    - Set data_start to 7 (where reply data will begin)
    * -# Otherwise (standard format):
    *    - Set encoding to ADDRESS_SPACE_IN_BYTE_1
    *    - Extract byte count from payload position 6
    *    - Set data_start to 6 (where reply data will begin)
    *
    * OpenLCB supports two read command formats:
    * - Standard: Space in byte 1, address in bytes 2-5, count in byte 6
    * - Extended: Space in byte 6, address in bytes 2-5, count in byte 7
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to structure to populate with extracted parameters
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
    *
    * @see OpenLcbUtilities_extract_dword_from_openlcb_payload
    */
static void _extract_read_command_parameters(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    config_mem_read_request_info->address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);

    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == CONFIG_MEM_READ_SPACE_IN_BYTE_6) {

        config_mem_read_request_info->encoding = ADDRESS_SPACE_IN_BYTE_6;
        config_mem_read_request_info->bytes = *statemachine_info->incoming_msg_info.msg_ptr->payload[7];
        config_mem_read_request_info->data_start = 7;

    } else {

        config_mem_read_request_info->encoding = ADDRESS_SPACE_IN_BYTE_1;
        config_mem_read_request_info->bytes = *statemachine_info->incoming_msg_info.msg_ptr->payload[6];
        config_mem_read_request_info->data_start = 6;
    }

}

    /**
    * @brief Validates read command parameters for correctness
    *
    * @details Algorithm:
    * -# Check if read_space_func callback is registered
    *    - If NULL, return ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN
    * -# Check if address space is present (exists)
    *    - If not present, return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN
    * -# Check if requested address is within space bounds
    *    - If address > highest_address, return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS
    * -# Check if byte count exceeds maximum (64 bytes)
    *    - If bytes > 64, return ERROR_PERMANENT_INVALID_ARGUMENTS
    * -# Check if byte count is zero
    *    - If bytes == 0, return ERROR_PERMANENT_INVALID_ARGUMENTS
    * -# If all checks pass, return S_OK
    *
    * This function enforces OpenLCB protocol constraints:
    * - Maximum 64 bytes per read operation
    * - Address must be within defined space
    * - Byte count must be non-zero
    * - Space must exist and be supported
    *
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info containing parameters to validate
    * @endverbatim
    *
    * @return S_OK (0) if parameters are valid, otherwise OpenLCB error code
    *
    * @warning Pointer must not be NULL
    * @warning The space_info structure must be valid
    *
    * @see _check_for_read_overrun
    */
static uint16_t _is_valid_read_parameters(config_mem_read_request_info_t *config_mem_read_request_info) {

    if (!config_mem_read_request_info->read_space_func) {

        return ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN;
    }

    if (!config_mem_read_request_info->space_info->present) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN;
    }

    if (config_mem_read_request_info->address > config_mem_read_request_info->space_info->highest_address) {

        return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS;
    }

    if (config_mem_read_request_info->bytes > 64) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    if (config_mem_read_request_info->bytes == 0) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    return S_OK;
}

    /**
    * @brief Adjusts read byte count to prevent reading past end of address space
    *
    * @details Algorithm:
    * -# Calculate end address: requested_address + requested_bytes
    * -# Check if end address exceeds space highest_address
    * -# If overrun detected:
    *    - Calculate safe byte count: (highest_address - address) + 1
    *    - Update bytes in request_info to safe count
    *    - Note: +1 is due to inclusive addressing (0...end)
    * -# If no overrun, bytes remain unchanged
    *
    * This function ensures read operations never access beyond the defined
    * address space boundaries, preventing potential errors or undefined behavior.
    *
    * Example: Space with highest_address=99, read request at address=95 for 10 bytes
    * - Would try to read 95-104, but max is 99
    * - Adjusted to read (99-95)+1 = 5 bytes instead
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context (unused but maintained for consistency)
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info, bytes field may be modified
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Both structure parameters must be valid
    * @attention Silently truncates read to space boundary
    *
    * @see _is_valid_read_parameters
    */
static void _check_for_read_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    // Don't read past the end of the space

    if ((config_mem_read_request_info->address + config_mem_read_request_info->bytes) >= config_mem_read_request_info->space_info->highest_address) {

        config_mem_read_request_info->bytes = (uint8_t) (config_mem_read_request_info->space_info->highest_address - config_mem_read_request_info->address) + 1; // length +1 due to 0...end

    }
}

    /**
    * @brief Central dispatcher for configuration memory read requests
    *
    * @details Algorithm:
    * -# Extract read command parameters from incoming message
    * -# Check if datagram acknowledgment has been sent
    * -# If not sent yet (first call):
    *    - Validate read parameters
    *    - If validation fails:
    *      * Load datagram rejected message with error code
    *      * Return
    *    - If validation succeeds:
    *      * Check for delayed_reply_time callback
    *      * If present, call it to get delay value
    *      * If absent, use default delay of 0x00
    *      * Load datagram OK message with delay value
    *      * Set openlcb_datagram_ack_sent flag to true
    *      * Set enumerate flag to true (re-invoke handler)
    *      * Return
    * -# If ACK already sent (second call):
    *    - Check for address overrun and adjust byte count if needed
    *    - Invoke space-specific read callback to read data
    *    - Reset openlcb_datagram_ack_sent flag to false
    *    - Reset enumerate flag to false
    *
    * This function implements a two-phase processing pattern:
    * - Phase 1: Validate request and send datagram acknowledgment
    * - Phase 2: Execute the actual read operation via registered callback
    *
    * The two-phase approach allows the datagram ACK to be sent quickly while
    * potentially time-consuming read operations are deferred to the second invocation.
    *
    * Use cases:
    * - All configuration memory read command processing
    * - Coordinating datagram ACK with read execution
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request information including read callback
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Read callback in request_info must be valid
    * @attention Uses state flags to implement two-phase processing
    *
    * @see _extract_read_command_parameters
    * @see _is_valid_read_parameters
    * @see _check_for_read_overrun
    */
static void _handle_read_request(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    uint16_t error_code = S_OK;

    _extract_read_command_parameters(statemachine_info, config_mem_read_request_info);

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        error_code = _is_valid_read_parameters(config_mem_read_request_info);

        if (error_code) {

            _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

        } else {

            if (_interface->delayed_reply_time) {

                _interface->load_datagram_received_ok_message(statemachine_info, _interface->delayed_reply_time(statemachine_info, config_mem_read_request_info));

            } else {

                _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

            }

            statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
            statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data
        }

        return;
    }

    // Try to Complete Command Request, we know that config_mem_read_request_info->read_space_func is valid if we get here

    _check_for_read_overrun(statemachine_info, config_mem_read_request_info);
    config_mem_read_request_info->read_space_func(statemachine_info, config_mem_read_request_info);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // Done
    statemachine_info->incoming_msg_info.enumerate = false; // done
}

    /**
    * @brief Processes a read request for Configuration Definition Info (CDI) space
    *
    * @details Algorithm:
    * -# Load read reply OK message header into outgoing message
    * -# Copy CDI data from node parameters to outgoing payload:
    *    - Source: node->parameters->cdi array at requested address offset
    *    - Destination: payload starting at data_start position
    *    - Count: requested number of bytes
    * -# Set outgoing message as valid
    *
    * This function reads XML configuration definition data from the CDI buffer
    * stored in the node's parameters. The CDI describes the structure and meaning
    * of the configuration memory.
    *
    * Use cases:
    * - Responding to CDI read requests from configuration tools
    * - Providing configuration structure information
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address and byte count
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning CDI buffer in node parameters must be valid
    * @warning Address and byte count must have been validated by caller
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload
    */
void ProtocolConfigMemReadHandler_read_request_config_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    OpenLcbUtilities_copy_byte_array_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            &statemachine_info->openlcb_node->parameters->cdi[config_mem_read_request_info->address],
            config_mem_read_request_info->data_start,
            config_mem_read_request_info->bytes);

    statemachine_info->outgoing_msg_info.valid = true;
}

    /**
    * @brief Processes a read request for Train Function Definition Info (FDI) space (0xFA)
    *
    * @details Algorithm:
    * -# Load read reply OK message header into outgoing message
    * -# Copy FDI data from node parameters to outgoing payload:
    *    - Source: node->parameters->fdi array at requested address offset
    *    - Destination: payload starting at data_start position
    *    - Count: requested number of bytes
    * -# Set outgoing message as valid
    *
    * This function reads XML function definition data from the FDI buffer
    * stored in the node's parameters. The FDI describes the train's function
    * layout (kind, number, name) similar to how CDI describes configuration.
    * FDI is read-only.
    *
    * Use cases:
    * - Responding to FDI read requests from configuration tools (JMRI)
    * - Providing function description information for train nodes
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address and byte count
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning FDI buffer in node parameters must be valid
    * @warning Address and byte count must have been validated by caller
    *
    * @see ProtocolConfigMemReadHandler_read_request_config_definition_info
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_copy_byte_array_to_openlcb_payload
    */
void ProtocolConfigMemReadHandler_read_request_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    OpenLcbUtilities_copy_byte_array_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            &statemachine_info->openlcb_node->parameters->fdi[config_mem_read_request_info->address],
            config_mem_read_request_info->data_start,
            config_mem_read_request_info->bytes);

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Processes a read request for Train Function Configuration Memory space (0xF9)
    *
    * @details Algorithm:
    * -# Load read reply OK message header into outgoing message
    * -# Get train state for the node
    * -# If train state exists:
    *    - Iterate over requested bytes
    *    - For each byte, calculate function index (address / 2) and byte selector (address % 2)
    *    - Byte selector 0 = high byte (big-endian), byte selector 1 = low byte
    *    - Copy each byte to outgoing payload
    * -# Set outgoing message as valid
    *
    * This function reads function values from the train_state_t.functions[] array
    * as a flat byte array in big-endian format. Function N's 16-bit value occupies
    * byte offsets N*2 (high byte) and N*2+1 (low byte). Bulk reads spanning
    * multiple functions are supported.
    *
    * Use cases:
    * - Responding to function value read requests from configuration tools
    * - Bulk reading multiple function values in a single datagram
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address and byte count
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Node must have train_state initialized via OpenLcbApplicationTrain_setup()
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_copy_byte_to_openlcb_payload
    */
void ProtocolConfigMemReadHandler_read_request_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    train_state_t *state = OpenLcbApplicationTrain_get_state(statemachine_info->openlcb_node);

    if (state) {

        uint32_t addr = config_mem_read_request_info->address;
        uint16_t bytes = config_mem_read_request_info->bytes;
        uint16_t payload_offset = config_mem_read_request_info->data_start;

        for (uint16_t i = 0; i < bytes; i++) {

            uint16_t fn_index = (addr + i) / 2;
            uint8_t byte_sel = (addr + i) % 2;

            uint8_t val = 0;

            if (fn_index < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

                val = (byte_sel == 0)
                        ? (uint8_t) (state->functions[fn_index] >> 8)
                        : (uint8_t) (state->functions[fn_index] & 0xFF);

            }

            OpenLcbUtilities_copy_byte_to_openlcb_payload(
                    statemachine_info->outgoing_msg_info.msg_ptr,
                    val,
                    payload_offset + i);

        }

    }

    statemachine_info->outgoing_msg_info.valid = true;

}

    /**
    * @brief Processes a read request for Configuration Memory space
    *
    * @details Algorithm:
    * -# Check if config_memory_read callback is registered
    * -# If callback exists:
    *    - Load read reply OK message header
    *    - Call config_memory_read callback to read data:
    *      * Pass node pointer, address, byte count, and destination buffer
    *      * Buffer points to outgoing payload at data_start position
    *    - Store actual bytes read count
    *    - Update outgoing payload_count by adding bytes read
    *    - Check if read count is less than requested:
    *      * If partial read, load read fail message with TRANSFER_ERROR
    *      * Set outgoing message valid
    *      * Return
    *    - If full read succeeded, set outgoing message valid
    * -# If callback not registered:
    *    - Load read fail message with INVALID_ARGUMENTS error
    *    - Set outgoing message valid
    *
    * This function delegates the actual memory reading to the application-provided
    * callback, allowing flexible implementation of configuration storage (EEPROM,
    * flash, RAM, etc.). Partial reads are treated as errors.
    *
    * Use cases:
    * - Reading actual configuration data values
    * - Responding to configuration tool read requests
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address and byte count
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning The callback should be registered
    * @attention Partial reads (fewer bytes than requested) are treated as errors
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_load_config_mem_reply_read_fail_message_header
    */
void ProtocolConfigMemReadHandler_read_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    if (_interface->config_memory_read) {

        OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

        uint16_t read_count = _interface->config_memory_read(
                statemachine_info->openlcb_node,
                config_mem_read_request_info->address,
                config_mem_read_request_info->bytes,
                (configuration_memory_buffer_t*) & statemachine_info->outgoing_msg_info.msg_ptr->payload[config_mem_read_request_info->data_start]
                );

        statemachine_info->outgoing_msg_info.msg_ptr->payload_count += read_count;

        if (read_count < config_mem_read_request_info->bytes) {

            OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_TEMPORARY_TRANSFER_ERROR);

            statemachine_info->outgoing_msg_info.valid = true;

            return;

        }


        statemachine_info->outgoing_msg_info.valid = true;

    } else {

        OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

        statemachine_info->outgoing_msg_info.valid = true;

    }

}

    /**
    * @brief Processes a read request for ACDI Manufacturer space
    *
    * @details Algorithm:
    * -# Load read reply OK message header
    * -# Use switch statement on requested address to determine SNIP field:
    *    - CONFIG_MEM_ACDI_MANUFACTURER_VERSION_ADDRESS:
    *      * Check if snip_load_manufacturer_version_id callback exists
    *      * If exists, call it to load version data
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_MANUFACTURER_NAME_ADDRESS:
    *      * Check if snip_load_name callback exists
    *      * If exists, call it to load manufacturer name
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_MANUFACTURER_MODEL_ADDRESS:
    *      * Check if snip_load_model callback exists
    *      * If exists, call it to load model name
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_HARDWARE_VERSION_ADDRESS:
    *      * Check if snip_load_hardware_version callback exists
    *      * If exists, call it to load hardware version
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_SOFTWARE_VERSION_ADDRESS:
    *      * Check if snip_load_software_version callback exists
    *      * If exists, call it to load software version
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - default (unrecognized address):
    *      * Load datagram rejected message with ADDRESS_SPACE_UNKNOWN error
    * -# Set outgoing message valid
    *
    * This function maps fixed ACDI manufacturer addresses to SNIP (Simple Node
    * Ident Protocol) data fields containing factory-set identification information.
    *
    * ACDI Manufacturer field addresses:
    * - Version: Manufacturer-assigned version number
    * - Name: Manufacturer company name
    * - Model: Product model name
    * - Hardware Version: Hardware revision string
    * - Software Version: Software/firmware version string
    *
    * Use cases:
    * - Responding to manufacturer identification queries
    * - Providing node hardware/software version information
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address indicating which field
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Appropriate SNIP callbacks should be registered for supported fields
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_load_config_mem_reply_read_fail_message_header
    */
void ProtocolConfigMemReadHandler_read_request_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    switch (config_mem_read_request_info->address) {

        case CONFIG_MEM_ACDI_MANUFACTURER_VERSION_ADDRESS:

            if (_interface->snip_load_manufacturer_version_id) {

                _interface->snip_load_manufacturer_version_id(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_MANUFACTURER_ADDRESS:

            if (_interface->snip_load_name) {

                _interface->snip_load_name(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_MODEL_ADDRESS:

            if (_interface->snip_load_model) {

                _interface->snip_load_model(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_HARDWARE_VERSION_ADDRESS:

            if (_interface->snip_load_hardware_version) {

                _interface->snip_load_hardware_version(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_SOFTWARE_VERSION_ADDRESS:

            if (_interface->snip_load_software_version) {

                _interface->snip_load_software_version(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        default:

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);

            statemachine_info->outgoing_msg_info.valid = true;

            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

    /**
    * @brief Processes a read request for ACDI User space
    *
    * @details Algorithm:
    * -# Load read reply OK message header
    * -# Use switch statement on requested address to determine SNIP field:
    *    - CONFIG_MEM_ACDI_USER_VERSION_ADDRESS:
    *      * Check if snip_load_user_version_id callback exists
    *      * If exists, call it to load user version data
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_USER_NAME_ADDRESS:
    *      * Check if snip_load_user_name callback exists
    *      * If exists, call it to load user-defined name
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS:
    *      * Check if snip_load_user_description callback exists
    *      * If exists, call it to load user description
    *      * If not, load read fail message with INVALID_ARGUMENTS
    *    - default (unrecognized address):
    *      * Load datagram rejected message with ADDRESS_SPACE_UNKNOWN error
    * -# Set outgoing message valid
    *
    * This function maps fixed ACDI user addresses to SNIP data fields containing
    * user-customizable identification information.
    *
    * ACDI User field addresses:
    * - Version: User-assigned version/revision number
    * - Name: User-defined node name (e.g. "Front Porch Light")
    * - Description: User-defined description (e.g. "Controls porch lighting")
    *
    * Use cases:
    * - Responding to user identification queries
    * - Providing custom node naming information
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_read_request_info Pointer to request info with address indicating which field
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Appropriate SNIP callbacks should be registered for supported fields
    *
    * @see OpenLcbUtilities_load_config_mem_reply_read_ok_message_header
    * @see OpenLcbUtilities_load_config_mem_reply_read_fail_message_header
    */
void ProtocolConfigMemReadHandler_read_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    switch (config_mem_read_request_info->address) {

        case CONFIG_MEM_ACDI_USER_VERSION_ADDRESS:

            if (_interface->snip_load_user_version_id) {

                _interface->snip_load_user_version_id(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_USER_NAME_ADDRESS:


            if (_interface->snip_load_user_name) {

                _interface->snip_load_user_name(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        case CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS:

            if (_interface->snip_load_user_description) {

                _interface->snip_load_user_description(
                        statemachine_info->openlcb_node,
                        statemachine_info->outgoing_msg_info.msg_ptr,
                        config_mem_read_request_info->data_start,
                        config_mem_read_request_info->bytes
                        );

            } else {

                OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(statemachine_info, config_mem_read_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

                statemachine_info->outgoing_msg_info.valid = true;

            }

            break;

        default:

            _interface->load_datagram_received_rejected_message(statemachine_info, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);

            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

    /**
    * @brief Entry point for processing read command for Configuration Definition Info space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for CDI reads
    * -# Set space_info to point to CDI address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper function sets up the request context for CDI reads and delegates
    * to the central handler which manages the two-phase processing (ACK then execute).
    *
    * Use cases:
    * - Called by datagram handler when CDI read command is received
    * - Entry point for CDI read processing
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
    *
    * @see _handle_read_request
    * @see ProtocolConfigMemReadHandler_read_request_config_definition_info
    */
void ProtocolConfigMemReadHandler_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_config_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for All memory space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for All space reads
    * -# Set space_info to point to All address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to the unified All memory space which provides
    * access to all readable memory.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_read_request
    */
void ProtocolConfigMemReadHandler_read_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_all;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for Configuration Memory space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for config memory reads
    * -# Set space_info to point to Configuration Memory address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to the actual configuration data storage.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning The callback must be registered
    *
    * @see _handle_read_request
    * @see ProtocolConfigMemReadHandler_read_request_config_mem
    */
void ProtocolConfigMemReadHandler_read_space_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_config_mem;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for ACDI Manufacturer space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for ACDI manufacturer reads
    * -# Set space_info to point to ACDI Manufacturer address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to manufacturer identification fields.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Appropriate SNIP callbacks should be registered
    *
    * @see _handle_read_request
    * @see ProtocolConfigMemReadHandler_read_request_acdi_manufacturer
    */
void ProtocolConfigMemReadHandler_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_acdi_manufacturer;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for ACDI User space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for ACDI user reads
    * -# Set space_info to point to ACDI User address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to user-defined identification fields.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Appropriate SNIP callbacks should be registered
    *
    * @see _handle_read_request
    * @see ProtocolConfigMemReadHandler_read_request_acdi_user
    */
void ProtocolConfigMemReadHandler_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_acdi_user;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for Train Function Definition space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for train function CDI reads
    * -# Set space_info to point to Train Function Definition address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to train function configuration structure (XML).
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_read_request
    */
void ProtocolConfigMemReadHandler_read_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_train_function_config_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /**
    * @brief Entry point for processing read command for Train Function Configuration space
    *
    * @details Algorithm:
    * -# Create local config_mem_read_request_info structure
    * -# Set read_space_func to interface callback for train function config reads
    * -# Set space_info to point to Train Function Config address space definition
    * -# Call central _handle_read_request dispatcher
    *
    * This wrapper processes reads to train function configuration data.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _handle_read_request
    */
void ProtocolConfigMemReadHandler_read_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_train_function_config_memory;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

// Message handling stub functions are documented in the header file
// These are intentional stubs reserved for future implementation

void ProtocolConfigMemReadHandler_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {

    // Intentional stub - reserved for future implementation

}

void ProtocolConfigMemReadHandler_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {

    // Intentional stub - reserved for future implementation

}

void ProtocolConfigMemReadHandler_read_reply_reject_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {

    // Intentional stub - reserved for future implementation

}
