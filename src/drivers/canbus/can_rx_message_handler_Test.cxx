#include "test/main_Test.hxx"

#include "can_types.h"
#include "can_utilities.h"
#include "can_rx_message_handler.h"
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_buffer_fifo.h"
#include "../../openlcb/openlcb_buffer_list.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../drivers/common/alias_mappings.h"
#include "../../drivers/common/can_main_statemachine.h"
#include "../../drivers/common/can_login_statemachine.h"

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

bool fail_buffer = false;
bool force_fail_allocate = false;

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

openlcb_msg_t *openlcb_buffer_store_allocate_buffer(payload_type_enum payload_type)
{
    // only fail larger buffers, assume BASIC buffers are always avaialble

    if (force_fail_allocate)
    {

        return nullptr;
    }

    if (!fail_buffer || (payload_type == BASIC))
    {

        return OpenLcbBufferStore_allocate_buffer(payload_type);
    }
    else
    {

        return nullptr;
    }
}

can_msg_t *can_buffer_store_allocate_buffer(void)
{

    if (!fail_buffer)
    {

        return CanBufferStore_allocate_buffer();
    }
    else
    {

        return nullptr;
    }
}

const interface_can_rx_message_handler_t interface_rx_message_handler = {

    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag,
    .openlcb_buffer_store_allocate_buffer = &openlcb_buffer_store_allocate_buffer,
    .can_buffer_store_allocate_buffer = &can_buffer_store_allocate_buffer

};

const interface_openlcb_node_t interface_openlcb_node = {

};

static void _lock_shared_resources(void)
{
}

static void _unlock_shared_resources(void)
{
}

static bool _send_can_message(can_msg_t *can_msg)
{

    return true;
}

void _can_login_statemachine_run(can_statemachine_info_t *can_statemachine_info)
{
}

bool _handle_duplicate_aliases(void)
{

    return CanMainStatemachine_handle_duplicate_aliases();
}

bool _handle_login_outgoing_can_message(void)
{

    return CanMainStatemachine_handle_login_outgoing_can_message();
}

bool _handle_outgoing_can_message(void)
{

    return CanMainStatemachine_handle_outgoing_can_message();
}

bool _handle_try_enumerate_first_node(void)
{

    return CanMainStatemachine_handle_try_enumerate_first_node();
}

bool _handle_try_enumerate_next_node(void)
{

    return CanMainStatemachine_handle_try_enumerate_next_node();
}

const interface_can_main_statemachine_t interface_can_main_statemachine = {

    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_unregister = &AliasMappings_unregister,
    .lock_shared_resources = &_lock_shared_resources,
    .login_statemachine_run = &_can_login_statemachine_run,
    .openlcb_node_find_by_alias = &OpenLcbNode_find_by_alias,
    .openlcb_node_get_first = &OpenLcbNode_get_first,
    .openlcb_node_get_next = &OpenLcbNode_get_next,
    .send_can_message = &_send_can_message,
    .unlock_shared_resources = &_unlock_shared_resources,

    .handle_duplicate_aliases = &_handle_duplicate_aliases,
    .handle_login_outgoing_can_message = &_handle_login_outgoing_can_message,
    .handle_outgoing_can_message = &_handle_outgoing_can_message,
    .handle_try_enumerate_first_node = &_handle_try_enumerate_first_node,
    .handle_try_enumerate_next_node = &_handle_try_enumerate_next_node};

void _global_initialize(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    AliasMappings_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);
    CanRxMessageHandler_initialize(&interface_rx_message_handler);
    CanMainStatemachine_initialize(&interface_can_main_statemachine);
}

void _global_reset_variables(void)
{

    fail_buffer = false;
    force_fail_allocate = false;
}

bool _compare_can_msg(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t bytes[])
{

    bool result = (can_msg->identifier == identifier) &&
                  (can_msg->payload_count == payload_size);

    if (!result)
    {
        fprintf(stderr, "\n _compare_can_msg failed\n\n");
        fprintf(stderr, "can_msg->identifier: 0x%08X, identifier = 0x%04X\n", can_msg->identifier, identifier);
        fprintf(stderr, "can_msg->payload_count: %d, payload_size = %d\n", can_msg->payload_count, payload_size);

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (can_msg->payload[i] != bytes[i])
        {

            fprintf(stderr, "can_msg->payload[%d]: %02X, bytes[%d]: 0x%02X\n  ", i, can_msg->payload[i], i, bytes[i]);

            return false;
        }
    }

    return true;
}

bool _compare_openlcb_msg(openlcb_msg_t *openlcb_msg, uint16_t mti, node_id_t source_id, uint16_t source_alias, node_id_t dest_id, uint16_t dest_alias, uint16_t payload_size, uint8_t bytes[])
{

    bool result = (openlcb_msg->mti == mti) &&
                  (openlcb_msg->source_id == source_id) &&
                  (openlcb_msg->source_alias == source_alias) &&
                  (openlcb_msg->dest_id == dest_id) &&
                  (openlcb_msg->dest_alias == dest_alias) &&
                  (openlcb_msg->payload_count == payload_size);

    if (!result)
    {

        fprintf(stderr, "\n _compare_openlcb_msg failed\n\n");
        fprintf(stderr, "openlcb_msg->mti: 0x%04X, mti = 0x%04X\n", openlcb_msg->mti, mti);
        fprintf(stderr, "openlcb_msg->source_alias: 0x%04X, source_alias: 0x%04X\n", openlcb_msg->source_alias, source_alias);
        fprintf(stderr, "openlcb_msg->source_id: 0x%08llX, source_id: 0x%08llX\n", openlcb_msg->source_id, source_id);
        fprintf(stderr, "openlcb_msg->dest_alias: 0x%04X, dest_alias: 0x%04X\n", openlcb_msg->dest_alias, dest_alias);
        fprintf(stderr, "openlcb_msg->dest_id: 0x%08llX, dest_id: 0x%08llX\n", openlcb_msg->dest_id, dest_id);
        fprintf(stderr, "openlcb_msg->payload_count: %d, payload_size: %d\n", openlcb_msg->payload_count, payload_size);
        fprintf(stderr, "\n\n\n\n Test");

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (*openlcb_msg->payload[i] != bytes[i])
        {
            fprintf(stderr, "*openlcb_msg->payload[%d]: %02X, bytes[%d]: 0x%02X\n  ", i, *openlcb_msg->payload[i], i, bytes[i]);

            return false;
        }
    }

    return true;
}

void _test_for_all_buffer_lists_empty(void)
{

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
}

void _test_for_all_buffer_lists_not_empty(void)
{

    EXPECT_FALSE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
}

void _test_for_all_buffer_stores_empty(void)
{

    if (CanBufferStore_messages_allocated() != 0)
    {

        fprintf(stderr, "\nCanBufferStore_messages_allocated() is not empty\n\n");
    }

    if (OpenLcbBufferStore_basic_messages_allocated() != 0)
    {

        fprintf(stderr, "\nOpenLcbBufferStore_basic_messages_allocated() is not empty\n\n");
    }

    if (OpenLcbBufferStore_snip_messages_allocated() != 0)
    {

        fprintf(stderr, "\nOpenLcbBufferStore_snip_messages_allocated() is not empty\n\n");
    }

    if (OpenLcbBufferStore_datagram_messages_allocated() != 0)
    {

        fprintf(stderr, "\nOpenLcbBufferStore_datagram_messages_allocated() is not empty\n\n");
    }

    if (OpenLcbBufferStore_stream_messages_allocated() != 0)
    {

        fprintf(stderr, "\nOpenLcbBufferStore_stream_messages_allocated() is not empty\n\n");
    }

    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_snip_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
}

void _test_for_only_can_buffer_fifo_not_empty(void)
{

    EXPECT_FALSE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
}

void _test_for_only_openlcb_buffer_fifo_not_empty(void)
{

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
}

void _test_for_only_openlcb_buffer_list_not_empty(void)
{

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
}

void _test_for_openlcb_buffer_list_and_openlcb_buffer_fifo_not_empty(void)
{

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
}

TEST(CanRxMessageHandler, initialize)
{

    _global_reset_variables();
    _global_initialize();
}

TEST(CanRxMessageHandler, cid_frame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // No conflict
    // ************************************************************************

    can_msg.identifier = 0x17050000 | SOURCE_ALIAS;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    EXPECT_TRUE(openlcb_node1->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node1->state.permitted);
    EXPECT_TRUE(openlcb_node1->state.initialized);
    EXPECT_TRUE(openlcb_node2->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node2->state.permitted);
    EXPECT_TRUE(openlcb_node2->state.initialized);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);

    // ************************************************************************

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x16050000 | SOURCE_ALIAS;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    EXPECT_TRUE(openlcb_node1->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node1->state.permitted);
    EXPECT_TRUE(openlcb_node1->state.initialized);
    EXPECT_TRUE(openlcb_node2->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node2->state.permitted);
    EXPECT_TRUE(openlcb_node2->state.initialized);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);
    // ************************************************************************

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x15050000 | SOURCE_ALIAS;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    EXPECT_TRUE(openlcb_node1->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node1->state.permitted);
    EXPECT_TRUE(openlcb_node1->state.initialized);
    EXPECT_TRUE(openlcb_node2->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node2->state.permitted);
    EXPECT_TRUE(openlcb_node2->state.initialized);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);
    // ************************************************************************

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x17040000 | SOURCE_ALIAS;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    EXPECT_TRUE(openlcb_node1->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node1->state.permitted);
    EXPECT_TRUE(openlcb_node1->state.initialized);
    EXPECT_TRUE(openlcb_node2->state.run_state == RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node2->state.permitted);
    EXPECT_TRUE(openlcb_node2->state.initialized);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);
    EXPECT_NE(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);
    // ************************************************************************

    // ************************************************************************
    // Cause a conflict CID 7
    // ************************************************************************
    can_msg.identifier = 0x17070000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10700000 | NODE_ALIAS_1), 0, nullptr));
    CanBufferStore_free_buffer(outgoing_can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // Cause a conflict CID 6
    // ************************************************************************
    can_msg.identifier = 0x17060000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10700000 | NODE_ALIAS_1), 0, nullptr));
    CanBufferStore_free_buffer(outgoing_can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // Cause a conflict CID 5
    // ************************************************************************
    can_msg.identifier = 0x17050000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10700000 | NODE_ALIAS_1), 0, nullptr));
    CanBufferStore_free_buffer(outgoing_can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // Cause a conflict CID 4
    // ************************************************************************
    can_msg.identifier = 0x17040000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_cid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10700000 | NODE_ALIAS_1), 0, nullptr));
    CanBufferStore_free_buffer(outgoing_can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************
}

TEST(CanRxMessageHandler, rid_frame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x10700000 | (NODE_ALIAS_1 + 1);
    can_msg.payload_count = 0;
    CanRxMessageHandler_rid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, but not permittted
    // ************************************************************************
    openlcb_node1->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    openlcb_node1->state.permitted = false;
    openlcb_node1->state.initialized = false;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_rid_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, permittted on node1
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_rid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_1), 6, bytes));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);
    // ************************************************************************

    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_2;
    can_msg.payload_count = 0;
    CanRxMessageHandler_rid_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes20[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_2), 6, bytes20));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node2->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node2->state.permitted);
    EXPECT_FALSE(openlcb_node2->state.initialized);
    EXPECT_FALSE(openlcb_node2->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node2->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node2->state.resend_datagram);
    EXPECT_FALSE(openlcb_node2->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node2->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node2->alias, 0x00);
    // ************************************************************************
}

TEST(CanRxMessageHandler, amd_frame)
{
    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x10700000 | (NODE_ALIAS_1 + 1);
    can_msg.payload_count = 0;
    CanRxMessageHandler_amd_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, but not permittted
    // ************************************************************************
    openlcb_node1->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    openlcb_node1->state.permitted = false;
    openlcb_node1->state.initialized = false;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amd_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, permittted on node1
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amd_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_1), 6, bytes));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);
    // ************************************************************************

    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_2;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amd_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes20[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_2), 6, bytes20));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node2->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node2->state.permitted);
    EXPECT_FALSE(openlcb_node2->state.initialized);
    EXPECT_FALSE(openlcb_node2->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node2->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node2->state.resend_datagram);
    EXPECT_FALSE(openlcb_node2->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node2->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node2->alias, 0x00);
    // ************************************************************************
}

TEST(CanRxMessageHandler, ame_frame)
{

    can_msg_t can_msg;
    //  openlcb_msg_t *openlcb_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // Not a conflict nor for us
    // ************************************************************************

    CanUtilities_load_can_message(&can_msg, (0x10702000 | (NODE_ALIAS_1 + 1)), 6, 0x99, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x00);
    CanRxMessageHandler_ame_frame(&can_msg);

    // Process the any set flag for duplicate
    CanMainStateMachine_run();

    // Test that the nodes are untouched
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node1->state.permitted);
    EXPECT_TRUE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, NODE_ALIAS_1);

    EXPECT_EQ(openlcb_node2->state.run_state, RUNSTATE_RUN);
    EXPECT_TRUE(openlcb_node2->state.permitted);
    EXPECT_TRUE(openlcb_node2->state.initialized);
    EXPECT_FALSE(openlcb_node2->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node2->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node2->state.resend_datagram);
    EXPECT_FALSE(openlcb_node2->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node2->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node2->alias, NODE_ALIAS_2);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, but not permittted
    // ************************************************************************
    openlcb_node1->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    openlcb_node1->state.permitted = false;
    openlcb_node1->state.initialized = false;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, permittted on node1
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_ame_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_1), 6, bytes));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    CanBufferStore_free_buffer(outgoing_can_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // No conflict request one of our nodes
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    CanUtilities_load_can_message(&can_msg, (0x10702000 | (NODE_ALIAS_1 + 1)), 6, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x00);
    CanRxMessageHandler_ame_frame(&can_msg);
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes10[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10701000 | NODE_ALIAS_2), 6, bytes10));

    CanBufferStore_free_buffer(outgoing_can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    //************************************************************************

    // ************************************************************************
    // No conflict
    // Request all of our nodes
    // ************************************************************************

    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    CanUtilities_load_can_message(&can_msg, (0x10702000 | (NODE_ALIAS_1 + 1)), 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanRxMessageHandler_ame_frame(&can_msg);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes11[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10701000 | NODE_ALIAS_1), 6, bytes11));

    CanBufferStore_free_buffer(outgoing_can_msg);

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    uint8_t bytes12[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10701000 | NODE_ALIAS_2), 6, bytes12));

    CanBufferStore_free_buffer(outgoing_can_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************
}

TEST(CanRxMessageHandler, amr_frame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x10700000 | (NODE_ALIAS_1 + 1);
    can_msg.payload_count = 0;
    CanRxMessageHandler_amr_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, but not permittted
    // ************************************************************************
    openlcb_node1->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    openlcb_node1->state.permitted = false;
    openlcb_node1->state.initialized = false;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amr_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, permittted on node1
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amr_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_1), 6, bytes));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);
    // ************************************************************************

    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_2;
    can_msg.payload_count = 0;
    CanRxMessageHandler_amr_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes20[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_2), 6, bytes20));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node2->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node2->state.permitted);
    EXPECT_FALSE(openlcb_node2->state.initialized);
    EXPECT_FALSE(openlcb_node2->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node2->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node2->state.resend_datagram);
    EXPECT_FALSE(openlcb_node2->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node2->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node2->alias, 0x00);
    // ************************************************************************
}

TEST(CanRxMessageHandler, handle_single_frame_frame_buffer_fail)
{

    can_msg_t can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    openlcb_node_t *node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    node2->alias = NODE_ALIAS_2;

    // ************************************************************************
    // Addressed to us but can't allocate a buffer
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x19668000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    force_fail_allocate = true;
    CanRxMessageHandler_single_frame(&can_msg, 2, BASIC);
    force_fail_allocate = false;
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, handle_single_frame_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    openlcb_node_t *node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    node2->alias = NODE_ALIAS_2;

    // ************************************************************************
    // Addressed to us but can't allocate a buffer
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    // ************************************************************************
    // _test_for_all_buffer_lists_empty();
    // _test_for_all_buffer_stores_empty();
    // CanUtilities_load_can_message(&can_msg, (0x19668000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    // fail_buffer = true;
    //  CanRxMessageHandler_single_frame(&can_msg, 2, BASIC);
    //  fail_buffer = false;
    // _test_for_all_buffer_lists_empty();
    // _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Addressed message to us
    // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x19668000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
    CanRxMessageHandler_single_frame(&can_msg, 2, BASIC);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes[6] = {0x54, 0x58, 0x20, 0x00, 0x00, 0x00};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_PROTOCOL_SUPPORT_REPLY, 0x0000, SOURCE_ALIAS, 0x0000, NODE_ALIAS_2, 6, bytes));
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    //************************************************************************

    // ************************************************************************
    // Unaddressed message to everyone
    // [[195476be]  Producer Identified Unknown for EventID:05.01.01.01.07.FF.00.25
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, (0x19547000 | SOURCE_ALIAS), 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25);
    CanRxMessageHandler_single_frame(&can_msg, 0, BASIC);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes1[8] = {0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_PRODUCER_IDENTIFIED_UNKNOWN, 0x0000, SOURCE_ALIAS, 0x0000, 0x0000, 8, bytes1));
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************

    // ************************************************************************
    // Addressed Datagram message to us
    // [1a5556eb]f 20 61 00 00 00 00 08   ]  Datagram: (7) 20.61.0.0.0.0.8
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, (0x1A000000 | (NODE_ALIAS_2 << 12) | SOURCE_ALIAS), 7, 0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00);
    CanRxMessageHandler_single_frame(&can_msg, 0, BASIC);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes2[7] = {0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_DATAGRAM, 0x0000, SOURCE_ALIAS, 0x0000, NODE_ALIAS_2, 7, bytes2));
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************
}

TEST(CanRxMessageHandler, handle_first_frame_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    openlcb_node_t *node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    node2->alias = NODE_ALIAS_2;

    // ************************************************************************
    //  SNIP.......
    // ************************************************************************

    // ************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x19a08000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_first_frame(&can_msg, 2, SNIP);
    _test_for_only_openlcb_buffer_list_not_empty();

    openlcb_msg = OpenLcbBufferList_find(SOURCE_ALIAS, NODE_ALIAS_2, MTI_SIMPLE_NODE_INFO_REPLY);
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes[6] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_SIMPLE_NODE_INFO_REPLY, 0x0000, SOURCE_ALIAS, 0x0000, NODE_ALIAS_2, 6, bytes));
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // SNIP addressed to us but there is one already sent
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, (0x19a08000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_first_frame(&can_msg, 2, SNIP);
    _test_for_only_openlcb_buffer_list_not_empty();

    CanRxMessageHandler_first_frame(&can_msg, 2, SNIP);
    _test_for_openlcb_buffer_list_and_openlcb_buffer_fifo_not_empty();

    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes1[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x42}; // define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_OPTIONAL_INTERACTION_REJECTED, 0x00, NODE_ALIAS_2, 0x00, SOURCE_ALIAS, 4, bytes1));
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    openlcb_msg = OpenLcbBufferList_find(SOURCE_ALIAS, NODE_ALIAS_2, MTI_SIMPLE_NODE_INFO_REPLY);
    EXPECT_NE(openlcb_msg, nullptr);
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // SNIP addressed to us but we can't allocate a buffer
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    fail_buffer = true;
    CanUtilities_load_can_message(&can_msg, (0x19a08000 | SOURCE_ALIAS), 8, (MULTIFRAME_FIRST | NODE_ALIAS_2_HI), NODE_ALIAS_2_LO, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_first_frame(&can_msg, 2, SNIP);
    fail_buffer = false;
    _test_for_only_openlcb_buffer_fifo_not_empty();

    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
    uint8_t bytes10[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x20};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_OPTIONAL_INTERACTION_REJECTED, 0x00, NODE_ALIAS_2, 0x00, SOURCE_ALIAS, 4, bytes10));
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    //  Datagram.......
    // ************************************************************************

    // ************************************************************************
    // Datagram addressed to us
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x1b000000 | (NODE_ALIAS_1 << 12) | SOURCE_ALIAS), 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    _test_for_only_openlcb_buffer_list_not_empty();

    openlcb_msg = OpenLcbBufferList_find(SOURCE_ALIAS, NODE_ALIAS_1, MTI_DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes3[8] = {0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, 0x1C48, 0x0000, SOURCE_ALIAS, 0x0000, NODE_ALIAS_1, 8, bytes3));
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, (0x1b000000 | (NODE_ALIAS_1 << 12) | SOURCE_ALIAS), 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    _test_for_only_openlcb_buffer_list_not_empty();

    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    _test_for_openlcb_buffer_list_and_openlcb_buffer_fifo_not_empty();

    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    uint8_t bytes4[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x42}; // define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_DATAGRAM_REJECTED_REPLY, 0x00, NODE_ALIAS_1, 0x00, SOURCE_ALIAS, 4, bytes4));
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    openlcb_msg = OpenLcbBufferList_find(SOURCE_ALIAS, NODE_ALIAS_1, MTI_DATAGRAM);
    EXPECT_NE(openlcb_msg, nullptr);
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // DATAGRAM addressed to us but we can't allocate a buffer
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************

    fail_buffer = true;
    CanUtilities_load_can_message(&can_msg, (0x1b000000 | (NODE_ALIAS_1 << 12) | SOURCE_ALIAS), 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    fail_buffer = false;

    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
    uint8_t bytes6[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x20};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, MTI_DATAGRAM_REJECTED_REPLY, 0x00, NODE_ALIAS_1, 0x00, SOURCE_ALIAS, 4, bytes6));
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************
}

TEST(CanRxMessageHandler, handle_middle_frame_frame)
{
    can_msg_t can_msg;
    openlcb_msg_t *outgoing_openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    openlcb_node_t *node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    node2->alias = NODE_ALIAS_2;

    // ************************************************************************
    // SNIP addressed to us but have not received the start frame yet
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x19a08000 | SOURCE_ALIAS), 8, 0x89, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);

    uint8_t bytes[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x41}; // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    EXPECT_TRUE(_compare_openlcb_msg(outgoing_openlcb_msg, MTI_OPTIONAL_INTERACTION_REJECTED, 0x00, NODE_ALIAS_2, 0x00, SOURCE_ALIAS, 4, bytes));
    OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    //  Datagram.......
    // ************************************************************************

    // ************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x1c666000 | SOURCE_ALIAS), 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_middle_frame(&can_msg, 0);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);

    // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes1[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x41}; // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    EXPECT_TRUE(_compare_openlcb_msg(outgoing_openlcb_msg, MTI_DATAGRAM_REJECTED_REPLY, 0x00, NODE_ALIAS_1, 0x00, SOURCE_ALIAS, 4, bytes1));
    OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************
}

TEST(CanRxMessageHandler, handle_last_frame)
{

    can_msg_t can_msg;
    openlcb_msg_t *outgoing_openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    node1->alias = NODE_ALIAS_1;
    openlcb_node_t *node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    node2->alias = NODE_ALIAS_2;

    // ************************************************************************
    // SNIP addressed to us but have not received the start frame yet
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x19a08000 | SOURCE_ALIAS), 8, 0x89, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_last_frame(&can_msg, 2);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);

    uint8_t bytes[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x41}; // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    EXPECT_TRUE(_compare_openlcb_msg(outgoing_openlcb_msg, MTI_OPTIONAL_INTERACTION_REJECTED, 0x00, NODE_ALIAS_2, 0x00, SOURCE_ALIAS, 4, bytes));
    OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    //  Datagram.......
    // ************************************************************************

    // ************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    CanUtilities_load_can_message(&can_msg, (0x1c666000 | SOURCE_ALIAS), 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_last_frame(&can_msg, 0);
    _test_for_only_openlcb_buffer_fifo_not_empty();
    outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(outgoing_openlcb_msg, nullptr);

    // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes1[4] = {SOURCE_ALIAS_HI, SOURCE_ALIAS_LO, 0x20, 0x41}; // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    EXPECT_TRUE(_compare_openlcb_msg(outgoing_openlcb_msg, MTI_DATAGRAM_REJECTED_REPLY, 0x00, NODE_ALIAS_1, 0x00, SOURCE_ALIAS, 4, bytes1));
    OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
    // ************************************************************************
}

TEST(CanRxMessageHandler, multi_frame_sequence_snip_frame)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0x666;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x999;

    // ************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_first_frame(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x65, 0x65, 0x72, 0x69, 0x6E, 0x67);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x30, 0x32, 0x35, 0x30, 0x38, 0x30);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x37, 0x2E, 0x30, 0x37, 0x35, 0x37);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x89, 0x99, 0x31, 0x38, 0x00, 0x02, 0x00, 0x00);
    CanRxMessageHandler_last_frame(&can_msg, 2);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[66] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61,
                         0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B,
                         0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E,
                         0x65, 0x65, 0x72, 0x69, 0x6E, 0x67,
                         0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F,
                         0x75, 0x74, 0x42, 0x6F, 0x73, 0x73,
                         0x00, 0x56, 0x65, 0x72, 0x73, 0x69,
                         0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32,
                         0x30, 0x32, 0x35, 0x30, 0x38, 0x30,
                         0x37, 0x2E, 0x30, 0x37, 0x35, 0x37,
                         0x31, 0x38, 0x00, 0x02, 0x00, 0x00};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 66, bytes));
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanRxMessageHandler, multi_frame_sequence_datagram_frame)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0x666;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x777;

    // ************************************************************************
    // Datagram addressed to us
    //[[1b7776be] 20 53 00 00 00 00 3C 3F] R : (Start of Datagram)
    //[[1c7776be] 78 6D 6C 20 76 65 72 73] R : (Middle of Datagram)
    //[[1c7776be] 69 6F 6E 3D 22 31 2E 30] R : (Middle of Datagram)
    //[[1d7776be] 22 20                  ]  R: 05.01.01.01.07.FF - 02.01.12.FE.83.2E Datagram: (26) 20.53.0.0.0.0.3C.3F.78.6D.6C.20.76.65.72.73.69.6F.6E.3D.22.31.2E.30.22.20
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x1b7776be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_first_frame(&can_msg, 0, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x1c7776be, 8, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73);
    CanRxMessageHandler_middle_frame(&can_msg, 0);
    CanUtilities_load_can_message(&can_msg, 0x1c7776be, 8, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30);
    CanRxMessageHandler_middle_frame(&can_msg, 0);
    CanUtilities_load_can_message(&can_msg, 0x1d7776be, 2, 0x22, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanRxMessageHandler_last_frame(&can_msg, 0);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x777, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[26] = {0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F,
                         0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73,
                         0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30,
                         0x22, 0x20};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, 0x1C48, 0x0000, 0x6be, 0x0000, 0x777, 26, bytes));
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanRxMessageHandler, multi_frame_sequence_legacy_snip_frame)
{
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0x666;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x999;

    // ************************************************************************
    // SNIP addressed to us
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x65, 0x65, 0x72, 0x69, 0x6E, 0x67);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x00, 0x56, 0x65, 0x72, 0x73, 0x69);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x30, 0x32, 0x35, 0x30, 0x38, 0x30);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x37, 0x2E, 0x30, 0x37, 0x35, 0x37);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x09, 0x99, 0x31, 0x38, 0x00, 0x02, 0x00, 0x00);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);

    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_FALSE(OpenLcbBufferFifo_is_empty());
    openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
    EXPECT_EQ(openlcb_msg, nullptr);
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);

    uint8_t bytes[66] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61,
                         0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B,
                         0x20, 0x45, 0x6E, 0x67, 0x69, 0x6E,
                         0x65, 0x65, 0x72, 0x69, 0x6E, 0x67,
                         0x00, 0x54, 0x75, 0x72, 0x6E, 0x6F,
                         0x75, 0x74, 0x42, 0x6F, 0x73, 0x73,
                         0x00, 0x56, 0x65, 0x72, 0x73, 0x69,
                         0x6F, 0x6E, 0x20, 0x32, 0x00, 0x32,
                         0x30, 0x32, 0x35, 0x30, 0x38, 0x30,
                         0x37, 0x2E, 0x30, 0x37, 0x35, 0x37,
                         0x31, 0x38, 0x00, 0x02, 0x00, 0x00};
    EXPECT_TRUE(_compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 66, bytes));
    OpenLcbBufferList_release(openlcb_msg);
    OpenLcbBufferStore_free_buffer(openlcb_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanRxMessageHandler, error_information_report_frame)
{

    can_msg_t can_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // ************************************************************************
    // No conflict
    // ************************************************************************
    can_msg.identifier = 0x10700000 | (NODE_ALIAS_1 + 1);
    can_msg.payload_count = 0;
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, but not permittted
    // ************************************************************************
    openlcb_node1->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;
    openlcb_node1->state.permitted = false;
    openlcb_node1->state.initialized = false;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);

    // ************************************************************************

    // ************************************************************************
    // Cause a conflict, permittted on node1
    // ************************************************************************
    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_1;
    can_msg.payload_count = 0;
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_1), 6, bytes));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_1), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node1->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node1->state.permitted);
    EXPECT_FALSE(openlcb_node1->state.initialized);
    EXPECT_FALSE(openlcb_node1->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node1->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node1->state.resend_datagram);
    EXPECT_FALSE(openlcb_node1->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node1->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node1->alias, 0x00);
    // ************************************************************************

    _global_reset_variables();
    _global_initialize();

    openlcb_node1 = OpenLcbNode_allocate(NODE_ID_1, &_node_parameters_main_node);
    openlcb_node1->alias = NODE_ALIAS_1;
    openlcb_node1->state.run_state = RUNSTATE_RUN;
    openlcb_node1->state.permitted = true;
    openlcb_node1->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_1)->is_permitted = true;

    openlcb_node2 = OpenLcbNode_allocate(NODE_ID_2, &_node_parameters_main_node);
    openlcb_node2->alias = NODE_ALIAS_2;
    openlcb_node2->state.run_state = RUNSTATE_RUN;
    openlcb_node2->state.permitted = true;
    openlcb_node2->state.initialized = true;
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    AliasMappings_find_mapping_by_alias(NODE_ALIAS_2)->is_permitted = true;

    can_msg.identifier = 0x10700000 | NODE_ALIAS_2;
    can_msg.payload_count = 0;
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    _test_for_only_can_buffer_fifo_not_empty();

    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);

    uint8_t bytes20[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, (0x10703000 | NODE_ALIAS_2), 6, bytes20));

    // Process the set flag for duplicate
    CanMainStateMachine_run();

    // Should have unregistered the mapping
    EXPECT_EQ(AliasMappings_find_mapping_by_alias(NODE_ALIAS_2), nullptr);

    // Test that the node is reset and ready to generate a new Alias
    EXPECT_EQ(openlcb_node2->state.run_state, RUNSTATE_GENERATE_SEED);
    EXPECT_FALSE(openlcb_node2->state.permitted);
    EXPECT_FALSE(openlcb_node2->state.initialized);
    EXPECT_FALSE(openlcb_node2->state.duplicate_id_detected);
    EXPECT_FALSE(openlcb_node2->state.firmware_upgrade_active);
    EXPECT_FALSE(openlcb_node2->state.resend_datagram);
    EXPECT_FALSE(openlcb_node2->state.openlcb_datagram_ack_sent);
    EXPECT_EQ(openlcb_node2->last_received_datagram, nullptr);
    EXPECT_EQ(openlcb_node2->alias, 0x00);
    // ************************************************************************
}

TEST(CanRxMessageHandler, CanRxMessageHandler_stream_frame)
{
    can_msg_t can_msg;

    CanRxMessageHandler_stream_frame(&can_msg, 0, STREAM);
}