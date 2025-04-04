/*
 * File:   pca9685_driver.c
 * Author: jimkueneman
 *
 * Created on April 4, 2025, 6:25 AM
 */

#include "pca9685_driver.h"

#include <libpic30.h>
#include "i2c_driver.h";


//void main(){
//    // block variables
//    unsigned char              n = 0;   // LED flash
//    unsigned int            loop = 0;   // loop counter
//    unsigned int             pwm = 0;
//    unsigned int                 x=0;
//    // set up ports
//    ADCON1 = 0b00001110;                // A0 is analog
//    CMCON  = 0b00000111;                // disable comparators
//    // set up peripherals
//    I2C1_Init(100000);
//    PCA9685_init();
//    // ports
//    INTCON2.RBPU = 1;                   // PORTB pullups disabled
//    TRISA = 0b00000001;                 // A0 is A/D input
//    TRISC = 0b11111111;                 // I2C2 pins must be inputs
//
//    while(1){
//        // Ramp SERVO up
//        for(pwm=230;pwm<=902;pwm++){
//            PCA9685_send(pwm,1);
//            delay_ms(1);
//        }
//        // Ramp SERVO down
//        for(pwm=902;pwm>=230;pwm--){
//            PCA9685_send(pwm,1);
//            delay_ms(1);
//        }
//        
//        // Use byte write to set LED15 to 50% duty cycle 4096/2 = 2046
//        // 2046 = 0x07FE so hi_byte=0x07 and lo_byte = 0xFE
//        PCA9685_write_byte(0x42,0);     // turn on low byte
//        PCA9685_write_byte(0x43,0);     // turn on high byte
//        PCA9685_write_byte(0x44,0xFE);  // turn off low byte
//        PCA9685_write_byte(0x45,0x07);  // turn off high byte
//        
//        // Use word write to set LED14 to 10% duty cycle 4096/10 = 409
//        // 409 = 0x0199
//        PCA9685_Write_word(0x3E,0x0000);// turn on at time 0
//        PCA9685_Write_word(0x40,0x0199);// turn off at time 409 0r 0x0199
//        
//        // Use byte read to get a register value
//        // Must bracket the shift because of precedence
//        x = (PCA9685_read_byte(0x45)<<8) + PCA9685_read_byte(0x44) ;
//        
//        // Housekeep
//        HEARTBEAT = ~HEARTBEAT;
//        loop++;
//        delay_ms(20);
//
//    }
//}



////////////////////////////////////////////////////////////////////////////////
// Functions
// Init the chip with pwm frequency and MODE2 settings

void PCA9685_init() {
    
    I2C1_start(); // Start
    I2C1_wr(Write_address); // Slave Write_address
    I2C1_wr(MODE1); // Mode 1 ADDRESS
    I2I2C1_wrC1_Wr(0b00110001); // Sleep and change default PWM frequency
    I2C1_stop(); // Stop
    __delay_ms(1); // Required 50 us delay
    I2C1_start(); // Start
    I2C1_wr(Write_address); // Slave Write_address
    I2C1_wr(0xFE); // PWM frequency PRE_SCALE ADDRESS to set pwm at 100Hz
    I2C1_wr(0x3C); // Osc_clk/(4096*update_rate)=25000000/(4096*100)=60=0x3C
    I2C1_stop(); // Stop
    __delay_ms(1); // delay at least 500 us
    I2C1_start(); // Start
    I2C1_wr(Write_address); // Slave Write_address
    I2C1_wr(MODE1); // Mode 1 register ADDRESS
    I2C1_wr(0b10100001); // Set MODE1
    I2C1_stop(); // Stop
    __delay_ms(1); // delay at least 500 us
    I2C1_start(); // Start
    I2C1_wr(Write_address); // Slave Address
    I2C1_wr(MODE2); // Mode2 register ADDRESS
    I2C1_wr(0b00000100); // Set MODE2
    I2C1_stop(); //
    
}

void PCA9685_send(unsigned int value, unsigned char led) {

    unsigned char pulse_length; // temp variable for PWM
    I2C1_start(); // Start
    I2C1_wr(Write_address); // address of selected pca9685
    I2C1_wr(LED0 + 4 * led); // select selected LED ADDRESS
    I2C1_wr(0x00); // LED_ON_L
    I2C1_wr(0x00); // LED_ON_H
    pulse_length = value; // PWM value lo byte
    I2C1_wr(pulse_length); // LED_OFF_L
    pulse_length = value >> 8; // pwm 16 bit long, now shift upper 8 to lower 8
    I2C1_wr(pulse_length); // LED_OFF_H
    I2C1_stop(); // stop

}

void PCA9685AllLedOff() {

    I2C1_start(); // atart
    I2C1_wr(Write_address); // select pca9685
    I2C1_wr(ALL_CH_OFF_L_reg); // All LEDs Off regiter
    I2C1_wr(0b00000000); // low byte
    I2C1_wr(0b00010000); // high byte, bit4 set so full_off see page 21
    I2C1_stop(); // Stop

}
// Read a byte and return it's value

unsigned char PCA9685_read_byte(unsigned char chip_register) {
    
    unsigned char temp = 0x00;
    
    I2C1_start();
    I2C1_wr(Write_address);
    I2C1_wr(chip_register);
    I2C1_start();
    I2C1_wr(Read_address);
    temp = I2C1_rd(0);
    I2C1_stop();
    
    return temp;
    
}

void PCA9685_write_byte(unsigned char chip_register, unsigned char value) {
    
    I2C1_start();
    I2C1_wr(Write_address);
    I2C1_wr(chip_register);
    I2C1_wr(value);
    I2C1_stop();
    
}
// Write 16bits to chip_register, increments automatically from lo to hi byte

void PCA9685_write_word(unsigned char chip_register, unsigned int word_value) {
    
    unsigned char hb = 0x00;
    unsigned char lb = 0x00;
    lb = (word_value & 0x00FF);
    hb = ((word_value & 0xFF00) >> 0x08);
    PCA9685_write_byte(chip_register, lb);
    PCA9685_write_byte((chip_register + 1), hb);
    
}
// Soft re-set

void PCA9685_soft_reset() {
    
    I2C1_start();
    I2C1_wr(0x00);
    I2C1_wr(PCA9685_software_reset);
    I2C1_stop();
    
}