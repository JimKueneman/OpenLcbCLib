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
    * @details This structure defines the callback interface for handling OpenLCB
    * datagram protocol operations. It contains function pointers for resource management,
    * configuration memory operations, and datagram transport handling as defined in the
    * OpenLCB Datagram Transport and Memory Configuration Protocol specifications.
    *
    * The interface allows the application layer to customize behavior for different
    * memory operations and transport methods while the datagram handler manages the
    * message formatting, acknowledgment, rejection, and state machine logic.
    *
    * The structure organizes callbacks into functional groups:
    * - Resource locking (required)
    * - Memory read operations via datagram transport
    * - Memory read operations via stream transport
    * - Memory write operations via datagram transport
    * - Memory write operations via stream transport
    * - Write-under-mask operations
    * - Configuration memory commands
    * - Reply handlers for client-initiated operations
    *
    * @note Required callbacks must be set before calling ProtocolDatagramHandler_initialize
    * @note Optional callbacks can be NULL if the corresponding functionality is not needed
    * @note Setting an optional callback to NULL will result in datagram rejection with
    * ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN when that operation is requested
    *
    * @see ProtocolDatagramHandler_initialize
    * @see openlcb_statemachine_info_t
    */
typedef struct {

        /**
         * @brief Callback to lock shared resources before buffer access
         *
         * @details This required callback provides mutual exclusion protection
         * for shared data structures during datagram processing. Typically disables
         * interrupts or acquires a mutex to prevent concurrent access.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*lock_shared_resources)(void);

        /**
         * @brief Callback to unlock shared resources after buffer access
         *
         * @details This required callback releases mutual exclusion protection
         * established by lock_shared_resources. Typically re-enables interrupts
         * or releases a mutex.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*unlock_shared_resources)(void);

        /**
         * @brief Optional callback to handle Configuration Description Information read via datagram
         *
         * @details Handles incoming datagram requests to read from Configuration Description
         * Information (CDI) address space (0xFF), which contains XML describing the node's
         * configuration structure.
         *
         * @note Optional - can be NULL if CDI reads are not supported via datagram
         */
    void (*memory_read_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle All Memory Space read via datagram
         *
         * @details Handles incoming datagram requests to read from All Memory Space (0xFD),
         * which provides unified access to all readable memory regions.
         *
         * @note Optional - can be NULL if All Memory Space reads are not supported via datagram
         */
    void (*memory_read_space_all)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Memory read via datagram
         *
         * @details Handles incoming datagram requests to read from Configuration Memory
         * address space (0xFE), which contains node configuration variables.
         *
         * @note Optional - can be NULL if Configuration Memory reads are not supported via datagram
         */
    void (*memory_read_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI Manufacturer Information read via datagram
         *
         * @details Handles incoming datagram requests to read from ACDI Manufacturer
         * address space (0xFC), which contains read-only manufacturer identification data.
         *
         * @note Optional - can be NULL if ACDI Manufacturer reads are not supported via datagram
         */
    void (*memory_read_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI User Information read via datagram
         *
         * @details Handles incoming datagram requests to read from ACDI User address space
         * (0xFB), which contains user-editable identification data.
         *
         * @note Optional - can be NULL if ACDI User reads are not supported via datagram
         */
    void (*memory_read_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Definition Info read via datagram
         *
         * @details Handles incoming datagram requests to read from Train Function
         * Definition Information address space (0xFA), which describes available train
         * control functions.
         *
         * @note Optional - can be NULL if Train FDI reads are not supported via datagram
         */
    void (*memory_read_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Configuration read via datagram
         *
         * @details Handles incoming datagram requests to read from Train Function
         * Configuration address space (0xF9), which contains train function settings.
         *
         * @note Optional - can be NULL if Train Function Config reads are not supported via datagram
         */
    void (*memory_read_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful CDI read reply via datagram
         *
         * @details Handles incoming successful read reply from Configuration Description
         * Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI read operations
         */
    void (*memory_read_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful All Memory Space read reply via datagram
         *
         * @details Handles incoming successful read reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory reads
         */
    void (*memory_read_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Configuration Memory read reply via datagram
         *
         * @details Handles incoming successful read reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory reads
         */
    void (*memory_read_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI Manufacturer read reply via datagram
         *
         * @details Handles incoming successful read reply from ACDI Manufacturer address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer reads
         */
    void (*memory_read_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI User read reply via datagram
         *
         * @details Handles incoming successful read reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User reads
         */
    void (*memory_read_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train FDI read reply via datagram
         *
         * @details Handles incoming successful read reply from Train Function Definition
         * Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI reads
         */
    void (*memory_read_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train Function Config read reply via datagram
         *
         * @details Handles incoming successful read reply from Train Function Configuration
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config reads
         */
    void (*memory_read_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed CDI read reply via datagram
         *
         * @details Handles incoming failed read reply from Configuration Description
         * Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI read operations
         */
    void (*memory_read_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed All Memory Space read reply via datagram
         *
         * @details Handles incoming failed read reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory reads
         */
    void (*memory_read_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Configuration Memory read reply via datagram
         *
         * @details Handles incoming failed read reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory reads
         */
    void (*memory_read_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI Manufacturer read reply via datagram
         *
         * @details Handles incoming failed read reply from ACDI Manufacturer address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer reads
         */
    void (*memory_read_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI User read reply via datagram
         *
         * @details Handles incoming failed read reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User reads
         */
    void (*memory_read_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train FDI read reply via datagram
         *
         * @details Handles incoming failed read reply from Train Function Definition
         * Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI reads
         */
    void (*memory_read_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train Function Config read reply via datagram
         *
         * @details Handles incoming failed read reply from Train Function Configuration
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config reads
         */
    void (*memory_read_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Description Information read via stream
         *
         * @details Handles incoming stream-based requests to read from Configuration Description
         * Information (CDI) address space (0xFF), which contains XML describing the node's
         * configuration structure.
         *
         * @note Optional - can be NULL if CDI reads are not supported via stream transport
         */
    void (*memory_read_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle All Memory Space read via stream
         *
         * @details Handles incoming stream-based requests to read from All Memory Space (0xFD),
         * which provides unified access to all readable memory regions.
         *
         * @note Optional - can be NULL if All Memory Space reads are not supported via stream transport
         */
    void (*memory_read_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Memory read via stream
         *
         * @details Handles incoming stream-based requests to read from Configuration Memory
         * address space (0xFE), which contains node configuration variables.
         *
         * @note Optional - can be NULL if Configuration Memory reads are not supported via stream transport
         */
    void (*memory_read_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI Manufacturer Information read via stream
         *
         * @details Handles incoming stream-based requests to read from ACDI Manufacturer
         * address space (0xFC), which contains read-only manufacturer identification data.
         *
         * @note Optional - can be NULL if ACDI Manufacturer reads are not supported via stream transport
         */
    void (*memory_read_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI User Information read via stream
         *
         * @details Handles incoming stream-based requests to read from ACDI User address space
         * (0xFB), which contains user-editable identification data.
         *
         * @note Optional - can be NULL if ACDI User reads are not supported via stream transport
         */
    void (*memory_read_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Definition Info read via stream
         *
         * @details Handles incoming stream-based requests to read from Train Function
         * Definition Information address space (0xFA), which describes available train
         * control functions.
         *
         * @note Optional - can be NULL if Train FDI reads are not supported via stream transport
         */
    void (*memory_read_stream_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Configuration read via stream
         *
         * @details Handles incoming stream-based requests to read from Train Function
         * Configuration address space (0xF9), which contains train function settings.
         *
         * @note Optional - can be NULL if Train Function Config reads are not supported via stream transport
         */
    void (*memory_read_stream_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful CDI read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from Configuration
         * Description Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI stream reads
         */
    void (*memory_read_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful All Memory Space read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory stream reads
         */
    void (*memory_read_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Configuration Memory read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory stream reads
         */
    void (*memory_read_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI Manufacturer read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from ACDI Manufacturer
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer stream reads
         */
    void (*memory_read_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI User read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User stream reads
         */
    void (*memory_read_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train FDI read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from Train Function
         * Definition Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI stream reads
         */
    void (*memory_read_stream_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train Function Config read reply via stream
         *
         * @details Handles incoming successful stream-based read reply from Train Function
         * Configuration address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config stream reads
         */
    void (*memory_read_stream_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed CDI read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from Configuration Description
         * Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI stream reads
         */
    void (*memory_read_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed All Memory Space read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory stream reads
         */
    void (*memory_read_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Configuration Memory read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory stream reads
         */
    void (*memory_read_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI Manufacturer read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from ACDI Manufacturer
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer stream reads
         */
    void (*memory_read_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI User read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User stream reads
         */
    void (*memory_read_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train FDI read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from Train Function
         * Definition Information address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI stream reads
         */
    void (*memory_read_stream_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train Function Config read reply via stream
         *
         * @details Handles incoming failed stream-based read reply from Train Function
         * Configuration address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config stream reads
         */
    void (*memory_read_stream_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle CDI write via datagram
         *
         * @details Handles incoming datagram write requests to Configuration Description Information
         * address space (0xFF). Typically NULL as CDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as CDI is usually a read-only address space
         */
    void (*memory_write_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle All Memory Space write via datagram
         *
         * @details Handles incoming datagram write requests to All Memory Space (0xFD).
         * Typically NULL as All Memory Space is read-only in most implementations.
         *
         * @note Optional - typically NULL as All Memory Space is usually read-only
         */
    void (*memory_write_space_all)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Memory write via datagram
         *
         * @details Handles incoming datagram write requests to Configuration Memory address space
         * (0xFE), which contains user-configurable node settings and parameters.
         *
         * @note Optional - can be NULL if Configuration Memory writes are not supported via datagram
         */
    void (*memory_write_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI Manufacturer write via datagram
         *
         * @details Handles incoming datagram write requests to ACDI Manufacturer address space (0xFC).
         * Typically NULL as manufacturer data is read-only in most implementations.
         *
         * @note Optional - typically NULL as ACDI Manufacturer is usually a read-only address space
         */
    void (*memory_write_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI User write via datagram
         *
         * @details Handles incoming datagram write requests to ACDI User address space (0xFB),
         * which contains user-editable node identification and description information.
         *
         * @note Optional - can be NULL if ACDI User writes are not supported via datagram
         */
    void (*memory_write_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train FDI write via datagram
         *
         * @details Handles incoming datagram write requests to Train Function Definition
         * Information address space (0xFA). Typically NULL as FDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as Train FDI is usually a read-only address space
         */
    void (*memory_write_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Config write via datagram
         *
         * @details Handles incoming datagram write requests to Train Function Configuration
         * address space (0xF9), which contains train control function settings.
         *
         * @note Optional - can be NULL if Train Function Config writes are not supported via datagram
         */
    void (*memory_write_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Firmware Upgrade write via datagram
         *
         * @details Handles incoming datagram write requests to Firmware Upgrade address space,
         * which is used for firmware update operations.
         *
         * @note Optional - can be NULL if firmware upgrades are not supported via datagram
         */
    void (*memory_write_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful CDI write reply via datagram
         *
         * @details Handles incoming successful write reply from CDI address space.
         * Typically never called as CDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as CDI is usually a read-only address space
         */
    void (*memory_write_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful All Memory Space write reply via datagram
         *
         * @details Handles incoming successful write reply from All Memory Space.
         * Typically never called as All Memory Space is read-only in most implementations.
         *
         * @note Optional - typically NULL as All Memory Space is usually read-only
         */
    void (*memory_write_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Configuration Memory write reply via datagram
         *
         * @details Handles incoming successful write reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory writes
         */
    void (*memory_write_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI Manufacturer write reply via datagram
         *
         * @details Handles incoming successful write reply from ACDI Manufacturer address space.
         * Typically never called as manufacturer data is read-only in most implementations.
         *
         * @note Optional - typically NULL as ACDI Manufacturer is usually a read-only address space
         */
    void (*memory_write_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI User write reply via datagram
         *
         * @details Handles incoming successful write reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User writes
         */
    void (*memory_write_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train FDI write reply via datagram
         *
         * @details Handles incoming successful write reply from Train FDI address space.
         * Typically never called as FDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as Train FDI is usually a read-only address space
         */
    void (*memory_write_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train Function Config write reply via datagram
         *
         * @details Handles incoming successful write reply from Train Function Configuration
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config writes
         */
    void (*memory_write_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed CDI write reply via datagram
         *
         * @details Handles incoming failed write reply from CDI address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI write operations
         */
    void (*memory_write_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed All Memory Space write reply via datagram
         *
         * @details Handles incoming failed write reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory writes
         */
    void (*memory_write_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Configuration Memory write reply via datagram
         *
         * @details Handles incoming failed write reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory writes
         */
    void (*memory_write_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI Manufacturer write reply via datagram
         *
         * @details Handles incoming failed write reply from ACDI Manufacturer address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer writes
         */
    void (*memory_write_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI User write reply via datagram
         *
         * @details Handles incoming failed write reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User writes
         */
    void (*memory_write_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train FDI write reply via datagram
         *
         * @details Handles incoming failed write reply from Train FDI address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI writes
         */
    void (*memory_write_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train Function Config write reply via datagram
         *
         * @details Handles incoming failed write reply from Train Function Configuration
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config writes
         */
    void (*memory_write_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle CDI write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to CDI address space.
         * Typically NULL as CDI is read-only in most implementations. Write-under-mask allows
         * bit-level modification using alternating mask and value bytes.
         *
         * @note Optional - typically NULL as CDI is usually a read-only address space
         */
    void (*memory_write_under_mask_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle All Memory Space write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to All Memory Space.
         * Typically NULL as All Memory Space is read-only in most implementations.
         *
         * @note Optional - typically NULL as All Memory Space is usually read-only
         */
    void (*memory_write_under_mask_space_all)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Memory write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to Configuration Memory.
         * Allows bit-level modification using alternating mask and value bytes for precise control.
         *
         * @note Optional - can be NULL if write-under-mask is not supported for Configuration Memory
         */
    void (*memory_write_under_mask_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI Manufacturer write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to ACDI Manufacturer space.
         * Typically NULL as manufacturer data is read-only in most implementations.
         *
         * @note Optional - typically NULL as ACDI Manufacturer is usually a read-only address space
         */
    void (*memory_write_under_mask_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI User write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to ACDI User address space.
         * Allows bit-level modification of user identification data.
         *
         * @note Optional - can be NULL if write-under-mask is not supported for ACDI User
         */
    void (*memory_write_under_mask_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train FDI write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to Train FDI space.
         * Typically NULL as FDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as Train FDI is usually a read-only address space
         */
    void (*memory_write_under_mask_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Config write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to Train Function
         * Configuration space. Allows bit-level modification of function settings.
         *
         * @note Optional - can be NULL if write-under-mask is not supported for Train Function Config
         */
    void (*memory_write_under_mask_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Firmware Upgrade write under mask via datagram
         *
         * @details Handles incoming datagram write-under-mask requests to Firmware Upgrade
         * address space. Used for firmware update operations with bit-level control.
         *
         * @note Optional - can be NULL if firmware upgrades do not support write-under-mask
         */
    void (*memory_write_under_mask_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle CDI write via stream
         *
         * @details Handles incoming stream-based write requests to CDI address space.
         * Typically NULL as CDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as CDI is usually a read-only address space
         */
    void (*memory_write_stream_space_config_description_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle All Memory Space write via stream
         *
         * @details Handles incoming stream-based write requests to All Memory Space.
         * Typically NULL as All Memory Space is read-only in most implementations.
         *
         * @note Optional - typically NULL as All Memory Space is usually read-only
         */
    void (*memory_write_stream_space_all)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Configuration Memory write via stream
         *
         * @details Handles incoming stream-based write requests to Configuration Memory.
         * Streams enable efficient transfer of large configuration blocks.
         *
         * @note Optional - can be NULL if Configuration Memory writes are not supported via stream
         */
    void (*memory_write_stream_space_configuration_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI Manufacturer write via stream
         *
         * @details Handles incoming stream-based write requests to ACDI Manufacturer space.
         * Typically NULL as manufacturer data is read-only in most implementations.
         *
         * @note Optional - typically NULL as ACDI Manufacturer is usually a read-only address space
         */
    void (*memory_write_stream_space_acdi_manufacturer)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle ACDI User write via stream
         *
         * @details Handles incoming stream-based write requests to ACDI User address space.
         * Streams enable efficient transfer of user identification data.
         *
         * @note Optional - can be NULL if ACDI User writes are not supported via stream
         */
    void (*memory_write_stream_space_acdi_user)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train FDI write via stream
         *
         * @details Handles incoming stream-based write requests to Train FDI space.
         * Typically NULL as FDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as Train FDI is usually a read-only address space
         */
    void (*memory_write_stream_space_train_function_definition_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Train Function Config write via stream
         *
         * @details Handles incoming stream-based write requests to Train Function Configuration.
         * Streams enable efficient transfer of function configuration blocks.
         *
         * @note Optional - can be NULL if Train Function Config writes are not supported via stream
         */
    void (*memory_write_stream_space_train_function_config_memory)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to handle Firmware Upgrade write via stream
         *
         * @details Handles incoming stream-based write requests to Firmware Upgrade address space.
         * Streams are typically preferred for large firmware image transfers.
         *
         * @note Optional - can be NULL if firmware upgrades are not supported via stream
         */
    void (*memory_write_stream_space_firmware_upgrade)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful CDI write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from CDI address space.
         * Typically never called as CDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as CDI is usually a read-only address space
         */
    void (*memory_write_stream_space_config_description_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful All Memory Space write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from All Memory Space.
         * Typically never called as All Memory Space is read-only in most implementations.
         *
         * @note Optional - typically NULL as All Memory Space is usually read-only
         */
    void (*memory_write_stream_space_all_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Configuration Memory write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory stream writes
         */
    void (*memory_write_stream_space_configuration_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI Manufacturer write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from ACDI Manufacturer space.
         * Typically never called as manufacturer data is read-only in most implementations.
         *
         * @note Optional - typically NULL as ACDI Manufacturer is usually a read-only address space
         */
    void (*memory_write_stream_space_acdi_manufacturer_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful ACDI User write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User stream writes
         */
    void (*memory_write_stream_space_acdi_user_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train FDI write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from Train FDI space.
         * Typically never called as FDI is read-only in most implementations.
         *
         * @note Optional - typically NULL as Train FDI is usually a read-only address space
         */
    void (*memory_write_stream_space_train_function_definition_info_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process successful Train Function Config write reply via stream
         *
         * @details Handles incoming successful stream-based write reply from Train Function
         * Configuration when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config stream writes
         */
    void (*memory_write_stream_space_train_function_config_memory_reply_ok)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed CDI write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from CDI address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate CDI stream writes
         */
    void (*memory_write_stream_space_config_description_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed All Memory Space write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from All Memory Space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate All Memory stream writes
         */
    void (*memory_write_stream_space_all_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Configuration Memory write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from Configuration Memory
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory stream writes
         */
    void (*memory_write_stream_space_configuration_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI Manufacturer write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from ACDI Manufacturer space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI Manufacturer stream writes
         */
    void (*memory_write_stream_space_acdi_manufacturer_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed ACDI User write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from ACDI User address space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate ACDI User stream writes
         */
    void (*memory_write_stream_space_acdi_user_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train FDI write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from Train FDI space
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train FDI stream writes
         */
    void (*memory_write_stream_space_train_function_definition_info_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process failed Train Function Config write reply via stream
         *
         * @details Handles incoming failed stream-based write reply from Train Function
         * Configuration when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Train Function Config stream writes
         */
    void (*memory_write_stream_space_train_function_config_memory_reply_fail)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Get Configuration Options command
         *
         * @details Handles incoming Configuration Memory Options command datagram,
         * which queries the node's configuration capabilities and supported features.
         *
         * @note Optional - can be NULL if Configuration Memory Options queries are not supported
         */
    void (*memory_options_cmd)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Configuration Options reply
         *
         * @details Handles incoming reply to Configuration Memory Options query
         * when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate Configuration Memory operations
         */
    void (*memory_options_reply)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Get Address Space Information command
         *
         * @details Handles incoming request for information about a specific address space,
         * including size, flags, and description.
         *
         * @note Optional - can be NULL if Address Space Information queries are not supported
         */
    void (*memory_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Address Space Not Present reply
         *
         * @details Handles incoming reply indicating that the requested address space
         * does not exist on the target node.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*memory_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Address Space Present reply
         *
         * @details Handles incoming reply containing information about a requested
         * address space when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*memory_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Lock/Reserve command
         *
         * @details Handles incoming request to lock or reserve the node's configuration
         * for exclusive access during configuration operations.
         *
         * @note Optional - can be NULL if configuration locking is not supported
         */
    void (*memory_reserve_lock)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Lock/Reserve reply
         *
         * @details Handles incoming reply to lock/reserve request when this node
         * is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*memory_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Get Unique ID command
         *
         * @details Handles incoming request for the node's unique event ID,
         * used in configuration protocols.
         *
         * @note Optional - can be NULL if unique ID queries are not supported
         */
    void (*memory_get_unique_id)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Get Unique ID reply
         *
         * @details Handles incoming reply containing the unique ID when this node
         * is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*memory_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Unfreeze command
         *
         * @details Handles incoming request to resume operations in a specific
         * address space after configuration updates.
         *
         * @note Optional - can be NULL if freeze/unfreeze operations are not supported
         */
    void (*memory_unfreeze)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Freeze command
         *
         * @details Handles incoming request to suspend operations in a specific
         * address space during configuration updates.
         *
         * @note Optional - can be NULL if freeze/unfreeze operations are not supported
         */
    void (*memory_freeze)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Update Complete command
         *
         * @details Handles incoming notification that configuration updates are complete
         * and the node should apply changes or reset as appropriate.
         *
         * @note Optional - can be NULL if Update Complete handling is not needed
         */
    void (*memory_update_complete)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Reset/Reboot command
         *
         * @details Handles incoming request to reset or reboot the node, typically
         * after configuration changes that require a restart.
         *
         * @note Optional - can be NULL if reset/reboot commands are not supported
         */
    void (*memory_reset_reboot)(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Optional callback to process Factory Reset command
         *
         * @details Handles incoming request to reset the node to factory default
         * configuration, erasing all user settings and customizations.
         *
         * @note Optional - can be NULL if factory reset is not supported
         */
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
        * @details Sets up the datagram protocol handler with the provided callback
        * interface. This function must be called once during system initialization
        * before any datagram processing begins.
        *
        * The interface structure is stored internally and all callbacks will be
        * invoked through this interface. Required callbacks (lock/unlock) must be
        * valid; optional callbacks may be NULL.
        *
        * Use cases:
        * - System initialization before network operations
        * - Called once during application startup
        *
        * @param interface Pointer to callback interface structure containing
        * function pointers for all supported datagram operations
        *
        * @warning Interface pointer must remain valid for lifetime of application
        * @warning Required callbacks must not be NULL
        * @attention Call before any datagram processing begins
        *
        * @see interface_protocol_datagram_handler_t - Interface structure definition
        */
    extern void ProtocolDatagramHandler_initialize(const interface_protocol_datagram_handler_t *interface);

        /**
        * @brief Loads a datagram received OK acknowledgment message
        *
        * @details Formats a positive datagram acknowledgment message (MTI 0x0A28)
        * indicating successful reception. The return_code can include optional flags
        * such as reply pending time.
        *
        * Use cases:
        * - Acknowledging successfully received configuration datagram
        * - Indicating delayed response with Reply Pending flag
        * - Normal datagram protocol acknowledgment
        *
        * @param statemachine_info Pointer to state machine context for message generation
        * @param return_code Status code and flags (typically 0 for simple OK, or includes
        * Reply Pending bit and timeout value)
        *
        * @note Sets outgoing_msg_info.valid flag in statemachine_info
        * @note Return code format: bit 7 = Reply Pending, bits 3-0 = timeout (2^N seconds)
        *
        * @warning Requires valid statemachine_info pointer
        * @attention Must be called from datagram receive processing context
        *
        * @see ProtocolDatagramHandler_load_datagram_rejected_message - For negative acknowledgment
        * @see ProtocolDatagramHandler_datagram - Calls this for accepted datagrams
        */
    extern void ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code);

        /**
        * @brief Loads a datagram rejected message
        *
        * @details Formats a negative datagram acknowledgment message (MTI 0x0A48)
        * indicating rejection. The return_code specifies the error condition per
        * OpenLCB error code definitions.
        *
        * Common rejection reasons:
        * - ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN (0x1041)
        * - ERROR_TEMPORARY_BUFFER_UNAVAILABLE (0x2020)
        * - ERROR_PERMANENT_INVALID_ARGUMENTS (0x1080)
        *
        * Use cases:
        * - Rejecting unsupported datagram commands
        * - Indicating temporary resource unavailability
        * - Reporting invalid parameters or data
        *
        * @param statemachine_info Pointer to state machine context for message generation
        * @param return_code OpenLCB error code indicating rejection reason
        *
        * @note Sets outgoing_msg_info.valid flag in statemachine_info
        * @note Error codes with bit 13 set (0x2000) indicate temporary errors allowing retry
        *
        * @warning Requires valid statemachine_info pointer
        * @attention Permanent errors (0x1xxx) indicate operation should not be retried
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
        * @attention Requires valid incoming datagram in statemachine_info
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
