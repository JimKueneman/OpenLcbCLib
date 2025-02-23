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
 * \file turnoutboss_bootloader_traps.c
 * 
 * Trap code for MicroChip dsPIC33 parts.
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


#include <xc.h>
#include "../TurnoutBossCommon/common_loader_app.h"



void __attribute__((interrupt, no_auto_psv)) _OscillatorFail(void) {
    
    INTCON1bits.OSCFAIL = 0; //Clear the trap flag

    if (CommonLoaderApp_interrupt_redirect) {
   
        if (CommonLoaderApp_jumptable.oscillatorfail_hander)
            CommonLoaderApp_jumptable.oscillatorfail_hander();
        

    } else {

        while (1);

    }
}

void __attribute__((interrupt, no_auto_psv)) _AddressError(void) {
    
    INTCON1bits.ADDRERR = 0; //Clear the trap flag

    if (CommonLoaderApp_interrupt_redirect) {

        if (CommonLoaderApp_jumptable.addresserror_hander)
            CommonLoaderApp_jumptable.addresserror_hander();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _StackError(void) {
    
    INTCON1bits.STKERR = 0; //Clear the trap flag

    if (CommonLoaderApp_interrupt_redirect) {
        
        if (CommonLoaderApp_jumptable.stackerror_hander)
            CommonLoaderApp_jumptable.stackerror_hander();

    } else {

        while (1);

    }

}

void __attribute__((interrupt, no_auto_psv)) _MathError(void) {
    
    INTCON1bits.MATHERR = 0; //Clear the trap flag

    if (CommonLoaderApp_interrupt_redirect) {

        if (CommonLoaderApp_jumptable.matherror_hander)
            CommonLoaderApp_jumptable.matherror_hander();

    } else {

        while (1);

    }


}

void __attribute__((interrupt, no_auto_psv)) _DMACError(void) {

    INTCON1bits.DMACERR = 0; //Clear the trap flag

    if (CommonLoaderApp_interrupt_redirect) {

        if (CommonLoaderApp_jumptable.dmacerror_hander)
            CommonLoaderApp_jumptable.dmacerror_hander();

    } else {

        while (1);

    }

}

