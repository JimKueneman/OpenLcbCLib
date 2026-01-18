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
     * @file protocol_config_mem_operations_handler.h
     * @brief Configuration memory operations protocol handler
     * @author Jim Kueneman
     * @date 17 Jan 2026
     */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__
#define    __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

    /**
     * @brief Interface structure for configuration memory operations protocol handler
     *
     * @details This structure defines the callback interface for handling OpenLCB
     * Configuration Memory Operations protocol messages. It contains function pointers
     * for datagram acknowledgment and various configuration memory operation commands
     * as defined in the OpenLCB Memory Configuration Protocol specification.
     *
     * The interface allows the application layer to customize behavior for different
     * configuration operations while the protocol handler manages the message formatting
     * and state machine logic.
     *
     * @note Required callbacks must be set before calling ProtocolConfigMemOperationsHandler_initialize
     * @note Optional callbacks can be NULL if the corresponding functionality is not needed
     *
     * @see ProtocolConfigMemOperationsHandler_initialize
     * @see config_mem_operations_request_info_t
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
         * message indicating the datagram was rejected. The error_code specifies the
         * reason for rejection per OpenLCB error code definitions.
         *
         * @note This is a REQUIRED callback - must not be NULL
         */
    void (*load_datagram_received_rejected_message)(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code);

        /**
         * @brief Optional callback to process a Get Configuration Options command request
         *
         * @details Handles incoming requests for configuration options information,
         * which describes the capabilities and features supported by this node's
         * configuration memory implementation.
         *
         * @note Optional - can be NULL if this command is not supported
         */
    void (*operations_request_options_cmd)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Get Configuration Options reply message
         *
         * @details Handles incoming replies to configuration options queries,
         * typically used when this node is acting as a configuration tool.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*operations_request_options_cmd_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Get Address Space Information command request
         *
         * @details Handles incoming requests for information about a specific address
         * space, including size, flags, and description.
         *
         * @note Optional - can be NULL if address space queries are not supported
         */
    void (*operations_request_get_address_space_info)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process an Address Space Information Present reply
         *
         * @details Handles incoming replies indicating the requested address space
         * exists and contains information about its characteristics.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*operations_request_get_address_space_info_reply_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process an Address Space Information Not Present reply
         *
         * @details Handles incoming replies indicating the requested address space
         * does not exist on the target node.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*operations_request_get_address_space_info_reply_not_present)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Lock/Reserve command request
         *
         * @details Handles incoming requests to lock or reserve the node's configuration
         * for exclusive access during configuration operations.
         *
         * @note Optional - can be NULL if locking is not supported
         */
    void (*operations_request_reserve_lock)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Lock/Reserve reply message
         *
         * @details Handles incoming replies to lock/reserve requests, indicating
         * success or failure and the current lock holder.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*operations_request_reserve_lock_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Get Unique ID command request
         *
         * @details Handles incoming requests for the node's unique event ID,
         * used in the configuration protocol.
         *
         * @note Optional - can be NULL if unique ID queries are not supported
         */
    void (*operations_request_get_unique_id)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Get Unique ID reply message
         *
         * @details Handles incoming replies containing a node's unique event ID.
         *
         * @note Optional - can be NULL if this node does not initiate configuration operations
         */
    void (*operations_request_get_unique_id_reply)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Freeze command request
         *
         * @details Handles incoming requests to freeze (suspend) operations in a
         * specific address space during configuration updates.
         *
         * @note Optional - can be NULL if freeze operations are not supported
         */
    void (*operations_request_freeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process an Unfreeze command request
         *
         * @details Handles incoming requests to unfreeze (resume) operations in a
         * specific address space after configuration updates are complete.
         *
         * @note Optional - can be NULL if freeze operations are not supported
         */
    void (*operations_request_unfreeze)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process an Update Complete command request
         *
         * @details Handles incoming notifications that configuration updates are
         * complete and the node should apply changes or reset as appropriate.
         *
         * @note Optional - can be NULL if update complete notifications are not supported
         */
    void (*operations_request_update_complete)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Reset/Reboot command request
         *
         * @details Handles incoming requests to reset or reboot the node,
         * typically after configuration changes.
         *
         * @note Optional - can be NULL if reset operations are not supported
         * @warning Implementation must handle safe shutdown and restart
         */
    void (*operations_request_reset_reboot)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Optional callback to process a Factory Reset command request
         *
         * @details Handles incoming requests to reset the node to factory default
         * configuration, erasing all user settings.
         *
         * @note Optional - can be NULL if factory reset is not supported
         * @warning Implementation must handle safe restoration of factory defaults
         */
    void (*operations_request_factory_reset)(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


} interface_protocol_config_mem_operations_handler_t;

#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Initializes the configuration memory operations protocol handler
         *
         * @details Sets up the protocol handler with the required callback interface
         * for processing configuration memory operations commands. This must be called
         * once during system initialization before any configuration operations can
         * be processed.
         *
         * The interface structure provides callbacks for datagram acknowledgment
         * (required) and specific operation handlers (optional based on supported features).
         *
         * Use cases:
         * - Called once during application startup
         * - Must be called before processing any configuration datagrams
         *
         * @param interface_protocol_config_mem_operations_handler Pointer to interface structure with callback functions
         *
         * @warning interface_protocol_config_mem_operations_handler must not be NULL
         * @warning Required callbacks (load_datagram_received_ok_message, load_datagram_received_rejected_message) must be set
         * @attention Call during initialization before enabling datagram reception
         *
         * @see interface_protocol_config_mem_operations_handler_t
         */
    extern void ProtocolConfigMemOperationsHandler_initialize(const interface_protocol_config_mem_operations_handler_t *interface_protocol_config_mem_operations_handler);

        /**
         * @brief Processes an incoming Get Configuration Options command datagram
         *
         * @details Handles a request for this node's configuration capabilities and
         * features. The handler validates the request, sends a datagram acknowledgment,
         * and invokes the registered callback to generate the response containing
         * supported options flags.
         *
         * Use cases:
         * - Responding to configuration tools querying node capabilities
         * - Advertising supported address spaces and features
         *
         * @param statemachine_info Pointer to state machine context containing incoming message and node information
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid Get Configuration Options command
         *
         * @see ProtocolConfigMemOperationsHandler_options_reply
         * @see interface_protocol_config_mem_operations_handler_t::operations_request_options_cmd
         */
    extern void ProtocolConfigMemOperationsHandler_options_cmd(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Get Configuration Options reply datagram
         *
         * @details Handles a reply containing configuration capabilities from another
         * node, typically when this node is acting as a configuration tool. The handler
         * parses the options flags and invokes the registered callback for processing.
         *
         * Use cases:
         * - Processing responses when acting as a configuration tool
         * - Discovering capabilities of nodes being configured
         *
         * @param statemachine_info Pointer to state machine context containing incoming message and node information
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid options reply message
         *
         * @see ProtocolConfigMemOperationsHandler_options_cmd
         * @see interface_protocol_config_mem_operations_handler_t::operations_request_options_cmd_reply
         */
    extern void ProtocolConfigMemOperationsHandler_options_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Get Address Space Information command datagram
         *
         * @details Handles a request for information about a specific address space
         * identified in the command. The handler decodes the requested space, validates
         * it exists, sends acknowledgment, and generates a response with space size,
         * flags, and description.
         *
         * Use cases:
         * - Responding to queries about configuration memory layout
         * - Advertising address space characteristics to configuration tools
         *
         * @param statemachine_info Pointer to state machine context containing incoming message and node information
         *
         * @warning statemachine_info must not be NULL
         * @warning statemachine_info->incoming_msg_info.msg_ptr must contain valid Get Address Space Info command
         *
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Address Space Information Not Present reply
         *
         * @details Handles a reply indicating the requested address space does not
         * exist on the target node. Used when acting as a configuration tool to
         * determine which address spaces are available.
         *
         * Use cases:
         * - Discovering available address spaces on target nodes
         * - Handling queries for unsupported spaces
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Address Space Information Present reply
         *
         * @details Handles a reply containing information about an existing address
         * space, including size, flags (read-only, low address valid), and description.
         * Used when acting as a configuration tool to understand target node's memory layout.
         *
         * Use cases:
         * - Discovering address space characteristics on target nodes
         * - Determining read/write permissions and size limits
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info_reply_not_present
         */
    extern void ProtocolConfigMemOperationsHandler_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Lock/Reserve command datagram
         *
         * @details Handles a request to lock or reserve the node's configuration for
         * exclusive access. The command includes a Node ID requesting the lock. If the
         * node is unlocked or the requester already holds the lock, the lock is granted.
         * A Node ID of 0 releases the lock.
         *
         * Use cases:
         * - Granting exclusive configuration access to a configuration tool
         * - Preventing concurrent configuration modifications
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @attention Lock state persists until explicitly released or node resets
         *
         * @see ProtocolConfigMemOperationsHandler_reserve_lock_reply
         */
    extern void ProtocolConfigMemOperationsHandler_reserve_lock(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Lock/Reserve reply datagram
         *
         * @details Handles a reply to a lock/reserve request, indicating the current
         * lock holder's Node ID. Used when acting as a configuration tool to determine
         * if exclusive access was granted.
         *
         * Use cases:
         * - Verifying lock acquisition before configuration operations
         * - Discovering current lock holder
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_reserve_lock
         */
    extern void ProtocolConfigMemOperationsHandler_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Get Unique ID command datagram
         *
         * @details Handles a request for the node's unique event ID used in the
         * configuration protocol. The handler sends acknowledgment and generates
         * a reply containing the node's unique ID.
         *
         * Use cases:
         * - Providing unique identification for configuration protocols
         * - Supporting configuration tool unique ID queries
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_get_unique_id_reply
         */
    extern void ProtocolConfigMemOperationsHandler_get_unique_id(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Get Unique ID reply datagram
         *
         * @details Handles a reply containing a node's unique event ID. Used when
         * acting as a configuration tool to retrieve unique identifiers.
         *
         * Use cases:
         * - Storing unique IDs of configured nodes
         * - Verifying node identity during configuration
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_get_unique_id
         */
    extern void ProtocolConfigMemOperationsHandler_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Unfreeze command datagram
         *
         * @details Handles a request to resume normal operations in a specific address
         * space after configuration updates are complete. The command specifies which
         * address space to unfreeze.
         *
         * Use cases:
         * - Resuming normal operations after configuration changes
         * - Re-enabling address space after freeze
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @attention Unfreeze must correspond to a previous Freeze command
         *
         * @see ProtocolConfigMemOperationsHandler_freeze
         */
    extern void ProtocolConfigMemOperationsHandler_unfreeze(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Freeze command datagram
         *
         * @details Handles a request to suspend operations in a specific address space
         * during configuration updates. The command specifies which address space to
         * freeze, allowing safe modification without concurrent access.
         *
         * Use cases:
         * - Suspending operations before configuration changes
         * - Ensuring consistent state during updates
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @attention Must be followed by Unfreeze to resume operations
         *
         * @see ProtocolConfigMemOperationsHandler_unfreeze
         */
    extern void ProtocolConfigMemOperationsHandler_freeze(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Update Complete command datagram
         *
         * @details Handles a notification that configuration updates are complete
         * and the node should apply changes or reset as appropriate. This signals
         * the end of a configuration session.
         *
         * Use cases:
         * - Applying accumulated configuration changes
         * - Triggering node reset after configuration
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @attention Implementation may trigger node reset or configuration reload
         *
         * @see ProtocolConfigMemOperationsHandler_reset_reboot
         */
    extern void ProtocolConfigMemOperationsHandler_update_complete(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Reset/Reboot command datagram
         *
         * @details Handles a request to reset or reboot the node, typically after
         * configuration changes. The implementation should perform a safe shutdown
         * and restart sequence.
         *
         * Use cases:
         * - Applying configuration that requires restart
         * - Recovering from configuration errors
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning Implementation must ensure safe shutdown before reset
         * @attention May cause node to become temporarily unavailable
         *
         * @see ProtocolConfigMemOperationsHandler_factory_reset
         */
    extern void ProtocolConfigMemOperationsHandler_reset_reboot(openlcb_statemachine_info_t *statemachine_info);

        /**
         * @brief Processes an incoming Factory Reset command datagram
         *
         * @details Handles a request to reset the node to factory default configuration,
         * erasing all user settings and customizations. This is a destructive operation
         * that should be implemented with appropriate safeguards.
         *
         * Use cases:
         * - Restoring node to factory defaults
         * - Recovering from corrupt configuration
         *
         * @param statemachine_info Pointer to state machine context containing incoming message
         *
         * @warning statemachine_info must not be NULL
         * @warning Implementation must safely erase user configuration
         * @warning This is a destructive operation - all user settings will be lost
         * @attention Consider requiring confirmation before executing
         *
         * @see ProtocolConfigMemOperationsHandler_reset_reboot
         */
    extern void ProtocolConfigMemOperationsHandler_factory_reset(openlcb_statemachine_info_t *statemachine_info);



        /**
         * @brief Generates an outgoing Get Configuration Options command datagram
         *
         * @details Creates and formats a Get Configuration Options command message
         * to query another node's configuration capabilities. Used when this node
         * is acting as a configuration tool.
         *
         * Use cases:
         * - Initiating configuration capability queries
         * - Discovering supported features of target nodes
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_operations_request_info Pointer to request information structure
         *
         * @warning Both parameters must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_options_cmd
         */
    extern void ProtocolConfigMemOperationsHandler_request_options_cmd(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Generates an outgoing Get Address Space Information command datagram
         *
         * @details Creates and formats a Get Address Space Information command message
         * to query information about a specific address space on another node. Used
         * when acting as a configuration tool.
         *
         * Use cases:
         * - Querying address space characteristics
         * - Discovering memory layout of target nodes
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_operations_request_info Pointer to request information including target address space
         *
         * @warning Both parameters must not be NULL
         * @warning config_mem_operations_request_info must specify valid address space
         *
         * @see ProtocolConfigMemOperationsHandler_get_address_space_info
         */
    extern void ProtocolConfigMemOperationsHandler_request_get_address_space_info(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

        /**
         * @brief Generates an outgoing Lock/Reserve command datagram
         *
         * @details Creates and formats a Lock/Reserve command message to request
         * exclusive access to another node's configuration. Used when acting as
         * a configuration tool.
         *
         * Use cases:
         * - Requesting exclusive configuration access
         * - Releasing configuration lock (with Node ID 0)
         *
         * @param statemachine_info Pointer to state machine context for message generation
         * @param config_mem_operations_request_info Pointer to request information
         *
         * @warning Both parameters must not be NULL
         *
         * @see ProtocolConfigMemOperationsHandler_reserve_lock
         */
    extern void ProtocolConfigMemOperationsHandler_request_reserve_lock(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);


#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_PROTOCOL_CONFIG_MEM_OPERATIONS_HANDLER__ */

