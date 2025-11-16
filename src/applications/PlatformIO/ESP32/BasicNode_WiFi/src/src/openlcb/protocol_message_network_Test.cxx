#include "test/main_Test.hxx"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Test",
    .snip.model = "Test Model J",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "These are options that defined the memory space capabilities",

    // Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

node_parameters_t _node_parameters_main_node_simple = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Test",
    .snip.model = "Test Model J",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_SIMPLE |
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "These are options that defined the memory space capabilities",

    // Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

interface_openlcb_protocol_message_network_t interface_openlcb_protocol_message_network = {

};

interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{
}

void _global_initialize(void)
{

    ProtocolMessageNetwork_initialize(&interface_openlcb_protocol_message_network);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolMessageNetowrk, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolMessageNetowrk, handle_protocol_support_inquiry)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);

        ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_PROTOCOL_SUPPORT_REPLY);
        EXPECT_EQ(outgoing_msg->source_alias, DEST_ALIAS);
        EXPECT_EQ(outgoing_msg->source_id, DEST_ID);
        EXPECT_EQ(outgoing_msg->dest_alias, SOURCE_ALIAS);
        EXPECT_EQ(outgoing_msg->dest_id, SOURCE_ID);
        EXPECT_EQ(outgoing_msg->dest_id, SOURCE_ID);
        EXPECT_EQ(outgoing_msg->payload_count, 6);

        uint64_t supported_protocols = OpenLcbUtilities_extract_dword_from_openlcb_payload(outgoing_msg, 0);

        fprintf(stderr, "extracted supported_protocols: 0x%0llX\n", supported_protocols);
        fprintf(stderr, "calculated supported_protocols: 0x%llX\n", (uint64_t)(PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO));

        EXPECT_EQ(supported_protocols, (PSI_DATAGRAM | PSI_FIRMWARE_UPGRADE | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO) << 8);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_INQUIRY);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        node1->state.firmware_upgrade_active = true;
        // NOTE: Handler expects the message is for them.....
        ProtocolMessageNetwork_handle_protocol_support_inquiry(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_PROTOCOL_SUPPORT_REPLY);
        EXPECT_EQ(outgoing_msg->source_alias, DEST_ALIAS);
        EXPECT_EQ(outgoing_msg->source_id, DEST_ID);
        EXPECT_EQ(outgoing_msg->dest_alias, SOURCE_ALIAS);
        EXPECT_EQ(outgoing_msg->dest_id, SOURCE_ID);
        EXPECT_EQ(outgoing_msg->dest_id, SOURCE_ID);
        EXPECT_EQ(outgoing_msg->payload_count, 6);

        supported_protocols = OpenLcbUtilities_extract_dword_from_openlcb_payload(outgoing_msg, 0);

        fprintf(stderr, "extracted supported_protocols: 0x%0llX\n", supported_protocols);
        fprintf(stderr, "calculated supported_protocols: 0x%llX\n", (uint64_t)(PSI_DATAGRAM | PSI_FIRMWARE_UPGRADE_ACTIVE | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO));

        EXPECT_EQ(supported_protocols, (PSI_DATAGRAM | PSI_FIRMWARE_UPGRADE_ACTIVE | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO) << 8);
    }
}

TEST(ProtocolMessageNetowrk, handle_protocol_support_reply)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(SNIP);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_PROTOCOL_SUPPORT_REPLY);

        // NOTE: Handler expects the message is for them.....
        // Does nothing just allows a place to inject a dependance into the statemachine for user applications to handle at PIP reply
        ProtocolMessageNetwork_handle_protocol_support_reply(&statemachine_info);
    }
}

TEST(ProtocolMessageNetowrk, verify_node_id_global)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {
        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // no data to compare
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
        EXPECT_EQ(DEST_ID, OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0));
        EXPECT_EQ(DEST_ALIAS, outgoing_msg->source_alias);

        // matching node id to compare
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
        EXPECT_EQ(DEST_ID, OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0));
        EXPECT_EQ(DEST_ALIAS, outgoing_msg->source_alias);

        // non matching node id to compare
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);

        ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, 0x00);
    }
}

TEST(ProtocolMessageNetowrk, verify_node_id_addressed)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);

        ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
        EXPECT_EQ(DEST_ID, OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0));
        EXPECT_EQ(DEST_ALIAS, outgoing_msg->source_alias);
    }
}

TEST(ProtocolMessageNetowrk, verify_node_id_addressed_simple)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_simple);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        // NOTE: Handler expects the message is for them.....
        ProtocolMessageNetwork_handle_verify_node_id_global(&statemachine_info);

        EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID_SIMPLE);
        EXPECT_EQ(DEST_ID, OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_msg, 0));
        EXPECT_EQ(DEST_ALIAS, outgoing_msg->source_alias);
    }
}

TEST(ProtocolMessageNetowrk, verified_node_id)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // a non duplicate node id
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
        ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
        EXPECT_EQ(outgoing_msg->mti, 0x00);

        // a duplicate node id
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
        ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
        EXPECT_EQ(outgoing_msg->mti, MTI_PC_EVENT_REPORT);
        EXPECT_EQ(EVENT_ID_DUPLICATE_NODE_DETECTED, OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_msg));
        EXPECT_TRUE(node1->state.duplicate_id_detected);

        // a duplicate node id again, should not resent event
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_GLOBAL);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID, 0);
        ProtocolMessageNetwork_handle_verified_node_id(&statemachine_info);
        EXPECT_EQ(outgoing_msg->mti, 0x00);
    }
}

TEST(ProtocolMessageNetowrk, handle_initialization_complete)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // Initialization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
        ProtocolMessageNetwork_handle_initialization_complete(&statemachine_info);
        EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);

        // Initialization Complete Simple
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_INITIALIZATION_COMPLETE_SIMPLE);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, DEST_ID + 1, 0);
        ProtocolMessageNetwork_handle_initialization_complete_simple(&statemachine_info);
        EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    }
}

TEST(ProtocolMessageNetowrk, handle_optional_interaction_rejected)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // Initialization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_OPTIONAL_INTERACTION_REJECTED);
        openlcb_msg->payload_count = 4;
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);
        ProtocolMessageNetwork_handle_optional_interaction_rejected(&statemachine_info);
        EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    }
}

TEST(ProtocolMessageNetowrk, handle_terminate_due_to_error)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // Initialization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_TERMINATE_DO_TO_ERROR);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, ERROR_PERMANENT_NOT_IMPLEMENTED, 0);
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x00, 2);
        ProtocolMessageNetwork_handle_terminate_due_to_error(&statemachine_info);
        EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    }
}

TEST(ProtocolMessageNetowrk, handle_verify_node_id_addressed)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(openlcb_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    if (openlcb_msg)
    {

        openlcb_statemachine_info_t statemachine_info;
        statemachine_info.openlcb_node = node1;
        statemachine_info.incoming_msg_info.msg_ptr = openlcb_msg;
        statemachine_info.incoming_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
        statemachine_info.outgoing_msg_info.enumerate = false;
        statemachine_info.outgoing_msg_info.valid = false;

        // Initialization Complete
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_VERIFY_NODE_ID_ADDRESSED);
        ProtocolMessageNetwork_handle_verify_node_id_addressed(&statemachine_info);
        EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
        EXPECT_EQ(outgoing_msg->mti, MTI_VERIFIED_NODE_ID);
        EXPECT_EQ(outgoing_msg->payload_count, 6);
    }
}