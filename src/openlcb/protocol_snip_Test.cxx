#include "test/main_Test.hxx"

#include "protocol_snip.h"

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
#define SNIP_NAME_FULL "0123456789012345678901234567890123456789"
#define SNIP_MODEL "Test Model J"

bool lock_node_list_called = false;
bool unlock_node_list_called = false;

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = 0,
    .producer_count_autocreate = 0,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = SNIP_NAME_FULL,
    .snip.model = SNIP_MODEL,
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

    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,

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
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

void lock_node_list(void)
{

    lock_node_list_called = true;
}

void unlock_node_list(void)
{

    unlock_node_list_called = true;
}

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

uint16_t _configuration_memory_read(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    //  fprintf(stderr, "_configuration_memory_read: address (what we put in the array) = %08X, count = %d, buffer = %p\n", address, count, buffer);

    (*buffer)[0] = 'H';
    (*buffer)[1] = 'i';
    (*buffer)[2] = 'X';
    (*buffer)[3] = 0x00;

    return 3;
}

interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {

    .configuration_memory_read = &_configuration_memory_read};

void _reset_variables(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;
}

void _global_initialize(void)
{

    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolSnip, initialize)
{

    _reset_variables();
    _global_initialize();
}

typedef struct
{
    uint8_t *version_id;      // points to the index in the SNIP data where the version id is
    char *name;               // points to the index in the SNIP data where the name string is
    char *model;              // points to the index in the SNIP data where the model string is
    char *hardware_version;   // points to the index in the SNIP data where the hardware_version string is
    char *software_version;   // points to the index in the SNIP data where the software_version string is
    uint8_t *user_version_id; // points to the index in the SNIP data where the user_version_id  is
    char *user_name;          // points to the index in the SNIP data where the user_name string is
    char *user_description;   // points to the index in the SNIP data where the user_description string is
} _snip_data_indexer_t;

bool _extract_snip(openlcb_msg_t *openlcb_msg, _snip_data_indexer_t *snip_data_indexer)
{
    uint16_t index = 0;
    openlcb_payload_t *local_payload_ptr = openlcb_msg->payload;

    snip_data_indexer->version_id = (uint8_t *)&local_payload_ptr[index];
    index++;

    fprintf(stderr, "version_id: %d\n", *snip_data_indexer->version_id);

    snip_data_indexer->name = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->name);
    index++; // jump over the null

    if (index >= openlcb_msg->payload_count)
    {

        return false;
    }

    fprintf(stderr, "name: %s\n", snip_data_indexer->name);

    snip_data_indexer->model = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->model);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        return false;
    }

    fprintf(stderr, "model: %s\n", snip_data_indexer->model);

    snip_data_indexer->hardware_version = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->hardware_version);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        return false;
    }

    fprintf(stderr, "hardware_version: %s\n", snip_data_indexer->hardware_version);

    snip_data_indexer->software_version = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->software_version);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        return false;
    }

    fprintf(stderr, "software_version: %s\n", snip_data_indexer->software_version);

    snip_data_indexer->user_version_id = (uint8_t *)(&local_payload_ptr[index]);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        return false;
    }

    fprintf(stderr, "version_id: %d\n", *snip_data_indexer->user_version_id);

    snip_data_indexer->user_name = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->user_name);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        fprintf(stderr, "bail \n");
        return false;
    }

    fprintf(stderr, "user_name: %s\n", snip_data_indexer->user_name);

    snip_data_indexer->user_description = (char *)(&local_payload_ptr[index]);
    index = index + strlen(snip_data_indexer->user_description);
    index++; // jump over the null

    if (index > openlcb_msg->payload_count)
    {

        fprintf(stderr, "bail \n");
        return false;
    }

    fprintf(stderr, "user_description: %s\n", snip_data_indexer->user_description);

    return true;
}

TEST(ProtocolSnip, handle_simple_node_info_request)
{

    _snip_data_indexer_t snip_data_indexer;

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

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);

        ProtocolSnip_handle_simple_node_info_request(node1, openlcb_msg, outgoing_msg);

        EXPECT_TRUE(_extract_snip(outgoing_msg, &snip_data_indexer));

        EXPECT_EQ(*snip_data_indexer.version_id, 4);
        EXPECT_STREQ(snip_data_indexer.name, SNIP_NAME_FULL);
        EXPECT_STREQ(snip_data_indexer.model, "Test Model J");
        EXPECT_STREQ(snip_data_indexer.hardware_version, "0.001");
        EXPECT_STREQ(snip_data_indexer.software_version, "0.002");
        EXPECT_EQ(*snip_data_indexer.user_version_id, 2);
        EXPECT_STREQ(snip_data_indexer.user_name, "HiX");
        EXPECT_STREQ(snip_data_indexer.user_description, "HiX");
    }
}

TEST(ProtocolSnip, validate_snip_reply)
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

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);

        ProtocolSnip_handle_simple_node_info_request(node1, openlcb_msg, outgoing_msg);

        EXPECT_FALSE(ProtocolSnip_validate_snip_reply(openlcb_msg)); // incorrect message type

        EXPECT_TRUE(ProtocolSnip_validate_snip_reply(outgoing_msg)); // correct message type

        uint16_t old_count = outgoing_msg->payload_count;
        outgoing_msg->payload_count = LEN_MESSAGE_BYTES_SNIP + 1;
        EXPECT_FALSE(ProtocolSnip_validate_snip_reply(outgoing_msg)); // correct message type but payload count corrupted
        outgoing_msg->payload_count = old_count;

        uint8_t old_byte = *outgoing_msg->payload[LEN_SNIP_NAME_BUFFER];
        EXPECT_EQ(old_byte, 0x00);
        *outgoing_msg->payload[LEN_SNIP_NAME_BUFFER] = 'D';
        EXPECT_FALSE(ProtocolSnip_validate_snip_reply(outgoing_msg)); // correct message type but not enough nulls
        *outgoing_msg->payload[LEN_SNIP_NAME_BUFFER] = 0x00;
    }
}

TEST(ProtocolSnip, load_name_request_too_many_bytes)
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

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);

        EXPECT_EQ(ProtocolSnip_load_name(node1, outgoing_msg, 0, LEN_SNIP_NAME_BUFFER + 10), LEN_SNIP_NAME_BUFFER); // ask for more than expected
    }
}

TEST(ProtocolSnip, load_with_asking_for_partial_strings)
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

        //  only ask for 1 byte
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_manufacturer_version_id(node1, outgoing_msg, 0, 1), 1);
        EXPECT_EQ(outgoing_msg->payload_count, 1);

        //  is the name so only ask for 16 bytes
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_name(node1, outgoing_msg, 0, 16), 16);
        EXPECT_EQ(outgoing_msg->payload_count, 16);
        EXPECT_EQ(*outgoing_msg->payload[0], '0');
        EXPECT_EQ(*outgoing_msg->payload[1], '1');
        EXPECT_EQ(*outgoing_msg->payload[2], '2');

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_model(node1, outgoing_msg, 0, 3), 3);
        EXPECT_EQ(outgoing_msg->payload_count, 3);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_hardware_version(node1, outgoing_msg, 0, 3), 3);
        EXPECT_EQ(outgoing_msg->payload_count, 3);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_software_version(node1, outgoing_msg, 0, 3), 3);
        EXPECT_EQ(outgoing_msg->payload_count, 3);

        // Ask for more bytes than the buffer allows
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_model(node1, outgoing_msg, 0, LEN_SNIP_MODEL_BUFFER + 10), strlen(SNIP_MODEL) + 1);
        // Has the null
        EXPECT_EQ(outgoing_msg->payload_count, strlen(SNIP_MODEL) + 1);
    }
}

TEST(ProtocolSnip, load_versions_with_0_request_bytes)
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

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_manufacturer_version_id(node1, outgoing_msg, 0, 0), 0);
        EXPECT_EQ(outgoing_msg->payload_count, 0);

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, SOURCE_ALIAS, SOURCE_ID, DEST_ALIAS, DEST_ID, MTI_SIMPLE_NODE_INFO_REQUEST, 0);
        OpenLcbUtilities_clear_openlcb_message(outgoing_msg);
        EXPECT_EQ(ProtocolSnip_load_user_version_id(node1, outgoing_msg, 0, 0), 0);
        EXPECT_EQ(outgoing_msg->payload_count, 0);
    }
}
