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
     * @brief Configuration memory read handler — two-phase dispatch for read
     *        commands across all standard address spaces (CDI, All, Config,
     *        ACDI-Mfg, ACDI-User, Train FDI, Train Fn Config).
     *
     * @author Jim Kueneman
     * @date 28 Feb 2026
     *
     * @see protocol_config_mem_read_handler.h
     * @see MemoryConfigurationS.pdf
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

    /** @brief Stored callback interface pointer; set by _initialize(). */
static interface_protocol_config_mem_read_handler_t *_interface;

    /**
     * @brief Stores the callback interface.  Call once at startup.
     *
     * @verbatim
     * @param interface_protocol_config_mem_read_handler  Populated table.
     * @endverbatim
     *
     * @warning Structure must remain valid for application lifetime.
     */
void ProtocolConfigMemReadHandler_initialize(const interface_protocol_config_mem_read_handler_t *interface_protocol_config_mem_read_handler) {

    _interface = (interface_protocol_config_mem_read_handler_t *) interface_protocol_config_mem_read_handler;
}

    /**
     * @brief Parse address, byte count, and encoding from the incoming read datagram.
     *
     * @details Algorithm:
     * -# Extract 4-byte address from payload[2..5]
     * -# Detect encoding: SPACE_IN_BYTE_6 vs SPACE_IN_BYTE_1
     * -# Set bytes, data_start accordingly
     *
     * @verbatim
     * @param statemachine_info            Context with incoming message.
     * @param config_mem_read_request_info  Output: populated request fields.
     * @endverbatim
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
     * @brief Validate read parameters: callback, space present, bounds, 1–64 bytes.
     *
     * @verbatim
     * @param config_mem_read_request_info  Request to validate.
     * @endverbatim
     *
     * @return S_OK or an OpenLCB error code.
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

    /** @brief Clamp byte count so the read does not exceed highest_address. */
static void _check_for_read_overrun(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    // Don't read past the end of the space

    if ((config_mem_read_request_info->address + config_mem_read_request_info->bytes) >= config_mem_read_request_info->space_info->highest_address) {

        config_mem_read_request_info->bytes = (uint8_t) (config_mem_read_request_info->space_info->highest_address - config_mem_read_request_info->address) + 1; // length +1 due to 0...end

    }
}

    /**
     * @brief Two-phase dispatcher: phase 1 validates + ACKs, phase 2 reads.
     *
     * @details Algorithm:
     * -# Extract parameters from incoming datagram
     * -# Phase 1: validate → reject or ACK + re-invoke
     * -# Phase 2: clamp overrun, call space-specific read, reset flags
     *
     * @verbatim
     * @param statemachine_info            Context.
     * @param config_mem_read_request_info  Carries callback + space_info.
     * @endverbatim
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

    /** @brief Read from CDI (0xFF): copy bytes from node->parameters->cdi[]. */
void ProtocolConfigMemReadHandler_read_request_config_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info) {

    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(statemachine_info, config_mem_read_request_info);

    OpenLcbUtilities_copy_byte_array_to_openlcb_payload(
            statemachine_info->outgoing_msg_info.msg_ptr,
            &statemachine_info->openlcb_node->parameters->cdi[config_mem_read_request_info->address],
            config_mem_read_request_info->data_start,
            config_mem_read_request_info->bytes);

    statemachine_info->outgoing_msg_info.valid = true;
}

    /** @brief Read from Train FDI (0xFA): copy bytes from node->parameters->fdi[]. */
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
     * @brief Read from Train Fn Config (0xF9): map flat byte address to functions[].
     *
     * @details Each 16-bit function value occupies 2 bytes big-endian:
     *          address/2 = fn_index, address%2 selects high/low byte.
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
     * @brief Read from Config space (0xFD) via config_memory_read callback.
     *
     * @details Partial reads (fewer bytes than requested) return TRANSFER_ERROR.
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

    /** @brief Read from ACDI-Mfg (0xFC): dispatch to SNIP loaders by address. */
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

    /** @brief Read from ACDI-User (0xFB): dispatch to SNIP loaders by address. */
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

    /** @brief Dispatch CDI (0xFF) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_config_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_configuration_definition;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch All (0xFE) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_all(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_all;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_all;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch Config (0xFD) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_config_memory(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_config_mem;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_config_memory;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch ACDI-Mfg (0xFC) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_acdi_manufacturer;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_manufacturer;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch ACDI-User (0xFB) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_acdi_user;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_acdi_user;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch Train FDI (0xFA) read to two-phase handler. */
void ProtocolConfigMemReadHandler_read_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info) {

    config_mem_read_request_info_t config_mem_read_request_info;

    config_mem_read_request_info.read_space_func = _interface->read_request_train_function_config_definition_info;
    config_mem_read_request_info.space_info = &statemachine_info->openlcb_node->parameters->address_space_train_function_definition_info;

    _handle_read_request(statemachine_info, &config_mem_read_request_info);
}

    /** @brief Dispatch Train Fn Config (0xF9) read to two-phase handler. */
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
