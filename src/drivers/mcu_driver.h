
/* 
 * File: mcu_driver.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __MCU_DRIVER__
#define	__MCU_DRIVER__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "../drivers/common/can_types.h"


// Timer -----------------------------------------------------------------------


extern void McuDriver_initialization(void);

extern uint8_t McuDriver_is_can_tx_buffer_clear(uint16_t Channel);

extern void McuDriver_pause_can_rx();
extern void McuDriver_resume_can_rx();

extern void McuDriver_pause_can_tx_complete_notify();
extern void McuDriver_resume_can_tx_complete_notify();

extern void McuDriver_pause_100ms_timer();
extern void McuDriver_resume_100ms_timer();

extern uint8_t McuDriver_transmit_raw_can_frame(uint8_t channel, can_msg_t* msg);


// How full the chips CAN fifo has gotten
extern uint8_t max_can_fifo_depth;


// Assign the function pointer to where the incoming CAN messages should be dispatched to.
// WARNING: Is in the context of the interrupt, be careful
// void func(uint8_t channel, can_msg_t* can_msg)
typedef void (*incoming_can_message_callback_func_t) (uint8_t, can_msg_t*);
extern incoming_can_message_callback_func_t Incoming_CAN_Message_Callback_Func;


// Assign the function pointer to where the timer tick should call out to
// WARNING: Is in the context of the interrupt, be careful
// void func()
typedef void (*_100ms_timer_callback_func_t) ();
extern _100ms_timer_callback_func_t _100ms_Timer_Callback_Func;

// Assign the function pointer to where the UART Rx should call back with the byte it received
// WARNING: Is in the context of the interrupt, be careful
// void func(rx_data);
typedef void (*uart_rx_callback_func_t) (uint16_t);
extern uart_rx_callback_func_t Uart_Rx_Callback_Func;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __MCU_DRIVER__ */

