/** \copyright
 * Copyright (c) 2026, Jim Kueneman
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
 * \file protocol_modes.h
 *
 * Extensible protocol mode registry for the ComplianceTestNode.
 * Adding a new protocol means adding one entry to the protocol_modes[] array.
 *
 * @author Jim Kueneman
 * @date 8 Mar 2026
 */

#ifndef __PROTOCOL_MODES__
#define __PROTOCOL_MODES__

#include "openlcb_c_lib/openlcb/openlcb_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    // Setup function signature: called after OpenLcb_create_node() to
    // register protocol-specific event ranges, handlers, etc.
    typedef void (*protocol_setup_fn)(openlcb_node_t *node);

    // Login complete function signature: called when the node finishes alias
    // negotiation and is ready on the bus.
    typedef bool (*protocol_login_fn)(openlcb_node_t *node);

    typedef struct {
        const char *flag;                       // CLI flag (e.g., "--train")
        const char *name;                       // Human-readable name for logging
        const char *test_section;               // Section name for run_tests.py
        const node_parameters_t *params;        // Node parameters for this mode
        protocol_setup_fn setup;                // Post-creation setup (NULL if none)
        protocol_login_fn on_login;             // Login complete handler (NULL if none)
    } protocol_mode_t;

    // Registry of all available protocol modes.
    // Terminated by an entry with flag == NULL.
    extern const protocol_mode_t protocol_modes[];

    // Find a mode by CLI flag. Returns NULL if not found.
    extern const protocol_mode_t *ProtocolModes_find(const char *flag);

    // Returns the default (basic) mode.
    extern const protocol_mode_t *ProtocolModes_default(void);

    // Print all available modes to stdout.
    extern void ProtocolModes_print_usage(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PROTOCOL_MODES__ */
