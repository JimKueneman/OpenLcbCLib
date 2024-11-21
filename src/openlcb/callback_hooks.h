

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CALLBACK_HOOKS__
#define	__CALLBACK_HOOKS__

#include <xc.h> // include processor files - each processor file is guarded.  

// Node Alias and Node ID
typedef void (*alias_change_callback_t) (uint16_t, uint64_t);


extern alias_change_callback_t CallbackHooks_alias_change;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CALLBACK_HOOKS__ */

