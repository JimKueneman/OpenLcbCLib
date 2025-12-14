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
 * \file alias_mappings.h
 *
 *
 * @author Jim Kueneman
 * @date 27 Sept 2025
 */
// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DRIVERS_ALIAS_MAPPINGS__
#define __DRIVERS_ALIAS_MAPPINGS__

#include <stdbool.h>
#include <stdint.h>

#include "common/can_types.h"
#include "../openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void AliasMappings_initialize(void);

    extern alias_mapping_info_t *AliasMappings_get_alias_mapping_info(void);

    extern void AliasMappings_set_has_duplicate_alias_flag(void);

    extern alias_mapping_t *AliasMappings_register(uint16_t alias, node_id_t node_id);

    extern void AliasMappings_unregister(uint16_t alias);

    extern alias_mapping_t *AliasMappings_find_mapping_by_alias(uint16_t alias);

    extern alias_mapping_t *AliasMappings_find_mapping_by_node_id(node_id_t node_id);

    extern void AliasMappings_flush(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_ALIAS_MAPPINGS__ */
