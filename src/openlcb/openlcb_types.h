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
 * @file openlcb_types.h
 * @brief Type definitions and constants for the OpenLcb library
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_TYPES__
#define __OPENLCB_OPENLCB_TYPES__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // ************************ USER DEFINED VARIABLES *****************************

    // Total number of message buffers available for use
    // Note you can override these with Define Macros in your compiler

    // The SUM of the previous 4 buffer defines must be no greater than 126 for an 8 bit processor (8 bit signed integer)
    //  Should never really need more than 10-20

#ifndef USER_DEFINED_BASIC_BUFFER_DEPTH
#define USER_DEFINED_BASIC_BUFFER_DEPTH 32 // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#endif

#ifndef USER_DEFINED_DATAGRAM_BUFFER_DEPTH
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH 4 // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#endif

#ifndef USER_DEFINED_SNIP_BUFFER_DEPTH
#define USER_DEFINED_SNIP_BUFFER_DEPTH 4 // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#endif

#ifndef USER_DEFINED_STREAM_BUFFER_DEPTH
#define USER_DEFINED_STREAM_BUFFER_DEPTH 1 // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#endif

    // The SUM of the previous 4 buffer defines must be no greater than 126 for an 8 bit processor (8 bit signed integer)

#ifndef USER_DEFINED_NODE_BUFFER_DEPTH
#define USER_DEFINED_NODE_BUFFER_DEPTH 4 // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#endif

#ifndef USER_DEFINED_CDI_LENGTH
#define USER_DEFINED_CDI_LENGTH 20000 // USER DEFINED
#endif

#ifndef USER_DEFINED_FDI_LENGTH
#define USER_DEFINED_FDI_LENGTH 1000 // USER DEFINED
#endif

#ifndef USER_DEFINED_PRODUCER_COUNT    // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#define USER_DEFINED_PRODUCER_COUNT 64 // USER DEFINED Max of 126 for an 8 bit processor (8 bit signed integer)
#endif

#ifndef USER_DEFINED_CONSUMER_COUNT    // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#define USER_DEFINED_CONSUMER_COUNT 32 // USER DEFINED Max of 126 for an 8 bit processor (8 bit signed integer)
#endif

#ifndef USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS    // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#define USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS 0x00000000 // USER DEFINED Address into the Configuration Memory to save the User Defined Name string
#endif

#ifndef USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS    // USER DEFINED through overriding in the compiler macros, leave these alone so the Google Tests work
#define USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS LEN_SNIP_USER_NAME_BUFFER // USER DEFINED Address into the Configuration Memory to save the User Defined Name string
#endif

#define LEN_CONFIG_MEM_OPTIONS_DESCRIPTION 64 - 1       // space for null Size is limited by required return values - the max size of a datagram (72)
#define LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION 60 - 1 // space for null; If the low address is used then we only will have 72-12 = 60 bytes (including the null)

    // *********************END USER DEFINED VARIABLES *****************************

#define NULL_NODE_ID 0x000000000000
#define NULL_EVENT_ID 0x0000000000000000

    // Per the SNIP specification, this is the number of bytes allocated _including_ the null so the string is limited to one less
#define LEN_SNIP_NAME_BUFFER 41
#define LEN_SNIP_MODEL_BUFFER 41
#define LEN_SNIP_HARDWARE_VERSION_BUFFER 21
#define LEN_SNIP_SOFTWARE_VERSION_BUFFER 21

#define LEN_SNIP_USER_NAME_BUFFER 63
#define LEN_SNIP_USER_DESCRIPTION_BUFFER 64

#define LEN_SNIP_USER_DATA (LEN_SNIP_USER_NAME_BUFFER + LEN_SNIP_USER_DESCRIPTION_BUFFER)

#define LEN_SNIP_VERSION 1
#define LEN_SNIP_USER_VERSION 1

    // Event with Payload = 256 Payload + 8 bytes for Event ID = 264; SNIP max = 253
#define LEN_SNIP_STRUCTURE 264

#define LEN_MESSAGE_BYTES_BASIC 16 // most are 8 bytes but a few protocols take 2 frames like Traction
#define LEN_MESSAGE_BYTES_DATAGRAM 72
#define LEN_MESSAGE_BYTES_SNIP 256 // will cover Events with Payload as well
#define LEN_MESSAGE_BYTES_STREAM 512

#define LEN_EVENT_ID 8

#define LEN_MESSAGE_BUFFER (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH)

#define LEN_DATAGRAM_MAX_PAYLOAD 64 // After subtracting the overhead of a datagram message the remaining bytes available to carry the payload

#define LEN_EVENT_PAYLOAD LEN_MESSAGE_BYTES_SNIP

    typedef enum {
        BASIC,
        DATAGRAM,
        SNIP,
        STREAM

    } payload_type_enum;

    typedef enum {
        EVENT_STATUS_UNKNOWN,
        EVENT_STATUS_SET,
        EVENT_STATUS_CLEAR

    } event_status_enum;

    typedef enum {
        ADDRESS_SPACE_IN_BYTE_1 = 0,
        ADDRESS_SPACE_IN_BYTE_6 = 1

    } space_encoding_enum;

    typedef uint8_t payload_basic_t[LEN_MESSAGE_BYTES_BASIC];
    typedef uint8_t payload_datagram_t[LEN_MESSAGE_BYTES_DATAGRAM];
    typedef uint8_t payload_snip_t[LEN_MESSAGE_BYTES_SNIP];
    typedef uint8_t payload_stream_t[LEN_MESSAGE_BYTES_STREAM];

    typedef payload_basic_t openlcb_basic_data_buffer_t[USER_DEFINED_BASIC_BUFFER_DEPTH];
    typedef payload_datagram_t openlcb_datagram_data_buffer_t[USER_DEFINED_DATAGRAM_BUFFER_DEPTH];
    typedef payload_snip_t openlcb_snip_data_buffer_t[USER_DEFINED_SNIP_BUFFER_DEPTH];
    typedef payload_stream_t openlcb_stream_data_buffer_t[USER_DEFINED_STREAM_BUFFER_DEPTH];

    typedef uint8_t openlcb_payload_t[1];

    typedef uint64_t event_id_t;

    typedef struct {
        event_id_t event;
        event_status_enum status;

    } event_id_struct_t;

    typedef uint64_t node_id_t;

    typedef uint8_t event_payload_t[LEN_EVENT_PAYLOAD];

    typedef uint8_t configuration_memory_buffer_t[LEN_DATAGRAM_MAX_PAYLOAD];

    typedef struct {
        bool allocated : 1; // message has been allocated and is in use
        bool inprocess : 1; // message is being collected from multiple CAN frames and not complete yet
    } openlcb_msg_state_t;

    typedef struct {
        openlcb_msg_state_t state;
        uint16_t mti;
        uint16_t source_alias;
        uint16_t dest_alias;
        node_id_t source_id;
        node_id_t dest_id;
        payload_type_enum payload_type; // How many bytes the payload can hold
        uint16_t payload_count; // valid bytes in payload
        openlcb_payload_t *payload; // pointer to one of the data structures in the message_buffer_t type.  Size depend of the buffer type and defined as payload_size
        uint8_t timerticks; // timeouts, etc
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
        char name[LEN_SNIP_NAME_BUFFER]; // really wanted checking here but it seem C compiler initializer work a bit differently... some include the null some don't so we can't make this fool proof
        char model[LEN_SNIP_MODEL_BUFFER];
        char hardware_version[LEN_SNIP_HARDWARE_VERSION_BUFFER];
        char software_version[LEN_SNIP_SOFTWARE_VERSION_BUFFER];
        uint8_t user_version;

    } user_snip_struct_t;

    typedef struct {
        bool write_under_mask_supported : 1;
        bool unaligned_reads_supported : 1;
        bool unaligned_writes_supported : 1;
        bool read_from_manufacturer_space_0xfc_supported : 1;
        bool read_from_user_space_0xfb_supported : 1;
        bool write_to_user_space_0xfb_supported : 1;
        bool stream_read_write_supported : 1;
        uint8_t high_address_space;
        uint8_t low_address_space;
        char description[LEN_CONFIG_MEM_OPTIONS_DESCRIPTION];
    } user_configuration_options;

    typedef struct {
        bool present : 1;
        bool read_only : 1;
        bool low_address_valid : 1;
        uint8_t address_space;
        uint32_t highest_address;
        uint32_t low_address;
        char description[LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION];
    } user_address_space_info_t;

    typedef struct {
        user_snip_struct_t snip;
        uint64_t protocol_support;
        uint8_t consumer_count_autocreate;
        uint8_t producer_count_autocreate;
        uint8_t cdi[USER_DEFINED_CDI_LENGTH];
        uint8_t fdi[USER_DEFINED_FDI_LENGTH];
        user_address_space_info_t address_space_configuration_definition;
        user_address_space_info_t address_space_all;
        user_address_space_info_t address_space_config_memory;
        user_address_space_info_t address_space_acdi_manufacturer;
        user_address_space_info_t address_space_acdi_user;
        user_address_space_info_t address_space_traction_function_definition_info;
        user_address_space_info_t address_space_traction_function_config_memory;
        user_configuration_options configuration_options;
        user_address_space_info_t address_space_firmware;

    } node_parameters_t;

    // Event ID Structures

    typedef struct {
        bool running : 1; // Alway, always, always reset these to false when you have finished processing a
        uint8_t enum_index; // allows a counter for enumerating the event ids
    } event_id_enum_t;

    typedef struct {
        uint16_t count;
        event_id_struct_t list[USER_DEFINED_CONSUMER_COUNT];
        event_id_enum_t enumerator;

    } event_id_consumer_list_t;

    typedef struct {
        uint16_t count;
        event_id_struct_t list[USER_DEFINED_PRODUCER_COUNT];
        event_id_enum_t enumerator;

    } event_id_producer_list_t;

    typedef struct {
        uint8_t run_state : 5; // Run state... limits the number to how many bits here.... 32 possible states.
        bool allocated : 1; // Allocated to be used
        bool permitted : 1; // Has the CAN alias been allocated and the network notified
        bool initialized : 1; // Has the node been logged into the the network
        bool duplicate_id_detected : 1; // Node has detected a duplicated Node ID and has sent the PCER
        bool openlcb_datagram_ack_sent : 1; // replying to a datagram requires two messages to be sent, first an ack/nack to say it was successfully received (or not) then the actual response.  This tracks which state the node is in
        bool resend_datagram : 1; // if set the message loop will bypass pulling the next message from the fifo and send the message in sent_datagrams first
        bool firmware_upgrade_active : 1; // Set if the node is in firmware upgrade mode
    } openlcb_node_state_t;

    typedef struct {
        openlcb_node_state_t state;
        uint64_t id;
        uint16_t alias;
        uint64_t seed; // Seed for generating the alias
        event_id_consumer_list_t consumers;
        event_id_producer_list_t producers;
        const node_parameters_t *parameters;
        uint16_t timerticks; // Counts the 100ms timer ticks during the CAN alias allocation
        uint64_t owner_node; // node that has this node locked
        openlcb_msg_t *last_received_datagram;
        uint8_t index; // what index in the node list this node is, used to help with offsets for config memory, fdi memory, etc.
    } openlcb_node_t;

    typedef struct {
        openlcb_node_t node[USER_DEFINED_NODE_BUFFER_DEPTH];
        uint16_t count; // How many have been allocated, you can not deallocate a node so one it is allocated it is there to the end (it can be not permitted)

    } openlcb_nodes_t;

    typedef struct {
        openlcb_msg_t worker;
        payload_stream_t worker_buffer;
        openlcb_msg_t *active_msg;
    } openlcb_statemachine_worker_t;

    typedef void (*parameterless_callback_t)(void);

    typedef struct {
        openlcb_msg_t openlcb_msg;
        payload_stream_t openlcb_payload;

    } openlcb_stream_message_t;

    typedef struct {
        openlcb_msg_t *msg_ptr;
        uint8_t valid : 1;
        uint8_t enumerate : 1;
        openlcb_stream_message_t openlcb_msg;

    } openlcb_outgoing_stream_msg_info_t;

    typedef struct {
        openlcb_msg_t *msg_ptr;
        uint8_t enumerate : 1;

    } openlcb_incoming_msg_info_t;

    typedef struct {
        openlcb_node_t *openlcb_node;
        openlcb_incoming_msg_info_t incoming_msg_info;
        openlcb_outgoing_stream_msg_info_t outgoing_msg_info;

    } openlcb_statemachine_info_t;

    typedef struct {
        openlcb_msg_t openlcb_msg;
        payload_basic_t openlcb_payload;

    } openlcb_basic_message_t;

    typedef struct {
        openlcb_msg_t *msg_ptr;
        uint8_t valid : 1;
        uint8_t enumerate : 1;
        openlcb_basic_message_t openlcb_msg;

    } openlcb_outgoing_basic_msg_info_t;

    typedef struct {
        openlcb_node_t *openlcb_node;
        openlcb_outgoing_basic_msg_info_t outgoing_msg_info;

    } openlcb_login_statemachine_info_t;

    struct config_mem_operations_request_info_TAG;
    typedef void (*operations_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_operations_request_info_TAG *config_mem_operations_request_info);

    typedef struct config_mem_operations_request_info_TAG {
        const user_address_space_info_t *space_info;
        operations_config_mem_space_func_t operations_func;

    } config_mem_operations_request_info_t;

    struct config_mem_read_request_info_TAG;
    typedef void (*read_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_read_request_info_TAG *config_mem_read_request_info);

    typedef struct config_mem_read_request_info_TAG {
        space_encoding_enum encoding;
        uint32_t address;
        uint16_t bytes;
        uint16_t data_start; // what offset into the payload to insert the data begin requested
        const user_address_space_info_t *space_info;
        read_config_mem_space_func_t read_space_func;

    } config_mem_read_request_info_t;


    struct config_mem_write_request_info_TAG;
    typedef void (*write_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_write_request_info_TAG *config_mem_write_request_info);

    typedef struct config_mem_write_request_info_TAG {
        space_encoding_enum encoding;
        uint32_t address;
        uint16_t bytes;
        configuration_memory_buffer_t* write_buffer;
        uint16_t data_start; // what offset into the payload to insert the data begin requested
        const user_address_space_info_t *space_info;
        write_config_mem_space_func_t write_space_func;

    } config_mem_write_request_info_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_TYPES__ */
