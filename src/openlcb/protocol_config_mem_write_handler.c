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
 * @brief Configuration memory write handler — two-phase dispatch for write
 *        commands across all standard address spaces.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see protocol_config_mem_write_handler.h
 * @see MemoryConfigurationS.pdf
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


    /** @brief Stored callback interface pointer; set by _initialize(). */
static interface_protocol_config_mem_write_handler_t* _interface;

    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @verbatim
     * @param interface_protocol_config_mem_write_handler  Populated table.
     * @endverbatim
     *
     * @warning Structure must remain valid for application lifetime.
     */
void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler) {

    _interface = (interface_protocol_config_mem_write_handler_t*) interface_protocol_config_mem_write_handler;

}

    /**
     * @brief Parse address, byte count, encoding, and write_buffer from incoming write datagram.
     *
     * @verbatim
     * @param statemachine_info             Context with incoming message.
     * @param config_mem_write_request_info  Output: populated request fields.
     * @endverbatim
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
     * @brief Validate write parameters: space present, not read-only, bounds, 1–64 bytes.
     *
     * @return S_OK or an OpenLCB error code.
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

    /** @brief Clamp byte count so the write does not exceed highest_address. */
static void _check_for_write_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info) {

    // Don't read past the end of the space

    if ((config_mem_write_request_info->address + config_mem_write_request_info->bytes) >= config_mem_write_request_info->space_info->highest_address) {

        config_mem_write_request_info->bytes = (uint8_t) (config_mem_write_request_info->space_info->highest_address - config_mem_write_request_info->address) + 1; // length +1 due to 0...end

    }
}

    /**
     * @brief Two-phase dispatcher: phase 1 validates + ACKs, phase 2 writes.
     *
     * @details Algorithm:
     * -# Extract parameters from incoming datagram
     * -# Phase 1: validate → reject or ACK + re-invoke
     * -# Phase 2: clamp overrun, call space-specific write, reset flags
     *
     * @verbatim
     * @param statemachine_info             Context.
     * @param config_mem_write_request_info  Carries callback + space_info.
     * @endverbatim
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

    /** @brief Dispatch CDI (0xFF) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch All (0xFE) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_all;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch Config (0xFD) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_config_mem;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch ACDI-Mfg (0xFC) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_manufacturer;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch ACDI-User (0xFB) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_acdi_user;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch Train FDI (0xFA) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_train_function_config_definition_info;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch Train Fn Config (0xF9) write to two-phase handler. */
void ProtocolConfigMemWriteHandler_write_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_write_request_info_t config_mem_write_request_info;

    config_mem_write_request_info.write_space_func = _interface->write_request_train_function_config_memory;
    config_mem_write_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_config_memory;

    _dispatch_write_request(statemachine_info, &config_mem_write_request_info);
}

    /** @brief Dispatch Firmware (0xEF) write to two-phase handler. */
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

        for (uint16_t fn = first_fn; fn <= last_fn && fn < USER_DEFINED_MAX_TRAIN_FUNCTIONS; fn++) {

            if (_interface && _interface->on_function_changed) {

                _interface->on_function_changed(
                        statemachine_info->openlcb_node,
                        (uint32_t) fn,
                        state->functions[fn]);

            }

        }

    }

    statemachine_info->outgoing_msg_info.valid = true;

}
