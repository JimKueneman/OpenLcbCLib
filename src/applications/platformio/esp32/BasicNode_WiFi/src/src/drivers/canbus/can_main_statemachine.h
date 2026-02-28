/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
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
 * @file can_main_statemachine.h
 * @brief Main CAN layer state machine — orchestrates alias management, login, and message dispatch.
 *
 * @details Coordinates duplicate alias detection, outgoing message transmission, login
 * sequencing, and round-robin node enumeration across all virtual nodes.
 * Non-blocking: call CanMainStateMachine_run() as fast as possible in the main loop.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__
#define __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Dependency-injection interface for the CAN main state machine.
     *
     * @details All pointers are REQUIRED (must not be NULL).
     * Each call to CanMainStateMachine_run() processes one operation in priority order:
     * duplicate aliases → outgoing CAN message → login message → first node → next node.
     *
     * @see CanMainStatemachine_initialize
     */
    typedef struct {

        /** @brief REQUIRED. Disable interrupts / acquire mutex. Typical: application lock function. */
        void (*lock_shared_resources)(void);

        /** @brief REQUIRED. Re-enable interrupts / release mutex. Typical: application unlock function. */
        void (*unlock_shared_resources)(void);

        /** @brief REQUIRED. Transmit a pre-built CAN frame. Typical: CanTxStatemachine_send_can_message. */
        bool (*send_can_message)(can_msg_t *msg);

        /** @brief REQUIRED. Return the first allocated node (start of enumeration). Typical: OpenLcbNode_get_first. */
        openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /** @brief REQUIRED. Return the next node in the enumeration sequence. Typical: OpenLcbNode_get_next. */
        openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /** @brief REQUIRED. Find a node by its 12-bit CAN alias. Typical: OpenLcbNode_find_by_alias. */
        openlcb_node_t *(*openlcb_node_find_by_alias)(uint16_t alias);

        /** @brief REQUIRED. Advance the login state machine one step. Typical: CanLoginStateMachine_run. */
        void (*login_statemachine_run)(can_statemachine_info_t *can_statemachine_info);

        /** @brief REQUIRED. Return pointer to the alias mapping table. Typical: AliasMappings_get_alias_mapping_info. */
        alias_mapping_info_t *(*alias_mapping_get_alias_mapping_info)(void);

        /** @brief REQUIRED. Remove an alias from the mapping table. Typical: AliasMappings_unregister. */
        void (*alias_mapping_unregister)(uint16_t alias);

        /** @brief REQUIRED. Scan and resolve all duplicate aliases. Typical: CanMainStatemachine_handle_duplicate_aliases. */
        bool (*handle_duplicate_aliases)(void);

        /** @brief REQUIRED. Pop and transmit one outgoing CAN message. Typical: CanMainStatemachine_handle_outgoing_can_message. */
        bool (*handle_outgoing_can_message)(void);

        /** @brief REQUIRED. Transmit a pending login frame (CID/RID/AMD). Typical: CanMainStatemachine_handle_login_outgoing_can_message. */
        bool (*handle_login_outgoing_can_message)(void);

        /** @brief REQUIRED. Start enumeration and process the first node. Typical: CanMainStatemachine_handle_try_enumerate_first_node. */
        bool (*handle_try_enumerate_first_node)(void);

        /** @brief REQUIRED. Continue enumeration to the next node. Typical: CanMainStatemachine_handle_try_enumerate_next_node. */
        bool (*handle_try_enumerate_next_node)(void);

    } interface_can_main_statemachine_t;


    /**
     * @brief Registers the dependency-injection interface and prepares internal buffers.
     *
     * @details Must be called once at startup, after CanBufferStore_initialize(),
     * CanBufferFifo_initialize(), and CanLoginStateMachine_initialize(), and before
     * CAN reception begins.
     *
     * @param interface_can_main_statemachine Pointer to a fully populated
     *        @ref interface_can_main_statemachine_t. Must remain valid for the
     *        lifetime of the application. All pointers must be non-NULL.
     *
     * @warning NOT thread-safe - call during single-threaded initialization only.
     *
     * @see CanMainStateMachine_run
     */
    extern void CanMainStatemachine_initialize(const interface_can_main_statemachine_t *interface_can_main_statemachine);


    /**
     * @brief Executes one iteration of the main CAN state machine.
     *
     * @details Non-blocking. Processes one operation per call and returns immediately.
     * Call as fast as possible in the main loop.
     *
     * @warning Must be called frequently to keep CAN traffic flowing.
     * @warning NOT thread-safe - call from a single context only.
     *
     * @see CanMainStatemachine_initialize - must be called first
     */
    extern void CanMainStateMachine_run(void);


    /**
     * @brief Returns a pointer to the internal state machine context.
     *
     * @details Intended for unit testing and debugging only.
     *
     * @return Pointer to the internal @ref can_statemachine_info_t (never NULL after initialize).
     *
     * @warning Do not modify the returned structure.
     * @warning NOT thread-safe.
     */
    extern can_statemachine_info_t *CanMainStateMachine_get_can_statemachine_info(void);


    /**
     * @brief Scans the alias table, unregisters duplicates, and resets affected nodes.
     *
     * @details Exposed for unit testing. Normally called via the interface pointer.
     *
     * @return true if any duplicate aliases were found and resolved, false if none.
     *
     * @warning Locks shared resources during operation.
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_handle_duplicate_aliases(void);


    /**
     * @brief Attempts to transmit the pending login frame (CID, RID, or AMD).
     *
     * @details Exposed for unit testing. Normally called via the interface pointer.
     *
     * @return true if a login frame was pending (sent or not), false if nothing pending.
     *
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_handle_login_outgoing_can_message(void);


    /**
     * @brief Pops one message from the outgoing CAN FIFO and attempts transmission.
     *
     * @details Exposed for unit testing. Normally called via the interface pointer.
     * Frees the buffer only on successful transmission; retries on the next call otherwise.
     *
     * @return true if a message was in the FIFO (sent or not), false if FIFO empty.
     *
     * @warning Locks shared resources during FIFO access.
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_handle_outgoing_can_message(void);


    /**
     * @brief Gets the first node and processes it through its appropriate state machine.
     *
     * @details Exposed for unit testing. Normally called via the interface pointer.
     *
     * @return true if the first node was found (or no nodes exist), false if already enumerating.
     *
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_handle_try_enumerate_first_node(void);


    /**
     * @brief Continues enumeration and processes the next node.
     *
     * @details Exposed for unit testing. Normally called via the interface pointer.
     *
     * @return true if no more nodes remain (enumeration complete), false if more nodes exist.
     *
     * @warning NOT thread-safe.
     */
    extern bool CanMainStatemachine_handle_try_enumerate_next_node(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_MAIN_STATEMACHINE__ */
