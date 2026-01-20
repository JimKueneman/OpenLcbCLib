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
 * @file protocol_snip.h
 * @brief Simple Node Information Protocol (SNIP) implementation
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_SNIP__
#define    __OPENLCB_PROTOCOL_SNIP__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Interface structure for Simple Node Information Protocol (SNIP) handler
     *
     * @details This structure defines the callback interface for the SNIP protocol handler,
     * which provides basic node identification information to other nodes on the OpenLCB network.
     * SNIP is a lightweight alternative to the full Memory Configuration Protocol for reading
     * node identification data.
     *
     * The SNIP protocol provides access to two categories of information:
     * - Manufacturer information (read-only): Version, name, model, hardware version, software version
     * - User information (read-write): Version, user-assigned name, user description
     *
     * The interface requires a configuration memory read callback to access user-editable
     * information stored in the ACDI User address space. Manufacturer information is
     * typically stored in the node parameters structure.
     *
     * SNIP data format consists of null-terminated strings in this order:
     * 1. Manufacturer version (1 byte)
     * 2. Manufacturer name (null-terminated string)
     * 3. Model (null-terminated string)
     * 4. Hardware version (null-terminated string)
     * 5. Software version (null-terminated string)
     * 6. User version (1 byte)
     * 7. User name (null-terminated string, from config memory)
     * 8. User description (null-terminated string, from config memory)
     *
     * @note Required callback must be set before calling ProtocolSnip_initialize
     *
     * @see ProtocolSnip_initialize
     * @see ProtocolSnip_handle_simple_node_info_request
     * @see OpenLCB Simple Node Information Protocol Specification
     */
typedef struct {

        /**
         * @brief Callback to read from configuration memory
         *
         * @details This required callback reads data from the node's configuration memory,
         * specifically the ACDI User address space containing user-assigned name and description.
         * The SNIP handler uses this to retrieve user-editable identification information.
         *
         * The callback should read the requested number of bytes from the specified address
         * in configuration memory and copy them into the provided buffer. Typical SNIP reads
         * access:
         * - User name at address USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS
         * - User description at address USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
   uint16_t(*config_memory_read)(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t* buffer);

} interface_openlcb_protocol_snip_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the Simple Node Information Protocol handler
     *
     * @details Registers the application's callback interface with the SNIP protocol handler.
     * Must be called once during system initialization before any SNIP messages are processed.
     *
     * The interface provides access to configuration memory for reading user-editable
     * node information (user name and user description).
     *
     * Use cases:
     * - Called during application startup
     * - Required before processing SNIP request messages
     *
     * @param interface_openlcb_protocol_snip Pointer to callback interface structure
     *
     * @warning interface_openlcb_protocol_snip must remain valid for lifetime of application
     * @warning config_memory_read callback must not be NULL
     * @warning NOT thread-safe - call during single-threaded initialization only
     *
     * @attention Call before enabling message reception
     *
     * @see interface_openlcb_protocol_snip_t - Callback interface structure
     */
    extern void ProtocolSnip_initialize(const interface_openlcb_protocol_snip_t *interface_openlcb_protocol_snip);

    /**
     * @brief Handles Simple Node Information Request message
     *
     * @details Processes an incoming SNIP request (MTI 0x0DE8) and generates a SNIP reply
     * containing all node identification information. Loads manufacturer data from node
     * parameters and user data from configuration memory.
     *
     * The reply message contains eight fields in sequence:
     * 1. Manufacturer version ID (1 byte)
     * 2. Manufacturer name (null-terminated string)
     * 3. Model (null-terminated string)
     * 4. Hardware version (null-terminated string)
     * 5. Software version (null-terminated string)
     * 6. User version ID (1 byte)
     * 7. User name (null-terminated string from config memory)
     * 8. User description (null-terminated string from config memory)
     *
     * Use cases:
     * - Configuration tools requesting node identification
     * - Network browsers displaying node information
     * - Diagnostic tools identifying nodes
     *
     * @param statemachine_info Pointer to state machine context containing incoming message
     *
     * @warning statemachine_info must NOT be NULL
     * @warning config_memory_read callback must be valid
     *
     * @note Sets outgoing_msg_info.valid to true with SNIP reply
     * @note Reply MTI is 0x0A08 (Simple Node Info Reply)
     * @note Maximum SNIP reply length is 253 bytes
     *
     * @attention Requires configuration memory read callback for user data
     *
     * @see ProtocolSnip_load_manufacturer_version_id - Loads manufacturer version
     * @see ProtocolSnip_load_name - Loads manufacturer name
     * @see ProtocolSnip_load_model - Loads model string
     * @see ProtocolSnip_load_hardware_version - Loads hardware version
     * @see ProtocolSnip_load_software_version - Loads software version
     * @see ProtocolSnip_load_user_version_id - Loads user version
     * @see ProtocolSnip_load_user_name - Loads user name from config memory
     * @see ProtocolSnip_load_user_description - Loads user description from config memory
     */
    extern void ProtocolSnip_handle_simple_node_info_request(openlcb_statemachine_info_t *statemachine_info);

    /**
     * @brief Handles Simple Node Information Reply message
     *
     * @details Processes an incoming SNIP reply (MTI 0x0A08) from another node on the network.
     * This handler is used when this node acts as a configuration tool requesting information
     * from other nodes.
     *
     * Use cases:
     * - Configuration tool receiving node identification data
     * - Network browser collecting node information
     * - Diagnostic tool cataloging network nodes
     *
     * @param statemachine_info Pointer to state machine context containing incoming message
     *
     * @warning statemachine_info must NOT be NULL
     *
     * @note Always sets outgoing_msg_info.valid to false (no automatic response)
     * @note Application should parse SNIP data from incoming message payload
     *
     * @see ProtocolSnip_validate_snip_reply - Validates SNIP reply format
     */
    extern void ProtocolSnip_handle_simple_node_info_reply(openlcb_statemachine_info_t *statemachine_info);


    /**
     * @brief Loads Manufacturer Version ID into SNIP reply message
     *
     * @details Copies the manufacturer version ID byte from the node parameters into the
     * outgoing message payload at the specified offset. The version ID is a single byte
     * indicating the version of the manufacturer information structure.
     *
     * Use cases:
     * - Building SNIP reply message
     * - First field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing manufacturer version in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where version byte should be written
     * @param requested_bytes Maximum number of bytes to copy (typically 1 for version byte)
     *
     * @return Number of bytes actually copied (always 1 for version ID)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note Version byte is typically 0x04 for current SNIP specification
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_manufacturer_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads Manufacturer Name string into SNIP reply message
     *
     * @details Copies the manufacturer name string from node parameters into the outgoing
     * message payload at the specified offset. The string is null-terminated and typically
     * contains the company or organization name that manufactured the node.
     *
     * Use cases:
     * - Building SNIP reply message
     * - Second field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing manufacturer name in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads Manufacturer Model string into SNIP reply message
     *
     * @details Copies the model name string from node parameters into the outgoing
     * message payload at the specified offset. The string is null-terminated and typically
     * contains the product model name or number.
     *
     * Use cases:
     * - Building SNIP reply message
     * - Third field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing model name in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_model(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads Manufacturer Hardware Version string into SNIP reply message
     *
     * @details Copies the hardware version string from node parameters into the outgoing
     * message payload at the specified offset. The string is null-terminated and typically
     * contains hardware revision information (e.g., "1.0", "Rev A", "v2.1").
     *
     * Use cases:
     * - Building SNIP reply message
     * - Fourth field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing hardware version in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_hardware_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads Manufacturer Software Version string into SNIP reply message
     *
     * @details Copies the software version string from node parameters into the outgoing
     * message payload at the specified offset. The string is null-terminated and typically
     * contains firmware or software version information (e.g., "1.0.5", "v2.3 beta").
     *
     * Use cases:
     * - Building SNIP reply message
     * - Fifth field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing software version in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_software_version(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads User Version ID into SNIP reply message
     *
     * @details Copies the user version ID byte from node parameters into the outgoing
     * message payload at the specified offset. The version ID is a single byte indicating
     * the version of the user information structure.
     *
     * Use cases:
     * - Building SNIP reply message
     * - Sixth field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node containing user version in parameters
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where version byte should be written
     * @param requested_bytes Maximum number of bytes to copy (typically 1 for version byte)
     *
     * @return Number of bytes actually copied (always 1 for version ID)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note Version byte is typically 0x02 for current SNIP specification
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     */
    extern uint16_t ProtocolSnip_load_user_version_id(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads User Name string into SNIP reply message from configuration memory
     *
     * @details Reads the user-assigned name from configuration memory and copies it into
     * the outgoing message payload at the specified offset. The string is null-terminated
     * and contains a user-editable friendly name for the node (e.g., "East Yard Turnout 1").
     *
     * Uses the config_memory_read callback to access the ACDI User address space at
     * USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS. The user name can be edited via
     * configuration tools or the Memory Configuration Protocol.
     *
     * Use cases:
     * - Building SNIP reply message
     * - Seventh field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node for configuration memory access
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     * @warning config_memory_read callback must be valid
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note Default maximum length is 63 bytes
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     * @see interface_openlcb_protocol_snip_t - Provides config_memory_read callback
     */
    extern uint16_t ProtocolSnip_load_user_name(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Loads User Description string into SNIP reply message from configuration memory
     *
     * @details Reads the user-assigned description from configuration memory and copies it
     * into the outgoing message payload at the specified offset. The string is null-terminated
     * and contains a user-editable description of the node (e.g., "Controls turnout motor 1").
     *
     * Uses the config_memory_read callback to access the ACDI User address space at
     * USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS. The user description can be edited
     * via configuration tools or the Memory Configuration Protocol.
     *
     * Use cases:
     * - Building SNIP reply message
     * - Eighth (final) field in SNIP data sequence
     *
     * @param openlcb_node Pointer to node for configuration memory access
     * @param outgoing_msg Pointer to message being constructed with SNIP data
     * @param offset Payload index where string should be written
     * @param requested_bytes Maximum number of bytes to copy (includes null terminator)
     *
     * @return Number of bytes actually copied (may be less than requested if string is shorter)
     *
     * @warning All parameters must NOT be NULL
     * @warning offset must be valid within message payload bounds
     * @warning config_memory_read callback must be valid
     *
     * @note Automatically updates outgoing_msg payload_count
     * @note String is null-terminated in the payload
     * @note Default maximum length is 64 bytes
     * @note If string exceeds requested_bytes, it may be truncated without null terminator
     *
     * @see ProtocolSnip_handle_simple_node_info_request - Uses this to build reply
     * @see interface_openlcb_protocol_snip_t - Provides config_memory_read callback
     */
    extern uint16_t ProtocolSnip_load_user_description(openlcb_node_t* openlcb_node, openlcb_msg_t* outgoing_msg, uint16_t offset, uint16_t requested_bytes);


    /**
     * @brief Validates the format of a Simple Node Information reply message
     *
     * @details Checks that a SNIP reply message conforms to the protocol specification
     * by verifying:
     * - Message MTI is correct (0x0A08)
     * - Payload length is within valid range
     * - Payload contains exactly 6 null terminators (separating 6 string fields)
     *
     * Use cases:
     * - Validating received SNIP replies before parsing
     * - Detecting malformed SNIP data
     * - Configuration tool data validation
     *
     * @param snip_reply_msg Pointer to message to validate
     *
     * @return true if message is a valid SNIP reply, false otherwise
     *
     * @warning snip_reply_msg must NOT be NULL
     *
     * @note Valid SNIP format has 8 fields: 2 version bytes + 6 null-terminated strings
     * @note Maximum SNIP reply length is 253 bytes
     *
     * @see ProtocolSnip_handle_simple_node_info_reply - Should validate before parsing
     */
    extern bool ProtocolSnip_validate_snip_reply(openlcb_msg_t* snip_reply_msg);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_SNIP__ */
