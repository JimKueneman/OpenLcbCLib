/*
 * File:   main.c
 * Author: jimkueneman
 *
 * Created on February 7, 2025, 11:45 AM
 */


#include "xc.h"


// DSPIC33EP512GP504 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define AppStartAddress 0x4000
#define ResetVectorSize 0x0004

#define OscillatorFailInterruptOffset 0
#define AddressErrorInterruptOffset 2
#define StackErrorInterruptOffset 4
#define MathErrorInterruptOffset 6
#define DMACErrorInterruptOffset 8
#define T2InterruptOffset 10
#define U1RXInterruptOffset 12
#define U1TXInterruptOffset 14
#define C1InterruptOffset 16

uint16_t writing_application = 0;

void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void) {

    if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + OscillatorFailInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }
}

void __attribute__((interrupt, no_auto_psv)) _AddressError(void) {

    if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + AddressErrorInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _StackError(void) {

     if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + StackErrorInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _MathError(void) {

     if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + MathErrorInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _DMACError(void) {

    if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + DMACErrorInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {

     if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + T2InterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {

     if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + U1RXInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {

     if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + U1TXInterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

void __attribute__((interrupt, no_auto_psv)) _C1Interrupt(void) {

    if (!writing_application) {
        
       uint16_t address =  __builtin_tblrdl((AppStartAddress + ResetVectorSize) + C1InterruptOffset);
       
       if (address) {
           
           ((void(*)())address)();
           
       }

    }

}

int main(void) {


    while (1) {


    }


}
