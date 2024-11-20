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

void LegacySimpleSnipSequence(can_msg_t* can_msg) {
    
        // SIMPLE LEGACY SNIP
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x01, 0xAA, 0x00, 0xBB, 0x00, 0xCC);
    _StatemachineIncomingCan(0, can_msg);

    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0x04, 0x79, 0x00, 0xDD, 0x00, 0x01, 0xFF, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    CanUtilties_load_can_message(can_msg, 0x19a0803a, 4, 0x04, 0x79, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());
      
}

void DatagramSequence(can_msg_t* can_msg) {
    
        //  DATAGRAM
    //[[1a03a479] 20 43 00 00 00 00 28   ]  S: 02.01.12.FE.27.F3 - 05.02.01.02.02.4B Datagram: (7) 20.43.0.0.0.0.28 
    CanUtilties_load_can_message(can_msg, 0x1a03a479, 7, 0x20, 0x43, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[19a2803a] 04 79                  ]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 DatagramReceivedOK with no payload
    CanUtilties_load_can_message(can_msg, 0x19a2803a, 2, 0x04, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1b47903a] 20 53 00 00 00 00 3C 3F]  R: (Start of Datagram)
    CanUtilties_load_can_message(can_msg, 0x1b47903a, 8, 0x20, 0x53, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1c47903a] 78 6D 6C 20 76 65 72 73]  R: (Middle of Datagram)
    CanUtilties_load_can_message(can_msg, 0x1c47903a, 8, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1c47903a] 69 6F 6E 3D 27 31 2E 30]  R: (Middle of Datagram)
    CanUtilties_load_can_message(can_msg, 0x1c47903a, 8, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x31, 0x2E, 0x30);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1c47903a] 27 20 65 6E 63 6F 64 69]  R: (Middle of Datagram)
    CanUtilties_load_can_message(can_msg, 0x1c47903a, 8, 0x27, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1c47903a] 6E 67 3D 27 55 54 46 2D]  R: (Middle of Datagram)
    CanUtilties_load_can_message(can_msg, 0x1c47903a, 8, 0x6E, 0x67, 0x3D, 0x27, 0x55, 0x54, 0x46, 0x2D);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[1d47903a] 38 27 3F 3E 3C 3F      ]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Datagram: (46) 20.53.0.0.0.0.3C.3F.78.6D.6C.20.76.65.72.73.69.6F.6E.3D.27.31.2E.30.27.20.65.6E.63.6F.64.69.6E.67.3D.27.55.54.46.2D.38.27.3F.3E.3C.3F
    CanUtilties_load_can_message(can_msg, 0x1d47903a, 6, 0x38, 0x27, 0x3F, 0x3E, 0x3C, 0x3F, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());

    //[[19a28479] 00 3A                  ]  S: 02.01.12.FE.27.F3 - 05.02.01.02.02.4B DatagramReceivedOK with no payload
    CanUtilties_load_can_message(can_msg, 0x19a28479, 2, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    printf("%d\n", BufferFifo_get_allocated_count());
    
    
}

void MultiFrameSnipSequence(can_msg_t* can_msg) {
    
       ////  MULTI-FRAME SNIP

    //[[19de8479] 04 90                  ]  S: 02.01.12.FE.27.F3 - AA.02.03.04.05.07 SimpleNodeIdentInfoRequest with no payload
    CanUtilties_load_can_message(can_msg, 0x19de8479, 2, 0x04, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 01 52 61 69 6C 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Rails'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0x44, 0x79, 0x01, 0x52, 0x61, 0x69, 0x6C, 0x73);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 74 61 72 73 20 4C]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'tars L'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x74, 0x61, 0x72, 0x73, 0x20, 0x4C);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 69 6D 69 74 65 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'imited'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x69, 0x6D, 0x69, 0x74, 0x65, 0x64);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 00 49 6F 20 44 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ',Io De'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x00, 0x49, 0x6F, 0x20, 0x44, 0x65);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 76 65 6C 6F 70 65]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'velope'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x76, 0x65, 0x6C, 0x6F, 0x70, 0x65);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 72 27 73 20 42 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'r's Bo'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x72, 0x27, 0x73, 0x20, 0x42, 0x6F);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 61 72 64 00 31 2E]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ard,1.'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x61, 0x72, 0x64, 0x00, 0x31, 0x2E);
    _StatemachineIncomingCan(0, can_msg);


    //[[19a0803a] 04 79 30 00 31 2E 34 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '0,1.4,'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x30, 0x00, 0x31, 0x2E, 0x34, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 01 4A 69 6D 27 73]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content '1,Jim's'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x01, 0x4A, 0x69, 0x6D, 0x27, 0x73);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 20 49 4F 20 4E 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content ' IO No'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x20, 0x49, 0x4F, 0x20, 0x4E, 0x6F);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 64 65 20 23 31 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'de #1,'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x64, 0x65, 0x20, 0x23, 0x31, 0x00);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 4D 79 20 66 69 72]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'My fir'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x4D, 0x79, 0x20, 0x66, 0x69, 0x72);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 73 74 20 62 61 74]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'st bat'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x73, 0x74, 0x20, 0x63, 0x61, 0x74);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 63 68 20 6F 66 20]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'ch of '
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x63, 0x68, 0x20, 0x6F, 0x66, 0x20);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 49 4F 20 4E 6F 64]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'IO Nod'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x49, 0x4F, 0x20, 0x4E, 0x6F, 0x64);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 65 73 20 66 72 6F]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'es fro'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0xC4, 0x79, 0x65, 0x73, 0x20, 0x66, 0x72, 0x6F);
    _StatemachineIncomingCan(0, can_msg);

    //[[19a0803a] 04 79 6D 20 44 6F 6E 00]  R: 05.02.01.02.02.4B - 02.01.12.FE.27.F3 Simple Node Ident Info with content 'm Don,'
    CanUtilties_load_can_message(can_msg, 0x19a0803a, 8, 0x84, 0x79, 0x6D, 0x20, 0x44, 0x6F, 0x6E, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    
}

void CanFrameLogin(can_msg_t* can_msg) {
    
    // CID7
    CanUtilties_load_can_message(can_msg, 0x17050641, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    // CID6
    CanUtilties_load_can_message(can_msg, 0x16101641, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    // CID5
    CanUtilties_load_can_message(can_msg, 0x16101641, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    // CID4
    CanUtilties_load_can_message(can_msg, 0x16101641, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    //RID
    CanUtilties_load_can_message(can_msg, 0x10700641, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _StatemachineIncomingCan(0, can_msg);
    
    //AMD
    CanUtilties_load_can_message(can_msg, 0x1070103b, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanUtilities_copy_node_id_to_payload(can_msg, 0x050101010700, 0);
    _StatemachineIncomingCan(0, can_msg);

    
    
}

int main(void) {

    CanRxStatemachine_initialize();
    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();

    //   McuDriver_initialization();


    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    //  U1MODEbits.UARTEN = 1; // Enable UART
    // U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    printf("\n\nTest Start **********************************\n");

    can_msg_t can_msg;
    
    LegacySimpleSnipSequence(&can_msg);
    
    LegacySimpleSnipSequence(&can_msg);

    MultiFrameSnipSequence(&can_msg);
    
    CanFrameLogin(&can_msg);
            
            
    printf("%d\\nn", BufferFifo_get_allocated_count());
 
    printf("OpenLcb Message Pop: \n");
    openlcb_msg_t* openlcb_msg = BufferFifo_pop();
    while (openlcb_msg) {
        printf("%d\n\n", BufferFifo_get_allocated_count());
        PrintOpenLcbMsg(openlcb_msg);
        printf("\n\n");
        BufferStore_freeBuffer(openlcb_msg);
        openlcb_msg = BufferFifo_pop();
    }
    
    printf("\n\n Messages allocated: %d\n\n", BufferStore_messages_allocated());
    
    printf("CAN Message Pop: \n");
    can_msg_t* can_msg_ptr = CanBufferFifo_pop();
    while (can_msg_ptr) {
        printf("%d\n\n", CanBufferFifo_get_allocated_count());
        PrintCanMsg(can_msg_ptr);
        printf("\n\n");
        CanBufferStore_freeBuffer(can_msg_ptr);
        can_msg_ptr = CanBufferFifo_pop();
    }


    while (1) {



    }

}
