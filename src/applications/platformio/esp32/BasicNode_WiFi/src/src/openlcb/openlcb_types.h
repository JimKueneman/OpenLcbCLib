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
 * @brief Core type definitions, structures, and configuration constants for OpenLCB library
 *
 * @details This header file defines all fundamental types and data structures used
 * throughout the OpenLCB protocol implementation. It provides:
 *
 * **User-Configurable Constants:**
 * - Buffer pool depths for different message types (BASIC, DATAGRAM, SNIP, STREAM)
 * - Maximum node count
 * - Event producer/consumer array sizes
 * - CDI and FDI buffer lengths
 * - Configuration memory addresses
 *
 * **Core Type Definitions:**
 * - Node identifiers (48-bit Node IDs)
 * - Event identifiers (64-bit Event IDs)
 * - Message structures with variable-size payloads
 * - Node state and configuration structures
 * - Event producer/consumer lists
 * - Configuration memory structures
 *
 * **Message Buffer Architecture:**
 * The library uses segregated buffer pools organized by payload size:
 * - BASIC: 16 bytes - For simple messages (most MTIs)
 * - DATAGRAM: 72 bytes - For datagram protocol
 * - SNIP: 256 bytes - For SNIP and Events with Payload
 * - STREAM: 512 bytes - For stream protocol
 *
 * **Node Architecture:**
 * Each node maintains:
 * - Unique 48-bit Node ID and 12-bit CAN alias
 * - State flags (allocated, permitted, initialized)
 * - Lists of produced and consumed Event IDs
 * - Pointer to configuration parameters
 * - Reference to last received datagram (for replies)
 *
 * **Configuration Memory:**
 * Supports multiple address spaces per OpenLCB Memory Configuration Protocol:
 * - 0xFF: CDI (Configuration Description Information)
 * - 0xFE: All memory combined
 * - 0xFD: Configuration Memory (user settings)
 * - 0xFC: ACDI Manufacturer (read-only)
 * - 0xFB: ACDI User (user name/description)
 * - 0xFA/0xF9: Traction FDI and configuration
 * - 0xEF: Firmware upgrade
 *
 * **Design Philosophy:**
 * - Static allocation: All buffers pre-allocated at compile time
 * - Reference counting: Messages can be shared between queues
 * - Type safety: Extensive use of typedef for better type checking
 * - Segregated pools: Different message types have different buffer sizes
 * - Enumeration support: Multiple subsystems can enumerate nodes independently
 *
 * @note All buffer depths can be overridden via compiler macros
 * @note Total buffer count must not exceed 126 for 8-bit processors
 * @note Node IDs and Event IDs use 64-bit types for consistency
 *
 * @warning Changing buffer depths requires recompilation
 * @warning Node structures cannot be deallocated once allocated
 *
 * @see openlcb_defines.h - Protocol constants and MTI definitions
 * @see openlcb_buffer_store.h - Buffer allocation functions
 * @see openlcb_node.h - Node management functions
 * @see OpenLCB Memory Configuration Protocol Standard
 * @see OpenLCB Message Network Standard S-9.7.1.1
 *
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

        /**
        * @defgroup user_config_constants User-Configurable Constants
        * @brief Compile-time configuration parameters for buffer sizes and limits
        *
        * @details These constants control memory allocation for the OpenLCB library.
        * They can be overridden by defining them in your compiler's preprocessor
        * macros before including this header.
        *
        * **Buffer Pool Configuration:**
        * The total number of message buffers is the sum of all four buffer types.
        * For 8-bit processors, this sum must not exceed 126 (signed 8-bit limit).
        *
        * **Typical Configurations:**
        * - Small nodes: BASIC=10, DATAGRAM=2, SNIP=2, STREAM=0, NODE=1
        * - Medium nodes: BASIC=20, DATAGRAM=4, SNIP=4, STREAM=1, NODE=2
        * - Large nodes: BASIC=32, DATAGRAM=4, SNIP=4, STREAM=1, NODE=4
        *
        * **Event Configuration:**
        * Producer/consumer counts determine how many events a node can handle.
        * Maximum is 126 for 8-bit processors.
        *
        * @note Override these in your build system, not by editing this file
        * @note Test defaults are provided for Google Tests
        * @{
        */

    /** @brief Number of BASIC message buffers (16 bytes each) in the pool */
#ifndef USER_DEFINED_BASIC_BUFFER_DEPTH
#define USER_DEFINED_BASIC_BUFFER_DEPTH 32
#endif

    /** @brief Number of DATAGRAM message buffers (72 bytes each) in the pool */
#ifndef USER_DEFINED_DATAGRAM_BUFFER_DEPTH
#define USER_DEFINED_DATAGRAM_BUFFER_DEPTH 4
#endif

    /** @brief Number of SNIP message buffers (256 bytes each) in the pool */
#ifndef USER_DEFINED_SNIP_BUFFER_DEPTH
#define USER_DEFINED_SNIP_BUFFER_DEPTH 4
#endif

    /** @brief Number of STREAM message buffers (512 bytes each) in the pool */
#ifndef USER_DEFINED_STREAM_BUFFER_DEPTH
#define USER_DEFINED_STREAM_BUFFER_DEPTH 1
#endif

    /** @brief Maximum number of virtual nodes that can be allocated */
#ifndef USER_DEFINED_NODE_BUFFER_DEPTH
#define USER_DEFINED_NODE_BUFFER_DEPTH 4
#endif

    /** @brief Size of CDI (Configuration Description Information) buffer in bytes */
#ifndef USER_DEFINED_CDI_LENGTH
#define USER_DEFINED_CDI_LENGTH 20000
#endif

    /** @brief Size of FDI (Function Description Information) buffer in bytes for traction nodes */
#ifndef USER_DEFINED_FDI_LENGTH
#define USER_DEFINED_FDI_LENGTH 1000
#endif

    /** @brief Maximum number of events a node can produce */
#ifndef USER_DEFINED_PRODUCER_COUNT
#define USER_DEFINED_PRODUCER_COUNT 64
#endif

    /** @brief Maximum number of event ranges a node can produce, must be at least one for a non zero array */
#ifndef USER_DEFINED_PRODUCER_RANGE_COUNT
#define USER_DEFINED_PRODUCER_RANGE_COUNT 5
#endif

        /** @brief Maximum number of events a node can consume */
#ifndef USER_DEFINED_CONSUMER_COUNT
#define USER_DEFINED_CONSUMER_COUNT 32
#endif

    /** @brief Maximum number of event ranges a node can consume, must be at least one for a non zero array */
#ifndef USER_DEFINED_CONSUMER_RANGE_COUNT
#define USER_DEFINED_CONSUMER_RANGE_COUNT 5
#endif

        /** @brief Address in Configuration Memory for user-defined node name */
#ifndef USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS
#define USER_DEFINED_CONFIG_MEM_USER_NAME_ADDRESS 0x00000000
#endif

    /** @brief Address in Configuration Memory for user description */
#ifndef USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS
#define USER_DEFINED_CONFIG_MEM_USER_DESCRIPTION_ADDRESS (LEN_SNIP_USER_NAME_BUFFER - 1)
#endif

        /** @} */ // end of user_config_constants

        /**
        * @defgroup buffer_size_constants Message Buffer Size Constants
        * @brief Fixed-size buffer lengths for different message and data types
        *
        * @details These constants define the payload sizes for different message types
        * and SNIP (Simple Node Information Protocol) data fields.
        *
        * **Message Payload Sizes:**
        * - BASIC (16 bytes): Handles most OpenLCB messages; some protocols like
        *   Traction may use two consecutive frames
        * - DATAGRAM (72 bytes): Maximum datagram payload after protocol overhead
        * - SNIP (256 bytes): Supports SNIP replies and Events with Payload
        * - STREAM (512 bytes): Stream protocol bulk data transfer
        *
        * **SNIP Field Sizes:**
        * Per OpenLCB SNIP specification, these are the maximum lengths including
        * the null terminator, so actual string content is one byte less.
        *
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

    /** @brief SNIP manufacturer name field length (including null terminator) */
#define LEN_SNIP_NAME_BUFFER 41

    /** @brief SNIP model name field length (including null terminator) */
#define LEN_SNIP_MODEL_BUFFER 41

    /** @brief SNIP hardware version field length (including null terminator) */
#define LEN_SNIP_HARDWARE_VERSION_BUFFER 21

    /** @brief SNIP software version field length (including null terminator) */
#define LEN_SNIP_SOFTWARE_VERSION_BUFFER 21

    /** @brief SNIP user-assigned name field length (including null terminator) */
#define LEN_SNIP_USER_NAME_BUFFER 63

    /** @brief SNIP user description field length (including null terminator) */
#define LEN_SNIP_USER_DESCRIPTION_BUFFER 64

    /** @brief Total SNIP user data size (name + description) */
#define LEN_SNIP_USER_DATA (LEN_SNIP_USER_NAME_BUFFER + LEN_SNIP_USER_DESCRIPTION_BUFFER)

    /** @brief SNIP manufacturer version field length (1 byte) */
#define LEN_SNIP_VERSION 1

    /** @brief SNIP user version field length (1 byte) */
#define LEN_SNIP_USER_VERSION 1

    /** @brief Maximum SNIP structure size (Event with Payload: 256 payload + 8 Event ID) */
#define LEN_SNIP_STRUCTURE 264

    /** @brief BASIC message payload size (most messages fit in 8 bytes, some use 2 frames) */
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

    /** @brief Maximum datagram payload after subtracting protocol overhead */
#define LEN_DATAGRAM_MAX_PAYLOAD 64

    /** @brief Event payload maximum size (uses SNIP buffer) */
#define LEN_EVENT_PAYLOAD LEN_MESSAGE_BYTES_SNIP

        /** @} */ // end of buffer_size_constants

        /**
        * @enum payload_type_enum
        * @brief Message buffer payload type enumeration
        *
        * @details Identifies which size category a message buffer belongs to.
        * Used for buffer allocation and payload pointer management.
        *
        * The buffer store maintains separate pools for each type, allowing
        * efficient memory usage by matching buffer size to message requirements.
        *
        * @see OpenLcbBufferStore_allocate_buffer
        * @see openlcb_msg_t.payload_type
        */
    typedef enum {
        BASIC,      /**< 16-byte payload buffer for simple messages */
        DATAGRAM,   /**< 72-byte payload buffer for datagrams */
        SNIP,       /**< 256-byte payload buffer for SNIP and Events with Payload */
        STREAM      /**< 512-byte payload buffer for stream data */

    } payload_type_enum;

        /**
        * @enum event_status_enum
        * @brief Event status indicator for Producer/Consumer events
        *
        * @details Indicates the current state of an event from a producer's
        * or consumer's perspective. Used in Event Transport Protocol messages.
        *
        * **Producer perspective:**
        * - UNKNOWN: Producer hasn't determined current state
        * - SET: Event is currently active/true
        * - CLEAR: Event is currently inactive/false
        *
        * **Consumer perspective:**
        * - UNKNOWN: Consumer doesn't know if it would react to event
        * - SET: Consumer would act if event occurs
        * - CLEAR: Consumer would not act if event occurs
        *
        * @see MTI_PRODUCER_IDENTIFIED_SET
        * @see MTI_CONSUMER_IDENTIFIED_CLEAR
        * @see event_id_struct_t
        */
    typedef enum {
        EVENT_STATUS_UNKNOWN,   /**< Event state is unknown */
        EVENT_STATUS_SET,       /**< Event is in SET state */
        EVENT_STATUS_CLEAR      /**< Event is in CLEAR state */

    } event_status_enum;

        /**
        * @enum space_encoding_enum
        * @brief Configuration memory address space encoding method
        *
        * @details Specifies where the address space identifier is located in
        * a Configuration Memory Protocol command message.
        *
        * - ADDRESS_SPACE_IN_BYTE_1: Space ID is in byte 1 of command (for spaces < 0xFD)
        * - ADDRESS_SPACE_IN_BYTE_6: Space ID is in byte 6 (for well-known spaces 0xFD-0xFF)
        *
        * @see CONFIG_MEM_READ_SPACE_IN_BYTE_6
        * @see CONFIG_MEM_READ_SPACE_FD
        * @see config_mem_read_request_info_t.encoding
        */
    typedef enum {
        ADDRESS_SPACE_IN_BYTE_1 = 0,    /**< Address space ID in command byte 1 */
        ADDRESS_SPACE_IN_BYTE_6 = 1     /**< Address space ID in command byte 6 */

    } space_encoding_enum;

    typedef enum
    {
        EVENT_RANGE_COUNT_1 = 0,         /**< 2^0 Single event */
        EVENT_RANGE_COUNT_2 = 2,         /**< 2^1 Two consecutive events */
        EVENT_RANGE_COUNT_4 = 4,         /**< 2^2 Four consecutive events */
        EVENT_RANGE_COUNT_8 = 8,         /**< 2^3 Eight consecutive events */
        EVENT_RANGE_COUNT_16 = 16,       /**< 2^4 Sixteen consecutive events */
        EVENT_RANGE_COUNT_32 = 32,       /**< 2^5 Thirty-two    */
        EVENT_RANGE_COUNT_64 = 64,       /**< 2^6 Sixty-four    */
        EVENT_RANGE_COUNT_128 = 128,     /**< 2^7 One hundred twenty-eight  */
        EVENT_RANGE_COUNT_256 = 256,     /**< 2^8 Two hundred fifty-six  */
        EVENT_RANGE_COUNT_512 = 512,     /**< 2^9 Five hundred twelve  */
        EVENT_RANGE_COUNT_1024 = 1024,   /**< 2^10 One thousand twenty-four  */
        EVENT_RANGE_COUNT_2048 = 2048,   /**< 2^11 Two thousand forty-eight  */
        EVENT_RANGE_COUNT_4096 = 4096,   /**< 2^12 Four thousand ninety-six  */
        EVENT_RANGE_COUNT_8192 = 8192,   /**< 2^13 Eight thousand one hundred ninety-two  */
        EVENT_RANGE_COUNT_16384 = 16384, /**< 2^14 Sixteen thousand three hundred eighty-four  */
        EVENT_RANGE_COUNT_32768 = 32768  /**< 2^15 Thirty-two thousand seven hundred sixty-eight  */

    } event_range_count_enum;

    /**
     * @enum broadcast_time_event_type_enum
     * @brief Broadcast Time Protocol event type enumeration
     *
     * @details Identifies the type of broadcast time event encoded in an Event ID.
     * Used to determine which data (time/date/year/rate/command) is present.
     *
     * @see OpenLcbUtilities_get_broadcast_time_event_type
     * @see ProtocolBroadcastTime_handle_time_event
     */
    typedef enum {
        
        BROADCAST_TIME_EVENT_REPORT_TIME = 0,    /**< Report Time event (hour/minute) */
        BROADCAST_TIME_EVENT_REPORT_DATE = 1,    /**< Report Date event (month/day) */
        BROADCAST_TIME_EVENT_REPORT_YEAR = 2,    /**< Report Year event (year) */
        BROADCAST_TIME_EVENT_REPORT_RATE = 3,    /**< Report Rate event (clock rate) */
        BROADCAST_TIME_EVENT_SET_TIME = 4,       /**< Set Time event (hour/minute) */
        BROADCAST_TIME_EVENT_SET_DATE = 5,       /**< Set Date event (month/day) */
        BROADCAST_TIME_EVENT_SET_YEAR = 6,       /**< Set Year event (year) */
        BROADCAST_TIME_EVENT_SET_RATE = 7,       /**< Set Rate event (clock rate) */
        BROADCAST_TIME_EVENT_QUERY = 8,          /**< Query event (request sync) */
        BROADCAST_TIME_EVENT_STOP = 9,           /**< Stop event (stop clock) */
        BROADCAST_TIME_EVENT_START = 10,         /**< Start event (start clock) */
        BROADCAST_TIME_EVENT_DATE_ROLLOVER = 11, /**< Date Rollover event (midnight) */
        BROADCAST_TIME_EVENT_UNKNOWN = 255       /**< Unknown/invalid event type */

    } broadcast_time_event_type_enum;

    /**
     * @defgroup payload_buffer_types Message Payload Buffer Type Definitions
     * @brief Array types for different message payload sizes
     *
     * @details These typedefs define fixed-size byte arrays for each message
     * payload category. The buffer store allocates arrays of these types.
     *
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
        * @defgroup payload_pool_types Payload Buffer Pool Array Definitions
        * @brief Arrays of payload buffers organized by type
        *
        * @details These define the actual storage pools for message payloads.
        * The buffer store contains one instance of each type.
        *
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

        /**
        * @brief Generic payload pointer type
        *
        * @details Minimum 1-byte array type for generic payload pointer.
        * Actual payload is accessed through payload_type and proper casting.
        */
    typedef uint8_t openlcb_payload_t[1];

        /**
        * @brief Event ID type (64-bit unsigned integer)
        *
        * @details Event IDs uniquely identify events in the Producer/Consumer model.
        * Format: 8 bytes, most significant byte first.
        *
        * Event ID structure (recommendation, not enforced):
        * - Bytes 0-5: Derived from Node ID of defining node
        * - Bytes 6-7: Event-specific identifier within node
        *
        * @see OpenLCB Event Identifiers Standard
        */
    typedef uint64_t event_id_t;

        /**
        * @struct event_id_struct_t
        * @brief Event ID with status information
        *
        * @details Pairs an Event ID with its current status (SET, CLEAR, UNKNOWN).
        * Used in producer/consumer event lists to track event states.
        *
        * @see event_id_consumer_list_t
        * @see event_id_producer_list_t
        */
    typedef struct {
        event_id_t event;           /**< 64-bit Event ID */
        event_status_enum status;   /**< Current event status */

    } event_id_struct_t;


    typedef struct {
        event_id_t start_base;              /**< Starting Event ID of the range (bottom 16 bits must be 00.00) */
        event_range_count_enum event_count; /**< Number of consecutive Event IDs in the range (max 65536) */

    } event_id_range_t;

        /**
        * @brief Node ID type (64-bit unsigned integer)
        *
        * @details Uniquely identifies a node on the OpenLCB network.
        * Format: 48-bit value stored in 64-bit type (upper 16 bits unused).
        *
        * Transmission order: Most significant byte first
        * Valid range: Must contain at least one '1' bit
        *
        * @see OpenLCB Unique Identifiers Standard
        * @see NULL_NODE_ID
        */
    typedef uint64_t node_id_t;

        /**
        * @brief Event payload data buffer
        *
        * @details Buffer for event payload data in Events with Payload messages.
        * Maximum size is LEN_MESSAGE_BYTES_SNIP (256 bytes).
        *
        * @see MTI_PC_EVENT_REPORT_WITH_PAYLOAD
        */
    typedef uint8_t event_payload_t[LEN_EVENT_PAYLOAD];

        /**
        * @brief Configuration memory operation buffer
        *
        * @details Buffer for read/write operations in Configuration Memory Protocol.
        * Size matches maximum datagram payload (64 bytes).
        *
        * @see CONFIG_MEM_READ_SPACE_FD
        * @see CONFIG_MEM_WRITE_SPACE_FD
        */
    typedef uint8_t configuration_memory_buffer_t[LEN_DATAGRAM_MAX_PAYLOAD];

        /**
         * @struct broadcast_time_t
        * @brief Time representation for Broadcast Time Protocol
        *
        * @details Stores hour and minute for fast clock synchronization.
        *
        * @see broadcast_clock_state_t.time
        */
    typedef struct
    {
        uint8_t hour;   /**< Hour 0-23 */
        uint8_t minute; /**< Minute 0-59 */
        bool valid;     /**< Time data is valid (true if valid, false if invalid) */    

    } broadcast_time_t;

        /**
         * @struct broadcast_date_t
         * @brief Date representation for Broadcast Time Protocol
         *
         * @details Stores month and day for fast clock synchronization.
         *
         * @see broadcast_clock_state_t.date
         */
    typedef struct
    {
        uint8_t month; /**< Month 1-12 */
        uint8_t day;   /**< Day 1-31 */
        bool valid;    /**< Date data is valid (true if valid, false if invalid) */

    } broadcast_date_t;

        /**
         * @struct broadcast_year_t
         * @brief Year representation for Broadcast Time Protocol
        *
         * @details Stores year for fast clock synchronization.
         *
         * @see broadcast_clock_state_t.year
         */
    typedef struct
    {

        uint16_t year; /**< Year 0-4095 AD */
        bool valid;    /**< Year data is valid (true if valid, false if invalid) */ 

    } broadcast_year_t;

        /**
         * @struct broadcast_rate_t
         * @brief Clock rate for fast/slow time simulation
         *
         * @details 12-bit signed fixed point value with 2 fractional bits.
         * Format: rrrrrrrrrr.rr
         * - Range: -512.00 to +511.75 in 0.25 increments
         * - Examples: 0x0004 = 1.00 (real-time), 0x0010 = 4.00 (4x speed)
         * - Negative values in 2's complement (e.g., 0xFFFC = -1.00)
         *
         * @see broadcast_clock_state_t.rate
        */
    typedef struct
    {

        int16_t rate; /**< Clock rate (12-bit signed fixed point, 2 fractional bits) */
        bool valid;   /**< Rate data is valid (true if valid, false if invalid) */  

    } broadcast_rate_t;

         /**
         * @struct broadcast_clock_state_t
         * @brief Complete clock state for Broadcast Time Protocol
         *
         * @details Contains all time/date/rate information and validity flags for a
         * broadcast time clock. Updated as time events are received.
         *
         * @see openlcb_node_t.clock_state
         * @see ProtocolBroadcastTime_handle_time_event
         */
    typedef struct
    {
        uint64_t clock_id;     /**< Clock identifier (upper 6 bytes of event IDs) */
        broadcast_time_t time; /**< Current time */
        broadcast_date_t date; /**< Current date */
        broadcast_year_t year; /**< Current year */
        broadcast_rate_t rate; /**< Clock rate */
        uint8_t is_running;    /**< Clock running state: 1=running, 0=stopped */
        uint32_t ms_accumulator; /**< Internal: accumulated ms toward next minute (fixed-point scale) */

    } broadcast_clock_state_t;


        /**
        * @struct openlcb_msg_state_t
        * @brief Message state flags
        *
        * @details Tracks the allocation and processing state of a message buffer.
        *
        * **State Transitions:**
        * - Unallocated: allocated=0, inprocess=0
        * - Allocated, complete: allocated=1, inprocess=0
        * - Allocated, assembling: allocated=1, inprocess=1
        *
        * @see openlcb_msg_t.state
        */
    typedef struct {
        bool allocated : 1;     /**< Message buffer is allocated and in use */
        bool inprocess : 1;     /**< Multi-frame message being assembled, not yet complete */
    } openlcb_msg_state_t;

        /**
        * @struct openlcb_msg_t
        * @brief OpenLCB message structure
        *
        * @details Core message structure containing all information for an OpenLCB message.
        *
        * **Message Lifecycle:**
        * -# Allocated from buffer store (allocate sets state.allocated=1)
        * -# Populated with MTI, addresses, and payload data
        * -# Queued in FIFO or list
        * -# Processed by state machine
        * -# Reference count decremented (freed when reaches 0)
        *
        * **Multi-frame Assembly:**
        * For messages spanning multiple CAN frames (datagrams, SNIP), the first
        * frame creates the message with state.inprocess=1. Subsequent frames
        * append data. Final frame clears inprocess flag.
        *
        * **Reference Counting:**
        * Messages can be referenced by multiple subsystems (e.g., TX queue and
        * retry buffer). Reference count tracks number of active references.
        * Buffer is returned to pool when count reaches zero.
        *
        * @warning Do not access payload directly; use payload pointer with proper type cast
        * @warning Reference count must be managed correctly to prevent leaks
        *
        * @see OpenLcbBufferStore_allocate_buffer
        * @see OpenLcbBufferStore_free_buffer
        * @see OpenLcbBufferStore_inc_reference_count
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
        openlcb_payload_t *payload;     /**< Pointer to payload buffer (cast to appropriate type) */
        uint8_t timerticks;             /**< Timer tick counter for timeouts */
        uint8_t reference_count;        /**< Number of active references to this message */
    } openlcb_msg_t;

        /**
        * @brief Array of OpenLCB message structures
        *
        * @details The message buffer store contains one array with all message
        * structures (BASIC + DATAGRAM + SNIP + STREAM count).
        */
    typedef openlcb_msg_t openlcb_msg_array_t[LEN_MESSAGE_BUFFER];

        /**
        * @struct message_buffer_t
        * @brief Complete message buffer storage
        *
        * @details Master structure containing all message structures and segregated
        * payload buffers. One instance of this exists in the buffer store.
        *
        * **Buffer Organization:**
        * - messages[]: Array of all message structures
        * - basic[]: BASIC payload buffers (16 bytes each)
        * - datagram[]: DATAGRAM payload buffers (72 bytes each)
        * - snip[]: SNIP payload buffers (256 bytes each)
        * - stream[]: STREAM payload buffers (512 bytes each)
        *
        * **Allocation Strategy:**
        * When allocating a message, the store finds an unallocated message structure
        * and links it to an appropriate payload buffer based on requested type.
        *
        * @see OpenLcbBufferStore_initialize
        * @see OpenLcbBufferStore_allocate_buffer
        */
    typedef struct {
        openlcb_msg_array_t messages;               /**< Array of message structures */
        openlcb_basic_data_buffer_t basic;          /**< Pool of BASIC payload buffers */
        openlcb_datagram_data_buffer_t datagram;    /**< Pool of DATAGRAM payload buffers */
        openlcb_snip_data_buffer_t snip;            /**< Pool of SNIP payload buffers */
        openlcb_stream_data_buffer_t stream;        /**< Pool of STREAM payload buffers */
    } message_buffer_t;

        /**
        * @struct user_snip_struct_t
        * @brief SNIP (Simple Node Information Protocol) data structure
        *
        * @details Contains all SNIP strings for a node, both manufacturer-provided
        * (read-only) and user-assignable fields.
        *
        * **Manufacturer Fields (read-only from ACDI Manufacturer space 0xFC):**
        * - mfg_version: Always 1
        * - name: Manufacturer name
        * - model: Model name/number
        * - hardware_version: Hardware version string
        * - software_version: Software version string
        *
        * **User Fields (writable in ACDI User space 0xFB):**
        * - user_version: Always 1
        * - User name and description stored separately in node configuration
        *
        * @note String lengths include null terminator
        * @note C compiler initializers handle null terminators inconsistently; some include it, some don't
        *
        * @see MTI_SIMPLE_NODE_INFO_REQUEST
        * @see MTI_SIMPLE_NODE_INFO_REPLY
        * @see CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS
        */
    typedef struct {
        uint8_t mfg_version;                                    /**< Manufacturer SNIP version (always 1) */
        char name[LEN_SNIP_NAME_BUFFER];                        /**< Manufacturer name */
        char model[LEN_SNIP_MODEL_BUFFER];                      /**< Model name */
        char hardware_version[LEN_SNIP_HARDWARE_VERSION_BUFFER];/**< Hardware version */
        char software_version[LEN_SNIP_SOFTWARE_VERSION_BUFFER];/**< Software version */
        uint8_t user_version;                                   /**< User SNIP version (always 1) */

    } user_snip_struct_t;

        /**
        * @struct user_configuration_options
        * @brief Configuration memory capabilities and options
        *
        * @details Returned by Get Configuration Options command to indicate
        * which optional memory configuration features are supported.
        *
        * **Capability Flags:**
        * - Write under mask: Selective bit modification
        * - Unaligned reads/writes: Non-word-boundary access
        * - ACDI access: Manufacturer and user spaces readable/writable
        * - Stream support: Bulk read/write via streams
        *
        * **Address Space Range:**
        * - high_address_space: Highest numbered supported space
        * - low_address_space: Lowest numbered supported space
        *
        * @see CONFIG_MEM_OPTIONS_CMD
        * @see CONFIG_OPTIONS_COMMANDS_WRITE_UNDER_MASK
        */
    typedef struct {
        bool write_under_mask_supported : 1;                /**< Write Under Mask command supported */
        bool unaligned_reads_supported : 1;                 /**< Unaligned read operations supported */
        bool unaligned_writes_supported : 1;                /**< Unaligned write operations supported */
        bool read_from_manufacturer_space_0xfc_supported : 1;/**< Can read ACDI Manufacturer space */
        bool read_from_user_space_0xfb_supported : 1;       /**< Can read ACDI User space */
        bool write_to_user_space_0xfb_supported : 1;        /**< Can write ACDI User space */
        bool stream_read_write_supported : 1;               /**< Stream read/write operations supported */
        uint8_t high_address_space;                         /**< Highest supported address space number */
        uint8_t low_address_space;                          /**< Lowest supported address space number */
        char description[LEN_CONFIG_MEM_OPTIONS_DESCRIPTION];/**< Human-readable description */
    } user_configuration_options;

        /**
        * @struct user_address_space_info_t
        * @brief Address space information and properties
        *
        * @details Describes characteristics of a configuration memory address space.
        * Returned by Get Address Space Information command.
        *
        * **Space Properties:**
        * - present: Space exists and is accessible
        * - read_only: Space cannot be written
        * - low_address_valid: Non-zero starting address (space doesn't start at 0)
        *
        * **Address Range:**
        * - highest_address: Last valid address in space
        * - low_address: Starting address (if low_address_valid set)
        *
        * @see CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD
        * @see CONFIG_OPTIONS_SPACE_INFO_FLAG_READ_ONLY
        */
    typedef struct {
        bool present : 1;               /**< Address space exists */
        bool read_only : 1;             /**< Space is read-only */
        bool low_address_valid : 1;     /**< low_address field is valid */
        uint8_t address_space;          /**< Address space identifier (0x00-0xFF) */
        uint32_t highest_address;       /**< Highest address in space */
        uint32_t low_address;           /**< Starting address (if low_address_valid) */
        char description[LEN_CONFIG_MEM_ADDRESS_SPACE_DESCRIPTION]; /**< Space description */
    } user_address_space_info_t;

        /**
        * @struct node_parameters_t
        * @brief Complete node configuration and parameters
        *
        * @details Contains all configuration data for an OpenLCB node. This structure
        * is typically const and stored in program memory (flash).
        *
        * **Node Identification:**
        * - snip: SNIP strings (manufacturer and user info)
        * - protocol_support: 48-bit Protocol Support Indicator
        *
        * **Event Configuration:**
        * - consumer_count_autocreate: Number of consumer events to auto-create
        * - producer_count_autocreate: Number of producer events to auto-create
        *
        * **Configuration Definitions:**
        * - cdi[]: Configuration Description Information (XML)
        * - fdi[]: Function Description Information (for traction nodes)
        *
        * **Memory Spaces:**
        * - configuration_options: Supported memory operations
        * - address_space_*: Information for each supported address space
        *
        * @note Structure is usually const and pointed to by openlcb_node_t.parameters
        * @note CDI and FDI buffers are large; consider using program memory
        *
        * @see openlcb_node_t.parameters
        * @see OpenLcbNode_allocate
        */
    typedef struct {
        user_snip_struct_t snip;                                        /**< SNIP identification strings */
        uint64_t protocol_support;                                      /**< Protocol Support Indicator bits */
        uint8_t consumer_count_autocreate;                              /**< Auto-create this many consumer events */
        uint8_t producer_count_autocreate;                              /**< Auto-create this many producer events */
        uint8_t cdi[USER_DEFINED_CDI_LENGTH];                           /**< CDI XML data */
        uint8_t fdi[USER_DEFINED_FDI_LENGTH];                           /**< FDI data (traction nodes) */
        user_address_space_info_t address_space_configuration_definition;/**< Space 0xFF info */
        user_address_space_info_t address_space_all;                    /**< Space 0xFE info */
        user_address_space_info_t address_space_config_memory;          /**< Space 0xFD info */
        user_address_space_info_t address_space_acdi_manufacturer;      /**< Space 0xFC info */
        user_address_space_info_t address_space_acdi_user;              /**< Space 0xFB info */
        user_address_space_info_t address_space_traction_function_definition_info; /**< Space 0xFA info */
        user_address_space_info_t address_space_traction_function_config_memory;   /**< Space 0xF9 info */
        user_configuration_options configuration_options;               /**< Memory operation capabilities */
        user_address_space_info_t address_space_firmware;               /**< Space 0xEF info */

    } node_parameters_t;

        /**
        * @struct event_id_enum_t
        * @brief Event ID enumeration state
        *
        * @details Tracks enumeration state when iterating through event lists.
        * Used by protocol handlers when responding to Event Identify messages.
        *
        * **Usage Pattern:**
        * -# Set running=1, enum_index=0 to start enumeration
        * -# Process event at enum_index
        * -# Increment enum_index
        * -# When finished: Set running=0
        *
        * @warning Always, always, always reset these to false when you have finished processing a message
        *
        * @see event_id_consumer_list_t.enumerator
        * @see event_id_producer_list_t.enumerator
        */
    typedef struct {
        bool running : 1;       /**< Enumeration is in progress */
        uint8_t enum_index;     /**< Current position in event list */
        uint8_t range_enum_index; /**< Current position in range list */
    } event_id_enum_t;

        /**
        * @struct event_id_consumer_list_t
        * @brief List of events consumed by a node
        *
        * @details Maintains the array of Event IDs that this node consumes.
        *
        * **List Management:**
        * - count: Number of valid entries in list
        * - list[]: Array of event IDs with status
        * - enumerator: State for iterating through list
        *
        * @see MTI_CONSUMER_IDENTIFY
        * @see openlcb_node_t.consumers
        */
    typedef struct {
        uint16_t count;                                     /**< Number of events in list */
        event_id_struct_t list[USER_DEFINED_CONSUMER_COUNT];/**< Array of consumed Event IDs */
        uint16_t range_count;                               /**< Number of ranges in list */
        event_id_range_t range_list[USER_DEFINED_CONSUMER_RANGE_COUNT]; /**< Array of consumed Event ID ranges */
        event_id_enum_t enumerator;                                     /**< Enumeration state */

    } event_id_consumer_list_t;

        /**
        * @struct event_id_producer_list_t
        * @brief List of events produced by a node
        *
        * @details Maintains the array of Event IDs that this node produces.
        *
        * **List Management:**
        * - count: Number of valid entries in list
        * - list[]: Array of event IDs with status
        * - enumerator: State for iterating through list
        *
        * @see MTI_PRODUCER_IDENTIFY
        * @see openlcb_node_t.producers
        */
    typedef struct {
        uint16_t count;                                      /**< Number of events in list */
        event_id_struct_t list[USER_DEFINED_PRODUCER_COUNT]; /**< Array of produced Event IDs */
        uint16_t range_count;                                /**< Number of ranges in list */                                 
        event_id_range_t range_list[USER_DEFINED_PRODUCER_RANGE_COUNT]; /**< Array of produced Event ID ranges */
        event_id_enum_t enumerator;                                     /**< Enumeration state */

    } event_id_producer_list_t;

        /**
        * @struct openlcb_node_state_t
        * @brief Node state flags
        *
        * @details Bit-packed state flags for an OpenLCB node.
        *
        * **Login State:**
        * - run_state: Current position in login state machine (0-13)
        * - allocated: Node structure is in use
        * - permitted: CAN alias allocated, node can send messages
        * - initialized: Node login complete, fully operational
        *
        * **Error Conditions:**
        * - duplicate_id_detected: Another node has same Node ID
        *
        * **Protocol State:**
        * - openlcb_datagram_ack_sent: Datagram ACK sent, awaiting reply
        * - resend_datagram: Retry last datagram instead of pulling from FIFO
        * - firmware_upgrade_active: Node in firmware upgrade mode
        *
        * @see RUNSTATE_INIT
        * @see RUNSTATE_RUN
        * @see openlcb_node_t.state
        */
    typedef struct {
        uint8_t run_state : 5;                  /**< Login state machine state (0-31) */
        bool allocated : 1;                     /**< Node is allocated */
        bool permitted : 1;                     /**< CAN alias allocated, permitted to send */
        bool initialized : 1;                   /**< Node fully initialized and operational */
        bool duplicate_id_detected : 1;         /**< Duplicate Node ID conflict detected */
        bool openlcb_datagram_ack_sent : 1;     /**< Datagram ACK sent, awaiting actual reply */
        bool resend_datagram : 1;               /**< Resend last datagram (retry logic) */
        bool firmware_upgrade_active : 1;       /**< Firmware upgrade in progress */
    } openlcb_node_state_t;

        /**
        * @struct openlcb_node_t
        * @brief OpenLCB virtual node structure
        *
        * @details Represents a single virtual node on the OpenLCB network.
        *
        * **Node Identity:**
        * - id: 48-bit globally unique Node ID
        * - alias: 12-bit CAN bus alias for this session
        * - seed: 48-bit seed for alias generation
        *
        * **Node State:**
        * - state: State flags (allocated, permitted, initialized, etc.)
        * - timerticks: 100ms timer for CAN login timing
        *
        * **Events:**
        * - consumers: List of consumed Event IDs
        * - producers: List of produced Event IDs
        *
        * **Configuration:**
        * - parameters: Pointer to const configuration (SNIP, CDI, etc.)
        * - index: Position in node array (for config memory offsets)
        *
        * **Protocol State:**
        * - owner_node: Node ID that currently has this node locked
        * - last_received_datagram: Saved for reply processing
        *
        * @warning Node structures cannot be deallocated once allocated
        * @warning parameters pointer must remain valid for node lifetime
        *
        * @see OpenLcbNode_allocate
        * @see openlcb_nodes_t
        */
    typedef struct {
        openlcb_node_state_t state;             /**< Node state flags */
        uint64_t id;                            /**< 48-bit Node ID */
        uint16_t alias;                         /**< 12-bit CAN alias */
        uint64_t seed;                          /**< Seed for alias generation */
        event_id_consumer_list_t consumers;     /**< Consumed Event ID list */
        event_id_producer_list_t producers;     /**< Produced Event ID list */
        const node_parameters_t *parameters;    /**< Pointer to configuration parameters */
        uint16_t timerticks;                    /**< 100ms timer tick counter */
        uint64_t owner_node;                    /**< Node ID that has locked this node */
        openlcb_msg_t *last_received_datagram;  /**< Last datagram received (for replies) */
        uint8_t index;                          /**< Index in node array */
    } openlcb_node_t;

        /**
        * @struct openlcb_nodes_t
        * @brief Collection of all virtual nodes
        *
        * @details Master structure containing all allocated nodes.
        *
        * **Node Management:**
        * - node[]: Array of node structures
        * - count: Number of allocated nodes (never decreases)
        *
        * @note Nodes cannot be deallocated; count only increases
        *
        * @see OpenLcbNode_allocate
        */
    typedef struct {
        openlcb_node_t node[USER_DEFINED_NODE_BUFFER_DEPTH];   /**< Array of nodes */
        uint16_t count;                                         /**< Number of allocated nodes */

    } openlcb_nodes_t;

        /**
        * @struct openlcb_statemachine_worker_t
        * @brief State machine working buffers
        *
        * @details Provides working space for state machine message processing.
        *
        * **Worker Buffers:**
        * - worker: Temporary message structure
        * - worker_buffer: Temporary payload buffer (STREAM size)
        * - active_msg: Pointer to message currently being processed
        *
        * @see openlcb_statemachine_info_t
        */
    typedef struct {
        openlcb_msg_t worker;               /**< Worker message structure */
        payload_stream_t worker_buffer;     /**< Worker payload buffer */
        openlcb_msg_t *active_msg;          /**< Currently active message */
    } openlcb_statemachine_worker_t;

        /**
        * @brief Callback function type with no parameters
        *
        * @details Used for simple callbacks like timer ticks.
        *
        * @see interface_openlcb_node_t.on_100ms_timer_tick
        */
    typedef void (*parameterless_callback_t)(void);

        /**
        * @struct openlcb_stream_message_t
        * @brief Stream message with embedded payload
        *
        * @details Message structure with inline STREAM-sized payload buffer.
        * Used for outgoing stream messages.
        */
    typedef struct {
        openlcb_msg_t openlcb_msg;          /**< Message structure */
        payload_stream_t openlcb_payload;   /**< Inline payload buffer */

    } openlcb_stream_message_t;

        /**
        * @struct openlcb_outgoing_stream_msg_info_t
        * @brief Outgoing stream message information
        *
        * @details Context for outgoing stream message in state machine.
        *
        * **Control Flags:**
        * - valid: Message is valid and ready
        * - enumerate: Continue enumeration after sending
        *
        * @see openlcb_statemachine_info_t.outgoing_msg_info
        */
    typedef struct {
        openlcb_msg_t *msg_ptr;             /**< Pointer to message (NULL or valid buffer) */
        uint8_t valid : 1;                  /**< Message is valid */
        uint8_t enumerate : 1;              /**< Continue enumeration */
        openlcb_stream_message_t openlcb_msg;/**< Message with inline payload */

    } openlcb_outgoing_stream_msg_info_t;

        /**
        * @struct openlcb_incoming_msg_info_t
        * @brief Incoming message information
        *
        * @details Context for incoming message being processed.
        *
        * @see openlcb_statemachine_info_t.incoming_msg_info
        */
    typedef struct {
        openlcb_msg_t *msg_ptr;             /**< Pointer to incoming message */
        uint8_t enumerate : 1;              /**< Enumeration flag */

    } openlcb_incoming_msg_info_t;

        /**
        * @struct openlcb_statemachine_info_t
        * @brief State machine context information
        *
        * @details Complete context passed to protocol handler functions.
        * Contains node, incoming message, and outgoing message information.
        *
        * @see interface_openlcb_main_statemachine_t
        */
    typedef struct {
        openlcb_node_t *openlcb_node;                       /**< Node being processed */
        openlcb_incoming_msg_info_t incoming_msg_info;      /**< Incoming message context */
        openlcb_outgoing_stream_msg_info_t outgoing_msg_info;/**< Outgoing message context */

    } openlcb_statemachine_info_t;

        /**
        * @struct openlcb_basic_message_t
        * @brief Basic message with embedded payload
        *
        * @details Message structure with inline BASIC-sized payload buffer.
        * Used for login state machine messages.
        */
    typedef struct {
        openlcb_msg_t openlcb_msg;          /**< Message structure */
        payload_basic_t openlcb_payload;    /**< Inline BASIC payload */

    } openlcb_basic_message_t;

        /**
        * @struct openlcb_outgoing_basic_msg_info_t
        * @brief Outgoing basic message information
        *
        * @details Context for outgoing basic message in login state machine.
        *
        * @see openlcb_login_statemachine_info_t.outgoing_msg_info
        */
    typedef struct {
        openlcb_msg_t *msg_ptr;             /**< Pointer to message */
        uint8_t valid : 1;                  /**< Message is valid */
        uint8_t enumerate : 1;              /**< Enumeration flag */
        openlcb_basic_message_t openlcb_msg;/**< Message with inline payload */

    } openlcb_outgoing_basic_msg_info_t;

        /**
        * @struct openlcb_login_statemachine_info_t
        * @brief Login state machine context
        *
        * @details Context for CAN login state machine processing.
        *
        * @see CanLoginStateMachine_run
        */
    typedef struct {
        openlcb_node_t *openlcb_node;                       /**< Node being logged in */
        openlcb_outgoing_basic_msg_info_t outgoing_msg_info;/**< Outgoing message context */

    } openlcb_login_statemachine_info_t;

        /**
        * @brief Forward declaration for configuration memory operations callback
        */
    struct config_mem_operations_request_info_TAG;

        /**
        * @brief Configuration memory operations callback function type
        *
        * @details Callback for handling configuration memory operations.
        * Application implements this to process Get Options and Get Address Space Info.
        *
        * @param statemachine_info State machine context
        * @param config_mem_operations_request_info Operation request details
        */
    typedef void (*operations_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_operations_request_info_TAG *config_mem_operations_request_info);

        /**
        * @struct config_mem_operations_request_info_t
        * @brief Configuration memory operations request information
        *
        * @details Information for processing configuration memory operations
        * (Get Options, Get Address Space Info).
        *
        * @see CONFIG_MEM_OPTIONS_CMD
        * @see CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD
        */
    typedef struct config_mem_operations_request_info_TAG {
        const user_address_space_info_t *space_info;        /**< Address space information */
        operations_config_mem_space_func_t operations_func; /**< Callback function */

    } config_mem_operations_request_info_t;

        /**
        * @brief Forward declaration for configuration memory read callback
        */
    struct config_mem_read_request_info_TAG;

        /**
        * @brief Configuration memory read callback function type
        *
        * @details Callback for handling configuration memory read requests.
        * Application implements this to provide data from address spaces.
        *
        * @param statemachine_info State machine context
        * @param config_mem_read_request_info Read request details
        */
    typedef void (*read_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_read_request_info_TAG *config_mem_read_request_info);

        /**
        * @struct config_mem_read_request_info_t
        * @brief Configuration memory read request information
        *
        * @details Complete information for processing a memory read request.
        *
        * **Request Parameters:**
        * - encoding: Where address space ID is located in command
        * - address: Starting address to read
        * - bytes: Number of bytes requested
        * - data_start: Offset in reply payload to insert data
        *
        * **Response:**
        * Application fills data into outgoing message payload at data_start offset.
        *
        * @see CONFIG_MEM_READ_SPACE_FD
        */
    typedef struct config_mem_read_request_info_TAG {
        space_encoding_enum encoding;                   /**< Address space encoding method */
        uint32_t address;                               /**< Starting address */
        uint16_t bytes;                                 /**< Number of bytes to read */
        uint16_t data_start;                            /**< Offset into the payload to insert the data being requested */
        const user_address_space_info_t *space_info;    /**< Address space information */
        read_config_mem_space_func_t read_space_func;   /**< Read callback function */

    } config_mem_read_request_info_t;

        /**
        * @brief Forward declaration for configuration memory write callback
        */
    struct config_mem_write_request_info_TAG;

        /**
        * @brief Configuration memory write callback function type
        *
        * @details Callback for handling configuration memory write requests.
        * Application implements this to store data to address spaces.
        *
        * @param statemachine_info State machine context
        * @param config_mem_write_request_info Write request details
        */
    typedef void (*write_config_mem_space_func_t)(openlcb_statemachine_info_t *statemachine_info, struct config_mem_write_request_info_TAG *config_mem_write_request_info);

        /**
        * @struct config_mem_write_request_info_t
        * @brief Configuration memory write request information
        *
        * @details Complete information for processing a memory write request.
        *
        * **Request Parameters:**
        * - encoding: Where address space ID is located in command
        * - address: Starting address to write
        * - bytes: Number of bytes to write
        * - write_buffer: Buffer containing data to write
        * - data_start: Offset in write_buffer where data begins
        *
        * **Processing:**
        * Application writes bytes from write_buffer to specified address space.
        *
        * @see CONFIG_MEM_WRITE_SPACE_FD
        */
    typedef struct config_mem_write_request_info_TAG {
        space_encoding_enum encoding;                   /**< Address space encoding method */
        uint32_t address;                               /**< Starting address */
        uint16_t bytes;                                 /**< Number of bytes to write */
        configuration_memory_buffer_t *write_buffer;    /**< Buffer with data to write */
        uint16_t data_start;                            /**< Offset into the payload to insert the data being requested */
        const user_address_space_info_t *space_info;    /**< Address space information */
        write_config_mem_space_func_t write_space_func; /**< Write callback function */

    } config_mem_write_request_info_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_TYPES__ */
