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
 * \file protocol_modes.c
 *
 * Protocol mode registry implementation. To add a new protocol:
 *   1. Define node_parameters_t in openlcb_user_config.c
 *   2. Write callbacks_<protocol>.c/h
 *   3. Add one entry to protocol_modes[] below
 *   4. Add test section to run_olcbchecker.sh and run_tests.py
 *
 * @author Jim Kueneman
 * @date 8 Mar 2026
 */

#include "protocol_modes.h"

#include <stdio.h>
#include <string.h>

#include "openlcb_user_config.h"
#include "application_callbacks/callbacks_broadcast_time.h"
#include "application_callbacks/callbacks_train.h"
#include "openlcb_c_lib/openlcb/openlcb_defines.h"
#include "openlcb_c_lib/openlcb/openlcb_application.h"
#include "openlcb_c_lib/openlcb/openlcb_application_broadcast_time.h"
#include "openlcb_c_lib/openlcb/openlcb_application_train.h"
#include "openlcb_c_lib/openlcb/openlcb_config.h"

#define VIRTUAL_TRAIN_COUNT 3

static openlcb_node_t *virtual_train_pool[VIRTUAL_TRAIN_COUNT];

// =============================================================================
// Setup functions — called after OpenLcb_create_node()
// =============================================================================

static void setup_basic(openlcb_node_t *node) {
    OpenLcbApplication_register_consumer_eventid(node, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);
    OpenLcbApplication_register_consumer_eventid(node, EVENT_ID_CLEAR_EMERGENCY_STOP, EVENT_STATUS_UNKNOWN);
    OpenLcbApplication_register_producer_eventid(node, EVENT_ID_DUPLICATE_NODE_DETECTED, EVENT_STATUS_UNKNOWN);
    OpenLcbApplication_register_producer_eventid(node, EVENT_ID_IDENT_BUTTON_COMBO_PRESSED, EVENT_STATUS_UNKNOWN);
}

static void setup_broadcast_time_consumer(openlcb_node_t *node) {
    OpenLcbApplicationBroadcastTime_setup_consumer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
}

static void setup_broadcast_time_producer(openlcb_node_t *node) {
    OpenLcbApplicationBroadcastTime_setup_producer(node, BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK);
}

static void setup_train(openlcb_node_t *node) {
    OpenLcbApplicationTrain_setup(node);
    OpenLcbApplicationTrain_set_dcc_address(node, 3, false);
    OpenLcbApplicationTrain_set_speed_steps(node, 3);

    // Enable heartbeat with 3-second timeout for compliance testing
    // (TrainControlS section 6.6 — timeout is at the train node's discretion)
    // Counter starts at 0; _handle_set_speed restarts it when speed goes non-zero.
    if (node->train_state) {

        node->train_state->heartbeat_timeout_s = 3;
        node->train_state->heartbeat_counter_100ms = 0;

    }

    // Pre-allocate virtual train nodes for dynamic search allocation
    for (int i = 0; i < VIRTUAL_TRAIN_COUNT; i++) {
        node_id_t virtual_id = node->id + 1 + i;
        virtual_train_pool[i] = OpenLcb_create_node(virtual_id, node->parameters);
        if (virtual_train_pool[i]) {
            OpenLcbApplicationTrain_setup(virtual_train_pool[i]);
            // Leave dcc_address at 0 — assigned on search allocation
        }
    }
    CallbacksTrain_set_virtual_pool(virtual_train_pool, VIRTUAL_TRAIN_COUNT);
}


// =============================================================================
// Protocol mode registry
// =============================================================================

const protocol_mode_t protocol_modes[] = {
    {
        .flag         = "--basic",
        .name         = "Basic Node",
        .test_section = "core",
        .params       = &compliance_basic_params,
        .setup        = setup_basic,
        .on_login     = NULL,
    },
    {
        .flag         = "--broadcast-time-consumer",
        .name         = "Broadcast Time Consumer",
        .test_section = "broadcast_time_consumer",
        .params       = &compliance_broadcast_time_consumer_params,
        .setup        = setup_broadcast_time_consumer,
        .on_login     = CallbacksBroadcastTime_on_login_complete_consumer,
    },
    {
        .flag         = "--broadcast-time-producer",
        .name         = "Broadcast Time Producer",
        .test_section = "broadcast_time_producer",
        .params       = &compliance_broadcast_time_producer_params,
        .setup        = setup_broadcast_time_producer,
        .on_login     = CallbacksBroadcastTime_on_login_complete_producer,
    },
    {
        .flag         = "--train",
        .name         = "Train Node",
        .test_section = "trains",
        .params       = &compliance_train_params,
        .setup        = setup_train,
        .on_login     = NULL,
    },
    // ---- Add new protocol modes here ----
    { .flag = NULL }  // sentinel
};


// =============================================================================
// Lookup functions
// =============================================================================

const protocol_mode_t *ProtocolModes_find(const char *flag) {
    for (int i = 0; protocol_modes[i].flag != NULL; i++) {
        if (strcmp(protocol_modes[i].flag, flag) == 0)
            return &protocol_modes[i];
    }
    return NULL;
}

const protocol_mode_t *ProtocolModes_default(void) {
    return &protocol_modes[0];  // basic
}

void ProtocolModes_print_usage(void) {
    printf("Usage: ComplianceTestNode [--mode-flag] [--node-id 0xNNNNNNNNNNNN]\n\n");
    printf("Available protocol modes:\n");
    for (int i = 0; protocol_modes[i].flag != NULL; i++) {
        printf("  %-30s  %s\n", protocol_modes[i].flag, protocol_modes[i].name);
    }
    printf("\nDefault: %s\n", protocol_modes[0].name);
}
