/*
 * File:   trainboss_main.c
 * Author: jimkueneman
 *
 * Created on March 13, 2025, 1:49 PM
 */


#include "xc.h"

#include "../../../drivers/nmra_dcc/nmra_dcc_driver.h"


void _one_bit_lo_callback(uint16_olcb_t timing_in_us) {
    
    
}

void _one_bit_hi_callback(uint16_olcb_t timing_in_us) {
    
    
}


void _zero_bit_lo_callback(uint16_olcb_t timing_in_us) {
    
    
}


void _zero_bit_hi_callback(uint16_olcb_t timing_in_us) {
    
    
}


void _high_z_cutout_callback(uint16_olcb_t timing_in_us) {
    
    
}



int main(void) {
    
    NmraDccDriver_initalize(
            &_one_bit_lo_callback,
            &_one_bit_hi_callback,
            &_zero_bit_lo_callback,
            &_zero_bit_hi_callback,
            &_high_z_cutout_callback
            );
    
    while (1) {
        
        
        
        
        
    }
   
}
