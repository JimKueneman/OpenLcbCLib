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
 * @file openlcb_buffer_store.h
 * @brief Core buffer store for OpenLcb/LCC message buffers with allocation and management
 *
 * @details This module provides the memory pool for OpenLCB message allocation with
 * support for four different payload sizes: Basic, Datagram, SNIP, and Stream. The buffer
 * store pre-allocates a fixed pool of message structures at initialization and manages
 * their lifecycle through allocation and deallocation.
 *
 * Key features:
 * - Pre-allocated memory pool (no dynamic allocation during runtime)
 * - Four segregated buffer types by payload size
 * - Reference counting for shared buffer management
 * - Allocation telemetry for monitoring and stress testing
 * - Thread-unsafe (designed for single-threaded or externally synchronized use)
 *
 * Buffer types and sizes (defined in openlcb_types.h):
 * - BASIC: Small messages (8 bytes payload)
 * - DATAGRAM: Medium messages (72 bytes payload)
 * - SNIP: Simple Node Information (255 bytes payload)
 * - STREAM: Large messages (for streaming data)
 *
 * The buffer store must be initialized before any other OpenLCB operations and should
 * never be reinitialized during runtime.
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_BUFFER_STORE__
#define __OPENLCB_OPENLCB_BUFFER_STORE__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief Initializes the OpenLcb Buffer Store
     *
     * @details Sets up the pre-allocated message pool by:
     * - Clearing all message structures to zero
     * - Linking each message to its appropriate payload buffer
     * - Organizing buffers by type: [BASIC][DATAGRAM][SNIP][STREAM]
     * - Resetting all allocation counters and telemetry
     *
     * The buffer pool layout after initialization:
     * - Messages 0 to (BASIC_DEPTH-1): BASIC payload type
     * - Messages (BASIC_DEPTH) to (BASIC+DATAGRAM-1): DATAGRAM payload type
     * - Messages (BASIC+DATAGRAM) to (BASIC+DATAGRAM+SNIP-1): SNIP payload type
     * - Remaining messages: STREAM payload type
     *
     * Use cases:
     * - Called once during application startup
     * - Required before any buffer allocation operations
     * - Must be called before OpenLcbBufferFifo_initialize() and OpenLcbBufferList_initialize()
     *
     * @warning MUST be called exactly once during application initialization before
     *          any buffer allocation operations. Calling multiple times will reset all
     *          allocation state and invalidate any outstanding buffer pointers.
     * @warning This function is NOT thread-safe. Must be called during single-threaded
     *          initialization phase only.
     *
     * @attention Call this function before:
     *            - OpenLcbBufferFifo_initialize()
     *            - OpenLcbBufferList_initialize()
     *            - Any call to OpenLcbBufferStore_allocate_buffer()
     * @attention The buffer pool size is determined by compile-time constants:
     *            USER_DEFINED_BASIC_BUFFER_DEPTH, USER_DEFINED_DATAGRAM_BUFFER_DEPTH,
     *            USER_DEFINED_SNIP_BUFFER_DEPTH, USER_DEFINED_STREAM_BUFFER_DEPTH
     *
     * @see OpenLcbBufferStore_allocate_buffer - Allocates buffers from initialized pool
     * @see OpenLcbBufferStore_free_buffer - Returns buffers to the pool
     */
    extern void OpenLcbBufferStore_initialize(void);

    /**
     * @brief Allocates a new buffer of the specified payload type
     *
     * @details Searches the appropriate buffer pool segment for an unallocated buffer and
     * returns a pointer to it. The buffer is marked as allocated, cleared, and its reference
     * count is set to 1.
     *
     * Allocation strategy:
     * - Linear search within the specified payload type's range
     * - Returns first available (non-allocated) buffer
     * - Clears the message structure before returning
     * - Sets reference_count = 1
     * - Updates allocation telemetry counters
     *
     * Use cases:
     * - Creating new outgoing OpenLCB messages
     * - Assembling multi-frame received messages
     * - Storing messages in FIFO or list structures
     *
     * @param payload_type Type of buffer requested (BASIC, DATAGRAM, SNIP, or STREAM)
     * @return Pointer to allocated message buffer, or NULL if:
     *         - Pool exhausted (all buffers of this type allocated)
     *         - Invalid payload_type parameter
     *
     * @warning Returns NULL when buffer pool is exhausted. Caller MUST check for NULL
     *          before dereferencing. Dereferencing NULL will cause immediate crash.
     * @warning Buffer pool is fixed size. System will stop allocating when pool is full.
     *          Design your application to handle allocation failures gracefully.
     * @warning This function is NOT thread-safe. Concurrent calls may return the same
     *          buffer to multiple callers.
     *
     * @attention Always check return value for NULL before use
     * @attention Buffer remains allocated until reference count reaches zero
     * @attention Use OpenLcbBufferStore_inc_reference_count() when sharing buffers
     *
     * @note Buffer is automatically cleared (zeroed) before being returned
     * @note Allocation telemetry is updated on successful allocation
     *
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count and potentially frees buffer
     * @see OpenLcbBufferStore_inc_reference_count - Increments reference count for shared buffers
     * @see OpenLcbBufferFifo_push - Common use case for storing allocated buffers
     * @see OpenLcbBufferList_add - Another common use case for storing allocated buffers
     */
    extern openlcb_msg_t *OpenLcbBufferStore_allocate_buffer(payload_type_enum payload_type);

    /**
     * @brief Decrements reference count and potentially frees the buffer for reuse
     *
     * @details Implements reference-counted buffer deallocation. Decrements the buffer's
     * reference count and only marks it as free when the count reaches zero. This allows
     * multiple parts of the system to safely share buffer pointers.
     *
     * Use cases:
     * - Releasing a buffer after message transmission
     * - Removing a buffer from FIFO or list
     * - Cleaning up after message processing
     * - Each holder releasing their shared reference
     *
     * @param msg Pointer to message buffer to be freed (NULL safe)
     *
     * @warning Do NOT access buffer after calling free unless you know reference count was > 1
     * @warning Reference count underflow (calling free too many times) will cause undefined behavior
     * @warning This function is NOT thread-safe
     *
     * @attention Safe to call with NULL pointer (function will return immediately)
     * @attention Buffer is only marked as free when reference_count reaches exactly 0
     * @attention Always ensure each inc_reference_count() is paired with a free_buffer() call
     *
     * @note Telemetry counters are updated when buffer is actually freed
     * @note Common pattern: allocate (count=1), increment for sharing (count=2), free twice to release
     *
     * @see OpenLcbBufferStore_allocate_buffer - Creates buffer with reference_count = 1
     * @see OpenLcbBufferStore_inc_reference_count - Increments count when sharing buffer
     */
    extern void OpenLcbBufferStore_free_buffer(openlcb_msg_t *msg);

    /**
     * @brief Returns the number of BASIC messages currently allocated
     *
     * @details Provides real-time count of allocated BASIC-type message buffers.
     * Useful for monitoring system load and detecting buffer leaks.
     *
     * Use cases:
     * - Runtime monitoring of buffer usage
     * - Detecting buffer leaks
     * - Load balancing decisions
     *
     * @return Number of BASIC sized messages currently allocated (0 to USER_DEFINED_BASIC_BUFFER_DEPTH)
     *
     * @note This is a live count that changes as buffers are allocated and freed
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Peak usage counter
     */
    extern uint16_t OpenLcbBufferStore_basic_messages_allocated(void);

    /**
     * @brief Returns the maximum number of BASIC messages allocated simultaneously
     *
     * @details Tracks peak BASIC buffer usage for capacity planning and stress testing.
     * This counter only increases, never decreases (until cleared).
     *
     * Use cases:
     * - Stress testing to determine minimum buffer pool size
     * - Capacity planning for production systems
     * - Verifying buffer pool configuration is adequate
     *
     * @return Maximum number of BASIC sized messages that have been allocated simultaneously
     *
     * @note If this value equals USER_DEFINED_BASIC_BUFFER_DEPTH during testing, consider increasing pool size
     *
     * @see OpenLcbBufferStore_basic_messages_allocated - Current count
     * @see OpenLcbBufferStore_clear_max_allocated - Resets all peak counters
     */
    extern uint16_t OpenLcbBufferStore_basic_messages_max_allocated(void);

    /**
     * @brief Returns the number of DATAGRAM messages currently allocated
     *
     * @details Provides real-time count of datagram message buffers. Datagram messages
     * are used for configuration memory access and other protocol operations.
     *
     * Use cases:
     * - Monitoring datagram protocol activity
     * - Detecting datagram buffer leaks
     * - Analyzing configuration memory operations
     *
     * @return Number of DATAGRAM sized messages currently allocated (0 to USER_DEFINED_DATAGRAM_BUFFER_DEPTH)
     *
     * @note Datagram messages are larger than basic messages
     *
     * @see OpenLcbBufferStore_datagram_messages_max_allocated - Peak usage counter
     * @see OpenLcbBufferStore_allocate_buffer - Allocates datagram buffers
     */
    extern uint16_t OpenLcbBufferStore_datagram_messages_allocated(void);

    /**
     * @brief Returns the maximum number of DATAGRAM messages allocated simultaneously
     *
     * @details Tracks peak DATAGRAM buffer usage for capacity planning and stress testing.
     * Datagram operations (like configuration memory access) can be resource intensive.
     *
     * Use cases:
     * - Sizing datagram pool for configuration memory operations
     * - Stress testing with multiple simultaneous datagram operations
     * - Ensuring adequate buffers for expected load
     *
     * @return Maximum number of DATAGRAM sized messages that have been allocated simultaneously
     *
     * @note If this equals USER_DEFINED_DATAGRAM_BUFFER_DEPTH during testing, increase pool size
     *
     * @see OpenLcbBufferStore_datagram_messages_allocated - Current count
     * @see OpenLcbBufferStore_clear_max_allocated - Reset peak counters
     */
    extern uint16_t OpenLcbBufferStore_datagram_messages_max_allocated(void);

    /**
     * @brief Returns the number of SNIP messages currently allocated
     *
     * @details Provides real-time count of Simple Node Information Protocol message buffers.
     * SNIP messages are larger and used for node identification and manufacturer information.
     *
     * Use cases:
     * - Monitoring SNIP protocol activity
     * - Detecting SNIP buffer leaks
     * - Analyzing node discovery operations
     *
     * @return Number of SNIP sized messages currently allocated (0 to USER_DEFINED_SNIP_BUFFER_DEPTH)
     *
     * @note SNIP buffers contain node identification strings and manufacturer data
     *
     * @see OpenLcbBufferStore_snip_messages_max_allocated - Peak usage counter
     */
    extern uint16_t OpenLcbBufferStore_snip_messages_allocated(void);

    /**
     * @brief Returns the maximum number of SNIP messages allocated simultaneously
     *
     * @details Tracks peak SNIP buffer usage. SNIP operations typically occur during
     * node discovery and enumeration phases.
     *
     * Use cases:
     * - Sizing SNIP pool for network enumeration
     * - Testing with multiple node discovery operations
     *
     * @return Maximum number of SNIP sized messages that have been allocated simultaneously
     *
     * @see OpenLcbBufferStore_snip_messages_allocated - Current count
     * @see OpenLcbBufferStore_clear_max_allocated - Reset counters
     */
    extern uint16_t OpenLcbBufferStore_snip_messages_max_allocated(void);

    /**
     * @brief Returns the number of STREAM message buffers currently allocated
     *
     * @details Provides real-time count of STREAM-type message buffers. Stream buffers
     * are the largest and used for high-bandwidth data transfer operations.
     *
     * Use cases:
     * - Monitoring stream protocol activity
     * - Detecting stream buffer leaks
     * - Analyzing firmware update or bulk data transfer operations
     *
     * @return Number of STREAM sized messages currently allocated (0 to USER_DEFINED_STREAM_BUFFER_DEPTH)
     *
     * @note Stream buffers are used for firmware updates and large data transfers
     *
     * @see OpenLcbBufferStore_stream_messages_max_allocated - Peak usage
     */
    extern uint16_t OpenLcbBufferStore_stream_messages_allocated(void);

    /**
     * @brief Returns the maximum number of STREAM messages allocated simultaneously
     *
     * @details Tracks peak STREAM buffer usage for capacity planning of bulk data
     * transfer operations.
     *
     * Use cases:
     * - Sizing stream pool for firmware update operations
     * - Testing large data transfer scenarios
     *
     * @return Maximum number of STREAM sized messages that have been allocated simultaneously
     *
     * @see OpenLcbBufferStore_stream_messages_allocated - Current count
     * @see OpenLcbBufferStore_clear_max_allocated - Reset counters
     */
    extern uint16_t OpenLcbBufferStore_stream_messages_max_allocated(void);

    /**
     * @brief Increments the reference count on an allocated buffer
     *
     * @details Increases the buffer's reference count to indicate that an additional
     * part of the system is now holding a pointer to this buffer. This prevents the
     * buffer from being freed prematurely when one holder calls free() while another
     * still needs the buffer. Each allocated buffer starts with reference_count = 1.
     *
     * Use cases:
     * - Sharing a buffer between transmit and retry queues
     * - Holding a buffer in multiple lists simultaneously
     * - Passing a buffer to callback while keeping local reference
     *
     * @param msg Pointer to message buffer to increment reference count
     *
     * @warning Passing NULL will cause immediate crash on reference count access - no NULL check performed
     * @warning Reference count is NOT bounds-checked. Incrementing indefinitely may
     *          cause overflow and prevent buffer from ever being freed.
     * @warning This function is NOT thread-safe
     *
     * @attention Always pair with corresponding OpenLcbBufferStore_free_buffer() call
     * @attention Reference count must be managed carefully to prevent leaks
     * @attention If you increment, you must eventually decrement (via free)
     *
     * @note Common pattern: allocate (count=1), increment for sharing (count=2),
     *       free twice (once per holder) to reach count=0
     *
     * @see OpenLcbBufferStore_allocate_buffer - Creates buffer with reference_count = 1
     * @see OpenLcbBufferStore_free_buffer - Decrements reference count
     */
    extern void OpenLcbBufferStore_inc_reference_count(openlcb_msg_t *msg);

    /**
     * @brief Resets all peak allocation counters to zero
     *
     * @details Clears the maximum allocated counters for all four buffer types (BASIC,
     * DATAGRAM, SNIP, STREAM). This allows you to measure peak usage during specific
     * test scenarios or operational phases.
     *
     * Use cases:
     * - Starting a new stress test run
     * - Measuring peak usage for specific operational scenarios
     * - Resetting after configuration changes to remeasure
     * - Periodic monitoring with fresh baselines
     *
     * @attention This does NOT affect current allocation counts, only the peak counters
     * @attention Current allocations remain valid and tracked
     *
     * @note Typically called at the start of a test scenario or monitoring period
     * @note Does not free any buffers or change allocation state
     *
     * @see OpenLcbBufferStore_basic_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_datagram_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_snip_messages_max_allocated - Counter that gets cleared
     * @see OpenLcbBufferStore_stream_messages_max_allocated - Counter that gets cleared
     */
    extern void OpenLcbBufferStore_clear_max_allocated(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_BUFFER_STORE__ */
