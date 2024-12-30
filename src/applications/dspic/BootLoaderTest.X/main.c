/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.5
        Device            :  dsPIC33EP256GP504
    The generated drivers are tested against the following:
        Compiler          :  XC16 v2.10
        MPLAB 	          :  MPLAB X v6.05
 */

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
 */

/**
  Section: Included Files
 */
//#include "mcc_generated_files/system.h"
#include "mcc_generated_files/memory/flash_demo.h"
#include "mcc_generated_files/memory/flash.h"
#include "stdio.h"

/*
                         Main application
 */

uint32_t flash_storage_address;
bool result;

int main(void) {
    // initialize the device
    //  SYSTEM_Initialize();

    flash_storage_address = FLASH_GetErasePageAddress(0x7000);
    
   printf("flash_storage_address: 0x%04X", (uint16_t) (flash_storage_address >> 16) & 0xFFFF);
    printf("%04X\n", (uint16_t) (flash_storage_address & 0xFFFF));
    
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    
    for (int flashOffset = 0; flashOffset < FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS; flashOffset += 4) {
        
        result = FLASH_WriteDoubleWord24(flash_storage_address+flashOffset, 0xAAAAAAAA, 0xBBBBBBBB);
        
        if (! result) 
            printf("Failed\n");
        
    }
    
    FLASH_Lock();
    
    printf("done\n");

    while (1) {
  
        // Check for a new image at 0x14000
        // if so 
        //    copy it to address 0
        //    update the first byte to jump to 0x28000 so a reset calls the bootloader first
        //    erase the copied image from 0x14000 to 0x27FFF
        // jump to the start of the normal program at 0x200
        
        asm("GOTO 0x0200");
        
        asm("GOTO 0x18000");
        
asm("GOTO 0x28000");

        
        
    }
    return 1;
}
/**
 End of File
 */

