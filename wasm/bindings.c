/** @file bindings.c
 *  @brief WASM bindings for OpenLcbCLib.
 *
 *  Transport: CAN with gridconnect text frames relayed over the JS-side
 *  WebSocket.  Native binary-TCP support planned for the future.
 *
 *  JS entry points:
 *    - wasm_init(node_id)           initialize CAN + OpenLCB and create one node
 *    - wasm_run()                   run one pass of the main state machine
 *    - wasm_100ms_tick()            increment the global 100ms tick counter
 *    - wasm_rx_gridconnect(cstr)    feed an incoming gridconnect string into the stack
 *
 *  JS-side hook:
 *    Module.onGridconnectTx(str)    called with each outgoing gridconnect frame
 */

#include <stdint.h>
#include <stdbool.h>

#include <emscripten/emscripten.h>

#include "openlcb_user_config.h"
#include "openlcb/openlcb_config.h"
#include "openlcb/openlcb_node.h"
#include "openlcb/openlcb_gridconnect.h"
#include "drivers/canbus/can_config.h"
#include "drivers/canbus/can_types.h"
#include "drivers/canbus/can_rx_statemachine.h"

extern const node_parameters_t OpenLcbUserConfig_node_parameters;

// ---------------------------------------------------------------------------
// Outbound: C -> gridconnect text -> JS
// ---------------------------------------------------------------------------

static bool _transmit_raw_can_frame(can_msg_t *can_msg)
{

    gridconnect_buffer_t buf;
    OpenLcbGridConnect_from_can_msg(&buf, can_msg);

    EM_ASM({
        if (Module.onGridconnectTx) {
            Module.onGridconnectTx(UTF8ToString($0));
        }
    }, (const char *) buf);

    return true;
}

static bool _is_tx_buffer_clear(void)
{

    return true;
}

static void _lock(void)
{

}

static void _unlock(void)
{

}

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION

static uint16_t _stub_config_mem_read(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    (void) node; (void) address; (void) buffer;
    return count;
}

static uint16_t _stub_config_mem_write(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    (void) node; (void) address; (void) buffer;
    return count;
}

#endif

#ifdef OPENLCB_COMPILE_FIRMWARE

static void _stub_freeze(openlcb_statemachine_info_t *sm, config_mem_operations_request_info_t *info)
{

    (void) sm; (void) info;
}

static void _stub_unfreeze(openlcb_statemachine_info_t *sm, config_mem_operations_request_info_t *info)
{

    (void) sm; (void) info;
}

static void _stub_firmware_write(openlcb_statemachine_info_t *sm, config_mem_write_request_info_t *info, write_result_t write_result)
{

    write_result(sm, info, true);
}

#endif

// ---------------------------------------------------------------------------
// Config structs
// ---------------------------------------------------------------------------

static const can_config_t _can_config = {
    .transmit_raw_can_frame  = &_transmit_raw_can_frame,
    .is_tx_buffer_clear      = &_is_tx_buffer_clear,
    .lock_shared_resources   = &_lock,
    .unlock_shared_resources = &_unlock,
};

static const openlcb_config_t _openlcb_config = {
    .lock_shared_resources   = &_lock,
    .unlock_shared_resources = &_unlock,
#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
    .config_mem_read         = &_stub_config_mem_read,
    .config_mem_write        = &_stub_config_mem_write,
#endif
#ifdef OPENLCB_COMPILE_FIRMWARE
    .freeze                  = &_stub_freeze,
    .unfreeze                = &_stub_unfreeze,
    .firmware_write          = &_stub_firmware_write,
#endif
};

// ---------------------------------------------------------------------------
// Exported entry points
// ---------------------------------------------------------------------------

EMSCRIPTEN_KEEPALIVE
void wasm_init(uint64_t node_id)
{

    CanConfig_initialize(&_can_config);
    OpenLcbConfig_initialize(&_openlcb_config);
    (void) OpenLcbConfig_create_node(node_id, &OpenLcbUserConfig_node_parameters);
}

EMSCRIPTEN_KEEPALIVE
void wasm_run(void)
{

    OpenLcbConfig_run();
}

EMSCRIPTEN_KEEPALIVE
void wasm_100ms_tick(void)
{

    OpenLcbConfig_100ms_timer_tick();
}

/**
 *  Feed a null-terminated gridconnect string (one or more concatenated frames)
 *  into the parser.  Completed frames are injected into the CAN RX path.
 */
EMSCRIPTEN_KEEPALIVE
void wasm_rx_gridconnect(const char *cstr)
{

    if (cstr == NULL) { return; }

    gridconnect_buffer_t gc_buf;
    can_msg_t can_msg;

    for (const char *p = cstr; *p != '\0'; p++)
    {

        if (OpenLcbGridConnect_copy_out_gridconnect_when_done((uint8_t) *p, &gc_buf))
        {

            OpenLcbGridConnect_to_can_msg(&gc_buf, &can_msg);
            CanRxStatemachine_incoming_can_driver_callback(&can_msg);
        }
    }
}
