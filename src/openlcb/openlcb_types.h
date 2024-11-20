
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_TYPES__
#define	__OPENLCB_TYPES__

#include <xc.h> // include processor files - each processor file is guarded. 

// ************************ USER DEFINED VARIABLES *****************************

// Total number of message buffers available for use

#define LEN_BASIC_BUFFER     10  // USER DEFINED
#define LEN_DATAGRAM_BUFFER  10  // USER DEFINED
#define LEN_SNIP_BUFFER      10  // USER DEFINED
#define LEN_STREAM_BUFFER    0  // USER DEFINED

#define USER_DEFINED_PRODUCER_COUNT     8   // The maximum number of events supported is 32
#define USER_DEFINED_CONSUMER_COUNT     8   // The maximum number of events supported is 32

// *********************END USER DEFINED VARIABLES *****************************


#define FALSE 0;
#define TRUE  1;


#define LEN_MESSAGE_BYTES_BASIC        16     // most are 8 bytes but a few protocols take 2 frames like Traction
#define LEN_MESSAGE_BYTES_DATAGRAM     72
#define LEN_MESSAGE_BYTES_SNIP        256
#define LEN_MESSAGE_BYTES_STREAM     1024

#define LEN_EVENT_ID                    8

#define LEN_MESSAGE_BUFFER  LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER + LEN_SNIP_BUFFER + LEN_STREAM_BUFFER

typedef uint8_t payload_basic_t[LEN_MESSAGE_BYTES_BASIC];
typedef uint8_t payload_datagram_t[LEN_MESSAGE_BYTES_DATAGRAM];
typedef uint8_t payload_snip_t[LEN_MESSAGE_BYTES_SNIP];
typedef uint8_t payload_stream_t[LEN_MESSAGE_BYTES_STREAM];

typedef payload_basic_t openlcb_basic_data_buffer_t[LEN_BASIC_BUFFER];
typedef payload_datagram_t openlcb_datagram_data_buffer_t[LEN_DATAGRAM_BUFFER];
typedef payload_snip_t openlcb_snip_data_buffer_t[LEN_SNIP_BUFFER];
typedef payload_stream_t openlcb_stream_data_buffer_t[LEN_STREAM_BUFFER];


typedef uint8_t openlcb_payload_t[1];

typedef uint64_t event_id_t;
typedef uint64_t node_id_t;

typedef struct {
    uint8_t allocated:1;               // message has been allocated and is in use
    uint8_t inprocess:1;               // message is being collected from multiple CAN frames
} openlcb_msg_state;

typedef struct {
    openlcb_msg_state state;
    uint16_t mti;
    uint16_t source_alias;
    uint16_t dest_alias;
    node_id_t source_id;
    node_id_t dest_id;
    uint16_t payload_size;     // How many bytes the payload can hold
    uint16_t payload_count;    // valid bytes in payload
    openlcb_payload_t* payload;          // size depend of the buffer type and defined as payload_size
    uint8_t timerticks;        // timeouts, etc
    uint8_t retry_count; 
} openlcb_msg_t;

typedef openlcb_msg_t openlcb_msg_array_t[LEN_MESSAGE_BUFFER];

typedef struct {
   openlcb_msg_array_t messages;   // array of openlcb message structures
   openlcb_basic_data_buffer_t basic;          // array of basic arrays
   openlcb_datagram_data_buffer_t datagram;    // array of datagram arrays
   openlcb_snip_data_buffer_t snip;            // array of snip arrays
   openlcb_stream_data_buffer_t stream;        // array of stream arrays
} message_buffer_t;



// Event ID Structures

typedef struct {
    uint16_t running : 1; // Alway, always, always reset these to FALSE when you have finished processing a
    uint16_t flag : 1; // message so the next message starts in a known state

} event_id_enum_t;

typedef struct {
    event_id_t list[USER_DEFINED_CONSUMER_COUNT];
    event_id_enum_t enumerator;
    uint64_t event_state; // 2 bits for each event limiting it to 32 events

} event_id_consumer_list_t;

typedef struct {
    event_id_t list[USER_DEFINED_PRODUCER_COUNT];
    event_id_enum_t enumerator;
    uint64_t event_state; // 2 bits for each event limiting it to 32 events

} event_id_producer_list_t;

typedef struct {
    uint16_t run : 6; // Run state... limits the number to how many bits here.... 64 possible states.
    uint16_t allocated : 1; // Allocated to be used
    uint16_t permitted : 1; // Has the CAN alias been allocated and the network notified
    uint16_t initalized : 1; // Has the node been logged into the the network
    uint16_t duplicate_id_detected : 1; // Node has detected a duplicated Node ID and has sent the PCER
} nodestateBITS;

typedef struct {
    nodestateBITS state;
    uint64_t id;
    uint16_t alias;
    uint64_t seed; // Seed for generating the alias 
    event_id_consumer_list_t consumers;
    event_id_producer_list_t producers;
    uint16_t timerticks; // Counts the 100ms timer ticks during the CAN alias allocation
    openlcb_msg_t* working_msg; // When a handler is called the statemachine loads this with the message being dispatched.  
    // The handler MUST clear this so the loop knows the message is fully handled and can move on to the next incoming message.

    uint16_t working_msg_index;
} openlcb_node_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_TYPES__ */

