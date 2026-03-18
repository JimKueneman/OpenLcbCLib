/** @file can_user_config.h
 *  @brief User-editable CAN bus driver configuration for OpenLcbCLib
 *
 *  OPTIONAL: If this file is not present the CAN driver will use built-in
 *  defaults.  Copy this file to your project's include path and edit to
 *  override any value.
 *
 *  All values use #ifndef guards in can_types.h so defining them here (or
 *  via -D compiler flags) takes priority over the library defaults.
 */

#ifndef __CAN_USER_CONFIG__
#define __CAN_USER_CONFIG__

// =============================================================================
// CAN Message Buffer Pool
// =============================================================================
// Number of raw CAN message buffers in the driver pool.  Each buffer holds one
// CAN 2.0 frame (8 data bytes + header).  Tune for your platform's available
// RAM and expected bus traffic.
//
// Maximum value is 254 (0xFE).

#define USER_DEFINED_CAN_MSG_BUFFER_DEPTH            20     // must be >= 1; enforced by compiler

#endif /* __CAN_USER_CONFIG__ */
