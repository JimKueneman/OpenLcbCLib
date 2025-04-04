/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __PCA9685_DRIVER__
#define	__PCA9685_DRIVER__

#define Write_address 0x80  // I2C address for PCA9865 with no solder bridges
#define Read_address Write_address+1
#define PCA9685_software_reset 0x06
#define Reset   0x01        // Reset the device
#define MODE1   0x00        // 0x00 location for Mode1 register address
#define MODE2   0x01        // 0x01 location for Mode2 reigster address
#define LED0    0x06        // location for start of LED0 registers
#define ALL_CH_ON_L_reg   0xFA
#define ALL_CH_ON_H_reg   0xFB
#define ALL_CH_OFF_L_reg  0xFC
#define ALL_CH_OFF_H_reg  0xFD
#define HEARTBEAT LATH0_bit // green led on ETT board


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    
extern void PCA9685_init();

extern void PCA9685_send(unsigned int value, unsigned char led);

extern void PCA9685AllLedOff();

extern unsigned char PCA9685_read_byte(unsigned char chip_register);

extern void PCA9685_write_byte(unsigned char chip_register, unsigned char value);

extern void PCA9685_write_word(unsigned char chip_register, unsigned int word_value);

extern void PCA9685_soft_reset();
 
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __PCA9685_DRIVER__ */

