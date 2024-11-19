
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

#define LEN_BASIC_BUFFER     2  // USER DEFINED
#define LEN_DATAGRAM_BUFFER  2  // USER DEFINED
#define LEN_SNIP_BUFFER      2  // USER DEFINED
#define LEN_STREAM_BUFFER    0  // USER DEFINED

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




#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_TYPES__ */

