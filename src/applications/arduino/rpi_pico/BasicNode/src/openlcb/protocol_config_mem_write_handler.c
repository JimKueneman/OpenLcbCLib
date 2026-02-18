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
 * @file protocol_config_mem_write_handler.c
 * @brief Implementation of configuration memory write protocol handler
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "protocol_config_mem_write_handler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf
#include <string.h>

#include "openlcb_types.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_application_train.h"
#include "protocol_train_handler.h"


static interface_protocol_config_mem_write_handler_t* _interface;

    /**
    * @brief Initializes the configuration memory write protocol handler
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
    * - Must be called before processing any configuration write datagrams
    *
    * @verbatim
    * @param interface_protocol_config_mem_write_handler Pointer to interface structure with callback functions
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Interface structure must remain valid throughout application lifetime
    * @warning Required callbacks must be set (load_datagram_received_ok_message, load_datagram_received_rejected_message, config_memory_write)
    * @attention Call during initialization before enabling datagram reception
    *
    * @see interface_protocol_config_mem_write_handler_t
    */
void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler) {

    _interface = (interface_protocol_config_mem_write_handler_t*) interface_protocol_config_mem_write_handler;

}

    /**
    * @brief Extracts write command parameters from incoming datagram payload
    *
    * @details Algorithm:
    * -# Check command format by examining payload byte 1
    * -# If format is CONFIG_MEM_WRITE_SPACE_IN_BYTE_6:
    *    - Set encoding to ADDRESS_SPACE_IN_BYTE_6
    *    - Calculate bytes: payload_count - 7 (header overhead)
    *    - Set data_start to 7 (where write data begins in payload)
    * -# Otherwise (standard format):
    *    - Set encoding to ADDRESS_SPACE_IN_BYTE_1
    *    - Calculate bytes: payload_count - 6 (header overhead)
    *    - Set data_start to 6 (where write data begins in payload)
    * -# Extract address (4 bytes) from payload starting at position 2
    * -# Set write_buffer pointer to payload at data_start position
    *
    * OpenLCB supports two write command formats:
    * - Standard: Space in byte 1, address in bytes 2-5, data starts at byte 6
    * - Extended: Space in byte 6, address in bytes 2-5, data starts at byte 7
    *
    * The write buffer pointer is cast to configuration_memory_buffer_t* to
    * allow the config_memory_write callback to access the data directly from
    * the payload without copying.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to structure to populate with extracted parameters
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command with data
    *
    * @see OpenLcbUtilities_extract_dword_from_openlcb_payload
    */
static void _extract_write_command_parameters(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    if (*statemachine_info->incoming_msg_info.msg_ptr->payload[1] == CONFIG_MEM_WRITE_SPACE_IN_BYTE_6) {

        config_mem_write_request_info->encoding = ADDRESS_SPACE_IN_BYTE_6;
        config_mem_write_request_info->bytes = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 7;
        config_mem_write_request_info->data_start = 7;

    } else {

        config_mem_write_request_info->encoding = ADDRESS_SPACE_IN_BYTE_1;
        config_mem_write_request_info->bytes = statemachine_info->incoming_msg_info.msg_ptr->payload_count - 6;
        config_mem_write_request_info->data_start = 6;
    }

    config_mem_write_request_info->address = OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 2);
    config_mem_write_request_info->write_buffer = (configuration_memory_buffer_t*) & statemachine_info->incoming_msg_info.msg_ptr->payload[config_mem_write_request_info->data_start];

}

    /**
    * @brief Validates write command parameters for correctness
    *
    * @details Algorithm:
    * -# Check if address space is present (exists)
    *    - If not present, return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN
    * -# Check if address space is read-only
    *    - If read-only, return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY
    * -# Check if requested address is within space bounds
    *    - If address > highest_address, return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS
    * -# Check if byte count exceeds maximum (64 bytes)
    *    - If bytes > 64, return ERROR_PERMANENT_INVALID_ARGUMENTS
    * -# Check if byte count is zero
    *    - If bytes == 0, return ERROR_PERMANENT_INVALID_ARGUMENTS
    * -# If all checks pass, return S_OK
    *
    * This function enforces OpenLCB protocol constraints:
    * - Maximum 64 bytes per write operation
    * - Address must be within defined space
    * - Byte count must be non-zero
    * - Space must exist, be supported, and be writeable
    *
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info containing parameters to validate
    * @endverbatim
    *
    * @return S_OK (0) if parameters are valid, otherwise OpenLCB error code
    *
    * @warning Pointer must not be NULL
    * @warning space_info must be valid
    *
    * @see _check_for_write_overrun
    */
static uint16_t _is_valid_write_parameters(config_mem_write_request_info_t *config_mem_write_request_info) {

    if (!config_mem_write_request_info->space_info->present) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN;
    }

    if (config_mem_write_request_info->space_info->read_only) {

        return ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY;
    }

    if (config_mem_write_request_info->address > config_mem_write_request_info->space_info->highest_address) {

        return ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS;
    }

    if (config_mem_write_request_info->bytes > 64) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    if (config_mem_write_request_info->bytes == 0) {

        return ERROR_PERMANENT_INVALID_ARGUMENTS;
    }

    return S_OK;
}

    /**
    * @brief Adjusts write byte count to prevent writing past end of address space
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
    * This function ensures write operations never access beyond the defined
    * address space boundaries, preventing potential corruption or undefined behavior.
    *
    * Example: Space with highest_address=99, write request at address=95 for 10 bytes
    * - Would try to write 95-104, but max is 99
    * - Adjusted to write (99-95)+1 = 5 bytes instead
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context (unused but maintained for consistency)
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info, bytes field may be modified
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning space_info and highest_address must be valid
    * @attention Silently truncates write to space boundary
    *
    * @see _is_valid_write_parameters
    */
static void _check_for_write_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    // Don't read past the end of the space

    if ((config_mem_write_request_info->address + config_mem_write_request_info->bytes) >= config_mem_write_request_info->space_info->highest_address) {

        config_mem_write_request_info->bytes = (uint8_t) (config_mem_write_request_info->space_info->highest_address - config_mem_write_request_info->address) + 1; // length +1 due to 0...end

    }
}

    /**
    * @brief Central dispatcher for configuration memory write requests
    *
    * @details Algorithm:
    * -# Extract write command parameters from incoming message
    * -# Check if datagram acknowledgment has been sent
    * -# If not sent yet (first call):
    *    - Validate write parameters
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
    *    - Invoke space-specific write callback to write data
    *    - Reset openlcb_datagram_ack_sent flag to false
    *    - Reset enumerate flag to false
    *
    * This function implements a two-phase processing pattern:
    * - Phase 1: Validate request and send datagram acknowledgment
    * - Phase 2: Execute the actual write operation via registered callback
    *
    * The two-phase approach allows the datagram ACK to be sent quickly while
    * potentially time-consuming write operations (e.g., EEPROM writes) are
    * deferred to the second invocation.
    *
    * Use cases:
    * - All configuration memory write command processing
    * - Coordinating datagram ACK with write execution
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request information including write callback
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Write callback in request_info must be valid
    * @attention Uses state flags to implement two-phase processing
    *
    * @see _extract_write_command_parameters
    * @see _is_valid_write_parameters
    * @see _check_for_write_overrun
    */
static void _dispatch_write_request(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    uint16_t error_code = S_OK;

    _extract_write_command_parameters(statemachine_info, config_mem_write_request_info);

    if (!statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent) {

        error_code = _is_valid_write_parameters(config_mem_write_request_info);

        if (error_code) {

            _interface->load_datagram_received_rejected_message(statemachine_info, error_code);

        } else {

            if (_interface->delayed_reply_time) {

                _interface->load_datagram_received_ok_message(statemachine_info, _interface->delayed_reply_time(statemachine_info, config_mem_write_request_info));

            } else {

                _interface->load_datagram_received_ok_message(statemachine_info, 0x00);

            }

            statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = true;
            statemachine_info->incoming_msg_info.enumerate = true; // call this again for the data
        }

        return;
    }

    // Try to Complete Command Request, we know that config_mem_write_request_info->write_space_func is valid if we get here

    _check_for_write_overrun(statemachine_info, config_mem_write_request_info);
    config_mem_write_request_info->write_space_func(statemachine_info, config_mem_write_request_info);

    statemachine_info->openlcb_node->state.openlcb_datagram_ack_sent = false; // Done
    statemachine_info->incoming_msg_info.enumerate = false; // done

}

    /**
    * @brief Entry point for processing write command for Configuration Definition Info space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for CDI writes
    * -# Set space_info to point to CDI address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper function sets up the request context for CDI writes. Note that
    * CDI space is typically read-only, so this handler will normally reject write
    * attempts unless custom writeable CDI is implemented.
    *
    * Use cases:
    * - Called by datagram handler when CDI write command is received
    * - Typically rejects writes (CDI is read-only)
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @attention CDI space is typically read-only per OpenLCB specification
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for All memory space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for All space writes
    * -# Set space_info to point to All address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to the unified All memory space which maps
    * writes to the appropriate underlying writeable space.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_all;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for Configuration Memory space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for config memory writes
    * -# Set space_info to point to Configuration Memory address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to the actual configuration data storage.
    * This is the primary writeable space for node configuration.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning config_memory_write callback must be registered
    *
    * @see _dispatch_write_request
    * @see ProtocolConfigMemWriteHandler_write_request_config_mem
    */
void ProtocolConfigMemWriteHandler_write_space_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_mem;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for ACDI Manufacturer space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for ACDI manufacturer writes
    * -# Set space_info to point to ACDI Manufacturer address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to manufacturer identification fields. Note that
    * ACDI Manufacturer space is typically read-only (factory-set), so this handler
    * will normally reject write attempts.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @attention ACDI Manufacturer space is typically read-only
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_manufacturer;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for ACDI User space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for ACDI user writes
    * -# Set space_info to point to ACDI User address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to user-defined identification fields (name, description).
    * This space is writeable to allow users to customize node identification.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _dispatch_write_request
    * @see ProtocolConfigMemWriteHandler_write_request_acdi_user
    */
void ProtocolConfigMemWriteHandler_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_user;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for Train Function Definition space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for train function CDI writes
    * -# Set space_info to point to Train Function Definition address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to train function configuration structure (XML).
    * This space is typically read-only.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @attention Train Function CDI space is typically read-only
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_train_function_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for Train Function Configuration space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for train function config writes
    * -# Set space_info to point to Train Function Config address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to train function configuration data.
    * This space is writeable for configuring train functions.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_train_function_config_memory;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /**
    * @brief Entry point for processing write command for Firmware space
    *
    * @details Algorithm:
    * -# Create local config_mem_write_request_info structure
    * -# Set write_space_func to interface callback for firmware writes
    * -# Set space_info to point to Firmware address space definition
    * -# Call central _dispatch_write_request dispatcher
    *
    * This wrapper processes writes to firmware update space. This is a critical
    * operation used for uploading new firmware to the node.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context containing incoming message
    * @endverbatim
    *
    * @warning Pointer must not be NULL
    * @warning Implementation must verify firmware integrity before applying
    * @attention Firmware updates are critical operations - handle with care
    *
    * @see _dispatch_write_request
    */
void ProtocolConfigMemWriteHandler_write_space_firmware(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_firmware;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_firmware;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

// Message handling stub functions are documented in the header file
// These are intentional stubs reserved for future implementation

    /**
    * @brief Processes a write command with bit mask (stub)
    *
    * @details This stub is reserved for implementing write-under-mask operations
    * which allow modifying specific bits in memory without affecting other bits.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param space Address space identifier
    * @endverbatim
    * @verbatim
    * @param return_msg_ok Message type for successful write response
    * @endverbatim
    * @verbatim
    * @param return_msg_fail Message type for failed write response
    * @endverbatim
    *
    * @note Intentional stub - reserved for future implementation
    */
void ProtocolConfigMemWriteHandler_write_space_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {

    // Intentional stub - reserved for future implementation

}

    /**
    * @brief Processes a generic write message (stub)
    *
    * @details This stub provides a generic entry point for write command processing.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param space Address space identifier
    * @endverbatim
    * @verbatim
    * @param return_msg_ok Message type for successful write response
    * @endverbatim
    * @verbatim
    * @param return_msg_fail Message type for failed write response
    * @endverbatim
    *
    * @note Intentional stub - reserved for future implementation
    */
void ProtocolConfigMemWriteHandler_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail) {

    // Intentional stub - reserved for future implementation

}

    /**
    * @brief Processes a write reply OK message (stub)
    *
    * @details This stub handles successful write response messages when acting
    * as a configuration tool.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param space Address space identifier
    * @endverbatim
    *
    * @note Intentional stub - reserved for future implementation
    */
void ProtocolConfigMemWriteHandler_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {

    // Intentional stub - reserved for future implementation

}

    /**
    * @brief Processes a write reply fail message (stub)
    *
    * @details This stub handles failed write response messages when acting
    * as a configuration tool.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context
    * @endverbatim
    * @verbatim
    * @param space Address space identifier
    * @endverbatim
    *
    * @note Intentional stub - reserved for future implementation
    */
void ProtocolConfigMemWriteHandler_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space) {

    // Intentional stub - reserved for future implementation

}

// ----------------------------------------------------------------------------
// Implemented Write Requests
// ----------------------------------------------------------------------------


    /**
    * @brief Performs the actual configuration memory write operation
    *
    * @details Algorithm:
    * -# Initialize write_count to 0
    * -# Check if config_memory_write callback is registered
    * -# If callback exists:
    *    - Call config_memory_write to write data:
    *      * Pass node pointer, address, byte count, and source buffer
    *      * Source buffer points to write data from incoming datagram
    *    - Store actual bytes written count
    *    - Update outgoing payload_count by adding bytes written
    *    - Check if write count is less than requested:
    *      * If partial write, load write fail message with TRANSFER_ERROR
    * -# If callback not registered:
    *    - Load write fail message with INVALID_ARGUMENTS error
    * -# Set outgoing message valid
    * -# Return actual bytes written
    *
    * This function delegates the actual memory writing to the application-provided
    * callback, allowing flexible implementation of configuration storage (EEPROM,
    * flash, RAM, etc.). Partial writes are treated as errors.
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info with address, byte count, and data
    * @endverbatim
    *
    * @return Number of bytes actually written to configuration memory
    *
    * @warning Both parameters must not be NULL
    * @warning config_memory_write callback should be registered
    * @attention Partial writes (fewer bytes than requested) are treated as errors
    *
    * @see OpenLcbUtilities_load_config_mem_reply_write_fail_message_header
    */
static uint16_t _write_data(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    uint16_t write_count = 0;

    if (_interface->config_memory_write) {

        write_count = _interface->config_memory_write(
                statemachine_info->openlcb_node,
                config_mem_write_request_info->address,
                config_mem_write_request_info->bytes,
                config_mem_write_request_info->write_buffer
                );

        statemachine_info->outgoing_msg_info.msg_ptr->payload_count += write_count;

        if (write_count < config_mem_write_request_info->bytes) {

            OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_TEMPORARY_TRANSFER_ERROR);

        }

    } else {


        OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_PERMANENT_INVALID_ARGUMENTS);

    }

    statemachine_info->outgoing_msg_info.valid = true;

    return write_count;

}

    /**
    * @brief Processes a write request for Configuration Memory space
    *
    * @details Algorithm:
    * -# Load write reply OK message header
    * -# Call _write_data to perform the actual write operation
    *
    * This function handles writes to the primary configuration data storage space.
    * The actual write is delegated to the config_memory_write callback which can
    * implement any storage mechanism (EEPROM, flash, RAM, etc.).
    *
    * Use cases:
    * - Writing configuration values to non-volatile storage
    * - Responding to configuration tool write requests
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info with address, byte count, and data buffer
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning config_memory_write callback must be implemented
    * @attention Writes may affect node behavior - ensure data is validated before writing
    *
    * @see _write_data
    * @see OpenLcbUtilities_load_config_mem_reply_write_ok_message_header
    */
void ProtocolConfigMemWriteHandler_write_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

     OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    _write_data(statemachine_info, config_mem_write_request_info);

}

    /**
    * @brief Processes a write request for ACDI User space
    *
    * @details Algorithm:
    * -# Load write reply OK message header
    * -# Use switch statement on requested address to determine SNIP field:
    *    - CONFIG_MEM_ACDI_USER_NAME_ADDRESS:
    *      * Call _write_data to write user name
    *    - CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS:
    *      * Call _write_data to write user description
    *    - default (unrecognized address):
    *      * Load write fail message with OUT_OF_BOUNDS_INVALID_ADDRESS error
    * -# Set outgoing message valid
    *
    * This function maps fixed ACDI user addresses to SNIP data fields for
    * user-customizable identification information. Only name and description
    * fields are writeable.
    *
    * ACDI User writeable fields:
    * - Name: User-defined node name (e.g. "Front Porch Light")
    * - Description: User-defined description (e.g. "Controls porch lighting")
    *
    * Use cases:
    * - Writing user-defined node name
    * - Writing user description text
    * - Customizing node identification
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info with address indicating which field and data to write
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning config_memory_write callback must be implemented
    * @attention User name and description fields have size limits
    *
    * @see _write_data
    * @see OpenLcbUtilities_load_config_mem_reply_write_ok_message_header
    * @see OpenLcbUtilities_load_config_mem_reply_write_fail_message_header
    */
void ProtocolConfigMemWriteHandler_write_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    switch (config_mem_write_request_info->address) {

        case CONFIG_MEM_ACDI_USER_NAME_ADDRESS:

            _write_data(statemachine_info, config_mem_write_request_info);

            break;

        case CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS:

            _write_data(statemachine_info, config_mem_write_request_info);

            break;

        default:

            OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(statemachine_info, config_mem_write_request_info, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);

            break;
    }

    statemachine_info->outgoing_msg_info.valid = true;
}

    /**
    * @brief Processes a write request for Train Function Configuration Memory space (0xF9)
    *
    * @details Algorithm:
    * -# Load write reply OK message header
    * -# Get train state for the node
    * -# If train state exists:
    *    - Iterate over incoming bytes
    *    - For each byte, calculate function index (address / 2) and byte selector (address % 2)
    *    - Byte selector 0 = high byte (big-endian), byte selector 1 = low byte
    *    - Update the corresponding byte of the function value
    *    - Fire on_function_changed notifier for each function whose bytes were touched
    * -# Set outgoing message as valid
    *
    * This function writes function values into the train_state_t.functions[] array
    * from datagram data using big-endian byte order. Function N's 16-bit value
    * occupies byte offsets N*2 (high byte) and N*2+1 (low byte). Bulk writes
    * spanning multiple functions are supported.
    *
    * After storing the values, this fires the same on_function_changed notifier
    * that Set Function commands use, ensuring consistent application behavior
    * regardless of whether the function was set via Train Control command or
    * via Memory Config write to 0xF9.
    *
    * Use cases:
    * - Writing function values from configuration tools (JMRI)
    * - Bulk writing multiple function values in a single datagram
    *
    * @verbatim
    * @param statemachine_info Pointer to state machine context for message generation
    * @endverbatim
    * @verbatim
    * @param config_mem_write_request_info Pointer to request info with address, data, and byte count
    * @endverbatim
    *
    * @warning Both parameters must not be NULL
    * @warning Node must have train_state initialized via OpenLcbApplicationTrain_setup()
    *
    * @see OpenLcbUtilities_load_config_mem_reply_write_ok_message_header
    * @see ProtocolTrainHandler_get_interface
    */
void ProtocolConfigMemWriteHandler_write_request_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(statemachine_info, config_mem_write_request_info);

    train_state_t *state = OpenLcbApplicationTrain_get_state(statemachine_info->openlcb_node);

    if (state) {

        uint32_t address = config_mem_write_request_info->address;
        uint16_t bytes = config_mem_write_request_info->bytes;

        for (uint16_t i = 0; i < bytes; i++) {

            uint16_t fn_index = (address + i) / 2;
            uint8_t byte_sel = (address + i) % 2;

            if (fn_index < USER_DEFINED_MAX_TRAIN_FUNCTIONS) {

                uint8_t incoming = (*config_mem_write_request_info->write_buffer)[i];

                if (byte_sel == 0) {

                    state->functions[fn_index] = (state->functions[fn_index] & 0x00FF) | ((uint16_t) incoming << 8);

                } else {

                    state->functions[fn_index] = (state->functions[fn_index] & 0xFF00) | (uint16_t) incoming;

                }

            }

        }

        uint16_t first_fn = address / 2;
        uint16_t last_fn = (address + bytes - 1) / 2;
        const interface_protocol_train_handler_t *train_iface = ProtocolTrainHandler_get_interface();

        for (uint16_t fn = first_fn; fn <= last_fn && fn < USER_DEFINED_MAX_TRAIN_FUNCTIONS; fn++) {

            if (train_iface && train_iface->on_function_changed) {

                train_iface->on_function_changed(
                        statemachine_info->openlcb_node,
                        (uint32_t) fn,
                        state->functions[fn]);

            }

        }

    }

    statemachine_info->outgoing_msg_info.valid = true;

}
