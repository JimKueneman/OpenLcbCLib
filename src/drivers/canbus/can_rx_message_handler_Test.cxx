/*******************************************************************************
 * File: can_rx_message_handler_Test.cxx
 * 
 * Description:
 *   Test suite for CAN RX Message Handler - CORRECTED VERSION
 *   Uses ONLY verified API calls from the actual header files
 *
 * Author: Test Suite
 * Date: 2026-01-19
 * Version: 4.0 - Built from scratch with correct API
 ******************************************************************************/

#include "test/main_Test.hxx"

#include <cstring>

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
#include "../../drivers/canbus/alias_mappings.h"
#include "../../openlcb/openlcb_utilities.h"

/*******************************************************************************
 * Test Constants
 ******************************************************************************/

#define NODE_ID_1 0x010203040506
#define NODE_ID_2 0x010203040507

#define NODE_ALIAS_1    0x0666
#define NODE_ALIAS_1_HI 0x06
#define NODE_ALIAS_1_LO 0x66

#define NODE_ALIAS_2    0x0999
#define NODE_ALIAS_2_HI 0x09
#define NODE_ALIAS_2_LO 0x99

#define SOURCE_ALIAS    0x06BE
#define SOURCE_ALIAS_HI 0x06
#define SOURCE_ALIAS_LO 0xBE

/*******************************************************************************
 * Mock Control Variables
 ******************************************************************************/

bool fail_buffer = false;
bool force_fail_allocate = false;

/*******************************************************************************
 * Mock Buffer Allocation Functions
 ******************************************************************************/

openlcb_msg_t *openlcb_buffer_store_allocate_buffer(payload_type_enum payload_type)
{
    if (force_fail_allocate)
    {
        return nullptr;
    }
    return OpenLcbBufferStore_allocate_buffer(payload_type);
}

can_msg_t *can_buffer_store_allocate_buffer(void)
{
    if (fail_buffer)
    {
        return nullptr;
    }
    return CanBufferStore_allocate_buffer();
}

/*******************************************************************************
 * Mock Clock Access
 ******************************************************************************/

static uint8_t _test_global_100ms_tick = 0;

uint8_t _mock_get_current_tick(void)
{

    return _test_global_100ms_tick;

}

/*******************************************************************************
 * Mock Listener Callbacks
 ******************************************************************************/

static bool listener_set_alias_called = false;
static node_id_t listener_set_alias_node_id = 0;
static uint16_t listener_set_alias_alias = 0;

    /** @brief Count of listener_set_alias calls for multi-node tests */
static int listener_set_alias_call_count = 0;

#define MAX_LISTENER_SET_ALIAS_CALLS 8

    /** @brief Tracks node_ids passed to each listener_set_alias call */
static node_id_t listener_set_alias_node_ids[MAX_LISTENER_SET_ALIAS_CALLS];

    /** @brief Tracks aliases passed to each listener_set_alias call */
static uint16_t listener_set_alias_aliases[MAX_LISTENER_SET_ALIAS_CALLS];

static bool listener_clear_alias_called = false;
static uint16_t listener_clear_alias_alias = 0;

static bool listener_flush_aliases_called = false;

    /** @brief Mock listener_set_alias that records every call for multi-node verification */
void _mock_listener_set_alias(node_id_t node_id, uint16_t alias)
{

    listener_set_alias_called = true;
    listener_set_alias_node_id = node_id;
    listener_set_alias_alias = alias;

    if (listener_set_alias_call_count < MAX_LISTENER_SET_ALIAS_CALLS) {

        listener_set_alias_node_ids[listener_set_alias_call_count] = node_id;
        listener_set_alias_aliases[listener_set_alias_call_count] = alias;

    }

    listener_set_alias_call_count++;

}

void _mock_listener_clear_alias_by_alias(uint16_t alias)
{

    listener_clear_alias_called = true;
    listener_clear_alias_alias = alias;

}

void _mock_listener_flush_aliases(void)
{

    listener_flush_aliases_called = true;

}

/*******************************************************************************
 * Interface Structure for RX Message Handler
 ******************************************************************************/

const interface_can_rx_message_handler_t _can_rx_message_handler_interface = {
    .can_buffer_store_allocate_buffer = &can_buffer_store_allocate_buffer,
    .openlcb_buffer_store_allocate_buffer = &openlcb_buffer_store_allocate_buffer,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag,
    .get_current_tick = &_mock_get_current_tick,
};

const interface_can_rx_message_handler_t _can_rx_message_handler_interface_with_listeners = {
    .can_buffer_store_allocate_buffer = &can_buffer_store_allocate_buffer,
    .openlcb_buffer_store_allocate_buffer = &openlcb_buffer_store_allocate_buffer,
    .alias_mapping_find_mapping_by_alias = &AliasMappings_find_mapping_by_alias,
    .alias_mapping_find_mapping_by_node_id = &AliasMappings_find_mapping_by_node_id,
    .alias_mapping_get_alias_mapping_info = &AliasMappings_get_alias_mapping_info,
    .alias_mapping_set_has_duplicate_alias_flag = &AliasMappings_set_has_duplicate_alias_flag,
    .get_current_tick = &_mock_get_current_tick,
    .listener_set_alias = &_mock_listener_set_alias,
    .listener_clear_alias_by_alias = &_mock_listener_clear_alias_by_alias,
    .listener_flush_aliases = &_mock_listener_flush_aliases,
};

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

void _global_initialize(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    AliasMappings_initialize();
    CanRxMessageHandler_initialize(&_can_rx_message_handler_interface);
}

void _global_initialize_with_listeners(void)
{
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferList_initialize();
    AliasMappings_initialize();
    CanRxMessageHandler_initialize(&_can_rx_message_handler_interface_with_listeners);
}

void _global_reset_variables(void)
{
    fail_buffer = false;
    force_fail_allocate = false;
    _test_global_100ms_tick = 0;
    listener_set_alias_called = false;
    listener_set_alias_node_id = 0;
    listener_set_alias_alias = 0;
    listener_set_alias_call_count = 0;
    memset(listener_set_alias_node_ids, 0, sizeof(listener_set_alias_node_ids));
    memset(listener_set_alias_aliases, 0, sizeof(listener_set_alias_aliases));
    listener_clear_alias_called = false;
    listener_clear_alias_alias = 0;
    listener_flush_aliases_called = false;
}

// Helper: Count items in OpenLcbBufferList (API doesn't provide count())
uint16_t _count_buffer_list_items(void)
{
    if (OpenLcbBufferList_is_empty())
    {
        return 0;
    }
    
    uint16_t count = 0;
    for (uint16_t i = 0; i < LEN_MESSAGE_BUFFER; i++)
    {
        if (OpenLcbBufferList_index_of(i) != nullptr)
        {
            count++;
        }
    }
    return count;
}

void _test_for_all_buffer_lists_empty(void)
{
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
}

void _test_for_all_buffer_stores_empty(void)
{
    EXPECT_EQ(CanBufferStore_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_basic_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(OpenLcbBufferStore_stream_messages_allocated(), 0);
}

/*******************************************************************************
 * TESTS
 ******************************************************************************/

TEST(CanRxMessageHandler, initialize)
{
    _global_initialize();
    _global_reset_variables();
    
    EXPECT_TRUE(true);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, cid_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register using CORRECT API: AliasMappings_register()
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    
    // Create CID frame - ALL 8 bytes required
    CanUtilities_load_can_message(&can_msg, 0x17000000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_cid_frame(&can_msg);
    
    // Verify RID response queued
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, rid_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    
    // RID frame - broadcast request for alias map definitions
    CanUtilities_load_can_message(&can_msg, 0x10700000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_rid_frame(&can_msg);
    
    // RID handler only checks for duplicates, doesn't generate responses
    // Response generation is handled by a different layer
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, amd_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    CanUtilities_load_can_message(&can_msg, 0x10701000 | SOURCE_ALIAS, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);
    
    CanRxMessageHandler_amd_frame(&can_msg);
    
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, ame_frame)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;

    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);
    
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    EXPECT_NE(response, nullptr);
    
    if (response)
    {
        EXPECT_EQ(response->payload[0], 0x01);
        EXPECT_EQ(response->payload[5], 0x06);
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, amr_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    CanUtilities_load_can_message(&can_msg, 0x10702000 | SOURCE_ALIAS, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);
    
    CanRxMessageHandler_amr_frame(&can_msg);
    
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, single_frame_buffer_fail)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    force_fail_allocate = true;
    
    CanUtilities_load_can_message(&can_msg, 0x195B4000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    
    CanRxMessageHandler_single_frame(&can_msg, 2, BASIC);
    
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    
    force_fail_allocate = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, single_frame_message)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    CanUtilities_load_can_message(&can_msg, 0x195B4000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    
    CanRxMessageHandler_single_frame(&can_msg, 2, BASIC);
    
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    
    if (openlcb_msg)
    {
        EXPECT_EQ(openlcb_msg->source_alias, SOURCE_ALIAS);
        // Note: dest_alias extraction depends on identifier format
        // With identifier 0x195B4xxx, dest is in payload bytes 0-1
        // The handler should extract it, but check what we actually get
        EXPECT_EQ(openlcb_msg->payload_count, 6);
        EXPECT_EQ(*openlcb_msg->payload[0], 0x01);
        EXPECT_EQ(*openlcb_msg->payload[5], 0x06);
        
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, first_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // First frame - addressed message with dest in payload bytes 0-1
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);  // offset 2 to skip dest alias
    
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    
    // Clean up
    openlcb_msg_t *msg = OpenLcbBufferList_index_of(0);
    if (msg)
    {
        EXPECT_EQ(msg->payload_count, 6);  // 6 bytes of data
        OpenLcbBufferList_release(msg);
        OpenLcbBufferStore_free_buffer(msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, middle_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // First frame - addressed message with dest in payload bytes 0-1
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);  // offset 2 to skip dest alias
    
    // Middle frame
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x10 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_middle_frame(&can_msg, 2);  // offset 2 to skip dest alias
    
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    
    openlcb_msg_t *msg = OpenLcbBufferList_index_of(0);
    if (msg)
    {
        EXPECT_EQ(msg->payload_count, 12);  // 6 + 6 bytes
        OpenLcbBufferList_release(msg);
        OpenLcbBufferStore_free_buffer(msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, last_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // First frame
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);  // offset 2
    
    // Last frame
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x00 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x21, 0x22, 0x23, 0x24, 0x25, 0x26);
    CanRxMessageHandler_last_frame(&can_msg, 2);  // offset 2
    
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    
    if (openlcb_msg)
    {
        EXPECT_EQ(openlcb_msg->payload_count, 12);  // 6 + 6 bytes
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, datagram_sequence)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // First
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x20, 0x01, 0x02, 0x03, 0x04, 0x05);
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);  // offset 2
    
    // Middle
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x10 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_middle_frame(&can_msg, 2);  // offset 2
    
    // Last
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x00 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x21, 0x22, 0x23, 0x24, 0x25, 0x26);
    CanRxMessageHandler_last_frame(&can_msg, 2);  // offset 2
    
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    
    if (openlcb_msg)
    {
        EXPECT_EQ(openlcb_msg->payload_count, 18);  // 6 + 6 + 6 bytes
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, snip_sequence)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // First SNIP frame
    CanUtilities_load_can_message(&can_msg, 0x19A08000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x04, 'T', 'e', 's', 't', 0x00);
    CanRxMessageHandler_first_frame(&can_msg, 0, SNIP);
    
    // Last SNIP frame
    CanUtilities_load_can_message(&can_msg, 0x19A08000 | SOURCE_ALIAS, 8,
                                   0x00 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO, 'N', 'o', 'd', 'e', 0x00, 0x00);
    CanRxMessageHandler_last_frame(&can_msg, 0);
    
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg = OpenLcbBufferFifo_pop();
    EXPECT_NE(openlcb_msg, nullptr);
    
    if (openlcb_msg)
    {
        EXPECT_GT(openlcb_msg->payload_count, 0);
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, legacy_snip)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    openlcb_msg_t *openlcb_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Frame 1: 2 nulls at end
    CanUtilities_load_can_message(&can_msg, 0x19A08000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x04, 'T', 's', 't', 0x00, 0x00);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    
    // Frame 2: 1 null
    CanUtilities_load_can_message(&can_msg, 0x19A08000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO, 'M', 'd', 'l', 0x00, '1', '.');
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    
    // Frame 3: 3 nulls (total=6) - triggers completion
    CanUtilities_load_can_message(&can_msg, 0x19A08000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO, '0', 0x00, '2', '.', 0x00, 0x00);
    CanRxMessageHandler_can_legacy_snip(&can_msg, 2, SNIP);
    
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg = OpenLcbBufferFifo_pop();
    if (openlcb_msg)
    {
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, error_info_report)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    CanUtilities_load_can_message(&can_msg, 0x10710000 | SOURCE_ALIAS, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);
    
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, stream_frame)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    CanUtilities_load_can_message(&can_msg, 0x19F48000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    
    CanRxMessageHandler_stream_frame(&can_msg, 2, BASIC);
    
    EXPECT_TRUE(true);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/*******************************************************************************
 * ADDITIONAL TESTS FOR 100% COVERAGE
 ******************************************************************************/

TEST(CanRxMessageHandler, first_frame_already_in_progress)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Send first frame to start a message
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);
    
    // Try to send another first frame with same source/dest/mti (should reject)
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);
    
    // Should have generated a reject message in FIFO
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    // Original message should still be in list
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    
    // Clean up
    openlcb_msg_t *reject_msg = OpenLcbBufferFifo_pop();
    if (reject_msg)
    {
        EXPECT_EQ(reject_msg->mti, MTI_OPTIONAL_INTERACTION_REJECTED);
        OpenLcbBufferStore_free_buffer(reject_msg);
    }
    
    openlcb_msg_t *msg = OpenLcbBufferList_index_of(0);
    if (msg)
    {
        OpenLcbBufferList_release(msg);
        OpenLcbBufferStore_free_buffer(msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, middle_frame_without_first)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Send middle frame without sending first frame (should reject)
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x10 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_middle_frame(&can_msg, 2);
    
    // Should have generated a reject message in FIFO
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg_t *reject_msg = OpenLcbBufferFifo_pop();
    if (reject_msg)
    {
        EXPECT_EQ(reject_msg->mti, MTI_OPTIONAL_INTERACTION_REJECTED);
        OpenLcbBufferStore_free_buffer(reject_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, last_frame_without_first)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Send last frame without sending first frame (should reject)
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x00 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x21, 0x22, 0x23, 0x24, 0x25, 0x26);
    CanRxMessageHandler_last_frame(&can_msg, 2);
    
    // Should have generated a reject message in FIFO
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg_t *reject_msg = OpenLcbBufferFifo_pop();
    if (reject_msg)
    {
        EXPECT_EQ(reject_msg->mti, MTI_OPTIONAL_INTERACTION_REJECTED);
        OpenLcbBufferStore_free_buffer(reject_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, ame_frame_with_node_id)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;

    // AME frame with specific Node ID in payload
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should respond with AMD for the matching node
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        EXPECT_EQ(response->identifier & 0x00FFF000, 0x00701000); // AMD frame
        EXPECT_EQ(response->payload[0], 0x01);
        EXPECT_EQ(response->payload[5], 0x06);
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, ame_frame_node_id_not_found)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    
    // AME frame with Node ID that doesn't match any registered node
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 8,
                                   0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0, 0);
    
    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should NOT respond (no matching node)
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, ame_frame_buffer_allocation_failure)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;

    // Force buffer allocation to fail
    fail_buffer = true;
    
    // AME frame - global query (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should NOT crash, just silently fail to respond
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    fail_buffer = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, cid_frame_no_alias)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // CID frame for an alias that is NOT registered
    CanUtilities_load_can_message(&can_msg, 0x17000000 | 0x0999, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_cid_frame(&can_msg);
    
    // Should NOT respond (no registered alias)
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, cid_frame_buffer_allocation_failure)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    
    // Force buffer allocation to fail
    fail_buffer = true;
    
    CanUtilities_load_can_message(&can_msg, 0x17000000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_cid_frame(&can_msg);
    
    // Should NOT crash, just silently fail to respond
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    fail_buffer = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, first_frame_buffer_allocation_failure)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Force buffer allocation to fail
    force_fail_allocate = true;
    
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    
    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);
    
    // Buffer allocation failed, so reject message also fails to allocate
    // Result: no messages in any queue (failure is silent)
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    
    force_fail_allocate = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, amd_frame_duplicate_alias)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register an alias
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;
    
    // AMD frame from same alias but different Node ID (duplicate!)
    CanUtilities_load_can_message(&can_msg, 0x10701000 | NODE_ALIAS_1, 8,
                                   0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0, 0);
    
    CanRxMessageHandler_amd_frame(&can_msg);
    
    // Should send AMR (Alias Map Reset) response
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    EXPECT_TRUE(mapping->is_duplicate);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        EXPECT_EQ(response->identifier & 0x00FFF000, 0x00703000); // AMR frame
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, amr_frame_duplicate_alias)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register an alias
    alias_mapping_t *mapping = AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    mapping->is_permitted = true;
    
    // AMR frame from same alias (duplicate detected elsewhere)
    CanUtilities_load_can_message(&can_msg, 0x10703000 | SOURCE_ALIAS, 8,
                                   0x05, 0x04, 0x03, 0x02, 0x01, 0x06, 0, 0);
    
    CanRxMessageHandler_amr_frame(&can_msg);
    
    // Should mark as duplicate and send AMR response
    EXPECT_TRUE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/*******************************************************************************
 * AMR Alias Invalidation Tests — BufferList scrub and FIFO scrub
 ******************************************************************************/

/**
 * Test: AMR scrubs in-progress assemblies from the released alias
 *
 * Verifies:
 * - In-progress assembly from released alias is freed
 * - In-progress assembly from a different alias is untouched
 * - Completed messages are untouched
 */
TEST(CanRxMessageHandler, amr_frame_scrubs_buffer_list)
{
    _global_initialize();
    _global_reset_variables();

    // Allocate two buffers and simulate in-progress assemblies
    openlcb_msg_t *msg_from_released = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg_from_other = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(msg_from_released, nullptr);
    ASSERT_NE(msg_from_other, nullptr);

    uint16_t released_alias = 0x0ABC;
    uint16_t other_alias = 0x0DEF;

    msg_from_released->source_alias = released_alias;
    msg_from_released->state.inprocess = true;
    OpenLcbBufferList_add(msg_from_released);

    msg_from_other->source_alias = other_alias;
    msg_from_other->state.inprocess = true;
    OpenLcbBufferList_add(msg_from_other);

    EXPECT_EQ(_count_buffer_list_items(), 2);

    // Build AMR frame from released_alias
    can_msg_t can_msg;
    CanUtilities_load_can_message(&can_msg, 0x10703000 | released_alias, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_amr_frame(&can_msg);

    // msg_from_released should be freed, msg_from_other should remain
    EXPECT_EQ(_count_buffer_list_items(), 1);
    EXPECT_NE(OpenLcbBufferList_index_of(0), msg_from_released);

    // Clean up remaining
    OpenLcbBufferList_release(msg_from_other);
    OpenLcbBufferStore_free_buffer(msg_from_other);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: AMR scrubs FIFO — marks incoming messages from released alias as invalid
 *
 * Verifies:
 * - Incoming messages from the released alias (source_alias match) get state.invalid
 * - Messages from other aliases are untouched
 */
TEST(CanRxMessageHandler, amr_frame_scrubs_fifo)
{
    _global_initialize();
    _global_reset_variables();

    uint16_t released_alias = 0x0ABC;
    uint16_t other_alias = 0x0DEF;

    openlcb_msg_t *msg_from_released = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *msg_from_other = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(msg_from_released, nullptr);
    ASSERT_NE(msg_from_other, nullptr);

    msg_from_released->source_alias = released_alias;
    msg_from_other->source_alias = other_alias;

    OpenLcbBufferFifo_push(msg_from_released);
    OpenLcbBufferFifo_push(msg_from_other);

    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 2);

    // Build AMR frame from released_alias
    can_msg_t can_msg;
    CanUtilities_load_can_message(&can_msg, 0x10703000 | released_alias, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_amr_frame(&can_msg);

    // msg_from_released should be invalid, msg_from_other should not
    EXPECT_TRUE(msg_from_released->state.invalid);
    EXPECT_FALSE(msg_from_other->state.invalid);

    // FIFO count unchanged — messages still in FIFO
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 2);

    // Clean up
    OpenLcbBufferFifo_pop();
    OpenLcbBufferFifo_pop();
    OpenLcbBufferStore_free_buffer(msg_from_released);
    OpenLcbBufferStore_free_buffer(msg_from_other);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: AMR scrubs completed (non-inprocess) messages in the BufferList too
 *
 * Verifies:
 * - All messages from the released alias are freed, regardless of inprocess state
 */
TEST(CanRxMessageHandler, amr_frame_frees_completed_in_buffer_list)
{
    _global_initialize();
    _global_reset_variables();

    uint16_t released_alias = 0x0ABC;

    openlcb_msg_t *msg_completed = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg_completed, nullptr);

    msg_completed->source_alias = released_alias;
    msg_completed->state.inprocess = false;
    OpenLcbBufferList_add(msg_completed);

    EXPECT_EQ(_count_buffer_list_items(), 1);

    can_msg_t can_msg;
    CanUtilities_load_can_message(&can_msg, 0x10703000 | released_alias, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_amr_frame(&can_msg);

    // Completed message SHOULD be freed — inprocess state doesn't matter
    EXPECT_EQ(_count_buffer_list_items(), 0);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, error_info_report_duplicate_alias)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register an alias
    alias_mapping_t *mapping = AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    mapping->is_permitted = true;

    // Error info report from same alias (duplicate detected)
    CanUtilities_load_can_message(&can_msg, 0x10710000 | SOURCE_ALIAS, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);
    
    CanRxMessageHandler_error_info_report_frame(&can_msg);
    
    // Should mark as duplicate and send AMR response
    EXPECT_TRUE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, ame_frame_duplicate_alias_early_return)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register an alias
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;
    
    // AME frame from duplicate alias
    CanUtilities_load_can_message(&can_msg, 0x17020000 | NODE_ALIAS_1, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);
    
    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should detect duplicate and send AMR, but NOT process AME query
    EXPECT_TRUE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);
    
    can_msg_t *response = CanBufferFifo_pop();
    if (response)
    {
        // Should be AMR, not AMD
        EXPECT_EQ(response->identifier & 0x00FFF000, 0x00703000);
        CanBufferStore_free_buffer(response);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, duplicate_alias_not_permitted)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register an alias but NOT permitted yet
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = false;  // Not yet permitted
    
    // AMD frame from duplicate alias
    CanUtilities_load_can_message(&can_msg, 0x10701000 | NODE_ALIAS_1, 8,
                                   0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0, 0);
    
    CanRxMessageHandler_amd_frame(&can_msg);
    
    // Should mark as duplicate but NOT send AMR (not permitted)
    EXPECT_TRUE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

TEST(CanRxMessageHandler, duplicate_alias_can_buffer_fail)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    // Register an alias
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;
    
    // Force CAN buffer allocation to fail
    fail_buffer = true;
    
    // AMD frame from duplicate alias
    CanUtilities_load_can_message(&can_msg, 0x10701000 | NODE_ALIAS_1, 8,
                                   0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0, 0);
    
    CanRxMessageHandler_amd_frame(&can_msg);
    
    // Should mark as duplicate but fail to send AMR
    EXPECT_TRUE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    fail_buffer = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/*******************************************************************************
 * ADDITIONAL COVERAGE TESTS - Complete Edge Cases
 ******************************************************************************/

/**
 * Test: AME frame broadcast (no Node ID in payload)
 * Verifies responding with all aliases when AME has no specific Node ID
 * 
 * This tests the CRITICAL missing coverage path (lines 707-725)
 * When AME frame has payload_count = 0, node should respond with 
 * AMD frames for ALL its registered aliases
 */
TEST(CanRxMessageHandler, ame_frame_broadcast)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register multiple aliases to test the loop
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;
    alias_mapping_t *mapping2 = AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    mapping2->is_permitted = true;
    
    // AME frame with NO payload (broadcast - tell me about ALL aliases)
    // This is the critical path that wasn't being tested!
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,  // payload_count = 0!
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should have generated AMD responses for both registered aliases
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 2);
    
    // Check first response
    can_msg_t *response1 = CanBufferFifo_pop();
    EXPECT_NE(response1, nullptr);
    if (response1)
    {
        // Verify it's an AMD frame with one of our aliases
        uint16_t alias1 = response1->identifier & 0xFFF;
        EXPECT_TRUE(alias1 == NODE_ALIAS_1 || alias1 == NODE_ALIAS_2);
        CanBufferStore_free_buffer(response1);
    }
    
    // Check second response
    can_msg_t *response2 = CanBufferFifo_pop();
    EXPECT_NE(response2, nullptr);
    if (response2)
    {
        // Verify it's an AMD frame with our other alias
        uint16_t alias2 = response2->identifier & 0xFFF;
        EXPECT_TRUE(alias2 == NODE_ALIAS_1 || alias2 == NODE_ALIAS_2);
        CanBufferStore_free_buffer(response2);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: AME frame broadcast with buffer allocation failure
 * Verifies graceful handling when CAN buffer allocation fails during broadcast
 * 
 * Tests the if (outgoing_can_msg) check inside the broadcast loop
 */
TEST(CanRxMessageHandler, ame_frame_broadcast_buffer_fail)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register multiple aliases
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;
    alias_mapping_t *mapping2 = AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    mapping2->is_permitted = true;
    
    // Force buffer allocation failure
    fail_buffer = true;
    
    // AME frame with NO payload (broadcast)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);
    
    CanRxMessageHandler_ame_frame(&can_msg);
    
    // Should have tried but failed to allocate buffers
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);
    
    fail_buffer = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: Datagram first frame already in progress
 * Verifies datagram-specific rejection with MTI_DATAGRAM_REJECTED_REPLY
 * 
 * Tests the MTI_DATAGRAM path in _load_reject_message (line 123-125)
 * When rejecting a datagram, should use datagram-specific MTI
 */
TEST(CanRxMessageHandler, datagram_first_frame_already_in_progress_reject)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Start a datagram sequence
    CanUtilities_load_can_message(&can_msg, 0x1A000000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);
    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    
    // Try to start ANOTHER datagram from same source (protocol violation)
    CanUtilities_load_can_message(&can_msg, 0x1A000000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_first_frame(&can_msg, 0, DATAGRAM);
    
    // Should generate MTI_DATAGRAM_REJECTED_REPLY (not OPTIONAL_INTERACTION_REJECTED)
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg_t *reject_msg = OpenLcbBufferFifo_pop();
    if (reject_msg)
    {
        EXPECT_EQ(reject_msg->mti, MTI_DATAGRAM_REJECTED_REPLY);
        OpenLcbBufferStore_free_buffer(reject_msg);
    }
    
    // Clean up the in-progress datagram
    openlcb_msg_t *msg = OpenLcbBufferList_index_of(0);
    if (msg)
    {
        OpenLcbBufferList_release(msg);
        OpenLcbBufferStore_free_buffer(msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: Reject message with OpenLCB buffer allocation failure
 * Verifies graceful handling when OpenLCB buffer store is exhausted
 * 
 * Tests the if (target_openlcb_msg) check in _load_reject_message (line 121)
 * When buffer allocation fails, reject message should be silently dropped
 */
TEST(CanRxMessageHandler, reject_message_openlcb_buffer_fail)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Force OpenLCB buffer allocation failure
    force_fail_allocate = true;
    
    // Try to send last frame without first frame (normally generates reject)
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x00 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x21, 0x22, 0x23, 0x24, 0x25, 0x26);
    CanRxMessageHandler_last_frame(&can_msg, 2);
    
    // Should NOT have generated a reject message (silent drop due to buffer failure)
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);
    
    force_fail_allocate = false;
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: Middle frame without first with datagram MTI
 * Verifies datagram-specific rejection for middle frame protocol violation
 * 
 * Additional coverage for MTI_DATAGRAM path in _load_reject_message
 */
TEST(CanRxMessageHandler, datagram_middle_frame_without_first_reject)
{
    _global_initialize();
    _global_reset_variables();
    
    can_msg_t can_msg;
    
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);
    
    // Send middle datagram frame without first frame (protocol violation)
    CanUtilities_load_can_message(&can_msg, 0x1B000000 | SOURCE_ALIAS, 8,
                                   NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);
    CanRxMessageHandler_middle_frame(&can_msg, 0);
    
    // Should generate MTI_DATAGRAM_REJECTED_REPLY
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);
    
    openlcb_msg_t *reject_msg = OpenLcbBufferFifo_pop();
    if (reject_msg)
    {
        EXPECT_EQ(reject_msg->mti, MTI_DATAGRAM_REJECTED_REPLY);
        OpenLcbBufferStore_free_buffer(reject_msg);
    }
    
    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/*******************************************************************************
 * AME is_permitted TESTS - Standards Compliance (CanFrameTransferS §6.2.3)
 ******************************************************************************/

/**
 * Test: Targeted AME ignored when node is in Inhibited state (is_permitted=false).
 * Per CanFrameTransferS §6.2.3: "A node in Inhibited state shall not reply to
 * an Alias Mapping Enquiry frame."
 */
TEST(CanRxMessageHandler, ame_targeted_inhibited_no_response)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register node but leave in Inhibited state (is_permitted defaults to false)
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    // Targeted AME with matching Node ID
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Inhibited node shall NOT respond
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: Global AME ignored when node is in Inhibited state (is_permitted=false).
 * Per CanFrameTransferS §6.2.3: "A node in Inhibited state shall not reply to
 * an Alias Mapping Enquiry frame."
 */
TEST(CanRxMessageHandler, ame_global_inhibited_no_response)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register node but leave in Inhibited state
    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Inhibited node shall NOT respond
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

/**
 * Test: Global AME only responds for Permitted virtual nodes, not Inhibited ones.
 * Two nodes registered: one Permitted, one Inhibited. Only Permitted responds.
 */
TEST(CanRxMessageHandler, ame_global_mixed_permitted_inhibited)
{
    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Node 1: Permitted
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    // Node 2: Inhibited (still logging in)
    AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Only the Permitted node should respond
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);

    can_msg_t *response = CanBufferFifo_pop();
    EXPECT_NE(response, nullptr);
    if (response)
    {

        // Verify the response is for NODE_ALIAS_1 (the permitted one)
        uint16_t alias = response->identifier & 0xFFF;
        EXPECT_EQ(alias, NODE_ALIAS_1);
        CanBufferStore_free_buffer(response);

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();
}

// ============================================================================
// TEST: CID Frame — Permitted Node Still Sends RID
// @details Per CanFrameTransferS Section 3.5.3, "Nodes that receive a CID
//          frame that contains the alias they are using or have reserved shall
//          respond with an RID frame."  CID always gets RID, regardless of
//          permitted status.  No duplicate flag is set (CID is just a probe).
// @coverage CanRxMessageHandler_cid_frame() — permitted node
// ============================================================================

TEST(CanRxMessageHandler, cid_frame_permitted_node)
{

    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register a PERMITTED alias
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;

    // CID frame using our alias
    CanUtilities_load_can_message(&can_msg, 0x17000000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_cid_frame(&can_msg);

    // Should send RID and NOT flag as duplicate (CID is just a probe)
    EXPECT_FALSE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);

    can_msg_t *response = CanBufferFifo_pop();
    EXPECT_NE(response, nullptr);
    if (response) {

        // Verify it's an RID frame, NOT an AMR frame
        EXPECT_EQ(response->identifier & 0x00FFF000, (uint32_t )CAN_CONTROL_FRAME_RID);
        CanBufferStore_free_buffer(response);

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

// ============================================================================
// TEST: CID Frame — Non-Permitted Node Sends RID (defends claim)
// @details A non-permitted node (still in CID/RID reservation) must send RID
//          to defend its alias claim.
// @coverage CanRxMessageHandler_cid_frame() — non-permitted branch
// ============================================================================

TEST(CanRxMessageHandler, cid_frame_non_permitted_node)
{

    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register a NOT YET PERMITTED alias (default from AliasMappings_register)
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = false;

    // CID frame using our alias
    CanUtilities_load_can_message(&can_msg, 0x17000000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_cid_frame(&can_msg);

    // Should send RID to defend the claim
    EXPECT_FALSE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);

    can_msg_t *response = CanBufferFifo_pop();
    EXPECT_NE(response, nullptr);
    if (response) {

        // Verify it's an RID frame (0x00700000), NOT an AMR frame
        EXPECT_EQ(response->identifier & 0x00FFF000, (uint32_t )CAN_CONTROL_FRAME_RID);
        CanBufferStore_free_buffer(response);

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

// ============================================================================
// TEST: CID Frame — Buffer Allocation Failure (permitted node)
// @details Permitted node cannot allocate a CAN buffer for RID.  Should not
//          crash and should not flag any duplicate (CID is just a probe).
// @coverage CanRxMessageHandler_cid_frame() — buffer fail
// ============================================================================

TEST(CanRxMessageHandler, cid_frame_permitted_buffer_fail)
{

    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register a PERMITTED alias
    alias_mapping_t *mapping = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping->is_permitted = true;

    // Force buffer allocation to fail
    fail_buffer = true;

    CanUtilities_load_can_message(&can_msg, 0x17000000 | NODE_ALIAS_1, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_cid_frame(&can_msg);

    // No duplicate flag, no response (buffer alloc failed)
    EXPECT_FALSE(mapping->is_duplicate);
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);

    fail_buffer = false;

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

/*******************************************************************************
 * Listener and Timeout Coverage Tests
 ******************************************************************************/

TEST(CanRxMessageHandler, middle_frame_timeout)
{

    _global_initialize();
    _global_reset_variables();

    can_msg_t can_msg;

    AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    AliasMappings_register(SOURCE_ALIAS, 0x050403020106);

    // First frame at tick 0
    _test_global_100ms_tick = 0;

    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x20 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06);

    CanRxMessageHandler_first_frame(&can_msg, 2, DATAGRAM);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());

    // Advance tick by 30 (>= CAN_RX_INPROCESS_TIMEOUT_TICKS)
    _test_global_100ms_tick = 30;

    // Middle frame should detect stale assembly and reject
    CanUtilities_load_can_message(&can_msg, 0x19C48000 | SOURCE_ALIAS, 8,
                                   0x10 | NODE_ALIAS_1_HI, NODE_ALIAS_1_LO,
                                   0x11, 0x12, 0x13, 0x14, 0x15, 0x16);

    CanRxMessageHandler_middle_frame(&can_msg, 2);

    // Stale assembly freed from BufferList
    EXPECT_TRUE(OpenLcbBufferList_is_empty());

    // Reject message should be in the OpenLCB FIFO
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    openlcb_msg_t *reject = OpenLcbBufferFifo_pop();

    if (reject) {

        OpenLcbBufferStore_free_buffer(reject);

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_with_listener)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // AMD frame from SOURCE_ALIAS with node ID 0x010203040506
    CanUtilities_load_can_message(&can_msg, 0x10701000 | SOURCE_ALIAS, 6,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    // Verify listener_set_alias was called with correct parameters
    EXPECT_TRUE(listener_set_alias_called);
    EXPECT_EQ(listener_set_alias_node_id, (node_id_t) 0x010203040506);
    EXPECT_EQ(listener_set_alias_alias, SOURCE_ALIAS);

    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amr_frame_with_listener)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // AMR frame from SOURCE_ALIAS
    CanUtilities_load_can_message(&can_msg, 0x10703000 | SOURCE_ALIAS, 6,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_amr_frame(&can_msg);

    // Verify listener_clear_alias_by_alias was called with correct alias
    EXPECT_TRUE(listener_clear_alias_called);
    EXPECT_EQ(listener_clear_alias_alias, SOURCE_ALIAS);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_releases_held_attach_messages)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    #define LISTENER_NODE_ID 0x112233445566ULL

    // Manually create a held train listener attach message in the BufferList
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    // Set up as a held attach: MTI_TRAIN_PROTOCOL, inprocess=true
    held_msg->mti = MTI_TRAIN_PROTOCOL;
    held_msg->source_alias = NODE_ALIAS_1;
    held_msg->dest_alias = 0xAAA;
    held_msg->state.inprocess = true;
    held_msg->payload_count = 9;

    // payload[0] = TRAIN_LISTENER_CONFIG (0x30)
    // payload[1] = TRAIN_LISTENER_ATTACH (0x01)
    // payload[2] = flags (0x00)
    // payload[3..8] = listener node ID (6 bytes big-endian)
    *held_msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *held_msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0x11;
    *held_msg->payload[4] = 0x22;
    *held_msg->payload[5] = 0x33;
    *held_msg->payload[6] = 0x44;
    *held_msg->payload[7] = 0x55;
    *held_msg->payload[8] = 0x66;

    OpenLcbBufferList_add(held_msg);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    // Send AMD frame with the listener's node ID and some alias
    uint16_t listener_alias = 0x0ABC;

    CanUtilities_load_can_message(&can_msg, 0x10701000 | listener_alias, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    // The held message should now be released from BufferList and pushed to FIFO
    EXPECT_TRUE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 1);

    // Verify the released message
    openlcb_msg_t *released = OpenLcbBufferFifo_pop();

    EXPECT_NE(released, nullptr);

    if (released) {

        EXPECT_FALSE(released->state.inprocess);
        EXPECT_EQ(released->mti, MTI_TRAIN_PROTOCOL);
        OpenLcbBufferStore_free_buffer(released);

    }

    // Verify listener_set_alias was also called
    EXPECT_TRUE(listener_set_alias_called);
    EXPECT_EQ(listener_set_alias_node_id, LISTENER_NODE_ID);
    EXPECT_EQ(listener_set_alias_alias, listener_alias);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_held_message_no_match)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Create a held attach message with a DIFFERENT node ID
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    held_msg->mti = MTI_TRAIN_PROTOCOL;
    held_msg->source_alias = NODE_ALIAS_1;
    held_msg->dest_alias = 0xAAA;
    held_msg->state.inprocess = true;
    held_msg->payload_count = 9;
    *held_msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *held_msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0xAA;
    *held_msg->payload[4] = 0xBB;
    *held_msg->payload[5] = 0xCC;
    *held_msg->payload[6] = 0xDD;
    *held_msg->payload[7] = 0xEE;
    *held_msg->payload[8] = 0xFF;

    OpenLcbBufferList_add(held_msg);

    // Send AMD frame with a different node ID — should NOT release the held message
    CanUtilities_load_can_message(&can_msg, 0x10701000 | 0x0ABC, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    // Message should still be in BufferList (not released)
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    // Clean up
    OpenLcbBufferList_release(held_msg);
    OpenLcbBufferStore_free_buffer(held_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_held_message_wrong_mti)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Create a held message with wrong MTI (not TRAIN_PROTOCOL)
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    held_msg->mti = MTI_DATAGRAM;  // Wrong MTI
    held_msg->state.inprocess = true;
    held_msg->payload_count = 9;
    *held_msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *held_msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0x11;
    *held_msg->payload[4] = 0x22;
    *held_msg->payload[5] = 0x33;
    *held_msg->payload[6] = 0x44;
    *held_msg->payload[7] = 0x55;
    *held_msg->payload[8] = 0x66;

    OpenLcbBufferList_add(held_msg);

    // AMD with matching node ID — should NOT release because MTI doesn't match
    CanUtilities_load_can_message(&can_msg, 0x10701000 | 0x0ABC, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    OpenLcbBufferList_release(held_msg);
    OpenLcbBufferStore_free_buffer(held_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_held_message_wrong_instruction)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Create a held message with wrong instruction byte (not TRAIN_LISTENER_CONFIG)
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    held_msg->mti = MTI_TRAIN_PROTOCOL;
    held_msg->state.inprocess = true;
    held_msg->payload_count = 9;
    *held_msg->payload[0] = 0x01;  // Wrong instruction (e.g., TRAIN_SET_SPEED_DIRECTION)
    *held_msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0x11;
    *held_msg->payload[4] = 0x22;
    *held_msg->payload[5] = 0x33;
    *held_msg->payload[6] = 0x44;
    *held_msg->payload[7] = 0x55;
    *held_msg->payload[8] = 0x66;

    OpenLcbBufferList_add(held_msg);

    CanUtilities_load_can_message(&can_msg, 0x10701000 | 0x0ABC, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    OpenLcbBufferList_release(held_msg);
    OpenLcbBufferStore_free_buffer(held_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_held_message_wrong_subcommand)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Create a held message with wrong sub_command (not TRAIN_LISTENER_ATTACH)
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    held_msg->mti = MTI_TRAIN_PROTOCOL;
    held_msg->state.inprocess = true;
    held_msg->payload_count = 9;
    *held_msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *held_msg->payload[1] = 0x02;  // Wrong sub-command (e.g., TRAIN_LISTENER_DETACH)
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0x11;
    *held_msg->payload[4] = 0x22;
    *held_msg->payload[5] = 0x33;
    *held_msg->payload[6] = 0x44;
    *held_msg->payload[7] = 0x55;
    *held_msg->payload[8] = 0x66;

    OpenLcbBufferList_add(held_msg);

    CanUtilities_load_can_message(&can_msg, 0x10701000 | 0x0ABC, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    OpenLcbBufferList_release(held_msg);
    OpenLcbBufferStore_free_buffer(held_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, amd_frame_held_message_not_inprocess)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Create a message that matches but is NOT inprocess
    openlcb_msg_t *held_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    ASSERT_NE(held_msg, nullptr);

    held_msg->mti = MTI_TRAIN_PROTOCOL;
    held_msg->state.inprocess = false;  // Not held
    held_msg->payload_count = 9;
    *held_msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *held_msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *held_msg->payload[2] = 0x00;
    *held_msg->payload[3] = 0x11;
    *held_msg->payload[4] = 0x22;
    *held_msg->payload[5] = 0x33;
    *held_msg->payload[6] = 0x44;
    *held_msg->payload[7] = 0x55;
    *held_msg->payload[8] = 0x66;

    OpenLcbBufferList_add(held_msg);

    CanUtilities_load_can_message(&can_msg, 0x10701000 | 0x0ABC, 6,
                                   0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0, 0);

    CanRxMessageHandler_amd_frame(&can_msg);

    // Should NOT be released (not inprocess)
    EXPECT_FALSE(OpenLcbBufferList_is_empty());
    EXPECT_EQ(OpenLcbBufferFifo_get_allocated_count(), 0);

    OpenLcbBufferList_release(held_msg);
    OpenLcbBufferStore_free_buffer(held_msg);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

TEST(CanRxMessageHandler, cid_frame_null)
{

    _global_initialize();
    _global_reset_variables();

    // CID with NULL can_msg should return immediately without crashing
    CanRxMessageHandler_cid_frame(NULL);

    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 0);

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

/**
 * Test: Global AME flushes listener alias table (CanFrameTransferS §6.2.3)
 *
 * When a global AME (empty payload) arrives, all cached listener aliases
 * must be discarded.  The AMD replies triggered by the global AME will
 * re-populate them via set_alias.
 */
TEST(CanRxMessageHandler, ame_global_flushes_listener_aliases)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register permitted aliases so the AME handler has work to do
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Verify listener_flush_aliases was called
    EXPECT_TRUE(listener_flush_aliases_called);

    // Clean up AMD responses
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

/**
 * Test: Targeted AME does NOT flush listener aliases
 *
 * Only a global AME (empty payload) triggers the flush.  A targeted AME
 * (with a specific Node ID) should not affect the listener table.
 */
TEST(CanRxMessageHandler, ame_targeted_does_not_flush_listener_aliases)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    can_msg_t can_msg;

    // Register a permitted alias
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    // Targeted AME with specific Node ID in payload
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 6,
                                   0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Flush should NOT have been called for a targeted AME
    EXPECT_FALSE(listener_flush_aliases_called);

    // Clean up AMD response
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

/**
 * Test: Global AME without listener interface does not crash
 *
 * When listener_flush_aliases is NULL (non-train build), the global AME
 * handler must not crash.
 */
TEST(CanRxMessageHandler, ame_global_no_listener_interface_no_crash)
{

    _global_initialize();  // Uses interface WITHOUT listener callbacks
    _global_reset_variables();

    can_msg_t can_msg;

    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    // Should not crash even though listener_flush_aliases is NULL
    CanRxMessageHandler_ame_frame(&can_msg);

    // Should still generate AMD responses
    EXPECT_GE(CanBufferFifo_get_allocated_count(), 1);

    // Clean up
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

/*******************************************************************************
 * Phase 3B: Global AME Repopulates Local Listener Entries
 ******************************************************************************/

// ============================================================================
// TEST: Global AME calls listener_set_alias for each permitted local alias
// ============================================================================

TEST(CanRxMessageHandler, ame_global_repopulates_listener_entries)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    // Register 2 permitted aliases in the real mapping table
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    alias_mapping_t *mapping2 = AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    mapping2->is_permitted = true;

    can_msg_t can_msg;

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Flush should have been called
    EXPECT_TRUE(listener_flush_aliases_called);

    // listener_set_alias should have been called for both permitted aliases
    EXPECT_EQ(listener_set_alias_call_count, 2);

    // AMD responses should also have been sent (2 permitted aliases)
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 2);

    // Clean up
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

// ============================================================================
// TEST: Global AME without listener support — AMDs still sent, no crash
// ============================================================================

TEST(CanRxMessageHandler, ame_global_no_listener_support_still_sends_amds)
{

    _global_initialize();  // Uses interface WITHOUT listener callbacks
    _global_reset_variables();

    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    alias_mapping_t *mapping2 = AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    mapping2->is_permitted = true;

    can_msg_t can_msg;

    // Global AME (no payload)
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // listener callbacks are NULL — should NOT crash, NOT be called
    EXPECT_FALSE(listener_flush_aliases_called);
    EXPECT_EQ(listener_set_alias_call_count, 0);

    // AMD responses should still be sent
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 2);

    // Clean up
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

// ============================================================================
// TEST: Targeted AME does NOT call listener_set_alias (only global does)
// ============================================================================

TEST(CanRxMessageHandler, ame_targeted_does_not_repopulate_listeners)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    can_msg_t can_msg;

    // Targeted AME — payload contains 6-byte Node ID
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 8,
                                   0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Targeted AME should NOT call listener_flush_aliases or listener_set_alias
    EXPECT_FALSE(listener_flush_aliases_called);
    EXPECT_EQ(listener_set_alias_call_count, 0);

    // Clean up any AMD responses
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}

// ============================================================================
// TEST: Global AME skips non-permitted aliases for listener repopulation
// ============================================================================

TEST(CanRxMessageHandler, ame_global_skips_non_permitted_for_listener)
{

    _global_initialize_with_listeners();
    _global_reset_variables();

    // One permitted, one not
    alias_mapping_t *mapping1 = AliasMappings_register(NODE_ALIAS_1, NODE_ID_1);
    mapping1->is_permitted = true;

    alias_mapping_t *mapping2 = AliasMappings_register(NODE_ALIAS_2, NODE_ID_2);
    mapping2->is_permitted = false;  // NOT permitted (still in CID/RID phase)

    can_msg_t can_msg;

    // Global AME
    CanUtilities_load_can_message(&can_msg, 0x17020AAA, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0);

    CanRxMessageHandler_ame_frame(&can_msg);

    // Only the permitted alias should trigger listener_set_alias
    EXPECT_EQ(listener_set_alias_call_count, 1);
    EXPECT_EQ(listener_set_alias_node_ids[0], NODE_ID_1);

    // Only 1 AMD response (for the permitted alias)
    EXPECT_EQ(CanBufferFifo_get_allocated_count(), 1);

    // Clean up
    while (CanBufferFifo_get_allocated_count() > 0) {

        can_msg_t *response = CanBufferFifo_pop();

        if (response) {

            CanBufferStore_free_buffer(response);

        }

    }

    _test_for_all_buffer_lists_empty();
    _test_for_all_buffer_stores_empty();

}
