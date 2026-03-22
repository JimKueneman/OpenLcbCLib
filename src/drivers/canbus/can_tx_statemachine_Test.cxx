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

#ifdef OPENLCB_COMPILE_TRAIN
#include "can_buffer_store.h"
#include "can_buffer_fifo.h"
#endif

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
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_can_frame = &_handle_can_frame
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
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_can_frame = &_handle_can_frame,
    .listener_find_by_node_id = &_mock_listener_find_by_node_id
};

#ifdef OPENLCB_COMPILE_TRAIN

/*******************************************************************************
 * Listener Config Reply Sniff Mocks
 ******************************************************************************/

// Track listener register/unregister mock calls
bool _listener_register_called = false;
node_id_t _listener_register_last_arg = 0;
bool _listener_unregister_called = false;
node_id_t _listener_unregister_last_arg = 0;

// Track lock/unlock calls
int _lock_call_count = 0;
int _unlock_call_count = 0;

// Static entry returned by register mock
listener_alias_entry_t _listener_register_mock_entry;

listener_alias_entry_t *_mock_listener_register(node_id_t node_id) {

    _listener_register_called = true;
    _listener_register_last_arg = node_id;
    _listener_register_mock_entry.node_id = node_id;
    _listener_register_mock_entry.alias = 0;

    return &_listener_register_mock_entry;

}

void _mock_listener_unregister(node_id_t node_id) {

    _listener_unregister_called = true;
    _listener_unregister_last_arg = node_id;

}

void _mock_lock_shared_resources(void) {

    _lock_call_count++;

}

void _mock_unlock_shared_resources(void) {

    _unlock_call_count++;

}

/*******************************************************************************
 * Interface With Listener Sniff Support
 ******************************************************************************/

const interface_can_tx_statemachine_t interface_can_tx_statemachine_with_sniff = {
    .is_tx_buffer_empty = &_is_can_tx_buffer_empty,
    .handle_addressed_msg_frame = &_handle_addressed_msg_frame,
    .handle_unaddressed_msg_frame = &_handle_unaddressed_msg_frame,
    .handle_datagram_frame = &_handle_datagram_frame,
    .handle_stream_frame = &_handle_stream_frame,
    .handle_can_frame = &_handle_can_frame,
    .listener_find_by_node_id = &_mock_listener_find_by_node_id,
    .listener_register = &_mock_listener_register,
    .listener_unregister = &_mock_listener_unregister,
    .lock_shared_resources = &_mock_lock_shared_resources,
    .unlock_shared_resources = &_mock_unlock_shared_resources
};

#endif

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

#ifdef OPENLCB_COMPILE_TRAIN
    _listener_register_called = false;
    _listener_register_last_arg = 0;
    _listener_unregister_called = false;
    _listener_unregister_last_arg = 0;
    _lock_call_count = 0;
    _unlock_call_count = 0;
#endif

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

#ifdef OPENLCB_COMPILE_TRAIN

/**
 * Initialize all subsystems with listener sniff support (register/unregister/AME)
 */
void _initialize_with_sniff(void)
{

    OpenLcbBufferStore_initialize();
    CanBufferStore_initialize();
    CanBufferFifo_initialize();
    CanTxStatemachine_initialize(&interface_can_tx_statemachine_with_sniff);

}

#endif

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

#ifdef OPENLCB_COMPILE_TRAIN

/*******************************************************************************
 * Listener Config Reply Sniff Tests
 ******************************************************************************/

/**
 * Helper: build a Listener Config Reply message (MTI_TRAIN_REPLY).
 *
 * Payload layout:
 *   Byte 0: TRAIN_LISTENER_CONFIG (0x30)
 *   Byte 1: sub_cmd (ATTACH=0x01 or DETACH=0x02)
 *   Bytes 2-7: listener node_id (big-endian)
 *   Byte 8: result (0x00 = success)
 */
static void _load_listener_config_reply(openlcb_msg_t *msg, uint16_t source_alias,
                                         node_id_t source_id, uint16_t dest_alias,
                                         node_id_t dest_id, uint8_t sub_cmd,
                                         node_id_t listener_id, uint8_t result) {

    OpenLcbUtilities_load_openlcb_message(msg, source_alias, source_id,
                                           dest_alias, dest_id,
                                           MTI_TRAIN_REPLY);
    msg->payload_count = 9;

    *msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *msg->payload[1] = sub_cmd;
    *msg->payload[2] = (uint8_t) (listener_id >> 40);
    *msg->payload[3] = (uint8_t) (listener_id >> 32);
    *msg->payload[4] = (uint8_t) (listener_id >> 24);
    *msg->payload[5] = (uint8_t) (listener_id >> 16);
    *msg->payload[6] = (uint8_t) (listener_id >> 8);
    *msg->payload[7] = (uint8_t) (listener_id);
    *msg->payload[8] = result;

}

/**
 * Test: Successful attach reply registers listener and sends AME
 */
TEST(CanTxStatemachine, attach_reply_success_registers_listener) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_ATTACH, 0xA1A2A3A4A5A6, 0x00);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    // Verify register was called with correct node_id
    EXPECT_TRUE(_listener_register_called);
    EXPECT_EQ(_listener_register_last_arg, (node_id_t) 0xA1A2A3A4A5A6);

    // Verify unregister was NOT called
    EXPECT_FALSE(_listener_unregister_called);

    // Clean up AME from FIFO
    can_msg_t *ame = CanBufferFifo_pop();
    if (ame) { CanBufferStore_free_buffer(ame); }

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Successful attach reply sends targeted AME
 */
TEST(CanTxStatemachine, attach_reply_success_sends_ame) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_ATTACH, 0xA1A2A3A4A5A6, 0x00);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    // Verify an AME was pushed to the FIFO
    can_msg_t *ame = CanBufferFifo_pop();
    ASSERT_NE(ame, nullptr);

    // AME identifier: RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME | source_alias
    EXPECT_EQ(ame->identifier, (uint32_t) (RESERVED_TOP_BIT | CAN_CONTROL_FRAME_AME | 0xAAA));
    EXPECT_EQ(ame->payload_count, 6);

    // Verify the node_id in the AME payload (big-endian, 6 bytes)
    EXPECT_EQ(ame->payload[0], 0xA1);
    EXPECT_EQ(ame->payload[1], 0xA2);
    EXPECT_EQ(ame->payload[2], 0xA3);
    EXPECT_EQ(ame->payload[3], 0xA4);
    EXPECT_EQ(ame->payload[4], 0xA5);
    EXPECT_EQ(ame->payload[5], 0xA6);

    CanBufferStore_free_buffer(ame);
    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Failed attach reply does not register or send AME
 */
TEST(CanTxStatemachine, attach_reply_fail_does_not_register) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_ATTACH, 0xA1A2A3A4A5A6, 0xFF);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    EXPECT_FALSE(_listener_register_called);
    EXPECT_FALSE(_listener_unregister_called);

    // Verify no AME was pushed
    can_msg_t *ame = CanBufferFifo_pop();
    EXPECT_EQ(ame, nullptr);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Successful detach reply unregisters listener
 */
TEST(CanTxStatemachine, detach_reply_success_unregisters_listener) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_DETACH, 0xA1A2A3A4A5A6, 0x00);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    // Verify unregister was called with correct node_id
    EXPECT_TRUE(_listener_unregister_called);
    EXPECT_EQ(_listener_unregister_last_arg, (node_id_t) 0xA1A2A3A4A5A6);

    // Verify register was NOT called
    EXPECT_FALSE(_listener_register_called);

    // Verify no AME was sent (detach does not need alias resolution)
    can_msg_t *ame = CanBufferFifo_pop();
    EXPECT_EQ(ame, nullptr);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Failed detach reply does not unregister
 */
TEST(CanTxStatemachine, detach_reply_fail_does_not_unregister) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_DETACH, 0xA1A2A3A4A5A6, 0xFF);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    EXPECT_FALSE(_listener_unregister_called);
    EXPECT_FALSE(_listener_register_called);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Non-listener Train Reply is ignored
 * (byte 0 != TRAIN_LISTENER_CONFIG)
 */
TEST(CanTxStatemachine, non_listener_reply_ignored) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Use a speed/direction reply (byte 0 = something other than 0x30)
    OpenLcbUtilities_load_openlcb_message(msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_TRAIN_REPLY);
    msg->payload_count = 9;
    *msg->payload[0] = 0x01;  // Not TRAIN_LISTENER_CONFIG
    for (int i = 1; i < 9; i++) {

        *msg->payload[i] = 0;

    }

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    EXPECT_FALSE(_listener_register_called);
    EXPECT_FALSE(_listener_unregister_called);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Non-train-reply MTI is ignored
 */
TEST(CanTxStatemachine, non_train_reply_ignored) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Use a different MTI
    OpenLcbUtilities_load_openlcb_message(msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_VERIFY_NODE_ID_ADDRESSED);
    msg->payload_count = 9;
    *msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *msg->payload[1] = TRAIN_LISTENER_ATTACH;
    *msg->payload[8] = 0x00;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    EXPECT_FALSE(_listener_register_called);
    EXPECT_FALSE(_listener_unregister_called);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: NULL listener_register pointer is safe (no crash)
 */
TEST(CanTxStatemachine, null_listener_register_safe) {

    // Use the interface WITHOUT sniff pointers (listener_register == NULL)
    _initialize_with_listener();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_ATTACH, 0xA1A2A3A4A5A6, 0x00);

    // Should not crash — listener_register is NULL, sniff exits early
    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Short payload (< 9 bytes) is ignored
 */
TEST(CanTxStatemachine, short_payload_ignored) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    OpenLcbUtilities_load_openlcb_message(msg, 0xAAA, 0x010203040506,
                                           0xBBB, 0x060504030201,
                                           MTI_TRAIN_REPLY);
    msg->payload_count = 8;  // One byte short
    *msg->payload[0] = TRAIN_LISTENER_CONFIG;
    *msg->payload[1] = TRAIN_LISTENER_ATTACH;

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    EXPECT_FALSE(_listener_register_called);
    EXPECT_FALSE(_listener_unregister_called);

    OpenLcbBufferStore_free_buffer(msg);

}

/**
 * Test: Lock/unlock are called around AME buffer operations
 */
TEST(CanTxStatemachine, lock_unlock_called_around_ame) {

    _initialize_with_sniff();
    _reset_variables();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    _load_listener_config_reply(msg, 0xAAA, 0x010203040506,
                                 0xBBB, 0x060504030201,
                                 TRAIN_LISTENER_ATTACH, 0xA1A2A3A4A5A6, 0x00);

    EXPECT_TRUE(CanTxStatemachine_send_openlcb_message(msg));

    // Lock/unlock called twice: once for allocate, once for push
    EXPECT_EQ(_lock_call_count, 2);
    EXPECT_EQ(_unlock_call_count, 2);

    // Clean up the AME from the FIFO
    can_msg_t *ame = CanBufferFifo_pop();
    if (ame) { CanBufferStore_free_buffer(ame); }

    OpenLcbBufferStore_free_buffer(msg);

}

#endif
