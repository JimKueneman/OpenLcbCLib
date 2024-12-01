
/* 
 * File: mcu_driver.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __MCU_DRV__
#define	__MCU_DRV__

#include <xc.h> // include processor files - each processor file is guarded. 


// Timer -----------------------------------------------------------------------

extern void McuDriver_initialization(void);


// Assign the function pointer to where the UART Rx should call back with the byte it received
// WARNING: Is in the context of the interrupt, be careful
// void func(rx_data);
typedef void (*uart_rx_callback_t) (uint16_t);
extern uart_rx_callback_t McuDriver_uart_rx_callback_func;


#endif	/* XC_HEADER_TEMPLATE_H */

