/** @file bindings.c
 *  @brief WASM bindings for OpenLcbCLib.
 *
 *  Transport: CAN with gridconnect text frames relayed over the JS-side
 *  socket.  Native binary TCP planned for the future.
 *
 *  Everything is keyed off 48-bit node IDs — there are no opaque handles.
 *
 *  ABI summary (see wasm/README.md for full docs):
 *
 *    wasm_initialize()                          one-time transport + stack bring-up
 *    wasm_rx_gridconnect(cstr)                  feed incoming gridconnect into RX
 *    wasm_run()                                 run main state machine one pass
 *    wasm_100ms_tick()                          advance the 100ms counter
 *
 *    wasm_node_builder_reset()                  clear scratch parameters
 *    wasm_node_set_snip(...)                    fill SNIP strings
 *    wasm_node_set_protocol_support(bits)       protocol support mask
 *    wasm_node_set_event_autocreate(p, c)       producer / consumer autocreate counts
 *    wasm_node_set_configuration_options(...)   memory-config capability flags
 *    wasm_node_set_address_space(...)           populate one address-space entry
 *    wasm_node_set_cdi(ptr, len)                stage CDI bytes (copied)
 *    wasm_node_set_fdi(ptr, len)                stage FDI bytes (copied)
 *    wasm_create_node(node_id)                  commit scratch as a new node
 *
 *    wasm_send_event_pc_report(node_id, event_id)
 *
 *  JS hooks (set on the Emscripten Module before factory resolves):
 *    Module.onGridconnectTx(frame)
 *    Module.onLoginComplete(nodeIdBigInt)
 *    Module.onPcEventReport(nodeIdBigInt, eventIdBigInt)
 *    Module.onConsumedEventPcer(nodeIdBigInt, eventIndex, eventIdBigInt)
 *    Module.onConsumedEventIdentified(nodeIdBigInt, eventIndex, eventIdBigInt, status)
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten/emscripten.h>

#include "openlcb_user_config.h"
#include "openlcb/openlcb_config.h"
#include "openlcb/openlcb_node.h"
#include "openlcb/openlcb_application.h"
#include "openlcb/openlcb_application_broadcast_time.h"
#include "openlcb/openlcb_application_train.h"
#include "openlcb/openlcb_application_dcc_detector.h"
#include "openlcb/openlcb_gridconnect.h"
#include "openlcb/openlcb_utilities.h"
#include "openlcb/openlcb_float16.h"
#ifdef OPENLCB_COMPILE_BROADCAST_TIME
#include "openlcb/protocol_broadcast_time_handler.h"
#endif
#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
#include "openlcb/protocol_train_search_handler.h"
#endif
#include "drivers/canbus/can_config.h"
#include "drivers/canbus/can_types.h"
#include "drivers/canbus/can_rx_statemachine.h"
#include "drivers/canbus/internal_node_alias_table.h"

// ---------------------------------------------------------------------------
// Error codes — document in wasm/README.md
// ---------------------------------------------------------------------------

#define WASM_OK                        0
#define WASM_ERR_INVALID_ARG          -1
#define WASM_ERR_CEILING_EXCEEDED     -2
#define WASM_ERR_UNKNOWN_NODE         -3
#define WASM_ERR_TX_BUSY              -4
#define WASM_ERR_NOT_INITIALIZED      -5

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void _copy_cstr(char *dst, size_t dst_size, const char *src)
{

    if (dst_size == 0) { return; }

    if (src == NULL)
    {

        dst[0] = '\0';
        return;
    }

    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++)
    {

        dst[i] = src[i];
    }
    dst[i] = '\0';
}

// ---------------------------------------------------------------------------
// Outbound C -> JS: gridconnect TX
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

static void _lock(void)   { }
static void _unlock(void) { }

// ---------------------------------------------------------------------------
// Application-layer callbacks wired into the openlcb_config_t
// ---------------------------------------------------------------------------

static bool _on_login_complete(openlcb_node_t *node)
{

    EM_ASM({
        if (Module.onLoginComplete) {
            Module.onLoginComplete(BigInt($0) | (BigInt($1) << 32n));
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu));
    return true;
}

#ifdef OPENLCB_COMPILE_EVENTS

static void _on_pc_event_report(openlcb_node_t *node, event_id_t *event_id)
{

    EM_ASM({
        if (Module.onPcEventReport) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var eid = BigInt($2) | (BigInt($3) << 32n);
            Module.onPcEventReport(nid, eid);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (*event_id & 0xFFFFFFFFu),
       (uint32_t) ((*event_id >> 32) & 0xFFFFFFFFu));
}

static void _on_consumed_event_pcer(openlcb_node_t *node, uint16_t index, event_id_t *event_id, event_payload_t *payload)
{

    (void) payload;
    EM_ASM({
        if (Module.onConsumedEventPcer) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var eid = BigInt($2) | (BigInt($3) << 32n);
            Module.onConsumedEventPcer(nid, $4, eid);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (*event_id & 0xFFFFFFFFu),
       (uint32_t) ((*event_id >> 32) & 0xFFFFFFFFu),
       (int) index);
}

static void _on_consumed_event_identified(openlcb_node_t *node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload)
{

    (void) payload;
    EM_ASM({
        if (Module.onConsumedEventIdentified) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var eid = BigInt($2) | (BigInt($3) << 32n);
            Module.onConsumedEventIdentified(nid, $4, eid, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (*event_id & 0xFFFFFFFFu),
       (uint32_t) ((*event_id >> 32) & 0xFFFFFFFFu),
       (int) index,
       (int) status);
}

// Simple (node, event_id) events
#define _EVENT_SIMPLE_TRAMPOLINE(fn_name, js_hook)                               \
static void fn_name(openlcb_node_t *node, event_id_t *event_id)                  \
{                                                                                \
    EM_ASM({                                                                     \
        if (Module.js_hook) {                                                    \
            var nid = BigInt($0) | (BigInt($1) << 32n);                          \
            var eid = BigInt($2) | (BigInt($3) << 32n);                          \
            Module.js_hook(nid, eid);                                            \
        }                                                                        \
    }, (uint32_t) (node->id & 0xFFFFFFFFu),                                      \
       (uint32_t) ((node->id >> 32) & 0xFFFFu),                                  \
       (uint32_t) (*event_id & 0xFFFFFFFFu),                                     \
       (uint32_t) ((*event_id >> 32) & 0xFFFFFFFFu));                            \
}

_EVENT_SIMPLE_TRAMPOLINE(_on_event_learn,                      onEventLearn)
_EVENT_SIMPLE_TRAMPOLINE(_on_consumer_range_identified,        onConsumerRangeIdentified)
_EVENT_SIMPLE_TRAMPOLINE(_on_consumer_identified_unknown,      onConsumerIdentifiedUnknown)
_EVENT_SIMPLE_TRAMPOLINE(_on_consumer_identified_set,          onConsumerIdentifiedSet)
_EVENT_SIMPLE_TRAMPOLINE(_on_consumer_identified_clear,        onConsumerIdentifiedClear)
_EVENT_SIMPLE_TRAMPOLINE(_on_consumer_identified_reserved,     onConsumerIdentifiedReserved)
_EVENT_SIMPLE_TRAMPOLINE(_on_producer_range_identified,        onProducerRangeIdentified)
_EVENT_SIMPLE_TRAMPOLINE(_on_producer_identified_unknown,      onProducerIdentifiedUnknown)
_EVENT_SIMPLE_TRAMPOLINE(_on_producer_identified_set,          onProducerIdentifiedSet)
_EVENT_SIMPLE_TRAMPOLINE(_on_producer_identified_clear,        onProducerIdentifiedClear)
_EVENT_SIMPLE_TRAMPOLINE(_on_producer_identified_reserved,     onProducerIdentifiedReserved)

static void _on_pc_event_report_with_payload(openlcb_node_t *node, event_id_t *event_id, uint16_t count, event_payload_t *payload)
{

    EM_ASM({
        if (Module.onPcEventReportWithPayload) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var eid = BigInt($2) | (BigInt($3) << 32n);
            Module.onPcEventReportWithPayload(nid, eid, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (*event_id & 0xFFFFFFFFu),
       (uint32_t) ((*event_id >> 32) & 0xFFFFFFFFu),
       (int) count,
       (uint32_t) (uintptr_t) payload);
}

#endif /* OPENLCB_COMPILE_EVENTS */

// ---------------------------------------------------------------------------
// Broadcast Time callbacks
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

static void _on_broadcast_time_changed(broadcast_clock_t *clock)
{

    broadcast_clock_state_t *s = &clock->state;
    EM_ASM({
        if (Module.onBroadcastTimeChanged) {
            var cid = BigInt($0) | (BigInt($1) << 32n);
            Module.onBroadcastTimeChanged(cid, $2, $3);
        }
    }, (uint32_t) (s->clock_id & 0xFFFFFFFFu),
       (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu),
       (int) s->time.hour,
       (int) s->time.minute);
}

static void _on_broadcast_time_received(openlcb_node_t *node, broadcast_clock_state_t *s)
{

    EM_ASM({
        if (Module.onBroadcastTimeReceived) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($2) | (BigInt($3) << 32n);
            Module.onBroadcastTimeReceived(nid, cid, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (s->clock_id & 0xFFFFFFFFu), (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu),
       (int) s->time.hour, (int) s->time.minute);
}

static void _on_broadcast_date_received(openlcb_node_t *node, broadcast_clock_state_t *s)
{

    EM_ASM({
        if (Module.onBroadcastDateReceived) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($2) | (BigInt($3) << 32n);
            Module.onBroadcastDateReceived(nid, cid, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (s->clock_id & 0xFFFFFFFFu), (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu),
       (int) s->date.month, (int) s->date.day);
}

static void _on_broadcast_year_received(openlcb_node_t *node, broadcast_clock_state_t *s)
{

    EM_ASM({
        if (Module.onBroadcastYearReceived) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($2) | (BigInt($3) << 32n);
            Module.onBroadcastYearReceived(nid, cid, $4);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (s->clock_id & 0xFFFFFFFFu), (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu),
       (int) s->year.year);
}

static void _on_broadcast_rate_received(openlcb_node_t *node, broadcast_clock_state_t *s)
{

    EM_ASM({
        if (Module.onBroadcastRateReceived) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($2) | (BigInt($3) << 32n);
            Module.onBroadcastRateReceived(nid, cid, $4);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (s->clock_id & 0xFFFFFFFFu), (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu),
       (int) s->rate.rate);
}

#define _BTIME_SIMPLE_TRAMPOLINE(fn_name, js_hook)                                    \
static void fn_name(openlcb_node_t *node, broadcast_clock_state_t *s)                 \
{                                                                                     \
    EM_ASM({                                                                          \
        if (Module.js_hook) {                                                         \
            var nid = BigInt($0) | (BigInt($1) << 32n);                               \
            var cid = BigInt($2) | (BigInt($3) << 32n);                               \
            Module.js_hook(nid, cid);                                                 \
        }                                                                             \
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),  \
       (uint32_t) (s->clock_id & 0xFFFFFFFFu),                                        \
       (uint32_t) ((s->clock_id >> 32) & 0xFFFFFFFFu));                               \
}

_BTIME_SIMPLE_TRAMPOLINE(_on_broadcast_clock_started, onBroadcastClockStarted)
_BTIME_SIMPLE_TRAMPOLINE(_on_broadcast_clock_stopped, onBroadcastClockStopped)
_BTIME_SIMPLE_TRAMPOLINE(_on_broadcast_date_rollover, onBroadcastDateRollover)

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

// ---------------------------------------------------------------------------
// Train callbacks
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_TRAIN

#define _TRAIN_NODE_TRAMPOLINE(fn_name, js_hook)                                 \
static void fn_name(openlcb_node_t *node)                                        \
{                                                                                \
    EM_ASM({                                                                     \
        if (Module.js_hook) {                                                    \
            var nid = BigInt($0) | (BigInt($1) << 32n);                          \
            Module.js_hook(nid);                                                 \
        }                                                                        \
    }, (uint32_t) (node->id & 0xFFFFFFFFu),                                      \
       (uint32_t) ((node->id >> 32) & 0xFFFFu));                                 \
}

static void _on_train_speed_changed(openlcb_node_t *node, uint16_t speed_f16)
{

    EM_ASM({
        if (Module.onTrainSpeedChanged) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainSpeedChanged(nid, $2);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) speed_f16);
}

static void _on_train_function_changed(openlcb_node_t *node, uint32_t fn_address, uint16_t fn_value)
{

    EM_ASM({
        if (Module.onTrainFunctionChanged) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainFunctionChanged(nid, $2 >>> 0, $3);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) fn_address,
       (int) fn_value);
}

static void _on_train_emergency_entered(openlcb_node_t *node, train_emergency_type_enum t)
{

    EM_ASM({
        if (Module.onTrainEmergencyEntered) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainEmergencyEntered(nid, $2);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) t);
}

static void _on_train_emergency_exited(openlcb_node_t *node, train_emergency_type_enum t)
{

    EM_ASM({
        if (Module.onTrainEmergencyExited) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainEmergencyExited(nid, $2);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) t);
}

static void _on_train_controller_assigned(openlcb_node_t *node, node_id_t ctrl)
{

    EM_ASM({
        if (Module.onTrainControllerAssigned) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($2) | (BigInt($3) << 32n);
            Module.onTrainControllerAssigned(nid, cid);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (ctrl & 0xFFFFFFFFu),
       (uint32_t) ((ctrl >> 32) & 0xFFFFu));
}

_TRAIN_NODE_TRAMPOLINE(_on_train_controller_released, onTrainControllerReleased)
_TRAIN_NODE_TRAMPOLINE(_on_train_listener_changed,    onTrainListenerChanged)
_TRAIN_NODE_TRAMPOLINE(_on_train_heartbeat_timeout,   onTrainHeartbeatTimeout)

static bool _on_train_controller_assign_request(openlcb_node_t *node, node_id_t cur, node_id_t req)
{

    int accept = EM_ASM_INT({
        if (Module.onTrainControllerAssignRequest) {
            var nid  = BigInt($0) | (BigInt($1) << 32n);
            var curr = BigInt($2) | (BigInt($3) << 32n);
            var rq   = BigInt($4) | (BigInt($5) << 32n);
            return Module.onTrainControllerAssignRequest(nid, curr, rq) ? 1 : 0;
        }
        return 1;
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (cur & 0xFFFFFFFFu),      (uint32_t) ((cur >> 32) & 0xFFFFu),
       (uint32_t) (req & 0xFFFFFFFFu),      (uint32_t) ((req >> 32) & 0xFFFFu));
    return accept != 0;
}

static bool _on_train_controller_changed_request(openlcb_node_t *node, node_id_t nc)
{

    int accept = EM_ASM_INT({
        if (Module.onTrainControllerChangedRequest) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var new_ctrl = BigInt($2) | (BigInt($3) << 32n);
            return Module.onTrainControllerChangedRequest(nid, new_ctrl) ? 1 : 0;
        }
        return 1;
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (nc & 0xFFFFFFFFu),       (uint32_t) ((nc >> 32) & 0xFFFFu));
    return accept != 0;
}

static void _on_train_query_speeds_reply(openlcb_node_t *node, uint16_t set_speed, uint8_t status, uint16_t commanded, uint16_t actual)
{

    EM_ASM({
        if (Module.onTrainQuerySpeedsReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainQuerySpeedsReply(nid, $2, $3, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) set_speed, (int) status, (int) commanded, (int) actual);
}

static void _on_train_query_function_reply(openlcb_node_t *node, uint32_t fn_address, uint16_t fn_value)
{

    EM_ASM({
        if (Module.onTrainQueryFunctionReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainQueryFunctionReply(nid, $2 >>> 0, $3);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) fn_address, (int) fn_value);
}

static void _on_train_controller_assign_reply(openlcb_node_t *node, uint8_t result, node_id_t current)
{

    EM_ASM({
        if (Module.onTrainControllerAssignReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cur = BigInt($3) | (BigInt($4) << 32n);
            Module.onTrainControllerAssignReply(nid, $2, cur);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) result,
       (uint32_t) (current & 0xFFFFFFFFu), (uint32_t) ((current >> 32) & 0xFFFFu));
}

static void _on_train_controller_query_reply(openlcb_node_t *node, uint8_t flags, node_id_t ctrl)
{

    EM_ASM({
        if (Module.onTrainControllerQueryReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var cid = BigInt($3) | (BigInt($4) << 32n);
            Module.onTrainControllerQueryReply(nid, $2, cid);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) flags,
       (uint32_t) (ctrl & 0xFFFFFFFFu), (uint32_t) ((ctrl >> 32) & 0xFFFFu));
}

static void _on_train_controller_changed_notify_reply(openlcb_node_t *node, uint8_t result)
{

    EM_ASM({
        if (Module.onTrainControllerChangedNotifyReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainControllerChangedNotifyReply(nid, $2);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) result);
}

static void _on_train_listener_attach_reply(openlcb_node_t *node, node_id_t lid, uint8_t result)
{

    EM_ASM({
        if (Module.onTrainListenerAttachReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var lnode = BigInt($2) | (BigInt($3) << 32n);
            Module.onTrainListenerAttachReply(nid, lnode, $4);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (lid & 0xFFFFFFFFu), (uint32_t) ((lid >> 32) & 0xFFFFu),
       (int) result);
}

static void _on_train_listener_detach_reply(openlcb_node_t *node, node_id_t lid, uint8_t result)
{

    EM_ASM({
        if (Module.onTrainListenerDetachReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var lnode = BigInt($2) | (BigInt($3) << 32n);
            Module.onTrainListenerDetachReply(nid, lnode, $4);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (lid & 0xFFFFFFFFu), (uint32_t) ((lid >> 32) & 0xFFFFu),
       (int) result);
}

static void _on_train_listener_query_reply(openlcb_node_t *node, uint8_t count, uint8_t index, uint8_t flags, node_id_t lid)
{

    EM_ASM({
        if (Module.onTrainListenerQueryReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var lnode = BigInt($4) | (BigInt($5) << 32n);
            Module.onTrainListenerQueryReply(nid, $2, $3, lnode);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) count, (int) index,
       (uint32_t) (lid & 0xFFFFFFFFu), (uint32_t) ((lid >> 32) & 0xFFFFu));
    (void) flags;
}

static void _on_train_reserve_reply(openlcb_node_t *node, uint8_t result)
{

    EM_ASM({
        if (Module.onTrainReserveReply) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainReserveReply(nid, $2);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (int) result);
}

static void _on_train_heartbeat_request(openlcb_node_t *node, uint32_t timeout_s)
{

    EM_ASM({
        if (Module.onTrainHeartbeatRequest) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            Module.onTrainHeartbeatRequest(nid, $2 >>> 0);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) timeout_s);
}

#endif /* OPENLCB_COMPILE_TRAIN */

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

static void _on_train_search_matched(openlcb_node_t *node, event_id_t search_event_id)
{

    // Pass the node ID and event ID to JS as two 32-bit halves each so the
    // trampoline stays in int-sized EM_ASM arguments.  Node IDs are 48-bit;
    // event IDs are full 64-bit.
    EM_ASM({
        if (Module.onTrainSearchMatched) {
            var nid = BigInt($0 >>> 0) | (BigInt($1 >>> 0) << 32n);
            var eid = BigInt($2 >>> 0) | (BigInt($3 >>> 0) << 32n);
            Module.onTrainSearchMatched(nid, eid);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (search_event_id & 0xFFFFFFFFu),
       (uint32_t) ((search_event_id >> 32) & 0xFFFFFFFFu));
}

static openlcb_node_t *_on_train_search_no_match_with_allocate(event_id_t search_event_id)
{

    // JS may return a node ID (BigInt) to allocate-on-search.  Coerce to a
    // double — OpenLCB node IDs are 48-bit and fit losslessly.  Return 0
    // (or anything non-BigInt) to decline.  The JS callback is responsible
    // for creating the node via wasm_create_node before returning its ID.
    // After we hand the node back, the CS must call
    // wasm_train_send_search_match to emit the Producer Identified reply
    // (TrainSearchS §6.2).
    double r = EM_ASM_DOUBLE({
        if (Module.onTrainSearchNoMatch) {
            var eid = BigInt($0 >>> 0) | (BigInt($1 >>> 0) << 32n);
            var v = Module.onTrainSearchNoMatch(eid);
            if (typeof v === 'bigint') return Number(v);
            if (typeof v === 'number') return v;
        }
        return 0;
    }, (uint32_t) (search_event_id & 0xFFFFFFFFu),
       (uint32_t) ((search_event_id >> 32) & 0xFFFFFFFFu));
    if (r <= 0) { return NULL; }
    node_id_t nid = (node_id_t) r;
    return OpenLcbNode_find_by_node_id(nid);
}

// Throttle-side hook — fires when a remote train replies to a search this
// device sent.  Carries the replier's 48-bit Node ID + 12-bit CAN alias so
// the throttle can address subsequent Train Control messages to that train
// without reconstructing the ID from the CS base.
static void _on_train_search_reply(source_info_t *source, event_id_t event_id)
{

    if (source == NULL) { return; }
    EM_ASM({
        if (Module.onTrainSearchReply) {
            var sid = BigInt($0 >>> 0) | (BigInt($1 >>> 0) << 32n);
            var alias = $2 & 0xFFF;
            var eid = BigInt($3 >>> 0) | (BigInt($4 >>> 0) << 32n);
            Module.onTrainSearchReply(sid, alias, eid);
        }
    }, (uint32_t) (source->source_id & 0xFFFFFFFFu),
       (uint32_t) ((source->source_id >> 32) & 0xFFFFu),
       (int) source->source_alias,
       (uint32_t) (event_id & 0xFFFFFFFFu),
       (uint32_t) ((event_id >> 32) & 0xFFFFFFFFu));
}

#endif

// ---------------------------------------------------------------------------
// Stream callbacks
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_STREAM

static bool _on_stream_initiate_request(openlcb_statemachine_info_t *sm, stream_state_t *s)
{

    (void) sm;
    int accept = EM_ASM_INT({
        if (Module.onStreamInitiateRequest) {
            return Module.onStreamInitiateRequest($0) ? 1 : 0;
        }
        return 1;
    }, (uint32_t) (uintptr_t) s);
    return accept != 0;
}

#define _STREAM_TRAMPOLINE(fn_name, js_hook)                                     \
static void fn_name(openlcb_statemachine_info_t *sm, stream_state_t *s)          \
{                                                                                \
    (void) sm;                                                                   \
    EM_ASM({                                                                     \
        if (Module.js_hook) { Module.js_hook($0); }                              \
    }, (uint32_t) (uintptr_t) s);                                                \
}

_STREAM_TRAMPOLINE(_on_stream_initiate_reply, onStreamInitiateReply)
_STREAM_TRAMPOLINE(_on_stream_data_received,  onStreamDataReceived)
_STREAM_TRAMPOLINE(_on_stream_data_proceed,   onStreamDataProceed)
_STREAM_TRAMPOLINE(_on_stream_complete,       onStreamComplete)

#endif /* OPENLCB_COMPILE_STREAM */

// ---------------------------------------------------------------------------
// Core application error callbacks
// ---------------------------------------------------------------------------

static void _on_optional_interaction_rejected(openlcb_node_t *node, node_id_t src, uint16_t error_code, uint16_t rejected_mti)
{

    EM_ASM({
        if (Module.onOptionalInteractionRejected) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var src_id = BigInt($2) | (BigInt($3) << 32n);
            Module.onOptionalInteractionRejected(nid, src_id, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (src & 0xFFFFFFFFu),      (uint32_t) ((src >> 32) & 0xFFFFu),
       (int) error_code, (int) rejected_mti);
}

static void _on_terminate_due_to_error(openlcb_node_t *node, node_id_t src, uint16_t error_code, uint16_t rejected_mti)
{

    EM_ASM({
        if (Module.onTerminateDueToError) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            var src_id = BigInt($2) | (BigInt($3) << 32n);
            Module.onTerminateDueToError(nid, src_id, $4, $5);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu), (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (src & 0xFFFFFFFFu),      (uint32_t) ((src >> 32) & 0xFFFFu),
       (int) error_code, (int) rejected_mti);
}

// Receiver-side hook for Verified Node ID replies from remote nodes.
// Carries both the resolved 48-bit NodeID and the source 12-bit alias from
// the wire frame so the JS application can address subsequent messages by
// either identifier (forward-compatible with TCP transport).
static void _on_verified_node_id(openlcb_node_t *node, source_info_t *source)
{

    if (source == NULL) { return; }
    EM_ASM({
        if (Module.onVerifiedNodeId) {
            var nid = BigInt($0 >>> 0) | (BigInt($1 >>> 0) << 32n);
            var sid = BigInt($2 >>> 0) | (BigInt($3 >>> 0) << 32n);
            var alias = $4 & 0xFFF;
            Module.onVerifiedNodeId(nid, sid, alias);
        }
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) (source->source_id & 0xFFFFFFFFu),
       (uint32_t) ((source->source_id >> 32) & 0xFFFFu),
       (int) source->source_alias);
}

static void _on_100ms_timer(void)
{

    EM_ASM({
        if (Module.on100msTimer) { Module.on100msTimer(); }
    });
}

// ---------------------------------------------------------------------------
// Config-memory driver callbacks (ptr-based JS trampolines)
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION

static uint16_t _config_mem_read(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    int result = EM_ASM_INT({
        if (Module.onConfigMemRead) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            return Module.onConfigMemRead(nid, $2 >>> 0, $3, $4) | 0;
        }
        return 0;
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) address,
       (int) count,
       (uint32_t) (uintptr_t) buffer);

    if (result < 0) { return 0; }
    if ((uint32_t) result > count) { return count; }
    return (uint16_t) result;
}

static uint16_t _config_mem_write(openlcb_node_t *node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    int result = EM_ASM_INT({
        if (Module.onConfigMemWrite) {
            var nid = BigInt($0) | (BigInt($1) << 32n);
            return Module.onConfigMemWrite(nid, $2 >>> 0, $3, $4) | 0;
        }
        return 0;
    }, (uint32_t) (node->id & 0xFFFFFFFFu),
       (uint32_t) ((node->id >> 32) & 0xFFFFu),
       (uint32_t) address,
       (int) count,
       (uint32_t) (uintptr_t) buffer);

    if (result < 0) { return 0; }
    if ((uint32_t) result > count) { return count; }
    return (uint16_t) result;
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
    .config_mem_read         = &_config_mem_read,
    .config_mem_write        = &_config_mem_write,
#endif
    .on_optional_interaction_rejected = &_on_optional_interaction_rejected,
    .on_terminate_due_to_error        = &_on_terminate_due_to_error,
    .on_verified_node_id              = &_on_verified_node_id,
    .on_100ms_timer                   = &_on_100ms_timer,
    .on_login_complete       = &_on_login_complete,
#ifdef OPENLCB_COMPILE_EVENTS
    .on_pc_event_report               = &_on_pc_event_report,
    .on_pc_event_report_with_payload  = &_on_pc_event_report_with_payload,
    .on_consumed_event_pcer           = &_on_consumed_event_pcer,
    .on_consumed_event_identified     = &_on_consumed_event_identified,
    .on_event_learn                   = &_on_event_learn,
    .on_consumer_range_identified     = &_on_consumer_range_identified,
    .on_consumer_identified_unknown   = &_on_consumer_identified_unknown,
    .on_consumer_identified_set       = &_on_consumer_identified_set,
    .on_consumer_identified_clear     = &_on_consumer_identified_clear,
    .on_consumer_identified_reserved  = &_on_consumer_identified_reserved,
    .on_producer_range_identified     = &_on_producer_range_identified,
    .on_producer_identified_unknown   = &_on_producer_identified_unknown,
    .on_producer_identified_set       = &_on_producer_identified_set,
    .on_producer_identified_clear     = &_on_producer_identified_clear,
    .on_producer_identified_reserved  = &_on_producer_identified_reserved,
#endif
#ifdef OPENLCB_COMPILE_BROADCAST_TIME
    .on_broadcast_time_changed   = &_on_broadcast_time_changed,
    .on_broadcast_time_received  = &_on_broadcast_time_received,
    .on_broadcast_date_received  = &_on_broadcast_date_received,
    .on_broadcast_year_received  = &_on_broadcast_year_received,
    .on_broadcast_rate_received  = &_on_broadcast_rate_received,
    .on_broadcast_clock_started  = &_on_broadcast_clock_started,
    .on_broadcast_clock_stopped  = &_on_broadcast_clock_stopped,
    .on_broadcast_date_rollover  = &_on_broadcast_date_rollover,
#endif
#ifdef OPENLCB_COMPILE_TRAIN
    .on_train_speed_changed               = &_on_train_speed_changed,
    .on_train_function_changed            = &_on_train_function_changed,
    .on_train_emergency_entered           = &_on_train_emergency_entered,
    .on_train_emergency_exited            = &_on_train_emergency_exited,
    .on_train_controller_assigned         = &_on_train_controller_assigned,
    .on_train_controller_released         = &_on_train_controller_released,
    .on_train_listener_changed            = &_on_train_listener_changed,
    .on_train_heartbeat_timeout           = &_on_train_heartbeat_timeout,
    .on_train_controller_assign_request   = &_on_train_controller_assign_request,
    .on_train_controller_changed_request  = &_on_train_controller_changed_request,
    .on_train_query_speeds_reply          = &_on_train_query_speeds_reply,
    .on_train_query_function_reply        = &_on_train_query_function_reply,
    .on_train_controller_assign_reply     = &_on_train_controller_assign_reply,
    .on_train_controller_query_reply      = &_on_train_controller_query_reply,
    .on_train_controller_changed_notify_reply = &_on_train_controller_changed_notify_reply,
    .on_train_listener_attach_reply       = &_on_train_listener_attach_reply,
    .on_train_listener_detach_reply       = &_on_train_listener_detach_reply,
    .on_train_listener_query_reply        = &_on_train_listener_query_reply,
    .on_train_reserve_reply               = &_on_train_reserve_reply,
    .on_train_heartbeat_request           = &_on_train_heartbeat_request,
#endif
#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)
    .on_train_search_matched                = &_on_train_search_matched,
    .on_train_search_no_match_with_allocate = &_on_train_search_no_match_with_allocate,
    .on_train_search_reply                  = &_on_train_search_reply,
#endif
#ifdef OPENLCB_COMPILE_STREAM
    .on_stream_initiate_request  = &_on_stream_initiate_request,
    .on_stream_initiate_reply    = &_on_stream_initiate_reply,
    .on_stream_data_received     = &_on_stream_data_received,
    .on_stream_data_proceed      = &_on_stream_data_proceed,
    .on_stream_complete          = &_on_stream_complete,
#endif
};

static bool _initialized = false;

// ---------------------------------------------------------------------------
// Scratch node_parameters builder
// ---------------------------------------------------------------------------

static node_parameters_t _scratch;
static uint8_t *_scratch_cdi = NULL;
static uint8_t *_scratch_fdi = NULL;

static user_address_space_info_t *_lookup_space(uint8_t space_id)
{

    switch (space_id)
    {

        case 0xFF: return &_scratch.address_space_configuration_definition;
        case 0xFE: return &_scratch.address_space_all;
        case 0xFD: return &_scratch.address_space_config_memory;
        case 0xFC: return &_scratch.address_space_acdi_manufacturer;
        case 0xFB: return &_scratch.address_space_acdi_user;
        case 0xFA: return &_scratch.address_space_train_function_definition_info;
        case 0xF9: return &_scratch.address_space_train_function_config_memory;
        case 0xEF: return &_scratch.address_space_firmware;
        default:   return NULL;
    }
}

// ---------------------------------------------------------------------------
// Exported entry points
// ---------------------------------------------------------------------------

EMSCRIPTEN_KEEPALIVE
void wasm_initialize(void)
{

    if (_initialized) { return; }
    CanConfig_initialize(&_can_config);
    OpenLcbConfig_initialize(&_openlcb_config);
    _initialized = true;
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

// ----- scratch builder -----

EMSCRIPTEN_KEEPALIVE
void wasm_node_builder_reset(void)
{

    memset(&_scratch, 0, sizeof(_scratch));
    if (_scratch_cdi != NULL) { free(_scratch_cdi); _scratch_cdi = NULL; }
    if (_scratch_fdi != NULL) { free(_scratch_fdi); _scratch_fdi = NULL; }
}

EMSCRIPTEN_KEEPALIVE
void wasm_node_set_snip(uint8_t mfg_version,
                        const char *name,
                        const char *model,
                        const char *hardware_version,
                        const char *software_version,
                        uint8_t user_version)
{

    _scratch.snip.mfg_version  = mfg_version;
    _scratch.snip.user_version = user_version;
    _copy_cstr(_scratch.snip.name,             sizeof(_scratch.snip.name),             name);
    _copy_cstr(_scratch.snip.model,            sizeof(_scratch.snip.model),            model);
    _copy_cstr(_scratch.snip.hardware_version, sizeof(_scratch.snip.hardware_version), hardware_version);
    _copy_cstr(_scratch.snip.software_version, sizeof(_scratch.snip.software_version), software_version);
}

EMSCRIPTEN_KEEPALIVE
void wasm_node_set_protocol_support(uint32_t low, uint32_t high)
{

    _scratch.protocol_support = ((uint64_t) high << 32) | (uint64_t) low;
}

EMSCRIPTEN_KEEPALIVE
void wasm_node_set_event_autocreate(uint8_t producer_count, uint8_t consumer_count)
{

    _scratch.producer_count_autocreate = producer_count;
    _scratch.consumer_count_autocreate = consumer_count;
}

/**
 *  flags is a bitfield of the configuration_options booleans:
 *    bit 0  write_under_mask_supported
 *    bit 1  unaligned_reads_supported
 *    bit 2  unaligned_writes_supported
 *    bit 3  read_from_manufacturer_space_0xfc_supported
 *    bit 4  read_from_user_space_0xfb_supported
 *    bit 5  write_to_user_space_0xfb_supported
 *    bit 6  stream_read_write_supported
 */
EMSCRIPTEN_KEEPALIVE
void wasm_node_set_configuration_options(uint32_t flags,
                                         uint8_t high_address_space,
                                         uint8_t low_address_space,
                                         const char *description)
{

    user_configuration_options_t *o = &_scratch.configuration_options;
    o->write_under_mask_supported                  = (flags & (1u << 0)) != 0;
    o->unaligned_reads_supported                   = (flags & (1u << 1)) != 0;
    o->unaligned_writes_supported                  = (flags & (1u << 2)) != 0;
    o->read_from_manufacturer_space_0xfc_supported = (flags & (1u << 3)) != 0;
    o->read_from_user_space_0xfb_supported         = (flags & (1u << 4)) != 0;
    o->write_to_user_space_0xfb_supported          = (flags & (1u << 5)) != 0;
    o->stream_read_write_supported                 = (flags & (1u << 6)) != 0;
    o->high_address_space = high_address_space;
    o->low_address_space  = low_address_space;
    _copy_cstr(o->description, sizeof(o->description), description);
}

/**
 *  flags:
 *    bit 0  present
 *    bit 1  read_only
 *    bit 2  low_address_valid
 */
EMSCRIPTEN_KEEPALIVE
int32_t wasm_node_set_address_space(uint8_t space_id,
                                    uint32_t flags,
                                    uint32_t low_address,
                                    uint32_t highest_address,
                                    const char *description)
{

    user_address_space_info_t *s = _lookup_space(space_id);
    if (s == NULL) { return WASM_ERR_INVALID_ARG; }

    s->present           = (flags & (1u << 0)) != 0;
    s->read_only         = (flags & (1u << 1)) != 0;
    s->low_address_valid = (flags & (1u << 2)) != 0;
    s->address_space     = space_id;
    s->low_address       = low_address;
    s->highest_address   = highest_address;
    _copy_cstr(s->description, sizeof(s->description), description);

    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_node_set_cdi(const uint8_t *ptr, uint32_t len)
{

    if (_scratch_cdi != NULL) { free(_scratch_cdi); _scratch_cdi = NULL; }
    if (ptr == NULL || len == 0) { return WASM_OK; }

    _scratch_cdi = (uint8_t *) malloc(len);
    if (_scratch_cdi == NULL) { return WASM_ERR_CEILING_EXCEEDED; }

    memcpy(_scratch_cdi, ptr, len);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_node_set_fdi(const uint8_t *ptr, uint32_t len)
{

    if (_scratch_fdi != NULL) { free(_scratch_fdi); _scratch_fdi = NULL; }
    if (ptr == NULL || len == 0) { return WASM_OK; }

    _scratch_fdi = (uint8_t *) malloc(len);
    if (_scratch_fdi == NULL) { return WASM_ERR_CEILING_EXCEEDED; }

    memcpy(_scratch_fdi, ptr, len);
    return WASM_OK;
}

/**
 *  Commit the scratch parameters as a new node.  The parameters struct and
 *  any staged CDI/FDI bytes are duplicated to the heap and retained for the
 *  lifetime of the WASM instance (OpenLcbConfig_create_node stores pointers,
 *  and OpenLCB has no notion of removing a node from the network).
 *
 *  After a successful commit the scratch is reset so the next node can be
 *  built without carrying over state.
 */
EMSCRIPTEN_KEEPALIVE
int32_t wasm_create_node(uint64_t node_id)
{

    if (!_initialized) { return WASM_ERR_NOT_INITIALIZED; }

    node_parameters_t *params = (node_parameters_t *) malloc(sizeof(node_parameters_t));
    if (params == NULL) { return WASM_ERR_CEILING_EXCEEDED; }
    memcpy(params, &_scratch, sizeof(node_parameters_t));

    params->cdi = _scratch_cdi;
    params->fdi = _scratch_fdi;

    openlcb_node_t *node = OpenLcbConfig_create_node(node_id, params);
    if (node == NULL)
    {

        free(params);
        if (_scratch_cdi != NULL) { free(_scratch_cdi); }
        if (_scratch_fdi != NULL) { free(_scratch_fdi); }
        _scratch_cdi = NULL;
        _scratch_fdi = NULL;
        return WASM_ERR_CEILING_EXCEEDED;
    }

    _scratch_cdi = NULL;
    _scratch_fdi = NULL;
    memset(&_scratch, 0, sizeof(_scratch));

    return WASM_OK;
}

// ----- event production -----

EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_event_pc_report(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }

    if (!OpenLcbApplication_send_event_pc_report(node, event_id))
    {

        return WASM_ERR_TX_BUSY;
    }

    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_event_with_mti(uint64_t node_id, uint64_t event_id, uint32_t mti)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_send_event_with_mti(node, event_id, (uint16_t) mti) ? WASM_OK : WASM_ERR_TX_BUSY;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_teach_event(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_send_teach_event(node, event_id) ? WASM_OK : WASM_ERR_TX_BUSY;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_initialization_event(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_send_initialization_event(node) ? WASM_OK : WASM_ERR_TX_BUSY;
}

// Send Verify Node ID Addressed to a specific remote alias.  Pass
// dest_node_id = 0 for unconditional identify; pass non-zero for
// verification (receiver replies only on match).
EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_verify_node_id_addressed(uint64_t node_id, uint32_t dest_alias, uint64_t dest_node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_send_verify_node_id_addressed(node, (uint16_t) dest_alias, dest_node_id) ? WASM_OK : WASM_ERR_TX_BUSY;
}

// Send Verify Node ID Global — every node on the bus replies, each
// firing Module.onVerifiedNodeId once.
EMSCRIPTEN_KEEPALIVE
int32_t wasm_send_verify_node_id_global(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_send_verify_node_id_global(node) ? WASM_OK : WASM_ERR_TX_BUSY;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_register_consumer_eventid(uint64_t node_id, uint64_t event_id, uint32_t status)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    uint16_t idx = OpenLcbApplication_register_consumer_eventid(node, event_id, (event_status_enum) status);
    if (idx == 0xFFFF) { return WASM_ERR_CEILING_EXCEEDED; }
    return (int32_t) idx;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_register_producer_eventid(uint64_t node_id, uint64_t event_id, uint32_t status)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    uint16_t idx = OpenLcbApplication_register_producer_eventid(node, event_id, (event_status_enum) status);
    if (idx == 0xFFFF) { return WASM_ERR_CEILING_EXCEEDED; }
    return (int32_t) idx;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_clear_consumer_eventids(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplication_clear_consumer_eventids(node);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_clear_producer_eventids(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplication_clear_producer_eventids(node);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_register_consumer_range(uint64_t node_id, uint64_t event_id_base, uint32_t range_size)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_register_consumer_range(node, event_id_base, (event_range_count_enum) range_size) ? WASM_OK : WASM_ERR_CEILING_EXCEEDED;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_register_producer_range(uint64_t node_id, uint64_t event_id_base, uint32_t range_size)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplication_register_producer_range(node, event_id_base, (event_range_count_enum) range_size) ? WASM_OK : WASM_ERR_CEILING_EXCEEDED;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_clear_consumer_ranges(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplication_clear_consumer_ranges(node);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_clear_producer_ranges(uint64_t node_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplication_clear_producer_ranges(node);
    return WASM_OK;
}

// ---------------------------------------------------------------------------
// Broadcast Time exports
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_BROADCAST_TIME

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_is_consumer(uint64_t clock_id)
{

    return OpenLcbApplicationBroadcastTime_is_consumer(clock_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_is_producer(uint64_t clock_id)
{

    return OpenLcbApplicationBroadcastTime_is_producer(clock_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void wasm_bt_start(uint64_t clock_id)
{

    OpenLcbApplicationBroadcastTime_start(clock_id);
}

EMSCRIPTEN_KEEPALIVE
void wasm_bt_stop(uint64_t clock_id)
{

    OpenLcbApplicationBroadcastTime_stop(clock_id);
}

#define _BT_SEND_NODE_CLOCK(fn_name, c_fn)                                        \
EMSCRIPTEN_KEEPALIVE                                                              \
int32_t fn_name(uint64_t node_id, uint64_t clock_id)                              \
{                                                                                 \
    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);                     \
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }                              \
    return c_fn(n, clock_id) ? WASM_OK : WASM_ERR_TX_BUSY;                        \
}

#define _BT_SEND_NODE_CLOCK_2U8(fn_name, c_fn)                                    \
EMSCRIPTEN_KEEPALIVE                                                              \
int32_t fn_name(uint64_t node_id, uint64_t clock_id, uint32_t a, uint32_t b)      \
{                                                                                 \
    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);                     \
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }                              \
    return c_fn(n, clock_id, (uint8_t) a, (uint8_t) b) ? WASM_OK : WASM_ERR_TX_BUSY; \
}

#define _BT_SEND_NODE_CLOCK_U16(fn_name, c_fn, cast_t)                            \
EMSCRIPTEN_KEEPALIVE                                                              \
int32_t fn_name(uint64_t node_id, uint64_t clock_id, int32_t v)                   \
{                                                                                 \
    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);                     \
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }                              \
    return c_fn(n, clock_id, (cast_t) v) ? WASM_OK : WASM_ERR_TX_BUSY;            \
}

_BT_SEND_NODE_CLOCK_2U8 (wasm_bt_send_report_time,  OpenLcbApplicationBroadcastTime_send_report_time)
_BT_SEND_NODE_CLOCK_2U8 (wasm_bt_send_report_date,  OpenLcbApplicationBroadcastTime_send_report_date)
_BT_SEND_NODE_CLOCK_U16 (wasm_bt_send_report_year,  OpenLcbApplicationBroadcastTime_send_report_year, uint16_t)
_BT_SEND_NODE_CLOCK_U16 (wasm_bt_send_report_rate,  OpenLcbApplicationBroadcastTime_send_report_rate, int16_t)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_start,        OpenLcbApplicationBroadcastTime_send_start)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_stop,         OpenLcbApplicationBroadcastTime_send_stop)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_date_rollover, OpenLcbApplicationBroadcastTime_send_date_rollover)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_query,        OpenLcbApplicationBroadcastTime_send_query)
_BT_SEND_NODE_CLOCK_2U8 (wasm_bt_send_query_reply,  OpenLcbApplicationBroadcastTime_send_query_reply)
_BT_SEND_NODE_CLOCK_2U8 (wasm_bt_send_set_time,     OpenLcbApplicationBroadcastTime_send_set_time)
_BT_SEND_NODE_CLOCK_2U8 (wasm_bt_send_set_date,     OpenLcbApplicationBroadcastTime_send_set_date)
_BT_SEND_NODE_CLOCK_U16 (wasm_bt_send_set_year,     OpenLcbApplicationBroadcastTime_send_set_year, uint16_t)
_BT_SEND_NODE_CLOCK_U16 (wasm_bt_send_set_rate,     OpenLcbApplicationBroadcastTime_send_set_rate, int16_t)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_command_start, OpenLcbApplicationBroadcastTime_send_command_start)
_BT_SEND_NODE_CLOCK     (wasm_bt_send_command_stop,  OpenLcbApplicationBroadcastTime_send_command_stop)

// ---------------------------------------------------------------------------
// Broadcast Time — clock lifecycle, triggers, and event-ID codec helpers.
// ---------------------------------------------------------------------------

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_setup_consumer(uint64_t node_id, uint64_t clock_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplicationBroadcastTime_setup_consumer(n, clock_id) != NULL
        ? WASM_OK
        : WASM_ERR_CEILING_EXCEEDED;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_setup_producer(uint64_t node_id, uint64_t clock_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplicationBroadcastTime_setup_producer(n, clock_id) != NULL
        ? WASM_OK
        : WASM_ERR_CEILING_EXCEEDED;
}

EMSCRIPTEN_KEEPALIVE
void wasm_bt_trigger_query_reply(uint64_t clock_id)
{

    OpenLcbApplicationBroadcastTime_trigger_query_reply(clock_id);
}

EMSCRIPTEN_KEEPALIVE
void wasm_bt_trigger_sync_delay(uint64_t clock_id)
{

    OpenLcbApplicationBroadcastTime_trigger_sync_delay(clock_id);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_make_clock_id(uint64_t unique_id_48bit)
{

    return OpenLcbApplicationBroadcastTime_make_clock_id(unique_id_48bit);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_is_time_event(uint64_t event_id)
{

    return ProtocolBroadcastTimeHandler_is_time_event(event_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_extract_clock_id(uint64_t event_id)
{

    return ProtocolBroadcastTimeHandler_extract_clock_id(event_id);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_bt_get_event_type(uint64_t event_id)
{

    return (uint32_t) ProtocolBroadcastTimeHandler_get_event_type(event_id);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_extract_time(uint64_t event_id)
{

    uint8_t hour = 0, minute = 0;
    if (!ProtocolBroadcastTimeHandler_extract_time(event_id, &hour, &minute)) { return -1; }
    return ((int32_t) hour << 8) | (int32_t) minute;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_extract_date(uint64_t event_id)
{

    uint8_t month = 0, day = 0;
    if (!ProtocolBroadcastTimeHandler_extract_date(event_id, &month, &day)) { return -1; }
    return ((int32_t) month << 8) | (int32_t) day;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_extract_year(uint64_t event_id)
{

    uint16_t year = 0;
    if (!ProtocolBroadcastTimeHandler_extract_year(event_id, &year)) { return -1; }
    return (int32_t) year;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_bt_extract_rate(uint64_t event_id, int16_t *rate_out)
{

    if (rate_out == NULL) { return 0; }
    int16_t rate = 0;
    if (!ProtocolBroadcastTimeHandler_extract_rate(event_id, &rate)) { return 0; }
    *rate_out = rate;
    return 1;
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_create_time_event_id(uint64_t clock_id, uint32_t hour, uint32_t minute, int32_t is_set)
{

    return ProtocolBroadcastTimeHandler_create_time_event_id(clock_id, (uint8_t) hour, (uint8_t) minute, is_set != 0);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_create_date_event_id(uint64_t clock_id, uint32_t month, uint32_t day, int32_t is_set)
{

    return ProtocolBroadcastTimeHandler_create_date_event_id(clock_id, (uint8_t) month, (uint8_t) day, is_set != 0);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_create_year_event_id(uint64_t clock_id, uint32_t year, int32_t is_set)
{

    return ProtocolBroadcastTimeHandler_create_year_event_id(clock_id, (uint16_t) year, is_set != 0);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_create_rate_event_id(uint64_t clock_id, int32_t rate, int32_t is_set)
{

    return ProtocolBroadcastTimeHandler_create_rate_event_id(clock_id, (int16_t) rate, is_set != 0);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_bt_create_command_event_id(uint64_t clock_id, uint32_t command_enum)
{

    return ProtocolBroadcastTimeHandler_create_command_event_id(clock_id, (broadcast_time_event_type_enum) command_enum);
}

#endif /* OPENLCB_COMPILE_BROADCAST_TIME */

// ---------------------------------------------------------------------------
// Train exports (throttle side)
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_TRAIN

#define _TRAIN_THROTTLE_SEND(fn_name, c_fn)                                       \
EMSCRIPTEN_KEEPALIVE                                                              \
int32_t fn_name(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id) \
{                                                                                 \
    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);            \
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }                              \
    c_fn(n, (uint16_t) train_alias, train_node_id);                               \
    return WASM_OK;                                                               \
}

_TRAIN_THROTTLE_SEND(wasm_train_send_emergency_stop,     OpenLcbApplicationTrain_send_emergency_stop)
_TRAIN_THROTTLE_SEND(wasm_train_send_query_speeds,       OpenLcbApplicationTrain_send_query_speeds)
_TRAIN_THROTTLE_SEND(wasm_train_send_assign_controller,  OpenLcbApplicationTrain_send_assign_controller)
_TRAIN_THROTTLE_SEND(wasm_train_send_release_controller, OpenLcbApplicationTrain_send_release_controller)
_TRAIN_THROTTLE_SEND(wasm_train_send_noop,               OpenLcbApplicationTrain_send_noop)

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_set_speed(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, int32_t speed_f16)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_set_speed(n, (uint16_t) train_alias, train_node_id, (uint16_t) speed_f16);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_set_function(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint32_t fn_address, int32_t fn_value)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_set_function(n, (uint16_t) train_alias, train_node_id, fn_address, (uint16_t) fn_value);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_query_function(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint32_t fn_address)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_query_function(n, (uint16_t) train_alias, train_node_id, fn_address);
    return WASM_OK;
}

// Allocate train_state for a node and register its standard event IDs.
// Must be called once after wasm_create_node() for any node that wants to
// behave as a train (i.e. the node itself is a DCC locomotive, not a
// throttle).  Returns -2 if the pool is full, -3 if the node is unknown.
EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_setup(uint64_t node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    train_state_t *st = OpenLcbApplicationTrain_setup(n);
    if (st == NULL) { return WASM_ERR_CEILING_EXCEEDED; }
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_set_dcc_address(uint64_t node_id, uint32_t dcc_address, int32_t is_long)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_set_dcc_address(n, (uint16_t) dcc_address, is_long != 0);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_get_dcc_address(uint64_t node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return (int32_t) OpenLcbApplicationTrain_get_dcc_address(n);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_is_long_address(uint64_t node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return OpenLcbApplicationTrain_is_long_address(n) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_set_speed_steps(uint64_t node_id, uint32_t speed_steps)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_set_speed_steps(n, (uint8_t) speed_steps);
    return WASM_OK;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_get_speed_steps(uint64_t node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    return (int32_t) OpenLcbApplicationTrain_get_speed_steps(n);
}

// --- Additional throttle senders (3-arg shape, covered by macro) ------------

_TRAIN_THROTTLE_SEND(wasm_train_send_query_controller,  OpenLcbApplicationTrain_send_query_controller)
_TRAIN_THROTTLE_SEND(wasm_train_send_reserve,           OpenLcbApplicationTrain_send_reserve)
_TRAIN_THROTTLE_SEND(wasm_train_send_release_reserve,   OpenLcbApplicationTrain_send_release_reserve)

// --- 4-arg: (throttle, alias, train, new_controller_node_id) ---------------

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_controller_changing_notify(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint64_t new_controller_node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_controller_changing_notify(n, (uint16_t) train_alias, train_node_id, new_controller_node_id);
    return WASM_OK;
}

// --- 4-arg: (throttle, alias, train, listener_node_id) ---------------------

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_listener_detach(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint64_t listener_node_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_listener_detach(n, (uint16_t) train_alias, train_node_id, listener_node_id);
    return WASM_OK;
}

// --- 5-arg: (throttle, alias, train, listener_node_id, flags) --------------

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_listener_attach(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint64_t listener_node_id, uint32_t flags)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_listener_attach(n, (uint16_t) train_alias, train_node_id, listener_node_id, (uint8_t) flags);
    return WASM_OK;
}

// --- 4-arg: (throttle, alias, train, listener_index) -----------------------

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_listener_query(uint64_t throttle_node_id, uint32_t train_alias, uint64_t train_node_id, uint32_t listener_index)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(throttle_node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_listener_query(n, (uint16_t) train_alias, train_node_id, (uint8_t) listener_index);
    return WASM_OK;
}

#endif /* OPENLCB_COMPILE_TRAIN */

// ---------------------------------------------------------------------------
// DCC detector helpers (pure functions — no state, no node lookup)
// ---------------------------------------------------------------------------

#ifdef OPENLCB_COMPILE_DCC_DETECTOR

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_dcc_encode_event_id(uint64_t detector_node_id, uint32_t direction, uint32_t raw_address_14)
{

    return OpenLcbApplicationDccDetector_encode_event_id(
        detector_node_id,
        (dcc_detector_direction_enum) direction,
        (uint16_t) raw_address_14);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_make_short_address(uint32_t short_address)
{

    return (uint32_t) OpenLcbApplicationDccDetector_make_short_address((uint8_t) short_address);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_make_consist_address(uint32_t consist_address)
{

    return (uint32_t) OpenLcbApplicationDccDetector_make_consist_address((uint8_t) consist_address);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_extract_direction(uint64_t event_id)
{

    return (uint32_t) OpenLcbApplicationDccDetector_extract_direction(event_id);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_extract_address_type(uint64_t event_id)
{

    return (uint32_t) OpenLcbApplicationDccDetector_extract_address_type(event_id);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_extract_raw_address(uint64_t event_id)
{

    return (uint32_t) OpenLcbApplicationDccDetector_extract_raw_address(event_id);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_dcc_extract_dcc_address(uint64_t event_id)
{

    return (uint32_t) OpenLcbApplicationDccDetector_extract_dcc_address(event_id);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_dcc_extract_detector_id(uint64_t event_id)
{

    return OpenLcbApplicationDccDetector_extract_detector_id(event_id);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_dcc_is_track_empty(uint64_t event_id)
{

    return OpenLcbApplicationDccDetector_is_track_empty(event_id) ? 1 : 0;
}

#endif /* OPENLCB_COMPILE_DCC_DETECTOR */

// ---------------------------------------------------------------------------
// Utility helpers — pure node/event queries and the event-range ID builder.
// All node-scoped queries return -1 for an unknown node_id.
// ---------------------------------------------------------------------------

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_util_generate_event_range_id(uint64_t base_event_id, uint32_t count_enum)
{

    return OpenLcbUtilities_generate_event_range_id(base_event_id, (event_range_count_enum) count_enum);
}

// Look up the CAN alias assigned to a given 48-bit node ID.  Returns 0 if
// the mapping is unknown (remote node we haven't heard from) or not yet
// permitted (own node still logging in).  Used by throttle apps to address
// remote trains after a Producer Identified reply.
EMSCRIPTEN_KEEPALIVE
uint32_t wasm_util_alias_for_node_id(uint64_t node_id)
{

    alias_mapping_t *m = InternalNodeAliasTable_find_mapping_by_node_id(node_id);
    if (m == NULL) { return 0; }
    return (uint32_t) m->alias;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_util_is_producer_event_assigned(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return -1; }
    uint16_t index = 0;
    if (OpenLcbUtilities_is_producer_event_assigned_to_node(node, event_id, &index)) {
        return (int32_t) index;
    }
    return -1;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_util_is_consumer_event_assigned(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return -1; }
    uint16_t index = 0;
    if (OpenLcbUtilities_is_consumer_event_assigned_to_node(node, event_id, &index)) {
        return (int32_t) index;
    }
    return -1;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_util_is_event_in_producer_ranges(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return -1; }
    return OpenLcbUtilities_is_event_id_in_producer_ranges(node, event_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_util_is_event_in_consumer_ranges(uint64_t node_id, uint64_t event_id)
{

    openlcb_node_t *node = OpenLcbNode_find_by_node_id(node_id);
    if (node == NULL) { return -1; }
    return OpenLcbUtilities_is_event_id_in_consumer_ranges(node, event_id) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Float16 helpers — always-on.  Pure bit-twiddling math, no protocol state.
// JS wrapper calls these directly; no compile flag gates the family.
// ---------------------------------------------------------------------------

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_float16_from_float(float value)
{

    return (uint32_t) OpenLcbFloat16_from_float(value);
}

EMSCRIPTEN_KEEPALIVE
float wasm_float16_to_float(uint32_t half)
{

    return OpenLcbFloat16_to_float((uint16_t) half);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_float16_negate(uint32_t half)
{

    return (uint32_t) OpenLcbFloat16_negate((uint16_t) half);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_float16_is_nan(uint32_t half)
{

    return OpenLcbFloat16_is_nan((uint16_t) half) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_float16_is_zero(uint32_t half)
{

    return OpenLcbFloat16_is_zero((uint16_t) half) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_float16_speed_with_direction(float speed, int32_t reverse)
{

    return (uint32_t) OpenLcbFloat16_speed_with_direction(speed, reverse != 0);
}

EMSCRIPTEN_KEEPALIVE
float wasm_float16_get_speed(uint32_t half)
{

    return OpenLcbFloat16_get_speed((uint16_t) half);
}

EMSCRIPTEN_KEEPALIVE
int32_t wasm_float16_get_direction(uint32_t half)
{

    return OpenLcbFloat16_get_direction((uint16_t) half) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Train-search helpers — gated by TRAIN + TRAIN_SEARCH.
// ---------------------------------------------------------------------------

#if defined(OPENLCB_COMPILE_TRAIN) && defined(OPENLCB_COMPILE_TRAIN_SEARCH)

EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_search_is_search_event(uint64_t event_id)
{

    return ProtocolTrainSearchHandler_is_search_event(event_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_train_search_extract_flags(uint64_t event_id)
{

    return (uint32_t) ProtocolTrainSearchHandler_extract_flags(event_id);
}

EMSCRIPTEN_KEEPALIVE
void wasm_train_search_extract_digits(uint64_t event_id, uint8_t *digits_out)
{

    if (digits_out == NULL) { return; }
    ProtocolTrainSearchHandler_extract_digits(event_id, digits_out);
}

EMSCRIPTEN_KEEPALIVE
uint32_t wasm_train_search_digits_to_address(const uint8_t *digits_in)
{

    if (digits_in == NULL) { return 0; }
    return (uint32_t) ProtocolTrainSearchHandler_digits_to_address(digits_in);
}

EMSCRIPTEN_KEEPALIVE
uint64_t wasm_train_search_create_event_id(uint32_t address, uint32_t flags)
{

    return ProtocolTrainSearchHandler_create_event_id((uint16_t) address, (uint8_t) flags);
}

// Send a Producer Identified for a train-search event this node matched.
// Used by a Command Station after creating a train in response to a search
// with the Allocate flag — it publishes the match so the throttle can
// proceed to assign.
EMSCRIPTEN_KEEPALIVE
int32_t wasm_train_send_search_match(uint64_t node_id, uint64_t search_event_id)
{

    openlcb_node_t *n = OpenLcbNode_find_by_node_id(node_id);
    if (n == NULL) { return WASM_ERR_UNKNOWN_NODE; }
    OpenLcbApplicationTrain_send_search_match(n, search_event_id);
    return WASM_OK;
}

#endif /* OPENLCB_COMPILE_TRAIN && OPENLCB_COMPILE_TRAIN_SEARCH */
