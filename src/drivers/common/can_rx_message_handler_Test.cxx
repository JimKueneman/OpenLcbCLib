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

bool fail_buffer = false;

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

    .cdi =
        {
            // </cdi>
        },

};

openlcb_msg_t *openlcb_buffer_store_allocate_buffer(payload_type_enum payload_type)
{

    if (!fail_buffer)
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

// void pause_can_rx(void)
// {
// }

// void resume_can_rx(void)
// {
// }

const interface_can_frame_message_handler_t can_frame_message_handler_interface = {

    .openlcb_buffer_store_allocate_buffer = &openlcb_buffer_store_allocate_buffer,
    .can_buffer_store_allocate_buffer = &can_buffer_store_allocate_buffer

};

const interface_openlcb_node_t interface_openlcb_node = {

};

void _global_initialize(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    OpenLcbNode_initialize(&interface_openlcb_node);

    CanRxMessageHandler_initialize(&can_frame_message_handler_interface);
}

void _global_reset_variables(void)
{

    fail_buffer = false;
}

void _initalize_can_statemachine_info(node_id_t node_id, node_parameters_t *node_parameters, can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node = OpenLcbNode_allocate(node_id, node_parameters);
    can_statemachine_info->incoming_msg = NULL;
    can_statemachine_info->outgoing_can_msg = CanBufferStore_allocate_buffer();
    can_statemachine_info->outgoing_can_msg_valid = false;
    can_statemachine_info->outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    can_statemachine_info->outgoing_openlcb_msg_valid = false;
}

void _set_can_statemachine_info_node_active(can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node->state.allocated = true;
    can_statemachine_info->openlcb_node->state.initalized = true;
    can_statemachine_info->openlcb_node->state.permitted = true;
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_RUN;
}

void _set_can_statemachine_info_node_inactive(can_statemachine_info_t *can_statemachine_info)
{

    can_statemachine_info->openlcb_node->state.allocated = true;
    can_statemachine_info->openlcb_node->state.initalized = false;
    can_statemachine_info->openlcb_node->state.permitted = false;
    can_statemachine_info->openlcb_node->state.run_state = RUNSTATE_INIT;
}

bool _compare_can_msg(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, uint8_t bytes[])
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

        return false;
    }

    for (int i = 0; i < payload_size; i++)
    {

        if (*openlcb_msg->payload[i] != bytes[i])
        {

            return false;
        }
    }

    return true;
}

TEST(CanRxMessageHandler, initialize)
{

    _global_reset_variables();
    _global_initialize();
}

TEST(CanRxMessageHandler, cid)
{

#define NODE_ID_1 0x010203040506
#define NODE_ID_2 0x010203040507

    can_msg_t incoming_can_msg;

    _global_reset_variables();
    _global_initialize();

    can_statemachine_info_t can_statemachine_info_1;
    _initalize_can_statemachine_info(NODE_ID_1, &_node_parameters_main_node, &can_statemachine_info_1);
    can_statemachine_info_1.openlcb_node->alias = 0xAAA;
    can_statemachine_info_1.incoming_msg = &incoming_can_msg;

    can_statemachine_info_t can_statemachine_info_2;
    _initalize_can_statemachine_info(NODE_ID_2, &_node_parameters_main_node, &can_statemachine_info_2);
    can_statemachine_info_2.openlcb_node->alias = 0x777;
    can_statemachine_info_2.incoming_msg = &incoming_can_msg;

    // // ************************************************************************
    // // No conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x17050AAB;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);

    // // ************************************************************************

    // // ************************************************************************
    // // No conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x16050AAB;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);
    // // ************************************************************************

    // // ************************************************************************
    // // No conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x15050AAB;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);

    // // ************************************************************************

    // // ************************************************************************
    // // No conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x17040AAB;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);

    // // ************************************************************************

    // // ************************************************************************
    // // Cause a conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x17050AAA;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_NE(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_TRUE(_compare_can_msg(can_statemachine_info_1.outgoing_can_msg, 0x10700AAA, 0, nullptr));
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);

    // // ************************************************************************

    // // ************************************************************************
    // // Cause a conflict
    // // ************************************************************************
    // _set_can_statemachine_info_node_active(&can_statemachine_info_1);
    // _set_can_statemachine_info_node_active(&can_statemachine_info_2);
    // incoming_can_msg.identifier = 0x17040777;
    // incoming_can_msg.payload_count = 0;
    // CanRxMessageHandler_cid_frame(&incoming_can_msg);
    // can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
    // EXPECT_NE(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_TRUE(_compare_can_msg(can_statemachine_info_1.outgoing_can_msg, 0x10700777, 0, nullptr));
    // EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);
    // EXPECT_FALSE(can_statemachine_info_1.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_1.openlcb_node->state.initalized);
    // EXPECT_FALSE(can_statemachine_info_2.openlcb_node->state.run_state == RUNSTATE_GENERATE_SEED);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.permitted);
    // EXPECT_TRUE(can_statemachine_info_2.openlcb_node->state.initalized);

    // // ************************************************************************
}

// TEST(CanRxMessageHandler, rid)
// {

// #define NODE_ID_1 0x010203040506
// #define NODE_ID_2 0x010203040507

//     can_msg_t incoming_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     can_statemachine_info_t can_statemachine_info_1;
//     _initalize_can_statemachine_info(NODE_ID_1, &_node_parameters_main_node, &can_statemachine_info_1);
//     can_statemachine_info_1.openlcb_node->alias = 0xAAA;
//     can_statemachine_info_1.incoming_msg = &incoming_can_msg;

//     can_statemachine_info_t can_statemachine_info_2;
//     _initalize_can_statemachine_info(NODE_ID_2, &_node_parameters_main_node, &can_statemachine_info_2);
//     can_statemachine_info_2.openlcb_node->alias = 0x777;
//     can_statemachine_info_2.incoming_msg = &incoming_can_msg;

//     // ************************************************************************
//     // No conflict
//     // ************************************************************************
//     _set_can_statemachine_info_node_active(&can_statemachine_info_1);
//     _set_can_statemachine_info_node_active(&can_statemachine_info_2);
//     incoming_can_msg.identifier = 0x10700AAB;
//     incoming_can_msg.payload_count = 0;
//     CanRxMessageHandler_rid_frame(&incoming_can_msg);
//     can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);

//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict, but not permittted
//     // ************************************************************************
//     _set_can_statemachine_info_node_inactive(&can_statemachine_info_1);
//     _set_can_statemachine_info_node_inactive(&can_statemachine_info_2);
//     incoming_can_msg.identifier = 0x10700AAA;
//     incoming_can_msg.payload_count = 0;
//     CanRxMessageHandler_rid_frame(&incoming_can_msg);
//     can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(can_statemachine_info_1.outgoing_can_msg, nullptr);

//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict, permittted
//     // ************************************************************************
//     _set_can_statemachine_info_node_active(&can_statemachine_info_1);
//     _set_can_statemachine_info_node_active(&can_statemachine_info_2);
//     incoming_can_msg.identifier = 0x10700AAA;
//     incoming_can_msg.payload_count = 0;
//     CanRxMessageHandler_rid_frame(&incoming_can_msg);
//     can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(can_statemachine_info_1.outgoing_can_msg, nullptr);

//     uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(_compare_can_msg(can_statemachine_info_1.outgoing_can_msg, 0x10703AAA, 6, bytes));

//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict permitted
//     // ************************************************************************
//     _set_can_statemachine_info_node_active(&can_statemachine_info_1);
//     _set_can_statemachine_info_node_active(&can_statemachine_info_2);
//     incoming_can_msg.identifier = 0x10700777;
//     incoming_can_msg.payload_count = 0;
//     CanRxMessageHandler_rid_frame(&incoming_can_msg);
//     can_statemachine_info_1.outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(can_statemachine_info_1.outgoing_can_msg, nullptr);
//     uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
//     EXPECT_TRUE(_compare_can_msg(can_statemachine_info_1.outgoing_can_msg, 0x10703777, 6, bytes1));

//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, amd)
// {

//     can_msg_t can_msg;
//     can_msg_t *outgoing_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0xAAA;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x777;

//     // ************************************************************************
//     // No conflict
//     // ************************************************************************
//     can_msg.identifier = 0x10701AAB;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_amd(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);
//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict not permitted
//     // ************************************************************************
//     can_msg.identifier = 0x10701AAA;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_amd(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);

//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict permitted
//     // ************************************************************************
//     node1->state.permitted = true;
//     node2->state.permitted = true;

//     can_msg.identifier = 0x10701AAA;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_amd(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));
//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict
//     // ************************************************************************
//     can_msg.identifier = 0x10701777;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_rid(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703777, 6, bytes1));
//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_TRUE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, ame)
// {

//     can_msg_t can_msg;
//     can_msg_t *outgoing_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0xAAA;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x777;

//     // ************************************************************************
//     // Not a conflict nor for us
//     // ************************************************************************

//     node1->state.permitted = true;
//     node2->state.permitted = true;

//     CanUtilities_load_can_message(&can_msg, 0x10702AAB, 6, 0x99, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x00);
//     CanRxMessageHandler_ame(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);

//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict permitted
//     // ************************************************************************

//     node1->state.permitted = true;
//     node2->state.permitted = true;

//     can_msg.identifier = 0x10702AAA;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_ame(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));

//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************

//     // ************************************************************************
//     // No conflict request one of our nodes
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x10702AAB, 6, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x00, 0x00);
//     CanRxMessageHandler_ame(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes10[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701777, 6, bytes10));
//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     //************************************************************************

//     // ************************************************************************
//     // No conflict
//     // Request all of our nodes
//     // ************************************************************************
//     can_msg.identifier = 0x10702AAB;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_ame(&can_msg);

//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701AAA, 6, bytes1));

//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes2[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10701777, 6, bytes2));

//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, amr)
// {

//     can_msg_t can_msg;
//     can_msg_t *outgoing_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0xAAA;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x777;

//     // ************************************************************************
//     // Cause a conflict
//     // ************************************************************************
//     node1->state.permitted = true;
//     node2->state.permitted = true;
//     CanUtilities_load_can_message(&can_msg, 0x10703AAA, 6, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0x00, 0x00);
//     CanRxMessageHandler_amr(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes6[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes6));
//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************

//     // ************************************************************************
//     // Non  conflict
//     // ************************************************************************
//     can_msg.identifier = 0x10703AAB;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_amr(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);

//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, handle_single_frame)
// {

//     can_msg_t can_msg;
//     openlcb_msg_t *outgoing_openlcb_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0xAAA;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x777;

//     // ************************************************************************
//     // Addressed to us but can't allocate a buffer
//     // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAA, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
//     fail_buffer = true;
//     CanRxMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
//     fail_buffer = false;
//     EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
//     EXPECT_TRUE(CanBufferFifo_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // Addressed message to us
//     // [[196686be] 0A AA 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAA, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
//     CanRxMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
//     outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
//     EXPECT_NE(outgoing_openlcb_msg, nullptr);
//     uint8_t bytes[6] = {0x54, 0x58, 0x20, 0x00, 0x00, 0x00};
//     EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x668, 0x0000, 0x06BE, 0x0000, 0x0AAA, 6, bytes));
//     //************************************************************************

//     // ************************************************************************
//     // Addressed message to someone else
//     // [[196686be] 0A AB 54 58 20 00 00 00]  R:  ProtocolSupportReply with payload 54 58 20 00 00 00
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x196686BE, 8, 0x0A, 0xAB, 0x54, 0x58, 0x20, 0x00, 0x00, 0x00);
//     CanRxMessageHandler_handle_single_frame(&can_msg, 2, BASIC);
//     outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
//     EXPECT_EQ(outgoing_openlcb_msg, nullptr);
//     // ************************************************************************

//     // ************************************************************************
//     // Unaddressed message to everyone
//     // [[195476be]  Producer Identified Unknown for EventID:05.01.01.01.07.FF.00.25
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x195476BE, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25);
//     CanRxMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
//     outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
//     EXPECT_NE(outgoing_openlcb_msg, nullptr);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     uint8_t bytes1[8] = {0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x25};
//     EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x547, 0x0000, 0x06BE, 0x0000, 0x0000, 8, bytes1));
//     // ************************************************************************

//     // ************************************************************************
//     // Addressed Datagram message to us
//     // [1a5556eb]f 20 61 00 00 00 00 08   ]  Datagram: (7) 20.61.0.0.0.0.8
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1A7776BE, 7, 0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00);
//     CanRxMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
//     outgoing_openlcb_msg = OpenLcbBufferFifo_pop();
//     EXPECT_NE(outgoing_openlcb_msg, nullptr);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     uint8_t bytes2[7] = {0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08};
//     EXPECT_TRUE(compare_openlcb_msg(outgoing_openlcb_msg, 0x1C48, 0x0000, 0x06BE, 0x0000, 0x0777, 7, bytes2));
//     // ************************************************************************

//     // ************************************************************************
//     // Addressed Datagram message not to us
//     // Addressed Datagram message to us
//     // [1a5546eb]f 20 61 00 00 00 00 08   ]  Datagram: (7) 20.61.0.0.0.0.8
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1a5546BE, 7, 0x20, 0x61, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00);
//     CanRxMessageHandler_handle_single_frame(&can_msg, 0, BASIC);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferFifo_is_empty());
//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, handle_first_frame)
// {
//     can_msg_t can_msg;
//     openlcb_msg_t *openlcb_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0x666;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x999;

//     // ************************************************************************
//     // SNIP addressed to somone else
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x4F, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     // ************************************************************************

//     // ************************************************************************
//     //  SNIP.......
//     // ************************************************************************

//     // ************************************************************************
//     // SNIP addressed to us
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     openlcb_msg = OpenLcbBufferList_find(0x6be, 0x0999, 0xa08);
//     EXPECT_NE(openlcb_msg, nullptr);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     uint8_t bytes[6] = {0x04, 0x4D, 0x75, 0x73, 0x74, 0x61};
//     EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x0A08, 0x0000, 0x6be, 0x0000, 0x999, 6, bytes));
//     OpenLcbBufferList_free(openlcb_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // SNIP addressed to us but there is one already sent
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     EXPECT_FALSE(CanBufferFifo_is_empty());
//     can_msg_t *outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
//     uint8_t bytes9[4] = {0x06, 0xbe, 0x20, 0x42};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes9));
//     OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // SNIP addressed to us but we can't allocate a buffer
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x49, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     fail_buffer = true;
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, SNIP);
//     fail_buffer = false;
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
//     uint8_t bytes10[4] = {0x06, 0xbe, 0x20, 0x20};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes10));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     //  Datagram.......
//     // ************************************************************************

//     // ************************************************************************
//     // datagram addressed to somone else
//     // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1bf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // Datagram addressed to us
//     // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     openlcb_msg = OpenLcbBufferList_find(0x06be, 0x0666, MTI_DATAGRAM);
//     EXPECT_NE(openlcb_msg, nullptr);
//     uint8_t bytes1[8] = {0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F};
//     EXPECT_TRUE(compare_openlcb_msg(openlcb_msg, 0x1C48, 0x0000, 0x6be, 0x0000, 0x666, 8, bytes1));
//     OpenLcbBufferList_free(openlcb_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // Datagram addressed to us but there is one already sent
//     // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     CanRxMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     CanRxMessageHandler_handle_first_frame(&can_msg, 0, DATAGRAM);
//     EXPECT_FALSE(OpenLcbBufferList_is_empty());
//     EXPECT_FALSE(CanBufferFifo_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042
//     uint8_t bytes2[4] = {0x06, 0xbe, 0x20, 0x42};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes2));
//     OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0666, MTI_DATAGRAM));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     // DATAGRAM addressed to us but we can't allocate a buffer
//     // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1b6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     fail_buffer = true;
//     CanRxMessageHandler_handle_first_frame(&can_msg, 2, DATAGRAM);
//     fail_buffer = false;
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020
//     uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x20};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     OpenLcbBufferList_free(OpenLcbBufferList_find(0x6be, 0x0999, 0xa08));
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     // ************************************************************************
// }

// TEST(CanRxMessageHandler, handle_middle_frame)
// {
//     can_msg_t can_msg;
//     can_msg_t *outgoing_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0x666;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x999;

//     // ************************************************************************
//     // SNIP addressed to somone else
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xCF, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_middle_frame(&can_msg, 2);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     // ************************************************************************

//     // ************************************************************************
//     // SNIP addressed to us but have not received the start frame yet
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_middle_frame(&can_msg, 2);
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     EXPECT_FALSE(CanBufferFifo_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
//     uint8_t bytes[4] = {0x06, 0xbe, 0x20, 0x41};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     // ************************************************************************

//     // ************************************************************************
//     // SNIP addressed to us but there is one already sent
//     // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0xC9, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
//     CanRxMessageHandler_handle_middle_frame(&can_msg, 2);
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     EXPECT_FALSE(CanBufferFifo_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
//     uint8_t bytes9[4] = {0x06, 0xbe, 0x20, 0x41};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes9));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************

//     // ************************************************************************
//     //  Datagram.......
//     // ************************************************************************

//     // ************************************************************************
//     // datagram addressed to somone else
//     // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1cf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     CanRxMessageHandler_handle_middle_frame(&can_msg, 0);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     // ************************************************************************

//     // ************************************************************************
//     // Datagram addressed to us but there is one already sent
//     // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
//     // ************************************************************************
//     CanUtilities_load_can_message(&can_msg, 0x1c6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
//     CanRxMessageHandler_handle_middle_frame(&can_msg, 0);
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());
//     EXPECT_FALSE(CanBufferFifo_is_empty());
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
//     uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x41};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));
//     CanBufferStore_free_buffer(outgoing_can_msg);
//     EXPECT_TRUE(CanBufferFifo_is_empty());
//     EXPECT_TRUE(OpenLcbBufferList_is_empty());

//     // ************************************************************************
// }

TEST(CanRxMessageHandler, handle_last_frame)
{
    can_msg_t can_msg;
    //  openlcb_msg_t *openlcb_msg;
    can_msg_t *outgoing_can_msg;

    _global_reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
    node1->alias = 0x666;
    openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
    node2->alias = 0x999;

    // ************************************************************************
    // SNIP addressed to somone else
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x8F, 0x37, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_last_frame(&can_msg, 2);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // ************************************************************************

    // ************************************************************************
    // SNIP addressed to us but have not received the start frame yet
    // [19a086be] 4F 37 04 4D 75 73 74 61]  Simple Node Ident Info with content '4,Musta'
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x19a086be, 8, 0x89, 0x99, 0x04, 0x4D, 0x75, 0x73, 0x74, 0x61);
    CanRxMessageHandler_last_frame(&can_msg, 2);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    // define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, 0x19068999, 4, bytes));
    CanBufferStore_free_buffer(outgoing_can_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // ************************************************************************

    // ************************************************************************
    //  Datagram.......
    // ************************************************************************

    // ************************************************************************
    // datagram addressed to somone else
    // [[1bf376be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x1cf376be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_last_frame(&can_msg, 0);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // ************************************************************************

    // ************************************************************************
    // Datagram addressed to us but there is one already sent
    // // [[1b6666be] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    // ************************************************************************
    CanUtilities_load_can_message(&can_msg, 0x1c6666be, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    CanRxMessageHandler_last_frame(&can_msg, 0);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_FALSE(CanBufferFifo_is_empty());
    outgoing_can_msg = CanBufferFifo_pop();
    EXPECT_NE(outgoing_can_msg, nullptr);
    // ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
    uint8_t bytes3[4] = {0x06, 0xbe, 0x20, 0x41};
    EXPECT_TRUE(_compare_can_msg(outgoing_can_msg, 0x19A48666, 4, bytes3));
    CanBufferStore_free_buffer(outgoing_can_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    // ************************************************************************
}

TEST(CanRxMessageHandler, multi_frame_sequence_snip)
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
    OpenLcbBufferList_free(openlcb_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanRxMessageHandler, multi_frame_sequence_datagram)
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
    OpenLcbBufferList_free(openlcb_msg);

    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

TEST(CanRxMessageHandler, multi_frame_sequence_legacy_snip)
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
    OpenLcbBufferList_free(openlcb_msg);
    EXPECT_TRUE(CanBufferFifo_is_empty());
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

// TEST(CanRxMessageHandler, error_information_report)
// {

//     can_msg_t can_msg;
//     can_msg_t *outgoing_can_msg;

//     _global_reset_variables();
//     _global_initialize();

//     openlcb_node_t *node1 = OpenLcbNode_allocate(0x010203040506, &_node_parameters_main_node);
//     node1->alias = 0xAAA;
//     openlcb_node_t *node2 = OpenLcbNode_allocate(0x010203040507, &_node_parameters_main_node);
//     node2->alias = 0x777;

//     // ************************************************************************
//     // No conflict
//     // ************************************************************************
//     can_msg.identifier = 0x10710AAB;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_error_information_report(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);
//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict, but not permittted
//     // ************************************************************************
//     can_msg.identifier = 0x10710AAA;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_error_information_report(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_EQ(outgoing_can_msg, nullptr);
//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict, permittted
//     // ************************************************************************
//     node1->state.permitted = true;
//     node2->state.permitted = true;

//     can_msg.identifier = 0x10710AAA;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_error_information_report(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);

//     uint8_t bytes[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703AAA, 6, bytes));
//     EXPECT_TRUE(node1->state.duplicate_alias_detected);
//     EXPECT_FALSE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;
//     // ************************************************************************

//     // ************************************************************************
//     // Cause a conflict permitted
//     // ************************************************************************
//     can_msg.identifier = 0x10714777;
//     can_msg.payload_count = 0;
//     CanRxMessageHandler_error_information_report(&can_msg);
//     outgoing_can_msg = CanBufferFifo_pop();
//     EXPECT_NE(outgoing_can_msg, nullptr);
//     uint8_t bytes1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x07};
//     EXPECT_TRUE(compare_can_msg(outgoing_can_msg, 0x10703777, 6, bytes1));
//     EXPECT_FALSE(node1->state.duplicate_alias_detected);
//     EXPECT_TRUE(node2->state.duplicate_alias_detected);

//     node1->state.duplicate_alias_detected = false;
//     node2->state.duplicate_alias_detected = false;

//     // ************************************************************************
// }

TEST(CanRxMessageHandler, handle_stream)
{
    can_msg_t can_msg;

    CanRxMessageHandler_stream(&can_msg, 0, STREAM);
}