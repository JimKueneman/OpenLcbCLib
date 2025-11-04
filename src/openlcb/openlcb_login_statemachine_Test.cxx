#include "test/main_Test.hxx"

#include "openlcb_login_statemachine.h"

#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_buffer_store.h"
#include "openlcb_utilities.h"

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

void *called_function_ptr = nullptr;

void _update_called_function_ptr(void *function_ptr)
{

    called_function_ptr = (void *)((long long)function_ptr + (long long)called_function_ptr);
}

void _load_initialization_complete(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{

    _update_called_function_ptr((void *)&_load_initialization_complete);
}

void _load_producer_events(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{

    _update_called_function_ptr((void *)&_load_producer_events);
}

void _load_consumer_events(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{

    _update_called_function_ptr((void *)&_load_consumer_events);
}

bool _send_openlcb_msg(openlcb_msg_t *outgoing_msg)
{

    _update_called_function_ptr((void *)&_send_openlcb_msg);

    return true;
}

openlcb_node_t *_openlcb_node_get_first(uint8_t key)
{

    _update_called_function_ptr((void *)&_openlcb_node_get_first);

    return nullptr;
}

openlcb_node_t *_openlcb_node_get_next(uint8_t key)
{

    _update_called_function_ptr((void *)&_openlcb_node_get_next);

    return nullptr;
}

void _process_login_statemachine(openlcb_login_statemachine_info_t *openlcb_statemachine_info)
{

    _update_called_function_ptr((void *)&_process_login_statemachine);
}

bool _handle_outgoing_openlcb_message(void)
{

    _update_called_function_ptr((void *)&_handle_outgoing_openlcb_message);

    return true;
}

bool _handle_try_reenumerate(void)
{

    _update_called_function_ptr((void *)&_handle_try_reenumerate);

    return true;
}

bool _handle_try_enumerate_first_node(void)
{

    _update_called_function_ptr((void *)&_handle_try_enumerate_first_node);

    return true;
}

bool _handle_try_enumerate_next_node(void)
{

    _update_called_function_ptr((void *)&_handle_try_enumerate_next_node);

    return true;
}

const interface_openlcb_login_state_machine_t interface_openlcb_login_state_machine = {

    .load_initialization_complete = &_load_initialization_complete,
    .load_producer_events = &_load_producer_events,
    .load_consumer_events = &_load_consumer_events,

    .send_openlcb_msg = &_send_openlcb_msg,

    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
    .process_login_statemachine = &_process_login_statemachine,

    // For test injection
    .handle_outgoing_openlcb_message = &_handle_outgoing_openlcb_message,
    .handle_try_reenumerate = &_handle_try_reenumerate,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node,

};

const interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{

    called_function_ptr = nullptr;
}

void _global_initialize(void)
{

    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    OpenLcbLoginStateMachine_initialize(&interface_openlcb_login_state_machine);
}

TEST(CanLoginStateMachine, initialize)
{

    _global_initialize();
    _reset_variables();
}

TEST(CanLoginStateMachine, run)
{

#define NODE_ID 0x010203040506

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_login_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;

    statemachine_info.openlcb_node->state.run_state = RUNSTATE_INIT;
    OpenLcbLoginStateMachine_process(&statemachine_info);
    EXPECT_EQ(called_function_ptr, nullptr);

    _reset_variables();

    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    OpenLcbLoginStateMachine_process(&statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_initialization_complete);

    _reset_variables();

    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;
    OpenLcbLoginStateMachine_process(&statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_consumer_events);

    _reset_variables();

    statemachine_info.openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;
    OpenLcbLoginStateMachine_process(&statemachine_info);
    EXPECT_EQ(called_function_ptr, &_load_producer_events);
}