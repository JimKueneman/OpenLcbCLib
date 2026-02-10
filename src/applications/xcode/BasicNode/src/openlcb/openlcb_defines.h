/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @file openlcb_defines.h
 * @brief OpenLCB protocol constants and Message Type Indicators (MTI)
 *
 * @details This header file defines all OpenLCB protocol constants including:
 * - CAN bus login state machine run states for alias allocation
 * - Message Type Indicators (MTI) for all OpenLCB message types
 * - CAN frame type identifiers and control frame definitions
 * - Protocol support indicator bits
 * - Error codes for permanent and temporary failures
 * - Configuration memory command and subcommand codes
 * - Memory address space identifiers
 * - ACDI (Abbreviated Configuration Description Information) memory addresses
 *
 * These constants are derived from the OpenLCB specifications:
 * - Message Network Standard
 * - CAN Frame Transfer Standard
 * - Event Transport Standard
 * - Datagram Transport Standard
 * - Memory Configuration Protocol Standard
 *
 * The constants in this file are organized into logical groups:
 * -# Node Login State Machine States
 * -# CAN Frame Format and Control Frames
 * -# Message Type Indicators (MTI)
 * -# Protocol Support Indicators
 * -# Error Codes
 * -# Configuration Memory Protocol
 * -# Address Space Definitions
 * -# ACDI Memory Layout
 *
 * @note All constant values follow the OpenLCB specification exactly
 * @note MTI values are 16-bit but only 12 bits are used in CAN adaptation
 *
 * @see openlcb_types.h - Type definitions for OpenLCB structures
 * @see OpenLCB Message Network Standard S-9.7.1.1
 * @see OpenLCB CAN Frame Transfer Standard S-9.7.3
 *
 * @author Jim Kueneman
 * @date 17 Jan 2026
 */

// This is a guard condition so that contents of this file are not included
// more than once.
#ifndef __OPENLCB_OPENLCB_DEFINES__
#define __OPENLCB_OPENLCB_DEFINES__

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup node_login_states Node Login State Machine States
 * @brief State definitions for CAN bus alias allocation and node login sequence
 *
 * @details These states control the multi-step process of allocating a CAN alias
 * and logging a node onto the OpenLCB network. The sequence ensures collision-free
 * alias assignment through the Check ID (CID) process.
 *
 * Normal login sequence:
 * -# RUNSTATE_INIT - Initialize with Node ID as seed
 * -# RUNSTATE_GENERATE_ALIAS - Generate 12-bit alias from seed
 * -# RUNSTATE_LOAD_CHECK_ID_07 through 04 - Send CID frames with Node ID fragments
 * -# RUNSTATE_WAIT_200ms - Wait for collision detection period
 * -# RUNSTATE_LOAD_RESERVE_ID - Send RID frame to claim alias
 * -# RUNSTATE_LOAD_ALIAS_MAP_DEFINITION - Send AMD frame, mark node as Permitted
 * -# RUNSTATE_LOAD_INITIALIZATION_COMPLETE - Send Initialization Complete message
 * -# RUNSTATE_LOAD_CONSUMER_EVENTS - Broadcast consumer event IDs
 * -# RUNSTATE_LOAD_PRODUCER_EVENTS - Broadcast producer event IDs
 * -# RUNSTATE_RUN - Normal operation mode
 *
 * If alias collision is detected during CID or wait period, state returns to
 * RUNSTATE_GENERATE_SEED to create a new alias.
 *
 * @note States are stored in 5-bit field, limiting to 32 possible states
 * @warning State transitions must follow the sequence defined in CAN Frame Transfer Standard
 *
 * @see openlcb_node_t.state.run_state
 * @see OpenLCB CAN Frame Transfer Standard S-9.7.3
 * @{
 */

    /** @brief General boot initialization - sets Node ID as initial seed */
#define RUNSTATE_INIT 0

    /** @brief Generate new 48-bit seed from previous seed (used after alias collision) */
#define RUNSTATE_GENERATE_SEED 1

    /** @brief Generate 12-bit alias from current seed using LFSR algorithm */
#define RUNSTATE_GENERATE_ALIAS 2

    /** @brief Send CID frame 7 with first 12 bits of 48-bit Node ID */
#define RUNSTATE_LOAD_CHECK_ID_07 3

    /** @brief Send CID frame 6 with 2nd 12 bits of 48-bit Node ID */
#define RUNSTATE_LOAD_CHECK_ID_06 4

    /** @brief Send CID frame 5 with 3rd 12 bits of 48-bit Node ID */
#define RUNSTATE_LOAD_CHECK_ID_05 5

    /** @brief Send CID frame 4 with last 12 bits of 48-bit Node ID */
#define RUNSTATE_LOAD_CHECK_ID_04 6

    /** @brief Wait 200ms for alias collision detection. An objection from another node could occur in this or the previous state, if they do then jump back to RUNSTATE_GENERATE_SEED to try again */
#define RUNSTATE_WAIT_200ms 7

    /** @brief Send Reserve ID (RID) frame to claim the alias */
#define RUNSTATE_LOAD_RESERVE_ID 8

    /** @brief Send Alias Map Definition (AMD) frame, node becomes "Permitted" */
#define RUNSTATE_LOAD_ALIAS_MAP_DEFINITION 9

    /** @brief Send Initialization Complete message, node becomes "Initialized" */
#define RUNSTATE_LOAD_INITIALIZATION_COMPLETE 10

    /** @brief Broadcast all consumer event IDs that this node handles */
#define RUNSTATE_LOAD_CONSUMER_EVENTS 11

    /** @brief Broadcast all producer event IDs that this node generates */
#define RUNSTATE_LOAD_PRODUCER_EVENTS 12

    /** @brief Normal operation mode - process messages from FIFO */
#define RUNSTATE_RUN 13

    /** @} */ // end of node_login_states

/**
 * @defgroup can_frame_format CAN Frame Format and Masks
 * @brief CAN identifier bit definitions and frame type identifiers
 *
 * @details The CAN 29-bit extended identifier is structured as follows:
 *
 * Bits 28-0:
 * - Bit 28: Reserved (must be 0)
 * - Bit 27: OpenLCB/CAN selector (1=OpenLCB, 0=CAN control frame)
 * - Bits 26-24: Frame type (for OpenLCB) or sequence number (for CAN control)
 * - Bits 23-12: Variable field (MTI for OpenLCB messages)
 * - Bits 11-0: Source alias (12-bit node identifier)
 *
 * Frame types for OpenLCB messages (bits 26-24):
 * - 000: Reserved
 * - 001: Global/Addressed message
 * - 010: Datagram single frame only
 * - 011: Datagram first frame
 * - 100: Datagram middle frame
 * - 101: Datagram final frame
 * - 110: Reserved
 * - 111: Stream data
 *
 * @see OpenLCB CAN Frame Transfer Standard S-9.7.3
 * @{
 */

    /** @brief Reserved bit in CAN identifier - must always be 0 */
#define RESERVED_TOP_BIT 0x10000000

    /** @brief OpenLCB message indicator - bit 27 set means this is an OpenLCB message, clear means CAN control frame */
#define CAN_OPENLCB_MSG 0x08000000

    /** @brief Mask for frame sequence number bits (26-24) in CAN control frames */
#define MASK_CAN_FRAME_SEQUENCE_NUMBER 0x07000000

    /** @brief Mask for frame type bits (26-24) in OpenLCB messages - same bits as sequence number */
#define MASK_CAN_FRAME_TYPE MASK_CAN_FRAME_SEQUENCE_NUMBER

    /** @brief Mask for variable field (bits 23-12) containing MTI in OpenLCB messages */
#define MASK_CAN_VARIABLE_FIELD 0x00FFF000

    /** @brief Frame type: Global or addressed OpenLCB message */
#define OPENLCB_MESSAGE_STANDARD_FRAME_TYPE 0x01000000

/** @brief Frame type: Datagram complete in single frame */
#define CAN_FRAME_TYPE_DATAGRAM_ONLY 0x02000000

    /** @brief Frame type: First frame of multi-frame datagram */
#define CAN_FRAME_TYPE_DATAGRAM_FIRST 0x03000000

    /** @brief Frame type: Middle frame of multi-frame datagram */
#define CAN_FRAME_TYPE_DATAGRAM_MIDDLE 0x04000000

    /** @brief Frame type: Final frame of multi-frame datagram */
#define CAN_FRAME_TYPE_DATAGRAM_FINAL 0x05000000

    /** @brief Frame type: Reserved for future use */
#define CAN_FRAME_TYPE_RESERVED 0x06000000

    /** @brief Frame type: Stream data frame */
#define CAN_FRAME_TYPE_STREAM 0x07000000

    /** @} */ // end of can_frame_format

/**
 * @defgroup mti_message_network Message Network MTI Codes
 * @brief Message Type Indicators for basic node communication
 *
 * @details These MTI codes handle fundamental node operations:
 * - Node initialization and identification
 * - Protocol capability discovery
 * - Error reporting and interaction rejection
 *
 * @see OpenLCB Message Network Standard S-9.7.1.1
 * @{
 */

    /** @brief Node initialization complete with full protocol support */
#define MTI_INITIALIZATION_COMPLETE 0x0100

    /** @brief Node initialization complete - Simple Node Protocol only */
#define MTI_INITIALIZATION_COMPLETE_SIMPLE 0x0101

    /** @brief Request specific node to identify itself (addressed) */
#define MTI_VERIFY_NODE_ID_ADDRESSED 0x0488

    /** @brief Request all nodes to identify themselves (global) */
#define MTI_VERIFY_NODE_ID_GLOBAL 0x0490

    /** @brief Node ID verification response with full protocol support */
#define MTI_VERIFIED_NODE_ID 0x0170

    /** @brief Node ID verification response - Simple Node Protocol only */
#define MTI_VERIFIED_NODE_ID_SIMPLE 0x0171

    /** @brief Node cannot or will not process the received message */
#define MTI_OPTIONAL_INTERACTION_REJECTED 0x0068

    /** @brief Fatal error detected, node is terminating operation */
#define MTI_TERMINATE_DO_TO_ERROR 0x00A8

    /** @brief Query what protocols a node supports */
#define MTI_PROTOCOL_SUPPORT_INQUIRY 0x0828

    /** @brief Response indicating supported protocols (6-byte bit field) */
#define MTI_PROTOCOL_SUPPORT_REPLY 0x0668

    /** @} */ // end of mti_message_network

/**
 * @defgroup mti_event_transport Event Transport Protocol MTI Codes
 * @brief Message Type Indicators for Producer/Consumer event handling
 *
 * @details OpenLCB uses a Producer/Consumer event model. Events are identified
 * by 64-bit Event IDs. Nodes declare themselves as producers and/or consumers
 * of specific events.
 *
 * Consumer identification messages indicate whether the node:
 * - Consumes the event (UNKNOWN if current state unknown)
 * - Has the event SET or CLEAR
 * - Uses RESERVED state (protocol-specific)
 *
 * Producer identification follows the same pattern.
 *
 * The PCER (Producer/Consumer Event Report) is the actual event notification.
 *
 * @see OpenLCB Event Transport Standard S-9.7.4
 * @{
 */

    /** @brief Request: Identify all consumers of specified Event ID */
#define MTI_CONSUMER_IDENTIFY 0x08F4

    /** @brief Response: Consumer identifies range of events with mask */
#define MTI_CONSUMER_RANGE_IDENTIFIED 0x04A4

    /** @brief Response: Node consumes event but current state unknown */
#define MTI_CONSUMER_IDENTIFIED_UNKNOWN 0x04C7

    /** @brief Response: Node consumes event and it is currently SET */
#define MTI_CONSUMER_IDENTIFIED_SET 0x04C4

    /** @brief Response: Node consumes event and it is currently CLEAR */
#define MTI_CONSUMER_IDENTIFIED_CLEAR 0x04C5

    /** @brief Response: Node consumes event in RESERVED state */
#define MTI_CONSUMER_IDENTIFIED_RESERVED 0x04C6

    /** @brief Request: Identify all producers of specified Event ID */
#define MTI_PRODUCER_IDENTIFY 0x0914

    /** @brief Response: Producer identifies range of events with mask */
#define MTI_PRODUCER_RANGE_IDENTIFIED 0x0524

    /** @brief Response: Node produces event but current state unknown */
#define MTI_PRODUCER_IDENTIFIED_UNKNOWN 0x0547

    /** @brief Response: Node produces event and it is currently SET */
#define MTI_PRODUCER_IDENTIFIED_SET 0x0544

    /** @brief Response: Node produces event and it is currently CLEAR */
#define MTI_PRODUCER_IDENTIFIED_CLEAR 0x0545

    /** @brief Response: Node produces event in RESERVED state */
#define MTI_PRODUCER_IDENTIFIED_RESERVED 0x0546

    /** @brief Request specific node to identify all consumed/produced events */
#define MTI_EVENTS_IDENTIFY_DEST 0x0968

    /** @brief Request all nodes to identify all consumed/produced events */
#define MTI_EVENTS_IDENTIFY 0x0970

    /** @brief Teaching/learning message for event configuration */
#define MTI_EVENT_LEARN 0x0594

    /** @brief Producer/Consumer Event Report - event has occurred */
#define MTI_PC_EVENT_REPORT 0x05B4

    /** @brief Event report with payload data (single frame, up to 8 bytes payload) */
#define MTI_PC_EVENT_REPORT_WITH_PAYLOAD 0x0F14

    /** @brief Event report with payload - first frame of segmented message */
#define MTI_PC_EVENT_REPORT_WITH_PAYLOAD_FIRST 0x0F16

    /** @brief Event report with payload - middle frame of segmented message */
#define MTI_PC_EVENT_REPORT_WITH_PAYLOAD_MIDDLE 0x0F15

    /** @brief Event report with payload - last frame of segmented message */
#define MTI_PC_EVENT_REPORT_WITH_PAYLOAD_LAST 0x0F14

    /** @} */ // end of mti_event_transport

/**
 * @defgroup mti_snip Simple Node Information Protocol MTI Codes
 * @brief Message Type Indicators for SNIP (node identification strings)
 *
 * @details SNIP provides human-readable node information including:
 * - Manufacturer name
 * - Model name
 * - Hardware version
 * - Software version
 * - User-assigned name
 * - User description
 *
 * @see OpenLCB Simple Node Information Protocol Standard
 * @{
 */

    /** @brief Request simple node information from specific node */
#define MTI_SIMPLE_NODE_INFO_REQUEST 0x0DE8

    /** @brief Response with SNIP data (manufacturer and user info) */
#define MTI_SIMPLE_NODE_INFO_REPLY 0x0A08

    /** @} */ // end of mti_snip

/**
 * @defgroup mti_traction Traction Protocol MTI Codes
 * @brief Message Type Indicators for locomotive control (traction)
 *
 * @details The traction protocol controls model railroad locomotives including:
 * - Speed and direction
 * - Function controls (lights, horn, etc.)
 * - Consisting (multiple locomotive control)
 * - Train identification
 *
 * @see OpenLCB Traction Protocol Standard
 * @{
 */

    /** @brief Train control command (speed, direction, functions) */
#define MTI_TRACTION_PROTOCOL 0x05EB

    /** @brief Train control command reply/acknowledgment */
#define MTI_TRACTION_REPLY 0x01E9

    /** @brief Request train node information */
#define MTI_SIMPLE_TRAIN_INFO_REQUEST 0x0DA8

    /** @brief Train node information response */
#define MTI_SIMPLE_TRAIN_INFO_REPLY 0x09C8

    /** @} */ // end of mti_traction

/**
 * @defgroup mti_stream Stream Protocol MTI Codes
 * @brief Message Type Indicators for bulk data streaming
 *
 * @details Stream protocol provides high-throughput data transfer with:
 * - Flow control (window-based)
 * - Connection setup/teardown
 * - Bi-directional streams
 *
 * @see OpenLCB Stream Transport Standard
 * @{
 */

    /** @brief Request to initiate a stream connection */
#define MTI_STREAM_INIT_REQUEST 0x0CC8

    /** @brief Response accepting or rejecting stream connection */
#define MTI_STREAM_INIT_REPLY 0x0868

    /** @brief Frame type identifier for stream data in CAN frame */
#define MTI_FRAME_TYPE_CAN_STREAM_SEND 0xF000

    /** @brief Stream data transmission message */
#define MTI_STREAM_SEND 0x1F88

    /** @brief Stream flow control - proceed with next window of data */
#define MTI_STREAM_PROCEED 0x0888

    /** @brief Stream completed successfully */
#define MTI_STREAM_COMPLETE 0x08A8

    /** @} */ // end of mti_stream

/**
 * @defgroup mti_datagram Datagram Protocol MTI Codes
 * @brief Message Type Indicators for datagram transport
 *
 * @details Datagrams provide reliable message delivery with:
 * - Up to 72 bytes per datagram
 * - Acknowledgment required (OK or Rejected)
 * - Used by Configuration Memory Protocol
 *
 * @see OpenLCB Datagram Transport Standard
 * @{
 */

    /** @brief Datagram message (may be single or multi-frame) */
#define MTI_DATAGRAM 0x1C48

    /** @brief Datagram received successfully (positive acknowledgment) */
#define MTI_DATAGRAM_OK_REPLY 0x0A28

    /** @brief Datagram rejected (negative acknowledgment with error code) */
#define MTI_DATAGRAM_REJECTED_REPLY 0x0A48

    /** @} */ // end of mti_datagram

/**
 * @defgroup data_field_masks Data Field Bit Masks and Values
 * @brief Masks and values for multi-frame message data fields
 *
 * @details Multi-frame messages (datagrams, SNIP) use the first data byte
 * to indicate frame position within the sequence.
 *
 * @{
 */

    /** @brief Mask for multi-frame indicator bits in first data byte */
#define MASK_MULTIFRAME_BITS 0xF0

    /** @brief Single frame only (complete message in one frame) */
#define MULTIFRAME_ONLY 0x00

    /** @brief First frame of multi-frame sequence */
#define MULTIFRAME_FIRST 0x40

    /** @brief Middle frame of multi-frame sequence */
#define MULTIFRAME_MIDDLE 0xC0

    /** @brief Final frame of multi-frame sequence */
#define MULTIFRAME_FINAL 0x80

    /** @} */ // end of data_field_masks

/**
 * @defgroup mti_field_masks MTI Bit Field Masks
 * @brief Bit masks for extracting information from MTI values
 *
 * @details The 16-bit MTI contains encoded information about:
 * - Message priority
 * - Stream vs datagram
 * - Simple protocol indicator
 * - Destination address presence
 * - Event ID presence
 *
 * @{
 */

    /** @brief Bit indicating stream or datagram message type */
#define MASK_STREAM_OR_DATAGRAM 0x01000

    /** @brief Priority bits (2 bits) */
#define MASK_PRIORITY 0x00C00

    /** @brief Simple protocol indicator bit */
#define MASK_SIMPLE_PROTOCOL 0x00010

    /** @brief Destination address present indicator */
#define MASK_DEST_ADDRESS_PRESENT 0x00008

    /** @brief Event ID present indicator */
#define MASK_EVENT_PRESENT 0x00004

    /** @brief Priority modifier bits */
#define MASK_PRIORITY_MODIFIER 0x00003

    /** @} */ // end of mti_field_masks

/**
 * @defgroup can_control_frames CAN Control Frame Identifiers
 * @brief Special CAN control frames for alias allocation
 *
 * @details CAN control frames (bit 27 = 0) are used during node login:
 *
 * Check ID (CID) frames:
 * - CID7-CID4 carry fragments of the 48-bit Node ID
 * - Sent during alias allocation to detect collisions
 * - Other nodes check if the Node ID conflicts with their alias
 *
 * Other control frames:
 * - RID: Reserve ID - claims alias after CID sequence
 * - AMD: Alias Map Definition - maps alias to Node ID
 * - AME: Alias Mapping Enquiry - query alias ownership
 * - AMR: Alias Map Reset - node is releasing alias
 *
 * @see OpenLCB CAN Frame Transfer Standard S-9.7.3
 * @{
 */

    /** @brief Check ID frame 7 - carries first 12 bits of 48-bit Node ID */
#define CAN_CONTROL_FRAME_CID7 0x07000000

    /** @brief Check ID frame 6 - carries 2nd 12 bits of 48-bit Node ID */
#define CAN_CONTROL_FRAME_CID6 0x06000000

    /** @brief Check ID frame 5 - carries 3rd 12 bits of 48-bit Node ID */
#define CAN_CONTROL_FRAME_CID5 0x05000000

    /** @brief Check ID frame 4 - carries last 12 bits of 48-bit Node ID */
#define CAN_CONTROL_FRAME_CID4 0x04000000

    /** @brief Check ID frame 3 - non-OpenLCB protocol use */
#define CAN_CONTROL_FRAME_CID3 0x03000000

    /** @brief Check ID frame 2 - non-OpenLCB protocol use */
#define CAN_CONTROL_FRAME_CID2 0x02000000

    /** @brief Check ID frame 1 - non-OpenLCB protocol use */
#define CAN_CONTROL_FRAME_CID1 0x01000000

    /** @brief Reserve ID frame - claims alias */
#define CAN_CONTROL_FRAME_RID 0x00700000

    /** @brief Alias Map Definition frame - maps alias to Node ID */
#define CAN_CONTROL_FRAME_AMD 0x00701000

    /** @brief Alias Mapping Enquiry frame - query alias ownership */
#define CAN_CONTROL_FRAME_AME 0x00702000

    /** @brief Alias Map Reset frame - node releasing alias */
#define CAN_CONTROL_FRAME_AMR 0x00703000

    /** @brief Error Information Report frame type 0 */
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_0 0x00710000

    /** @brief Error Information Report frame type 1 */
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_1 0x00711000

    /** @brief Error Information Report frame type 2 */
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_2 0x00712000

    /** @brief Error Information Report frame type 3 */
#define CAN_CONTROL_FRAME_ERROR_INFO_REPORT_3 0x00713000

    /** @} */ // end of can_control_frames

/**
 * @defgroup can_identifier_masks CAN Identifier Field Masks
 * @brief Bit masks for CAN 29-bit extended identifier fields
 *
 * @details These masks extract specific fields from the CAN identifier
 * when processing OpenLCB messages over CAN bus.
 *
 * @{
 */

    /** @brief Stream or datagram indicator in CAN identifier */
#define MASK_CAN_STREAM_OR_DATAGRAM 0x01000000

    /** @brief Priority field in CAN identifier */
#define MASK_CAN_PRIORITY 0x00C00000

    /** @brief Simple protocol indicator in CAN identifier */
#define MASK_CAN_SIMPLE_PROTOCOL 0x00010000

    /** @brief Destination address present indicator in CAN identifier */
#define MASK_CAN_DEST_ADDRESS_PRESENT 0x00008000

    /** @brief Event ID present indicator in CAN identifier */
#define MASK_CAN_EVENT_PRESENT 0x00004000

    /** @brief Priority modifier field in CAN identifier */
#define MASK_CAN_PRIORITY_MODIFIER 0x00003000

    /** @brief Source alias field (12 bits) in CAN identifier */
#define MASK_CAN_SOURCE_ALIAS 0x00000FFF

    /** @} */ // end of can_identifier_masks

/**
 * @defgroup protocol_support_bits Protocol Support Indicator Bits
 * @brief Bit flags for Protocol Support Inquiry/Reply messages
 *
 * @details The Protocol Support Reply message contains a 6-byte (48-bit)
 * bit field indicating which optional protocols the node implements.
 * Bits are numbered with bit 0 as the LSB of byte 0.
 *
 * Required protocols (always supported):
 * - Protocol Support Protocol itself
 * - Simple Node Information Protocol (SNIP)
 *
 * @see MTI_PROTOCOL_SUPPORT_INQUIRY
 * @see MTI_PROTOCOL_SUPPORT_REPLY
 * @{
 */

    /** @brief Simple Node Protocol support (required for all nodes) */
#define PSI_SIMPLE 0x800000

    /** @brief Datagram Protocol support */
#define PSI_DATAGRAM 0x400000

    /** @brief Stream Protocol support */
#define PSI_STREAM 0x200000

    /** @brief Memory Configuration Protocol support */
#define PSI_MEMORY_CONFIGURATION 0x100000

    /** @brief Reservation Protocol support */
#define PSI_RESERVATION 0x080000

    /** @brief Event Exchange (Producer/Consumer) Protocol support */
#define PSI_EVENT_EXCHANGE 0x040000

    /** @brief Identification Protocol support */
#define PSI_IDENTIFICATION 0x020000

    /** @brief Teaching/Learning Configuration Protocol support */
#define PSI_TEACHING_LEARNING 0x010000

    /** @brief Remote Button Protocol support */
#define PSI_REMOTE_BUTTON 0x008000

    /** @brief Abbreviated Default CDI Protocol support */
#define PSI_ABBREVIATED_DEFAULT_CDI 0x004000

    /** @brief Display Protocol support */
#define PSI_DISPLAY 0x002000

    /** @brief Simple Node Information Protocol support (required) */
#define PSI_SIMPLE_NODE_INFORMATION 0x001000

    /** @brief Configuration Description Information (CDI) Protocol support */
#define PSI_CONFIGURATION_DESCRIPTION_INFO 0x000800

    /** @brief Traction Control Protocol support */
#define PSI_TRAIN_CONTROL 0x000400

    /** @brief Function Description Information (FDI) Protocol support */
#define PSI_FUNCTION_DESCRIPTION 0x000200

    /** @brief Reserved bit 0 */
#define PSI_RESERVED_0 0x000100

    /** @brief Reserved bit 1 */
#define PSI_RESERVED_1 0x000080

    /** @brief Function Configuration Protocol support */
#define PSI_FUNCTION_CONFIGURATION 0x000040

    /** @brief Firmware Upgrade Protocol support */
#define PSI_FIRMWARE_UPGRADE 0x000020

    /** @brief Firmware Upgrade Active indicator (node currently in upgrade mode) */
#define PSI_FIRMWARE_UPGRADE_ACTIVE 0x000010

    /** @} */ // end of protocol_support_bits

/**
 * @defgroup well_known_events Well-Known Event IDs
 * @brief Predefined Event IDs for standard OpenLCB functions
 *
 * @details Well-Known Events are Event IDs defined by the OpenLCB standard
 * for common system-wide functions. They are divided into two categories:
 *
 * **Auto-Routed Events (0x0100000000xxxxxx):**
 * These events are automatically propagated across network segments by gateway
 * nodes. They handle critical system-wide functions like emergency stops.
 *
 * **Non-Auto-Routed Events (0x0101xxxxxxxxxxxxxx):**
 * These events are segment-local and not automatically forwarded by gateways.
 * They handle node-specific conditions and protocol-specific functions.
 *
 * **DCC-Specific Events:**
 * Several well-known events map to DCC (Digital Command Control) functions
 * for interoperability with existing DCC systems.
 *
 * @see OpenLCB Event Identifiers Standard
 * @see MTI_PC_EVENT_REPORT
 * @{
 */

/**
 * @defgroup well_known_events_auto Auto-Routed Well-Known Events
 * @brief Events automatically propagated across network segments
 *
 * @details These events use the 0x0100000000xxxxxx range and are automatically
 * forwarded by gateway nodes to all connected segments.
 *
 * @{
 */

    /** @brief Emergency Off - immediately stop all layout activity */
#define EVENT_ID_EMERGENCY_OFF 0x010000000000FFFF

    /** @brief Clear Emergency Off - resume normal operation */
#define EVENT_ID_CLEAR_EMERGENCY_OFF 0x010000000000FFFE

    /** @brief Emergency Stop - stop all moving trains but maintain power */
#define EVENT_ID_EMERGENCY_STOP 0x010000000000FFFD

    /** @brief Clear Emergency Stop - trains may resume operation */
#define EVENT_ID_CLEAR_EMERGENCY_STOP 0x010000000000FFFC

    /** @brief Node has recorded a new log entry */
#define EVENT_ID_NODE_RECORDED_NEW_LOG 0x010000000000FFF8

    /** @brief Power supply brown-out detected on specific node */
#define EVENT_ID_POWER_SUPPLY_BROWN_OUT_NODE 0x010000000000FFF1

    /** @brief Power supply brown-out detected on standard power bus */
#define EVENT_ID_POWER_SUPPLY_BROWN_OUT_STANDARD 0x010000000000FFF0

    /** @brief Identification button combination pressed on node */
#define EVENT_ID_IDENT_BUTTON_COMBO_PRESSED 0x010000000000FF00

    /** @brief Link layer error code 1 detected */
#define EVENT_ID_LINK_ERROR_CODE_1 0x010000000000FF01

    /** @brief Link layer error code 2 detected */
#define EVENT_ID_LINK_ERROR_CODE_2 0x010000000000FF02

    /** @brief Link layer error code 3 detected */
#define EVENT_ID_LINK_ERROR_CODE_3 0x010000000000FF03

    /** @brief Link layer error code 4 detected */
#define EVENT_ID_LINK_ERROR_CODE_4 0x010000000000FF04

    /** @} */ // end of well_known_events_auto

/**
 * @defgroup well_known_events_local Non-Auto-Routed Well-Known Events
 * @brief Events that remain local to network segment
 *
 * @details These events use the 0x0101xxxxxxxxxxxxxx range and are NOT
 * automatically forwarded across network segments by gateways.
 *
 * @{
 */

    /** @brief Duplicate Node ID detected on network (sent via PCER) */
#define EVENT_ID_DUPLICATE_NODE_DETECTED 0x0101000000000201

    /** @brief Train node identification event */
#define EVENT_ID_TRAIN 0x0101000000000303

    /** @brief Train proxy node identification (deprecated) */
#define EVENT_ID_TRAIN_PROXY 0x0101000000000304

    /** @brief Node firmware is corrupted */
#define EVENT_ID_FIRMWARE_CORRUPTED 0x0101000000000601

    /** @brief Firmware upgrade initiated by hardware switch */
#define EVENT_ID_FIRMWARE_UPGRADE_BY_HARDWARE_SWITCH 0x0101000000000602

    /** @brief CBUS (MERG) Off event space base */
#define EVENT_ID_CBUS_OFF_SPACE 0x0101010000000000

    /** @brief CBUS (MERG) On event space base */
#define EVENT_ID_CBUS_ON_SPACE 0x0101010100000000

    /** @brief DCC accessory decoder activate command space */
#define EVENT_ID_DCC_ACCESSORY_ACTIVATE 0x0101020000FF0000

    /** @brief DCC accessory decoder deactivate command space */
#define EVENT_ID_DCC_ACCESSORY_DEACTIVATE 0x0101020000FE0000

    /** @brief DCC turnout feedback high (thrown) space */
#define EVENT_ID_DCC_TURNOUT_FEEDBACK_HIGH 0x0101020000FD0000

    /** @brief DCC turnout feedback low (closed) space */
#define EVENT_ID_DCC_TURNOUT_FEEDBACK_LOW 0x0101020000FC0000

    /** @brief DCC sensor feedback high (occupied) space */
#define EVENT_ID_DCC_SENSOR_FEEDBACK_HIGH 0x0101020000FB0000

    /** @brief DCC sensor feedback low (clear) space */
#define EVENT_ID_DCC_SENSOR_FEEDBACK_LO 0x0101020000FA0000

    /** @brief DCC extended accessory command space */
#define EVENT_ID_DCC_EXTENDED_ACCESSORY_CMD_SPACE 0x01010200010000FF

    /** @brief Train search event space base */
#define EVENT_TRAIN_SEARCH_SPACE 0x090099FF00000000

    /** @} */ // end of well_known_events_local
    /** @} */ // end of well_known_events

/**
 * @defgroup error_codes OpenLCB Error Codes
 * @brief Error codes for Optional Interaction Rejected and Datagram Rejected
 *
 * @details Error codes are 16-bit values divided into:
 * - Permanent errors (0x1000-0x1FFF): Will not succeed if retried
 * - Temporary errors (0x2000-0x2FFF): May succeed if retried later
 *
 * Used in:
 * - Optional Interaction Rejected messages
 * - Datagram Rejected Reply messages
 * - Terminate Due To Error messages
 *
 * @{
 */

    /** @brief Success code - no error */
#define S_OK 0x00

    /** @brief Permanent error base code */
#define ERROR_PERMANENT 0x1000

    /** @brief Permanent: Unknown or unsupported address space */
#define ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN 0x1001

    /** @brief Permanent: Address is out of bounds for the address space */
#define ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS 0x1002

    /** @brief Permanent: Attempted write to read-only memory */
#define ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY 0x1003

    /** @brief Permanent: Source node not permitted to access this resource */
#define ERROR_PERMANENT_SOURCE_NOT_PERMITTED 0x1020

    /** @brief Permanent: Command or protocol not implemented */
#define ERROR_PERMANENT_NOT_IMPLEMENTED 0x1040

    /** @brief Permanent: Subcommand not recognized */
#define ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN 0x1041

    /** @brief Permanent: Command not recognized */
#define ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN 0x1042

    /** @brief Permanent: MTI or transport protocol not supported */
#define ERROR_PERMANENT_NOT_IMPLEMENTED_UNKNOWN_MTI_OR_TRANPORT_PROTOCOL 0x1043

    /** @brief Permanent: Count or length parameter out of valid range */
#define ERROR_CODE_PERMANENT_COUNT_OUT_OF_RANGE 0x1044

    /** @brief Permanent: Invalid arguments in command */
#define ERROR_PERMANENT_INVALID_ARGUMENTS 0x1080

    /** @brief Temporary error base code */
#define ERROR_TEMPORARY 0x2000

    /** @brief Temporary: Operation timed out */
#define ERROR_TEMPORARY_TIMEOUT 0x2010

    /** @brief Temporary: Buffer or resource currently unavailable */
#define ERROR_TEMPORARY_BUFFER_UNAVAILABLE 0x2020

    /** @brief Temporary: Message received out of expected sequence */
#define ERROR_TEMPORARY_NOT_EXPECTED_OUT_OF_ORDER 0x2040

    /** @brief Temporary: Transfer error occurred */
#define ERROR_TEMPORARY_TRANSFER_ERROR 0x2080

    /** @brief Temporary: Timeout waiting for response */
#define ERROR_TEMPORARY_TIME_OUT 0x2011

    /** @brief Temporary: Received middle/end frame without start frame */
#define ERROR_TEMPORARY_OUT_OF_ORDER_MIDDLE_END_WITH_NO_START 0x2041

    /** @brief Temporary: Received start frame before previous sequence completed */
#define ERROR_TEMPORARY_OUT_OF_ORDER_START_BEFORE_LAST_END 0x2042

    /** @} */ // end of error_codes

/**
 * @defgroup datagram_flags Datagram Protocol Flags
 * @brief Flag bits used in datagram protocol
 *
 * @{
 */

    /** @brief Datagram OK Reply flag: Response is pending, will send actual reply later */
#define DATAGRAM_OK_REPLY_PENDING 0x80

    /** @} */ // end of datagram_flags

/**
 * @defgroup config_mem_protocol Configuration Memory Protocol Commands
 * @brief Command codes for Memory Configuration Protocol
 *
 * @details The Memory Configuration Protocol operates via datagrams with
 * a command structure:
 * - Byte 0: Protocol identifier (0x20 for Configuration Memory)
 * - Byte 1: Command/subcommand code
 * - Bytes 2+: Command-specific data
 *
 * Address spaces are identified by:
 * - Single byte (0-254) for spaces < 0xFD
 * - 0xFD, 0xFE, 0xFF with space ID in byte 6 for well-known spaces
 *
 * @see OpenLCB Memory Configuration Protocol Standard
 * @{
 */

    /** @brief Configuration Memory Protocol identifier (first byte of datagram) */
#define CONFIG_MEM_CONFIGURATION 0x20

    /** @} */ // end of config_mem_protocol

/**
 * @defgroup config_mem_read Configuration Memory Read Commands
 * @brief Read command codes and reply codes
 *
 * @details Read commands request data from a memory address space.
 * Four variants handle different address space encodings.
 *
 * @{
 */

    /** @brief Read command: Address space in byte 6 */
#define CONFIG_MEM_READ_SPACE_IN_BYTE_6 0x40

    /** @brief Read command: Address space 0xFD */
#define CONFIG_MEM_READ_SPACE_FD 0x41

    /** @brief Read command: Address space 0xFE */
#define CONFIG_MEM_READ_SPACE_FE 0x42

    /** @brief Read command: Address space 0xFF */
#define CONFIG_MEM_READ_SPACE_FF 0x43

    /** @brief Read reply OK: Address space in byte 6 */
#define CONFIG_MEM_READ_REPLY_OK_SPACE_IN_BYTE_6 0x50

    /** @brief Read reply OK: Address space 0xFD */
#define CONFIG_MEM_READ_REPLY_OK_SPACE_FD 0x51

    /** @brief Read reply OK: Address space 0xFE */
#define CONFIG_MEM_READ_REPLY_OK_SPACE_FE 0x52

    /** @brief Read reply OK: Address space 0xFF */
#define CONFIG_MEM_READ_REPLY_OK_SPACE_FF 0x53

    /** @brief Read reply FAIL: Address space in byte 6 */
#define CONFIG_MEM_READ_REPLY_FAIL_SPACE_IN_BYTE_6 0x58

    /** @brief Read reply FAIL: Address space 0xFD */
#define CONFIG_MEM_READ_REPLY_FAIL_SPACE_FD 0x59

    /** @brief Read reply FAIL: Address space 0xFE */
#define CONFIG_MEM_READ_REPLY_FAIL_SPACE_FE 0x5A

    /** @brief Read reply FAIL: Address space 0xFF */
#define CONFIG_MEM_READ_REPLY_FAIL_SPACE_FF 0x5B

    /** @} */ // end of config_mem_read

/**
 * @defgroup config_mem_read_stream Configuration Memory Read Stream Commands
 * @brief Stream-based read command codes for large data transfers
 *
 * @{
 */

    /** @brief Read stream command: Address space in byte 6 */
#define CONFIG_MEM_READ_STREAM_SPACE_IN_BYTE_6 0x60

    /** @brief Read stream command: Address space 0xFD */
#define CONFIG_MEM_READ_STREAM_SPACE_FD 0x61

    /** @brief Read stream command: Address space 0xFE */
#define CONFIG_MEM_READ_STREAM_SPACE_FE 0x62

    /** @brief Read stream command: Address space 0xFF */
#define CONFIG_MEM_READ_STREAM_SPACE_FF 0x63

    /** @brief Read stream reply OK: Address space in byte 6 */
#define CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6 0x70

    /** @brief Read stream reply OK: Address space 0xFD */
#define CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FD 0x71

    /** @brief Read stream reply OK: Address space 0xFE */
#define CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FE 0x72

    /** @brief Read stream reply OK: Address space 0xFF */
#define CONFIG_MEM_READ_STREAM_REPLY_OK_SPACE_FF 0x73

    /** @brief Read stream reply FAIL: Address space in byte 6 */
#define CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6 0x78

    /** @brief Read stream reply FAIL: Address space 0xFD */
#define CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FD 0x79

    /** @brief Read stream reply FAIL: Address space 0xFE */
#define CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FE 0x7A

    /** @brief Read stream reply FAIL: Address space 0xFF */
#define CONFIG_MEM_READ_STREAM_REPLY_FAIL_SPACE_FF 0x7B

    /** @} */ // end of config_mem_read_stream

/**
 * @defgroup config_mem_write Configuration Memory Write Commands
 * @brief Write command codes and reply codes
 *
 * @{
 */

    /** @brief Write command: Address space in byte 6 */
#define CONFIG_MEM_WRITE_SPACE_IN_BYTE_6 0x00

    /** @brief Write command: Address space 0xFD */
#define CONFIG_MEM_WRITE_SPACE_FD 0x01

    /** @brief Write command: Address space 0xFE */
#define CONFIG_MEM_WRITE_SPACE_FE 0x02

    /** @brief Write command: Address space 0xFF */
#define CONFIG_MEM_WRITE_SPACE_FF 0x03

    /** @brief Write reply OK: Address space in byte 6 */
#define CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6 0x10

    /** @brief Write reply OK: Address space 0xFD */
#define CONFIG_MEM_WRITE_REPLY_OK_SPACE_FD 0x11

    /** @brief Write reply OK: Address space 0xFE */
#define CONFIG_MEM_WRITE_REPLY_OK_SPACE_FE 0x12

    /** @brief Write reply OK: Address space 0xFF */
#define CONFIG_MEM_WRITE_REPLY_OK_SPACE_FF 0x13

    /** @brief Write reply FAIL: Address space in byte 6 */
#define CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6 0x18

    /** @brief Write reply FAIL: Address space 0xFD */
#define CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FD 0x19

    /** @brief Write reply FAIL: Address space 0xFE */
#define CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FE 0x1A

    /** @brief Write reply FAIL: Address space 0xFF */
#define CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FF 0x1B

    /** @} */ // end of config_mem_write

/**
 * @defgroup config_mem_write_mask Configuration Memory Write Under Mask Commands
 * @brief Write commands that only modify specified bits
 *
 * @{
 */

    /** @brief Write under mask command: Address space in byte 6 */
#define CONFIG_MEM_WRITE_UNDER_MASK_SPACE_IN_BYTE_6 0x08

    /** @brief Write under mask command: Address space 0xFD */
#define CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FD 0x09

    /** @brief Write under mask command: Address space 0xFE */
#define CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FE 0x0A

    /** @brief Write under mask command: Address space 0xFF */
#define CONFIG_MEM_WRITE_UNDER_MASK_SPACE_FF 0x0B

    /** @} */ // end of config_mem_write_mask

/**
 * @defgroup config_mem_write_stream Configuration Memory Write Stream Commands
 * @brief Stream-based write commands for large data transfers
 *
 * @{
 */

    /** @brief Write stream command: Address space in byte 6 */
#define CONFIG_MEM_WRITE_STREAM_SPACE_IN_BYTE_6 0x20

    /** @brief Write stream command: Address space 0xFD */
#define CONFIG_MEM_WRITE_STREAM_SPACE_FD 0x21

    /** @brief Write stream command: Address space 0xFE */
#define CONFIG_MEM_WRITE_STREAM_SPACE_FE 0x22

    /** @brief Write stream command: Address space 0xFF */
#define CONFIG_MEM_WRITE_STREAM_SPACE_FF 0x23

    /** @brief Write stream reply OK: Address space in byte 6 */
#define CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6 0x30

    /** @brief Write stream reply OK: Address space 0xFD */
#define CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FD 0x31

    /** @brief Write stream reply OK: Address space 0xFE */
#define CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FE 0x32

    /** @brief Write stream reply OK: Address space 0xFF */
#define CONFIG_MEM_WRITE_STREAM_REPLY_OK_SPACE_FF 0x33

    /** @brief Write stream reply FAIL: Address space in byte 6 */
#define CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6 0x38

    /** @brief Write stream reply FAIL: Address space 0xFD */
#define CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FD 0x39

    /** @brief Write stream reply FAIL: Address space 0xFE */
#define CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FE 0x3A

    /** @brief Write stream reply FAIL: Address space 0xFF */
#define CONFIG_MEM_WRITE_STREAM_REPLY_FAIL_SPACE_FF 0x3B

    /** @} */ // end of config_mem_write_stream

/**
 * @defgroup config_mem_operations Configuration Memory Operation Commands
 * @brief General configuration memory operation commands
 *
 * @{
 */

    /** @brief Get Configuration Options command */
#define CONFIG_MEM_OPTIONS_CMD 0x80

    /** @brief Get Configuration Options reply */
#define CONFIG_MEM_OPTIONS_REPLY 0x82

    /** @brief Get Address Space Information command */
#define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_CMD 0x84

    /** @brief Get Address Space Information reply: Space not present */
#define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT 0x86

    /** @brief Get Address Space Information reply: Space present */
#define CONFIG_MEM_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT 0x87

    /** @brief Lock/Reserve command */
#define CONFIG_MEM_RESERVE_LOCK 0x88

    /** @brief Lock/Reserve reply */
#define CONFIG_MEM_RESERVE_LOCK_REPLY 0x8A

    /** @brief Get Unique ID command (request node's unique identifier) */
#define CONFIG_MEM_GET_UNIQUE_ID 0x8C

    /** @brief Get Unique ID reply */
#define CONFIG_MEM_GET_UNIQUE_ID_REPLY 0x8D

    /** @brief Unfreeze command (resume normal operation) */
#define CONFIG_MEM_UNFREEZE 0xA0

    /** @brief Freeze command (suspend operation for configuration) */
#define CONFIG_MEM_FREEZE 0xA1

    /** @brief Indicate update complete command */
#define CONFIG_MEM_UPDATE_COMPLETE 0xA8

    /** @brief Reset/Reboot command */
#define CONFIG_MEM_RESET_REBOOT 0xA9

    /** @brief Factory Reset command (restore defaults) */
#define CONFIG_MEM_FACTORY_RESET 0xAA

    /** @} */ // end of config_mem_operations

/**
 * @defgroup address_spaces Configuration Memory Address Spaces
 * @brief Well-known address space identifiers
 *
 * @details Address spaces organize different types of configuration data:
 * - 0xFF: CDI (Configuration Description Information) - XML description
 * - 0xFE: All memory spaces combined
 * - 0xFD: Configuration memory - user-configurable values
 * - 0xFC: ACDI Manufacturer - factory set identification
 * - 0xFB: ACDI User - user-assignable identification
 * - 0xFA: FDI (Function Description Information) - for traction nodes
 * - 0xF9: Function configuration memory - for traction nodes
 * - 0xEF: Firmware upgrade space
 *
 * @{
 */

    /** @brief CDI (Configuration Description Information) space - XML config description */
#define CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO 0xFF

    /** @brief All memory combined - virtual space containing all other spaces */
#define CONFIG_MEM_SPACE_ALL 0xFE

    /** @brief Configuration Memory space - user-configurable data */
#define CONFIG_MEM_SPACE_CONFIGURATION_MEMORY 0xFD

    /** @brief ACDI Manufacturer space - read-only manufacturer info */
#define CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS 0xFC

    /** @brief ACDI User space - user-assignable node name and description */
#define CONFIG_MEM_SPACE_ACDI_USER_ACCESS 0xFB

    /** @brief FDI (Function Description Information) space - traction function descriptions */
#define CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO 0xFA

    /** @brief Traction Function Configuration Memory space */
#define CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY 0xF9

    /** @brief Firmware upgrade space */
#define CONFIG_MEM_SPACE_FIRMWARE 0xEF

    /** @} */ // end of address_spaces

/**
 * @defgroup acdi_manufacturer_layout ACDI Manufacturer Space Memory Layout
 * @brief Memory addresses and lengths for ACDI manufacturer information
 *
 * @details The ACDI Manufacturer space (0xFC) contains read-only strings
 * set at manufacturing time. Layout:
 * - Byte 0: Version (always 1)
 * - Bytes 1-41: Manufacturer name (null-terminated, max 41 bytes including null)
 * - Bytes 42-82: Model name (null-terminated, max 41 bytes including null)
 * - Bytes 83-103: Hardware version (null-terminated, max 21 bytes including null)
 * - Bytes 104-124: Software version (null-terminated, max 21 bytes including null)
 *
 * @{
 */

    /** @brief Address of version byte in ACDI manufacturer space */
#define CONFIG_MEM_ACDI_MANUFACTURER_VERSION_ADDRESS 0x00

    /** @brief Address of manufacturer name string */
#define CONFIG_MEM_ACDI_MANUFACTURER_ADDRESS 0x01

    /** @brief Address of model name string */
#define CONFIG_MEM_ACDI_MODEL_ADDRESS 0x2A

    /** @brief Address of hardware version string */
#define CONFIG_MEM_ACDI_HARDWARE_VERSION_ADDRESS 0x53

    /** @brief Address of software version string */
#define CONFIG_MEM_ACDI_SOFTWARE_VERSION_ADDRESS 0x68

    /** @brief Length of version field (1 byte) */
#define CONFIG_MEM_ACDI_VERSION_LEN 1

    /** @brief Maximum length of manufacturer name (41 bytes including null) */
#define CONFIG_MEM_ACDI_MANUFACTURER_LEN 41

    /** @brief Maximum length of model name (41 bytes including null) */
#define CONFIG_MEM_ACDI_MODEL_LEN 41

    /** @brief Maximum length of hardware version (21 bytes including null) */
#define CONFIG_MEM_ACDI_HARDWARE_VERSION_LEN 21

    /** @brief Maximum length of software version (21 bytes including null) */
#define CONFIG_MEM_ACDI_SOFTWARE_VERSION_LEN 21

    /** @} */ // end of acdi_manufacturer_layout

/**
 * @defgroup acdi_user_layout ACDI User Space Memory Layout
 * @brief Memory addresses and lengths for ACDI user information
 *
 * @details The ACDI User space (0xFB) contains user-assignable strings.
 * Layout:
 * - Byte 0: Version (always 1)
 * - Bytes 1-63: User name (null-terminated, max 63 bytes including null)
 * - Bytes 64-127: User description (null-terminated, max 64 bytes including null)
 *
 * @{
 */

    /** @brief Address of user-assigned model name (deprecated, use USER_NAME) */
#define CONFIG_MEM_USER_MODEL_ADDRESS 0x00

    /** @brief Address of user description (deprecated, use proper offset) */
#define CONFIG_MEM_USER_DESCRIPTION_ADDRESS 0x3F

    /** @brief Address of version byte in ACDI user space */
#define CONFIG_MEM_ACDI_USER_VERSION_ADDRESS 0x00

    /** @brief Address of user-assigned name string */
#define CONFIG_MEM_ACDI_USER_NAME_ADDRESS 0x01

    /** @brief Address of user description string */
#define CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS 0x40

    /** @brief Length of version field (1 byte) */
#define CONFIG_MEM_ACDI_USER_VERSION_LEN 1

    /** @brief Maximum length of user name (63 bytes including null) */
#define CONFIG_MEM_ACDI_USER_NAME_LEN 63

    /** @brief Maximum length of user description (64 bytes including null) */
#define CONFIG_MEM_ACDI_USER_DESCRIPTION_LEN 64

    /** @} */ // end of acdi_user_layout

/**
 * @defgroup config_mem_reply_offsets Configuration Memory Reply Code Offsets
 * @brief Offset values to convert command codes to reply codes
 *
 * @details Add these offsets to command codes to get corresponding reply codes:
 * - Command + REPLY_OK_OFFSET = OK reply
 * - Command + REPLY_FAIL_OFFSET = FAIL reply
 *
 * @{
 */

    /** @brief Offset to add to command code to get OK reply code */
#define CONFIG_MEM_REPLY_OK_OFFSET 0x10

    /** @brief Offset to add to command code to get FAIL reply code */
#define CONFIG_MEM_REPLY_FAIL_OFFSET 0x18

    /** @} */ // end of config_mem_reply_offsets

/**
 * @defgroup config_options_bits Configuration Options Bit Flags
 * @brief Capability flags returned by Get Configuration Options
 *
 * @details These bits indicate which optional configuration memory
 * operations are supported by the node.
 *
 * @{
 */

    /** @brief Write Under Mask command supported */
#define CONFIG_OPTIONS_COMMANDS_WRITE_UNDER_MASK 0x8000

    /** @brief Unaligned read operations supported */
#define CONFIG_OPTIONS_COMMANDS_UNALIGNED_READS 0x4000

    /** @brief Unaligned write operations supported */
#define CONFIG_OPTIONS_COMMANDS_UNALIGNED_WRITES 0x2000

    /** @brief ACDI Manufacturer space (0xFC) readable */
#define CONFIG_OPTIONS_COMMANDS_ACDI_MANUFACTURER_READ 0x0800

    /** @brief ACDI User space (0xFB) readable */
#define CONFIG_OPTIONS_COMMANDS_ACDI_USER_READ 0x0400

    /** @brief ACDI User space (0xFB) writable */
#define CONFIG_OPTIONS_COMMANDS_ACDI_USER_WRITE 0x0200

    /** @} */ // end of config_options_bits

/**
 * @defgroup config_write_length_flags Configuration Write Length Flags
 * @brief Flags for write length capabilities
 *
 * @{
 */

    /** @brief Reserved bits in write length field */
#define CONFIG_OPTIONS_WRITE_LENGTH_RESERVED (0x80 | 0x40 | 0x20 | 0x02)

    /** @brief Stream read/write supported */
#define CONFIG_OPTIONS_WRITE_LENGTH_STREAM_READ_WRITE 0x01

    /** @} */ // end of config_write_length_flags

/**
 * @defgroup address_space_info_flags Address Space Information Flags
 * @brief Flags returned in Get Address Space Information reply
 *
 * @{
 */

    /** @brief Address space is read-only */
#define CONFIG_OPTIONS_SPACE_INFO_FLAG_READ_ONLY 0x01

    /** @brief Low address field is valid and should be used */
#define CONFIG_OPTIONS_SPACE_INFO_FLAG_USE_LOW_ADDRESS 0x02

    /** @} */ // end of address_space_info_flags

/**
 * @defgroup node_enum_keys Node Enumeration Key Management
 * @brief Constants for managing node enumeration keys
 *
 * @details Nodes can be enumerated using user-defined keys (0-3) and
 * internal system keys (4-6). This allows different subsystems to
 * independently enumerate nodes.
 *
 * User keys (0-3): Available for application use
 * Internal keys:
 * - Key 4: Main state machine enumeration
 * - Key 5: Login state machine enumeration
 * - Key 6: CAN state machine enumeration
 *
 * @{
 */

    /** @brief Maximum number of enumeration keys available for user/application */
#define MAX_INTERNAL_ENUM_KEYS_VALUES 4

    /** @brief Maximum number of internal system enumeration keys */
#define MAX_USER_ENUM_KEYS_VALUES 4

    /** @brief User enumeration key 1 */
#define USER_ENUM_KEYS_VALUES_1 0

    /** @brief User enumeration key 2 */
#define USER_ENUM_KEYS_VALUES_2 1

    /** @brief User enumeration key 3 */
#define USER_ENUM_KEYS_VALUES_3 2

    /** @brief User enumeration key 4 */
#define USER_ENUM_KEYS_VALUES_4 3

    /** @brief Total number of enumeration keys (user + internal) */
#define MAX_NODE_ENUM_KEY_VALUES (MAX_USER_ENUM_KEYS_VALUES + MAX_INTERNAL_ENUM_KEYS_VALUES)

    /** @brief Enumeration key used by main OpenLCB state machine */
#define OPENLCB_MAIN_STATMACHINE_NODE_ENUMERATOR_INDEX MAX_USER_ENUM_KEYS_VALUES

    /** @brief Enumeration key used by login state machine */
#define OPENLCB_LOGIN_STATMACHINE_NODE_ENUMERATOR_INDEX (MAX_USER_ENUM_KEYS_VALUES + 1)

    /** @brief Enumeration key used by CAN state machine */
#define CAN_STATEMACHINE_NODE_ENUMRATOR_KEY (MAX_USER_ENUM_KEYS_VALUES + 2)

    /** @} */ // end of node_enum_keys

/**
 * @defgroup broadcast_time_events Broadcast Time Protocol Event IDs
 * @brief Well-known Event IDs for clock synchronization and fast time
 *
 * @details The Broadcast Time Protocol uses well-known Event IDs to distribute
 * time information across the OpenLCB network. Time is encoded directly in the
 * Event ID itself (no payload data). This allows fast clock synchronization for
 * model railroad operations where time runs faster than real-time.
 *
 * **Event ID Structure (64 bits):**
 * - Bits 63-16 (6 bytes): Clock Identifier - identifies which clock
 * - Bits 15-0 (2 bytes): Time/Date/Year/Rate/Command Data
 *
 * **Well-Known Clock Identifiers (Upper 6 bytes):**
 * - 01.01.00.00.01.00 - Default Fast Clock
 * - 01.01.00.00.01.01 - Default Real-time Clock
 * - 01.01.00.00.01.02 - Alternate Clock 1
 * - 01.01.00.00.01.03 - Alternate Clock 2
 * - Custom unique IDs for additional clocks
 *
 * **Event Types (Lower 2 bytes encoding):**
 * - 0x0000-0x17FF: Report Time (hour 0-23, minute 0-59)
 * - 0x2100-0x2CFF: Report Date (month 1-12, day 1-31)
 * - 0x3000-0x3FFF: Report Year (year 0-4095 AD)
 * - 0x4000-0x4FFF: Report Rate (12-bit signed fixed point)
 * - 0x8000-0x97FF: Set Time (offset +0x8000 from Report Time)
 * - 0xA100-0xACFF: Set Date (offset +0x8000 from Report Date)
 * - 0xB000-0xBFFF: Set Year (offset +0x8000 from Report Year)
 * - 0xC000-0xCFFF: Set Rate (offset +0x8000 from Report Rate)
 * - 0xF000: Query (request clock synchronization)
 * - 0xF001: Stop (stop clock)
 * - 0xF002: Start (start clock)
 * - 0xF003: Date Rollover (midnight crossing notification)
 *
 * **Rate Format:**
 * Rate is a 12-bit signed fixed point value: rrrrrrrrrr.rr (2 fractional bits)
 * - Range: -512.00 to +511.75 in 0.25 increments
 * - Examples: 0x0004 = 1.00 (real-time), 0x0010 = 4.00 (4x speed)
 * - Negative values in 2's complement (e.g., 0xFFFC = -1.00)
 *
 * **Protocol Behaviors:**
 * - Report Time sent maximum once per real-world minute
 * - Report Time sent minimum once per real-world hour
 * - Date Rollover sent before midnight, Year/Date 3 seconds after
 * - Query triggers full synchronization sequence from producers
 * - Set commands echo as Report events, full sync 3 seconds later
 *
 * @see OpenLCB Broadcast Time Protocol Standard
 * @see MTI_PC_EVENT_REPORT - Transport MTI for time events
 * @{
 */

    /** @brief Default Fast Clock identifier (upper 6 bytes of Event ID) */
#define BROADCAST_TIME_ID_DEFAULT_FAST_CLOCK       0x0101000000010000ULL

    /** @brief Default Real-time Clock identifier (upper 6 bytes of Event ID) */
#define BROADCAST_TIME_ID_DEFAULT_REALTIME_CLOCK   0x0101000000010100ULL

    /** @brief Alternate Clock 1 identifier (upper 6 bytes of Event ID) */
#define BROADCAST_TIME_ID_ALTERNATE_CLOCK_1        0x0101000000010200ULL

    /** @brief Alternate Clock 2 identifier (upper 6 bytes of Event ID) */
#define BROADCAST_TIME_ID_ALTERNATE_CLOCK_2        0x0101000000010300ULL

    /** @brief Mask for extracting clock ID (upper 6 bytes) from Event ID */
#define BROADCAST_TIME_MASK_CLOCK_ID               0xFFFFFFFFFFFF0000ULL

    /** @brief Mask for extracting command/data (lower 2 bytes) from Event ID */
#define BROADCAST_TIME_MASK_COMMAND_DATA           0x000000000000FFFFULL

    /** @brief Report Time event base (lower 2 bytes: 0x0000-0x17FF) */
#define BROADCAST_TIME_REPORT_TIME_BASE            0x0000

    /** @brief Report Date event base (lower 2 bytes: 0x2100-0x2CFF) */
#define BROADCAST_TIME_REPORT_DATE_BASE            0x2100

    /** @brief Report Year event base (lower 2 bytes: 0x3000-0x3FFF) */
#define BROADCAST_TIME_REPORT_YEAR_BASE            0x3000

    /** @brief Report Rate event base (lower 2 bytes: 0x4000-0x4FFF) */
#define BROADCAST_TIME_REPORT_RATE_BASE            0x4000

    /** @brief Set Time event base (lower 2 bytes: 0x8000-0x97FF) */
#define BROADCAST_TIME_SET_TIME_BASE               0x8000

    /** @brief Set Date event base (lower 2 bytes: 0xA100-0xACFF) */
#define BROADCAST_TIME_SET_DATE_BASE               0xA100

    /** @brief Set Year event base (lower 2 bytes: 0xB000-0xBFFF) */
#define BROADCAST_TIME_SET_YEAR_BASE               0xB000

    /** @brief Set Rate event base (lower 2 bytes: 0xC000-0xCFFF) */
#define BROADCAST_TIME_SET_RATE_BASE               0xC000

    /** @brief Query event (lower 2 bytes: 0xF000) - request synchronization */
#define BROADCAST_TIME_QUERY                       0xF000

    /** @brief Stop event (lower 2 bytes: 0xF001) - stop clock */
#define BROADCAST_TIME_STOP                        0xF001

    /** @brief Start event (lower 2 bytes: 0xF002) - start clock */
#define BROADCAST_TIME_START                       0xF002

    /** @brief Date Rollover event (lower 2 bytes: 0xF003) - midnight crossing */
#define BROADCAST_TIME_DATE_ROLLOVER               0xF003

    /** @brief Offset to convert Report commands to Set commands (add 0x8000) */
#define BROADCAST_TIME_SET_COMMAND_OFFSET          0x8000

    /** @} */ // end of broadcast_time_events

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __OPENLCB_OPENLCB_DEFINES__ */
