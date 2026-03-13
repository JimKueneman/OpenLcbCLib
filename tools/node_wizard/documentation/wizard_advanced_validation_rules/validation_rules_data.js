/* ============================================================================
 * validation_rules_data.js — Advanced UI validation rules
 *
 * Each rule describes a constraint between two fields.
 * The validator checks: if ifField === ifValue, then thenField must satisfy
 * the thenOp against thenValue.
 *
 * Column layout:
 *   id        — unique rule identifier (e.g. "FF-001")
 *   severity  — "error" | "warning"
 *   ifField   — the field whose value triggers the check
 *   ifOp      — comparison for the trigger: "===" | "!==" | ">" | "<" | ">=" | "<="
 *   ifValue   — the trigger value (string)
 *   thenField — the field being validated
 *   thenOp    — required relationship: "===" | "!==" | ">" | "<" | ">=" | "<="
 *   thenValue — the required value (string)
 *   message   — human-readable explanation shown to the user
 *   notes     — internal notes for the matrix editor
 * ============================================================================ */

var VALIDATION_SECTIONS = [

    {
        id: "feature-flag-deps",
        title: "Feature Flag Dependencies",
        description: "Feature flags that require other flags to be enabled first.",
        rows: [
            {
                id: "FF-001",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "OPENLCB_COMPILE_DATAGRAMS",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Memory Configuration requires Datagrams to be ON",
                notes:     "Config-mem protocol runs over datagrams"
            },
            {
                id: "FF-002",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_FIRMWARE",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Firmware upgrade requires Memory Configuration to be ON",
                notes:     "Firmware upload uses memory config protocol"
            },
            {
                id: "FF-003",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_BROADCAST_TIME",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "OPENLCB_COMPILE_EVENTS",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Broadcast Time requires Events to be ON",
                notes:     "Broadcast time uses event producers/consumers"
            },
            {
                id: "FF-004",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "OPENLCB_COMPILE_EVENTS",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Train protocol requires Events to be ON",
                notes:     "Train uses event exchange for E-stop etc."
            },
            {
                id: "FF-005",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_STREAM",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "OPENLCB_COMPILE_STREAM",
                thenOp:    "===",
                thenValue: "OFF",
                message:   "Stream protocol is unreleased and must remain OFF",
                notes:     "Not yet implemented in this library"
            }
        ]
    },

    {
        id: "psi-consistency",
        title: "Protocol Support Bit Consistency",
        description: "PSI bits must match the corresponding feature flags.",
        rows: [
            {
                id: "PSI-001",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_DATAGRAMS",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_DATAGRAM",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_DATAGRAM must be ON when datagrams are enabled",
                notes:     ""
            },
            {
                id: "PSI-002",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_DATAGRAMS",
                ifOp:      "===",
                ifValue:   "OFF",
                thenField: "PSI_DATAGRAM",
                thenOp:    "===",
                thenValue: "OFF",
                message:   "PSI_DATAGRAM must be OFF when datagrams are disabled",
                notes:     ""
            },
            {
                id: "PSI-003",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_MEMORY_CONFIGURATION must be ON when config-mem is enabled",
                notes:     ""
            },
            {
                id: "PSI-004",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_CONFIGURATION_DESCRIPTION_INFO",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_CONFIGURATION_DESCRIPTION_INFO must be ON when config-mem is enabled",
                notes:     ""
            },
            {
                id: "PSI-005",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_ABBREVIATED_DEFAULT_CDI",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_ABBREVIATED_DEFAULT_CDI must be ON when config-mem is enabled",
                notes:     ""
            },
            {
                id: "PSI-006",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_TRAIN_CONTROL",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_TRAIN_CONTROL must be ON when train protocol is enabled",
                notes:     ""
            },
            {
                id: "PSI-007",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_FUNCTION_DESCRIPTION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_FUNCTION_DESCRIPTION must be ON when train protocol is enabled",
                notes:     ""
            },
            {
                id: "PSI-008",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_FIRMWARE",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_FIRMWARE_UPGRADE",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_FIRMWARE_UPGRADE must be ON when firmware is enabled",
                notes:     ""
            },
            {
                id: "PSI-009",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_EVENTS",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "PSI_EVENT_EXCHANGE",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_EVENT_EXCHANGE must be ON when events are enabled",
                notes:     ""
            },
            {
                id: "PSI-010",
                severity: "error",
                ifField:   "PSI_SIMPLE_NODE_INFORMATION",
                ifOp:      "===",
                ifValue:   "OFF",
                thenField: "PSI_SIMPLE_NODE_INFORMATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "PSI_SIMPLE_NODE_INFORMATION must always be ON",
                notes:     "Required by the OpenLCB specification"
            }
        ]
    },

    {
        id: "address-space-deps",
        title: "Address Space Dependencies",
        description: "Address spaces that require feature flags to be enabled.",
        rows: [
            {
                id: "AS-001",
                severity: "error",
                ifField:   "addr-0xff .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "CDI space (0xFF) requires Memory Configuration to be ON",
                notes:     ""
            },
            {
                id: "AS-002",
                severity: "error",
                ifField:   "addr-0xfd .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Config Memory space (0xFD) requires Memory Configuration to be ON",
                notes:     ""
            },
            {
                id: "AS-003",
                severity: "error",
                ifField:   "addr-0xfc .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "ACDI Manufacturer space (0xFC) requires Memory Configuration to be ON",
                notes:     ""
            },
            {
                id: "AS-004",
                severity: "error",
                ifField:   "addr-0xfb .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                thenOp:    "===",
                thenValue: "ON",
                message:   "ACDI User space (0xFB) requires Memory Configuration to be ON",
                notes:     ""
            },
            {
                id: "AS-005",
                severity: "error",
                ifField:   "addr-0xfa .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_TRAIN",
                thenOp:    "===",
                thenValue: "ON",
                message:   "FDI space (0xFA) requires Train protocol to be ON",
                notes:     ""
            },
            {
                id: "AS-006",
                severity: "error",
                ifField:   "addr-0xf9 .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_TRAIN",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Train Function Config space (0xF9) requires Train protocol to be ON",
                notes:     ""
            },
            {
                id: "AS-007",
                severity: "error",
                ifField:   "addr-0xef .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: "OPENLCB_COMPILE_FIRMWARE",
                thenOp:    "===",
                thenValue: "ON",
                message:   "Firmware space (0xEF) requires Firmware feature to be ON",
                notes:     ""
            }
        ]
    },

    {
        id: "buffer-pool-sanity",
        title: "Buffer Pool Sanity",
        description: "Buffer depth values must be consistent with enabled features.",
        rows: [
            {
                id: "BP-001",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_DATAGRAMS",
                ifOp:      "===",
                ifValue:   "OFF",
                thenField: "USER_DEFINED_DATAGRAM_BUFFER_DEPTH",
                thenOp:    "===",
                thenValue: "0",
                message:   "Datagram buffer depth must be 0 when datagrams are disabled",
                notes:     ""
            },
            {
                id: "BP-002",
                severity: "error",
                ifField:   "USER_DEFINED_BASIC_BUFFER_DEPTH",
                ifOp:      "<",
                ifValue:   "1",
                thenField: "USER_DEFINED_BASIC_BUFFER_DEPTH",
                thenOp:    ">=",
                thenValue: "1",
                message:   "Basic buffer depth must be at least 1",
                notes:     "Zero means no buffers allocated"
            },
            {
                id: "BP-003",
                severity: "error",
                ifField:   "USER_DEFINED_CAN_MSG_BUFFER_DEPTH",
                ifOp:      ">",
                ifValue:   "254",
                thenField: "USER_DEFINED_CAN_MSG_BUFFER_DEPTH",
                thenOp:    "<=",
                thenValue: "254",
                message:   "CAN message buffer depth cannot exceed 254",
                notes:     "Hardware limitation"
            },
            {
                id: "BP-004",
                severity: "warning",
                ifField:   "USER_DEFINED_SNIP_BUFFER_DEPTH",
                ifOp:      "<",
                ifValue:   "2",
                thenField: "USER_DEFINED_SNIP_BUFFER_DEPTH",
                thenOp:    ">=",
                thenValue: "2",
                message:   "SNIP buffer depth below 2 may cause issues",
                notes:     "Recommended minimum is 2"
            },
            {
                id: "BP-005",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_NODE_BUFFER_DEPTH",
                thenOp:    ">=",
                thenValue: "1",
                message:   "Node buffer depth must be at least 1 when train is enabled",
                notes:     "Train virtual nodes each need a buffer slot"
            }
        ]
    },

    {
        id: "event-counts",
        title: "Event Count Sanity",
        description: "Producer and consumer counts must be consistent with enabled features.",
        rows: [
            {
                id: "EV-001",
                severity: "warning",
                ifField:   "OPENLCB_COMPILE_EVENTS",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_PRODUCER_COUNT",
                thenOp:    ">",
                thenValue: "0",
                message:   "No producers defined — is this intentional?",
                notes:     "A node with events but no producers or consumers is unusual"
            },
            {
                id: "EV-002",
                severity: "warning",
                ifField:   "OPENLCB_COMPILE_BROADCAST_TIME",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_PRODUCER_RANGE_COUNT",
                thenOp:    ">",
                thenValue: "0",
                message:   "Broadcast Time is ON but no producer ranges are defined",
                notes:     "Broadcast time uses event ranges"
            },
            {
                id: "EV-003",
                severity: "warning",
                ifField:   "OPENLCB_COMPILE_BROADCAST_TIME",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_CONSUMER_RANGE_COUNT",
                thenOp:    ">",
                thenValue: "0",
                message:   "Broadcast Time is ON but no consumer ranges are defined",
                notes:     "Broadcast time uses event ranges"
            }
        ]
    },

    {
        id: "train-sanity",
        title: "Train Protocol Sanity",
        description: "Train configuration values must be valid when train is enabled.",
        rows: [
            {
                id: "TR-001",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_TRAIN_NODE_COUNT",
                thenOp:    ">=",
                thenValue: "1",
                message:   "Train node count must be at least 1 when train is enabled",
                notes:     ""
            },
            {
                id: "TR-002",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_MAX_LISTENERS_PER_TRAIN",
                thenOp:    ">=",
                thenValue: "1",
                message:   "Max listeners per train must be at least 1 when train is enabled",
                notes:     ""
            },
            {
                id: "TR-003",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "USER_DEFINED_MAX_TRAIN_FUNCTIONS",
                thenOp:    ">=",
                thenValue: "1",
                message:   "Max train functions must be at least 1 when train is enabled",
                notes:     ""
            },
            {
                id: "TR-004",
                severity: "error",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: "addr-0xfa .present",
                thenOp:    "===",
                thenValue: "true",
                message:   "FDI space (0xFA) must be present when train is enabled",
                notes:     "Train requires FDI to describe its functions"
            },
            {
                id: "TR-005",
                severity: "warning",
                ifField:   "OPENLCB_COMPILE_TRAIN",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: ".fdi[]",
                thenOp:    "!==",
                thenValue: "{0x00}",
                message:   "Train is enabled but no FDI file has been loaded",
                notes:     "FDI describes the train's function labels"
            }
        ]
    },

    {
        id: "cdi-fdi-sanity",
        title: "CDI / FDI Sanity",
        description: "CDI and FDI byte arrays must be loaded when config-mem or train is enabled.",
        rows: [
            {
                id: "CDI-001",
                severity: "warning",
                ifField:   "OPENLCB_COMPILE_MEMORY_CONFIGURATION",
                ifOp:      "===",
                ifValue:   "ON",
                thenField: ".cdi[]",
                thenOp:    "!==",
                thenValue: "{0x00}",
                message:   "Memory Configuration is ON but no CDI file has been loaded",
                notes:     "CDI describes the node's configuration to tools"
            },
            {
                id: "CDI-002",
                severity: "error",
                ifField:   "addr-0xff .present",
                ifOp:      "===",
                ifValue:   "true",
                thenField: ".cdi[]",
                thenOp:    "!==",
                thenValue: "{0x00}",
                message:   "CDI space (0xFF) is present but CDI byte array is empty",
                notes:     "CDI space with empty content will cause protocol errors"
            }
        ]
    },

    {
        id: "memory-config-sanity",
        title: "Memory Configuration Sanity",
        description: "Memory configuration addresses must be consistent.",
        rows: [
            {
                id: "MC-001",
                severity: "error",
                ifField:   "USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS",
                ifOp:      "<=",
                ifValue:   "USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS",
                thenField: "USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS",
                thenOp:    ">",
                thenValue: "USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS",
                message:   "User description address must be greater than user name address",
                notes:     "Name field comes first in config memory layout"
            },
            {
                id: "MC-002",
                severity: "warning",
                ifField:   "addr-0xfd .highest_address",
                ifOp:      "===",
                ifValue:   "0",
                thenField: "addr-0xfd .highest_address",
                thenOp:    ">",
                thenValue: "0",
                message:   "Config Memory space highest address is 0 — did you forget to set it?",
                notes:     "Should reflect the actual config memory size"
            }
        ]
    }

];
