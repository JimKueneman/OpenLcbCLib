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
 * @file openlcb_buffer_store.c
 * @brief Implementation of the core buffer store for OpenLCB message allocation
 *
 * @details This module implements a pre-allocated memory pool for OpenLCB messages
 * with support for four different payload sizes. The implementation uses segregated
 * buffer pools to avoid fragmentation and ensure predictable allocation behavior.
 *
 * Memory layout:
 * - Single static message_buffer_t structure containing all pools
 * - Messages array with pointers to payload buffers
 * - Four separate payload arrays (basic, datagram, snip, stream)
 *
 * Key algorithms:
 * - Linear search allocation within segregated pools
 * - Reference counting for shared buffer management
 * - Telemetry tracking for peak usage monitoring
 *
 * Thread safety:
 * - NOT thread-safe - designed for single-threaded use
 * - Must be externally synchronized if used in multi-threaded environment
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

#include "openlcb_buffer_store.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // printf

#include "openlcb_types.h"
#include "openlcb_utilities.h"

/** @brief Main buffer pool containing all message structures and payload buffers */
static message_buffer_t _message_buffer;

/** @brief Current number of allocated BASIC messages */
static uint16_t _buffer_store_basic_messages_allocated = 0;

/** @brief Current number of allocated DATAGRAM messages */
static uint16_t _buffer_store_datagram_messages_allocated = 0;

/** @brief Current number of allocated SNIP messages */
static uint16_t _buffer_store_snip_messages_allocated = 0;

/** @brief Current number of allocated STREAM messages */
static uint16_t _buffer_store_stream_messages_allocated = 0;

/** @brief Peak number of BASIC messages allocated simultaneously */
static uint16_t _buffer_store_basic_max_messages_allocated = 0;

/** @brief Peak number of DATAGRAM messages allocated simultaneously */
static uint16_t _buffer_store_datagram_max_messages_allocated = 0;

/** @brief Peak number of SNIP messages allocated simultaneously */
static uint16_t _buffer_store_snip_max_messages_allocated = 0;

/** @brief Peak number of STREAM messages allocated simultaneously */
static uint16_t _buffer_store_stream_max_messages_allocated = 0;


    /**
     * @brief Initializes the OpenLcb Buffer Store
     *
     * @details Algorithm:
     * Sets up the pre-allocated message pool by iterating through all message slots
     * and linking each to its appropriate payload buffer based on pool segmentation.
     * -# Iterate through all LEN_MESSAGE_BUFFER message slots
     * -# Call OpenLcbUtilities_clear_openlcb_message() to zero each message structure
     * -# Determine payload type based on index ranges:
     *    - [0 to BASIC_DEPTH-1] → BASIC type, link to basic[] array
     *    - [BASIC_DEPTH to BASIC+DATAGRAM-1] → DATAGRAM type, link to datagram[] array
     *    - [BASIC+DATAGRAM to BASIC+DATAGRAM+SNIP-1] → SNIP type, link to snip[] array
     *    - [Remaining] → STREAM type, link to stream[] array
     * -# Set payload_type field in each message
     * -# Calculate appropriate payload array index and link payload pointer
     * -# Reset all allocation counters to zero
     * -# Reset all peak usage counters to zero
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any buffer allocation operations
     * - Must be called before OpenLcbBufferFifo_initialize() and OpenLcbBufferList_initialize()
     *
     * @warning MUST be called exactly once during application initialization
     * @warning Calling multiple times will reset all allocation state
     * @warning NOT thread-safe
     *
     * @attention Must be called during single-threaded initialization only
     * @attention Call before any buffer allocation operations
     *
     * @see OpenLcbUtilities_clear_openlcb_message - Clears individual message structures
     * @see OpenLcbBufferStore_allocate_buffer - Uses the initialized pool
     */
void OpenLcbBufferStore_initialize(void) {

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++) {

        OpenLcbUtilities_clear_openlcb_message(&_message_buffer.messages[i]);

        if (i < USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = BASIC;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.basic[i];

        } else if (i < USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = DATAGRAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.datagram[i - USER_DEFINED_BASIC_BUFFER_DEPTH];

        } else if (i < USER_DEFINED_SNIP_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_BASIC_BUFFER_DEPTH) {

            _message_buffer.messages[i].payload_type = SNIP;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.snip[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH)];

        } else {

            _message_buffer.messages[i].payload_type = STREAM;
            _message_buffer.messages[i].payload = (openlcb_payload_t *) &_message_buffer.stream[i - (USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH)];

        }
    }

    _buffer_store_basic_messages_allocated = 0;
    _buffer_store_datagram_messages_allocated = 0;
    _buffer_store_snip_messages_allocated = 0;
    _buffer_store_stream_messages_allocated = 0;

    _buffer_store_basic_max_messages_allocated = 0;
    _buffer_store_datagram_max_messages_allocated = 0;
    _buffer_store_snip_max_messages_allocated = 0;
    _buffer_store_stream_max_messages_allocated = 0;

}

    /**
     * @brief Updates buffer allocation telemetry counters
     *
     * @details Algorithm:
     * Updates the current and peak allocation counters for the specified payload type.
     * -# Switch on payload_type parameter
     * -# Increment the appropriate current allocation counter
     * -# If current count exceeds max count, increment max counter
     * -# Break to prevent fall-through
     *
     * Use cases:
     * - Called internally by OpenLcbBufferStore_allocate_buffer()
     * - Tracks buffer usage statistics
     * - Updates peak usage for capacity planning
     *
     * @verbatim
     * @param payload_type Type of buffer being allocated (BASIC, DATAGRAM, SNIP, STREAM)
     * @endverbatim
     *
     * @note This is an internal static function
     * @note Only increments max counter when current exceeds previous max
     *
     * @see OpenLcbBufferStore_allocate_buffer - Calls this function on successful allocation
     */
static void _update_buffer_telemetry(payload_type_enum payload_type) {

    switch (payload_type) {

        case BASIC:

            _buffer_store_basic_messages_allocated++;
            if (_buffer_store_basic_messages_allocated > _buffer_store_basic_max_messages_allocated) {

                _buffer_store_basic_max_messages_allocated = _buffer_store_basic_messages_allocated;

                break;
            }

            break;

        case DATAGRAM:

            _buffer_store_datagram_messages_allocated++;
            if (_buffer_store_datagram_messages_allocated > _buffer_store_datagram_max_messages_allocated) {

                _buffer_store_datagram_max_messages_allocated = _buffer_store_datagram_messages_allocated;

                break;
            }

            break;

        case SNIP:

            _buffer_store_snip_messages_allocated++;
            if (_buffer_store_snip_messages_allocated > _buffer_store_snip_max_messages_allocated) {

                _buffer_store_snip_max_messages_allocated = _buffer_store_snip_messages_allocated;

                break;
            }

            break;

        case STREAM:

            _buffer_store_stream_messages_allocated++;
            if (_buffer_store_stream_messages_allocated > _buffer_store_stream_max_messages_allocated) {

                _buffer_store_stream_max_messages_allocated = _buffer_store_stream_messages_allocated;

                break;
            }

            break;

        default:

            break;

    }
}

    /**
     * @brief Allocates a new buffer of the specified payload type
     *
     * @details Algorithm:
     * Searches the appropriate pool segment for an unallocated buffer and returns it.
     * -# Calculate offset_start and offset_end based on payload_type:
     *    - BASIC: offset_start=0, offset_end=BASIC_DEPTH
     *    - DATAGRAM: offset_start=BASIC_DEPTH, offset_end=BASIC+DATAGRAM_DEPTH
     *    - SNIP: offset_start=BASIC+DATAGRAM, offset_end=BASIC+DATAGRAM+SNIP
     *    - STREAM: offset_start=BASIC+DATAGRAM+SNIP, offset_end=BASIC+DATAGRAM+SNIP+STREAM
     * -# If invalid payload_type, return NULL immediately
     * -# Linear search from offset_start to offset_end
     * -# Check each message's state.allocated flag
     * -# On first unallocated buffer found:
     *    - Call OpenLcbUtilities_clear_openlcb_message() to zero the structure
     *    - Set reference_count = 1
     *    - Set state.allocated = true
     *    - Call _update_buffer_telemetry() to update counters
     *    - Return pointer to the buffer
     * -# If no free buffer found in range, return NULL
     *
     * Use cases:
     * - Creating new outgoing OpenLCB messages
     * - Assembling multi-frame received messages
     * - Storing messages in FIFO or list structures
     *
     * @verbatim
     * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
     * @endverbatim
     *
     * @return Pointer to allocated message buffer, or NULL if pool exhausted or invalid type
     *
     * @warning Returns NULL when buffer pool exhausted - caller MUST check for NULL
     * @warning NOT thread-safe
     *
     * @attention Always check return value for NULL before dereferencing
     * @attention Buffer is automatically cleared before being returned
     *
     * @note Buffer starts with reference_count = 1
     * @note Allocation telemetry is updated on success
     *
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count and frees
     * @see OpenLcbBufferStore_inc_reference_count - Increments reference count for sharing
     * @see _update_buffer_telemetry - Updates allocation statistics
     */
openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type) {

    uint8_t offset_start = 0;
    uint8_t offset_end = 0;

    switch (payload_type) {

        case BASIC:

            offset_start = 0;
            offset_end = USER_DEFINED_BASIC_BUFFER_DEPTH;

            break;

        case DATAGRAM:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_DATAGRAM_BUFFER_DEPTH;

            break;

        case SNIP:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_SNIP_BUFFER_DEPTH;

            break;

        case STREAM:

            offset_start = USER_DEFINED_BASIC_BUFFER_DEPTH + USER_DEFINED_DATAGRAM_BUFFER_DEPTH + USER_DEFINED_SNIP_BUFFER_DEPTH;
            offset_end = offset_start + USER_DEFINED_STREAM_BUFFER_DEPTH;

            break;

        default:

            return NULL;

    }

    for (int i = offset_start; i < offset_end; i++) {

        if (!_message_buffer.messages[i].state.allocated) {

            OpenLcbUtilities_clear_openlcb_message(&_message_buffer.messages[i]);
            _message_buffer.messages[i].reference_count = 1;
            _message_buffer.messages[i].state.allocated = true;
            _update_buffer_telemetry(_message_buffer.messages[i].payload_type);

            return &_message_buffer.messages[i];
        }
    }

    return NULL;

}

    /**
     * @brief Decrements reference count and potentially frees the buffer for reuse
     *
     * @details Algorithm:
     * Implements reference-counted deallocation to support buffer sharing.
     * -# Check if msg pointer is NULL, return immediately if so
     * -# Decrement msg->reference_count
     * -# If reference_count > 0, buffer is still referenced elsewhere, return without freeing
     * -# If reference_count == 0, proceed with deallocation:
     *    - Switch on msg->payload_type
     *    - Decrement appropriate allocation counter
     *    - Set reference_count = 0 (redundant but explicit)
     *    - Set state.allocated = false to mark buffer as free
     *
     * Use cases:
     * - Releasing a buffer after message transmission
     * - Removing a buffer from FIFO or list
     * - Cleaning up after message processing
     *
     * @verbatim
     * @param msg Pointer to message buffer to be freed (NULL safe)
     * @endverbatim
     *
     * @warning Do NOT access buffer after calling free unless reference count was > 1
     * @warning Reference count underflow will cause undefined behavior
     * @warning NOT thread-safe
     *
     * @attention Safe to call with NULL pointer (no-op)
     * @attention Buffer only freed when reference_count reaches exactly 0
     *
     * @note Telemetry counters updated when buffer actually freed
     * @note Reference count must be managed correctly to prevent leaks
     *
     * @see OpenLcbBufferStore_allocate_buffer - Creates buffer with reference_count = 1
     * @see OpenLcbBufferStore_inc_reference_count - Increments count when sharing
     */
void OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg) {

    if (!msg) {

        return;

    }

    msg->reference_count = msg->reference_count - 1;

    if (msg->reference_count > 0) {

        return;

    }

    switch (msg->payload_type) {

        case BASIC:

            _buffer_store_basic_messages_allocated = _buffer_store_basic_messages_allocated - 1;

            break;

        case DATAGRAM:

            _buffer_store_datagram_messages_allocated = _buffer_store_datagram_messages_allocated - 1;

            break;

        case SNIP:

            _buffer_store_snip_messages_allocated = _buffer_store_snip_messages_allocated - 1;

            break;

        case STREAM:

            _buffer_store_stream_messages_allocated = _buffer_store_stream_messages_allocated - 1;

            break;

    }

    msg->reference_count = 0;

    msg->state.allocated = false;

}

    /**
     * @brief Returns the number of BASIC messages currently allocated
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_basic_messages_allocated
     *
     * Use cases:
     * - Runtime monitoring of buffer usage
     * - Detecting buffer leaks
     * - Load analysis
     *
     * @return Number of BASIC sized messages currently allocated
     *
     * @note This is a live count that changes as buffers are allocated and freed
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Peak usage
     */
uint16_t OpenLcbBufferStore_basic_messages_allocated(void) {

    return (_buffer_store_basic_messages_allocated);

}

    /**
     * @brief Returns the maximum number of BASIC messages allocated simultaneously
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_basic_max_messages_allocated
     *
     * Use cases:
     * - Stress testing to determine minimum buffer pool size
     * - Capacity planning
     *
     * @return Maximum number of BASIC sized messages that have been allocated simultaneously
     *
     * @note Counter only increases, never decreases (until cleared)
     *
     * @see OpenLcbBufferStore_clear_max_allocated - Resets this counter
     */
uint16_t OpenLcbBufferStore_basic_messages_max_allocated(void) {

    return (_buffer_store_basic_max_messages_allocated);

}

    /**
     * @brief Returns the number of DATAGRAM messages currently allocated
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_datagram_messages_allocated
     *
     * Use cases:
     * - Monitoring datagram protocol activity
     * - Detecting datagram buffer leaks
     *
     * @return Number of DATAGRAM sized messages currently allocated
     *
     * @see OpenLcbBufferStore_datagram_messages_max_allocated - Peak usage
     */
uint16_t OpenLcbBufferStore_datagram_messages_allocated(void) {

    return (_buffer_store_datagram_messages_allocated);

}

    /**
     * @brief Returns the maximum number of DATAGRAM messages allocated simultaneously
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_datagram_max_messages_allocated
     *
     * Use cases:
     * - Sizing datagram pool for configuration memory operations
     * - Stress testing
     *
     * @return Maximum number of DATAGRAM sized messages that have been allocated simultaneously
     *
     * @see OpenLcbBufferStore_clear_max_allocated - Resets counter
     */
uint16_t OpenLcbBufferStore_datagram_messages_max_allocated(void) {

    return (_buffer_store_datagram_max_messages_allocated);

}

    /**
     * @brief Returns the number of SNIP messages currently allocated
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_snip_messages_allocated
     *
     * Use cases:
     * - Monitoring SNIP protocol activity
     * - Detecting SNIP buffer leaks
     *
     * @return Number of SNIP sized messages currently allocated
     *
     * @see OpenLcbBufferStore_snip_messages_max_allocated - Peak usage
     */
uint16_t OpenLcbBufferStore_snip_messages_allocated(void) {

    return (_buffer_store_snip_messages_allocated);

}

    /**
     * @brief Returns the maximum number of SNIP messages allocated simultaneously
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_snip_max_messages_allocated
     *
     * Use cases:
     * - Sizing SNIP pool for network enumeration
     * - Testing with multiple node discovery operations
     *
     * @return Maximum number of SNIP sized messages that have been allocated simultaneously
     *
     * @see OpenLcbBufferStore_clear_max_allocated - Resets counter
     */
uint16_t OpenLcbBufferStore_snip_messages_max_allocated(void) {

    return (_buffer_store_snip_max_messages_allocated);

}

    /**
     * @brief Returns the number of STREAM message buffers currently allocated
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_stream_messages_allocated
     *
     * Use cases:
     * - Monitoring stream protocol activity
     * - Detecting stream buffer leaks
     *
     * @return Number of STREAM sized messages currently allocated
     *
     * @see OpenLcbBufferStore_stream_messages_max_allocated - Peak usage
     */
uint16_t OpenLcbBufferStore_stream_messages_allocated(void) {

    return (_buffer_store_stream_messages_allocated);

}

    /**
     * @brief Returns the maximum number of STREAM messages allocated simultaneously
     *
     * @details Algorithm:
     * -# Return the value of _buffer_store_stream_max_messages_allocated
     *
     * Use cases:
     * - Sizing stream pool for firmware update operations
     * - Testing large data transfer scenarios
     *
     * @return Maximum number of STREAM sized messages that have been allocated simultaneously
     *
     * @see OpenLcbBufferStore_clear_max_allocated - Resets counter
     */
uint16_t OpenLcbBufferStore_stream_messages_max_allocated(void) {

    return (_buffer_store_stream_max_messages_allocated);

}

    /**
     * @brief Increments the reference count on an allocated buffer
     *
     * @details Algorithm:
     * -# Add 1 to msg->reference_count
     *
     * Use cases:
     * - Sharing a buffer between transmit and retry queues
     * - Holding a buffer in multiple lists simultaneously
     * - Passing a buffer to callback while keeping local reference
     *
     * @verbatim
     * @param msg Pointer to message buffer to increment reference count (must NOT be NULL)
     * @endverbatim
     *
     * @warning Passing NULL will crash - no NULL check performed
     * @warning NOT thread-safe
     *
     * @attention Always pair with corresponding free_buffer() call
     * @attention Reference count overflow is not checked
     *
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count
     */
void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg) {

    msg->reference_count = msg->reference_count + 1;

}

    /**
     * @brief Resets all peak allocation counters to zero
     *
     * @details Algorithm:
     * -# Set _buffer_store_basic_max_messages_allocated = 0
     * -# Set _buffer_store_datagram_max_messages_allocated = 0
     * -# Set _buffer_store_snip_max_messages_allocated = 0
     * -# Set _buffer_store_stream_max_messages_allocated = 0
     *
     * Use cases:
     * - Starting a new stress test run
     * - Measuring peak usage for specific operational scenarios
     * - Periodic monitoring with fresh baselines
     *
     * @attention Does NOT affect current allocation counts
     * @attention Does not free any buffers
     *
     * @note Typically called at start of test scenario
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_datagram_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_snip_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_stream_messages_max_allocated - Counter that gets cleared
     */
void OpenLcbBufferStore_clear_max_allocated(void) {

    _buffer_store_basic_max_messages_allocated = 0;
    _buffer_store_datagram_max_messages_allocated = 0;
    _buffer_store_snip_max_messages_allocated = 0;
    _buffer_store_stream_max_messages_allocated = 0;

}
