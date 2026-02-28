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
 * @brief Core type definitions, structures, and configuration constants for the
 * OpenLCB library.
 *
 * @details Defines all fundamental data types used across the library: message
 * buffers (BASIC/DATAGRAM/SNIP/STREAM pools), node structures with event
 * producer/consumer lists, configuration memory request types, broadcast time
 * clock state, and train state.  All memory is statically allocated at compile
 * time — there is no dynamic allocation at runtime.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_TYPES__
#define __OPENLCB_OPENLCB_TYPES__

#include <stdbool.h>
#include <stdint.h>

#if __has_include("openlcb_user_config.h")
#include "openlcb_user_config.h"
#else
#error "openlcb_user_config.h not found. Copy templates/openlcb_user_config.h to your project include path."
#endif

#ifdef	__cplusplus
  extern "C" {
#endif /* __cplusplus */

    /**
     * @defgroup user_config_constants User-Configurable Constants
     * @brief Compile-time buffer depths, node limits, and event counts.
     *
     * @details All values must be defined in openlcb_user_config.h.  Total buffer
     * count must not exceed 126 on 8-bit processors.
     * @{
     */

    /** @brief Number of BASIC message buffers (16 bytes each) in the pool */
#ifndef USER_DEFINED_BASIC_BUFFER_DEPTH
#error "USER_DEFINED_BASIC_BUFFER_DEPTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Number of DATAGRAM message buffers (72 bytes each) in the pool */
#ifndef USER_DEFINED_DATAGRAM_BUFFER_DEPTH
#error "USER_DEFINED_DATAGRAM_BUFFER_DEPTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Number of SNIP message buffers (256 bytes each) in the pool */
#ifndef USER_DEFINED_SNIP_BUFFER_DEPTH
#error "USER_DEFINED_SNIP_BUFFER_DEPTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Number of STREAM message buffers (512 bytes each) in the pool */
#ifndef USER_DEFINED_STREAM_BUFFER_DEPTH
#error "USER_DEFINED_STREAM_BUFFER_DEPTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of virtual nodes that can be allocated */
#ifndef USER_DEFINED_NODE_BUFFER_DEPTH
#error "USER_DEFINED_NODE_BUFFER_DEPTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Size of CDI buffer in bytes */
#ifndef USER_DEFINED_CDI_LENGTH
#error "USER_DEFINED_CDI_LENGTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Size of FDI buffer in bytes (train nodes) */
#ifndef USER_DEFINED_FDI_LENGTH
#error "USER_DEFINED_FDI_LENGTH must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of events a node can produce */
#ifndef USER_DEFINED_PRODUCER_COUNT
#error "USER_DEFINED_PRODUCER_COUNT must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of producer event ranges (must be >= 1) */
#ifndef USER_DEFINED_PRODUCER_RANGE_COUNT
#error "USER_DEFINED_PRODUCER_RANGE_COUNT must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of events a node can consume */
#ifndef USER_DEFINED_CONSUMER_COUNT
#error "USER_DEFINED_CONSUMER_COUNT must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of consumer event ranges (must be >= 1) */
#ifndef USER_DEFINED_CONSUMER_RANGE_COUNT
#error "USER_DEFINED_CONSUMER_RANGE_COUNT must be defined in openlcb_user_config.h"
#endif

    /** @brief Config Memory address for user-defined node name */
#ifndef USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS
#error "USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS must be defined in openlcb_user_config.h"
#endif

    /** @brief Config Memory address for user description */
#ifndef USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS
#error "USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of train nodes that can be allocated */
#ifndef USER_DEFINED_TRAIN_NODE_COUNT
#error "USER_DEFINED_TRAIN_NODE_COUNT must be defined in openlcb_user_config.h"
#endif

    /** @brief Maximum number of listeners (consist members) per train node */
#ifndef USER_DEFINED_MAX_LISTENERS_PER_TRAIN
#error "USER_DEFINED_MAX_LISTENERS_PER_TRAIN must be defined in openlcb_user_config.h"
#endif

    /** @brief Number of DCC functions supported per train (29 = F0-F28) */
#ifndef USER_DEFINED_MAX_TRAIN_FUNCTIONS
#error "USER_DEFINED_MAX_TRAIN_FUNCTIONS must be defined in openlcb_user_config.h"
#endif

    /** @} */ // end of user_config_constants

    /**
     * @defgroup buffer_size_constants Message Buffer Size Constants
     * @brief Fixed payload sizes and SNIP field lengths.
     * @{
     */

    /** @brief Maximum description length for Configuration Options reply */
#define LEN_CONFIG_MEM_OPTIONS_DESCRIPTION (64 - 1)

    /** @brief Maximum description length for Address Space Info reply */
#define LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION (60 - 1)

    /** @brief NULL/unassigned Node ID value */
#define NULL_NODE_ID 0x000000000000

    /** @brief NULL/unassigned Event ID value */
#define NULL_EVENT_ID 0x0000000000000000

    /** @brief SNIP manufacturer name field length (including null) */
#define LEN_SNIP_NAME_BUFFER 41

    /** @brief SNIP model name field length (including null) */
#define LEN_SNIP_MODEL_BUFFER 41

    /** @brief SNIP hardware version field length (including null) */
#define LEN_SNIP_HARDWARE_VERSION_BUFFER 21

    /** @brief SNIP software version field length (including null) */
#define LEN_SNIP_SOFTWARE_VERSION_BUFFER 21

    /** @brief SNIP user-assigned name field length (including null) */
#define LEN_SNIP_USER_NAME_BUFFER 63

    /** @brief SNIP user description field length (including null) */
#define LEN_SNIP_USER_DESCRIPTION_BUFFER 64

    /** @brief Total SNIP user data size (name + description) */
#define LEN_SNIP_USER_DATA (LEN_SNIP_USER_NAME_BUFFER + LEN_SNIP_USER_DESCRIPTION_BUFFER)

    /** @brief SNIP manufacturer version field length (1 byte) */
#define LEN_SNIP_VERSION 1

    /** @brief SNIP user version field length (1 byte) */
#define LEN_SNIP_USER_VERSION 1

    /** @brief Maximum SNIP structure size (256 payload + 8 Event ID) */
#define LEN_SNIP_STRUCTURE 264

    /** @brief BASIC message payload size */
#define LEN_MESSAGE_BYTES_BASIC 16

    /** @brief DATAGRAM message maximum payload size */
#define LEN_MESSAGE_BYTES_DATAGRAM 72

    /** @brief SNIP message payload size (also covers Events with Payload) */
#define LEN_MESSAGE_BYTES_SNIP 256

    /** @brief STREAM message payload size */
#define LEN_MESSAGE_BYTES_STREAM 512

    /** @brief Event ID size in bytes */
#define LEN_EVENT_ID 8

    /** @brief Total number of message buffers (sum of all buffer types) */
#define LEN_MESSAGE_BUFFER (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_STREAM_BUFFER_DEPTH)

    /** @brief Maximum datagram payload after protocol overhead */
#define LEN_DATAGRAM_MAX_PAYLOAD 64

    /** @brief Event payload maximum size (uses SNIP buffer) */
#define LEN_EVENT_PAYLOAD LEN_MESSAGE_BYTES_SNIP

    /** @} */ // end of buffer_size_constants

        /** @brief Message buffer payload type identifier. */
    typedef enum {

        BASIC,      /**< 16-byte payload buffer */
        DATAGRAM,   /**< 72-byte payload buffer */
        SNIP,       /**< 256-byte payload buffer */
        STREAM      /**< 512-byte payload buffer */

    } payload_type_enum;

        /** @brief Event status for Producer/Consumer identification messages. */
    typedef enum {

        EVENT_STATUS_UNKNOWN,   /**< State is unknown */
        EVENT_STATUS_SET,       /**< Event is SET */
        EVENT_STATUS_CLEAR      /**< Event is CLEAR */

    } event_status_enum;

        /** @brief Where the address space byte is encoded in a Config Mem command. */
    typedef enum {

        ADDRESS_SPACE_IN_BYTE_1 = 0,    /**< Space ID in command byte 1 */
        ADDRESS_SPACE_IN_BYTE_6 = 1     /**< Space ID in command byte 6 */

    } space_encoding_enum;

        /** @brief Power-of-two event range sizes for range-identified events. */
    typedef enum {

        EVENT_RANGE_COUNT_1 = 0,
        EVENT_RANGE_COUNT_2 = 2,
        EVENT_RANGE_COUNT_4 = 4,
        EVENT_RANGE_COUNT_8 = 8,
        EVENT_RANGE_COUNT_16 = 16,
        EVENT_RANGE_COUNT_32 = 32,
        EVENT_RANGE_COUNT_64 = 64,
        EVENT_RANGE_COUNT_128 = 128,
        EVENT_RANGE_COUNT_256 = 256,
        EVENT_RANGE_COUNT_512 = 512,
        EVENT_RANGE_COUNT_1024 = 1024,
        EVENT_RANGE_COUNT_2048 = 2048,
        EVENT_RANGE_COUNT_4096 = 4096,
        EVENT_RANGE_COUNT_8192 = 8192,
        EVENT_RANGE_COUNT_16384 = 16384,
        EVENT_RANGE_COUNT_32768 = 32768

    } event_range_count_enum;

        /** @brief Broadcast Time Protocol event type decoded from an Event ID. */
    typedef enum {

        BROADCAST_TIME_EVENT_REPORT_TIME = 0,
        BROADCAST_TIME_EVENT_REPORT_DATE = 1,
        BROADCAST_TIME_EVENT_REPORT_YEAR = 2,
        BROADCAST_TIME_EVENT_REPORT_RATE = 3,
        BROADCAST_TIME_EVENT_SET_TIME = 4,
        BROADCAST_TIME_EVENT_SET_DATE = 5,
        BROADCAST_TIME_EVENT_SET_YEAR = 6,
        BROADCAST_TIME_EVENT_SET_RATE = 7,
        BROADCAST_TIME_EVENT_QUERY = 8,
        BROADCAST_TIME_EVENT_STOP = 9,
        BROADCAST_TIME_EVENT_START = 10,
        BROADCAST_TIME_EVENT_DATE_ROLLOVER = 11,
        BROADCAST_TIME_EVENT_UNKNOWN = 255

    } broadcast_time_event_type_enum;

        /** @brief Emergency state type for train protocol callbacks. */
    typedef enum {

        TRAIN_EMERGENCY_TYPE_ESTOP = 0,        /**< Point-to-point Emergency Stop */
        TRAIN_EMERGENCY_TYPE_GLOBAL_STOP = 1,  /**< Global Emergency Stop (event-based) */
        TRAIN_EMERGENCY_TYPE_GLOBAL_OFF = 2    /**< Global Emergency Off (event-based) */

    } train_emergency_type_enum;

    /**
     * @defgroup payload_buffer_types Payload Buffer Type Definitions
     * @brief Array typedefs for each message payload size.
     * @{
     */

        /** @brief BASIC message payload buffer (16 bytes) */
    typedef uint8_t payload_basic_t[LEN_MESSAGE_BYTES_BASIC];

        /** @brief DATAGRAM message payload buffer (72 bytes) */
    typedef uint8_t payload_datagram_t[LEN_MESSAGE_BYTES_DATAGRAM];

        /** @brief SNIP message payload buffer (256 bytes) */
    typedef uint8_t payload_snip_t[LEN_MESSAGE_BYTES_SNIP];

        /** @brief STREAM message payload buffer (512 bytes) */
    typedef uint8_t payload_stream_t[LEN_MESSAGE_BYTES_STREAM];

    /** @} */ // end of payload_buffer_types

    /**
     * @defgroup payload_pool_types Payload Buffer Pool Arrays
     * @brief Storage pools for each payload type.
     * @{
     */

        /** @brief Array of BASIC payload buffers */
    typedef payload_basic_t openlcb_basic_data_buffer_t[USER_DEFINED_BASIC_BUFFER_DEPTH];

        /** @brief Array of DATAGRAM payload buffers */
    typedef payload_datagram_t openlcb_datagram_data_buffer_t[USER_DEFINED_DATAGRAM_BUFFER_DEPTH];

        /** @brief Array of SNIP payload buffers */
    typedef payload_snip_t openlcb_snip_data_buffer_t[USER_DEFINED_SNIP_BUFFER_DEPTH];

        /** @brief Array of STREAM payload buffers */
    typedef payload_stream_t openlcb_stream_data_buffer_t[USER_DEFINED_STREAM_BUFFER_DEPTH];

    /** @} */ // end of payload_pool_types

        /** @brief Generic 1-byte payload pointer type for casting. */
    typedef uint8_t openlcb_payload_t[1];

        /** @brief 64-bit Event ID. */
    typedef uint64_t event_id_t;

        /** @brief Event ID paired with its current status. */
    typedef struct {

        event_id_t event;           /**< 64-bit Event ID */
        event_status_enum status;   /**< Current event status */

    } event_id_struct_t;

        /** @brief Contiguous range of Event IDs starting at a base address. */
    typedef struct {

        event_id_t start_base;              /**< Starting Event ID (bottom 16 bits must be 00.00) */
        event_range_count_enum event_count; /**< Number of consecutive Event IDs in the range */

    } event_id_range_t;

        /** @brief 48-bit Node ID stored in a 64-bit type (upper 16 bits unused). */
    typedef uint64_t node_id_t;

        /** @brief Event payload data buffer (LEN_EVENT_PAYLOAD bytes). */
    typedef uint8_t event_payload_t[LEN_EVENT_PAYLOAD];

        /** @brief Configuration memory read/write operation buffer (64 bytes). */
    typedef uint8_t configuration_memory_buffer_t[LEN_DATAGRAM_MAX_PAYLOAD];

        /** @brief Broadcast Time hour/minute. */
    typedef struct {

        uint8_t hour;   /**< Hour 0-23 */
        uint8_t minute; /**< Minute 0-59 */
        bool valid;     /**< true if data has been received */

    } broadcast_time_t;

        /** @brief Broadcast Time month/day. */
    typedef struct {

        uint8_t month; /**< Month 1-12 */
        uint8_t day;   /**< Day 1-31 */
        bool valid;    /**< true if data has been received */

    } broadcast_date_t;

        /** @brief Broadcast Time year. */
    typedef struct {

        uint16_t year; /**< Year 0-4095 AD */
        bool valid;    /**< true if data has been received */

    } broadcast_year_t;

        /**
         * @brief Broadcast Time clock rate (12-bit signed fixed point, 2 fractional bits).
         *
         * @details Range -512.00 to +511.75 in 0.25 increments.
         * Example: 0x0004 = 1.00x (real-time), 0x0010 = 4.00x.
         */
    typedef struct {

        int16_t rate; /**< Clock rate */
        bool valid;   /**< true if data has been received */

    } broadcast_rate_t;

        /** @brief Complete state for one Broadcast Time clock. */
    typedef struct {

        uint64_t clock_id;       /**< Clock identifier (upper 6 bytes of event IDs) */
        broadcast_time_t time;   /**< Current time */
        broadcast_date_t date;   /**< Current date */
        broadcast_year_t year;   /**< Current year */
        broadcast_rate_t rate;   /**< Clock rate */
        bool is_running;         /**< true = running, false = stopped */
        uint32_t ms_accumulator; /**< Internal: accumulated ms toward next minute */

    } broadcast_clock_state_t;

        /** @brief A clock slot with state and subscription flags. */
    typedef struct {

        broadcast_clock_state_t state;
        bool is_consumer : 1;
        bool is_producer : 1;
        bool is_allocated : 1;

    } broadcast_clock_t;

        /** @brief Message buffer allocation/assembly state flags. */
    typedef struct {

        bool allocated : 1;     /**< Buffer is in use */
        bool inprocess : 1;     /**< Multi-frame message being assembled */

    } openlcb_msg_state_t;

        /**
         * @brief Core OpenLCB message structure.
         *
         * @details Holds MTI, source/dest addressing, payload pointer, and
         * reference count.  Multi-frame messages are assembled with
         * state.inprocess = 1 until the final frame arrives.
         *
         * @warning Reference count must be managed correctly to prevent leaks.
         */
    typedef struct {

        openlcb_msg_state_t state;      /**< Message state flags */
        uint16_t mti;                   /**< Message Type Indicator */
        uint16_t source_alias;          /**< Source node 12-bit CAN alias */
        uint16_t dest_alias;            /**< Destination node 12-bit CAN alias (0 if global) */
        node_id_t source_id;            /**< Source node 48-bit Node ID */
        node_id_t dest_id;              /**< Destination node 48-bit Node ID (0 if global) */
        payload_type_enum payload_type; /**< Payload buffer size category */
        uint16_t payload_count;         /**< Valid bytes currently in payload */
        openlcb_payload_t *payload;     /**< Pointer to payload buffer */
        uint8_t timerticks;             /**< Timer tick counter for timeouts */
        uint8_t reference_count;        /**< Number of active references to this message */

    } openlcb_msg_t;

        /** @brief Array of all message structures in the buffer store. */
    typedef openlcb_msg_t openlcb_msg_array_t[LEN_MESSAGE_BUFFER];

        /** @brief Master buffer storage: message structures + segregated payload pools. */
    typedef struct {

        openlcb_msg_array_t messages;            /**< Array of message structures */
        openlcb_basic_data_buffer_t basic;       /**< Pool of BASIC payload buffers */
        openlcb_datagram_data_buffer_t datagram; /**< Pool of DATAGRAM payload buffers */
        openlcb_snip_data_buffer_t snip;         /**< Pool of SNIP payload buffers */
        openlcb_stream_data_buffer_t stream;     /**< Pool of STREAM payload buffers */

    } message_buffer_t;

        /**
         * @brief SNIP identification strings (manufacturer + user version byte).
         *
         * @details Manufacturer fields are read-only (ACDI space 0xFC).
         * User name/description are stored separately in node configuration.
         */
    typedef struct {

        uint8_t mfg_version;                                     /**< Manufacturer SNIP version (always 1) */
        char name[LEN_SNIP_NAME_BUFFER];                         /**< Manufacturer name */
        char model[LEN_SNIP_MODEL_BUFFER];                       /**< Model name */
        char hardware_version[LEN_SNIP_HARDWARE_VERSION_BUFFER]; /**< Hardware version */
        char software_version[LEN_SNIP_SOFTWARE_VERSION_BUFFER]; /**< Software version */
        uint8_t user_version;                                    /**< User SNIP version (always 1) */

    } user_snip_struct_t;

        /** @brief Capability flags returned by Get Configuration Options command. */
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

        /** @brief Properties of a single configuration memory address space. */
    typedef struct {

        bool present : 1;
        bool read_only : 1;
        bool low_address_valid : 1;
        uint8_t address_space;      /**< Space identifier (0x00-0xFF) */
        uint32_t highest_address;
        uint32_t low_address;       /**< Valid only when low_address_valid is set */
        char description[LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION];

    } user_address_space_info_t;

        /**
         * @brief Complete node configuration parameters (typically const/flash).
         *
         * @details Contains SNIP strings, protocol support bits, CDI/FDI data,
         * and address space information for every supported space.
         */
    typedef struct {

        user_snip_struct_t snip;
        uint64_t protocol_support;              /**< Protocol Support Indicator bits */
        uint8_t consumer_count_autocreate;
        uint8_t producer_count_autocreate;
        uint8_t cdi[USER_DEFINED_CDI_LENGTH];
        uint8_t fdi[USER_DEFINED_FDI_LENGTH];
        user_address_space_info_t address_space_configuration_definition; /**< Space 0xFF */
        user_address_space_info_t address_space_all;                     /**< Space 0xFE */
        user_address_space_info_t address_space_config_memory;           /**< Space 0xFD */
        user_address_space_info_t address_space_acdi_manufacturer;       /**< Space 0xFC */
        user_address_space_info_t address_space_acdi_user;               /**< Space 0xFB */
        user_address_space_info_t address_space_train_function_definition_info; /**< Space 0xFA */
        user_address_space_info_t address_space_train_function_config_memory;   /**< Space 0xF9 */
        user_configuration_options configuration_options;
        user_address_space_info_t address_space_firmware;                 /**< Space 0xEF */

    } node_parameters_t;

        /**
         * @brief Event list enumeration state.
         *
         * @warning Always reset running to false when finished processing a message.
         */
    typedef struct {

        bool running : 1;         /**< Enumeration is in progress */
        uint8_t enum_index;       /**< Current position in event list */
        uint8_t range_enum_index; /**< Current position in range list */

    } event_id_enum_t;

        /** @brief List of events consumed by a node. */
    typedef struct {

        uint16_t count;
        event_id_struct_t list[USER_DEFINED_CONSUMER_COUNT];
        uint16_t range_count;
        event_id_range_t range_list[USER_DEFINED_CONSUMER_RANGE_COUNT];
        event_id_enum_t enumerator;

    } event_id_consumer_list_t;

        /** @brief List of events produced by a node. */
    typedef struct {

        uint16_t count;
        event_id_struct_t list[USER_DEFINED_PRODUCER_COUNT];
        uint16_t range_count;
        event_id_range_t range_list[USER_DEFINED_PRODUCER_RANGE_COUNT];
        event_id_enum_t enumerator;

    } event_id_producer_list_t;

        /** @brief Bit-packed node state flags. */
    typedef struct {

        uint8_t run_state : 5;              /**< Login state machine state (0-31) */
        bool allocated : 1;                 /**< Node is allocated */
        bool permitted : 1;                 /**< CAN alias allocated */
        bool initialized : 1;              /**< Node fully initialized */
        bool duplicate_id_detected : 1;     /**< Duplicate Node ID conflict */
        bool openlcb_datagram_ack_sent : 1; /**< Datagram ACK sent, awaiting reply */
        bool resend_datagram : 1;           /**< Resend last datagram (retry logic) */
        bool firmware_upgrade_active : 1;   /**< Firmware upgrade in progress */

    } openlcb_node_state_t;

        /** @brief A single listener entry for a train consist. */
    typedef struct {

        node_id_t node_id; /**< Listener node ID (0 = unused) */
        uint8_t flags;     /**< Listener flags (reverse, link F0, link Fn, hide) */

    } train_listener_entry_t;

        /**
         * @brief Mutable runtime state for a single train node.
         *
         * @details Allocated from a static pool by OpenLcbApplicationTrain_setup().
         */
    typedef struct train_state_TAG {

        uint16_t set_speed;               /**< Last commanded speed (float16) */
        uint16_t commanded_speed;         /**< Control algorithm output speed (float16) */
        uint16_t actual_speed;            /**< Measured speed, optional (float16) */
        bool estop_active;                /**< Point-to-point E-stop active */
        bool global_estop_active;         /**< Global Emergency Stop active */
        bool global_eoff_active;          /**< Global Emergency Off active */
        node_id_t controller_node_id;     /**< Active controller (0 if none) */
        uint8_t reserved_node_count;      /**< Reservation count */
        uint32_t heartbeat_timeout_s;     /**< Heartbeat deadline in seconds (0 = disabled) */
        uint32_t heartbeat_counter_100ms; /**< Heartbeat countdown in 100ms ticks */

        train_listener_entry_t listeners[USER_DEFINED_MAX_LISTENERS_PER_TRAIN];
        uint8_t listener_count;

        uint16_t functions[USER_DEFINED_MAX_TRAIN_FUNCTIONS]; /**< Function values indexed by function number */

        uint16_t dcc_address;    /**< DCC address (0 = not set) */
        bool is_long_address;    /**< true = extended (long) DCC address */
        uint8_t speed_steps;     /**< 0=default, 1=14, 2=28, 3=128 */

        struct openlcb_node_TAG *owner_node; /**< Back-pointer to owning node */

    } train_state_t;

        /**
         * @brief OpenLCB virtual node.
         *
         * @details Holds identity, state, event lists, and a pointer to const
         * configuration parameters.  Nodes cannot be deallocated once allocated.
         */
    typedef struct openlcb_node_TAG {

        openlcb_node_state_t state;
        uint64_t id;                            /**< 48-bit Node ID */
        uint16_t alias;                         /**< 12-bit CAN alias */
        uint64_t seed;                          /**< Seed for alias generation */
        event_id_consumer_list_t consumers;
        event_id_producer_list_t producers;
        const node_parameters_t *parameters;
        uint16_t timerticks;                    /**< 100ms timer tick counter */
        uint64_t owner_node;                    /**< Node ID that has locked this node */
        openlcb_msg_t *last_received_datagram;  /**< Saved for reply processing */
        uint8_t index;                          /**< Index in node array */
        struct train_state_TAG *train_state;    /**< NULL if not a train node */

    } openlcb_node_t;

        /** @brief Collection of all virtual nodes. */
    typedef struct {

        openlcb_node_t node[USER_DEFINED_NODE_BUFFER_DEPTH];
        uint16_t count; /**< Number of allocated nodes (never decreases) */

    } openlcb_nodes_t;

        /** @brief State machine temporary working buffers. */
    typedef struct {

        openlcb_msg_t worker;
        payload_stream_t worker_buffer;
        openlcb_msg_t *active_msg;

    } openlcb_statemachine_worker_t;

        /** @brief Callback function type with no parameters. */
    typedef void (*parameterless_callback_t)(void);

        /** @brief Message with inline STREAM-sized payload. */
    typedef struct {

        openlcb_msg_t openlcb_msg;
        payload_stream_t openlcb_payload;

    } openlcb_stream_message_t;

        /** @brief Outgoing message context for the main state machine. */
    typedef struct {

        openlcb_msg_t *msg_ptr;
        uint8_t valid : 1;
        uint8_t enumerate : 1;
        openlcb_stream_message_t openlcb_msg;

    } openlcb_outgoing_stream_msg_info_t;

        /** @brief Incoming message context. */
    typedef struct {

        openlcb_msg_t *msg_ptr;
        uint8_t enumerate : 1;

    } openlcb_incoming_msg_info_t;

        /** @brief Complete context passed to protocol handler functions. */
    typedef struct {

        openlcb_node_t *openlcb_node;
        openlcb_incoming_msg_info_t incoming_msg_info;
        openlcb_outgoing_stream_msg_info_t outgoing_msg_info;

    } openlcb_statemachine_info_t;

        /** @brief Message with inline BASIC-sized payload. */
    typedef struct {

        openlcb_msg_t openlcb_msg;
        payload_basic_t openlcb_payload;

    } openlcb_basic_message_t;

        /** @brief Outgoing message context for the login state machine. */
    typedef struct {

        openlcb_msg_t *msg_ptr;
        uint8_t valid : 1;
        uint8_t enumerate : 1;
        openlcb_basic_message_t openlcb_msg;

    } openlcb_outgoing_basic_msg_info_t;

        /** @brief Login state machine context. */
    typedef struct {

        openlcb_node_t *openlcb_node;
        openlcb_outgoing_basic_msg_info_t outgoing_msg_info;

    } openlcb_login_statemachine_info_t;

        /** @brief Forward declaration for config mem operations callback. */
    struct config_mem_operations_request_info_TAG;

        /** @brief Config mem operations callback function type. */
    typedef void (*operations_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_operations_request_info_TAG *config_mem_operations_request_info);

        /** @brief Request info for Get Options / Get Address Space Info commands. */
    typedef struct config_mem_operations_request_info_TAG {

        const user_address_space_info_t *space_info;
        operations_config_mem_space_func_t operations_func;

    } config_mem_operations_request_info_t;

        /** @brief Forward declaration for config mem read callback. */
    struct config_mem_read_request_info_TAG;

        /** @brief Config mem read callback function type. */
    typedef void (*read_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_read_request_info_TAG *config_mem_read_request_info);

        /** @brief Request info for a configuration memory read operation. */
    typedef struct config_mem_read_request_info_TAG {

        space_encoding_enum encoding;
        uint32_t address;
        uint16_t bytes;
        uint16_t data_start;  /**< Offset into reply payload to insert data */
        const user_address_space_info_t *space_info;
        read_config_mem_space_func_t read_space_func;

    } config_mem_read_request_info_t;

        /** @brief Forward declaration for config mem write callback. */
    struct config_mem_write_request_info_TAG;

        /** @brief Config mem write callback function type. */
    typedef void (*write_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_write_request_info_TAG *config_mem_write_request_info);

        /** @brief Request info for a configuration memory write operation. */
    typedef struct config_mem_write_request_info_TAG {

        space_encoding_enum encoding;
        uint32_t address;
        uint16_t bytes;
        configuration_memory_buffer_t *write_buffer;
        uint16_t data_start;  /**< Offset into write_buffer where data begins */
        const user_address_space_info_t *space_info;
        write_config_mem_space_func_t write_space_func;

    } config_mem_write_request_info_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_TYPES__ */
