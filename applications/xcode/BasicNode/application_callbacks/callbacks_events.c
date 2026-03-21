
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
 * \file callbacks_events.c
 *
 * Event transport callback implementations for the BasicNode demo.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_events.h"

#include <stdio.h>

void CallbacksEvents_on_consumed_event_identified(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload) {

    printf("Received a produced event identified that we are register as a consumer of: EventID = 0x%08llX\n", *event_id);

    if (index == 0xFFFF) {

        printf("Within registered Consumer Range\n");

    } else {

        printf("at index: %d in Node.Consumers.List[]\n", index);

    }

}

void CallbacksEvents_on_consumed_event_pcer(openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload) {

    printf("Received a PECR event that we are regstered as a consumer of: EventID = 0x%08llX\n", *event_id);

    if (index == 0xFFFF) {

        printf("Within registered Consumer Range\n");

    } else {

        printf("at index: %d in Node.Consumers.List[]\n", index);

    }

}

void CallbacksEvents_on_event_learn(openlcb_node_t *openlcb_node, event_id_t *event_id) {

    printf("Received Event Learn: EventID = 0x%08llX\n", *event_id);

}
