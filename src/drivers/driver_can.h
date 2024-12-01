

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVER_CAN__
#define	__DRIVER_CAN__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../drivers/common/can_types.h"

// Assign the function pointer to where the incoming CAN messages should be dispatched to.
// WARNING: Is in the context of the interrupt, be careful
// void func(uint8_t channel, can_msg_t* can_msg)
typedef void (*can_rx_callback_func_t) (uint8_t, can_msg_t*);

extern void DriverCan_Initialization(can_rx_callback_func_t can_rx_callback_func);

extern uint8_t DriverCan_is_can_tx_buffer_clear(uint16_t Channel);

extern void DriverCan_pause_can_rx();

extern void DriverCan_resume_can_rx();

extern void DriverCan_pause_can_tx_complete_notify();

extern void DriverCan_resume_can_tx_complete_notify();

extern uint8_t DriverCan_transmit_raw_can_frame(uint8_t channel, can_msg_t* msg);

// How full the chips CAN fifo has gotten
extern uint8_t DriverCan_max_can_fifo_depth;

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVER_CAN__ */

