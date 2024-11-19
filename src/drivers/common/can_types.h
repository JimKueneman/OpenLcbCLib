
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

// ************************ USER DEFINED VARIABLES *****************************

#define LEN_CAN_CONTROL_FRAME_FIFO_BUFFER 1   // USER DEFINED 

// *********************END USER DEFINED VARIABLES *****************************

#define TX_CHANNEL_CAN_CONTROL 0
#define TX_CHANNEL_OPENLCB_MSG 0

#define LEN_CAN_DATA_ARRAY   8


// Structure for a basic CAN payload
typedef uint8_t payload_bytes_can_t[LEN_CAN_DATA_ARRAY];

typedef struct {
    uint8_t allocated;
    uint32_t identifier;              // CAN 29 bit identifier (extended)
    uint8_t payload_count;            // How many bytes are valid
    payload_bytes_can_t payload;      // Payload bytes
} can_msg_t;



typedef can_msg_t can_buffer_store_t[LEN_CAN_CONTROL_FRAME_FIFO_BUFFER];

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __CAN_TYPES__ */

