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
* @file openlcb_login_statemachine.h
* @brief Non-blocking login state machine that walks each node through
*        Initialization Complete, Producer Identified, Consumer Identified,
*        and on_login_complete before entering RUNSTATE_RUN.
*
* @author Jim Kueneman
* @date 28 Feb 2026
*/

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__
#define    __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__

#include "openlcb_types.h"

    /** @brief Callback interface for the login state machine.  All pointers are REQUIRED
     *         unless noted.  Internal function pointers are exposed for unit testing. */
typedef struct {

        /** @brief Queue a message for transmission.  Return false if buffer full (retried next tick). REQUIRED. */
    bool (*send_openlcb_msg)(openlcb_msg_t *outgoing_msg);

        /** @brief Return the first node in the pool (NULL if empty).  key separates iteration contexts. REQUIRED. */
    openlcb_node_t *(*openlcb_node_get_first)(uint8_t key);

        /** @brief Return the next node in the pool (NULL at end).  REQUIRED. */
    openlcb_node_t *(*openlcb_node_get_next)(uint8_t key);

        /** @brief Build Initialization Complete message for the current node.  REQUIRED. */
    void (*load_initialization_complete)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /** @brief Build the next Producer Identified message; set enumerate flag if more remain.  REQUIRED. */
    void (*load_producer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /** @brief Build the next Consumer Identified message; set enumerate flag if more remain.  REQUIRED. */
    void (*load_consumer_events)(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

    // ---- Internal function pointers (exposed for unit testing) ----

        /** @brief Dispatches to the handler matching node->run_state. */
    void (*process_login_statemachine)(openlcb_login_statemachine_info_t *statemachine_info);

        /** @brief Try to send the pending outgoing message; return true if one was pending. */
    bool (*handle_outgoing_openlcb_message)(void);

        /** @brief Re-enter the state processor if the enumerate flag is set. */
    bool (*handle_try_reenumerate)(void);

        /** @brief Start enumeration from the first node if no current node is active. */
    bool (*handle_try_enumerate_first_node)(void);

        /** @brief Advance to the next node that needs login processing. */
    bool (*handle_try_enumerate_next_node)(void);


        /** @brief Called after login completes, just before RUNSTATE_RUN.  Optional (may be NULL). */
    bool (*on_login_complete)(openlcb_node_t *openlcb_node);

} interface_openlcb_login_state_machine_t;


#ifdef    __cplusplus
extern "C" {
#endif /* __cplusplus */

        /**
         * @brief Stores the callback interface.  Call once at startup after
         *        OpenLcbLoginMessageHandler_initialize().
         *
         * @param interface_openlcb_login_state_machine  Must remain valid for application lifetime.
         */
    extern void OpenLcbLoginStateMachine_initialize(const interface_openlcb_login_state_machine_t *interface_openlcb_login_state_machine);

        /**
         * @brief Runs one non-blocking step of login processing.  Call from main loop.
         *
         * @details Tries to send a pending message, re-enumerate if flagged, or advance
         *          to the next node needing login.  Nodes already in RUNSTATE_RUN are skipped.
         */
    extern void OpenLcbLoginMainStatemachine_run(void);

        /**
         * @brief Dispatches to the handler matching node->run_state.  Exposed for unit testing.
         *
         * @param openlcb_statemachine_info  Pointer to @ref openlcb_login_statemachine_info_t context.
         */
    extern void OpenLcbLoginStateMachine_process(openlcb_login_statemachine_info_t *openlcb_statemachine_info);

        /** @brief Tries to send the pending message; returns true if one was pending.  Exposed for unit testing. */
    extern bool OpenLcbLoginStatemachine_handle_outgoing_openlcb_message(void);

        /** @brief Re-enters the state processor if the enumerate flag is set.  Exposed for unit testing. */
    extern bool OpenLcbLoginStatemachine_handle_try_reenumerate(void);

        /** @brief Starts enumeration from the first node if none is active.  Exposed for unit testing. */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_first_node(void);

        /** @brief Advances to the next node needing login.  Exposed for unit testing. */
    extern bool OpenLcbLoginStatemachine_handle_try_enumerate_next_node(void);

        /** @brief Returns pointer to internal static state machine info.  For unit testing only — do not modify. */
    extern openlcb_login_statemachine_info_t *OpenLcbLoginStatemachine_get_statemachine_info(void);

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif    /* __OPENLCB_OPENLCB_LOGIN_STATEMACHINE__ */
