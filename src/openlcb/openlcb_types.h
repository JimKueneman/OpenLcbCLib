
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

#define LEN_BASIC_BUFFER     16  // USER DEFINED this should be at least LEN_NODE_ARRAY + ~10
#define LEN_DATAGRAM_BUFFER  16  // USER DEFINED this should be at least LEN_NODE_ARRAY + ~10
#define LEN_SNIP_BUFFER      20  // USER DEFINED
#define LEN_STREAM_BUFFER    0  // USER DEFINED


#define LEN_NODE_ARRAY                16  // USER DEFINED 

#define LEN_MAX_CDI                   2000
#define LEN_MAX_FDI                   1000

#define LEN_PRODUCER_MAX_COUNT         8
#define LEN_CONSUMER_MAX_COUNT         8

#define CONFIG_MEM_READ_WRITE_DESCRIPTION_LEN       63-1   // space for null
#define CONFIG_MEM_OPTIONS_DESCRIPTION_LEN          64-1   // space for null
#define CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION_LEN    60-1   // space for null; If the low address is used then we only will have 72-12 = 60 bytes (including the null)

// *********************END USER DEFINED VARIABLES *****************************

#define FALSE 0;
#define TRUE  1;

// Per the SNIP specification
#define LEN_SNIP_NAME              41
#define LEN_SNIP_MODEL             41
#define LEN_SNIP_HARDWARE_VERSION  21
#define LEN_SNIP_SOFTWARE_VERSION  21

#define LEN_SNIP_USER_NAME         63
#define LEN_SNIP_USER_DESCRIPTION  64

#define LEN_SNIP_USER_DATA         (LEN_SNIP_USER_NAME + LEN_SNIP_USER_DESCRIPTION)

#define LEN_SNIP_VERSION            1
#define LEN_SNIP_USER_VERSION       1

#define LEN_SNIP_STRUCTURE        253


#define LEN_MESSAGE_BYTES_BASIC        16     // most are 8 bytes but a few protocols take 2 frames like Traction
#define LEN_MESSAGE_BYTES_DATAGRAM     72
#define LEN_MESSAGE_BYTES_SNIP        256     // will cover Event with Payload as well
#define LEN_MESSAGE_BYTES_STREAM      512

#define LEN_EVENT_ID                    8

#define LEN_MESSAGE_BUFFER  (LEN_BASIC_BUFFER + LEN_DATAGRAM_BUFFER + LEN_SNIP_BUFFER + LEN_STREAM_BUFFER)

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
    uint8_t allocated : 1; // message has been allocated and is in use
    uint8_t inprocess : 1; // message is being collected from multiple CAN frames and not complete yet
} openlcb_msg_state;

typedef struct {
    openlcb_msg_state state;
    uint16_t mti;
    uint16_t source_alias;
    uint16_t dest_alias;
    node_id_t source_id;
    node_id_t dest_id;
    uint16_t payload_size; // How many bytes the payload can hold
    uint16_t payload_count; // valid bytes in payload
    openlcb_payload_t* payload; // size depend of the buffer type and defined as payload_size
    uint8_t timerticks; // timeouts, etc
    uint8_t retry_count;
    uint8_t reference_count; // reference counted for garbage collection
} openlcb_msg_t;

typedef openlcb_msg_t openlcb_msg_array_t[LEN_MESSAGE_BUFFER];

typedef struct {
    openlcb_msg_array_t messages; // array of openlcb message structures
    openlcb_basic_data_buffer_t basic; // array of basic arrays
    openlcb_datagram_data_buffer_t datagram; // array of datagram arrays
    openlcb_snip_data_buffer_t snip; // array of snip arrays
    openlcb_stream_data_buffer_t stream; // array of stream arrays
} message_buffer_t;


// Defines a node for snip

typedef struct {
    uint8_t mfg_version;
    char name[LEN_SNIP_NAME + 1];
    char model[LEN_SNIP_MODEL + 1];
    char hardware_version[LEN_SNIP_HARDWARE_VERSION + 1];
    char software_version[LEN_SNIP_SOFTWARE_VERSION + 1];
    uint8_t user_version;

} user_snip_struct_t;

typedef struct {
    uint8_t write_under_mask_supported : 1;
    uint8_t unaligned_reads_supported : 1;
    uint8_t unaligned_writes_supported : 1;
    uint8_t read_from_manufacturer_space_0xfc_supported : 1;
    uint8_t read_from_user_space_0xfb_supported : 1;
    uint8_t write_to_user_space_0xfb_supported : 1;
    uint8_t stream_read_write_supported : 1;
    uint8_t high_address_space;
    uint8_t low_address_space;
    char description[CONFIG_MEM_OPTIONS_DESCRIPTION_LEN];
} user_configuration_options;

typedef struct {
    uint8_t present : 1;
    uint8_t read_only : 1;
    uint8_t low_address_valid : 1;
    uint8_t address_space;
    uint32_t highest_address;
    uint32_t low_address;
    char description[CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION_LEN]; 
} user_address_space_info_t;

typedef struct {
    user_snip_struct_t snip;
    uint64_t protocol_support;
    uint16_t consumer_count;
    uint16_t producer_count;
    uint8_t cdi[LEN_MAX_CDI];
    uint8_t fdi[LEN_MAX_FDI];
    user_address_space_info_t address_space_configuration_definition;
    user_address_space_info_t address_space_all;
    user_address_space_info_t address_space_config_memory;
    user_address_space_info_t address_space_acdi_manufacturer;
    user_address_space_info_t address_space_acdi_user;
    user_address_space_info_t address_space_train_function_definition;
    user_address_space_info_t address_space_train_function_config_memory;
    user_configuration_options configuration_options;

} node_parameters_t;


// Event ID Structures

typedef struct {
    uint16_t running : 1; // Alway, always, always reset these to FALSE when you have finished processing a
    uint8_t enum_index; // allows a counter for enumerating the event ids
} event_id_enum_t;

typedef struct {
    event_id_t list[LEN_CONSUMER_MAX_COUNT];
    event_id_enum_t enumerator;
    uint64_t event_state; // 2 bits for each event limiting it to 32 events

} event_id_consumer_list_t;

typedef struct {
    event_id_t list[LEN_PRODUCER_MAX_COUNT];
    event_id_enum_t enumerator;
    uint64_t event_state; // 2 bits for each event limiting it to 32 events

} event_id_producer_list_t;

typedef struct {
    uint16_t run_state : 6; // Run state... limits the number to how many bits here.... 64 possible states.
    uint16_t allocated : 1; // Allocated to be used
    uint16_t permitted : 1; // Has the CAN alias been allocated and the network notified
    uint16_t initalized : 1; // Has the node been logged into the the network
    uint16_t duplicate_id_detected : 1; // Node has detected a duplicated Node ID and has sent the PCER
    uint16_t can_msg_handled : 1; // allows message loops to know if this node has handled the can message that is currently being process so it knows when to move on to the next
    uint16_t openlcb_msg_handled : 1; // allows message loops to know if this node has handled the openlcb message that is currently being process so it knows when to move on to the next
    uint16_t openlcb_datagram_ack_sent : 1;
    uint16_t resend_datagram: 1; // if set the message loop will bypass pulling the next message from the fifo and send the message in sent_datagrams first
    uint16_t resend_optional_message: 1; // if set the message loop will bypass pulling the next message from the fifo and send the message in sent_datagrams first
} openlcb_node_state_t;

typedef struct {
    openlcb_node_state_t state;
    uint64_t id;
    uint16_t alias;
    uint64_t seed; // Seed for generating the alias 
    event_id_consumer_list_t consumers;
    event_id_producer_list_t producers;
    const node_parameters_t* parameters;
    uint16_t timerticks; // Counts the 100ms timer ticks during the CAN alias allocation
    uint64_t lock_node;  // node that has this node locked
    openlcb_msg_t* last_received_datagram;
    openlcb_msg_t* last_received_optional_interaction;
    uint8_t index; //what index in the node list this node is, used to help with offsets for config memory, fdi memory, etc.
} openlcb_node_t;

typedef struct {
    openlcb_node_t node[LEN_NODE_ARRAY];
    uint16_t count; // How many have been allocated, you can not deallocate a node so one it is allocated it is there to the end (it can be not permitted)
    //  openlcb_msg_t* working_msg; // When a OpenLcb message is sent on CAN it may need to be taken apart and sent in various frames.  Once popped it is stored here as the current working message that is being sent out

} openlcb_nodes_t;

typedef struct {
    openlcb_msg_t worker;
    payload_stream_t worker_buffer;
    openlcb_msg_t* active_msg;
} openlcb_statemachine_worker_t;



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_TYPES__ */

