/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * @file can_config.h
 * @brief User-facing CAN bus transport configuration
 *
 * @details Users provide their hardware-specific CAN driver functions here.
 * All other CAN-internal wiring is handled automatically by can_config.c.
 *
 * @author Jim Kueneman
 * @date 28 Feb 2026
 *
 * @see openlcb_config.h - OpenLCB protocol layer configuration
 */

#ifndef __DRIVERS_CANBUS_CAN_CONFIG__
#define __DRIVERS_CANBUS_CAN_CONFIG__

#include <stdbool.h>
#include <stdint.h>

#include "can_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /**
     * @brief CAN bus transport configuration.
     *
     * @details Users provide their hardware-specific CAN driver functions here.
     * All other CAN-internal wiring is handled automatically.
     *
     * Example:
     * @code
     * static const can_config_t can_config = {
     *     .transmit_raw_can_frame  = &MyCanDriver_transmit,
     *     .is_tx_buffer_clear      = &MyCanDriver_is_tx_clear,
     *     .lock_shared_resources   = &MyDriver_lock,
     *     .unlock_shared_resources = &MyDriver_unlock,
     *     .on_rx                   = &my_can_rx_handler,   // optional
     *     .on_tx                   = &my_can_tx_handler,   // optional
     *     .on_alias_change         = &my_alias_handler,    // optional
     * };
     *
     * CanConfig_initialize(&can_config);
     * @endcode
     */
    typedef struct {

        /** @brief Transmit a raw CAN frame. REQUIRED. */
        bool (*transmit_raw_can_frame)(can_msg_t *can_msg);

        /** @brief Check if CAN TX hardware buffer can accept another frame. REQUIRED. */
        bool (*is_tx_buffer_clear)(void);

        /** @brief Disable interrupts / acquire mutex for shared resource access. REQUIRED.
         *  Same function as openlcb_config_t.lock_shared_resources. */
        void (*lock_shared_resources)(void);

        /** @brief Re-enable interrupts / release mutex. REQUIRED.
         *  Same function as openlcb_config_t.unlock_shared_resources. */
        void (*unlock_shared_resources)(void);

        /** @brief Called when a CAN frame is received. Optional. */
        void (*on_rx)(can_msg_t *can_msg);

        /** @brief Called when a CAN frame is transmitted. Optional. */
        void (*on_tx)(can_msg_t *can_msg);

        /** @brief Called when a node's CAN alias changes. Optional. */
        void (*on_alias_change)(uint16_t alias, node_id_t node_id);

    } can_config_t;

        /**
         * @brief Initializes the CAN bus transport layer.
         *
         * @details Must be called BEFORE OpenLcb_initialize().
         *
         * @param config  Pointer to @ref can_config_t configuration. Must remain
         *                valid for the lifetime of the application.
         *
         * @warning NOT thread-safe - call during single-threaded initialization only.
         *
         * @see openlcb_config.h
         */
    extern void CanConfig_initialize(const can_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DRIVERS_CANBUS_CAN_CONFIG__ */
