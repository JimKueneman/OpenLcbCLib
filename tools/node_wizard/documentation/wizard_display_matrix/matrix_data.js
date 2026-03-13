/* ============================================================================
 * matrix_data.js — All configuration matrix data
 *
 * Column layout per row:
 *   field     — config field name (read-only)
 *
 *   Basic UI group:
 *     basic     — default value for Basic node type
 *     typical   — default value for Typical node type
 *     train     — default value for Train node type
 *     bVis      — visible to user in Basic UI?
 *     bDerived  — where the value comes from in Basic UI
 *     bNotes    — notes specific to Basic UI behaviour
 *
 *   Advanced UI group:
 *     aVis      — visible to user in Advanced UI?
 *     aDefault  — pre-populated default value in Advanced UI
 *     aNotes    — notes specific to Advanced UI behaviour
 * ============================================================================ */

var MATRIX_SECTIONS = [

    {
        id: "feature-flags",
        title: "Feature Flags (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "OPENLCB_COMPILE_EVENTS",               basic: "ON",  typical: "ON",  train: "ON",  bVis: "No",  bDerived: "node type",            bNotes: "Always on",                           aVis: "Yes", aDefault: "ON",  aNotes: "Required for all nodes" },
            { field: "OPENLCB_COMPILE_DATAGRAMS",             basic: "OFF", typical: "ON",  train: "ON",  bVis: "No",  bDerived: "node type",            bNotes: "Typical/Train need datagrams",         aVis: "Yes", aDefault: "ON",  aNotes: "Required by config-mem and firmware" },
            { field: "OPENLCB_COMPILE_MEMORY_CONFIGURATION",  basic: "OFF", typical: "ON",  train: "ON",  bVis: "No",  bDerived: "node type",            bNotes: "Typical/Train have config-mem",        aVis: "Yes", aDefault: "ON",  aNotes: "Requires datagrams to be ON" },
            { field: "OPENLCB_COMPILE_FIRMWARE",              basic: "OFF", typical: "OFF", train: "OFF", bVis: "Yes", bDerived: "Firmware add-on",       bNotes: "Disabled for Basic (needs config-mem)", aVis: "Yes", aDefault: "OFF", aNotes: "Requires config-mem to be ON" },
            { field: "OPENLCB_COMPILE_BROADCAST_TIME",        basic: "OFF", typical: "OFF", train: "OFF", bVis: "Yes", bDerived: "Broadcast Time add-on", bNotes: "ON if Producer or Consumer selected",  aVis: "Yes", aDefault: "OFF", aNotes: "Requires events to be ON" },
            { field: "OPENLCB_COMPILE_TRAIN",                 basic: "OFF", typical: "OFF", train: "ON",  bVis: "No",  bDerived: "node type",            bNotes: "Only Train node type",                 aVis: "Yes", aDefault: "OFF", aNotes: "Enables train protocol and FDI address spaces" },
            { field: "OPENLCB_COMPILE_STREAM",                basic: "OFF", typical: "OFF", train: "OFF", bVis: "No",  bDerived: "hardcoded",            bNotes: "Unreleased, always off",               aVis: "No",  aDefault: "OFF", aNotes: "Unreleased, always off" }
        ]
    },

    {
        id: "buffer-pool",
        title: "Buffer Pool (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "USER_DEFINED_BASIC_BUFFER_DEPTH",    basic: "16", typical: "16", train: "16", bVis: "No", bDerived: "hardcoded", bNotes: "",                                     aVis: "Yes", aDefault: "8",  aNotes: "" },
            { field: "USER_DEFINED_DATAGRAM_BUFFER_DEPTH", basic: "0",  typical: "8",  train: "8",  bVis: "No", bDerived: "node type", bNotes: "0 for Basic (no datagrams)",            aVis: "Yes", aDefault: "4",  aNotes: "Set to 0 if datagrams disabled" },
            { field: "USER_DEFINED_SNIP_BUFFER_DEPTH",     basic: "4",  typical: "4",  train: "4",  bVis: "No", bDerived: "hardcoded", bNotes: "",                                     aVis: "Yes", aDefault: "2",  aNotes: "" },
            { field: "USER_DEFINED_STREAM_BUFFER_DEPTH",   basic: "0",  typical: "0",  train: "0",  bVis: "No", bDerived: "hardcoded", bNotes: "Stream unreleased",                    aVis: "No",  aDefault: "0",  aNotes: "Stream unreleased, always 0" },
            { field: "USER_DEFINED_NODE_BUFFER_DEPTH",     basic: "1",  typical: "1",  train: "1",  bVis: "No", bDerived: "hardcoded", bNotes: "Train may need more for virtual nodes", aVis: "Yes", aDefault: "1",  aNotes: "Increase for virtual node support" },
            { field: "USER_DEFINED_CAN_MSG_BUFFER_DEPTH",  basic: "20", typical: "20", train: "20", bVis: "No", bDerived: "hardcoded", bNotes: "",                                     aVis: "Yes", aDefault: "10", aNotes: "CAN driver buffer pool, max 254" }
        ]
    },

    {
        id: "events",
        title: "Events (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "USER_DEFINED_PRODUCER_COUNT",       basic: "64", typical: "64", train: "64", bVis: "Yes", bDerived: "user input", bNotes: "Default 64; increase if application overruns at runtime",                          aVis: "Yes", aDefault: "64", aNotes: "Application dependent" },
            { field: "USER_DEFINED_PRODUCER_RANGE_COUNT", basic: "4",  typical: "4",  train: "4",  bVis: "Yes", bDerived: "user input", bNotes: "Default 4; Broadcast Time uses 2 ranges — add 2 more if enabled",                    aVis: "Yes", aDefault: "4",  aNotes: "Used by broadcast time" },
            { field: "USER_DEFINED_CONSUMER_COUNT",       basic: "64", typical: "64", train: "64", bVis: "Yes", bDerived: "user input", bNotes: "Default 64; increase if application overruns at runtime",                          aVis: "Yes", aDefault: "64", aNotes: "Application dependent" },
            { field: "USER_DEFINED_CONSUMER_RANGE_COUNT", basic: "4",  typical: "4",  train: "4",  bVis: "Yes", bDerived: "user input", bNotes: "Default 4; Broadcast Time uses 2 ranges — add 2 more if enabled",                    aVis: "Yes", aDefault: "4",  aNotes: "Used by broadcast time" }
        ]
    },

    {
        id: "memory-config",
        title: "Memory Configuration (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "USER_DEFINED_CDI_LENGTH",                          basic: "1",  typical: "from CDI", train: "from CDI", bVis: "No", bDerived: "CDI file",  bNotes: "Byte length of loaded CDI, 1 if not used to hold a null",  aVis: "No",  aDefault: "from CDI file", aNotes: "Computed from loaded CDI" },
            { field: "USER_DEFINED_FDI_LENGTH",                          basic: "1",  typical: "1",         train: "from FDI", bVis: "No", bDerived: "FDI file",  bNotes: "Byte length of loaded FDI, 1 if not used to hold a null",  aVis: "No",  aDefault: "from FDI file", aNotes: "Computed from loaded FDI" },
            { field: "USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS",        basic: "0",  typical: "0",         train: "0",        bVis: "No", bDerived: "hardcoded", bNotes: "May need to be user adjustable TODO",                      aVis: "Yes", aDefault: "0",             aNotes: "Byte offset in config space" },
            { field: "USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS", basic: "63", typical: "63",        train: "63",       bVis: "No", bDerived: "hardcoded", bNotes: "May need to be user adjustable TODO",                      aVis: "Yes", aDefault: "63",            aNotes: "Byte offset in config space" }
        ]
    },

    {
        id: "train-protocol",
        title: "Train Protocol (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "USER_DEFINED_TRAIN_NODE_COUNT",        basic: "1", typical: "1", train: "1",  bVis: "No", bDerived: "hardcoded",              bNotes: "", aVis: "Yes", aDefault: "1", aNotes: "How many trains this node manages" },
            { field: "USER_DEFINED_MAX_LISTENERS_PER_TRAIN", basic: "0", typical: "0", train: "6",  bVis: "No", bDerived: "hardcoded",              bNotes: "", aVis: "Yes", aDefault: "1", aNotes: "Max throttles per train" },
            { field: "USER_DEFINED_MAX_TRAIN_FUNCTIONS",     basic: "0", typical: "0", train: "29", bVis: "No", bDerived: "From NMRA DCC with F0-F28", bNotes: "", aVis: "Yes", aDefault: "1", aNotes: "F0-F28 or subset" }
        ]
    },

    {
        id: "alias-listener",
        title: "Alias Mapping Listener (openlcb_user_config.h)",
        fieldLabel: "#define",
        rows: [
            { field: "USER_DEFINED_LISTENER_PROBE_TICK_INTERVAL",  basic: "1",   typical: "1",   train: "1",   bVis: "No", bDerived: "hardcoded", bNotes: "needed only for train", aVis: "Yes", aDefault: "1",   aNotes: "100ms ticks between alias probes" },
            { field: "USER_DEFINED_LISTENER_PROBE_INTERVAL_TICKS", basic: "250", typical: "250", train: "250", bVis: "No", bDerived: "hardcoded", bNotes: "needed only for train", aVis: "Yes", aDefault: "250", aNotes: "100ms ticks between probe cycles" },
            { field: "USER_DEFINED_LISTENER_VERIFY_TIMEOUT_TICKS", basic: "30",  typical: "30",  train: "30",  bVis: "No", bDerived: "hardcoded", bNotes: "needed only for train", aVis: "Yes", aDefault: "30",  aNotes: "100ms ticks before verify timeout" }
        ]
    },

    {
        id: "snip",
        title: "SNIP (node_parameters_t.snip)",
        fieldLabel: "Field",
        rows: [
            { field: ".snip.mfg_version",     basic: "4",          typical: "4",        train: "4",        bVis: "No",               bDerived: "hardcoded",        bNotes: "Always 4 (spec version)",    aVis: "No",               aDefault: "4",  aNotes: "Spec version, always 4" },
            { field: ".snip.name",             basic: "user input", typical: "from CDI", train: "from CDI", bVis: "Yes (Basic only)", bDerived: "SNIP add-on text", bNotes: "Others: extracted from CDI", aVis: "Yes (Basic only)", aDefault: "",   aNotes: "Extracted from CDI if loaded" },
            { field: ".snip.model",            basic: "user input", typical: "from CDI", train: "from CDI", bVis: "Yes (Basic only)", bDerived: "SNIP add-on text", bNotes: "Others: extracted from CDI", aVis: "Yes (Basic only)", aDefault: "",   aNotes: "Extracted from CDI if loaded" },
            { field: ".snip.hardware_version", basic: "user input", typical: "from CDI", train: "from CDI", bVis: "Yes (Basic only)", bDerived: "SNIP add-on text", bNotes: "Others: extracted from CDI", aVis: "Yes (Basic only)", aDefault: "",   aNotes: "Extracted from CDI if loaded" },
            { field: ".snip.software_version", basic: "user input", typical: "from CDI", train: "from CDI", bVis: "Yes (Basic only)", bDerived: "SNIP add-on text", bNotes: "Others: extracted from CDI", aVis: "Yes (Basic only)", aDefault: "",   aNotes: "Extracted from CDI if loaded" },
            { field: ".snip.user_version",     basic: "2",          typical: "2",        train: "2",        bVis: "No",               bDerived: "hardcoded",        bNotes: "Always 2 (spec version)",    aVis: "No",               aDefault: "2",  aNotes: "Spec version, always 2" }
        ]
    },

    {
        id: "protocol-support",
        title: "Protocol Support (node_parameters_t.protocol_support)",
        fieldLabel: "PSI Bit",
        rows: [
            { field: "PSI_SIMPLE_NODE_INFORMATION",         basic: "OFF", typical: "ON",   train: "ON",   bVis: "No", bDerived: "hardcoded",       bNotes: "Never used",                     aVis: "No",  aDefault: "OFF", aNotes: "Never used" },
            { field: "PSI_DATAGRAM",                        basic: "OFF", typical: "ON",   train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Follows datagrams flag",         aVis: "Yes", aDefault: "ON",  aNotes: "Follows datagrams feature flag" },
            { field: "PSI_MEMORY_CONFIGURATION",            basic: "OFF", typical: "ON",   train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Follows config-mem flag",        aVis: "Yes", aDefault: "ON",  aNotes: "Follows config-mem feature flag" },
            { field: "PSI_CONFIGURATION_DESCRIPTION_INFO",  basic: "OFF", typical: "ON",   train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Follows config-mem flag",        aVis: "Yes", aDefault: "ON",  aNotes: "Follows config-mem feature flag" },
            { field: "PSI_ABBREVIATED_DEFAULT_CDI",         basic: "OFF", typical: "ON",   train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Follows config-mem flag",        aVis: "Yes", aDefault: "ON",  aNotes: "Follows config-mem feature flag" },
            { field: "PSI_EVENT_EXCHANGE",                  basic: "ON",  typical: "ON",   train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "All node types use events",      aVis: "Yes", aDefault: "ON",  aNotes: "Follows events feature flag" },
            { field: "PSI_TRAIN_CONTROL",                   basic: "OFF", typical: "OFF",  train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Only Train node type",           aVis: "Yes", aDefault: "OFF", aNotes: "Follows train feature flag" },
            { field: "PSI_FUNCTION_DESCRIPTION",            basic: "OFF", typical: "OFF",  train: "ON",   bVis: "No", bDerived: "node type",       bNotes: "Follows train flag",             aVis: "Yes", aDefault: "OFF", aNotes: "Follows train feature flag" },
            { field: "PSI_FIRMWARE_UPGRADE",                basic: "OFF", typical: "OFF*", train: "OFF*", bVis: "No", bDerived: "Firmware add-on", bNotes: "*ON if firmware add-on checked", aVis: "Yes", aDefault: "OFF", aNotes: "Follows firmware feature flag" },
            { field: "PSI_STREAM",                          basic: "OFF", typical: "OFF",  train: "OFF",  bVis: "No", bDerived: "hardcoded",       bNotes: "Unreleased",                     aVis: "Yes", aDefault: "OFF", aNotes: "Unreleased" }
        ]
    },

    {
        id: "config-options",
        title: "Configuration Options (node_parameters_t.configuration_options)",
        fieldLabel: "Field",
        rows: [
            { field: ".write_under_mask_supported",                  basic: "false", typical: "true",    train: "true",    bVis: "No",  bDerived: "node type",          bNotes: "Always on when config-mem present",          aVis: "No",  aDefault: "true",    aNotes: "Always on when config-mem enabled" },
            { field: ".unaligned_reads_supported",                   basic: "false", typical: "true",    train: "true",    bVis: "Yes", bDerived: "node type",          bNotes: "Use may be using a EEPROM/FRAM that has limitations", aVis: "Yes", aDefault: "true",    aNotes: "" },
            { field: ".unaligned_writes_supported",                  basic: "false", typical: "true",    train: "true",    bVis: "Yes", bDerived: "node type",          bNotes: "Use may be using a EEPROM/FRAM that has limitations", aVis: "Yes", aDefault: "true",    aNotes: "" },
            { field: ".read_from_manufacturer_space_0xfc_supported", basic: "false", typical: "true",    train: "true",    bVis: "No",  bDerived: "node type",          bNotes: "Always on when config-mem present",          aVis: "No",  aDefault: "true",    aNotes: "Always on when config-mem enabled" },
            { field: ".read_from_user_space_0xfb_supported",         basic: "false", typical: "true",    train: "true",    bVis: "No",  bDerived: "node type",          bNotes: "Always on when config-mem present",          aVis: "No",  aDefault: "true",    aNotes: "Always on when config-mem enabled" },
            { field: ".write_to_user_space_0xfb_supported",          basic: "false", typical: "true",    train: "true",    bVis: "No",  bDerived: "node type",          bNotes: "Always on when config-mem present",          aVis: "No",  aDefault: "true",    aNotes: "Always on when config-mem enabled" },
            { field: ".stream_read_write_supported",                 basic: "false", typical: "false",   train: "false",   bVis: "No",  bDerived: "hardcoded",          bNotes: "Stream unreleased",                          aVis: "No",  aDefault: "false",   aNotes: "Stream unreleased" },
            { field: ".high_address_space",                          basic: "false", typical: "0xFF",    train: "0xFF",    bVis: "No",  bDerived: "node type",          bNotes: "Always CDI space",                           aVis: "No",  aDefault: "0xFF",    aNotes: "Always CDI space" },
            { field: ".low_address_space",                           basic: "false", typical: "derived", train: "derived", bVis: "No",  bDerived: "node type + add-ons", bNotes: "Lowest active space",                       aVis: "No",  aDefault: "derived", aNotes: "Computed from enabled address spaces" },
            { field: ".description",                                 basic: "false", typical: "",        train: "",        bVis: "No",  bDerived: "hardcoded",          bNotes: "",                                           aVis: "Yes", aDefault: "",        aNotes: "Optional config description" }
        ]
    },

    {
        id: "addr-0xff",
        title: "Address Space 0xFF — CDI",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "true", train: "true", bVis: "No", bDerived: "node type", bNotes: "Typical/Train have CDI",  aVis: "No",  aDefault: "true",          aNotes: "Requires config-mem" },
            { field: ".read_only",         basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "CDI is always read-only", aVis: "No",  aDefault: "true",          aNotes: "CDI is always read-only" },
            { field: ".highest_address",   basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "CDI file",  bNotes: "CDI byte length - 1",    aVis: "No",  aDefault: "from CDI file", aNotes: "Computed from loaded CDI" },
            { field: ".low_address_valid", basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "false",                   aVis: "No",  aDefault: "false",         aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "0",                       aVis: "No",  aDefault: "0",             aNotes: "" },
            { field: ".description",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "null",                    aVis: "Yes", aDefault: "",              aNotes: "" }
        ]
    },

    {
        id: "addr-0xfe",
        title: "Address Space 0xFE — All Memory",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "false", train: "false", bVis: "No", bDerived: "hardcoded off", bNotes: "Rarely used",        aVis: "Yes", aDefault: "false",        aNotes: "Rarely used" },
            { field: ".read_only",         basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",     bNotes: "",                   aVis: "Yes", aDefault: "false",        aNotes: "" },
            { field: ".highest_address",   basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",     bNotes: "Default = max uint", aVis: "Yes", aDefault: "Default = max uint", aNotes: "" },
            { field: ".low_address_valid", basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",     bNotes: "",                   aVis: "Yes", aDefault: "false",        aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",     bNotes: "",                   aVis: "Yes", aDefault: "0",            aNotes: "" },
            { field: ".description",       basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",     bNotes: "null",               aVis: "Yes", aDefault: "",             aNotes: "" }
        ]
    },

    {
        id: "addr-0xfd",
        title: "Address Space 0xFD — Config Memory",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "true", train: "true", bVis: "No",  bDerived: "node type",  bNotes: "Typical/Train have config-mem", aVis: "No",  aDefault: "true",  aNotes: "Follows config-mem feature flag" },
            { field: ".read_only",         basic: "",      typical: "",     train: "",     bVis: "No",  bDerived: "hardcoded",  bNotes: "Config memory is RW",           aVis: "No",  aDefault: "false", aNotes: "Config memory is RW" },
            { field: ".highest_address",   basic: "",      typical: "",     train: "",     bVis: "Yes", bDerived: "user defined", bNotes: "default 0x200",               aVis: "Yes", aDefault: "0",     aNotes: "Application-dependent" },
            { field: ".low_address_valid", basic: "",      typical: "",     train: "",     bVis: "No",  bDerived: "hardcoded",  bNotes: "false",                         aVis: "No",  aDefault: "false", aNotes: "Derived from user name address" },
            { field: ".low_address",       basic: "",      typical: "",     train: "",     bVis: "No",  bDerived: "hardcoded",  bNotes: "default 0",                     aVis: "No",  aDefault: "0",     aNotes: "Derived from user name address" },
            { field: ".description",       basic: "",      typical: "",     train: "",     bVis: "No",  bDerived: "hardcoded",  bNotes: "null",                          aVis: "Yes", aDefault: "",      aNotes: "" }
        ]
    },

    {
        id: "addr-0xfc",
        title: "Address Space 0xFC — ACDI Manufacturer",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "true", train: "true", bVis: "No", bDerived: "node type", bNotes: "Follows config-mem",             aVis: "No", aDefault: "true",  aNotes: "Follows config-mem feature flag" },
            { field: ".read_only",         basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "Manufacturer data is read-only", aVis: "No", aDefault: "true",  aNotes: "Manufacturer data is read-only" },
            { field: ".highest_address",   basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "Fixed: 125 - 1",                 aVis: "No", aDefault: "124",   aNotes: "Fixed layout" },
            { field: ".low_address_valid", basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                               aVis: "No", aDefault: "false", aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                               aVis: "No", aDefault: "0",     aNotes: "" },
            { field: ".description",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "null",                           aVis: "No", aDefault: "",      aNotes: "Fixed layout" }
        ]
    },

    {
        id: "addr-0xfb",
        title: "Address Space 0xFB — ACDI User",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "true", train: "true", bVis: "No", bDerived: "node type", bNotes: "Follows config-mem",             aVis: "No", aDefault: "true",  aNotes: "Follows config-mem feature flag" },
            { field: ".read_only",         basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "User can write name/description", aVis: "No", aDefault: "false", aNotes: "User can write name/description" },
            { field: ".highest_address",   basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "Fixed: 128 - 1",                 aVis: "No", aDefault: "127",   aNotes: "Fixed layout" },
            { field: ".low_address_valid", basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                               aVis: "No", aDefault: "false", aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                               aVis: "No", aDefault: "0",     aNotes: "" },
            { field: ".description",       basic: "",      typical: "",     train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "null",                           aVis: "No", aDefault: "",      aNotes: "Fixed layout" }
        ]
    },

    {
        id: "addr-0xfa",
        title: "Address Space 0xFA — Train FDI",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "false", train: "true", bVis: "No", bDerived: "node type", bNotes: "Train only",           aVis: "No",  aDefault: "false",         aNotes: "Follows train feature flag" },
            { field: ".read_only",         basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "FDI is read-only",     aVis: "No",  aDefault: "true",          aNotes: "FDI is read-only" },
            { field: ".highest_address",   basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "FDI file",  bNotes: "FDI byte length - 1", aVis: "No",  aDefault: "from FDI file", aNotes: "Computed from loaded FDI" },
            { field: ".low_address_valid", basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                     aVis: "No",  aDefault: "false",         aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                     aVis: "No",  aDefault: "0",             aNotes: "" },
            { field: ".description",       basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "null",                 aVis: "Yes", aDefault: "",              aNotes: "" }
        ]
    },

    {
        id: "addr-0xf9",
        title: "Address Space 0xF9 — Train Function Config",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "false", train: "true", bVis: "No", bDerived: "node type", bNotes: "Train only",            aVis: "No",  aDefault: "false", aNotes: "Follows train feature flag" },
            { field: ".read_only",         basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "Function config is RW", aVis: "No",  aDefault: "false", aNotes: "Function config is RW" },
            { field: ".highest_address",   basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "?",         bNotes: "Application-dependent", aVis: "Yes", aDefault: "0",     aNotes: "Application-dependent" },
            { field: ".low_address_valid", basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                      aVis: "No",  aDefault: "false", aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "",                      aVis: "No",  aDefault: "0",     aNotes: "" },
            { field: ".description",       basic: "",      typical: "",      train: "",     bVis: "No", bDerived: "hardcoded", bNotes: "null",                  aVis: "Yes", aDefault: "",      aNotes: "" }
        ]
    },

    {
        id: "addr-0xef",
        title: "Address Space 0xEF — Firmware",
        fieldLabel: "Field",
        rows: [
            { field: ".present",           basic: "false", typical: "false", train: "false", bVis: "No", bDerived: "Firmware add-on", bNotes: "true when firmware add-on checked", aVis: "No", aDefault: "false",        aNotes: "Follows firmware feature flag" },
            { field: ".read_only",         basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",       bNotes: "Firmware writes required",          aVis: "No", aDefault: "false",        aNotes: "Firmware writes required" },
            { field: ".highest_address",   basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",       bNotes: "Default = max uint",                aVis: "No", aDefault: "Default = max uint", aNotes: "Full address range" },
            { field: ".low_address_valid", basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",       bNotes: "",                                  aVis: "No", aDefault: "false",        aNotes: "" },
            { field: ".low_address",       basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",       bNotes: "",                                  aVis: "No", aDefault: "0",            aNotes: "" },
            { field: ".description",       basic: "",      typical: "",      train: "",      bVis: "No", bDerived: "hardcoded",       bNotes: "null",                              aVis: "No", aDefault: "",             aNotes: "" }
        ]
    },

    {
        id: "auto-create",
        title: "Auto-create Counts",
        fieldLabel: "Field",
        rows: [
            { field: ".consumer_count_autocreate", basic: "0", typical: "0", train: "0", bVis: "No", bDerived: "hardcoded", bNotes: "Always 0", aVis: "No", aDefault: "0", aNotes: "Always 0" },
            { field: ".producer_count_autocreate", basic: "0", typical: "0", train: "0", bVis: "No", bDerived: "hardcoded", bNotes: "Always 0", aVis: "No", aDefault: "0", aNotes: "Always 0" }
        ]
    },

    {
        id: "cdi-fdi",
        title: "CDI / FDI Byte Arrays",
        fieldLabel: "Field",
        rows: [
            { field: ".cdi[]", basic: "{0x00}", typical: "from file", train: "from file", bVis: "Yes", bDerived: "CDI file upload", bNotes: "Typical/Train require CDI file", aVis: "Yes", aDefault: "{0x00}", aNotes: "Load CDI XML file" },
            { field: ".fdi[]", basic: "{0x00}", typical: "{0x00}",    train: "from file", bVis: "Yes", bDerived: "FDI file upload", bNotes: "Train requires FDI file",        aVis: "Yes", aDefault: "{0x00}", aNotes: "Load FDI XML file" }
        ]
    }

];
