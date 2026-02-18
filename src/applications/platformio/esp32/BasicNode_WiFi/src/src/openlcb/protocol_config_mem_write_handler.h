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
     * @file protocol_config_mem_write_handler.h
     * @brief Configuration memory write protocol handler
     * @author Jim Kueneman
     * @date 17 Jan 2026
     */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Interface structure for configuration memory write protocol handler
     *
     * @details This structure defines the callback interface for handling OpenLCB
     * Configuration Memory Write protocol messages. It contains function pointers
     * for datagram acknowledgment, memory writing operations, and address space-specific
     * write handlers.
     *
     * The interface allows the application layer to customize behavior for different
     * address spaces while the protocol handler manages message formatting and
     * state machine logic according to the OpenLCB Memory Configuration Protocol.
     *
     * @note Required callbacks must be set before calling ProtocolConfigMemWriteHandler_initialize
     * @note Optional callbacks can be NULL if the corresponding functionality is not needed
     *
     * @see ProtocolConfigMemWriteHandler_initialize
     * @see config_mem_write_request_info_t
     */
typedef struct {

        /**
         * @brief Callback to load a datagram received OK acknowledgment message
         *
         * @details This required callback formats a positive datagram acknowledgment
         * message indicating the datagram was successfully received and will be processed.
         * The reply_pending_time parameter indicates when a response message will be sent.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_datagram_received_ok_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t reply_pending_time_in_seconds);

        /**
         * @brief Callback to load a datagram received rejected acknowledgment message
         *
         * @details This required callback formats a negative datagram acknowledgment
         * message indicating the datagram was rejected. The return_code specifies the
         * reason for rejection per OpenLCB error code definitions.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /**
         * @brief Callback to write data to configuration memory
         *
         * @details This required callback writes the specified number of bytes from
         * the provided buffer to the given address in configuration memory. The callback
         * returns the actual number of bytes written, which may be less than requested
         * if an error occurs or memory bounds are exceeded.
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Implementation should handle address validation and bounds checking
         * @note Implementation should handle write-protection and read-only regions
         */
    uint16_t(*config_memory_write) (openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

        /**
         * @brief Optional callback to handle writes to Configuration Definition Info space
         *
         * @details Processes write requests for address space 0xFF (CDI). Typically
         * this space is read-only, but this callback allows custom handling if needed.
         *
         * @note Optional - can be NULL if CDI space is read-only
         * @warning CDI space is typically read-only per OpenLCB specification
         */
    void (*write_request_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to All memory space
         *
         * @details Processes write requests for address space 0xFE (All Memory).
         * This space typically maps writes to the underlying appropriate space.
         *
         * @note Optional - can be NULL if All space writes are not supported
         */
    void (*write_request_all)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to Configuration Memory space
         *
         * @details Processes write requests for address space 0xFD (Configuration Memory),
         * which contains the node's actual configuration data that can be modified.
         *
         * @note Optional - can be NULL if config memory writes use default implementation
         */
    void (*write_request_config_mem)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to ACDI Manufacturer space
         *
         * @details Processes write requests for address space 0xFC (ACDI Manufacturer).
         * Typically this space is read-only as it contains factory-set information.
         *
         * @note Optional - can be NULL if ACDI manufacturer space is read-only
         * @warning ACDI manufacturer space is typically read-only per OpenLCB specification
         */
    void (*write_request_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to ACDI User space
         *
         * @details Processes write requests for address space 0xFB (ACDI User),
         * which contains user-defined identification information that can be modified.
         *
         * @note Optional - can be NULL if ACDI user writes use default implementation
         */
    void (*write_request_acdi_user)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to Train Function Definition space
         *
         * @details Processes write requests for address space 0xFA (Train Function CDI).
         * Typically this space is read-only as it contains XML structure definitions.
         *
         * @note Optional - can be NULL if train function definition space is read-only
         * @warning Train function CDI space is typically read-only
         */
    void (*write_request_train_function_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to Train Function Configuration space
         *
         * @details Processes write requests for address space 0xF9 (Train Function Config),
         * which contains actual train function configuration data that can be modified.
         *
         * @note Optional - can be NULL if train function config writes use default implementation
         */
    void (*write_request_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to handle writes to Firmware space
         *
         * @details Processes write requests for address space 0xEF (Firmware Update),
         * which is used for uploading new firmware to the node.
         *
         * @note Optional - can be NULL if firmware updates are not supported
         * @warning Implementation must handle firmware verification and safe update procedures
         */
    void (*write_request_firmware)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

        /**
         * @brief Optional callback to override reply delay time
         *
         * @details Allows the application to specify a custom delay time (in powers of 2 seconds)
         * before the write response will be sent. Used in the datagram ACK to inform the
         * requester when to expect the response.
         *
         * @note Optional - if NULL, default delay of 0 seconds is used
         * @note Return value is encoded as power of 2: return N means 2^N seconds delay
         */
    uint16_t (*delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t* config_mem_write_request_info);

    /**
     * @brief Notifier called when a train function value changes via 0xF9 write
     * @param openlcb_node The train node whose function changed
     * @param fn_address The function address that was modified
     * @param fn_value The new function value
     */
    void (*on_function_changed)(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value);

} interface_protocol_config_mem_write_handler_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the configuration memory write protocol handler
         *
         * @details Sets up the protocol handler with the required callback interface
         * for processing configuration memory write commands. This must be called once
         * during system initialization before any write operations can be processed.
         *
         * The interface structure provides callbacks for datagram acknowledgment
         * (required), memory writing (required), and address space-specific handlers
         * (optional based on supported features).
         *
         * Use cases:
         * - Called once during application startup
         * - Must be called before processing any configuration write datagrams
         *
         * @param interface_protocol_config_mem_write_handler Pointer to interface structure with callback functions
         *
         * @warning interface_protocol_config_mem_write_handler must not be NULL
         * @warning Required callbacks must be set (load_datagram_received_ok_message, load_datagram_received_rejected_message, config_memory_write)
         * @attention Call during initialization before enabling datagram reception
         *
         * @see interface_protocol_config_mem_write_handler_t
         */
    extern void ProtocolConfigMemWriteHandler_initialize(const interface_protocol_config_mem_write_handler_t *interface_protocol_config_mem_write_handler);

        /**
         * @brief Processes an incoming write command for Configuration Definition Info space
         *
         * @details Handles write requests for address space 0xFF (CDI). This space
         * is typically read-only, so this handler will normally reject write attempts.
         *
         * Use cases:
         * - Rejecting writes to read-only CDI space
         * - Custom CDI handling if writeable CDI is supported
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command
         * @attention CDI space is typically read-only per OpenLCB specification
         *
         * @see ProtocolConfigMemWriteHandler_write_space_config_memory
         */
    extern void ProtocolConfigMemWriteHandler_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for All memory space
         *
         * @details Handles write requests for address space 0xFE (All Memory), which
         * maps writes to the appropriate underlying writeable space.
         *
         * Use cases:
         * - Generic memory write handling
         * - Unified write access to all spaces
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command
         *
         * @see ProtocolConfigMemWriteHandler_write_space_config_memory
         */
    extern void ProtocolConfigMemWriteHandler_write_space_all(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for Configuration Memory space
         *
         * @details Handles write requests for address space 0xFD (Configuration Memory),
         * which contains the node's actual configuration data. Validates the request,
         * sends acknowledgment, and writes the data using the config_memory_write callback.
         *
         * Use cases:
         * - Writing node configuration values
         * - Responding to configuration tool write requests
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command with data
         * @warning config_memory_write callback must be implemented
         * @attention Writes may affect node behavior - handle carefully
         *
         * @see ProtocolConfigMemWriteHandler_write_request_config_mem
         */
    extern void ProtocolConfigMemWriteHandler_write_space_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for ACDI Manufacturer space
         *
         * @details Handles write requests for address space 0xFC (ACDI Manufacturer).
         * This space is typically read-only, so this handler will normally reject
         * write attempts.
         *
         * Use cases:
         * - Rejecting writes to read-only manufacturer info
         * - Factory programming of manufacturer data (special cases only)
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command
         * @attention ACDI manufacturer space is typically read-only
         *
         * @see ProtocolConfigMemWriteHandler_write_space_acdi_user
         */
    extern void ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for ACDI User space
         *
         * @details Handles write requests for address space 0xFB (ACDI User), which
         * contains user-defined identification information that can be modified (name,
         * description).
         *
         * Use cases:
         * - Writing user-defined node name
         * - Writing user description text
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command with data
         *
         * @see ProtocolConfigMemWriteHandler_write_request_acdi_user
         */
    extern void ProtocolConfigMemWriteHandler_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for Train Function Definition space
         *
         * @details Handles write requests for address space 0xFA (Train Function CDI).
         * This space is typically read-only, so this handler will normally reject
         * write attempts.
         *
         * Use cases:
         * - Rejecting writes to read-only train CDI
         * - Custom train CDI handling if writeable
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command
         * @attention Train function CDI is typically read-only
         *
         * @see ProtocolConfigMemWriteHandler_write_space_train_function_config_memory
         */
    extern void ProtocolConfigMemWriteHandler_write_space_train_function_definition_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for Train Function Configuration space
         *
         * @details Handles write requests for address space 0xF9 (Train Function Config),
         * which contains actual train function configuration data that can be modified.
         *
         * Use cases:
         * - Writing train function settings
         * - Configuring train functions
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid write command with data
         *
         * @see ProtocolConfigMemWriteHandler_write_space_train_function_definition_info
         */
    extern void ProtocolConfigMemWriteHandler_write_space_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming write command for Firmware space
         *
         * @details Handles write requests for address space 0xEF (Firmware Update),
         * used for uploading new firmware to the node. This is a critical operation
         * requiring careful validation and handling.
         *
         * Use cases:
         * - Uploading firmware updates
         * - Performing over-the-air updates
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid firmware data
         * @warning Implementation must verify firmware integrity before applying
         * @attention Firmware updates are critical operations - handle with care
         *
         * @see ProtocolConfigMemOperationsHandler_reset_reboot
         */
    extern void ProtocolConfigMemWriteHandler_write_space_firmware(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes a write command with bit mask
         *
         * @details Handles write-under-mask commands which allow modifying specific
         * bits in memory without affecting other bits. The command includes both data
         * and mask bytes.
         *
         * Use cases:
         * - Modifying specific configuration bits
         * - Atomic bit-level updates
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         * @param return_msg_ok Message type for successful write response
         * @param return_msg_fail Message type for failed write response
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemWriteHandler_write_space_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);


        /**
         * @brief Generates a write request for Configuration Memory space
         *
         * @details Creates and sends a write request targeting address space 0xFD
         * (Configuration Memory). This function is used when acting as a configuration
         * tool to write configuration data to other nodes.
         *
         * Use cases:
         * - Writing configuration values to target nodes
         * - Sending settings during configuration operations
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_write_request_info Pointer to write request information including address, count, and data
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_write_request_info must specify valid address, byte count, and data buffer
         * @warning Byte count must not exceed 64 bytes
         *
         * @see ProtocolConfigMemWriteHandler_write_space_config_memory
         */
    extern void ProtocolConfigMemWriteHandler_write_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Generates a write request for ACDI User space
         *
         * @details Creates and sends a write request targeting address space 0xFB
         * (ACDI User). This function is used when acting as a configuration tool
         * to write user-defined identification to other nodes.
         *
         * Use cases:
         * - Writing user-defined node names
         * - Setting custom node descriptions
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_write_request_info Pointer to write request information including address, count, and data
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_write_request_info must specify valid SNIP field address and data
         *
         * @see ProtocolConfigMemWriteHandler_write_space_acdi_user
         */
    extern void ProtocolConfigMemWriteHandler_write_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Writes to Train Function Configuration Memory space (0xF9)
         *
         * @details Writes function values from datagram data into the train's
         * in-RAM functions[] array. Function N at byte offset N*2, big-endian.
         * Fires on_function_changed notifier for each modified function.
         *
         * @param statemachine_info Pointer to state machine context
         * @param config_mem_write_request_info Pointer to write request information
         */
    extern void ProtocolConfigMemWriteHandler_write_request_train_function_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Processes a generic write message
         *
         * @details Handles incoming write messages for any address space. This function
         * provides a generic entry point for write command processing.
         *
         * Use cases:
         * - Generic write message handling
         * - Protocol-level write processing
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         * @param return_msg_ok Message type for successful write response
         * @param return_msg_fail Message type for failed write response
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemWriteHandler_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

        /**
         * @brief Processes a write reply OK message
         *
         * @details Handles incoming successful write response messages. Used when
         * this node is acting as a configuration tool and receives confirmation
         * that a write was successful.
         *
         * Use cases:
         * - Processing successful write confirmations
         * - Tracking write completion
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemWriteHandler_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);

        /**
         * @brief Processes a write reply fail message
         *
         * @details Handles incoming failed write response messages. Used when this
         * node is acting as a configuration tool and receives a rejection indicating
         * the write could not be completed.
         *
         * Use cases:
         * - Processing write error responses
         * - Handling write failures and retries
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemWriteHandler_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_WRITE_HANDLER__ */