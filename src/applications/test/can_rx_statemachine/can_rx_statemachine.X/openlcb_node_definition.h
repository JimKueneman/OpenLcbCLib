
/* 
 * File: openlcb_node_definition.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_NODE_DEFINITION__
#define	__OPENLCB_NODE_DEFINITION__


#include <xc.h> // include processor files - each processor file is guarded. 
#include "../../../../openlcb/openlcb_types.h"


#define USER_DEFINED_NODE_ID                  0x050101010700
#define USER_DEFINED_NODE_ID_UPPER_RANGE      0x0501010107FF


#define USER_DEFINED_PRODUCER_COUNT           8   // The maximum number of events supported is 32
#define USER_DEFINED_CONSUMER_COUNT           8   // The maximum number of events supported is 32


extern const uint32_t USER_DEFINED_PROTOCOL_SUPPORT;


#define MFG_VERSION 1
#define MFG_NAME "Mustangpeak"
#define MFG_MODEL "GS400"
#define MFG_HARDWARE_VER "1.2.5.6"
#define MFG_SOFTWARE_VER "0.12" "Mustangpeak"
#define USER_VERSION 2
#define USER_NAME "Mustangpeak"
#define USER_DESCRIPTION "This is my node"


extern const uint16_t MAX_CDI_ARRAY;

extern const uint8_t CDI_ARRAY[];



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_NODE_DEFINITION__ */

