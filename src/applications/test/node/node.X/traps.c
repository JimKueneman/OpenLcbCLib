
/*
 * File:   traps.c
 * Author: jimkueneman
 *
 * Created on November 6, 2024, 5:44 AM
 */


#include <xc.h>

/* Prototype function declaration for functions in the file */
void __attribute__ ( (__interrupt__) )  _OscillatorFail( void );
void __attribute__ ( (__interrupt__) )  _AddressError( void );
void __attribute__ ( (__interrupt__) )  _StackError( void );
void __attribute__ ( (__interrupt__) )  _MathError( void );
void __attribute__ ( (__interrupt__) )  _DMACError( void );

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
void __attribute__ ( (interrupt, no_auto_psv) ) _OscillatorFail( void )
{
    INTCON1bits.OSCFAIL = 0;    //Clear the trap flag
    while( 1 );
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
void __attribute__ ( (interrupt, no_auto_psv) ) _AddressError( void )
{
    INTCON1bits.ADDRERR = 0;    //Clear the trap flag
    while( 1 );
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
void __attribute__ ( (interrupt, no_auto_psv) ) _StackError( void )
{
    INTCON1bits.STKERR = 0;     //Clear the trap flag
    while( 1 );
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
void __attribute__ ( (interrupt, no_auto_psv) ) _MathError( void )
{
    INTCON1bits.MATHERR = 0;    //Clear the trap flag
    while( 1 );
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
void __attribute__ ( (interrupt, no_auto_psv) ) _DMACError( void )
{
    INTCON1bits.DMACERR = 0;    //Clear the trap flag
    while( 1 );
}

/*******************************************************************************
 End of File
*/
