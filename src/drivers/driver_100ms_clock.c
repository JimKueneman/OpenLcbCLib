/*
 * File:   driver_100ms_clock.c
 * Author: jimkueneman
 *
 * Created on December 1, 2024, 6:58 AM
 */


#include "xc.h"

#include "driver_100ms_clock.h"



_100ms_timer_callback_func_t Driver100msClock_callback_func;


void Driver100msClock_Initialization(_100ms_timer_callback_func_t _100ms_callback_func) {
    
    Driver100msClock_callback_func = _100ms_callback_func;
    
    // Timer Initialize --------------------------------------------------------
    // -------------------------------------------------------------------------

    IPC1bits.T2IP0 = 1; // Timer 2 Interrupt Priority = 5   (1 means off)
    IPC1bits.T2IP1 = 0;
    IPC1bits.T2IP2 = 1;

    T2CONbits.TCS = 0; // internal clock
    T2CONbits.TCKPS0 = 1; // 256 Prescaler
    T2CONbits.TCKPS1 = 1;
    PR2 = 15625; // Clock ticks every (1/80MHz * 2 * 256 * 15625 = 100.00091ms interrupts

    IFS0bits.T2IF = 0; // Clear T2IF
    IEC0bits.T2IE = 1; // Enable the Interrupt

    T2CONbits.TON = 1; // Turn on 100ms Timer

    // -------------------------------------------------------------------------
    
    
}

// Timer 2 Interrupt 100ms timer

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF

    // Increment any timer counters assigned
    if (Driver100msClock_callback_func)
        Driver100msClock_callback_func();

    return;
}


void Driver100msClock_pause_100ms_timer() {
  
    T2CONbits.TON = 0; // Turn off 100ms Timer
    
}

extern void Driver100msClock_resume_100ms_timer() {
    
    T2CONbits.TON = 1; // Turn on 100ms Timer
    
}

