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
 * \file callbacks_config_mem.c
 *
 * Configuration memory callback implementations for the BasicNode demo.
 *
 * These callbacks are wired into the OpenLCB stack via openlcb_config_t in
 * main.c.  The library calls them when it receives the corresponding OpenLCB
 * Configuration Memory protocol commands from the network.
 *
 * The most important callback for the bootloader integration is
 * CallbacksConfigMem_freeze().  It implements the full application-to-
 * bootloader drop-back sequence that is unique to the dsPIC33 Harvard
 * architecture.  See the detailed comments in that function below.
 *
 * @author Jim Kueneman
 * @date 15 Mar 2026
 */

#include "callbacks_config_mem.h"

#include <xc.h>
#include <stdio.h>

#include "../openlcb_c_lib/openlcb/openlcb_utilities.h"
#include "../openlcb_c_lib/openlcb/openlcb_defines.h"

/*
 * bootloader_shared_ram.h declares the three shared-SRAM variables that
 * survive a software reset and are read by the bootloader on startup:
 *
 *   bootloader_vivt_jumptable  -- function pointers for interrupt redirect
 *   bootloader_cached_alias    -- CAN alias to reuse after reset
 *   bootloader_request_flag    -- magic value signalling bootloader mode
 *
 * Both this project and the bootloader project compile bootloader_shared_ram.c
 * which pins these variables to fixed SRAM addresses via
 * __attribute__((persistent, address(0xNNNN))).  The two linker scripts map
 * to the same physical SRAM so both binaries see the same bytes.
 */
#include "../../../shared/bootloader_shared_ram.h"

/* ====================================================================== */
/* Factory Reset                                                           */
/*                                                                         */
/* Called when the OpenLCB Config Tool sends a Factory Reset datagram.     */
/* A real product would erase user configuration from NVM here and         */
/* restore defaults.  This demo just prints the requesting node's ID.      */
/* ====================================================================== */

void CallbacksConfigMem_factory_reset(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    printf("Factory Reset: NodeID = 0x%06llX\n", OpenLcbUtilities_extract_node_id_from_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, 0));

}

/* ====================================================================== */
/* Freeze -- Application-to-Bootloader Drop-Back                           */
/*                                                                         */
/* Called when the OpenLCB Config Tool sends a Freeze datagram targeting   */
/* the firmware address space (CONFIG_MEM_SPACE_FIRMWARE = 0xEF).         */
/*                                                                         */
/* On ARM processors (STM32, MSPM0) the bootloader can simply relocate     */
/* VTOR and jump; no special handshake is needed.  On dsPIC33 the hardware */
/* IVT is fixed in flash and always owned by the bootloader, so we need a  */
/* more careful drop-back sequence explained below.                        */
/*                                                                         */
/* THE CORRECT DROP-BACK SEQUENCE (must happen in this exact order):       */
/*                                                                         */
/*  1. Disable global interrupts                                           */
/*     _GIE = 0 hard-disables all interrupts including traps.              */
/*     We never re-enable; asm("RESET") fires with interrupts off.         */
/*                                                                         */
/*  2. Zero every VIVT slot                                                */
/*     The bootloader owns the hardware IVT but routes interrupts to the   */
/*     application via a Virtual Interrupt Vector Table (bootloader_         */
/*     vivt_jumptable) in shared RAM.  While the application is running,   */
/*     these slots hold the application's handler addresses.               */
/*     If we reset without clearing them, the bootloader's first interrupt  */
/*     (e.g. the TMR2 100ms tick) will call into dead application code --  */
/*     corrupting state or hard-faulting.  Clearing them makes the         */
/*     bootloader's ISR stubs safe no-ops during firmware update.          */
/*                                                                         */
/*  3. Cache the CAN alias                                                 */
/*     The bootloader can reuse the application's current CAN alias        */
/*     instead of running the full 200ms alias negotiation sequence.       */
/*     This makes the drop-back invisible to the Config Tool -- it         */
/*     continues communicating with the same alias as before.              */
/*                                                                         */
/*  4. Set the magic flag                                                  */
/*     BOOTLOADER_REQUEST_MAGIC (0xB00710AD) tells the bootloader that     */
/*     it was deliberately requested by the application.  The bootloader   */
/*     reads this flag in is_bootloader_requested() and starts with        */
/*     firmware_active = 1, signalling the Config Tool that it can begin   */
/*     data transfer immediately without sending another Freeze command.   */
/*                                                                         */
/*  5. Software reset                                                      */
/*     asm("RESET") resets the CPU while leaving SRAM intact.              */
/*     The bootloader starts fresh, reads the magic flag and cached alias, */
/*     and enters firmware-update mode immediately.                        */
/*                                                                         */
/* WHY THE VIVT MUST BE CLEARED BEFORE THE MAGIC FLAG IS WRITTEN:         */
/*     Both the VIVT table and the magic flag live in the same shared-SRAM */
/*     region.  initialize_hardware() clears bootloader_request_flag       */
/*     unconditionally and clears bootloader_cached_alias on non-app-      */
/*     dropback resets, but leaves bootloader_cached_alias intact on the   */
/*     app drop-back path so the CAN state machine can reuse the alias.    */
/*     IF we wrote the magic flag first and THEN tried to clear the VIVT,  */
/*     an interrupt firing in that narrow window would see the magic flag   */
/*     already set but VIVT not yet cleared -- inconsistent state.         */
/*     Clearing the VIVT first (step 2) before writing anything (steps 3,4)*/
/*     avoids this race entirely.                                          */
/* ====================================================================== */

void CallbacksConfigMem_freeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        /* Step 1: Hard-disable global interrupts.
         * _GIE = 0 blocks all interrupt priorities including traps.
         * DISI only blocks priority 0-6 for a cycle count -- not sufficient.
         * We never re-enable; asm("RESET") fires with interrupts off. */
        _GIE = 0;

        /* Step 2: Clear every VIVT slot so the bootloader ISR stubs do not
         * call back into dead application code after the software reset.
         * This MUST happen before writing the magic flag (step 4) to avoid
         * an inconsistent state if an NMI or trap fires between those two
         * writes. */
        bootloader_vivt_jumptable.oscillatorfail_handler = 0;
        bootloader_vivt_jumptable.addresserror_handler   = 0;
        bootloader_vivt_jumptable.stackerror_handler     = 0;
        bootloader_vivt_jumptable.matherror_handler      = 0;
        bootloader_vivt_jumptable.dmacerror_handler      = 0;
        bootloader_vivt_jumptable.timer_2_handler        = 0;
        bootloader_vivt_jumptable.can1_handler           = 0;

        /* Step 3: Cache the current CAN alias.
         * statemachine_info->openlcb_node->alias holds the 12-bit alias
         * this node negotiated at startup.  The bootloader recovers it in
         * BootloaderDriversCan_get_cached_alias_passed_from_application()
         * to skip alias negotiation and keep the same alias on the network. */
        bootloader_cached_alias = statemachine_info->openlcb_node->alias;

        /* Step 4: Write the magic handshake flag.
         * The bootloader checks this in is_bootloader_requested().  Matching
         * BOOTLOADER_REQUEST_MAGIC causes it to return BOOTLOADER_REQUESTED_BY_APP,
         * which tells the library to start with firmware_active = 1 so the
         * Config Tool can send firmware data immediately. */
        bootloader_request_flag = BOOTLOADER_REQUEST_MAGIC;

        /* Step 5: Software reset.
         * SRAM is preserved across this reset (POR/BOR bits stay clear).
         * The bootloader reads the values written in steps 3 and 4 above. */
        asm("RESET");

    }

}

/* ====================================================================== */
/* Unfreeze                                                                */
/*                                                                         */
/* Called when the Config Tool sends an Unfreeze datagram after a firmware */
/* update completes without a drop-back reset.  In this demo the           */
/* bootloader handles firmware transfer directly; the application never     */
/* sees the firmware data bytes.  This stub is a placeholder.              */
/* ====================================================================== */

void CallbacksConfigMem_unfreeze(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {

    if (config_mem_operations_request_info->space_info->address_space == CONFIG_MEM_SPACE_FIRMWARE) {

        printf("Requesting Firmware firmware update complete, reboot");

    }

}
