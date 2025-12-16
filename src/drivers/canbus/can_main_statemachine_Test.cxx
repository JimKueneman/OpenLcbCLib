#include "test/main_Test.hxx"

#include "can_main_statemachine.h"

#include "can_utilities.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"

#include "alias_mappings.h"

#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"

#define NODE_ID_1 0x010203040506
#define NODE_ID_2 0x010203040507

#define NODE_ALIAS_1 0x0666
#define NODE_ALIAS_1_HI 0x06
#define NODE_ALIAS_1_LO 0x66

#define NODE_ALIAS_2 0x0999
#define NODE_ALIAS_2_HI 0x09
#define NODE_ALIAS_2_LO 0x99

#define SOURCE_ALIAS 0x06BE
#define SOURCE_ALIAS_HI 0x06
#define SOURCE_ALIAS_LO 0xBE

bool lock_shared_resources_called = false;
bool unlock_shared_resources_called = false;
bool send_can_message_called = false;
bool node_find_node_by_alias_called = false;
bool node_get_first_called = false;
bool node_get_next_called = false;
bool login_statemachine_run_called = false;
bool send_can_message_enabled = true;
bool alias_mapping_get_alias_mapping_info_called = false;
bool handle_duplicate_aliases_called = false;
bool handle_login_outgoing_can_message_called = false;
bool handle_outgoing_can_message_called = false;
bool handle_try_enumerate_first_node_called = false;
bool handle_try_enumerate_next_node_called = false;
bool node_find_node_by_alias_fail = false;

can_msg_t send_can_msg;

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

alias_mapping_info_t alias_mapping_info;

alias_mapping_info_t *_alias_mapping_get_alias_mapping_info(void)
{

    alias_mapping_get_alias_mapping_info_called = true;

    return (AliasMappings_get_alias_mapping_info());
}

void _alias_mapping_unregister(uint16_t alias)
{

    AliasMappings_unregister(alias);
}

void _lock_shared_resources(void)
{

    lock_shared_resources_called = true;
}

void _unlock_shared_resources(void)
{

    unlock_shared_resources_called = true;
}

bool _send_can_message(can_msg_t *msg)
{

    send_can_message_called = true;

    if (send_can_message_enabled)
    {

        CanUtilities_copy_can_message(msg, &send_can_msg);

        return true;
    }

    CanUtilities_clear_can_message(&send_can_msg);

    return false;
}

openlcb_node_t *_node_find_by_alias(uint16_t alias)
{

    node_find_node_by_alias_called = true;

    if (node_find_node_by_alias_fail)
    {

        return NULL;
    }

    return OpenLcbNode_find_by_alias(alias);
}

openlcb_node_t *_openlcb_node_get_first(uint8_t key)
{
    node_get_first_called = true;

    return OpenLcbNode_get_first(key);
}

openlcb_node_t *_openlcb_node_get_next(uint8_t key)
{
    node_get_next_called = true;

    return OpenLcbNode_get_next(key);
}

void _login_statemachine_run(can_statemachine_info_t *can_statemachine_info)
{
    login_statemachine_run_called = true;
}

bool _handle_duplicate_aliases(void)
{
    handle_duplicate_aliases_called = true;

    return CanMainStatemachine_handle_duplicate_aliases();
}

bool _handle_outgoing_can_message(void)
{

    handle_outgoing_can_message_called = true;

    return CanMainStatemachine_handle_outgoing_can_message();
}

bool _handle_login_outgoing_can_message(void)
{

    handle_login_outgoing_can_message_called = true;

    return CanMainStatemachine_handle_login_outgoing_can_message();
}

bool _handle_try_enumerate_first_node(void)
{

    handle_try_enumerate_first_node_called = true;

    return CanMainStatemachine_handle_try_enumerate_first_node();
}

bool _handle_try_enumerate_next_node(void)
{

    handle_try_enumerate_next_node_called = true;

    return CanMainStatemachine_handle_try_enumerate_next_node();
}

const interface_can_main_statemachine_t interface_can_main_statemachine = {

    .alias_mapping_get_alias_mapping_info = &_alias_mapping_get_alias_mapping_info,
    .alias_mapping_unregister = &_alias_mapping_unregister,
    .lock_shared_resources = &_lock_shared_resources,
    .login_statemachine_run = &_login_statemachine_run,
    .openlcb_node_find_by_alias = &_node_find_by_alias,
    .openlcb_node_get_first = &_openlcb_node_get_first,
    .openlcb_node_get_next = &_openlcb_node_get_next,
    .send_can_message = &_send_can_message,
    .unlock_shared_resources = &_unlock_shared_resources,

    .handle_duplicate_aliases = &_handle_duplicate_aliases,
    .handle_login_outgoing_can_message = &_handle_login_outgoing_can_message,
    .handle_outgoing_can_message = &_handle_outgoing_can_message,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node};

const interface_openlcb_node_t interface_openlcb_node = {

};

bool compare_can_msg(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t bytes[])
{
    bool result = (can_msg->identifier == identifier) &&
                  (can_msg->payload_count == payload_size);

    if (!result)
    {

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (can_msg->payload[i] != bytes[i])
        {

            return false;
        }
    }

    return true;
}

void _global_initialize(void)
{

    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanMainStatemachine_initialize(&interface_can_main_statemachine);
}

void _reset_variables(void)
{
    lock_shared_resources_called = false;
    unlock_shared_resources_called = false;
    send_can_message_called = false;
    node_find_node_by_alias_called = false;
    node_get_first_called = false;
    node_get_next_called = false;
    login_statemachine_run_called = false;
    send_can_message_enabled = true;
    alias_mapping_get_alias_mapping_info_called = false;
    handle_duplicate_aliases_called = false;
    handle_login_outgoing_can_message_called = false;
    handle_outgoing_can_message_called = false;
    handle_try_enumerate_first_node_called = false;
    handle_try_enumerate_next_node_called = false;
    node_find_node_by_alias_fail = false;
}

TEST(CanMainStatemachine, intialization)
{
    _global_initialize();
    _reset_variables();
}

TEST(CanMainStatemachine, run_empty_node_list)
{
    _global_initialize();
    _reset_variables();

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
}

TEST(CanMainStatemachine, run_node_list_single)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *openlcb_node_1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == openlcb_node_1);

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_TRUE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == nullptr);
}

TEST(CanMainStatemachine, run_node_list_mulitiple)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *openlcb_node_1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node_t *openlcb_node_2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == openlcb_node_1);

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_TRUE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == openlcb_node_2);

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_TRUE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == nullptr);

    openlcb_node_1->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;
    openlcb_node_2->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == openlcb_node_1);

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_TRUE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == openlcb_node_2);

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_TRUE(handle_try_enumerate_first_node_called);
    EXPECT_TRUE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(CanMainStateMachine_get_can_statemachine_info()->openlcb_node == nullptr);
}

TEST(CanMainStatemachine, handle_outgoing_can_message)
{
    can_msg_t *can_msg;

    _global_initialize();
    _reset_variables();

    // Load up 2 message
    can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);
    CanUtilities_load_can_message(can_msg, 0x19490AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CanBufferFifo_push(can_msg);
    can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);
    CanUtilities_load_can_message(can_msg, 0x19170AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CanBufferFifo_push(can_msg);

    send_can_message_enabled = true;

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x19490AAA, 0, nullptr));
    _reset_variables();

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);
    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x19170AAA, 0, nullptr));
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
}

TEST(CanMainStatemachine, handle_outgoing_can_message_with_tx_fail)
{

    can_msg_t *can_msg;

    _global_initialize();
    _reset_variables();

    // Load up 2 message
    can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);
    CanUtilities_load_can_message(can_msg, 0x19490AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CanBufferFifo_push(can_msg);
    can_msg = CanBufferStore_allocate_buffer();
    EXPECT_NE(can_msg, nullptr);
    CanUtilities_load_can_message(can_msg, 0x19170AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CanBufferFifo_push(can_msg);

    send_can_message_enabled = false;

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_EQ(CanBufferStore_messages_allocated(), 2);

    send_can_message_enabled = true;

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x19490AAA, 0, nullptr));
    _reset_variables();

    _reset_variables();
    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x19170AAA, 0, nullptr));
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
}

TEST(CanMainStatemachine, duplicate_alias)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    node1->state.permitted = true;
    node1->state.initialized = true;
    node1->state.run_state = RUNSTATE_RUN;

    alias_mapping_t *alias_mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    alias_mapping->is_duplicate = true;
    alias_mapping->is_permitted = true;
    AliasMappings_set_has_duplicate_alias_flag();

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_FALSE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);

    EXPECT_FALSE(node1->state.permitted);
    EXPECT_FALSE(node1->state.initialized);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
    EXPECT_FALSE(node1->state.firmware_upgrade_active);
    EXPECT_FALSE(node1->state.resend_datagram);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_GENERATE_SEED);

    _global_initialize();
    _reset_variables();

    node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    node1->state.permitted = true;
    node1->state.initialized = true;
    node1->state.run_state = RUNSTATE_RUN;
    node1->last_received_datagram = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    alias_mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    alias_mapping->is_duplicate = true;
    alias_mapping->is_permitted = true;
    AliasMappings_set_has_duplicate_alias_flag();

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_FALSE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);

    EXPECT_FALSE(node1->state.permitted);
    EXPECT_FALSE(node1->state.initialized);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
    EXPECT_FALSE(node1->state.firmware_upgrade_active);
    EXPECT_FALSE(node1->state.resend_datagram);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
}

TEST(CanMainStatemachine, duplicate_alias_fail_to_find_mapping)
{
    _global_initialize();
    _reset_variables();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    node1->state.permitted = true;
    node1->state.initialized = true;
    node1->state.run_state = RUNSTATE_RUN;

    alias_mapping_t *alias_mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    alias_mapping->is_duplicate = true;
    alias_mapping->is_permitted = true;
    AliasMappings_set_has_duplicate_alias_flag();

    node_find_node_by_alias_fail = true;

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_FALSE(handle_outgoing_can_message_called);
    EXPECT_FALSE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);

    // Nothing happend as the find node by alias failed
    EXPECT_TRUE(node1->state.permitted);
    EXPECT_TRUE(node1->state.initialized);
    EXPECT_FALSE(node1->state.duplicate_id_detected);
    EXPECT_FALSE(node1->state.firmware_upgrade_active);
    EXPECT_FALSE(node1->state.resend_datagram);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_EQ(node1->state.run_state, RUNSTATE_RUN);
}

TEST(CanMainStatemachine, handle_login_outgoing_can_message)
{

    _global_initialize();
    _reset_variables();

    can_statemachine_info_t *can_statemachine_info = CanMainStateMachine_get_can_statemachine_info();

    can_statemachine_info->login_outgoing_can_msg_valid = true;
    CanUtilities_load_can_message(can_statemachine_info->login_outgoing_can_msg, 0x19490AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    send_can_message_enabled = true;

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x19490AAA, 0, nullptr));
}

TEST(CanMainStatemachine, handle_login_outgoing_can_message_with_tx_fail)
{

    _global_initialize();
    _reset_variables();

    can_statemachine_info_t *can_statemachine_info = CanMainStateMachine_get_can_statemachine_info();

    can_statemachine_info->login_outgoing_can_msg_valid = true;
    CanUtilities_load_can_message(can_statemachine_info->login_outgoing_can_msg, 0x19490AAA, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    send_can_message_enabled = false;

    CanMainStateMachine_run();

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_TRUE(alias_mapping_get_alias_mapping_info_called);
    EXPECT_TRUE(handle_duplicate_aliases_called);
    EXPECT_TRUE(handle_outgoing_can_message_called);
    EXPECT_TRUE(handle_login_outgoing_can_message_called);
    EXPECT_FALSE(handle_try_enumerate_first_node_called);
    EXPECT_FALSE(handle_try_enumerate_next_node_called);
    EXPECT_TRUE(send_can_message_called);

    EXPECT_TRUE(compare_can_msg(&send_can_msg, 0x00, 0, nullptr));
}
