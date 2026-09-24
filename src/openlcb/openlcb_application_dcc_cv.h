/** \copyright
 * Copyright (c) 2026, Bob Gamble
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
 * @file openlcb_application_dcc_cv.h
 * @brief DCC CV memory space (0xF8) with deferred replies.
 *
 * @details Serves Memory Configuration space 0xF8, in which one byte is one DCC
 * configuration variable and the address is the CV number minus one.  This is the
 * space JMRI's OpenLCB programmer and OpenMRN's TractionCvSpace use for CV access:
 * an ops-mode (POM) read or write goes to a train node, a service-mode
 * (programming track) read or write goes to the node that produces the
 * "programming track" event.  It is a convention between those two
 * implementations; the adopted Train Control standard defines only 0xF9 and 0xFA.
 *
 * Reading or writing a CV on a real decoder takes from tens of milliseconds to a
 * few seconds and finishes on some other task or core, so this module does not
 * ask the application to answer inside the datagram handler.  The request
 * handlers below (wired in as the space's read/write request functions) call an
 * application hook that answers in one of three ways:
 *
 * - S_OK: done, the reply goes out through the normal state machine path;
 * - an OpenLCB error code: failed, same path;
 * - OPENLCB_DCC_CV_RESULT_PENDING: not done yet.  The library remembers who asked,
 *   sends nothing, and the application later calls
 *   OpenLcbApplicationDccCv_complete() with the handle it was given.  The library
 *   then builds and sends the reply datagram from OpenLcbConfig_run().  If the
 *   application never completes, the library answers ERROR_TEMPORARY_TIME_OUT
 *   itself after USER_DEFINED_DCC_CV_TIMEOUT_TICKS.
 *
 * The datagram handler has already acknowledged the request with Datagram
 * Received OK (Reply Pending) before the hook runs, exactly as for every other
 * space, so the requester is waiting for a reply datagram.
 *
 * Contexts: the hooks run inside OpenLcbConfig_run(); OpenLcbApplicationDccCv_complete()
 * must be called from the same context (it takes no lock).  If the CV work runs on
 * another task or core, pass the result back to the main loop and call complete()
 * there.
 *
 * Limits: one CV (one byte) per request, as JMRI and OpenMRN send them; a request
 * for any other length is answered with ERROR_PERMANENT_INVALID_ARGUMENTS.
 *
 * Timing: JMRI's memory configuration service re-sends a request that has had no
 * reply for 3 seconds, up to three sends in all, and then fails it.  An identical
 * request that arrives while the first is still outstanding is therefore absorbed
 * (no second operation is started) and the one reply answers both.  The default
 * USER_DEFINED_DCC_CV_TIMEOUT_TICKS of 8 seconds makes the library's own timeout
 * reply land before the third try runs out.  Leave the configuration-memory
 * "delayed reply time" hooks unset for this space: with no time advertised JMRI
 * uses its 3 second default, and it does not act sensibly on a non-zero one.
 *
 * Enabled with OPENLCB_COMPILE_DCC_CV (requires OPENLCB_COMPILE_MEMORY_CONFIGURATION).
 *
 *
 * @see protocol_config_mem_read_handler.h
 * @see protocol_config_mem_write_handler.h
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_APPLICATION_DCC_CV__
#define __OPENLCB_OPENLCB_APPLICATION_DCC_CV__

#include <stdbool.h>
#include <stdint.h>

#include "openlcb_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef OPENLCB_COMPILE_DCC_CV

    /** @brief How many requests can be waiting for their result at the same time. */
#ifndef USER_DEFINED_DCC_CV_PENDING_COUNT
#define USER_DEFINED_DCC_CV_PENDING_COUNT 4
#endif

    /**
     * @brief How long (in 100 ms ticks) the application has to complete a request.
     *
     * @details After this the library answers ERROR_TEMPORARY_TIME_OUT.  It also
     * bounds how long a finished reply is kept if the bus will not take it.
     * Must be at most 200 (the tick counter is 8 bits).
     */
#ifndef USER_DEFINED_DCC_CV_TIMEOUT_TICKS
#define USER_DEFINED_DCC_CV_TIMEOUT_TICKS 80
#endif

#if USER_DEFINED_DCC_CV_PENDING_COUNT < 1 || USER_DEFINED_DCC_CV_PENDING_COUNT > 255
#error "USER_DEFINED_DCC_CV_PENDING_COUNT must be 1..255"
#endif

#if USER_DEFINED_DCC_CV_TIMEOUT_TICKS < 1 || USER_DEFINED_DCC_CV_TIMEOUT_TICKS > 200
#error "USER_DEFINED_DCC_CV_TIMEOUT_TICKS must be 1..200"
#endif

    /**
     * @brief Hook return value meaning "the result will be reported later with
     *        OpenLcbApplicationDccCv_complete()".  Not an OpenLCB error code.
     */
#define OPENLCB_DCC_CV_RESULT_PENDING 0xFFFF

    /**
     * @defgroup dcc_cv_errors Error codes JMRI's OpenLCB programmer understands
     * @brief Node-specific codes used with space 0xF8 (JMRI OlcbProgrammer, OpenMRN).
     * @{
     */

        /** @brief Service mode: no locomotive detected.  POM: the locomotive is not on the track. */
#define OPENLCB_DCC_CV_ERROR_NO_LOCO 0x2031

        /** @brief The verify that follows a write got no acknowledgement. */
#define OPENLCB_DCC_CV_ERROR_VERIFY_FAILED 0x2032

        /** @brief POM: the locomotive never answered on RailCom.  (JMRI treats this as success for a POM write.) */
#define OPENLCB_DCC_CV_ERROR_NO_RAILCOM 0x2033

        /** @brief POM: only garbage came back on RailCom. */
#define OPENLCB_DCC_CV_ERROR_INVALID_RESPONSE 0x2034

        /** @brief Service mode: short circuit on the programming track. */
#define OPENLCB_DCC_CV_ERROR_PROGRAMMING_SHORT 0x2035

        /** @brief Programming is disabled. */
#define OPENLCB_DCC_CV_ERROR_PROGRAMMING_DISABLED 0x1021

    /** @} */ // end of dcc_cv_errors

        /**
         * @brief Application hook: read one CV.
         *
         * @param openlcb_node  The node that was addressed (a train node for ops mode).
         * @param cv_address    CV number minus one.
         * @param value         Where to store the byte when returning S_OK.
         * @param handle        Pass to OpenLcbApplicationDccCv_complete() when returning
         *                      OPENLCB_DCC_CV_RESULT_PENDING.  Meaningless otherwise.
         *
         * @return S_OK (result in *value), OPENLCB_DCC_CV_RESULT_PENDING, or an OpenLCB
         *         error code (0x1xxx permanent, 0x2xxx temporary; see @ref dcc_cv_errors).
         *
         * @warning Must not block.
         */
    typedef uint16_t (*dcc_cv_read_func_t)(openlcb_node_t *openlcb_node, uint32_t cv_address, uint8_t *value, uint16_t handle);

        /**
         * @brief Application hook: write one CV.
         *
         * @param openlcb_node  The node that was addressed.
         * @param cv_address    CV number minus one.
         * @param value         The byte to write.
         * @param handle        As for @ref dcc_cv_read_func_t.
         *
         * @return S_OK, OPENLCB_DCC_CV_RESULT_PENDING, or an OpenLCB error code.
         *
         * @warning Must not block.
         */
    typedef uint16_t (*dcc_cv_write_func_t)(openlcb_node_t *openlcb_node, uint32_t cv_address, uint8_t value, uint16_t handle);

        /** @brief Dependency-injection table; filled by OpenLcbConfig_initialize(). */
    typedef struct {

            /** @brief Sends a message to the bus.  Set by the library. */
        bool (*send_openlcb_msg)(openlcb_msg_t *openlcb_msg);

            /** @brief Application read hook.  NULL = reads are not served. */
        dcc_cv_read_func_t dcc_cv_read;

            /** @brief Application write hook.  NULL = writes are not served. */
        dcc_cv_write_func_t dcc_cv_write;

    } interface_openlcb_application_dcc_cv_t;

        /**
         * @brief Stores the interface and clears the pending table.  Called by OpenLcbConfig_initialize().
         *
         * @param interface  Pointer to @ref interface_openlcb_application_dcc_cv_t (must remain valid).
         */
    extern void OpenLcbApplicationDccCv_initialize(const interface_openlcb_application_dcc_cv_t *interface);

        /**
         * @brief Read request function for space 0xF8 (wired as read_request_dcc_cv).
         *
         * @param statemachine_info             Context; the reply is loaded into its outgoing message
         *                                      unless the request is deferred.
         * @param config_mem_read_request_info  Parsed request.
         */
    extern void OpenLcbApplicationDccCv_handle_read_request(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info);

        /**
         * @brief Write request function for space 0xF8 (wired as write_request_dcc_cv).
         *
         * @param statemachine_info              Context.
         * @param config_mem_write_request_info  Parsed request.
         */
    extern void OpenLcbApplicationDccCv_handle_write_request(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info);

        /**
         * @brief Reports the result of a request an application hook answered with
         *        OPENLCB_DCC_CV_RESULT_PENDING.
         *
         * @details The reply datagram is sent from OpenLcbConfig_run(), not from here.
         * Call from the same context as OpenLcbConfig_run().
         *
         * @param handle  The handle the hook was given.
         * @param result  S_OK, or an OpenLCB error code.  OPENLCB_DCC_CV_RESULT_PENDING is rejected.
         * @param value   For a read that succeeded, the byte read.  Ignored otherwise.
         *
         * @return true if the result was recorded; false if the handle is stale (the request already
         *         timed out or was answered) or unknown.
         */
    extern bool OpenLcbApplicationDccCv_complete(uint16_t handle, uint16_t result, uint8_t value);

        /**
         * @brief Sends finished replies and times out requests the application never completed.
         *
         * @details Called by OpenLcbConfig_run() through the periodic services.
         *
         * @param current_tick  Global 100 ms tick.
         */
    extern void OpenLcbApplicationDccCv_run(uint8_t current_tick);

#endif /* OPENLCB_COMPILE_DCC_CV */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_APPLICATION_DCC_CV__ */
