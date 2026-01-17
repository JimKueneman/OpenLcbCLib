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
 * @file openlcb_gridconnect.h
 * @brief GridConnect protocol implementation for CAN message conversion
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.

#ifndef __OPENLCB_OPENLCB_GRIDCONNECT__
#define __OPENLCB_OPENLCB_GRIDCONNECT__

#include <stdbool.h>
#include <stdint.h>

#include "../drivers/canbus/can_types.h"

#define GRIDCONNECT_STATE_SYNC_START 0
#define GRIDCONNECT_STATE_SYNC_FIND_HEADER 2
#define GRIDCONNECT_STATE_SYNC_FIND_DATA 4

// :X19170640N0501010107015555;#0  Example.....
// ^         ^                  ^
// 0         10                28
#define MAX_GRID_CONNECT_LEN 29

typedef uint8_t gridconnect_buffer_t[MAX_GRID_CONNECT_LEN];

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /**
     * @brief Processes incoming GridConnect byte stream and extracts complete message when done
     * @param next_byte Next byte from the GridConnect stream
     * @param buffer Pointer to buffer to store the extracted GridConnect message
     * @return True when a complete GridConnect message has been extracted, false otherwise
     */
    extern bool OpenLcbGridConnect_copy_out_gridconnect_when_done(uint8_t next_byte, gridconnect_buffer_t *buffer);

    /**
     * @brief Converts a GridConnect message to a CAN message structure
     * @param gridconnect Pointer to GridConnect message buffer
     * @param can_msg Pointer to CAN message structure to fill
     * @return None
     */
    extern void OpenLcbGridConnect_to_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg);

    /**
     * @brief Converts a CAN message structure to GridConnect format
     * @param gridconnect Pointer to buffer to store the GridConnect message
     * @param can_msg Pointer to source CAN message structure
     * @return None
     */
    extern void OpenLcbGridConnect_from_can_msg(gridconnect_buffer_t *gridconnect, can_msg_t *can_msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_STORE__ */