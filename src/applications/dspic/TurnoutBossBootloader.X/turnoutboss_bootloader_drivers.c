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
 * \turnoutboss_bootloader_drivers.c
 *
 *  
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "turnoutboss_bootloader_drivers.h"

#include "xc.h"
#include "stdio.h"  // printf
#include <libpic30.h> // delay

#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "turnoutboss_bootloader_ecan1_helper.h"
#include "../TurnoutBossCommon/common_debug_helper.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#include "mcc_generated_files/memory/flash.h"

uart_rx_callback_t _uart_rx_callback_func = (void*) 0;
parameterless_callback_t _100ms_timer_sink_func = (void*) 0;

void TurnoutBossBootloaderDrivers_setup(parameterless_callback_t _100ms_timer_sink) {

    _100ms_timer_sink_func = _100ms_timer_sink;

    CommonLoaderApp_initialize_sfrs();
    TurnoutbossBootloader_ecan1helper_initialization();
    _25AA1024_Driver_initialize(EEPROM_ADDRESS_SIZE_IN_BITS);

}

void TurnoutBossBootloaderDrivers_reboot(void) {

    asm("RESET ");

}

void TurnoutBossBootloaderDrivers_factory_reset(void) {

   

}

void TurnoutBossBootloaderDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback) {

    _uart_rx_callback_func = uart_rx_callback;

}

uint16_olcb_t TurnoutBossBootloaderDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    return _25AA1024_Driver_read(address, count, buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

}

uint16_olcb_t TurnoutBossBootloaderDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    _25AA1024_Driver_write_latch_enable();
    _25AA1024_Driver_write(address, count, buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

    while (_25AA1024_Driver_write_in_progress()) {

    }

    return count;

}

void TurnoutBossBootloaderDrivers_pause_100ms_timer() {

    T2CONbits.TON = 0; // Turn off 100ms Timer

}

void TurnoutBossBootloaderDrivers_resume_100ms_timer() {

    T2CONbits.TON = 1; // Turn off 100ms Timer

}

void TurnoutBossBootloaderDrivers_u1_tx_interrupt_handler(void) {

}

void TurnoutBossBootloaderDrivers_u1_rx_interrupt_handler(void) {

    if (U1STAbits.URXDA == 1) {

        if (_uart_rx_callback_func)
            _uart_rx_callback_func(U1RXREG);

    }

}

void TurnoutBossBootloaderDrivers_t2_interrupt_handler(void) {

    // Increment any timer counters assigned
    if (_100ms_timer_sink_func)
        _100ms_timer_sink_func();

}

void TurnoutBossBootloaderDrivers_t1_interrupt_handler(void) {


}

void __attribute__((interrupt(no_auto_psv))) _U1TXInterrupt(void) {

    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag  

    if (CommonLoaderApp_bootloader_state.interrupt_redirect) {

//        // Create a variable on the stack and grab the address of the U1 TX handler
//        uint16_t applicationISRAddress = __builtin_tblrdl(VIVT_ADDRESS_U1_TX_INTERRUPT); // Where the UART TX Interrupt Handler is in the Application
//
//        // Create a function pointer variable on the stack
//        void (*app_u1_tx_interrupt_func)() = (void*) applicationISRAddress;
//
//        app_u1_tx_interrupt_func();
        
        if (CommonLoaderApp_jumptable.u1_tx_hander)
            CommonLoaderApp_jumptable.u1_tx_hander();


    } else {

        TurnoutBossBootloaderDrivers_u1_tx_interrupt_handler();

    }

}

void __attribute__((interrupt(no_auto_psv))) _U1RXInterrupt(void) {

    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag 

    if (CommonLoaderApp_bootloader_state.interrupt_redirect) {

//        // Create a variable on the stack and grab the address of the U1 RX handler
//        uint16_t applicationISRAddress = __builtin_tblrdl(VIVT_ADDRESS_U1_RX_INTERRUPT); // Where the UART RX Interrupt Handler is in the Application
//
//        // Create a function pointer variable on the stack
//        void (*app_u1_rx_interrupt_func)() = (void*) applicationISRAddress;
//
//        app_u1_rx_interrupt_func();
        
        
        if (CommonLoaderApp_jumptable.u1_rx_hander)
            CommonLoaderApp_jumptable.u1_rx_hander();


    } else {

        TurnoutBossBootloaderDrivers_u1_rx_interrupt_handler();

    }

}

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF
   
    if (CommonLoaderApp_bootloader_state.interrupt_redirect) {

//        // Create a variable on the stack and grab the address of the T2 handler
//        uint16_t applicationISRAddress = __builtin_tblrdl(VIVT_ADDRESS_T2_INTERRUPT); // Where the T2 Interrupt Handler is in the Application
//        
//        uint16_t applicationISRAddress = FLASH_ReadWord16(VIVT_ADDRESS_T2_INTERRUPT);
//
//        // Create a function pointer variable on the stack
//        void (*app_interrupt_t2_func)() = (void*) applicationISRAddress;
//
//        app_interrupt_t2_func();

        
        
        
        if (CommonLoaderApp_jumptable.timer_2_hander) {
            
#ifdef BOSS1
            LED = !LED;
#endif
            CommonLoaderApp_jumptable.timer_2_hander();
            
        }
            

    } else {

        TurnoutBossBootloaderDrivers_t2_interrupt_handler();

    }

}

void __attribute__((interrupt(no_auto_psv))) _T1Interrupt(void) {

    IFS0bits.T1IF = 0; // Clear T2IF
   
    if (CommonLoaderApp_bootloader_state.interrupt_redirect) {
   
        if (CommonLoaderApp_jumptable.timer_1_hander) {
            
            CommonLoaderApp_jumptable.timer_1_hander();
            
        }
            

    } else {

        TurnoutBossBootloaderDrivers_t1_interrupt_handler();

    }

}
