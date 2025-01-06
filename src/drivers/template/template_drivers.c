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
 * \odx_drivers.c
 *
 *
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "template_drivers.h"

parameterless_callback_t _100ms_timer_sink_func = (void *)0;

void TemplateDrivers_setup(parameterless_callback_t _100ms_timer_sink)
{

    _100ms_timer_sink_func = _100ms_timer_sink;
}

void TemplateDrivers_reboot(void)
{
}

uint16_olcb_t TemplateDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    return count;
}

uint16_olcb_t TemplateDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{

    return count;
}

void TemplateDrivers_pause_100ms_timer()
{
}

void TemplateDrivers_resume_100ms_timer()
{
}
