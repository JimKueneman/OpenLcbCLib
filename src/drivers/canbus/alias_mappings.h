/*
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
 * 16 Dec 2025
 * Copyright (c) 2025, Jim Kueneman
 */

/**
 *
 * @brief Implements a buffer that stores Alias/NodeID pairs.
 *
 * This does not track external nodes it only tracks internal nodes.  It's main purpose is to allow the
 * CAN Rx Handler to search through allocated Alias and NodeIDs when processing incoming messages and
 * not need to access the raw Node data structures in \ref openlcb_node.h. This is because the Rx Handler
 * is typically running in an interrupt or thread so this keeps a secondary list that have clear insert/remove
 * points that can be protected through resource locking (pausing interrupts, threads, etc).
 *
 * @note Applications typically have no reason to access this structure as it can directly use the
 * \ref openlcb_node.h to look at this information in the main loop safely.
 *
 * @warning The CAN Receive Statemachine and 100ms timer access these buffers and typically
 * run within interrupts and/or threads. Care must be taken to Pause and Resume the
 * interrupts or threads if the main loop needs to access the buffers for any reason.
 *
 * @file alias_mappings.h
 *
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_CANBUS_ALIAS_MAPPINGS__
#define __DRIVERS_CANBUS_ALIAS_MAPPINGS__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"
#include "../../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Initializes the Alias Mapping buffers
     *
     * @param none
     *
     * @return none
     *
     * @attention This must always be called during application initialization
     */
    extern void AliasMappings_initialize(void);

    /**
     * @brief Allows access to the Alias Mapping Buffer itself
     *
     * @param none
     *
     * @return Pointer to the alias message buffer or NULL if it fails
     */
    extern alias_mapping_info_t *AliasMappings_get_alias_mapping_info(void);

    /**
     * @brief Sets a flag that tells the main loop that a received message has
     * been found to be using an Alias we have reserved.
     *
     * @param none
     *
     * @return none
     */
    extern void AliasMappings_set_has_duplicate_alias_flag(void);

    /**
     * @brief Registers a new Alias/NodeID pair.  If the NodeID exists then the old Alias is
     * replaced by the passed one
     *
     * @param uint16_t alias - The Alias to store
     * @param node_id_t node_id - The NodeID to store
     *
     * @return Pointer to the newly registered AliasMapping or NULL if fails
     */
    extern alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);

    /**
     * @brief Deregisters an existing Alias/NodeID pair.  If the pair does not exist it does nothing
     *
     * @param uint16_t alias - The Alias to unregister
     *
     * @return none
     */
    extern void AliasMappings_unregister(uint16_t alias);

    /**
     * @brief Finds a Alias/NodeID pair that matches the Alias passed
     *
     * @param uint16_t alias - The Alias to search for
     *
     * @return Pointer to the Alias Mapping pair if found, else NULL
     */
    extern alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias);

    /**
     * @brief Finds a Alias/NodeID pair that matches the NodeID passed
     *
     * @param uint16_t alias - The NodeID to search for
     *
     * @return Pointer to the Alias Mapping pair if found, else NULL
     *
     */
    extern alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id);

    /**
     * @brief Released all stored Alias Mapping pairs
     *
     * @param none
     *
     * @return none
     *
     */
    extern void AliasMappings_flush(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_ALIAS_MAPPINGS__ */
