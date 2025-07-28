/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file openlcb_types.h
 *
 * Typedefs and various constants for the library.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_TYPES__
#define	__OPENLCB_TYPES__

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // ************************ USER DEFINED VARIABLES *****************************

    // Total number of message buffers available for use
    // Note you can override these with Define Macros in your compiler 

#ifndef USER_DEFINED_BASIC_BUFFER_DEPTH
#define USER_DEFINED_BASIC_BUFFER_DEPTH     32  // USER DEFINED MAX LIMIT = 0xFE = 254
#endif

#ifndef USER_DEFINED_DATAGRAM_BUFFER_DEPTH
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH  4  // USER DEFINED MAX LIMIT = 0xFE = 254
#endif

#ifndef USER_DEFINED_SNIP_BUFFER_DEPTH
#define USER_DEFINED_SNIP_BUFFER_DEPTH      4  // USER DEFINED MAX LIMIT = 0xFE = 254
#endif

#ifndef USER_DEFINED_STREAM_BUFFER_DEPTH
#define USER_DEFINED_STREAM_BUFFER_DEPTH    0  // USER DEFINED MAX LIMIT = 0xFE = 254
#endif

#ifndef USER_DEFINED_NODE_BUFFER_DEPTH
#define USER_DEFINED_NODE_BUFFER_DEPTH      1  // USER DEFINED   MAX LIMIT = 0xFE = 254
#endif

#ifndef USER_DEFINED_CDI_LENGTH
#define USER_DEFINED_CDI_LENGTH            20000 // USER DEFINED 
#endif

#ifndef USER_DEFINED_FDI_LENGTH
#define USER_DEFINED_FDI_LENGTH             1000 // USER DEFINED 
#endif

#ifndef SUPPORT_FIRMWARE_BOOTLOADER
    
  #ifndef USER_DEFINED_PRODUCER_COUNT
    #define USER_DEFINED_PRODUCER_COUNT         64 // USER DEFINED 
  #endif

  #ifndef USER_DEFINED_CONSUMER_COUNT
    #define USER_DEFINED_CONSUMER_COUNT         32 // USER DEFINED 
  #endif

#else
    
  #define USER_DEFINED_PRODUCER_COUNT 0
  #define USER_DEFINED_CONSUMER_COUNT 0

#endif

#define LEN_CONFIG_MEM_OPTIONS_DESCRIPTION          64-1   // space for null Size is limited by required return values - the max size of a datagram (72)
#define LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION    60-1   // space for null; If the low address is used then we only will have 72-12 = 60 bytes (including the null)

    // *********************END USER DEFINED VARIABLES *****************************

    typedef uint8_t uint8_olcb_t;
    typedef uint64_t uint64_olcb_t;
    typedef uint16_t uint16_olcb_t;
    typedef uint32_t uint32_olcb_t;


#define FALSE 0
#define TRUE  1

#define LOW 0
#define HIGH 1

#define NULL_NODE_ID 0x000000000000
#define NULL_EVENT_ID 0x0000000000000000

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

#define LEN_MESSAGE_BUFFER  (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH)

#define LEN_DATAGRAM_MAX_PAYLOAD       64   // After subtracting the overhead of a datagram message the remaining bytes available to carry the payload

    typedef uint8_olcb_t payload_basic_t[LEN_MESSAGE_BYTES_BASIC];
    typedef uint8_olcb_t payload_datagram_t[LEN_MESSAGE_BYTES_DATAGRAM];
    typedef uint8_olcb_t payload_snip_t[LEN_MESSAGE_BYTES_SNIP];
    typedef uint8_olcb_t payload_stream_t[LEN_MESSAGE_BYTES_STREAM];

    typedef payload_basic_t openlcb_basic_data_buffer_t[USER_DEFINED_BASIC_BUFFER_DEPTH];
    typedef payload_datagram_t openlcb_datagram_data_buffer_t[USER_DEFINED_DATAGRAM_BUFFER_DEPTH];
    typedef payload_snip_t openlcb_snip_data_buffer_t[USER_DEFINED_SNIP_BUFFER_DEPTH];
    typedef payload_stream_t openlcb_stream_data_buffer_t[USER_DEFINED_STREAM_BUFFER_DEPTH];


    typedef uint8_olcb_t openlcb_payload_t[1];

    typedef uint64_olcb_t event_id_t;
    typedef uint64_olcb_t node_id_t;

    typedef uint8_olcb_t event_payload_t[255];

    typedef struct {
        uint8_olcb_t allocated : 1; // message has been allocated and is in use
        uint8_olcb_t inprocess : 1; // message is being collected from multiple CAN frames and not complete yet
    } openlcb_msg_state_t;

    typedef struct {
        openlcb_msg_state_t state;
        uint16_olcb_t mti;
        uint16_olcb_t source_alias;
        uint16_olcb_t dest_alias;
        node_id_t source_id;
        node_id_t dest_id;
        uint16_olcb_t payload_size; // How many bytes the payload can hold
        uint16_olcb_t payload_count; // valid bytes in payload
        openlcb_payload_t* payload; // pointer to one of the data structures in the message_buffer_t type.  Size depend of the buffer type and defined as payload_size
        uint8_olcb_t timerticks; // timeouts, etc
        uint8_olcb_t reference_count; // reference counted for garbage collection
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
        uint8_olcb_t mfg_version;
        char name[LEN_SNIP_NAME];
        char model[LEN_SNIP_MODEL];
        char hardware_version[LEN_SNIP_HARDWARE_VERSION];
        char software_version[LEN_SNIP_SOFTWARE_VERSION];
        uint8_olcb_t user_version;

    } user_snip_struct_t;

    typedef struct {
        uint8_olcb_t write_under_mask_supported : 1;
        uint8_olcb_t unaligned_reads_supported : 1;
        uint8_olcb_t unaligned_writes_supported : 1;
        uint8_olcb_t read_from_manufacturer_space_0xfc_supported : 1;
        uint8_olcb_t read_from_user_space_0xfb_supported : 1;
        uint8_olcb_t write_to_user_space_0xfb_supported : 1;
        uint8_olcb_t stream_read_write_supported : 1;
        uint8_olcb_t high_address_space;
        uint8_olcb_t low_address_space;
        char description[LEN_CONFIG_MEM_OPTIONS_DESCRIPTION];
    } user_configuration_options;

    typedef struct {
        uint8_olcb_t present : 1;
        uint8_olcb_t read_only : 1;
        uint8_olcb_t low_address_valid : 1;
        uint8_olcb_t address_space;
        uint32_olcb_t highest_address;
        uint32_olcb_t low_address;
        char description[LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION];
    } user_address_space_info_t;

    typedef struct {
        user_snip_struct_t snip;
        uint64_olcb_t protocol_support;
        uint8_olcb_t consumer_count_autocreate;
        uint8_olcb_t producer_count_autocreate;
        uint8_olcb_t cdi[USER_DEFINED_CDI_LENGTH];
#ifdef SUPPORT_TRACTION
        uint8_olcb_t fdi[USER_DEFINED_FDI_LENGTH];
#endif
        user_address_space_info_t address_space_configuration_definition;
        user_address_space_info_t address_space_all;
        user_address_space_info_t address_space_config_memory;
        user_address_space_info_t address_space_acdi_manufacturer;
        user_address_space_info_t address_space_acdi_user;
#ifdef SUPPORT_TRACTION
        user_address_space_info_t address_space_train_function_definition;
        user_address_space_info_t address_space_train_function_config_memory;
#endif
        user_configuration_options configuration_options;
        user_address_space_info_t address_space_firmware;

    } node_parameters_t;


    // Event ID Structures

#define EVENTS_ENCODED_IN_BYTE 4

    typedef struct {
        uint8_olcb_t running : 1; // Alway, always, always reset these to FALSE when you have finished processing a
        uint8_olcb_t enum_index; // allows a counter for enumerating the event ids
    } event_id_enum_t;

    typedef struct {
        uint16_olcb_t count;
        event_id_t list[USER_DEFINED_CONSUMER_COUNT];
        event_id_enum_t enumerator;
        uint8_olcb_t event_state_array[USER_DEFINED_CONSUMER_COUNT / EVENTS_ENCODED_IN_BYTE + 1]; // Can get 4 Event State encoded in a single byte but if it is an odd number may need and extra byte

    } event_id_consumer_list_t;

    typedef struct {
        uint16_olcb_t count;
        event_id_t list[USER_DEFINED_PRODUCER_COUNT];
        event_id_enum_t enumerator;
        uint8_olcb_t event_state_array[USER_DEFINED_PRODUCER_COUNT / EVENTS_ENCODED_IN_BYTE + 1]; // Can get 4 Event State encoded in a single byte but if it is an odd number may need and extra byte

    } event_id_producer_list_t;

    typedef struct {
        uint8_olcb_t run_state : 5; // Run state... limits the number to how many bits here.... 32 possible states.
        uint8_olcb_t allocated : 1; // Allocated to be used
        uint8_olcb_t permitted : 1; // Has the CAN alias been allocated and the network notified
        uint8_olcb_t initalized : 1; // Has the node been logged into the the network
        uint8_olcb_t initial_events_broadcast_complete : 1; // has the node finished its initial broadcast of events?
        uint8_olcb_t duplicate_id_detected : 1; // Node has detected a duplicated Node ID and has sent the PCER
        uint8_olcb_t can_msg_handled : 1; // allows message loops to know if this node has handled the can message that is currently being process so it knows when to move on to the next
        uint8_olcb_t openlcb_msg_handled : 1; // allows message loops to know if this node has handled the openlcb message that is currently being process so it knows when to move on to the next
        uint8_olcb_t openlcb_datagram_ack_sent : 1;
        uint8_olcb_t resend_datagram : 1; // if set the message loop will bypass pulling the next message from the fifo and send the message in sent_datagrams first
        uint8_olcb_t resend_optional_message : 1; // if set the message loop will bypass pulling the next message from the fifo and send the message in sent_datagrams first
        uint8_olcb_t firmware_upgrade_active : 1; // Set if the node is in firmware upgrade mode
    } openlcb_node_state_t;

    typedef struct {
        openlcb_node_state_t state;
        uint64_olcb_t id;
        uint16_olcb_t alias;
        uint64_olcb_t seed; // Seed for generating the alias 
        event_id_consumer_list_t consumers;
        event_id_producer_list_t producers;
        const node_parameters_t* parameters;
        uint16_olcb_t timerticks; // Counts the 100ms timer ticks during the CAN alias allocation
        uint64_olcb_t lock_node; // node that has this node locked
        openlcb_msg_t* last_received_datagram;
        openlcb_msg_t* last_received_optional_interaction;
        uint8_olcb_t index; //what index in the node list this node is, used to help with offsets for config memory, fdi memory, etc.
    } openlcb_node_t;

    typedef struct {
        openlcb_node_t node[USER_DEFINED_NODE_BUFFER_DEPTH];
        uint16_olcb_t count; // How many have been allocated, you can not deallocate a node so one it is allocated it is there to the end (it can be not permitted)
        //  openlcb_msg_t* working_msg; // When a OpenLcb message is sent on CAN it may need to be taken apart and sent in various frames.  Once popped it is stored here as the current working message that is being sent out

    } openlcb_nodes_t;

typedef struct {
        openlcb_msg_t worker;
        payload_stream_t worker_buffer;
        openlcb_msg_t* active_msg;
    } openlcb_statemachine_worker_t;


    typedef void(*parameterless_callback_t) (void);

    typedef void(*mcu_driver_callback_t) (parameterless_callback_t);

    typedef uint8_olcb_t configuration_memory_buffer_t[LEN_DATAGRAM_MAX_PAYLOAD];


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_TYPES__ */

