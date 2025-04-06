/*
 * File:   pca9685.c
 * Author: jimkueneman
 *
 * Created on April 6, 2025, 8:07 AM
 */


#include "pca9685.h"

#include "i2c1.h"

i2c_message_t PCA9685_i2c_message;

void PCA9685_initialize(void) {

    I2C1_initialize_message(&PCA9685_i2c_message);
    
}

void PCA9685_set_auto_register_increment(uint8_olcb_t do_set) {
    
    // Really need to read/modify/write here
    
    I2C1_initialize_message(&PCA9685_i2c_message);
    PCA9685_i2c_message.address = PCA9685_SIGNAL_ADDRESS;
    PCA9685_i2c_message.reg = PCA9685_MODE1;
    PCA9685_i2c_message.length = 1;
    PCA9685_i2c_message.write = TRUE;
    PCA9685_i2c_message.sent = 0;
    PCA9685_i2c_message.data[0] = MODE1_DEFAULT;
     
    if (do_set) {
        
        PCA9685_i2c_message.data[0] = PCA9685_i2c_message.data[0] | PCA9685_MODE1_AUTO_INC_ENABLED_BIT;       
    }

    I2C1_start_transmission_and_run_to_completion(&PCA9685_i2c_message);

}

void PCA9685_set_invert_outputs_increment(uint8_olcb_t do_set) {
    
    // Really need to read/modify/write here
    
    I2C1_initialize_message(&PCA9685_i2c_message);
    PCA9685_i2c_message.address = PCA9685_SIGNAL_ADDRESS;
    PCA9685_i2c_message.reg = PCA9685_MODE2;
    PCA9685_i2c_message.length = 1;
    PCA9685_i2c_message.write = TRUE;
    PCA9685_i2c_message.sent = 0;
    PCA9685_i2c_message.data[0] = MODE2_DEFAULT; 
    
    if (do_set) {
        
        PCA9685_i2c_message.data[0] = PCA9685_i2c_message.data[0] | PCA9685_MODE2_INVERT_ENABLED_BIT;    
        
    }
    
    I2C1_start_transmission_and_run_to_completion(&PCA9685_i2c_message);
    
}

void PCA9685_set_prescaler(uint8_olcb_t scaler) {
    
    // Really need to read/modify/write here
    
    // TODO
    //   Need to make sure the oscillator is not running before setting this.  Mode1 SLEEP must = 1
    
//    I2C1_initialize_message(&PCA9685_i2c_message);
//    PCA9685_i2c_message.address = PCA9685_SIGNAL_ADDRESS;
//    PCA9685_i2c_message.reg = PCA9685_MODE2;
//    PCA9685_i2c_message.length = 1;
//    PCA9685_i2c_message.write = TRUE;
//    PCA9685_i2c_message.sent = 0;
//    PCA9685_i2c_message.data[0] = scaler; 
//
//    I2C1_start_transmission_and_run_to_completion(&PCA9685_i2c_message);
    
}

void PCA9685_update_signals(void) {
  
    I2C1_start_transmission_and_run_to_completion(&PCA9685_i2c_message);

}