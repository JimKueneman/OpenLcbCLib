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
    
    uint16_t mti = 0;

    uint16_t alias = 0;
    
    payload_bytes_can_t payload;

    uint64_t node_id = 0xAABBCCDD11223344;

    uint8_t start_index = 0;


    uint64_t data = 0xFFAADDAADD4444;

    uint16_t* openlcb_msg_payload_index;

    uint8_t can_bytes_start = 2;


    event_id_t event_id = 0xFFEEDDCCBBAA9988;



    // extern void CanUtilities_clear_can_message(can_msg_t* can_msg);
    printf("\n");
    printf(" Testing.........: CanUtilities_clear_can_message()\n");
    printf("\n");
    printf("Loading CAN buffer with Identifier = 0x19452AAA and eight Databytes with 0xFF\n");
    can_msg.identifier = 0x19452AAA;
    can_msg.payload_count = 0;
    for (int i = 0; i < LEN_CAN_BYTE_ARRAY; i++)
        can_msg.payload[i] = 0xFF;
    can_msg.payload_count = 8;
    PrintCanMsg(&can_msg);
    printf("\n\n");
    printf("Clearing CAN Message\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n\n");
    
    
    // extern void CanUtilties_load_can_message(can_msg_t* can_msg, uint32_t identifier, uint8_t payload_size, uint8_t byte1, uint8_t byte2 , uint8_t byte3 , uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8)
    printf("\n");
    printf(" Testing.........: CanUtilties_load_can_message()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19452AAA and Databytes = 0x33, 0x45, 0x56\n");
    CanUtilties_load_can_message(&can_msg, 0x19452AAA, 3, 0xFF, 0x33, 0x45, 0x56, 0x78, 0xAA, 0xA5, 0xB2);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n");
    printf("Loading with Identifier = 0x19452BBB and Databytes = 0xFF, 0x33, 0x45, 0x56, 0x78, 0xAA, 0xA5, 0xB2\n");
    CanUtilties_load_can_message(&can_msg, 0x19452BBB, 8, 0xFF, 0x33, 0x45, 0x56, 0x78, 0xAA, 0xA5, 0xB2);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n\n");
    
   // CanUtilities_copy_node_id_to_payload(&can_msg, node_id, start_offset);
    printf("\n");
    printf(" Testing.........: CanUtilities_copy_node_id_to_payload()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19452BBB and Node ID = 0x020101005622 with a starting index of 0\n");
    can_msg.identifier = 0x19452BBB;
    CanUtilities_copy_node_id_to_payload(&can_msg, 0x020101005622, 0);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19452FFF and Node ID = 0x02010100500FF with a starting index of 1 (zero index array) \n");
    can_msg.identifier = 0x19452FFF;
    CanUtilities_copy_node_id_to_payload(&can_msg, 0x02010100500FF, 1);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n\n");
    
    //  CanUtilities_copy_64_bit_to_can_message(&can_msg, data);
    printf("\n");
    printf(" Testing.........: CanUtilities_copy_64_bit_to_can_message()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19452AAA and Node ID = 0x020101005622AAAA\n");
    can_msg.identifier = 0x19452AAA;
    CanUtilities_copy_64_bit_to_can_message(&can_msg, 0x020101005622AAAA);
    printf("\n");
    PrintCanMsg(&can_msg);
    printf("\n\n");
    
        //  CanUtilities_extract_can_payload_as_node_id(&payload);
    printf("\n");
    printf(" Testing.........: CanUtilities_extract_can_payload_as_node_id()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19876BBB and Node ID = 0x020101005622\n");
    CanUtilties_load_can_message(&can_msg, 0x19876BBB, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    CanUtilities_copy_node_id_to_payload(&can_msg, 0x020101005622, 0);
    printf("\n");
    node_id = CanUtilities_extract_can_payload_as_node_id(&can_msg.payload);
    printf("\n");
    printf("Read Back: ");
    PrintNodeID(node_id);
    printf("\n\n");
    
        //  CanUtilities_extract_source_alias_from_can_message(&can_msg);
    printf("\n");
    printf(" Testing.........: CanUtilities_extract_source_alias_from_can_message()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19100BBB\n");
    can_msg.identifier = 0x19100BBB;
    alias = CanUtilities_extract_source_alias_from_can_message(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", alias);
    printf("\n\n");
    
    
    //  CanUtilties_extract_dest_alias_from_can_message(&can_msg);(&can_msg);
    printf("\n");
    printf(" Testing.........: CanUtilties_extract_dest_alias_from_can_message();()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Datagram Identifier with the destination alias encoded as nibbles 4, 5 and 6  = 0x1C555BBB\n");
    can_msg.identifier = 0x1C555BBB;
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", alias);
    printf("\n");
    printf("Loading with Verify Node ID Dest Message [0x19488BBB]with the destination alias as the first 2 byes in the payload = 0x0C34\n");
    can_msg.identifier = 0x19488BBB;
    can_msg.payload_count = 8;
    can_msg.payload[0] = 0x0C;
    can_msg.payload[1] = 0x34;
    alias = CanUtilties_extract_dest_alias_from_can_message(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", alias);
    printf("\n\n");
    
          //  CanUtilties_extract_can_mti_from_can_identifier();
    printf("\n");
    printf(" Testing.........: CanUtilties_extract_can_mti_from_can_identifier()\n");
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19100BBB\n");
    can_msg.identifier = 0x19100BBB;
    mti = CanUtilties_extract_can_mti_from_can_identifier(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", mti);
    PrintMtiName(mti);
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x19488BBB\n");
    can_msg.identifier = 0x19488BBB;
    mti = CanUtilties_extract_can_mti_from_can_identifier(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", mti);
    PrintMtiName(mti);   
    printf("\n");
    CanUtilities_clear_can_message(&can_msg);
    printf("Loading with Identifier = 0x1C555BBB (this is a CAN datagram and should return 0)\n");
    can_msg.identifier = 0x1C555BBB;
    mti = CanUtilties_extract_can_mti_from_can_identifier(&can_msg);
    printf("\n");
    printf("Read Back: 0x%04X\n", mti);
    printf("\n\n");
 

    payload[0] = 0xAA;

    
    CanUtilties_convert_can_mti_to_openlcb_mti(&can_msg);

    CanUtilities_is_dest_alias_in_can_payload(&can_msg);

    CanUtilities_count_nulls_in_can_payload(&can_msg);

    CanUtilities_count_nulls_in_payload(openlcb_msg, &can_msg);

    

    

    CanUtilities_append_can_payload_to_openlcb_payload(openlcb_msg, &can_msg, start_index);

    CanUtilities_copy_openlcb_payload_to_can_payload(openlcb_msg, &can_msg, can_bytes_start, openlcb_msg_payload_index);
    

    CanUtilities_is_openlcb_message(&can_msg);

    while (1) {



    }

}
