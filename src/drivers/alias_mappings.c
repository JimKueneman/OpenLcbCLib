/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file alias_mappings.c
 *
 *
 * @author Jim Kueneman
 * @date 27 Sept 2025
 */

#include "alias_mappings.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "common/can_types.h"
#include "../openlcb/openlcb_types.h"


static interface_alias_mappings_t *_interface;
static alias_mapping_info_t alias_mapping;

void AliasMappings_initialize(const interface_alias_mappings_t *interface_alias_mappings) {

    _interface = (interface_alias_mappings_t*) interface_alias_mappings;

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        alias_mapping.list[i].alias = 0x00;
        alias_mapping.list[i].node_id = 0x00;

    }

}

alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id) {

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping.list[i].alias == 0x00) {

            alias_mapping.list[i].alias = alias;
            alias_mapping.list[i].node_id = node_id;
            
            return &alias_mapping.list[i];

        }

    }
    
    return NULL;

}

void AliasMappings_unregister(uint16_t alias) {

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping.list[i].alias == 0x00) {

            alias_mapping.list[i].alias = 0x00;
            alias_mapping.list[i].node_id = 0x00;
            
            break;

        }

    }

}

alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias) {

    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping.list[i].alias == alias) {

            return &alias_mapping.list[i];

        }

    }
    
    return NULL;

}

alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id) {


    for (int i = 0; i < USER_DEFINED_ALIAS_MAPPING_BUFFER_DEPTH; i++) {

        if (alias_mapping.list[i].node_id == node_id) {

            return &alias_mapping.list[i];

        }

    }
    
    return NULL;

}