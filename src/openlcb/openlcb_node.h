
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_NODE__
#define	__OPENLCB_NODE__

#include <xc.h>

#include "openlcb_types.h" // include processor files - each processor file is guarded.  


extern void Node_initialize();

extern openlcb_node_t* Node_allocate(uint64_t nodeid, const node_parameters_t* node_parameters);

extern openlcb_node_t* Node_get_first(uint8_t key);

extern openlcb_node_t* Node_get_next(uint8_t key);

extern openlcb_node_t* Node_find_by_alias(uint16_t alias);

extern openlcb_node_t* Node_find_by_node_id(uint64_t nodeid);

extern uint64_t Node_generate_seed(uint64_t start_seed);

extern uint16_t Node_generate_alias(uint64_t seed);


extern void Node_100ms_timer_tick();



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

