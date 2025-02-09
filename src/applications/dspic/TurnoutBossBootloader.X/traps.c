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
 * \file traps.c
 * 
 * Trap code for MicroChip dsPIC33 parts.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include <xc.h>

/* Prototype function declaration for functions in the file */
void __attribute__((__interrupt__)) _OscillatorFail(void);
void __attribute__((__interrupt__)) _AddressError(void);
void __attribute__((__interrupt__)) _StackError(void);
void __attribute__((__interrupt__)) _MathError(void);
void __attribute__((__interrupt__)) _DMACError(void);

void Traps_oscillator_fail_handler(void) {
    
    INTCON1bits.OSCFAIL = 0; //Clear the trap flag
    while (1);
    
    
}

void Traps_address_error_handler(void) {

     INTCON1bits.ADDRERR = 0; //Clear the trap flag
    while (1);
}

void Traps_stack_error_handler(void) {
    
    INTCON1bits.STKERR = 0; //Clear the trap flag
    while (1);

}

void Traps_math_error_handler(void) {
    
        INTCON1bits.MATHERR = 0; //Clear the trap flag
    while (1);

}

void Traps_dmac_error_handler(void) {
    
    INTCON1bits.DMACERR = 0; //Clear the trap flag
    while (1);

}



/*
Primary Exception Vector handlers:
These routines are used if INTCON2bits.ALTIVT = 0.
All trap service routines in this file simply ensure that device
continuously executes code within the trap service routine. Users
may modify the basic framework provided here to suit to the needs
of their application.
 */

/******************************************************************************
 * Function:  void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       Interrupt service routine for oscillator fail.
 *                 Clears the trap flag if OSCFAIL bit is set.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void) {
    
    // Allows a bootloader to call the normal function from it's interrupt
    Traps_oscillator_fail_handler();
}

/******************************************************************************
 * Function:    void __attribute__((interrupt, no_auto_psv)) _AddressError(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:       Interrupt service routine for Address Error.
 *                 Clears the trap flag if AddressErrr bit is set.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _AddressError(void) {
   
    // Allows a bootloader to call the normal function from it's interrupt
    Traps_address_error_handler();
    
}

/******************************************************************************
 * Function:      void __attribute__((interrupt, no_auto_psv)) _StackError(void)
 *
 * PreCondition:  None
 *
 * Input:         None
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      Interrupt service routine for stack error.
 *                Clears the trap flag if _StackError bit is set.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _StackError(void) {
    
    // Allows a bootloader to call the normal function from it's interrupt
    Traps_stack_error_handler();
    
}

/******************************************************************************
 * Function:       void __attribute__((interrupt, no_auto_psv)) _MathError(void)
 *
 * PreCondition:   None
 *
 * Input:          None
 *
 * Output:         None
 *
 * Side Effects:   None
 *
 * Overview:       Interrupt service routine for Math Error.
 *                 Clears the trap flag if _MathError bit is set.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _MathError(void) {
    
    // Allows a bootloader to call the normal function from it's interrupt
    Traps_math_error_handler();

}

/******************************************************************************
 * Function:       void __attribute__((interrupt, no_auto_psv)) _DMACError(void)
 *
 * PreCondition:   None
 *
 * Input:          None
 *
 * Output:         None
 *
 * Side Effects:   None
 *
 * Overview:       Interrupt service routine for DMACError.
 *                 Clears the trap flag if _DMACError bit is set.
 *****************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _DMACError(void) {
    
    // Allows a bootloader to call the normal function from it's interrupt
    Traps_dmac_error_handler();
    
}

/*******************************************************************************
 End of File
 */
