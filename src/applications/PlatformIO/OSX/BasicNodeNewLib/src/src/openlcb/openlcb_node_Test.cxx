#include "test/main_Test.hxx"

#include "openlcb_node.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"

node_parameters_t _node_parameters_main_node = {

    // force overruns for test
    .consumer_count_autocreate = USER_DEFINED_CONSUMER_COUNT + 1,
    .producer_count_autocreate = USER_DEFINED_PRODUCER_COUNT + 1,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Test",
    .snip.model = "Test Model J",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
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

    .cdi =
        {
            // </cdi>
        },

};

void lock_node_list(void)
{
}

void unlock_node_list(void)
{
}

const interface_openlcb_node_t interface_openlcb_node = {
    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _global_initialize(void)
{

    // CanBufferStore_initialize();
    // CanBufferFifo_initialize();
    // OpenLcbBufferStore_initialize();
    // OpenLcbBufferFifo_initialize();
    // OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
}

void _reset_variables(void)
{
}

TEST(OpenLcbNode, initialization)
{

    _global_initialize();
    _reset_variables();

    EXPECT_EQ(OpenLcbNode_get_first(0), nullptr);
    EXPECT_EQ(OpenLcbNode_get_next(0), nullptr);

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    EXPECT_EQ(OpenLcbNode_get_first(0), node1);
    EXPECT_EQ(OpenLcbNode_get_next(0), node2);
}

TEST(OpenLcbNode, buffer_full)
{

    _global_initialize();
    _reset_variables();

    node_id_t node_id = 0x010203040506;
    openlcb_node_t *openlcb_node;

    for (int i = 0; i < USER_DEFINED_NODE_BUFFER_DEPTH; i++)
    {

        openlcb_node = OpenLcbNode_allocate(node_id, &_node_parameters_main_node);
        EXPECT_NE(openlcb_node, nullptr);
        node_id++;
    }

    openlcb_node = OpenLcbNode_allocate(node_id, &_node_parameters_main_node);
    EXPECT_EQ(openlcb_node, nullptr);
}

TEST(OpenLcbNode, _10ms_timer_tick)
{

    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0xAAA;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    EXPECT_EQ(OpenLcbNode_get_first(0), node1);
    EXPECT_EQ(OpenLcbNode_get_next(0), node2);

    EXPECT_EQ(node1->timerticks, 0);
    EXPECT_EQ(node2->timerticks, 0);

    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();
    OpenLcbNode_100ms_timer_tick();

    EXPECT_EQ(node1->timerticks, 5);
    EXPECT_EQ(node2->timerticks, 5);
}