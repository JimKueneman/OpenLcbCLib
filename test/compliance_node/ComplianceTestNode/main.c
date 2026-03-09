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
 * \file main.c
 *
 * ComplianceTestNode entry point. Selects a protocol mode from the registry
 * based on CLI arguments, creates a single node, and enters the main loop.
 *
 * This file never changes when protocols are added — it just looks up the
 * mode and calls setup.
 *
 * @author Jim Kueneman
 * @date 8 Mar 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "protocol_modes.h"
#include "callbacks_core.h"
#include "callbacks_broadcast_time.h"
#include "callbacks_trains.h"
#include "openlcb_user_config.h"
#include "application_drivers/osx_drivers.h"
#include "application_drivers/osx_can_drivers.h"

#include "src/drivers/canbus/can_config.h"
#include "src/openlcb/openlcb_config.h"
#include "src/openlcb/openlcb_application_broadcast_time.h"

#define DEFAULT_NODE_ID 0x050701010033

// Parse "05.07.01.01.00.33" dotted hex or "0x050701010033" numeric node ID
static uint64_t parse_node_id(const char *str) {
    unsigned int b[6];
    if (sscanf(str, "%x.%x.%x.%x.%x.%x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) == 6) {
        return ((uint64_t)b[0] << 40) | ((uint64_t)b[1] << 32) |
               ((uint64_t)b[2] << 24) | ((uint64_t)b[3] << 16) |
               ((uint64_t)b[4] << 8)  | (uint64_t)b[5];
    }
    return strtoull(str, NULL, 0);
}

// The active protocol mode — set once before node creation
static const protocol_mode_t *active_mode = NULL;

// Login complete dispatcher — routes to the active mode's handler
static bool on_login_complete_dispatcher(openlcb_node_t *openlcb_node) {
    if (active_mode && active_mode->on_login)
        return active_mode->on_login(openlcb_node);
    return true;
}


// =============================================================================
// CAN and OpenLCB configuration structs
// =============================================================================

static const can_config_t can_config = {
    .transmit_raw_can_frame  = &OSxCanDriver_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &OSxCanDriver_is_can_tx_buffer_clear,
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .on_rx                   = &CallbacksCore_on_can_rx_callback,
    .on_tx                   = &CallbacksCore_on_can_tx_callback,
    .on_alias_change         = &CallbacksCore_alias_change_callback,
};

static const openlcb_config_t openlcb_config = {
    // Required hardware
    .lock_shared_resources   = &OSxDrivers_lock_shared_resources,
    .unlock_shared_resources = &OSxDrivers_unlock_shared_resources,
    .config_mem_read         = &OSxDrivers_config_mem_read,
    .config_mem_write        = &OSxDrivers_config_mem_write,
    .reboot                  = &OSxDrivers_reboot,

    // Optional hardware extensions
    .factory_reset           = &CallbacksCore_operations_request_factory_reset,
    .freeze                  = &OSxDrivers_freeze,
    .unfreeze                = &OSxDrivers_unfreeze,
    .firmware_write          = &OSxDrivers_write_firmware,

    // Core application callbacks
    .on_100ms_timer          = &CallbacksCore_on_100ms_timer_callback,
    .on_login_complete       = &on_login_complete_dispatcher,

    // Event transport callbacks
    .on_consumed_event_identified = &CallbacksCore_on_consumed_event_identified,
    .on_consumed_event_pcer       = &CallbacksCore_on_consumed_event_pcer,
    .on_event_learn               = &CallbacksCore_on_event_learn,

    // Broadcast time callbacks
    .on_broadcast_time_changed    = &CallbacksBroadcastTime_on_time_changed,

    // Train callbacks
    .on_train_speed_changed              = &CallbacksTrains_on_speed_changed,
    .on_train_function_changed           = &CallbacksTrains_on_function_changed,
    .on_train_emergency_entered          = &CallbacksTrains_on_emergency_entered,
    .on_train_emergency_exited           = &CallbacksTrains_on_emergency_exited,
    .on_train_controller_assigned        = &CallbacksTrains_on_controller_assigned,
    .on_train_controller_released        = &CallbacksTrains_on_controller_released,
    .on_train_heartbeat_timeout          = &CallbacksTrains_on_heartbeat_timeout,
    .on_train_controller_assign_request  = &CallbacksTrains_on_controller_assign_request,
    .on_train_controller_changed_request = &CallbacksTrains_on_controller_changed_request,

    // Train search callbacks
    .on_train_search_no_match            = &CallbacksTrains_on_search_no_match,
};


// =============================================================================
// Entry point
// =============================================================================

int main(int argc, char *argv[]) {

    // Disable stdout buffering so output appears immediately (even when piped)
    setvbuf(stdout, NULL, _IONBF, 0);

    // 1. Find the requested protocol mode
    active_mode = ProtocolModes_default();
    uint64_t nodeid = DEFAULT_NODE_ID;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--node-id") == 0 && i + 1 < argc) {
            nodeid = parse_node_id(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            ProtocolModes_print_usage();
            return 0;
        } else {
            const protocol_mode_t *found = ProtocolModes_find(argv[i]);
            if (found)
                active_mode = found;
        }
    }

    printf("ComplianceTestNode: mode=%s nodeid=0x%012llX\n", active_mode->name, nodeid);

    // 2. Initialize drivers and library
    CanConfig_initialize(&can_config);
    OpenLcb_initialize(&openlcb_config);

    OSxDrivers_setup();
    OSxCanDriver_setup();

    // 3. Wait for threads to connect
    printf("Waiting for CAN and 100ms Timer Drivers to connect\n");
    while (!(OSxDrivers_100ms_is_connected() && OSxCanDriver_is_connected())) {
        printf("Waiting for Threads\n");
        sleep(2);
    }

    // 4. Create node with the selected mode's parameters
    openlcb_node_t *node = OpenLcb_create_node(nodeid, active_mode->params);
    printf("Node Allocated.....\n");

    // 5. Run protocol-specific setup (if any)
    if (active_mode->setup) {
        active_mode->setup(node);
        printf("Setup complete for mode: %s\n", active_mode->name);
    }

    printf("Entering main loop (node index=%d)\n", node->index);

    // 6. Main loop (lazy sleep pattern)
    int idle_count = 0;

    while (1) {

        OpenLcb_run();

        if (OSxCanDriver_data_was_received())
            idle_count = 0;
        else
            idle_count++;

        if (idle_count > 100)
            usleep(50);
    }
}
