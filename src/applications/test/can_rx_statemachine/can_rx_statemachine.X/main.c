/*
 * File:   main.c
 * Author: jimkueneman
 *
 * Created on January 23, 2024, 6:26 AM
 */


// DSPIC33EP64GP502 Configuration Bit Settings

// 'C' source line config statements

// FICD
#pragma config ICS = PGD1               // ICD Communication Channel Select bits (Communicate on PGEC1 and PGED1)
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)

// FPOR
#pragma config ALTI2C1 = OFF            // Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins)
#pragma config ALTI2C2 = OFF            // Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins)
#pragma config WDTWIN = WIN25           // Watchdog Window Select bits (WDT Window is 25% of WDT period)

// FWDT
#pragma config WDTPOST = PS32768        // Watchdog Timer Postscaler bits (1:32,768)
#pragma config WDTPRE = PR128           // Watchdog Timer Prescaler bit (1:128)
#pragma config PLLKEN = ON              // PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software)

// FOSC
#pragma config POSCMD = HS              // Primary Oscillator Mode Select bits (HS Crystal Oscillator Mode)
#pragma config OSCIOFNC = OFF           // OSC2 Pin Function bit (OSC2 is clock output)
#pragma config IOL1WAY = OFF            // Peripheral pin select configuration (Allow multiple reconfigurations)
#pragma config FCKSM = CSDCMD           // Clock Switching Mode bits (Both Clock switching and Fail-safe Clock Monitor are disabled)

// FOSCSEL
#pragma config FNOSC = PRIPLL           // Oscillator Source Selection (Primary Oscillator with PLL module (XT + PLL, HS + PLL, EC + PLL))
#pragma config IESO = ON                // Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source)

// FGS
#pragma config GWRP = OFF               // General Segment Write-Protect bit (General Segment may be written)
#pragma config GCP = OFF                // General Segment Code-Protect bit (General Segment Code protect is Disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include "xc.h"
#include "stdio.h"  // printf

#include "../../../../openlcb/openlcb_buffer_fifo.h"
#include "../../../../openlcb/openlcb_buffer_list.h"
#include "../../../../openlcb/openlcb_buffer_store.h"
#include "../../../../openlcb/openlcb_defines.h"
#include "../../../../openlcb/openlcb_types.h"
#include "../../../../openlcb/openlcb_utilities.h"

#include "../../../../drivers/common/can_types.h"
#include "../../../../drivers/common/can_buffer_store.h"
#include "../../../../drivers/common/can_buffer_fifo.h"
#include "../../../../drivers/common/can_utilities.h"

#include "../../../../drivers/common/can_rx_statemachine.h"

#include "../../../../drivers/common/../mcu_driver.h"

#include "openlcb_node_definition.h"

#include "debug.h"

int main(void) {

    CanRxStatemachine_initialize();
    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();

    //     McuDriver_initialization();


    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    //  U1MODEbits.UARTEN = 1; // Enable UART
    // U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    printf("\n\nTest Start **********************************\n");

    can_msg_t can_msg;

    openlcb_msg_t* openlcb_msg1 = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);
    openlcb_msg_t* openlcb_msg2 = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);

    openlcb_msg_t openlcb_msg;

    payload_bytes_can_t payload;

    uint64_t node_id = 0xAABBCCDD11223344;
    ;

    uint8_t start_offset = 5;
    uint8_t start_index = 0;

    uint32_t identifier = 0x19345AAA;
    uint8_t payload_size = 8;
    uint8_t byte1 = 0x0ff;
    uint8_t byte2 = 0x0ff;
    uint8_t byte3 = 0x0ff;
    uint8_t byte4 = 0x0ff;
    uint8_t byte5 = 0x0ff;
    uint8_t byte6 = 0x0ff;
    uint8_t byte7 = 0x0ff;
    uint8_t byte8 = 0x0ff;

    uint64_t data = 0xFFAADDAADD4444;

    uint16_t* openlcb_msg_payload_index;

    uint8_t can_bytes_start = 2;

    uint16_t current_allocation = 0;
    uint16_t max_allocation_count = 5;

    openlcb_msg.mti = MTI_PROTOCOL_SUPPORT_INQUIRY;
    openlcb_msg.source_alias = 0x02AF;
    openlcb_msg.payload_count = 0;


    event_id_t event_id = 0xFFEEDDCCBBAA9988;


    LoadOpenLcbMessage(openlcb_msg1, 0xAAA, 0x112233445566, 0xBBB, 0xAABBCCDDEEFF, MTI_EVENTS_IDENTIFY, 0);

    printf("\n");
    printf(" Testing.........: CopyEventIdToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n");
    CopyEventIdToOpenLcbPayload(openlcb_msg1, event_id);
    printf("Copied event id 0xFFEEDDCCBBAA9988 to payload\n");
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n\n");


    printf(" Testing.........: CopyOpenLcbMessage()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("\n");
    printf("copying message 1 to message 2\n");
    printf("Message 1 should equal message 2\n");
    CopyOpenLcbMessage(openlcb_msg1, openlcb_msg2);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg2);
    printf("\n\n");

    printf(" Testing.........: Copy64BitToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("Copying a 64bit value to the payload: 0x5566778855667788\n");
    Copy64BitToOpenLcbPayload(openlcb_msg1, 0x5566778855667788);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n\n");

    printf(" Testing.........: CopyNodeIdToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("Copying a Node ID value to the payload: 0xAABBCCAABBCC\n");
    CopyNodeIdToOpenLcbPayload(openlcb_msg1, 0xAABBCCAABBCC);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n\n");

    printf(" Testing.........: CopyWordToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("Copying a word (2 bytes) value to the payload: 0xAABB at offset 0\n");
    CopyWordToOpenLcbPayload(openlcb_msg1, 0xAABB, 0);
    openlcb_msg1->payload_count = 2;
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n");
    printf("Copying a word (2 bytes) value to the payload: 0xAABB at offset 2\n");
    CopyWordToOpenLcbPayload(openlcb_msg1, 0xAABB, 2);
    openlcb_msg1->payload_count = 4;
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n");
    printf("Copying a word (2 bytes) value to the payload: 0xAABB at offset 6\n");
    CopyWordToOpenLcbPayload(openlcb_msg1, 0xAABB, 6);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n\n");


    printf(" Testing.........: CopyDWordToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("Copying a word (2 bytes) value to the payload: 0xAABBCCDD at offset 0\n");
    CopyDWordToOpenLcbPayload(openlcb_msg1, 0xAABBCCDD, 0);
    openlcb_msg1->payload_count = 4;
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n");
    printf("Copying a word (2 bytes) value to the payload: 0xAABBCCDD at offset 4\n");
    CopyDWordToOpenLcbPayload(openlcb_msg1, 0xAABBCCDD, 4);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg1);
    printf("\n\n");


    printf(" Testing.........: ExtractNodeIdFromOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("copied 0xFFEEDDCCAA99 to the message payload\n");
    CopyNodeIdToOpenLcbPayload(openlcb_msg1, 0xFFEEDDCCAA99);
    printf("reading node id from message..\n");
    node_id = ExtractNodeIdFromOpenLcbPayload(openlcb_msg1);
    printf("read: ");
    PrintNodeID(node_id);
    printf("\n\n");


    printf(" Testing.........: ExtractEventIdFromOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("copied 0xBB11223344556677 to the message payload\n");
    CopyEventIdToOpenLcbPayload(openlcb_msg1, 0xBB11223344556677);
    printf("reading event id from message..\n");
    event_id = ExtractEventIdFromOpenLcbPayload(openlcb_msg1);
    printf("read: ");
    PrintEventID(event_id);
    printf("\n\n");

    printf(" Testing.........: CopyDWordToOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("Copying a word (2 bytes) value to the payload: 0x3333 at index 0\n");
    CopyWordToOpenLcbPayload(openlcb_msg1, 0x3333, 0);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    printf("Extracting a word (2 bytes) at offset 0\n");
    uint16_t word = ExtractWordFromOpenLcbPayload(openlcb_msg1, 0);
    printf("Word read: %04X\n", word);
    printf("Copying a word (2 bytes) value to the payload: 0x5555 at index 3\n");
    CopyWordToOpenLcbPayload(openlcb_msg1, 0x5555, 3);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    printf("Extracting a word (2 bytes) at offset 0\n");
    word = ExtractWordFromOpenLcbPayload(openlcb_msg1, 3);
    printf("Word read: %04X\n", word);
    printf("\n\n");
    printf("Copying a dword (4 bytes) value to the payload: 0x33334444 at index 0\n");
    CopyDWordToOpenLcbPayload(openlcb_msg1, 0x33334444, 0);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    printf("Extracting a dword (4 bytes) at offset 0\n");
    uint32_t dword = ExtractDWordFromOpenLcbPayload(openlcb_msg1, 0);
    printf("DWord read: ");
    PrintDWord(dword);
    printf("\n");

    printf("Copying a dword (4 bytes) value to the payload: 0x55556666 at index 4\n");
    CopyDWordToOpenLcbPayload(openlcb_msg1, 0x55556666, 3);
    openlcb_msg1->payload_count = 8;
    printf("\n");
    printf("Extracting a dword (4 bytes) at offset 0\n");
    dword = ExtractDWordFromOpenLcbPayload(openlcb_msg1, 3);
    printf("DWord read: ");
    PrintDWord(dword);
    printf("\n\n");


    printf(" Testing.........: CountNullsInOpenLcbPayload()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    printf("copied 0xFF00440055660011 to the message payload\n");
    Copy64BitToOpenLcbPayload(openlcb_msg1, 0xFF00440055660011);
    printf("reading number of null bytes..\n");
    uint16_t count = CountNullsInOpenLcbPayload(openlcb_msg1);
    printf("read: %d null bytes", count);
    printf("\n\n");


    printf(" Testing.........: IsAddressedOpenLcbMessage()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    openlcb_msg1->mti = MTI_EVENTS_IDENTIFY;
    printf("copied MTI_EVENTS_IDENTIFY to the mti\n");
    if (IsAddressedOpenLcbMessage(openlcb_msg1))
        printf("Message is Addressed\n");
    else
        printf("Message is not Addressed\n");
    printf("\n");
    openlcb_msg1->mti = MTI_EVENTS_IDENTIFY_DEST;
    printf("copied MTI_EVENTS_IDENTIFY_DEST to the mti\n");
    if (IsAddressedOpenLcbMessage(openlcb_msg1))
        printf("Message is Addressed\n");
    else
        printf("Message is not Addressed\n");
    printf("\n\n");

    printf(" Testing.........: SetMultiFrameFlag()\n");
    printf("\n");
    ClearOpenLcbMessagePayload(openlcb_msg1);
    ClearOpenLcbMessagePayload(openlcb_msg2);
    uint8_t byte = 0x0F;
    printf("applying MULTIFRAME_ONLY to 0x0F\n");
    SetMultiFrameFlag(&byte, MULTIFRAME_ONLY);
    printf("result: %02X\n\n", byte);
    byte = 0x0F;
    
    printf("applying MULTIFRAME_FIRST to 0x0F\n");
    SetMultiFrameFlag(&byte, MULTIFRAME_FIRST);
    printf("result: %02X\n\n", byte);
    byte = 0x0F;
    
    printf("applying MULTIFRAME_MIDDLE to 0x0F\n");
    SetMultiFrameFlag(&byte, MULTIFRAME_MIDDLE);
    printf("result: %02X\n\n", byte);
    byte = 0x0F;
    
    printf("applying MULTIFRAME_FINAL to 0x0F\n");
    SetMultiFrameFlag(&byte, MULTIFRAME_FINAL);
    printf("result: %02X\n", byte);
    printf("\n\n");



    payload[0] = 0xAA;

    ExtractCanPayloadToNodeId(&payload);

    ExtractCanPayloadToNodeId(&payload);

    CopyNodeIdToCanPayload(&can_msg, node_id, start_offset);

    ExtractSourceAliasFromCanMessage(&can_msg);

    ExtractDestAliasFromCanMessage(&can_msg);

    ExtractCanMTIFromCanMessage(&can_msg); // caution this return 0 if the can message is a datagram/stream or other message that has a MTI that won't fit in 12 bits

    ConvertCanMtiToOpenLcbMti(&can_msg);

    IsDestAliasInCanPayload(&can_msg);

    CountNullsInCanPayload(&can_msg);

    CountNullsInPayload(&openlcb_msg, &can_msg);

    LoadCanMessage(&can_msg, identifier, payload_size, byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8);

    Copy64BitToCanMessage(&can_msg, data);

    CopyCanPayloadToOpenLcbPayload(&openlcb_msg, &can_msg, start_index);

    AppendCanPayloadToOpenLcbPayload(&openlcb_msg, &can_msg, start_index);

    CopyOpenLcbPayloadToCanPayload(&openlcb_msg, &can_msg, can_bytes_start, openlcb_msg_payload_index);

    CanUtilities_is_openlcb_message(&can_msg);

    while (1) {



    }

}
