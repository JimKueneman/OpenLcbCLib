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
#include "../../../../openlcb/openlcb_node.h"
#include "../../../../openlcb/openlcb_main_statemachine.h"
#include "../../../../openlcb/clock_distribution.h"
#include "../../../../openlcb/callback_hooks.h"
#include "../../../../openlcb/protocol_datagram.h"

#include "../../../../drivers/common/can_types.h"
#include "../../../../drivers/common/can_buffer_store.h"
#include "../../../../drivers/common/can_buffer_fifo.h"
#include "../../../../drivers/common/can_utilities.h"

#include "../../../../drivers/common/can_rx_statemachine.h"
#include "../../../../drivers/common/can_tx_statemachine.h"
#include "../../../../drivers/common/can_main_statemachine.h"

#include "../../../../drivers/common/../mcu_driver.h"

#include "node_parameters.h"

#include "debug.h"

void _uart_callback(uint16_t code) {

    switch (code) {
        case 'B':
        case 'b':

            printf("\nCan Buffers: %d\n", CanBufferStore_messages_allocated());
            printf("\nBuffers: %d\n", BufferStore_messages_allocated());

            printf("\nMax Can Buffers: %d\n", CanBufferStore_messages_max_allocated());
            printf("\nMax Buffers: %d\n", BufferStore_messages_max_allocated());
            printf("\nMax CAN FIFO depth: %d\n", max_can_fifo_depth);

            return;

        case 'P':
        case 'p':

            if (openlcb_helper.active_msg)
                PrintOpenLcbMsg(openlcb_helper.active_msg);

            return;

        case 'C':
        case 'c':

            if (can_helper.active_msg) {

                PrintCanMsg(can_helper.active_msg);
                printf("\n");
                PrintCanFrameIdentifierName(can_helper.active_msg->identifier);

            }

            return;

    }
}

void _alias_change_callback(uint16_t new_alias, uint64_t node_id) {

    printf("Alias Allocation: %d  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

//#define  _SIMULATOR_

int main(void) {

    McuDriver_uart_rx_callback_func = &_uart_callback;
    CallbackHooks_alias_change = &_alias_change_callback;

    CanBufferStore_initialize();
    CanBufferFifo_initialiaze();
    CanRxStatemachine_initialize();
    CanTxStatemachine_initialize();
    CanMainStatemachine_initialize();

    BufferStore_initialize();
    BufferList_initialiaze();
    BufferFifo_initialiaze();
    MainStatemachine_initialize();
    Node_initialize();
    ClockDistribution_initialize();

    ProtocolDatagram_initialize();




#ifdef _SIMULATOR_

    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

#else

    _TRISB4 = 0;
    _RB4 = 0;

    McuDriver_initialization();

#endif

    printf("\n\nBooted\n");

    openlcb_node_t* node = Node_allocate(0x050101010700, &NodeParameters_main_node);

#ifdef _SIMULATOR_

    node->state.run_state = RUNSTATE_RUN;
    can_msg_t* buff = CanBufferFifo_push();

    buff->identifier = 0x10702031;
    buff->payload_count = 0;

#endif

    while (1) {

        _RB4 = 1;

        CanMainStateMachine_run(); // Runnning a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();)

        _RB4 = 0;


    }

}
