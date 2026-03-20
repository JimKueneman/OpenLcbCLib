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
 * @file openlcb_minimal_compile_Test.cxx
 * @brief Compilation and init test for minimal config (no optional features).
 *
 * @author Jim Kueneman
 * @date 20 Mar 2026
 */

#include <gtest/gtest.h>

extern "C" {

#include "openlcb_user_config.h"
#include "openlcb/openlcb_types.h"
#include "openlcb/openlcb_defines.h"
#include "openlcb/openlcb_config.h"

}

// =============================================================================
// Compile-time validation — no optional features should be defined
// =============================================================================

#ifdef OPENLCB_COMPILE_EVENTS
#error "OPENLCB_COMPILE_EVENTS must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_DATAGRAMS
#error "OPENLCB_COMPILE_DATAGRAMS must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_MEMORY_CONFIGURATION
#error "OPENLCB_COMPILE_MEMORY_CONFIGURATION must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_FIRMWARE
#error "OPENLCB_COMPILE_FIRMWARE must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_BROADCAST_TIME
#error "OPENLCB_COMPILE_BROADCAST_TIME must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_TRAIN
#error "OPENLCB_COMPILE_TRAIN must NOT be defined in minimal config"
#endif

#ifdef OPENLCB_COMPILE_TRAIN_SEARCH
#error "OPENLCB_COMPILE_TRAIN_SEARCH must NOT be defined in minimal config"
#endif

// =============================================================================
// Stub callbacks
// =============================================================================

static void _stub_lock(void) { }
static void _stub_unlock(void) { }

// =============================================================================
// Tests
// =============================================================================

TEST(MinimalCompile, preprocessor_flags_correct) {

    SUCCEED();

}

TEST(MinimalCompile, initialize_and_teardown) {

    static const openlcb_config_t config = {

        .lock_shared_resources = &_stub_lock,
        .unlock_shared_resources = &_stub_unlock,

    };

    OpenLcb_initialize(&config);

    SUCCEED();

}
