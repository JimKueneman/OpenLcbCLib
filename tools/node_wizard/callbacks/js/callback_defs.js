/* =========================================================================
 * Callback Definitions — defines the application callback stubs per group.
 * Extracted from src/openlcb/openlcb_config.h (openlcb_config_t struct).
 *
 * Each callback maps to a function pointer in the openlcb_config_t struct.
 * The configField property matches the struct member name.
 * ========================================================================= */

const CALLBACK_GROUPS = {

    'cb-can': {

        title: 'CAN Callbacks',
        description: 'Notifications for CAN bus activity',
        groupDetail: 'Raw CAN bus frame notifications that fire before the protocol stack processes each frame. on_rx and on_tx are useful for activity LEDs, GridConnect frame logging, and bus traffic monitoring. on_alias_change fires during the alias allocation sequence (CID/RID/AMD frames) when the library assigns a 12-bit alias to a node — helpful for correlating raw CAN traffic with Node IDs during debugging. Most applications only need these callbacks for diagnostics.',
        filePrefix: 'callbacks_can',
        headerGuard: '__CALLBACKS_CAN__',
        includes: [
            '#include "src/drivers/canbus/can_types.h"',
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksCan',
        diStruct: 'can_config_t',
        functions: [

            {
                name: 'on_rx',
                returnType: 'void',
                params: 'can_msg_t *can_msg',
                description: 'Called when a CAN frame is received',
                detail: 'Fires for every CAN frame the library receives, before protocol processing. Useful for blinking an activity LED, logging raw frames in GridConnect format for debugging, or counting traffic. The can_msg contains the 29-bit extended identifier and 0–8 payload bytes. Do not modify the message — it is shared with the protocol handlers. All OpenLCB CAN frames use the identifier field to encode source alias, frame type, and message type. See CAN Frame Transfer Standard for physical encoding details.',
                required: false,
                configField: 'on_rx'
            },

            {
                name: 'on_tx',
                returnType: 'void',
                params: 'can_msg_t *can_msg',
                description: 'Called when a CAN frame is transmitted',
                detail: 'Fires after the library successfully queues a CAN frame for transmission. Useful for activity LEDs and debug logging. The can_msg contains the frame that was sent. Note that this fires when the frame enters the transmit queue — it does not guarantee the frame has left the physical bus (CAN arbitration may delay it).',
                required: false,
                configField: 'on_tx'
            },

            {
                name: 'on_alias_change',
                returnType: 'void',
                params: 'uint16_t alias, node_id_t node_id',
                description: 'Called when a node\'s CAN alias changes',
                detail: 'Fires when the library allocates or changes the 12-bit CAN alias for a node. The alias occupies bits 0–11 of the CAN identifier and represents the full 48-bit Node ID on the wire. Alias allocation uses a sequence of Check ID (CID) and Reserve ID (RID) frames followed by Alias Map Definition (AMD). Useful for diagnostic logging — print the alias and Node ID so you can correlate CAN traffic during debugging.',
                required: false,
                configField: 'on_alias_change'
            }

        ]

    },

    'cb-olcb': {

        title: 'OLCB/LCC Callbacks',
        description: 'Core application notifications',
        groupDetail: 'Core protocol lifecycle callbacks. on_100ms_timer is the periodic heartbeat tick that drives application tasks (LED blink, sensor reads, state machines) and all internal protocol timing — heartbeat countdowns, broadcast time accumulation, and alias allocation delays. on_login_complete gates node startup so you can defer initialization until hardware is ready. The error callbacks (on_optional_interaction_rejected, on_terminate_due_to_error) fire when a remote node rejects a message your node sent, useful for logging and graceful error handling.',
        filePrefix: 'callbacks_olcb',
        headerGuard: '__CALLBACKS_OLCB__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksOlcb',
        functions: [

            {
                name: 'on_optional_interaction_rejected',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, node_id_t source_node_id, uint16_t error_code, uint16_t rejected_mti',
                description: 'A remote node does not support a message you sent',
                detail: 'Fires when a remote node replies with Optional Interaction Rejected. This means the remote node received your message but does not implement that protocol. The error_code and rejected_mti identify what was rejected. Error codes in the 0x1000 range are permanent errors; 0x2000 range are temporary. Useful for logging or gracefully handling unsupported features on the network. See Message Network Standard for error code definitions.',
                required: false,
                configField: 'on_optional_interaction_rejected'
            },

            {
                name: 'on_terminate_due_to_error',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, node_id_t source_node_id, uint16_t error_code, uint16_t rejected_mti',
                description: 'A remote node reported an error in response to your message',
                detail: 'Fires when a remote node replies with Terminate Due To Error. This is more severe than Optional Interaction Rejected — something went wrong processing your message. The error_code indicates severity: 0x1000 series = permanent errors, 0x2000 series = temporary/retryable. Common causes include sending a message to a node that is resetting or has an internal error. See Message Network Standard for error code definitions.',
                required: false,
                configField: 'on_terminate_due_to_error'
            },

            {
                name: 'on_100ms_timer',
                returnType: 'void',
                params: 'void',
                description: 'Called every 100ms for periodic application tasks',
                detail: 'Fires once per 100 ms timer tick. Use this for periodic application tasks: blinking status LEDs, reading sensor inputs, updating display counters, or driving application state machines. Keep this callback fast since it runs in the main processing loop. The 100 ms timebase drives all internal protocol timing including heartbeat countdowns, broadcast time accumulation, and alias allocation delays. For hardware timers, this may be called from interrupt context on some platforms.',
                required: false,
                configField: 'on_100ms_timer'
            },

            {
                name: 'on_login_complete',
                returnType: 'bool',
                params: 'openlcb_node_t *openlcb_node',
                description: 'Called when a node is ready to run. Return true to start, false to wait',
                detail: 'Fires when a node finishes CAN alias allocation and is about to enter the running state. At this point the node has completed alias verification and is ready to send Initialization Complete. Return true to allow the node to start operating, or false to delay startup (the library will call again on the next cycle). Use this to perform post-login initialization: start broadcast time consumers, send initial event queries, or verify that required hardware is ready before the node begins accepting commands.',
                required: false,
                configField: 'on_login_complete'
            }

        ]

    },

    'cb-events': {

        title: 'Event Callbacks',
        description: 'Notifications for event activity on the bus',
        groupDetail: 'OpenLCB events are the primary way nodes communicate state changes across the layout. on_consumed_event_pcer is the main callback — it fires when an event this node registered as a consumer arrives on the bus, letting you toggle a relay, change a signal aspect, or drive any output. on_consumed_event_identified fires during network enumeration so you can synchronize your node\'s state at startup based on what producers are reporting. on_event_learn handles the teach/learn protocol for storing new Event IDs into configuration memory. The Advanced section adds unfiltered bus-wide callbacks and individual producer/consumer identification callbacks for network discovery applications.',
        filePrefix: 'callbacks_events',
        headerGuard: '__CALLBACKS_EVENTS__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksEvents',
        functions: [

            /* --- Common --- */

            {
                name: 'on_consumed_event_pcer',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_payload_t *payload',
                description: 'An event this node consumes was reported',
                detail: 'Fires when an event that this node registered as a consumer is reported on the bus via a Producer/Consumer Event Report. The index tells you which of your registered consumer events matched. This is the primary way to react to events — toggle a relay, change a signal aspect, update a display. The library has already matched the Event ID to your consumer list. The payload parameter carries any additional data beyond the 8-byte Event ID.',
                required: false,
                configField: 'on_consumed_event_pcer'
            },

            {
                name: 'on_consumed_event_identified',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint16_t index, event_id_t *event_id, event_status_enum status, event_payload_t *payload',
                description: 'An event this node consumes was identified on the network',
                detail: 'Fires during event identification when a producer announces the current state (set, clear, or unknown) of an event this node consumes via Producer Identified messages. Use this to synchronize your node state with the network at startup — for example, set initial LED or relay states based on the reported producer state. This fires as part of event enumeration initiated by Identify Events requests.',
                required: false,
                configField: 'on_consumed_event_identified'
            },

            {
                name: 'on_event_learn',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'Event teach/learn message received',
                detail: 'Fires when a Learn Event message is received on the bus. This is part of the event teaching protocol where a producer sends a Learn message and consumers in "learn mode" capture the Event ID. Use this to store the learned Event ID into your configuration memory so the node will react to that event in the future. See Event Transport Standard for the teach/learn protocol.',
                required: false,
                configField: 'on_event_learn'
            },

            /* --- Advanced --- */

            {
                name: 'on_pc_event_report',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'Any event was reported on the bus (unfiltered)',
                detail: 'Fires for every Producer/Consumer Event Report message on the bus, regardless of whether this node consumes the event. This is an unfiltered bus-wide notification. Useful for event logging, displays that show all network activity, or nodes that need to react to events they did not pre-register as consumers. Use on_consumed_event_pcer for normal filtered event handling instead.',
                required: false,
                configField: 'on_pc_event_report',
                advanced: true
            },

            {
                name: 'on_pc_event_report_with_payload',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id, uint16_t count, event_payload_t *payload',
                description: 'Any event with payload was reported on the bus (unfiltered)',
                detail: 'Same as on_pc_event_report but for events that carry additional payload bytes beyond the 8-byte Event ID. The count indicates how many payload bytes are present. Some protocols embed extra data in event reports for efficiency, such as sensor readings or train speed values.',
                required: false,
                configField: 'on_pc_event_report_with_payload',
                advanced: true
            },

            {
                name: 'on_consumer_range_identified',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A consumer range was identified on the network',
                detail: 'Fires when a Consumer Range Identified message is received. This indicates a remote node consumes a range of sequential Event IDs rather than individual events. The event_id encodes the range using masked bits. Useful for advanced routing or discovery applications. See Event Transport Standard for range encoding details.',
                required: false,
                configField: 'on_consumer_range_identified',
                advanced: true
            },

            {
                name: 'on_consumer_identified_unknown',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A consumer reported unknown state for an event',
                detail: 'Fires when a Consumer Identified Unknown message is received. The consumer does not know the current state of this event — neither set nor clear. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_consumer_identified_unknown',
                advanced: true
            },

            {
                name: 'on_consumer_identified_set',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A consumer reported set (active) state for an event',
                detail: 'Fires when a Consumer Identified Set message is received. The consumer reports that this event is currently in the active/set state. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_consumer_identified_set',
                advanced: true
            },

            {
                name: 'on_consumer_identified_clear',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A consumer reported clear (inactive) state for an event',
                detail: 'Fires when a Consumer Identified Clear message is received. The consumer reports that this event is currently in the inactive/clear state. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_consumer_identified_clear',
                advanced: true
            },

            {
                name: 'on_producer_range_identified',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A producer range was identified on the network',
                detail: 'Fires when a Producer Range Identified message is received. This indicates a remote node produces a range of sequential Event IDs. The event_id encodes the range using masked bits. Useful for advanced routing or discovery applications.',
                required: false,
                configField: 'on_producer_range_identified',
                advanced: true
            },

            {
                name: 'on_producer_identified_unknown',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A producer reported unknown state for an event',
                detail: 'Fires when a Producer Identified Unknown message is received. The producer cannot determine the current state of this event. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_producer_identified_unknown',
                advanced: true
            },

            {
                name: 'on_producer_identified_set',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A producer reported set (active) state for an event',
                detail: 'Fires when a Producer Identified Set message is received. The producer reports that this event is currently in the active/set state. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_producer_identified_set',
                advanced: true
            },

            {
                name: 'on_producer_identified_clear',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, event_id_t *event_id',
                description: 'A producer reported clear (inactive) state for an event',
                detail: 'Fires when a Producer Identified Clear message is received. The producer reports that this event is currently in the inactive/clear state. This is an advanced callback for detailed event status discovery during network enumeration.',
                required: false,
                configField: 'on_producer_identified_clear',
                advanced: true
            }

            /* Intentionally omitted (by design):
             *   on_consumer_identified_reserved
             *   on_producer_identified_reserved
             * These exist in openlcb_config_t but are not exposed in the
             * wizard because the "reserved" state is not useful to
             * application code. */

        ]

    },

    'cb-config-mem': {

        title: 'Config Memory Callbacks',
        description: 'Notifications for configuration memory operations',
        groupDetail: 'Called when a configuration tool reads or writes your node\'s persistent storage over the bus (Memory Space 0xFD). factory_reset and reboot are the most commonly needed — factory_reset erases EEPROM/Flash and restores defaults, reboot triggers a hardware reset. The delayed-reply callbacks (config_mem_read_delayed_reply_time, config_mem_write_delayed_reply_time) let you tell the requesting tool how long a slow storage operation will take, so it sends a Reply Pending acknowledgment and waits rather than timing out. Reads and writes arrive as datagram commands of up to 64 bytes per transaction.',
        filePrefix: 'callbacks_config_mem',
        headerGuard: '__CALLBACKS_CONFIG_MEM__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksConfigMem',
        functions: [

            {
                name: 'factory_reset',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Erase user configuration and restore factory defaults (highly recommended)',
                detail: 'Fires when a configuration tool sends a Factory Reset command. Clear your persistent storage (EEPROM, Flash) and write default values back from your CDI-defined defaults. The protocol includes a safety guard — the requesting node must send your exact Node ID in the command payload, so accidental resets are unlikely. After restoring defaults, the node should reboot. See Memory Configuration Standard for the full reset protocol.',
                required: false,
                configField: 'factory_reset'
            },

            {
                name: 'config_mem_read_delayed_reply_time',
                returnType: 'uint16_t',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info',
                description: 'Return how long a configuration read will take to complete',
                detail: 'Fires before a configuration memory read operation. Return 0 if the read will complete immediately (most EEPROM/Flash reads). If your storage is slow (e.g. reading over a network), return (0x80 | N) where N indicates the read will take up to 2^N seconds. The library uses this to send a Reply Pending response so the requesting tool knows to wait. See Memory Configuration Standard for timing semantics.',
                required: false,
                configField: 'config_mem_read_delayed_reply_time'
            },

            {
                name: 'config_mem_write_delayed_reply_time',
                returnType: 'uint16_t',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info',
                description: 'Return how long a configuration write will take to complete',
                detail: 'Same as the read version but for write operations. Flash storage with erase cycles may need longer. Return 0 for immediate writes (most EEPROM), or (0x80 | N) for delayed completion. The library will send a Reply Pending response if needed, then the actual write reply once the operation completes. See Memory Configuration Standard for write semantics.',
                required: false,
                configField: 'config_mem_write_delayed_reply_time'
            },

            {
                name: 'reboot',
                returnType: 'void',
                params: 'void',
                description: 'Perform a software reset of the node',
                detail: 'Fires when a configuration tool sends a Reset/Reboot command. Implement this to restart your microcontroller — typically by calling the platform reset function (e.g. NVIC_SystemReset on ARM, ESP.restart on ESP32, or a watchdog-triggered reset). The library calls this after completing any pending configuration writes. If this callback is NULL, the reboot command is acknowledged but no reset occurs.',
                required: false,
                configField: 'reboot'
            }

        ]

    },

    'cb-firmware': {

        title: 'Firmware Callbacks',
        description: 'Notifications for over-the-network firmware upgrades',
        groupDetail: 'Called during the OpenLCB Firmware Upgrade protocol sequence (Memory Space 0xEF). A configuration tool first sends a Freeze command (your node stops normal operation and prepares Flash for writing), then streams the firmware image in 64-byte blocks via datagram Write commands, then sends Unfreeze to trigger a reboot into the new image. freeze and unfreeze manage the state transitions; firmware_write handles each incoming data block and must handle Flash page erase and alignment for your specific platform. Requires Config Memory and Datagram support to be active.',
        filePrefix: 'callbacks_firmware',
        headerGuard: '__CALLBACKS_FIRMWARE__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksFirmware',
        functions: [

            {
                name: 'freeze',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Prepare the node for a firmware upgrade',
                detail: 'Fires when a configuration tool sends a Freeze command before uploading new firmware. Use this to stop normal application processing, disable outputs, and prepare your flash memory for writing. The node should enter a state where it is safe to erase and reprogram flash sectors. After freeze, the tool will send firmware data via memory write commands to address space 0xEF.',
                required: false,
                configField: 'freeze'
            },

            {
                name: 'unfreeze',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Resume normal operation after a firmware upgrade',
                detail: 'Fires when a configuration tool sends an Unfreeze command after the firmware upload is complete. Use this to finalize the flash write (verify checksums, mark the new image as valid), then reboot into the new firmware. If the upgrade failed, restore the previous firmware image if possible.',
                required: false,
                configField: 'unfreeze'
            },

            {
                name: 'firmware_write',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result',
                description: 'Write a block of firmware data to flash',
                detail: 'Fires for each firmware data block received during an upgrade. The write_request_info contains the target address and data buffer. Write the data to your flash memory at the specified offset. The address space is 0xEF (firmware). Handle flash page alignment and erase as needed for your platform. Call write_result when finished to send a write-OK or write-error reply.',
                required: false,
                configField: 'firmware_write'
            }

        ]

    },

    'cb-train': {

        title: 'Train Callbacks',
        description: 'Notifications for train control activity',
        groupDetail: 'Divided into four sub-sections. Train Node \u2014 Notifications fire on a locomotive node when a throttle sends speed, function, or emergency commands, and also cover controller assignment, consist (listener) changes, and heartbeat timeout (the safety mechanism that stops the train if the throttle goes silent). Train Node \u2014 Decisions let your application approve or reject controller takeover requests. Throttle \u2014 Replies fire on a controller node when a locomotive responds to speed queries, function queries, reserve, and listener commands \u2014 use these to update your throttle display. Train Search handles address-based locomotive discovery, including dynamic creation of new train nodes on demand.',
        filePrefix: 'callbacks_train',
        headerGuard: '__CALLBACKS_TRAIN__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksTrain',
        functions: [

            /* ---- Train Node — Notifications ---- */

            {
                name: 'on_speed_changed',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint16_t speed_float16',
                description: 'Speed was set on this train node',
                detail: 'Fires after a throttle sends a Set Speed command and the library has updated the internal train state. The speed_float16 is an IEEE half-precision float encoding both speed and direction — the sign bit indicates direction (positive = forward), magnitude indicates speed (0.0 = stop, 1.0 = full). Use this to drive your motor controller or DCC output — convert the float16 to a PWM duty cycle or DCC speed step. See Train Control Standard for float16 encoding.',
                required: false,
                configField: 'on_train_speed_changed',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_function_changed',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value',
                description: 'A function was set on this train node',
                detail: 'Fires after a throttle sends a Set Function command. The fn_address is the function number (0 = headlight, 1 = bell, 2 = horn, etc.) and fn_value is the new state (0 = off, non-zero = on/value). Function addresses follow the DCC convention but extend to 32-bit addressing. Use this to control DCC function outputs, sound triggers, or GPIO pins on your hardware. The library stores function states in the internal train_state.',
                required: false,
                configField: 'on_train_function_changed',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_emergency_entered',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type',
                description: 'Train entered an emergency state (e-stop)',
                detail: 'Fires when the train enters an emergency stop or emergency off state. The emergency_type distinguishes between local e-stop (single train), global e-stop (all trains via well-known Event ID), and global emergency off (all power cut). The library has already set speed to zero and cleared the heartbeat timer. Use this to immediately cut motor power, apply brakes, or disable outputs.',
                required: false,
                configField: 'on_train_emergency_entered',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_emergency_exited',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, train_emergency_type_enum emergency_type',
                description: 'Train exited an emergency state',
                detail: 'Fires when the emergency state is cleared. The train speed is still zero — the throttle must send a new speed command to resume movement. Use this to re-enable motor drive circuitry if you disabled it during the emergency.',
                required: false,
                configField: 'on_train_emergency_exited',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_controller_assigned',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, node_id_t controller_node_id',
                description: 'A throttle took control of this train',
                detail: 'Fires after a throttle successfully takes control via Traction Controller Assign. The controller_node_id identifies the new controlling throttle. The heartbeat timer starts automatically — the controller must periodically send commands or respond to heartbeat requests to prove it is still active. The heartbeat counter is initialized based on the negotiated timeout value.',
                required: false,
                configField: 'on_train_controller_assigned',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_controller_released',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node',
                description: 'The throttle released control of this train',
                detail: 'Fires after the controller voluntarily releases this train via Traction Controller Release. The heartbeat timer stops and the controller_node_id is cleared in the internal train state. The train continues at its current speed until another controller takes over or the node is stopped by other means.',
                required: false,
                configField: 'on_train_controller_released',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_listener_changed',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node',
                description: 'A listener was attached or detached from this train',
                detail: 'Fires when a listener is attached or detached via Traction Listener Attach/Detach commands. Listeners receive copies of speed and function commands sent to this train. This is used for consist operations — multiple trains moving together where the lead locomotive forwards commands to consist members. Use this to update a consist member display if your hardware has one. See Train Control Standard for consist semantics.',
                required: false,
                configField: 'on_train_listener_changed',
                section: 'Train Node \u2014 Notifications'
            },

            {
                name: 'on_heartbeat_timeout',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node',
                description: 'Heartbeat timed out — train has been emergency stopped',
                detail: 'Fires when the controlling throttle fails to respond within the heartbeat deadline. The heartbeat counter is decremented each 100 ms tick; when it reaches zero, this fires. The library has already set speed to zero, activated e-stop, and notified all listeners via consist notification messages. Use this to apply physical brakes or cut motor power on your hardware. The heartbeat mechanism ensures that if a throttle crashes or loses connection, the train stops safely rather than running away.',
                required: false,
                configField: 'on_train_heartbeat_timeout',
                section: 'Train Node \u2014 Notifications'
            },

            /* ---- Train Node — Decisions ---- */

            {
                name: 'on_controller_assign_request',
                returnType: 'bool',
                params: 'openlcb_node_t *openlcb_node, node_id_t current_controller, node_id_t requesting_controller',
                description: 'Another throttle wants to take over. Return true to allow',
                detail: 'Fires when a second throttle tries to take control while another throttle already has control. Return true to allow the takeover (the current controller will be notified via Controller Changed Notify), or false to reject the request. If this callback is NULL, the library defaults to accepting takeover requests. You could use a physical button or switch to let the user decide whether to permit control stealing.',
                required: false,
                configField: 'on_train_controller_assign_request',
                section: 'Train Node \u2014 Decisions'
            },

            {
                name: 'on_controller_changed_request',
                returnType: 'bool',
                params: 'openlcb_node_t *openlcb_node, node_id_t new_controller',
                description: 'Controller change was requested. Return true to allow',
                detail: 'Fires when a Controller Changed Notify message is received. This happens when the current controller is transferring control to a different throttle. The new_controller identifies the throttle that wishes to take over. Return true to accept the new controller, false to reject. If NULL, the library defaults to accepting.',
                required: false,
                configField: 'on_train_controller_changed_request',
                section: 'Train Node \u2014 Decisions'
            },

            /* ---- Throttle — Replies ---- */

            {
                name: 'on_query_speeds_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint16_t set_speed, uint8_t status, uint16_t commanded_speed, uint16_t actual_speed',
                description: 'Speed query reply received from a train',
                detail: 'Fires when a train responds to your Query Speeds command. Returns three speed values as float16: set_speed (last commanded), commanded_speed (after any limits applied by the train), and actual_speed (measured feedback, if available). The status byte contains flags for e-stop state and validity of each speed type. Use this to update your throttle display with the real train state.',
                required: false,
                configField: 'on_train_query_speeds_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_query_function_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint32_t fn_address, uint16_t fn_value',
                description: 'Function query reply received from a train',
                detail: 'Fires when a train responds to your Query Function command. Returns the current state of the requested function (fn_address and fn_value). Use this to synchronize your throttle function buttons with the actual train state — for example, illuminate the headlight button if function 0 is active.',
                required: false,
                configField: 'on_train_query_function_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_controller_assign_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint8_t result, node_id_t current_controller',
                description: 'Controller assign reply received (0 = success)',
                detail: 'Fires when a train responds to your Controller Assign command. If result is 0, you now have control and can send speed/function commands. If non-zero, another controller rejected the takeover and current_controller identifies who currently has control. Update your throttle UI accordingly — show "assigned" or "rejected" status.',
                required: false,
                configField: 'on_train_controller_assign_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_controller_query_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint8_t flags, node_id_t controller_node_id',
                description: 'Controller query reply received',
                detail: 'Fires when a train responds to your Controller Query command. Returns the Node ID of the current controller (or zero if no controller is assigned) and flags indicating controller status. Use this to check if a train is free before attempting to take control.',
                required: false,
                configField: 'on_train_controller_query_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_controller_changed_notify_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint8_t result',
                description: 'Controller changed notification reply received',
                detail: 'Fires when a train responds to your Controller Changed Notify command. Indicates whether the controller transfer was accepted (result 0) or rejected. This is used when transferring control from one throttle to another as part of consist or multi-throttle operations.',
                required: false,
                configField: 'on_train_controller_changed_notify_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_listener_attach_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result',
                description: 'Listener attach reply received',
                detail: 'Fires when a train responds to your Listener Attach command. Confirms that your node is now receiving copies of speed and function commands sent to that train. Result 0 means success; non-zero means the attach was rejected or the train is at maximum listener capacity. Used when building consists — attach your train as a listener on the lead locomotive.',
                required: false,
                configField: 'on_train_listener_attach_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_listener_detach_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, node_id_t node_id, uint8_t result',
                description: 'Listener detach reply received',
                detail: 'Fires when a train responds to your Listener Detach command. Confirms your node is no longer receiving forwarded commands from that train. Used when breaking up consists — detach your train from the lead locomotive to restore independent control.',
                required: false,
                configField: 'on_train_listener_detach_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_listener_query_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint8_t count, uint8_t index, uint8_t flags, node_id_t node_id',
                description: 'Listener query reply received',
                detail: 'Fires for each listener in the response to a Listener Query command. The count is the total number of listeners, index is which one this reply is for (0 to count\u20131), and node_id identifies the listener. May fire multiple times if multiple listeners are attached. Use this to display consist membership on your throttle UI.',
                required: false,
                configField: 'on_train_listener_query_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_reserve_reply',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint8_t result',
                description: 'Reserve reply received (0 = success)',
                detail: 'Fires when a train responds to your Reserve command. Reserve provides temporary exclusive access to the train (prevents other throttles from taking control). Result 0 means the reservation was granted; non-zero indicates failure (e.g. already reserved by another throttle). Use this before performing multi-step operations that should not be interrupted.',
                required: false,
                configField: 'on_train_reserve_reply',
                section: 'Throttle \u2014 Replies'
            },

            {
                name: 'on_heartbeat_request',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint32_t timeout_seconds',
                description: 'Train is requesting a heartbeat response before the deadline',
                detail: 'Fires when a train you are controlling sends a heartbeat request. You must send a command (speed, function, or NOOP) before timeout_seconds expires, or the train will emergency stop. The train sends heartbeat requests when the countdown reaches 50% of the negotiated timeout. Set up a timer or use on_100ms_timer to send periodic NOOP commands to keep the heartbeat alive. See Train Control Standard for heartbeat timing.',
                required: false,
                configField: 'on_train_heartbeat_request',
                section: 'Throttle \u2014 Replies'
            },

            /* ---- Train Search ---- */

            {
                name: 'on_search_matched',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, uint16_t search_address, uint8_t flags',
                description: 'A train search matched this node',
                detail: 'Fires when a throttle searches for a train address and this node matches. The search_address is the DCC address the throttle is looking for (0\u201316383), and flags contain match criteria (exact match, force long address, allocate if needed, etc.). The library has already sent the Search Matched response. Use this for logging or to update a display showing which throttle found this train. See Train Search Protocol for flag definitions.',
                required: false,
                configField: 'on_train_search_matched',
                section: 'Train Search'
            },

            {
                name: 'on_search_no_match',
                returnType: 'openlcb_node_t*',
                params: 'uint16_t search_address, uint8_t flags',
                description: 'No train matched the search. Return a new train node, or NULL to ignore',
                detail: 'Fires when a throttle searches for a train address and no existing train node matches. Return a pointer to a newly created train node (using OpenLcb_create_node) to dynamically create trains on demand, or return NULL to ignore the search. This enables command stations to create train nodes automatically when a throttle dials up a new address. The library will register the new node on the CAN network and send the appropriate search response if you return non-NULL.',
                required: false,
                configField: 'on_train_search_no_match',
                section: 'Train Search'
            }

        ]

    },

    'cb-bcast-time': {

        title: 'Broadcast Time Callbacks',
        description: 'Notifications for fast-clock time changes',
        groupDetail: 'The OpenLCB Broadcast Time protocol distributes a shared simulated fast-clock across the layout using Well-Known Event IDs. on_time_changed is the primary callback \u2014 it fires every simulated minute and is the right place for time-triggered automation such as turning on building lights at a specific fast-clock hour. The clock producer broadcasts Report Time, Date, Year, and Rate events during startup and after any clock set command; the library decodes them and calls the appropriate on_*_received callbacks so your node stays synchronized. The clock advances each 100ms tick scaled by the current rate (e.g. rate 4.0 means the clock runs 4x faster than real time). on_clock_started and on_clock_stopped let you pause and resume time-dependent operations.',
        filePrefix: 'callbacks_broadcast_time',
        headerGuard: '__CALLBACKS_BROADCAST_TIME__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'CallbacksBroadcastTime',
        functions: [

            {
                name: 'on_time_changed',
                returnType: 'void',
                params: 'broadcast_clock_t *clock',
                description: 'Fast-clock minute advanced',
                detail: 'Fires each time the fast clock advances to a new minute. The clock struct contains the current time, date, year, and rate. This is the most commonly used broadcast time callback — use it for time-triggered automation like turning on layout lights at dusk (e.g. 18:00 fast time) or triggering scheduled train departures. The clock accumulator resets and begins counting toward the next minute boundary. See Broadcast Time Standard for time event encoding.',
                required: false,
                configField: 'on_broadcast_time_changed'
            },

            {
                name: 'on_time_received',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Time update received from the clock generator',
                detail: 'Fires when a Report Time event is received from the clock producer, containing the current hour and minute. The clock_state is already updated with the new time. This fires during clock synchronization sequences and whenever the clock producer sends a time correction. The event uses well-known Event IDs in the broadcast time range.',
                required: false,
                configField: 'on_broadcast_time_received'
            },

            {
                name: 'on_date_received',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Date update received from the clock generator',
                detail: 'Fires when a Report Date event is received from the clock producer, containing the current month and day. The clock_state is already updated with the new date values. Use this if your application needs to track the simulated calendar date.',
                required: false,
                configField: 'on_broadcast_date_received'
            },

            {
                name: 'on_year_received',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Year update received from the clock generator',
                detail: 'Fires when a Report Year event is received from the clock producer. The clock_state is already updated. Useful for era-specific layouts where the simulated year affects operations (e.g. different consist rules or rolling stock for different decades).',
                required: false,
                configField: 'on_broadcast_year_received'
            },

            {
                name: 'on_rate_received',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Rate update received from the clock generator',
                detail: 'Fires when the clock rate changes. The rate is a multiplier — 1.0 means real time, 4.0 means the clock runs 4x faster than real time, 0.5 means half speed. Negative rates run the clock backwards. Use this if your application needs to adjust behavior based on how fast time is passing. See Broadcast Time Standard for rate encoding.',
                required: false,
                configField: 'on_broadcast_rate_received'
            },

            {
                name: 'on_clock_started',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Fast clock started running',
                detail: 'Fires when the clock producer starts the clock (transitions from stopped to running). The millisecond accumulator is reset and the clock begins advancing at the current rate each 100 ms tick. Use this to resume time-dependent operations that were paused when the clock stopped.',
                required: false,
                configField: 'on_broadcast_clock_started'
            },

            {
                name: 'on_clock_stopped',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Fast clock stopped',
                detail: 'Fires when the clock producer stops the clock. Time is frozen at the current value and the millisecond accumulator stops incrementing. Use this to pause time-dependent operations — for example, hold lighting states steady until the clock resumes.',
                required: false,
                configField: 'on_broadcast_clock_stopped'
            },

            {
                name: 'on_date_rollover',
                returnType: 'void',
                params: 'openlcb_node_t *openlcb_node, broadcast_clock_state_t *clock_state',
                description: 'Fast-clock date rolled over to the next day',
                detail: 'Fires when the clock crosses midnight (transitions from 23:59 to 00:00). The date has already been incremented (month and day updated). Use this to reset daily counters, trigger end-of-day events, or log operating session boundaries.',
                required: false,
                configField: 'on_broadcast_date_rollover'
            }

        ]

    }

};
