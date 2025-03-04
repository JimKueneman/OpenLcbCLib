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
 * \turnoutboss_drivers.c
 *
 *  
 *
 * @author Jim Kueneman
 * @date 4 Jan 2025
 */

#include "turnoutboss_drivers.h"

#include "xc.h"
#include "stdio.h"  // printf
#include <libpic30.h> // delay

#include "local_drivers/_25AA1024/25AA1024_driver.h"
#include "local_drivers/_MCP23S17/MCP23S17_driver.h"
#include "turnoutboss_hardware_handler.h"
#include "../dsPIC_Common/ecan1_helper.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#include "turnoutboss_teach_learn.h"
#include "turnoutboss_drivers.h"

uart_rx_callback_t _uart_rx_callback_func = (void*) 0;
parameterless_callback_t _100ms_timer_sink_func = (void*) 0;
parameterless_callback_t _signal_update_timer_sink_func = (void*) 0;

void TurnoutBossDrivers_setup(parameterless_callback_t _100ms_timer_sink) {

    _100ms_timer_sink_func = _100ms_timer_sink;

    CommonLoaderApp_initialize_sfrs();
    Ecan1Helper_initialization();
    MCP23S17Driver_initialize();
    _25AA1024_Driver_initialize(EEPROM_ADDRESS_SIZE_IN_BITS);

}

void TurnoutBossDrivers_set_signal_update_timer_sink(parameterless_callback_t signal_update_timer_sink_func) {

    _signal_update_timer_sink_func = signal_update_timer_sink_func;

}

void TurnoutBossDrivers_reboot(void) {

    asm("RESET ");

}

void TurnoutBossDrivers_assign_uart_rx_callback(uart_rx_callback_t uart_rx_callback) {

    _uart_rx_callback_func = uart_rx_callback;

}

uint16_olcb_t TurnoutBossDrivers_config_mem_read(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    if (CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes) {
        
        T3CONbits.TON = 0; // Turn off Timer  don't count these dumps in the timing 
        
    }
    
    // Don't let there be an overlap of the signals being written out to the Port Expander within Timer 1 and the need to access the EEPROM
    TurnoutBossDrivers_pause_signal_calculation_timer();

    return _25AA1024_Driver_read(address, count, buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

    TurnoutBossDrivers_resume_signal_calculation_timer();
    
    if (CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes) {
        
        T3CONbits.TON = 1; // Turn off Timer  don't count these dumps in the timing 
        
    }

}

uint16_olcb_t TurnoutBossDrivers_config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    uint16_olcb_t page_buffer_index;
    uint32_olcb_t running_address;
    uint16_olcb_t buffer_index;
    uint8_olcb_t page_buffer[EEPROM_PAGE_SIZE_IN_BYTES];
    uint16_olcb_t page_start_address;
    uint16_olcb_t end_address;
    
    if (CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes) {
        
        T3CONbits.TON = 0; // Turn off Timer  don't count these dumps in the timing 
        
    }

    // Don't let there be an overlap of the signals being written out to the Port Expander within Timer 1 and the need to access the EEPROM
    TurnoutBossDrivers_pause_signal_calculation_timer();

    running_address = address;
    buffer_index = 0;
    end_address = address + count;
    
    while (running_address < end_address) {

        page_buffer_index = 0;
        page_start_address = running_address;

        while (((running_address % EEPROM_PAGE_SIZE_IN_BYTES != 0) || (page_buffer_index == 0)) && (running_address < end_address)) {

            page_buffer[page_buffer_index] = (*buffer)[buffer_index];

            buffer_index = buffer_index + 1;
            page_buffer_index = page_buffer_index + 1;
            running_address = running_address + 1;

        };
        
        _25AA1024_Driver_write_latch_enable();

        _25AA1024_Driver_write(page_start_address, page_buffer_index, (configuration_memory_buffer_t*) & page_buffer, EEPROM_ADDRESS_SIZE_IN_BITS);

        while (_25AA1024_Driver_write_in_progress()) {

            __delay32(1000); // 25AA08 seems to be sensitive to how fast you check the register... it will lock up  

        }
    }

    TurnoutBossDrivers_resume_signal_calculation_timer();
    
    if (CommonLoaderApp_max_application_loop_delay_ignore_config_mem_writes) {
        
        T3CONbits.TON = 1; // Turn 1 Timer  don't count these dumps in the timing 
        
    }

    return count;

}

void TurnoutBossDrivers_pause_100ms_timer(void) {

    T2CONbits.TON = 0; // Turn off 100ms Timer

}

void TurnoutBossDrivers_resume_100ms_timer(void) {

    T2CONbits.TON = 1; // Turn 0n 100ms Timer

}

void TurnoutBossDrivers_pause_signal_calculation_timer(void) {

    T1CONbits.TON = 0; // Turn off 

}

void TurnoutBossDrivers_resume_signal_calculation_timer(void) {

    T1CONbits.TON = 1; // Turn on 

}

void TurnoutBossDrivers_u1_tx_interrupt_handler(void) {



}

void TurnoutBossDrivers_u1_rx_interrupt_handler(void) {

    uint16_olcb_t value;

    if (U1STAbits.URXDA == 1) {

        value = U1RXREG; // read it so it does not fill and overflow

        if (_uart_rx_callback_func)
            _uart_rx_callback_func(value);

    }

}

void TurnoutBossDrivers_t1_interrupt_handler(void) {

    if (_signal_update_timer_sink_func)
        _signal_update_timer_sink_func();

}

void TurnoutBossDrivers_t2_interrupt_handler(void) {

    TurnoutBossTeachLearn_update_leds(TurnoutBossTeachLearn_teach_learn_state.state);

    // Increment any timer counters assigned
    if (_100ms_timer_sink_func)
        _100ms_timer_sink_func();

}

void __attribute__((interrupt(no_auto_psv))) _U1TXInterrupt(void) {

    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag

    // Allows a bootloader to call the normal function from it's interrupt
    TurnoutBossDrivers_u1_tx_interrupt_handler();

}

void __attribute__((interrupt(no_auto_psv))) _U1RXInterrupt(void) {

    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag 

    // Allows a bootloader to call a normal function from it's interrupt
    TurnoutBossDrivers_u1_rx_interrupt_handler();

}

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF

    // Allows a bootloader to call the normal function from it's interrupt
    TurnoutBossDrivers_t2_interrupt_handler();

}

void __attribute__((interrupt(no_auto_psv))) _T1Interrupt(void) {

    IFS0bits.T1IF = 0; // Clear T1IF

    // Allows a bootloader to call the normal function from it's interrupt
    TurnoutBossDrivers_t1_interrupt_handler();

}
