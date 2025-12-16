
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
 * \file dependency_injectors.h
 *
 *
 * @author Jim Kueneman
 * @date 16 Nov 2025
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __DEPENDENCY_INJECTORS__
#define __DEPENDENCY_INJECTORS__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ti_msp_dl_config.h"

#include "src/openlcb/openlcb_types.h"
#include "src/drivers/canbus/can_types.h"
#include "src/openlcb/openlcb_gridconnect.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void DependencyInjectors_initialize(void);

    extern void DependencyInjectors_on_100ms_timer_callback(void);

    extern void DependencyInjectors_on_can_rx_callback(can_msg_t *can_msg);

    extern void DependencyInjectors_on_can_tx_callback(can_msg_t *can_msg);

    extern void DependencyInjectors_alias_change_callback(uint16_t new_alias, node_id_t node_id);

    extern void DependencyInjectors_operations_request_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DEPENDENCY_INJECTORS__ */