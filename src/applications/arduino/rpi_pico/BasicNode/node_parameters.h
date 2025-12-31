

/*
 * File:
 * Author:
 * Comments:
 * Revision history:
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __NODE_PARAMETERS__
#define __NODE_PARAMETERS__

#include "src/drivers/canbus/can_main_statemachine.h"
#include "src/openlcb/openlcb_types.h"

// nice reply for multiple ways of defining constant structures
// https://stackoverflow.com/questions/60375529/c-how-to-initialize-a-constant-structure

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    extern const node_parameters_t NodeParameters_main_node; // NOTE: explicit "extern" and no initializer

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NODE_PARAMETERS__ */
