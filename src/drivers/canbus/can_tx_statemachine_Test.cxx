/*******************************************************************************
 * File: can_tx_statemachine_Test.cxx
 * 
 * Description:
 *   Comprehensive test suite for CAN TX State Machine module.
 *   Tests outgoing message routing and frame transmission.
 *
 * Module Under Test:
 *   CanTxStatemachine - Routes outgoing OpenLCB messages to CAN frames
 *
 * Test Coverage:
 *   - Module initialization
 *   - CAN frame transmission (control frames, etc)
 *   - OpenLCB message routing:
 *     * Addressed messages (with destination)
 *     * Unaddressed/global messages
 *     * Datagram transmission (single and multi-frame)
 *     * Stream frame transmission
 *   - Buffer availability checking
 *   - Error handling (buffer full, transmission failures)
 *
 * Design Notes:
 *   The TX State Machine is responsible for:
 *   
 *   1. Message Classification:
 *      - Determine if addressed or unaddressed based on MTI
 *      - Identify datagrams and streams
 *      - Route to appropriate frame handler
 *   
 *   2. Frame Transmission:
 *      - Check if CAN TX buffer is available
 *      - Call appropriate handler to format CAN frames
 *      - Handle multi-frame sequences
 *   
 *   3. Flow Control:
 *      - Verify buffer availability before sending
 *      - Handle transmission failures
 *      - Coordinate with TX buffer management
 *
 * OpenLCB Message Types:
 *   
 *   Addressed Messages:
 *     - Have destination Node ID/alias
 *     - MTI bit 3 = 1 (MASK_DEST_ADDRESS_PRESENT)
 *     - Examples: Verify Node ID Addressed, Datagram
 *   
 *   Global/Unaddressed Messages:
 *     - No specific destination
 *     - MTI bit 3 = 0
 *     - Examples: Verify Node ID Global, Event Reports
 *   
 *   Datagrams:
 *     - Can be single or multi-frame
 *     - Use special CAN frame types (0x02-0x05)
 *     - Examples: Configuration memory operations
 *   
 *   Streams:
 *     - Continuous data flow
 *     - Use CAN frame type 0x07
 *     - Examples: Firmware updates, bulk data transfer
 *
 * Author: [Your Name]
 * Date: 2026-01-19
 ******************************************************************************/

#include "test/main_Test.hxx"

#include "can_tx_statemachine.h"
#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../openlcb/openlcb_utilities.h"

/*******************************************************************************
 * Mock Handler Tracking Variables
 ******************************************************************************/

// Track which handlers were called
bool _handle_addressed_msg_frame_called = false;
bool _handle_unaddressed_msg_frame_called = false;
bool _handle_datagram_frame_called = false;
bool _handle_stream_frame_called = false;
bool _handle_can_frame_called = false;
bool _is_can_tx_buffer_empty_called = false;

// Control mock behavior
bool _is_can_tx_buffer_empty_disabled = false;  // Simulate buffer full
bool _fail_handle_stream_frame = false;         // Simulate stream send failure
bool _fail_handle_addressed_frame = false;      // Simulate addressed frame failure
bool _fail_handle_datagram_frame = false;       // Simulate datagram failure
bool _fail_handle_unaddressed_frame = false;    // Simulate unaddressed frame failure
bool _fail_handle_can_frame = false;            // Simulate CAN frame failure

// Store last sent CAN message for verification
can_msg_t sent_can_msg;

/*******************************************************************************
 * Mock Handler Implementations
 ******************************************************************************/

/**
 * Mock: Handle addressed message frame
 * Simulates converting OpenLCB addressed message to CAN frame(s)
 * 
 * @param openlcb_msg Source OpenLCB message
 * @param can_msg_worker Working CAN frame buffer
 * @param openlcb_start_index Index into OpenLCB payload (updated)
 * @return true if frame sent successfully
 */
bool _handle_addressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker, 
                                  uint16_t *openlcb_start_index)
{
    _handle_addressed_msg_frame_called = true;
    
    // Simulate failure if requested
    if (_fail_handle_addressed_frame)
    {
        return false;
    }
    
    // Simulate processing 8 bytes per frame
    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {
        *openlcb_start_index = openlcb_msg->payload_count;
    }
    
    return true;
}

/**
 * Mock: Handle unaddressed/global message frame
 * Simulates converting OpenLCB global message to CAN frame(s)
 */
bool _handle_unaddressed_msg_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker,
                                     uint16_t *openlcb_start_index)
{
    _handle_unaddressed_msg_frame_called = true;
    
    // Simulate failure if requested
    if (_fail_handle_unaddressed_frame)
    {
        return false;
    }
    
    // Simulate processing 8 bytes per frame
    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {
        *openlcb_start_index = openlcb_msg->payload_count;
    }
    
    return true;
}

/**
 * Mock: Handle datagram frame
 * Simulates converting OpenLCB datagram to CAN frame(s)
 */
bool _handle_datagram_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker,
                             uint16_t *openlcb_start_index)
{
    _handle_datagram_frame_called = true;
    
    // Simulate failure if requested
    if (_fail_handle_datagram_frame)
    {
        return false;
    }
    
    // Simulate processing 8 bytes per frame
    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {
        *openlcb_start_index = openlcb_msg->payload_count;
    }
    
    return true;
}

/**
 * Mock: Handle stream frame
 * Simulates converting OpenLCB stream data to CAN frame
 */
bool _handle_stream_frame(openlcb_msg_t *openlcb_msg, can_msg_t *can_msg_worker,
                          uint16_t *openlcb_start_index)
{
    _handle_stream_frame_called = true;
    
    // Simulate failure if requested
    if (_fail_handle_stream_frame)
    {
        return false;
    }
    
    // Simulate processing 8 bytes per frame
    *openlcb_start_index = *openlcb_start_index + 8;
    if (*openlcb_start_index > openlcb_msg->payload_count)
    {
        *openlcb_start_index = openlcb_msg->payload_count;
    }
    
    return true;
}

/**
 * Mock: Handle direct CAN frame transmission
 * Simulates sending a raw CAN frame (control frames, etc)
 */
bool _handle_can_frame(can_msg_t *can_msg)
{
    _handle_can_frame_called = true;
    
    // Simulate failure if requested
    if (_fail_handle_can_frame)
    {
        return false;
    }
    
    // Store the message for verification
    CanUtilities_copy_can_message(can_msg, &sent_can_msg);
    
    return true;
}

/**
 * Mock: Check if CAN TX buffer is empty
 * Simulates checking hardware buffer availability
 */
bool _is_can_tx_buffer_empty(void)
{
    _is_can_tx_buffer_empty_called = true;
    
    // Return false if we're simulating buffer full
    if (_is_can_tx_buffer_empty_disabled)
    {
        return false;
    }
    
    return true;
}

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/

/**
 * Compare CAN message for verification
 * 
 * @param can_msg Message to check
 * @param identifier Expected identifier
 * @param payload_size Expected payload size
 * @param bytes Expected payload bytes
 * @return true if message matches
 */
bool compare_can_msg(can_msg_t *can_msg, uint32_t identifier, uint8_t payload_size, 
                     uint8_t bytes[])
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

/*******************************************************************************
 * Interface Definition
 ******************************************************************************/

const interface_can_tx_statemachine_t interface_can_tx_statemachine = {
    .is_tx_buffer_empty = &_is_can_tx_buffer_empty,
    .handle_addressed_msg_frame = &_handle_addressed_msg_frame,
    .handle_can_frame = &_handle_can_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame
};

/*******************************************************************************
 * Listener Alias Resolution Mock
 ******************************************************************************/

// Track listener mock calls
bool _listener_find_by_node_id_called = false;
node_id_t _listener_find_by_node_id_last_arg = 0;

// Control listener mock return value
listener_alias_entry_t *_listener_find_return_value = NULL;

// Static entry for the mock to return when non-NULL
listener_alias_entry_t _listener_mock_entry;

/**
 * Mock: Resolve a listener Node ID to its CAN alias entry
 * Returns _listener_find_return_value (NULL, or pointer to _listener_mock_entry)
 */
listener_alias_entry_t *_mock_listener_find_by_node_id(node_id_t node_id)
{

    _listener_find_by_node_id_called = true;
    _listener_find_by_node_id_last_arg = node_id;

    return _listener_find_return_value;

}

/*******************************************************************************
 * Interface With Listener Support
 ******************************************************************************/

const interface_can_tx_statemachine_t interface_can_tx_statemachine_with_listener = {
    .is_tx_buffer_empty = &_is_can_tx_buffer_empty,
    .handle_addressed_msg_frame = &_handle_addressed_msg_frame,
    .handle_can_frame = &_handle_can_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame,
    .listener_find_by_node_id = &_mock_listener_find_by_node_id
};

/**
 * Reset all mock tracking variables
 */
void _reset_variables(void)
{

    _handle_addressed_msg_frame_called = false;
    _handle_unaddressed_msg_frame_called = false;
    _handle_datagram_frame_called = false;
    _handle_stream_frame_called = false;
    _handle_can_frame_called = false;
    _is_can_tx_buffer_empty_called = false;
    _is_can_tx_buffer_empty_disabled = false;
    _fail_handle_stream_frame = false;
    _fail_handle_addressed_frame = false;
    _fail_handle_datagram_frame = false;
    _fail_handle_unaddressed_frame = false;
    _fail_handle_can_frame = false;

    _listener_find_by_node_id_called = false;
    _listener_find_by_node_id_last_arg = 0;
    _listener_find_return_value = NULL;
    _listener_mock_entry.node_id = 0;
    _listener_mock_entry.alias = 0;

}

/**
 * Initialize all subsystems (no listener support)
 */
void _initialize(void)
{

    OpenLcbBufferStore_initialize();
    CanTxStatemachine_initialize(&interface_can_tx_statemachine);

}

/**
 * Initialize all subsystems with listener alias resolution support
 */
void _initialize_with_listener(void)
{

    OpenLcbBufferStore_initialize();
    CanTxStatemachine_initialize(&interface_can_tx_statemachine_with_listener);

}

/*******************************************************************************
 * Tests
 ******************************************************************************/

/**
 * Test: Module initialization
 * Verifies TX statemachine can be initialized
 */
TEST(CanTxStatemachine, initialize)
{
    _reset_variables();
    _initialize();
}

/**
 * Test: Send CAN message directly
 * Verifies raw CAN frames (control frames, etc) can be sent
 * 
 * This path is used for:
 * - CID frames during alias allocation
 * - RID frames
 * - AMD frames
 * - Other control frames
 */
TEST(CanTxStatemachine, send_can_message)
{
    can_msg_t can_msg;
    
    _reset_variables();
    _initialize();
    
    // Create a CAN message (e.g., RID frame)
    CanUtilities_load_can_message(&can_msg, 0x170506BE, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    
    EXPECT_TRUE(CanTxStatemachine_send_can_message(&can_msg));
    EXPECT_TRUE(_handle_can_frame_called);
    EXPECT_TRUE(compare_can_msg(&can_msg, sent_can_msg.identifier, 
                                 sent_can_msg.payload_count, sent_can_msg.payload));
}

/**
 * Test: Send OpenLCB messages
 * Comprehensive test of all OpenLCB message types and routing
 * 
 * Tests:
 * 1. Global/unaddressed message (Verify Node ID Global)
 * 2. Addressed message with payload (Verify Node ID Addressed)
 * 3. Single-frame datagram
 * 4. Multi-frame datagram
 * 5. Stream frame
 * 6. Buffer full condition
 * 7. Stream transmission failure
 */
TEST(CanTxStatemachine, send_openlcb_message)
{
    _initialize();
    
    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    EXPECT_NE(openlcb_msg, nullptr);
    
    if (openlcb_msg)
    {
        // ====================================================================
        // Test 1: Global/Unaddressed Message
        // ====================================================================
        // Verify Node ID Global has no destination
        // Should route to unaddressed handler
        _reset_variables();
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_VERIFY_NODE_ID_GLOBAL);
        
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_TRUE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 2: Addressed Message with Payload
        // ====================================================================
        // Verify Node ID Addressed has destination
        // Should route to addressed handler
        _reset_variables();
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_VERIFY_NODE_ID_ADDRESSED);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);
        
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_TRUE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 3: Single-Frame Datagram
        // ====================================================================
        // Datagram with 6 bytes fits in single CAN frame
        // Should route to datagram handler
        _reset_variables();
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_DATAGRAM);
        openlcb_msg->payload_count = 6;
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);
        
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_TRUE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 4: Multi-Frame Datagram
        // ====================================================================
        // Datagram with 24 bytes requires multiple CAN frames
        // Should route to datagram handler (handles segmentation)
        _reset_variables();
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_DATAGRAM);
        openlcb_msg->payload_count = 24;
        
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_TRUE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 5: Stream Frame
        // ====================================================================
        // Stream messages use special routing
        // Should route to stream handler
        _reset_variables();
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_STREAM_PROCEED);
        openlcb_msg->payload_count = 8;
        
        EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_TRUE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 6: Buffer Full Condition
        // ====================================================================
        // TX buffer not empty - should reject message
        // Verifies flow control
        _reset_variables();
        _is_can_tx_buffer_empty_disabled = true;
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_VERIFY_NODE_ID_GLOBAL);
        
        EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_FALSE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        // ====================================================================
        // Test 7: Stream Transmission Failure
        // ====================================================================
        // Stream handler fails - should propagate failure
        // Verifies error handling
        _reset_variables();
        _fail_handle_stream_frame = true;
        
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                               0xBBB, 0x060504030201,
                                               MTI_STREAM_PROCEED);
        openlcb_msg->payload_count = 8;
        
        EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
        EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
        EXPECT_FALSE(_handle_addressed_msg_frame_called);
        EXPECT_FALSE(_handle_datagram_frame_called);
        EXPECT_TRUE(_handle_stream_frame_called);
        EXPECT_FALSE(_handle_can_frame_called);
        EXPECT_TRUE(_is_can_tx_buffer_empty_called);
        
        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

/*******************************************************************************
 * AMR Alias Invalidation Tests
 ******************************************************************************/

/**
 * Test: state.invalid causes immediate discard (returns true)
 * Verifies:
 * - Message with state.invalid == true is discarded (returns true)
 * - No handler is called (message never transmitted)
 * - TX buffer availability is NOT checked (early exit before that)
 */
TEST(CanTxStatemachine, send_openlcb_message_invalid_discarded)
{
    _initialize();
    _reset_variables();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    openlcb_msg->state.invalid = true;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));

    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_can_frame_called);
    EXPECT_FALSE(_is_can_tx_buffer_empty_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);
}

/**
 * Test: state.invalid == false allows normal transmission
 * Verifies:
 * - Normal messages (invalid == false) transmit as before
 */
TEST(CanTxStatemachine, send_openlcb_message_not_invalid_transmits)
{
    _initialize();
    _reset_variables();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    openlcb_msg->state.invalid = false;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));

    EXPECT_TRUE(_handle_addressed_msg_frame_called);
    EXPECT_TRUE(_is_can_tx_buffer_empty_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);
}

/*******************************************************************************
 * Additional Coverage Tests
 ******************************************************************************/

/**
 * Test: Stream MTI variants
 * Verifies all stream-related MTIs route to stream handler
 *
 * Stream MTIs tested:
 * - MTI_STREAM_INIT_REQUEST - Request stream initialization
 * - MTI_STREAM_INIT_REPLY - Reply to stream init
 * - MTI_STREAM_COMPLETE - Stream complete notification
 */
TEST(CanTxStatemachine, stream_mti_variants)
{

    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(STREAM);
    ASSERT_NE(openlcb_msg, nullptr);

    // Test MTI_STREAM_INIT_REQUEST
    _reset_variables();
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_STREAM_INIT_REQUEST);
    openlcb_msg->payload_count = 8;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);

    // Test MTI_STREAM_INIT_REPLY
    _reset_variables();
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_STREAM_INIT_REPLY);
    openlcb_msg->payload_count = 8;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);

    // Test MTI_STREAM_COMPLETE
    _reset_variables();
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_STREAM_COMPLETE);
    openlcb_msg->payload_count = 8;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Empty payload messages
 * Verifies messages with no payload are transmitted correctly
 *
 * Tests the special case: if (openlcb_msg->payload_count == 0)
 * which sends a single zero-payload frame and returns immediately.
 */
TEST(CanTxStatemachine, empty_payload_messages)
{

    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    // Test global message with empty payload
    _reset_variables();
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0x000, 0x000000000000,
                                           MTI_INITIALIZATION_COMPLETE);
    openlcb_msg->payload_count = 0;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    // Test addressed message with empty payload
    _reset_variables();
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 0;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Addressed message transmission failure
 * Verifies failure is detected and propagated when first frame fails
 */
TEST(CanTxStatemachine, addressed_message_transmission_failure)
{

    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    _reset_variables();
    _fail_handle_addressed_frame = true;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

    EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Multi-frame addressed message
 * Verifies large addressed messages are segmented correctly
 *
 * Uses a DATAGRAM buffer to have enough room for 20 bytes of payload
 * with an addressed (non-datagram) MTI to exercise the multi-frame loop.
 */
TEST(CanTxStatemachine, multiframe_addressed_message)
{

    _initialize();

    // Use DATAGRAM buffer so we have room for 20 bytes (BASIC is only 16)
    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(openlcb_msg, nullptr);

    _reset_variables();

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_PROTOCOL_SUPPORT_REPLY);
    openlcb_msg->payload_count = 20;

    for (int i = 0; i < 20; i++)
    {

        *openlcb_msg->payload[i] = i;

    }

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Datagram transmission failure
 * Verifies failure is detected when datagram cannot be sent
 */
TEST(CanTxStatemachine, datagram_transmission_failure)
{

    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(openlcb_msg, nullptr);

    _reset_variables();
    _fail_handle_datagram_frame = true;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_DATAGRAM);
    openlcb_msg->payload_count = 10;

    EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Unaddressed message transmission failure
 * Verifies failure detection for global messages
 */
TEST(CanTxStatemachine, unaddressed_message_transmission_failure)
{

    _initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    _reset_variables();
    _fail_handle_unaddressed_frame = true;

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0x000, 0x000000000000,
                                           MTI_VERIFY_NODE_ID_GLOBAL);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

    EXPECT_FALSE(CanTxStatemachine_send_openlcb_message(openlcb_msg));
    EXPECT_TRUE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: CAN frame transmission failure
 * Verifies failure is detected when CAN hardware cannot send
 */
TEST(CanTxStatemachine, can_frame_transmission_failure)
{

    _reset_variables();
    _initialize();

    _fail_handle_can_frame = true;

    can_msg_t can_msg;
    CanUtilities_load_can_message(&can_msg, 0x170506BE, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    EXPECT_FALSE(CanTxStatemachine_send_can_message(&can_msg));
    EXPECT_TRUE(_handle_can_frame_called);

}

/**
 * Test: Maximum payload sizes
 * Verifies handling of maximum-size payloads without buffer overruns
 */
TEST(CanTxStatemachine, maximum_payload_sizes)
{

    _initialize();

    // Test maximum datagram (72 bytes)
    openlcb_msg_t *datagram_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    ASSERT_NE(datagram_msg, nullptr);

    _reset_variables();

    OpenLcbUtilities_load_openlcb_message(datagram_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_DATAGRAM);
    datagram_msg->payload_count = 72;

    for (int i = 0; i < 72; i++)
    {

        *datagram_msg->payload[i] = i & 0xFF;

    }

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(datagram_msg));
    EXPECT_TRUE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_addressed_msg_frame_called);

    OpenLcbBufferStore_free_buffer(datagram_msg);

    // Test maximum stream data
    openlcb_msg_t *stream_msg = OpenLcbBufferStore_allocate_buffer(STREAM);
    ASSERT_NE(stream_msg, nullptr);

    _reset_variables();

    OpenLcbUtilities_load_openlcb_message(stream_msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_STREAM_PROCEED);
    stream_msg->payload_count = LEN_MESSAGE_BYTES_STREAM;

    for (int i = 0; i < LEN_MESSAGE_BYTES_STREAM; i++)
    {

        *stream_msg->payload[i] = i & 0xFF;

    }

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(stream_msg));
    EXPECT_TRUE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);

    OpenLcbBufferStore_free_buffer(stream_msg);

}

/*******************************************************************************
 * Listener Alias Resolution Tests
 ******************************************************************************/

/**
 * Test: Listener resolves alias successfully
 * Verifies:
 * - dest_alias=0, dest_id=valid triggers listener lookup
 * - When listener returns entry with valid alias, dest_alias is updated
 * - Message is then transmitted normally through the addressed handler
 */
TEST(CanTxStatemachine, listener_resolves_alias)
{

    _initialize_with_listener();
    _reset_variables();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    // dest_alias=0, dest_id=valid triggers the listener resolution path
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0x000, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

    // Configure mock to return an entry with alias 0x123
    _listener_mock_entry.node_id = 0x060504030201;
    _listener_mock_entry.alias = 0x123;
    _listener_find_return_value = &_listener_mock_entry;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));

    // Verify the listener was called with the correct node ID
    EXPECT_TRUE(_listener_find_by_node_id_called);
    EXPECT_EQ(_listener_find_by_node_id_last_arg, (node_id_t) 0x060504030201);

    // Verify dest_alias was updated from the listener entry
    EXPECT_EQ(openlcb_msg->dest_alias, 0x123);

    // Verify the message was transmitted through the addressed handler
    EXPECT_TRUE(_handle_addressed_msg_frame_called);
    EXPECT_TRUE(_is_can_tx_buffer_empty_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Listener returns NULL entry (node ID not found)
 * Verifies:
 * - dest_alias=0, dest_id=valid triggers listener lookup
 * - When listener returns NULL, message is dropped (returns true)
 * - No handler is called (message never transmitted)
 */
TEST(CanTxStatemachine, listener_returns_null_drops_message)
{

    _initialize_with_listener();
    _reset_variables();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    // dest_alias=0, dest_id=valid triggers the listener resolution path
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0x000, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

    // Configure mock to return NULL (node not found in listener table)
    _listener_find_return_value = NULL;

    // Returns true (drop message, don't retry) per the source code comment
    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));

    // Verify the listener was called
    EXPECT_TRUE(_listener_find_by_node_id_called);
    EXPECT_EQ(_listener_find_by_node_id_last_arg, (node_id_t) 0x060504030201);

    // Verify no handler was called (message dropped before transmission)
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_can_frame_called);
    EXPECT_FALSE(_is_can_tx_buffer_empty_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}

/**
 * Test: Listener returns entry with alias=0 (not yet resolved)
 * Verifies:
 * - dest_alias=0, dest_id=valid triggers listener lookup
 * - When listener returns entry but alias=0, message is dropped (returns true)
 * - No handler is called (message never transmitted)
 */
TEST(CanTxStatemachine, listener_entry_alias_zero_drops_message)
{

    _initialize_with_listener();
    _reset_variables();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(openlcb_msg, nullptr);

    // dest_alias=0, dest_id=valid triggers the listener resolution path
    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506,
                                           0x000, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    openlcb_msg->payload_count = 6;
    OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

    // Configure mock to return an entry where alias has not been resolved yet
    _listener_mock_entry.node_id = 0x060504030201;
    _listener_mock_entry.alias = 0;
    _listener_find_return_value = &_listener_mock_entry;

    // Returns true (drop message, don't retry) per the source code comment
    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(openlcb_msg));

    // Verify the listener was called
    EXPECT_TRUE(_listener_find_by_node_id_called);
    EXPECT_EQ(_listener_find_by_node_id_last_arg, (node_id_t) 0x060504030201);

    // Verify no handler was called (message dropped before transmission)
    EXPECT_FALSE(_handle_addressed_msg_frame_called);
    EXPECT_FALSE(_handle_unaddressed_msg_frame_called);
    EXPECT_FALSE(_handle_datagram_frame_called);
    EXPECT_FALSE(_handle_stream_frame_called);
    EXPECT_FALSE(_handle_can_frame_called);
    EXPECT_FALSE(_is_can_tx_buffer_empty_called);

    OpenLcbBufferStore_free_buffer(openlcb_msg);

}
