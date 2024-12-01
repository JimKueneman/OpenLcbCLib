

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../driver_configuration_memory.h"

extern uint8_t _25AA1024_Driver_read_status_register();

extern void _25AA1024_Driver_write_status_register(uint8_t new_status);

extern void _25AA1024_Driver_write_latch_enable();

extern void _25AA1024_Driver_write_latch_disable();

extern void _25AA1024_Driver_write_byte(uint32_t address, uint8_t byte);

extern uint16_t _25AA1024_Driver_write(uint32_t address, uint8_t count, DriverConfigurationMemory_buffer_t* buffer);

extern uint8_t _25AA1024_Driver_write_in_progress();

extern uint8_t _25AA1024_Driver_read_byte(uint32_t address);

extern uint16_t _25AA1024_Driver_read(uint32_t address, uint8_t count, DriverConfigurationMemory_buffer_t* buffer);



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

