
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_MAIN_STATEMACHINE__
#define	__CAN_MAIN_STATEMACHINE__

#include <xc.h> // include processor files - each processor file is guarded. 
#include "../../openlcb/openlcb_types.h"

extern void CanMainStatemachine_initialize();

extern void CanMainStateMachine_run();

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_MAIN_STATEMACHINE__ */

