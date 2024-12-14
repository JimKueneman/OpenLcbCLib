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
 * \file configuration_memory.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to write/read configuration memory.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// Add any compiler specific includes

#include "../../openlcb/openlcb_types.h"
#include "../../utilities/mustangpeak_string_helper.h"
#include "string.h"
#include "stdio.h"

char *user_data;

void DriverConfigurationMemory_initialization()
{
    user_data = strnew_initialized(LEN_SNIP_USER_NAME + LEN_SNIP_USER_DESCRIPTION + 1);  // add extra null since these are 2 null terminated strings
}

uint16_olcb_t DriverConfigurationMemory_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{
    printf("configmem read count: %d\n", count);
    printf("configmem read address: %08lX\n", address);

    printf("configmem read address: %02X\n", (*buffer)[0]);


    uint32_olcb_t buffer_index = 0;

    for (int i = address; i < (address + count); i++)
    {
    //    (*buffer)[buffer_index] = user_data[i];

         (*buffer)[buffer_index] = '\0';
        buffer_index++;
    }
    return count;
}

uint16_olcb_t DriverConfigurationMemory_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t *buffer)
{
    printf("configmem write\n");

    if (count == 0)
      return 0;

    uint32_olcb_t buffer_index = 0;

    for (int i = address; i < (address + count); i++)
    {
  //      user_data[i] = (*buffer)[buffer_index];
        buffer_index++;
    }
    // the caller may or may not have added the null to the string
   // if (user_data[address + count - 1] != '\0')
   //   user_data[address + count] = '\0';

    return count;
}