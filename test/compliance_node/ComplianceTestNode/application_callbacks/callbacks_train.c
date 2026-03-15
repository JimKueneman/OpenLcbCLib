
/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * \file callbacks_train.c
 *
 * Train protocol callback implementations for the ComplianceTestNode.
 * Accepts all requests and logs state changes for compliance testing.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_train.h"

#include <stdio.h>
#include <stdbool.h>

#include "../openlcb_c_lib/openlcb/openlcb_defines.h"
#include "../openlcb_c_lib/openlcb/openlcb_application_train.h"

static openlcb_node_t **_virtual_pool = NULL;
static int _virtual_pool_count = 0;


void CallbacksTrain_on_speed_changed(openlcb_node_t *openlcb_node, uint16_t speed_float16) {

    printf("Train speed changed: 0x%04X\n", speed_float16);

}

void CallbacksTrain_on_function_changed(openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value) {

    printf("Train function %u = %u\n", fn_address, fn_value);

}

void CallbacksTrain_on_emergency_entered(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type) {

    printf("Train emergency entered: type=%d\n", emergency_type);

}

void CallbacksTrain_on_emergency_exited(openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type) {

    printf("Train emergency exited: type=%d\n", emergency_type);

}

void CallbacksTrain_on_controller_assigned(openlcb_node_t *openlcb_node, node_id_t controller_node_id) {

    printf("Train controller assigned: 0x%06llX\n", controller_node_id);

}

void CallbacksTrain_on_controller_released(openlcb_node_t *openlcb_node) {

    printf("Train controller released\n");

}

void CallbacksTrain_on_heartbeat_timeout(openlcb_node_t *openlcb_node) {

    printf("Train heartbeat timeout (keeping alive for testing)\n");

}

bool CallbacksTrain_on_controller_assign_request(openlcb_node_t *openlcb_node, node_id_t current_controller, node_id_t requesting_controller) {

    printf("Train controller assign request: current=0x%06llX requesting=0x%06llX -> rejecting\n", current_controller, requesting_controller);
    return false;  // Reject takeover from different controller (per TrainControlS section 6.1)

}

bool CallbacksTrain_on_controller_changed_request(openlcb_node_t *openlcb_node, node_id_t new_controller) {

    printf("Train controller changed request: new=0x%06llX -> accepting\n", new_controller);
    return true;  // Accept all takeover requests for compliance testing

}

void CallbacksTrain_set_virtual_pool(openlcb_node_t **pool, int count) {

    _virtual_pool = pool;
    _virtual_pool_count = count;

}

openlcb_node_t* CallbacksTrain_on_search_no_match(uint16_t search_address, uint8_t flags) {

    printf("Train search no match: address=%u flags=0x%02X\n", search_address, flags);

    if (!_virtual_pool) {

        printf("  No virtual pool available\n");
        return NULL;

    }

    // Find an unassigned virtual train node (dcc_address == 0)
    for (int i = 0; i < _virtual_pool_count; i++) {

        openlcb_node_t *node = _virtual_pool[i];

        if (node && node->train_state && node->train_state->dcc_address == 0) {

            bool is_long = (flags & TRAIN_SEARCH_FLAG_LONG_ADDR) != 0;
            OpenLcbApplicationTrain_set_dcc_address(node, search_address, is_long);
            OpenLcbApplicationTrain_set_speed_steps(node, 3);  // 128 speed steps

            printf("  Allocated virtual train: node_id=0x%012llX address=%u\n",
                   node->id, search_address);
            return node;

        }

    }

    printf("  No free virtual train nodes\n");
    return NULL;

}
