#include "test/main_Test.hxx"

#include "can_login_message_handler.h"

// #include "can_types.h"
// #include "can_utilities.h"
// #include "can_rx_message_handler.h"
// #include "can_buffer_store.h"
// #include "can_buffer_fifo.h"
// #include "../../openlcb/openlcb_types.h"
// #include "../../openlcb/openlcb_buffer_store.h"
// #include "../../openlcb/openlcb_buffer_fifo.h"
// #include "../../openlcb/openlcb_buffer_list.h"
// #include "../../openlcb/openlcb_defines.h"
// #include "../../openlcb/openlcb_utilities.h"

// bool _alias_change_callback_called = false;
// uint16_t _alias_change_callback_alias = 0;
// uint64_t _alias_change_callback_node_id = 0;

// node_parameters_t _node_parameters_main_node = {

//     .consumer_count_autocreate = 0,
//     .producer_count_autocreate = 0,

//     .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
//     .snip.name = "Test",
//     .snip.model = "Test Model J",
//     .snip.hardware_version = "0.001",
//     .snip.software_version = "0.002",
//     .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

//     .protocol_support = (PSI_DATAGRAM |
//                          PSI_MEMORY_CONFIGURATION |
//                          PSI_EVENT_EXCHANGE |
//                          PSI_EVENT_EXCHANGE |
//                          PSI_ABBREVIATED_DEFAULT_CDI |
//                          PSI_SIMPLE_NODE_INFORMATION |
//                          PSI_CONFIGURATION_DESCRIPTION_INFO),

//     .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
//     .configuration_options.low_address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,

//     .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
//     .configuration_options.read_from_user_space_0xfb_supported = 1,
//     .configuration_options.stream_read_write_supported = 0,
//     .configuration_options.unaligned_reads_supported = 1,
//     .configuration_options.unaligned_writes_supported = 1,
//     .configuration_options.write_to_user_space_0xfb_supported = 1,
//     .configuration_options.write_under_mask_supported = 1,
//     .configuration_options.description = "These are options that defined the memory space capabilities",

//     // Space 0xFF
//     // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
//     //    make sure the CDI maps these 2 items to the first 128 bytes as well
//     .address_space_configuration_definition.read_only = 1,
//     .address_space_configuration_definition.present = 0,
//     .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
//     .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
//     .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
//     .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
//     .address_space_configuration_definition.description = "Configuration definition info",

//     // Space 0xFE
//     .address_space_all.read_only = 1,
//     .address_space_all.present = 0,
//     .address_space_all.low_address_valid = 0, // assume the low address starts at 0
//     .address_space_all.low_address = 0,       // ignored if low_address_valid is false
//     .address_space_all.highest_address = 0,
//     .address_space_all.address_space = ADDRESS_SPACE_ALL,
//     .address_space_all.description = "All memory Info",

//     // Space 0xFD
//     .address_space_config_memory.read_only = 0,
//     .address_space_config_memory.present = 0,
//     .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
//     .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
//     .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
//     .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
//     .address_space_config_memory.description = "Configuration memory storage",

//     .cdi =
//         {
//             // </cdi>
//         },

// };

// uint16_t extract_producer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
// {

//     return 0;
// }
// uint16_t extract_consumer_event_state_mti(openlcb_node_t *openlcb_node, uint16_t event_index)
// {

//     return 0;
// }

// void on_alias_change(uint16_t alias, uint64_t node_id)
// {
//     _alias_change_callback_called = true;
//     _alias_change_callback_alias = alias;
//     _alias_change_callback_node_id = node_id;
// }

// interface_can_login_message_handler_t interface_can_login_message_handler = {

//     interface_can_login_message_handler.extract_consumer_event_state_mti = &extract_consumer_event_state_mti,
//     interface_can_login_message_handler.extract_producer_event_state_mti = &extract_producer_event_state_mti,
//     interface_can_login_message_handler.on_alias_change = &on_alias_change

// };

// void _global_initialize(void)
// {

//     CanBufferStore_initialize();
//     CanBufferFifo_initialize();
//     OpenLcbBufferStore_initialize();
//     OpenLcbBufferFifo_initialize();
//     OpenLcbBufferList_initialize();

//     CanLoginMessageHandler_initialize(&interface_can_login_message_handler);
// }

// void _global_reset_variables(void)
// {

//     _alias_change_callback_called = false;
//     _alias_change_callback_alias = 0;
//     _alias_change_callback_node_id = 0;

// }

// uint16_t node_count = 0;

// openlcb_node_t *_create_openlcb_node(node_id_t node_id, uint16_t producer_count, uint16_t consumer_count)
// {
//     if (node_count >= USER_DEFINED_NODE_BUFFER_DEPTH)
//     {

//         return nullptr;
//     }

//     openlcb_node_t *result = (openlcb_node_t *)malloc(sizeof(openlcb_node_t));

//     result->state.run_state = RUNSTATE_INIT;
//     result->state.allocated = true;
//     result->state.permitted = false;
//     result->state.initalized = false;
//     result->state.duplicate_id_detected = false;
//     result->state.resend_datagram = false;
//     result->state.firmware_upgrade_active = false;
//     result->id = node_id;
//     result->alias = 0;
//     result->seed = 0;
//     result->index = node_count;
//     result->consumers.count = consumer_count;
//     result->parameters = &_node_parameters_main_node;

//     for (int i = 0; i < consumer_count; i++)
//     {

//         result->consumers.list[i].event = (node_id << 16) + i;
//         result->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
//     }
//     result->consumers.enumerator.running = false;
//     result->consumers.enumerator.enum_index = 0;

//     result->producers.count = producer_count;
//     for (int i = 0; i < producer_count; i++)
//     {

//         result->producers.list[i].event = (node_id << 16) + i;
//         result->consumers.list[i].status = EVENT_STATUS_UNKNOWN;
//     }
//     result->producers.enumerator.running = false;
//     result->producers.enumerator.enum_index = 0;

//     result->timerticks = 0;
//     result->lock_node = 0;
//     result->last_received_datagram = nullptr;

//     node_count++;
//     return result;
// }

// void _openlcb_node_free(openlcb_node_t *openlcb_node)
// {
//     if (!openlcb_node)
//     {

//         return;
//     }

//     free(openlcb_node);
//     node_count--;
// }

// TEST(CanLoginMessageHandler, init)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     CanLoginMessageHandler_init(openlcb_node);
//     EXPECT_EQ(openlcb_node->seed, NODE_ID);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, generate_seed)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->seed = NODE_ID;
//     openlcb_node->index = 2;
//     CanLoginMessageHandler_generate_seed(openlcb_node);
//     EXPECT_NE(openlcb_node->seed, NODE_ID);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_GENERATE_ALIAS);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, generate_alias)
// {
// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->seed = NODE_ID;
//     openlcb_node->alias = 0;

//     CanLoginMessageHandler_generate_alias(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
//     EXPECT_NE(openlcb_node->alias, 0);
//     EXPECT_TRUE(_alias_change_callback_called);

//     _global_reset_variables();
//     openlcb_node->seed = NODE_ID;
//     openlcb_node->alias = 0;

//     CanLoginMessageHandler_generate_alias(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_07);
//     EXPECT_NE(openlcb_node->alias, 0);
//     EXPECT_TRUE(_alias_change_callback_called);
//     EXPECT_EQ(_alias_change_callback_alias, openlcb_node->alias);
//     EXPECT_EQ(_alias_change_callback_node_id, openlcb_node->id);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_cid07)
// {

// #define NODE_ID 0x010203040506
// #define NODE_ALIAS 0xAAA

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_07;
//     openlcb_node->alias = NODE_ALIAS;

//     can_msg_t outgoing_can_msg;

//     CanLoginMessageHandler_load_cid07(openlcb_node, &outgoing_can_msg);
   
//     EXPECT_EQ(outgoing_can_msg.payload_count, 0);
//     EXPECT_EQ(outgoing_can_msg.identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID7 | (((openlcb_node->id >> 24) & 0xFFF000) | openlcb_node->alias));

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_06);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_cid06)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_06;

//     can_msg_t outgoing_can_msg;

//     CanLoginMessageHandler_load_cid06(openlcb_node, &outgoing_can_msg);
 
//     EXPECT_EQ(outgoing_can_msg.payload_count, 0);
//     EXPECT_EQ(outgoing_can_msg.identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID6 | (((openlcb_node->id >> 12) & 0xFFF000) | openlcb_node->alias));

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_05);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_cid05)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_05;

//     can_msg_t outgoing_can_msg;

//     CanLoginMessageHandler_load_cid05(openlcb_node, &outgoing_can_msg);
 
//     EXPECT_EQ(outgoing_can_msg.payload_count, 0);
//     EXPECT_EQ(outgoing_can_msg.identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID5 | ((openlcb_node->id & 0xFFF000) | openlcb_node->alias));

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CHECK_ID_04);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_cid04)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_CHECK_ID_04;

//     can_msg_t outgoing_can_msg;
    
//     CanLoginMessageHandler_load_cid04(openlcb_node, &outgoing_can_msg);

//     EXPECT_EQ(outgoing_can_msg.payload_count, 0);
//     EXPECT_EQ(outgoing_can_msg.identifier, RESERVED_TOP_BIT | CAN_CONTROL_FRAME_CID4 | (((openlcb_node->id << 12) & 0xFFF000) | openlcb_node->alias));

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, wait_200ms)
// {

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_WAIT_200ms;
//     openlcb_node->timerticks = 0;

//     CanLoginMessageHandler_wait_200ms(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

//     openlcb_node->timerticks++;
//     CanLoginMessageHandler_wait_200ms(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

//     openlcb_node->timerticks++;
//     CanLoginMessageHandler_wait_200ms(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

//     openlcb_node->timerticks++;
//     CanLoginMessageHandler_wait_200ms(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_WAIT_200ms);

//     openlcb_node->timerticks++;
//     CanLoginMessageHandler_wait_200ms(openlcb_node);

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_RESERVE_ID);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_rid)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_RESERVE_ID;

//     can_msg_t outgoing_can_msg;

//     CanLoginMessageHandler_load_rid(openlcb_node, &outgoing_can_msg);

//     EXPECT_EQ(outgoing_can_msg.payload_count, 0);
//     EXPECT_EQ(outgoing_can_msg.identifier, (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_RID | openlcb_node->alias));
//     EXPECT_FALSE(openlcb_node->state.initalized);
//     EXPECT_FALSE(openlcb_node->state.permitted);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_ALIAS_MAP_DEFINITION);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_amd)
// {

// #define NODE_ID 0x010203040506

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_ALIAS_MAP_DEFINITION;

//     can_msg_t outgoing_can_msg;

//     CanLoginMessageHandler_load_amd(openlcb_node, &outgoing_can_msg);

//     EXPECT_EQ(outgoing_can_msg.payload_count, 6);
//     EXPECT_EQ(CanUtilities_extract_can_payload_as_node_id(&outgoing_can_msg), NODE_ID);
//     EXPECT_EQ(outgoing_can_msg.identifier, (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AMD | openlcb_node->alias));
//     EXPECT_FALSE(openlcb_node->state.initalized);
//     EXPECT_TRUE(openlcb_node->state.permitted);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_INITIALIZATION_COMPLETE);

//     _openlcb_node_free(openlcb_node);
// }

// TEST(CanLoginMessageHandler, load_initialization_complete)
// {

// #define NODE_ID 0x010203040506
// #define ALIAS 0xAAA

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);
//     openlcb_node->alias = ALIAS;

//     openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

//     openlcb_msg_t *outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

//     openlcb_node->state.run_state = RUNSTATE_LOAD_INITIALIZATION_COMPLETE;

//     CanLoginMessageHandler_load_initialization_complete(openlcb_node, outgoing_openlcb_msg);

//     EXPECT_EQ(outgoing_openlcb_msg->mti, MTI_INITIALIZATION_COMPLETE);
//     EXPECT_EQ(outgoing_openlcb_msg->payload_count, 6);
//     EXPECT_EQ(outgoing_openlcb_msg->source_alias, ALIAS);
//     EXPECT_EQ(outgoing_openlcb_msg->source_id, NODE_ID);
//     EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_openlcb_msg, 0), NODE_ID);
//     EXPECT_TRUE(openlcb_node->state.initalized);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);

//     _global_reset_variables();

//     _node_parameters_main_node.protocol_support = _node_parameters_main_node.protocol_support | PSI_SIMPLE;
//     CanLoginMessageHandler_load_initialization_complete(openlcb_node, outgoing_openlcb_msg);

//     EXPECT_EQ(outgoing_openlcb_msg->mti, MTI_INITIALIZATION_COMPLETE_SIMPLE);
//     EXPECT_EQ(outgoing_openlcb_msg->payload_count, 6);
//     EXPECT_EQ(outgoing_openlcb_msg->source_alias, ALIAS);
//     EXPECT_EQ(outgoing_openlcb_msg->source_id, NODE_ID);
//     EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_openlcb_payload(outgoing_openlcb_msg, 0), NODE_ID);
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_PRODUCER_EVENTS);
//     _node_parameters_main_node.protocol_support = _node_parameters_main_node.protocol_support & ~PSI_SIMPLE;

//     _openlcb_node_free(openlcb_node);
//     OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
// }

// TEST(CanLoginMessageHandler, load_producer_events)
// {

// #define NODE_ID 0x010203040506
// #define ALIAS 0xAAA

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);
//     openlcb_node->alias = ALIAS;
//     openlcb_node->producers.enumerator.running = true;
//     openlcb_node->producers.enumerator.enum_index = 0;
//     openlcb_node->state.initalized = true;
//     openlcb_node->state.run_state = RUNSTATE_LOAD_PRODUCER_EVENTS;

//     openlcb_msg_t *outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

//     // *******************************************************************************************
//     // Test with the send buffer free and get all events
//     // *******************************************************************************************

//     _global_reset_variables();

//     uint16_t offset = 0;
//     while (openlcb_node->state.run_state == RUNSTATE_LOAD_PRODUCER_EVENTS)
//     {

//         CanLoginMessageHandler_load_producer_events(openlcb_node, outgoing_openlcb_msg);
//         EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_openlcb_msg), (uint64_t)(NODE_ID << 16) + offset);
//         offset++;
//     }

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

//     _openlcb_node_free(openlcb_node);

//     openlcb_node = _create_openlcb_node(NODE_ID, 0, 0);
//     openlcb_node->alias = ALIAS;
//     openlcb_node->consumers.enumerator.running = true;
//     openlcb_node->consumers.enumerator.enum_index = 0;
//     openlcb_node->state.initalized = true;
//     openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

//     _global_reset_variables();
//     // *******************************************************************************************
//     // Test with no events defined for the node
//     // *******************************************************************************************
//     CanLoginMessageHandler_load_producer_events(openlcb_node, outgoing_openlcb_msg);
  
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_LOAD_CONSUMER_EVENTS);

//     // *******************************************************************************************

//     _openlcb_node_free(openlcb_node);
//     OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
// }

// TEST(CanLoginMessageHandler, load_consumer_events)
// {

// #define NODE_ID 0x010203040506
// #define ALIAS 0xAAA

//     _global_initialize();
//     _global_reset_variables();

//     openlcb_node_t *openlcb_node = _create_openlcb_node(NODE_ID, 3, 4);
//     openlcb_node->alias = ALIAS;
//     openlcb_node->consumers.enumerator.running = true;
//     openlcb_node->consumers.enumerator.enum_index = 0;
//     openlcb_node->state.initalized = true;
//     openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

//      openlcb_msg_t *outgoing_openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

//     // *******************************************************************************************
//     // Test with the send buffer free and get all events
//     // *******************************************************************************************
//     _global_reset_variables();

//     uint16_t offset = 0;
//     while (openlcb_node->state.run_state == RUNSTATE_LOAD_CONSUMER_EVENTS)
//     {

//         CanLoginMessageHandler_load_consumer_events(openlcb_node, outgoing_openlcb_msg);
//         EXPECT_EQ(OpenLcbUtilities_extract_event_id_from_openlcb_payload(outgoing_openlcb_msg), (uint64_t)(NODE_ID << 16) + offset);
//         offset++;
//     }

//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_RUN);
//     // *******************************************************************************************

//     _openlcb_node_free(openlcb_node);

//     openlcb_node = _create_openlcb_node(NODE_ID, 0, 0);
//     openlcb_node->alias = ALIAS;
//     openlcb_node->consumers.enumerator.running = true;
//     openlcb_node->consumers.enumerator.enum_index = 0;
//     openlcb_node->state.initalized = true;
//     openlcb_node->state.run_state = RUNSTATE_LOAD_CONSUMER_EVENTS;

//     _global_reset_variables();
//     // *******************************************************************************************
//     // Test with no events defined for the node
//     // *******************************************************************************************
//     CanLoginMessageHandler_load_consumer_events(openlcb_node, outgoing_openlcb_msg);
    
//     EXPECT_EQ(openlcb_node->state.run_state, RUNSTATE_RUN);

//     // *******************************************************************************************

//     _openlcb_node_free(openlcb_node);
//     OpenLcbBufferStore_free_buffer(outgoing_openlcb_msg);
// }
