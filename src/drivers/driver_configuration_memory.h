

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __DRIVER_CONFIGURATION_MEMORY__
#define	__DRIVER_CONFIGURATION_MEMORY__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../drivers/common/can_types.h"

typedef uint8_t DriverConfigurationMemory_buffer_t[64];

extern void DriverConfigurationMemory_initialization();

extern uint16_t DriverConfigurationMemory_read(uint32_t address, uint16_t count, DriverConfigurationMemory_buffer_t* buffer);

extern uint16_t DriverConfigurationMemory_write(uint32_t address, uint16_t count, DriverConfigurationMemory_buffer_t* buffer);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __DRIVER_CONFIGURATION_MEMORY__ */

