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
#include "../../../../drivers/common/can_tx_statemachine.h"

#include "../../../../drivers/common/../mcu_driver.h"

#include "openlcb_node_definition.h"

#include "debug.h"

void _test_datagram() {

    can_msg_t can_msg;

    openlcb_msg_t* openlcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_DATAGRAM);

    *openlcb_msg->payload[0] = 0x20;
    *openlcb_msg->payload[1] = 0x53;
    *openlcb_msg->payload[2] = 0x00;
    *openlcb_msg->payload[3] = 0x00;
    *openlcb_msg->payload[4] = 0x00;
    *openlcb_msg->payload[5] = 0x00;

    uint16_t index = 50;


    for (int i = 6; i < index; i++)
        *openlcb_msg->payload[i] = i;

    openlcb_msg->payload_count = index;
    openlcb_msg->dest_alias = 0x0AA;
    openlcb_msg->source_alias = 0x555;
    openlcb_msg->mti = MTI_DATAGRAM;


    index = 0;
    while (index < openlcb_msg->payload_count)
        index = index + CanTxStatemachine_try_transmit_openlcb_message(openlcb_msg, &can_msg, index);

    BufferStore_freeBuffer(BufferFifo_pop());


}

void _test_snip() {
    
    can_msg_t can_msg;

    openlcb_msg_t* openlcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_SNIP);

    *(openlcb_msg->payload)[0] = 0x01;
    *(openlcb_msg->payload)[1] = 'N';   // 0x4E
    *(openlcb_msg->payload)[2] = 'a';
    *(openlcb_msg->payload)[3] = 'm';
    *(openlcb_msg->payload)[4] = 'e';
    *(openlcb_msg->payload)[5] = 0x00;
    *(openlcb_msg->payload)[6] = 'M';   // 0x4D
    *(openlcb_msg->payload)[7] = 'o';
    *(openlcb_msg->payload)[8] = 'd';
    *(openlcb_msg->payload)[9] = 'e';
    *(openlcb_msg->payload)[10] = 'l';   // 0x6C
    *(openlcb_msg->payload)[11] = 0x00;
    *(openlcb_msg->payload)[12] = '1';   // 0x31
    *(openlcb_msg->payload)[13] = '.';   // 0x2E
    *(openlcb_msg->payload)[14] = '2';   // 0x32
    *(openlcb_msg->payload)[15] = 0x00;
    *(openlcb_msg->payload)[16] = '0';   // 0x30
    *(openlcb_msg->payload)[17] = '.';   // 0x2E
    *(openlcb_msg->payload)[18] = '9';   // 0x39
    *(openlcb_msg->payload)[19] = 0x00;
    *(openlcb_msg->payload)[20] = 0x02;
    *(openlcb_msg->payload)[21] = 'J';   // 0x4A
    *(openlcb_msg->payload)[22] = 'i';   // 0x69
    *(openlcb_msg->payload)[23] = 'm';   // 0x6D
    *(openlcb_msg->payload)[24] = 0x00;
    *(openlcb_msg->payload)[25] = 'H';   // 0x48
    *(openlcb_msg->payload)[26] = 'i';   // 0x69
    *(openlcb_msg->payload)[27] = 0x00;
    
    openlcb_msg->payload_count = 28;
    openlcb_msg->dest_alias = 0xAAA;
    openlcb_msg->source_alias = 0x555;
    openlcb_msg->mti = MTI_SIMPLE_NODE_INFO_REPLY;
    
    uint16_t index = 0;
    
    while (index < openlcb_msg->payload_count)
        index = index + CanTxStatemachine_try_transmit_openlcb_message(openlcb_msg, &can_msg, index);

    BufferStore_freeBuffer(BufferFifo_pop());


}

int main(void) {

    CanTxStatemachine_initialize();

    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();

    //   McuDriver_initialization();
    CanRxStatemachine_initialize();

    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    //  U1MODEbits.UARTEN = 1; // Enable UART
    // U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    printf("\n\nTest Start **********************************\n");

    _test_datagram();
    _test_snip();

    while (1) {



    }

}
