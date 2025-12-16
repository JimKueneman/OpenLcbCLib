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
 * \file openlcb_node.h
 *
 * Implementation of the OpenLcb node structures and buffers with functions to manipulate them
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_NODE__
#define __OPENLCB_OPENLCB_NODE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h" // include processor files - each processor file is guarded.

typedef struct
{

    // callbacks
    void (*on_100ms_timer_tick)(void);

} interface_openlcb_node_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void OpenLcbNode_initialize(const interface_openlcb_node_t *interface);

    extern openlcb_node_t *OpenLcbNode_allocate(uint64_t nodeid, const node_parameters_t *node_parameters);

    extern openlcb_node_t *OpenLcbNode_get_first(uint8_t key);

    extern openlcb_node_t *OpenLcbNode_get_next(uint8_t key);

    extern openlcb_node_t *OpenLcbNode_find_by_alias(uint16_t alias);

    extern openlcb_node_t *OpenLcbNode_find_by_node_id(uint64_t nodeid);

    extern void OpenLcbNode_reset_state(void);

    extern void OpenLcbNode_100ms_timer_tick(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_NODE__ */
