
/* 
 * File: openlcb_defines.h
 * Author: Jim Kueneman
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef __OPENLCB_DEFINES__
#define	__OPENLCB_DEFINES__

#include <xc.h> // include processor files - each processor file is guarded.

// General boot initialization
#define RUNSTATE_INIT                             0
// Generate the 48 bit seed to create the Alias from
#define RUNSTATE_GENERATE_SEED                    1
// Generate the Alias
#define RUNSTATE_GENERATE_ALIAS                   2
// Send the 4 (CID) messages with the NodeID and suggested Alias
#define RUNSTATE_SEND_CHECK_ID_07                 3
#define RUNSTATE_SEND_CHECK_ID_06                 4
#define RUNSTATE_SEND_CHECK_ID_05                 5   
#define RUNSTATE_SEND_CHECK_ID_04                 6   
// Wait for 200ms to see if anyone objects.  They objection could occur in this or the previous state, 
// if they do then jump back to RUNSTATE_GENERATE_SEED to try again
#define RUNSTATE_WAIT_200ms                       7
// Send the Alias Reserved message (AMR)
#define RUNSTATE_TRANSMIT_RESERVE_ID              8
// At this point the alias is reserved
// To "log in" send the (AMD) message, this sets the node to "Permitted" but still not OpenLCB "Initialized"
#define RUNSTATE_TRANSMIT_ALIAS_MAP_DEFINITION    9
// At this point the CAN specific login is complete, the Alias is reserved and the node is ready to be an OpenLCB Node
#define RUNSTATE_TRANSMIT_INITIALIZATION_COMPLETE 10
// Node is Initialized and can send any message, need to send the events we handle
#define RUNSTATE_TRANSMIT_CONSUMER_EVENTS         11
// Node is Initialized and can send any message, need to send the events we handle
#define RUNSTATE_TRANSMIT_PRODUCER_EVENTS         12
// Runs the message loop
#define RUNSTATE_RUN                              13


#define RESERVED_TOP_BIT                 0x10000000
// OpenLCB Message Mask CAN adaptation (MTI is only 12 bits (vs 16) with upper 4 redefined)
#define CAN_OPENLCB_MSG                  0x08000000  // 1 in the 27th bit is OpenLcb; 0 is a CAN frame
// Masks out the 3 Frame Type Bits
#define MASK_CAN_FRAME_SEQUENCE_NUMBER   0x07000000  // if not an OpenLcb message then this is the type of CAN frame it is 
#define MASK_CAN_FRAME_TYPE              MASK_CAN_FRAME_SEQUENCE_NUMBER // if an OpenLcb frame the same 3 bits are the frame type
// Mask out the CAN adaptation 12 bit MTI
#define MASK_CAN_VARIABLE_FIELD          0x00FFF000

#define CAN_FRAME_TYPE_GLOBAL_ADDRESSED  0x01000000
#define CAN_FRAME_TYPE_DATAGRAM_ONLY     0x02000000
#define CAN_FRAME_TYPE_DATAGRAM_FIRST    0x03000000
#define CAN_FRAME_TYPE_DATAGRAM_MIDDLE   0x04000000
#define CAN_FRAME_TYPE_DATAGRAM_FINAL    0x05000000
#define CAN_FRAME_TYPE_RESERVED          0x06000000
#define CAN_FRAME_TYPE_STREAM            0x07000000

#define MTI_INITIALIZATION_COMPLETE         0x0100
#define MTI_INITIALIZATION_COMPLETE_SIMPLE  0x0101   // Only supports Simple Node
#define MTI_VERIFY_NODE_ID_ADDRESSED        0x0488
#define MTI_VERIFY_NODE_ID_GLOBAL           0x0490
#define MTI_VERIFIED_NODE_ID                0x0170
#define MTI_VERIFIED_NODE_ID_SIMPLE         0x0171   // Only supports Simple Node
#define MTI_OPTIONAL_INTERACTION_REJECTED   0x0068
#define MTI_TERMINATE_DO_TO_ERROR           0x00A8
#define MTI_PROTOCOL_SUPPORT_INQUIRY        0x0828
#define MTI_PROTOCOL_SUPPORT_REPLY          0x0668

#define MTI_CONSUMER_IDENTIFY                0x08F4                             // Databytes = EventID
#define MTI_CONSUMER_IDENTIFY_RANGE          0x04A4                            // Databytes = EventID with Mask
#define MTI_CONSUMER_IDENTIFIED_UNKNOWN      0x04C7                            // Databytes = EventID
#define MTI_CONSUMER_IDENTIFIED_SET          0x04C4                            // Databytes = EventID
#define MTI_CONSUMER_IDENTIFIED_CLEAR        0x04C5                            // Databytes = EventID
#define MTI_CONSUMER_IDENTIFIED_RESERVED     0x04C6                            // Databytes = EventID
#define MTI_PRODUCER_IDENDIFY                0x0914                            // Databytes = EventID
#define MTI_PRODUCER_IDENTIFY_RANGE          0x0524                            // Databytes = EventID with Mask
#define MTI_PRODUCER_IDENTIFIED_UNKNOWN      0x0547                            // Databytes = EventID
#define MTI_PRODUCER_IDENTIFIED_SET          0x0544                            // Databytes = EventID
#define MTI_PRODUCER_IDENTIFIED_CLEAR        0x0545                            // Databytes = EventID
#define MTI_PRODUCER_IDENTIFIED_RESERVED     0x0546                            // Databytes = EventID
#define MTI_EVENTS_IDENTIFY_DEST             0x0968                            // Databytes = Destination Alias
#define MTI_EVENTS_IDENTIFY                  0x0970                            //
#define MTI_EVENT_LEARN                      0x0594                            // Databytes = EventID
#define MTI_PC_EVENT_REPORT                  0x05B4                            // Databytes = EventID  (Infamouse PCER)
#define MTI_PC_EVENT_REPORT_WITH_PAYLOAD     0x0F14                            // Databytes = EventID + 256 bytes of payload (Infamouse PCER)

#define MTI_SIMPLE_NODE_INFO_REQUEST         0x0DE8                            // Databytes = Destination Alias
#define MTI_SIMPLE_NODE_INFO_REPLY           0x0A08                            // Databytes = Destination Alias, ACDI Data

#define MTI_SIMPLE_TRAIN_INFO_REQUEST        0x0DA8                            // Databytes = Destination Alias
#define MTI_SIMPLE_TRAIN_INFO_REPLY          0x09C8                            // Databytes = Destination Alias, ACDI Data

#define MTI_TRACTION_PROTOCOL                0x05EB                            // Databyte = depends;
#define MTI_TRACTION_REPLY                   0x01E9                            // Databyte = depends

#define MTI_STREAM_INIT_REQUEST              0x0CC8
#define MTI_STREAM_INIT_REPLY                0x0868
#define MTI_FRAME_TYPE_CAN_STREAM_SEND       0xF000
#define MTI_STREAM_PROCEED                   0x0888
#define MTI_STREAM_COMPLETE                  0x08A8

#define MTI_DATAGRAM                         0x1C48
#define MTI_DATAGRAM_OK_REPLY                0x0A28                            // Databytes = Destination Alias
#define MTI_DATAGRAM_REJECTED_REPLY          0x0A48                            // Databytes = Destination Alias, Error Code 


// Data field Masks
#define MASK_MULTIFRAME_BITS             0xF0

#define MULTIFRAME_ONLY                  0x00
#define MULTIFRAME_FIRST                 0x40
#define MULTIFRAME_MIDDLE                0xC0
#define MULTIFRAME_FINAL                 0x80

#define MASK_STREAM_OR_DATAGRAM      0x01000
#define MASK_PRIORITY                0x00C00
#define MASK_SIMPLE_PROTOCOL         0x00010
#define MASK_DEST_ADDRESS_PRESENT    0x00008
#define MASK_EVENT_PRESENT           0x00004
#define MASK_PRIORITY_MODIFIER       0x00003

// OLD

// CAN MTI

#define CAN_CONTROL_FRAME_CID7                  0x07000000                                // First 12 Bits of 48 bit Node ID
#define CAN_CONTROL_FRAME_CID6                  0x06000000                                // 2rd 12 Bits of 48 bit Node ID
#define CAN_CONTROL_FRAME_CID5                  0x05000000                                // 3nd 12 Bits of 48 bit Node ID
#define CAN_CONTROL_FRAME_CID4                  0x04000000                                // Last 12 Bits of 48 bit Node ID
#define CAN_CONTROL_FRAME_CID3                  0x03000000                                // non-OpenLCB Protocol
#define CAN_CONTROL_FRAME_CID2                  0x02000000                                // non-OpenLCB Protocol
#define CAN_CONTROL_FRAME_CID1                  0x01000000                                // non-OpenLCB Protocol

#define CAN_CONTROL_FRAME_RID                   0x00700000                                // Reserve ID
#define CAN_CONTROL_FRAME_AMD                   0x00701000                                // Alias Map Definition
#define CAN_CONTROL_FRAME_AME                   0x00702000                                // Alias Mapping Enquiry
#define CAN_CONTROL_FRAME_AMR                   0x00703000                                // Alias Map Reset Frame
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0   0x00710000
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1   0x00711000
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2   0x00712000
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3   0x00713000

// OpenLCB Message Masks Full MTI (16 bits)
#define MASK_OPENLCB_MSG_BIT             0x08000000 
#define MASK_RESERVED_2                  0x04000000
#define MASK_RESERVED_1                  0x04000000
#define MASK_SPECIAL                     0x02000000




// OpenLCB Message Masks CAN adaptation in the Identifier
#define MASK_CAN_STREAM_OR_DATAGRAM      0x01000000
#define MASK_CAN_PRIORITY                0x00C00000
#define MASK_CAN_SIMPLE_PROTOCOL         0x00010000
#define MASK_CAN_DEST_ADDRESS_PRESENT    0x00008000
#define MASK_CAN_EVENT_PRESENT           0x00004000
#define MASK_CAN_PRIORITY_MODIFIER       0x00003000
#define MASK_CAN_SOURCE_ALIAS            0x00000FFF

// CAN Control Message Masks
#define MASK_CAN_CONTROL_VAR_FIELD       0x07FFF000
#define MASK_CAN_CONTROL_FRAME_SEQUENCE  0x07000000
#define MASK_CAN_CONTROL_NODEID          0x00FFF000


// Protocol Support 

#define PSI_SIMPLE                         0x800000
#define PSI_DATAGRAM                       0x400000
#define PSI_STREAM                         0x200000
#define PSI_MEMORY_CONFIGURATION           0x100000
#define PSI_RESERVATION                    0x080000
#define PSI_EVENT_EXCHANGE                 0x040000
#define PSI_IDENTIFICATION                 0x020000
#define PSI_TEACHING_LEARNING              0x010000
#define PSI_REMOTE_BUTTON                  0x008000
#define PSI_ABBREVIATED_DEFAULT_CDI        0x004000
#define PSI_DISPLAY                        0x002000
#define PSI_SIMPLE_NODE_INFORMATION        0x001000
#define PSI_CONFIGURATION_DESCRIPTION_INFO 0x000800
#define PSI_TRAIN_CONTROL                  0x000400
#define PSI_FUNCTION_DESCRIPTION           0x000200
#define PSI_RESERVED_0                     0x000100
#define PSI_RESERVED_1                     0x000080
#define PSI_FUNCTION_CONFIGURATION         0x000040
#define PSI_FIRMWARE_UPGRADE               0x000020
#define PSI_FIRMWARE_UPGRADE_ACTIVE        0x000010

// Well Known Events (auto routed between segments)

#define EVENT_ID_EMERGENCY_OFF                     0x010000000000FFFF
#define EVENT_ID_CLEAR_EMERGENCY_OFF               0x010000000000FFFE
#define EVENT_ID_EMERGENCY_STOP                    0x010000000000FFFD
#define EVENT_ID_CLEAR_EMERGENCY_STOP              0x010000000000FFFC
#define EVENT_ID_NODE_RECORDED_NEW_LOG             0x010000000000FFF8
#define EVENT_ID_POWER_SUPPLY_BROWN_OUT_NODE       0x010000000000FFF1
#define EVENT_ID_POWER_SUPPLY_BROWN_OUT_STANDARD   0x010000000000FFF0
#define EVENT_ID_IDENT_BUTTON_COMBO_PRESSED        0x010000000000FF00
#define EVENT_ID_LINK_ERROR_CODE_1                 0x010000000000FF01
#define EVENT_ID_LINK_ERROR_CODE_2                 0x010000000000FF02
#define EVENT_ID_LINK_ERROR_CODE_3                 0x010000000000FF03
#define EVENT_ID_LINK_ERROR_CODE_4                 0x010000000000FF04
        
      // Well Known Events (not auto routed between segments)  

#define EVENT_ID_DUPLICATE_NODE_DETECTED             0x0101000000000201
#define EVENT_ID_TRAIN                               0x0101000000000303
#define EVENT_ID_TRAIN_PROXY                         0x0101000000000304  // Depreciated

#define EVENT_ID_FIRMWARE_CORRUPTED                  0x0101000000000601
#define EVENT_ID_FIRMWARE_UPGRADE_BY_HARDWARE_SWITCH 0x0101000000000602

#define EVENT_ID_CBUS_OFF_SPACE                      0x0101010000000000
#define EVENT_ID_CBUS_ON_SPACE                       0x0101010100000000

#define EVENT_ID_DCC_ACCESSORY_ACTIVATE              0x0101020000FF0000
#define EVENT_ID_DCC_ACCESSORY_DEACTIVATE            0x0101020000FE0000
#define EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH           0x0101020000FD0000
#define EVENT_ID_DCC_TURNOUT_FEEDBACK_LOW            0x0101020000FC0000
#define EVENT_ID_DCC_SENSOR_FEEDBACK_HIGH            0x0101020000FB0000
#define EVENT_ID_DCC_SENSOR_FEEDBACK_LO              0x0101020000FA0000
#define EVENT_ID_DCC_EXTENDED_ACCESSORY_CMD_SPACE    0x01010200010000FF

#define EVENT_TRAIN_SEARCH_SPACE                     0x090099FF00000000

#define ERROR_PERMANENT                                       0x1000
#define ERROR_PERMANENT_SOURCE_NOT_PERMITTED                  0x1020
#define ERROR_PERMANENT_NOT_IMPLEMENTED                       0x1040
#define ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_SUBCOMMAND    0x1041
#define ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_COMMAND       0x1042
#define ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI           0x1043
#define ERROR_PERMANENT_INVALID_ARGUMENTS                     0x1080

#define ERROR_TEMPORARY                                       0x2000
#define ERROR_TEMPORARY_TIMEOUT                               0x2010
#define ERROR_TEMPORARY_BUFFER_UNAVAILABLE                    0x2020
#define ERROR_TEMPORARY_NOT_EXPECTED_OUT_OF_ORDER             0x2040
#define ERROR_TEMPORARY_TRANSFER_ERROR                        0x2080



#define ERROR_TEMPORARY_TIME_OUT                              0x2011
#define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041
#define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END    0x2042




#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* __OPENLCB_DEFINES__ */

