

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __NODE_DEFINITION__
#define	__NODE_DEFINITION__

#include <xc.h> // include processor files - each processor file is guarded.  

#include "../../../../openlcb/openlcb_types.h"

// nice reply for multiple ways of defining constant structures
// https://stackoverflow.com/questions/60375529/c-how-to-initialize-a-constant-structure


extern const node_parameters_t NodeParameters_main_node;  // NOTE: explicit "extern" and no initializer


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

