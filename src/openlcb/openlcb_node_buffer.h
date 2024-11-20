
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_NODE_BUFFER__
#define	__OPENLCB_NODE_BUFFER__

#include <xc.h> // include processor files - each processor file is guarded.  


// ************************ USER DEFINED VARIABLES *****************************

#define LEN_NODE_ARRAY                64  // USER DEFINED 

// *********************END USER DEFINED VARIABLES *****************************

typedef struct {
    uint16_t alias;
    uint64_t id;
} alias_mapping_t;

typedef alias_mapping_t alias_mapping_array_t[LEN_NODE_ARRAY];

typedef uint16_t alias_array_t[LEN_NODE_ARRAY];


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

