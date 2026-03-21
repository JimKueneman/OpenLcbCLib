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
 * \file callbacks_config_mem.h
 *
 * Configuration memory callback declarations for the BasicNode application.
 *
 * @author Jim Kueneman
 * @date 31 Dec 2025
 */

#ifndef __CALLBACKS_CONFIG_MEM__
#define __CALLBACKS_CONFIG_MEM__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "../openlcb_c_lib/openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern void CallbacksConfigMem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern void CallbacksConfigMem_firmware_write(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result);

    extern void CallbacksConfigMem_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

    extern void CallbacksConfigMem_unfreeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CALLBACKS_CONFIG_MEM__ */
