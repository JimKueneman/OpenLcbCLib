
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __CAN_TYPES__
#define	__CAN_TYPES__

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../../openlcb/openlcb_types.h"

// ************************ USER DEFINED VARIABLES *****************************

#define LEN_CAN_BUFFER                    10   // USER DEFINED 

// *********************END USER DEFINED VARIABLES *****************************

#define TX_CHANNEL_CAN_CONTROL 0
#define TX_CHANNEL_OPENLCB_MSG 0

#define LEN_CAN_BYTE_ARRAY   8

#define OFFSET_CAN_WITHOUT_DEST_ADDRESS   0
#define OFFSET_CAN_WITH_DEST_ADDRESS      2


// Structure for a basic CAN payload
typedef uint8_t payload_bytes_can_t[LEN_CAN_BYTE_ARRAY];

typedef struct {
    uint8_t allocated: 1;
    uint8_t direct_tx: 1;    // If set the CAN statemachine will simply directly send it assuming all the source/dest/mti/data is all set up.  Mainly for sending error found during Can frame reception to allow for the rx thread/interrupt to not have to reach across boundries to send it.
} can_msg_state_t;

typedef struct {
    can_msg_state_t state;
    uint32_t identifier;              // CAN 29 bit identifier (extended)
    uint8_t payload_count;            // How many bytes are valid
    payload_bytes_can_t payload;      // Payload bytes
} can_msg_t;


typedef can_msg_t can_buffer_store_t[LEN_CAN_BUFFER];

typedef struct {
    openlcb_statemachine_worker_t* openlcb_worker;
    can_msg_t can_worker;
    can_msg_t* active_msg;
} can_main_statemachine_t;


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_TYPES__ */

