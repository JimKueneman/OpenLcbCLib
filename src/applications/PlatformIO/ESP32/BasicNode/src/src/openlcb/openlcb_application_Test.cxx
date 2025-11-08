#include "test/main_Test.hxx"

#include "openlcb_application.h"

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

#define CONFIG_MEM_START_ADDRESS 0x100
#define CONFIG_MEM_NODE_ADDRESS_ALLOCATION 0x200

typedef enum
{

    SEND_MSG_PC_REPORT,
    SEND_MSG_TEACH

} send_msg_enum_t;

node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = 5,
    .producer_count_autocreate = 5,

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
    .address_space_config_memory.low_address_valid = 0,                                // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
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

bool fail_transmit_openlcb_msg = false;
bool fail_configuration_read = false;
bool fail_configuration_write = false;
openlcb_msg_t *local_sent_msg = nullptr;
send_msg_enum_t send_msg_enum = SEND_MSG_PC_REPORT;
configuration_memory_buffer_t write_buffer;

bool _transmit_openlcb_message(openlcb_msg_t *openlcb_msg)
{

    if (fail_transmit_openlcb_msg)
    {

        local_sent_msg = nullptr;
        return false;
    }

    switch (send_msg_enum)
    {

    case SEND_MSG_PC_REPORT:

        EXPECT_EQ(openlcb_msg->mti, MTI_PC_EVENT_REPORT);
        EXPECT_EQ(openlcb_msg->payload_count, 8);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg), EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH);

        break;

    case SEND_MSG_TEACH:

        EXPECT_EQ(openlcb_msg->mti, MTI_EVENT_LEARN);
        EXPECT_EQ(openlcb_msg->payload_count, 8);
        EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg), EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH);

        break;
    }

    local_sent_msg = openlcb_msg;
    return true;
}

uint16_t _configuration_memory_read(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    EXPECT_EQ(address, 0x0000FFFF);
    EXPECT_EQ(count, 0x10);

    if (fail_configuration_read)
    {

        return 0;
    }

    for (int i = 0; i < count; i++)
    {

        (*buffer)[i] = i;
    }

    return count;
}

uint16_t _configuration_memory_write(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{
    EXPECT_EQ(address, 0x0000FFFF);
    EXPECT_EQ(count, 0x10);

    if (fail_configuration_write)
    {

        return 0;
    }

    for (int i = 0; i < count; i++)
    {

        write_buffer[i] = (*buffer)[i];
    }

    return count;
}

interface_openlcb_application_t interface_openlcb_application = {

    .send_openlcb_msg = &_transmit_openlcb_message,
    .configuration_memory_read = &_configuration_memory_read,
    .configuration_memory_write = &_configuration_memory_write};

interface_openlcb_application_t interface_openlcb_application_nulls = {

    .send_openlcb_msg = NULL,
    .configuration_memory_read = NULL,
    .configuration_memory_write = NULL};

interface_openlcb_node_t interface_openlcb_node = {

};

void _reset_variables(void)
{

    fail_transmit_openlcb_msg = false;
    local_sent_msg = nullptr;
    send_msg_enum = SEND_MSG_PC_REPORT;
    fail_configuration_read = false;
    fail_configuration_write = false;

    for (int i = 0; i < sizeof(write_buffer); i++)
    {

        write_buffer[i] = 0x00;
    }
}

void _global_initialize(void)
{

    OpenLcbApplication_initialize(&interface_openlcb_application);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_nulls(void)
{

    OpenLcbApplication_initialize(&interface_openlcb_application_nulls);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(OpenLcbApplication, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(OpenLcbApplication, clear_consumer_eventids)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    EXPECT_EQ(node1->consumers.count, 5);
    EXPECT_EQ(node1->producers.count, 5);

    OpenLcbApplication_clear_consumer_eventids(node1);

    EXPECT_EQ(node1->consumers.count, 0);
    EXPECT_EQ(node1->producers.count, 5);
}

TEST(OpenLcbApplication, clear_producer_eventids)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    EXPECT_EQ(node1->consumers.count, 5);
    EXPECT_EQ(node1->producers.count, 5);

    OpenLcbApplication_clear_producer_eventids(node1);

    EXPECT_EQ(node1->consumers.count, 5);
    EXPECT_EQ(node1->producers.count, 0);
}

TEST(OpenLcbApplication, register_consumer_eventid)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    // ************************************************************************
    // Register a new event on top of what is defined in the parameters
    // ************************************************************************

    EXPECT_EQ(node1->consumers.count, 5);
    EXPECT_EQ(node1->producers.count, 5);

    OpenLcbApplication_register_consumer_eventid(node1, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);

    EXPECT_EQ(node1->consumers.count, 6);

    EXPECT_EQ(node1->consumers.list[5].event, EVENT_ID_EMERGENCY_OFF);
    EXPECT_EQ(node1->consumers.list[5].status, EVENT_STATUS_SET);

    // ************************************************************************
    // Register a new event with zero to start with
    // ************************************************************************

    OpenLcbApplication_clear_producer_eventids(node1);
    OpenLcbApplication_clear_consumer_eventids(node1);

    EXPECT_EQ(node1->consumers.count, 0);
    EXPECT_EQ(node1->producers.count, 0);

    EXPECT_EQ(OpenLcbApplication_register_consumer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR), 1);
    EXPECT_EQ(node1->consumers.count, 1);

    EXPECT_EQ(node1->consumers.list[0].event, EVENT_ID_EMERGENCY_STOP);
    EXPECT_EQ(node1->consumers.list[0].status, EVENT_STATUS_CLEAR);

    // ************************************************************************
    // Check that error is returned if there is not enough slots for another event id
    // ************************************************************************
    OpenLcbApplication_clear_consumer_eventids(node1);

    for (int i = 0; i < USER_DEFINED_CONSUMER_COUNT; i++)
    {

        OpenLcbApplication_register_consumer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR);
    }

    EXPECT_EQ(OpenLcbApplication_register_consumer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR), 0xFFFF);
}

TEST(OpenLcbApplication, register_producer_eventid)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    // ************************************************************************
    // Register a new event on top of what is defined in the parameters
    // ************************************************************************

    EXPECT_EQ(node1->consumers.count, 5);
    EXPECT_EQ(node1->producers.count, 5);

    OpenLcbApplication_register_producer_eventid(node1, EVENT_ID_EMERGENCY_OFF, EVENT_STATUS_SET);

    EXPECT_EQ(node1->producers.count, 6);

    EXPECT_EQ(node1->producers.list[5].event, EVENT_ID_EMERGENCY_OFF);
    EXPECT_EQ(node1->producers.list[5].status, EVENT_STATUS_SET);

    // ************************************************************************
    // Register a new event with zero to start with
    // ************************************************************************

    OpenLcbApplication_clear_producer_eventids(node1);
    OpenLcbApplication_clear_consumer_eventids(node1);

    EXPECT_EQ(node1->consumers.count, 0);
    EXPECT_EQ(node1->producers.count, 0);

    OpenLcbApplication_register_producer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR);

    EXPECT_EQ(node1->producers.count, 1);

    EXPECT_EQ(node1->producers.list[0].event, EVENT_ID_EMERGENCY_STOP);
    EXPECT_EQ(node1->producers.list[0].status, EVENT_STATUS_CLEAR);

    // ************************************************************************
    // Check that error is returned if there is not enough slots for another event id
    // ************************************************************************
    OpenLcbApplication_clear_producer_eventids(node1);

    for (int i = 0; i < USER_DEFINED_PRODUCER_COUNT; i++)
    {

        OpenLcbApplication_register_producer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR);
    }

    EXPECT_EQ(OpenLcbApplication_register_producer_eventid(node1, EVENT_ID_EMERGENCY_STOP, EVENT_STATUS_CLEAR), 0xFFFF);
}

TEST(OpenLcbApplication, send_event_pc_report)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    send_msg_enum = SEND_MSG_PC_REPORT;
    EXPECT_TRUE(OpenLcbApplication_send_event_pc_report(node1, EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH));

    fail_transmit_openlcb_msg = true;
    EXPECT_FALSE(OpenLcbApplication_send_event_pc_report(node1, EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH));
    EXPECT_EQ(local_sent_msg, nullptr);
    fail_transmit_openlcb_msg = false;
}

TEST(OpenLcbApplication, send_teach_event)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    send_msg_enum = SEND_MSG_TEACH;
    EXPECT_TRUE(OpenLcbApplication_send_teach_event(node1, EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH));

    fail_transmit_openlcb_msg = true;
    EXPECT_FALSE(OpenLcbApplication_send_teach_event(node1, EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH));
    EXPECT_EQ(local_sent_msg, nullptr);
    fail_transmit_openlcb_msg = false;
}

TEST(OpenLcbApplication, read_configuration_memory)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    configuration_memory_buffer_t buffer;

    EXPECT_EQ(OpenLcbApplication_read_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0x10);

    for (int i = 0; i < 0x10; i++)
    {

        EXPECT_EQ(buffer[i], i);
        buffer[i] = 0x00;
    }

    fail_configuration_read = true;

    EXPECT_EQ(OpenLcbApplication_read_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0x00);

    for (int i = 0; i < 0x10; i++)
    {

        EXPECT_EQ(buffer[i], 0x00);
    }
}

TEST(OpenLcbApplication, write_configuration_memory)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    configuration_memory_buffer_t buffer;

    for (int i = 0; i < 0x10; i++)
    {

        write_buffer[i] = 0x00;
        buffer[i] = i;
    }

    EXPECT_EQ(OpenLcbApplication_write_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0x10);

    for (int i = 0; i < 0x10; i++)
    {

        EXPECT_EQ(write_buffer[i], buffer[i]);
    }

    fail_configuration_write = true;

    EXPECT_EQ(OpenLcbApplication_write_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0x00);
}

TEST(OpenLcbApplication, read_configuration_memory_null)
{

    _reset_variables();
    _global_initialize_nulls();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    configuration_memory_buffer_t buffer;

    EXPECT_EQ(OpenLcbApplication_read_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0xFFFF);
}

TEST(OpenLcbApplication, write_configuration_memory_null)
{

    _reset_variables();
    _global_initialize_nulls();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    EXPECT_NE(node1, nullptr);

    configuration_memory_buffer_t buffer;

    EXPECT_EQ(OpenLcbApplication_write_configuration_memory(node1, 0x0000FFFF, 0x10, &buffer), 0xFFFF);
}