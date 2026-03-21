/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <main.h>
#include "ti_msp_dl_config.h"

static int flag = 1;

/* Check whether swap bank and execute program from upper flash bank */
bool DoFlashSwapBank(){
    while (DL_GPIO_readPins(GPIO_GRP_0_PORT, GPIO_GRP_0_PIN_0_PIN) != GPIO_GRP_0_PIN_0_PIN) {}
    delay_cycles(16000000);
    /*  A short time press of S1 will make the program execute from lower bank (less 0.5s)
     *  A long time press of S1 will make the program execute from upper bank (over 0.5s)
     */
    return (DL_GPIO_readPins(GPIO_GRP_0_PORT, GPIO_GRP_0_PIN_0_PIN) == GPIO_GRP_0_PIN_0_PIN);
}


uint32_t * vector_table_backup;
static void start_app(uint32_t *vector_table)
{
    /* The following code resets the SP to the value specified in the
     * provided vector table, and then the Reset Handler is invoked.
     *
     * Per ARM Cortex specification:
     *
     *           ARM Cortex VTOR
     *
     *
     *   Offset             Vector
     *
     * 0x00000000  ++++++++++++++++++++++++++
     *             |    Initial SP value    |
     * 0x00000004  ++++++++++++++++++++++++++
     *             |         Reset          |
     * 0x00000008  ++++++++++++++++++++++++++
     *             |          NMI           |
     *             ++++++++++++++++++++++++++
     *             |           .            |
     *             |           .            |
     *             |           .            |
     *
     * */

    /* Back up of vector_table to avoid being changed because of SP update */
    vector_table_backup = vector_table;

    /* Set the Reset Vector to the new vector table (Will be reset to 0x000) */
    SCB->VTOR = (uint32_t) vector_table;

    /* Reset the SP with the value stored at vector_table[0] */
    __asm volatile(
        "LDR R3,[%[vectab],#0x0] \n"
        "MOV SP, R3       \n" ::[vectab] "r"(vector_table));

    /* Jump to the Reset Handler address at vector_table[1] */

    ((void (*)(void))(*(vector_table_backup + 1)))();
}

int main(void)
{
    SYSCFG_DL_init();

    if (DL_SYSCTL_isINITDONEIssued())
    {
        start_app((uint32_t *)VECTOR_ADDRESS_BANK0);
    }
    else //Init and SWAP
    {
        if (DoFlashSwapBank()){
            DL_SYSCTL_executeFromUpperFlashBank(); // set flash bank swap bit
            delay_cycles(160);
            DL_SYSCTL_issueINITDONE(); // Issue INITDOEN to trigger System Reset -> swap to bank1
        }else{
            DL_SYSCTL_executeFromLowerFlashBank(); // still execute program from bank0
            delay_cycles(160);
            DL_SYSCTL_issueINITDONE(); // Issue INITDOEN to trigger System Reset -> jump to bank0 app program
        }
    }
}
