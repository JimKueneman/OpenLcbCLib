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
     * @file protocol_config_mem_read_handler.h
     * @brief Configuration memory read protocol handler
     * @author Jim Kueneman
     * @date 17 Jan 2026
     */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Interface structure for configuration memory read protocol handler
     *
     * @details This structure defines the callback interface for handling OpenLCB
     * Configuration Memory Read protocol messages. It contains function pointers
     * for datagram acknowledgment, memory reading operations, SNIP (Simple Node
     * Ident Protocol) field loading, and address space-specific read handlers.
     *
     * The interface allows the application layer to customize behavior for different
     * address spaces while the protocol handler manages message formatting and
     * state machine logic according to the OpenLCB Memory Configuration Protocol.
     *
     * @note Required callbacks must be set before calling ProtocolConfigMemReadHandler_initialize
     * @note Optional callbacks can be NULL if the corresponding functionality is not needed
     * @note SNIP callbacks are only required if ACDI (Abbreviated Configuration Description Information) spaces 0xFB/0xFC are enabled
     *
     * @see ProtocolConfigMemReadHandler_initialize
     * @see config_mem_read_request_info_t
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
         * @brief Callback to read data from configuration memory
         *
         * @details This required callback reads the specified number of bytes from
         * the given address in configuration memory and stores them in the provided
         * buffer. The callback returns the actual number of bytes read, which may be
         * less than requested if an error occurs or end of memory is reached.
         *
         * @note This is a REQUIRED callback - must not be NULL
         * @note Implementation should handle address validation and bounds checking
         */
    uint16_t(*config_memory_read)(openlcb_node_t* openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

        /**
         * @brief Optional callback to load manufacturer version ID for SNIP
         *
         * @details Loads the manufacturer version number into the message payload
         * for ACDI manufacturer space (0xFC) reads. Only required if ACDI manufacturer
         * protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFC is supported
         */
    uint16_t(*snip_load_manufacturer_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load manufacturer name for SNIP
         *
         * @details Loads the manufacturer name string into the message payload
         * for ACDI manufacturer space (0xFC) reads. Only required if ACDI manufacturer
         * protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFC is supported
         */
    uint16_t(*snip_load_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load model name for SNIP
         *
         * @details Loads the model name string into the message payload
         * for ACDI manufacturer space (0xFC) reads. Only required if ACDI manufacturer
         * protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFC is supported
         */
    uint16_t(*snip_load_model)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load hardware version for SNIP
         *
         * @details Loads the hardware version string into the message payload
         * for ACDI manufacturer space (0xFC) reads. Only required if ACDI manufacturer
         * protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFC is supported
         */
    uint16_t(*snip_load_hardware_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load software version for SNIP
         *
         * @details Loads the software version string into the message payload
         * for ACDI manufacturer space (0xFC) reads. Only required if ACDI manufacturer
         * protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFC is supported
         */
    uint16_t(*snip_load_software_version)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load user version ID for SNIP
         *
         * @details Loads the user-defined version number into the message payload
         * for ACDI user space (0xFB) reads. Only required if ACDI user protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFB is supported
         */
    uint16_t(*snip_load_user_version_id)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load user name for SNIP
         *
         * @details Loads the user-defined name string into the message payload
         * for ACDI user space (0xFB) reads. Only required if ACDI user protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFB is supported
         */
    uint16_t(*snip_load_user_name)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to load user description for SNIP
         *
         * @details Loads the user-defined description string into the message payload
         * for ACDI user space (0xFB) reads. Only required if ACDI user protocol is enabled.
         *
         * @note Optional - only required if Address Space 0xFB is supported
         */
    uint16_t(*snip_load_user_description)(openlcb_node_t* openlcb_node, openlcb_msg_t* worker_msg, uint16_t payload_index, uint16_t requested_bytes);

        /**
         * @brief Optional callback to handle reads from Configuration Definition Info space
         *
         * @details Processes read requests for address space 0xFF (CDI - Configuration
         * Definition Information), which contains XML describing the node's configuration structure.
         *
         * @note Optional - can be NULL if CDI space reads are handled by default implementation
         */
    void (*read_request_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from All memory space
         *
         * @details Processes read requests for address space 0xFE (All Memory),
         * which provides access to all readable memory spaces.
         *
         * @note Optional - can be NULL if All space reads are not supported
         */
    void (*read_request_all)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from Configuration Memory space
         *
         * @details Processes read requests for address space 0xFD (Configuration Memory),
         * which contains the node's actual configuration data.
         *
         * @note Optional - can be NULL if config memory reads use default implementation
         */
    void (*read_request_config_mem)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from ACDI Manufacturer space
         *
         * @details Processes read requests for address space 0xFC (ACDI Manufacturer),
         * which contains manufacturer identification information.
         *
         * @note Optional - can be NULL if ACDI manufacturer reads use default implementation
         */
    void (*read_request_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from ACDI User space
         *
         * @details Processes read requests for address space 0xFB (ACDI User),
         * which contains user-defined identification information.
         *
         * @note Optional - can be NULL if ACDI user reads use default implementation
         */
    void (*read_request_acdi_user)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from Traction Function Definition space
         *
         * @details Processes read requests for address space 0xFA (Traction Function CDI),
         * which contains XML describing traction function configurations.
         *
         * @note Optional - can be NULL if traction function definition reads are not supported
         */
    void (*read_request_traction_function_config_definition_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to handle reads from Traction Function Configuration space
         *
         * @details Processes read requests for address space 0xF9 (Traction Function Config),
         * which contains actual traction function configuration data.
         *
         * @note Optional - can be NULL if traction function config reads are not supported
         */
    void (*read_request_traction_function_config_memory)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

        /**
         * @brief Optional callback to override reply delay time
         *
         * @details Allows the application to specify a custom delay time (in powers of 2 seconds)
         * before the read response will be sent. Used in the datagram ACK to inform the
         * requester when to expect the response.
         *
         * @note Optional - if NULL, default delay of 0 seconds is used
         * @note Return value is encoded as power of 2: return N means 2^N seconds delay
         */
    uint16_t (*delayed_reply_time)(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t* config_mem_read_request_info);

} interface_protocol_config_mem_read_handler_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the configuration memory read protocol handler
         *
         * @details Sets up the protocol handler with the required callback interface
         * for processing configuration memory read commands. This must be called once
         * during system initialization before any read operations can be processed.
         *
         * The interface structure provides callbacks for datagram acknowledgment
         * (required), memory reading (required), SNIP field loading (optional), and
         * address space-specific handlers (optional).
         *
         * Use cases:
         * - Called once during application startup
         * - Must be called before processing any configuration read datagrams
         *
         * @param interface_protocol_config_mem_read_handler Pointer to interface structure with callback functions
         *
         * @warning interface_protocol_config_mem_read_handler must not be NULL
         * @warning Required callbacks must be set (load_datagram_received_ok_message, load_datagram_received_rejected_message, config_memory_read)
         * @attention Call during initialization before enabling datagram reception
         *
         * @see interface_protocol_config_mem_read_handler_t
         */
    extern void ProtocolConfigMemReadHandler_initialize(const interface_protocol_config_mem_read_handler_t *interface_protocol_config_mem_read_handler);


        /**
         * @brief Processes an incoming read command for Configuration Definition Info space
         *
         * @details Handles read requests for address space 0xFF (CDI), which contains
         * XML describing the node's configuration structure. Validates the request,
         * sends datagram acknowledgment, and generates a response with the requested
         * CDI data.
         *
         * Use cases:
         * - Responding to configuration tool CDI queries
         * - Providing configuration structure information
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         *
         * @see ProtocolConfigMemReadHandler_read_request_config_definition_info
         */
    extern void ProtocolConfigMemReadHandler_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for All memory space
         *
         * @details Handles read requests for address space 0xFE (All Memory), which
         * provides unified access to all readable memory spaces. Validates the request,
         * sends acknowledgment, and generates a response with data from the appropriate
         * underlying space.
         *
         * Use cases:
         * - Responding to generic memory read requests
         * - Providing access to all readable memory
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         *
         * @see ProtocolConfigMemReadHandler_read_request_all
         */
    extern void ProtocolConfigMemReadHandler_read_space_all(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for Configuration Memory space
         *
         * @details Handles read requests for address space 0xFD (Configuration Memory),
         * which contains the node's actual configuration data. Validates the request,
         * sends acknowledgment, and reads the requested data using the config_memory_read
         * callback.
         *
         * Use cases:
         * - Reading node configuration values
         * - Responding to configuration tool read requests
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         * @warning config_memory_read callback must be implemented
         *
         * @see ProtocolConfigMemReadHandler_read_request_config_mem
         */
    extern void ProtocolConfigMemReadHandler_read_space_config_memory(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for ACDI Manufacturer space
         *
         * @details Handles read requests for address space 0xFC (ACDI Manufacturer),
         * which contains manufacturer identification information (name, model, versions).
         * Uses SNIP callbacks to load the appropriate fields.
         *
         * Use cases:
         * - Responding to manufacturer info queries
         * - Providing node identification to configuration tools
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         * @warning Appropriate SNIP callbacks must be implemented for requested fields
         *
         * @see ProtocolConfigMemReadHandler_read_request_acdi_manufacturer
         */
    extern void ProtocolConfigMemReadHandler_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for ACDI User space
         *
         * @details Handles read requests for address space 0xFB (ACDI User), which
         * contains user-defined identification information (name, description). Uses
         * SNIP callbacks to load the appropriate fields.
         *
         * Use cases:
         * - Responding to user-defined info queries
         * - Providing custom node identification
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         * @warning Appropriate SNIP callbacks must be implemented for requested fields
         *
         * @see ProtocolConfigMemReadHandler_read_request_acdi_user
         */
    extern void ProtocolConfigMemReadHandler_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for Traction Function Definition space
         *
         * @details Handles read requests for address space 0xFA (Traction Function CDI),
         * which contains XML describing traction function configurations for train control.
         *
         * Use cases:
         * - Responding to traction CDI queries
         * - Providing function configuration structure for trains
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         *
         * @see ProtocolConfigMemReadHandler_read_request_traction_function_config_definition_info
         */
    extern void ProtocolConfigMemReadHandler_read_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming read command for Traction Function Configuration space
         *
         * @details Handles read requests for address space 0xF9 (Traction Function Config),
         * which contains actual traction function configuration data for train control.
         *
         * Use cases:
         * - Reading traction function settings
         * - Responding to train configuration queries
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid read command
         *
         * @see ProtocolConfigMemReadHandler_read_request_traction_function_config_memory
         */
    extern void ProtocolConfigMemReadHandler_read_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info);



        /**
         * @brief Generates a read request for Configuration Definition Info space
         *
         * @details Creates and sends a read request targeting address space 0xFF (CDI).
         * This function is used when acting as a configuration tool to retrieve CDI
         * information from other nodes.
         *
         * Use cases:
         * - Requesting CDI from target nodes
         * - Discovering configuration structure of other nodes
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_read_request_info Pointer to read request information including address and count
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_read_request_info must specify valid address and byte count
         *
         * @see ProtocolConfigMemReadHandler_read_space_config_description_info
         */
    extern void ProtocolConfigMemReadHandler_read_request_config_definition_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Generates a read request for Configuration Memory space
         *
         * @details Creates and sends a read request targeting address space 0xFD
         * (Configuration Memory). This function is used when acting as a configuration
         * tool to read configuration data from other nodes.
         *
         * Use cases:
         * - Reading configuration values from target nodes
         * - Retrieving settings during configuration operations
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_read_request_info Pointer to read request information including address and count
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_read_request_info must specify valid address and byte count
         * @warning Byte count must not exceed 64 bytes
         *
         * @see ProtocolConfigMemReadHandler_read_space_config_memory
         */
    extern void ProtocolConfigMemReadHandler_read_request_config_mem(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Generates a read request for ACDI Manufacturer space
         *
         * @details Creates and sends a read request targeting address space 0xFC
         * (ACDI Manufacturer). This function is used when acting as a configuration
         * tool to retrieve manufacturer information from other nodes.
         *
         * Use cases:
         * - Requesting manufacturer identification
         * - Discovering node hardware and software versions
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_read_request_info Pointer to read request information including address and count
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_read_request_info must specify valid SNIP field address
         *
         * @see ProtocolConfigMemReadHandler_read_space_acdi_manufacturer
         */
    extern void ProtocolConfigMemReadHandler_read_request_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Generates a read request for ACDI User space
         *
         * @details Creates and sends a read request targeting address space 0xFB
         * (ACDI User). This function is used when acting as a configuration tool
         * to retrieve user-defined identification from other nodes.
         *
         * Use cases:
         * - Requesting user-defined node name
         * - Retrieving custom node description
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_read_request_info Pointer to read request information including address and count
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_read_request_info must specify valid SNIP field address
         *
         * @see ProtocolConfigMemReadHandler_read_space_acdi_user
         */
    extern void ProtocolConfigMemReadHandler_read_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);



        /**
         * @brief Processes a generic read message
         *
         * @details Handles incoming read messages for any address space. This function
         * provides a generic entry point for read command processing.
         *
         * Use cases:
         * - Generic read message handling
         * - Protocol-level read processing
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         * @param return_msg_ok Message type for successful read response
         * @param return_msg_fail Message type for failed read response
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemReadHandler_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail);

        /**
         * @brief Processes a read reply OK message
         *
         * @details Handles incoming successful read response messages. Used when
         * this node is acting as a configuration tool and receives read data from
         * another node.
         *
         * Use cases:
         * - Processing successful read responses
         * - Extracting read data from replies
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemReadHandler_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);

        /**
         * @brief Processes a read reply reject message
         *
         * @details Handles incoming failed read response messages. Used when this
         * node is acting as a configuration tool and receives a rejection from
         * another node indicating the read could not be completed.
         *
         * Use cases:
         * - Processing read error responses
         * - Handling read failures
         *
         * @param statemachine_info Pointer to state machine context
         * @param space Address space identifier
         *
         * @warning statemachine_info must not be NULL
         *
         * @note Intentional stub - reserved for future implementation
         */
    extern void ProtocolConfigMemReadHandler_read_reply_reject_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_READ_HANDLER__ */

