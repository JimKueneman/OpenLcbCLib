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
#include "openlcb_node_definition.h"
#include "debug.h"

int main(void) {

    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();



    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    //  U1MODEbits.UARTEN = 1; // Enable UART
    // U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    printf("\n\nTest Start **********************************\n");

    can_msg_t can_msg;

    openlcb_msg_t* openlcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);
    

    payload_bytes_can_t payload;

    uint64_t node_id = 0xAABBCCDD11223344;

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


    event_id_t event_id = 0xFFEEDDCCBBAA9988;


    Utilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x112233445566, 0xBBB, 0xAABBCCDDEEFF, MTI_EVENTS_IDENTIFY, 0);

    printf("\n");
    printf(" Testing.........: CopyEventIdToOpenLcbPayload()\n");
    printf("\n");
    Utilities_clear_openlcb_message_payload(openlcb_msg);
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg);
    printf("\n");
    Utilities_copy_event_id_to_openlcb_payload(openlcb_msg, event_id);
    printf("Copied event id 0xFFEEDDCCBBAA9988 to payload\n");
    printf("\n");
    PrintOpenLcbMsg(openlcb_msg);
    printf("\n\n");


    payload[0] = 0xAA;

    CanUtilities_extract_can_payload_as_node_id(&payload);

    CanUtilities_extract_can_payload_as_node_id(&payload);

    CanUtilities_copy_node_id_to_payload(&can_msg, node_id, start_offset);

    CanUtilities_extract_source_alias_from_can_message(&can_msg);

    CanUtilties_extract_dest_alias_from_can_message(&can_msg);

    CanUtilties_extract_can_mti_from_can_message(&can_msg); // caution this return 0 if the can message is a datagram/stream or other message that has a MTI that won't fit in 12 bits

    CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);

    CanUtilities_is_dest_alias_in_can_payload(&can_msg);

    CanUtilities_count_nulls_in_can_payload(&can_msg);

    CanUtilities_count_nulls_in_payload(openlcb_msg, &can_msg);

    CanUtilties_load_can_message(&can_msg, identifier, payload_size, byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8);

    CanUtilities_copy_64_bit_to_can_message(&can_msg, data);

    CanUtilities_copy_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, start_index);

    CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, start_index);

    CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, &can_msg, can_bytes_start, openlcb_msg_payload_index);

    CanUtilities_is_openlcb_message(&can_msg);

    while (1) {



    }

}
