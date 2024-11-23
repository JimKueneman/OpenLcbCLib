
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_MAIN_STATEMACHINE__
#define	__OPENLCB_MAIN_STATEMACHINE__

#include <xc.h> // include processor files - each processor file is guarded.  

typedef struct {
    openlcb_msg_t worker;
    payload_stream_t worker_buffer;
    openlcb_msg_t* active_msg;
   
} openlcb_statemachine_worker_t;

extern void MainStatemachine_initialize();

extern void MainStatemachine_run();

extern openlcb_statemachine_worker_t openlcb_helper;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_MAIN_STATEMACHINE__ */

