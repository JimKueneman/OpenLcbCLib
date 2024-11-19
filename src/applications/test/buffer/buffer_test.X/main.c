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
//#include "../../../../openlcb/openlcb_utilities.h"

#include "../../../../drivers/common/can_types.h"
#include "../../../../drivers/common/can_buffer_store.h"
#include "../../../../drivers/common/can_buffer_fifo.h"


//#include "../../../../drivers/common/can_rx_statemachine.h"

//#include "../../../../drivers/common/can_utilities.h"

//#include "../../../../drivers/common/../mcu_driver.h"

#include "openlcb_node_definition.h"
//#include "debug.h"

openlcb_msg_t* find_array[LEN_MESSAGE_BUFFER];
uint16_t find_array_index;
uint16_t unique_counter = 1;

void _load_olcb_basic_buffer() {

    openlcb_msg_t* olcb_msg;


    for (int i = 0; i < LEN_BASIC_BUFFER; i++) {

        olcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j & 0xFF;

            olcb_msg->dest_alias = i + 1;
            olcb_msg->source_alias = i + 1;
            olcb_msg->dest_id = i + 1;
            olcb_msg->source_id = i + 1;
            olcb_msg->mti = i + 1;
            olcb_msg->timerticks = i + 1;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;

        } else {
            printf("Could not push a basic buffer in the fifo");

        }
    }

}

void _load_olcb_datagram_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_DATAGRAM_BUFFER; i++) {

        olcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_DATAGRAM);
        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j & 0xFF;

            olcb_msg->dest_alias = i + 1;
            olcb_msg->source_alias = i + 1;
            olcb_msg->dest_id = i + 1;
            olcb_msg->source_id = i + 1;
            olcb_msg->mti = i + 1;
            olcb_msg->timerticks = i + 1;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;

        } else {
            printf("Could not push a datagram buffer in the fifo");

        }
    }

}

void _load_olcb_snip_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_SNIP_BUFFER; i++) {

        olcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_SNIP);
        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j & 0xFF;

            olcb_msg->dest_alias = i + 1;
            olcb_msg->source_alias = i + 1;
            olcb_msg->dest_id = i + 1;
            olcb_msg->source_id = i + 1;
            olcb_msg->mti = i + 1;
            olcb_msg->timerticks = i + 1;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;


        } else {
            printf("Could not push a snip buffer in the fifo");

        }
    }

}

void _load_olcb_stream_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_STREAM_BUFFER; i++) {

        olcb_msg = BufferFifo_push(LEN_MESSAGE_BYTES_STREAM);
        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j & 0xFF;

            olcb_msg->dest_alias = i + 1;
            olcb_msg->source_alias = i + 1;
            olcb_msg->dest_id = i + 1;
            olcb_msg->source_id = i + 1;
            olcb_msg->mti = i + 1;
            olcb_msg->timerticks = i + 1;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;


        } else {
            printf("Could not allocate a stream buffer in the fifo");

        }
    }


}

void _read_olcb_basic_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_BASIC_BUFFER; i++) {

        olcb_msg = BufferFifo_pop(LEN_MESSAGE_BYTES_BASIC);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != (j & 0xFF))
                    printf("payload mismatch basic fifo\n");

            }

            if (olcb_msg->dest_alias != i + 1)
                printf("dest_alais mismatch basic fifo\n");
            if (olcb_msg->source_alias != i + 1)
                printf("source_alias mismatch basic fifo\n");
            if (olcb_msg->source_id != i + 1)
                printf("source_id mismatch basic fifo\n");
            if (olcb_msg->dest_id != i + 1)
                printf("dest_id mismatch basic fifo\n");
            if (olcb_msg->mti != i + 1)
                printf("mti mismatch basic fifo\n");
            if (olcb_msg->timerticks != i + 1)
                printf("timerticks mismatch basic fifo\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch basic fifo\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch basic fifo\n");



            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not pop a basic buffer in the fifo\n");

        }
    }

}

void _read_olcb_datagram_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_DATAGRAM_BUFFER; i++) {

        olcb_msg = BufferFifo_pop(LEN_MESSAGE_BYTES_DATAGRAM);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != (j & 0xFF))
                    printf("payload mismatch datagram fifo\n");

            }

            if (olcb_msg->dest_alias != i + 1)
                printf("dest_alais mismatch datagram fifo\n");
            if (olcb_msg->source_alias != i + 1)
                printf("source_alias mismatch datagram fifo\n");
            if (olcb_msg->source_id != i + 1)
                printf("source_id mismatch datagram fifo\n");
            if (olcb_msg->dest_id != i + 1)
                printf("dest_id mismatch datagram fifo\n");
            if (olcb_msg->mti != i + 1)
                printf("mti mismatch datagram fifo\n");
            if (olcb_msg->timerticks != i + 1)
                printf("timerticks mismatch datagram fifo\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch datagram fifo\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch datagram fifo\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not pop a datagram buffer in the fifo\n");

        }
    }

}

void _read_olcb_snip_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_SNIP_BUFFER; i++) {

        olcb_msg = BufferFifo_pop(LEN_MESSAGE_BYTES_SNIP);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != (j & 0xFF))
                    printf("payload mismatch snip fifo\n");

            }

            if (olcb_msg->dest_alias != i + 1)
                printf("dest_alais mismatch snip fifo\n");
            if (olcb_msg->source_alias != i + 1)
                printf("source_alias mismatch snip fifo\n");
            if (olcb_msg->source_id != i + 1)
                printf("source_id mismatch snip fifo\n");
            if (olcb_msg->dest_id != i + 1)
                printf("dest_id mismatch snip fifo\n");
            if (olcb_msg->mti != i + 1)
                printf("mti mismatch snip fifo\n");
            if (olcb_msg->timerticks != i + 1)
                printf("timerticks mismatch snip fifo\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch snip fifo\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch snip fifo\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {
            printf("Could not pop a snip buffer in the fifo");

        }
    }

}

void _read_olcb_stream_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_STREAM_BUFFER; i++) {

        olcb_msg = BufferFifo_pop(LEN_MESSAGE_BYTES_STREAM);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != (j & 0xFF))

                    printf("payload mismatch stream fifo");

            }

            if (olcb_msg->dest_alias != i + 1)
                printf("dest_alais mismatch basic stream\n");
            if (olcb_msg->source_alias != i + 1)
                printf("source_alias mismatch basic stream\n");
            if (olcb_msg->source_id != i + 1)
                printf("source_id mismatch basic stream\n");
            if (olcb_msg->dest_id != i + 1)
                printf("dest_id mismatch basic stream\n");
            if (olcb_msg->mti != i + 1)
                printf("mti mismatch basic stream\n");
            if (olcb_msg->timerticks != i + 1)
                printf("timerticks mismatch stream fifo\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch stream fifo\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch stream fifo\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {
            printf("Could not pop a stream buffer in the fifo");

        }
    }


}

void _test_olcb_fifo() {

    _load_olcb_basic_buffer();
    _load_olcb_datagram_buffer();
    _load_olcb_snip_buffer();
    _load_olcb_stream_buffer();

    openlcb_msg_t* olcb_msg = BufferList_allocate(LEN_BASIC_BUFFER);
    if (olcb_msg)
        printf("allocation after FIFO test loads succeeded when all buffers should have been allocated\n");

    _read_olcb_basic_buffer();
    _read_olcb_datagram_buffer();
    _read_olcb_snip_buffer();
    _read_olcb_stream_buffer();

    uint16_t count = BufferStore_messages_allocated();
    if (count != 0)
        printf("FIFO test did not deallocate all buffers\n");

}

void _load_olcb_list_basic_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_BASIC_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        olcb_msg = BufferList_allocate(LEN_MESSAGE_BYTES_BASIC);

        find_array[find_array_index] = olcb_msg;
        find_array_index = find_array_index + 1;

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j;

            olcb_msg->dest_alias = unique_counter;
            olcb_msg->source_alias = unique_counter;
            olcb_msg->dest_id = olcb_msg->payload_size;
            olcb_msg->source_id = olcb_msg->payload_size;
            olcb_msg->mti = unique_counter;
            olcb_msg->timerticks = olcb_msg->payload_size;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;

        } else {

            printf("Could not push a basic buffer in the list");

        }
    }


}

void _load_olcb_list_datagram_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_DATAGRAM_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        olcb_msg = BufferList_allocate(LEN_MESSAGE_BYTES_DATAGRAM);

        find_array[find_array_index] = olcb_msg;
        find_array_index = find_array_index + 1;

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j;

            olcb_msg->dest_alias = unique_counter;
            olcb_msg->source_alias = unique_counter;
            olcb_msg->dest_id = olcb_msg->payload_size;
            olcb_msg->source_id = olcb_msg->payload_size;
            olcb_msg->mti = unique_counter;
            olcb_msg->timerticks = olcb_msg->payload_size;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;

        } else {

            printf("Could not push a datagram buffer in the list");

        }
    }
}

void _load_olcb_list_snip_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_SNIP_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        olcb_msg = BufferList_allocate(LEN_MESSAGE_BYTES_SNIP);

        find_array[find_array_index] = olcb_msg;
        find_array_index = find_array_index + 1;

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j;

            olcb_msg->dest_alias = unique_counter;
            olcb_msg->source_alias = unique_counter;
            olcb_msg->dest_id = olcb_msg->payload_size;
            olcb_msg->source_id = olcb_msg->payload_size;
            olcb_msg->mti = unique_counter;
            olcb_msg->timerticks = olcb_msg->payload_size;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;


        } else {

            printf("Could not push a snip buffer in the list");

        }
    }
}

void _load_olcb_list_stream_buffer() {


    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_STREAM_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        olcb_msg = BufferList_allocate(LEN_MESSAGE_BYTES_STREAM);

        find_array[find_array_index] = olcb_msg;
        find_array_index = find_array_index + 1;

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++)
                *olcb_msg->payload[j] = j;

            olcb_msg->dest_alias = unique_counter;
            olcb_msg->source_alias = unique_counter;
            olcb_msg->dest_id = olcb_msg->payload_size;
            olcb_msg->source_id = olcb_msg->payload_size;
            olcb_msg->mti = unique_counter;
            olcb_msg->timerticks = olcb_msg->payload_size;
            olcb_msg->state.allocated = 1;
            olcb_msg->state.inprocess = 1;


        } else {
            printf("Could not push a stream buffer in the list");

        }
    }
}

void _read_olcb_list_basic_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_BASIC_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        find_array_index = find_array_index - 1;
        olcb_msg = BufferList_find(find_array[find_array_index]->source_alias, find_array[find_array_index]->dest_alias, find_array[find_array_index]->mti);

        if (olcb_msg) {

            BufferList_release(olcb_msg);

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != j)
                    printf("payload mismatch basic list\n");

            }

            if (olcb_msg->dest_alias != unique_counter)
                printf("dest_alais mismatch basic list\n");
            if (olcb_msg->source_alias != unique_counter)
                printf("source_alias mismatch basic list\n");
            if (olcb_msg->source_id != olcb_msg->payload_size)
                printf("source_id mismatch basic list\n");
            if (olcb_msg->dest_id != olcb_msg->payload_size)
                printf("dest_id mismatch basic list\n");
            if (olcb_msg->mti != unique_counter)
                printf("mti mismatch basic list\n");
            if (olcb_msg->timerticks != olcb_msg->payload_size)
                printf("timerticks mismatch basic list\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch basic list\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch basic list\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not find a basic buffer in the list\n");

        }
    }

}

void _read_olcb_list_datagram_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_DATAGRAM_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        find_array_index = find_array_index - 1;
        olcb_msg = BufferList_find(find_array[find_array_index]->source_alias, find_array[find_array_index]->dest_alias, find_array[find_array_index]->mti);

        if (olcb_msg) {

            BufferList_release(olcb_msg);

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != j)
                    printf("payload mismatch datagram list\n");

            }

            if (olcb_msg->dest_alias != unique_counter)
                printf("dest_alais mismatch datagram list\n");
            if (olcb_msg->source_alias != unique_counter)
                printf("source_alias mismatch datagram list\n");
            if (olcb_msg->source_id != olcb_msg->payload_size)
                printf("source_id mismatch datagram list\n");
            if (olcb_msg->dest_id != olcb_msg->payload_size)
                printf("dest_id mismatch datagram list\n");
            if (olcb_msg->mti != unique_counter)
                printf("mti mismatch datagram list\n");
            if (olcb_msg->timerticks != olcb_msg->payload_size)
                printf("timerticks mismatch datagram list\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch datagram list\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch datagram list\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not find a datagram buffer in the list\n");

        }
    }


}

void _read_olcb_list_snip_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_SNIP_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        find_array_index = find_array_index - 1;
        olcb_msg = BufferList_find(find_array[find_array_index]->source_alias, find_array[find_array_index]->dest_alias, find_array[find_array_index]->mti);

        if (olcb_msg) {

            BufferList_release(olcb_msg);

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != j)
                    printf("payload mismatch snip list\n");

            }

            if (olcb_msg->dest_alias != unique_counter)
                printf("dest_alais mismatch snip list\n");
            if (olcb_msg->source_alias != unique_counter)
                printf("source_alias mismatch snip list\n");
            if (olcb_msg->source_id != olcb_msg->payload_size)
                printf("source_id mismatch snip list\n");
            if (olcb_msg->dest_id != olcb_msg->payload_size)
                printf("dest_id mismatch snip list\n");
            if (olcb_msg->mti != unique_counter)
                printf("mti mismatch snip list\n");
            if (olcb_msg->timerticks != olcb_msg->payload_size)
                printf("timerticks mismatch snip list\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch snip list\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch snip list\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not find a snip buffer in the list\n");

        }
    }

}

void _read_olcb_list_stream_buffer() {

    openlcb_msg_t* olcb_msg;

    for (int i = 0; i < LEN_STREAM_BUFFER; i++) {

        unique_counter = unique_counter + 1;

        find_array_index = find_array_index - 1;
        olcb_msg = BufferList_find(find_array[find_array_index]->source_alias, find_array[find_array_index]->dest_alias, find_array[find_array_index]->mti);

        if (olcb_msg) {

            for (int j = 0; j < olcb_msg->payload_size; j++) {

                if (*olcb_msg->payload[j] != j)
                    printf("payload mismatch stream list\n");

            }

            if (olcb_msg->dest_alias != unique_counter)
                printf("dest_alais mismatch stream list\n");
            if (olcb_msg->source_alias != unique_counter)
                printf("source_alias mismatch stream list\n");
            if (olcb_msg->source_id != olcb_msg->payload_size)
                printf("source_id mismatch stream list\n");
            if (olcb_msg->dest_id != olcb_msg->payload_size)
                printf("dest_id mismatch stream list\n");
            if (olcb_msg->mti != unique_counter)
                printf("mti mismatch stream list\n");
            if (olcb_msg->timerticks != olcb_msg->payload_size)
                printf("timerticks mismatch stream list\n");
            if (!olcb_msg->state.allocated)
                printf("state.allocated mismatch stream list\n");
            if (!olcb_msg->state.inprocess)
                printf("state.inprocess mismatch stream list\n");

            BufferStore_clear_openlcb_message(olcb_msg);
            BufferStore_freeBuffer(olcb_msg);

        } else {

            printf("Could not find a stream buffer in the list\n");

        }
    }

}

void _test_olcb_list() {

    unique_counter = 10;

    _load_olcb_list_basic_buffer();
    _load_olcb_list_datagram_buffer();
    _load_olcb_list_snip_buffer();
    _load_olcb_list_stream_buffer();

    openlcb_msg_t* olcb_msg = BufferList_allocate(LEN_BASIC_BUFFER);
    if (olcb_msg)
        printf("allocation after List test loads succeeded when all buffers should have been allocated\n");

    unique_counter = 10;

    _read_olcb_list_basic_buffer();
    _read_olcb_list_datagram_buffer();
    _read_olcb_list_snip_buffer();
    _read_olcb_list_stream_buffer();

    uint16_t count = BufferStore_messages_allocated();
    if (count != 0)
        printf("List test did not deallocate all buffers\n");

}

void _load_can_list_() {

    can_msg_t* can_msg;

    for (int i = 0; i < LEN_CAN_CONTROL_FRAME_FIFO_BUFFER; i++) {

        can_msg = CanBufferFifo_push();

        if (can_msg) {

            can_msg->identifier = 0xAABBCCDD;
            can_msg->payload_count = 8;
            for (int j = 0; j < LEN_CAN_DATA_ARRAY; j++)
                can_msg->payload[j] = 0xAA;

        }

    }

}

void _read_can_list() {

    can_msg_t* can_msg = CanBufferFifo_pop();
    while (can_msg) {
        
        CanBufferStore_freeBuffer(can_msg);
        
        can_msg = CanBufferFifo_pop();
    }

}

void _test_can_list() {

    _load_can_list_();
    _read_can_list();

}

int main(void) {

    // Setup the UART to send to the console in the simulator 
    //  https://developerhelp.microchip.com/xwiki/bin/view/software-tools/xc8/simulator-console/

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    printf("\n\nTest Start **********************************\n");

    openlcb_msg_t* olcb_msg_basic;
    openlcb_msg_t* olcb_msg_datagram;
    openlcb_msg_t* olcb_msg_snip;
    openlcb_msg_t* olcb_msg_stream;

    openlcb_msg_t* olcb_msg;

    can_msg_t can_msg;

    for (int i = 0; i < LEN_MESSAGE_BUFFER; i++)
        find_array[i] = (void*) 0;
    find_array_index = 0;


    BufferStore_initialize();
    BufferFifo_initialiaze();
    BufferList_initialiaze();
    //   CanBufferFifo_initialiaze();


    printf("\n\nTesting the Fifo\n");
    _test_olcb_fifo();
    printf("\nFifo Test Done\n");
    if (BufferStore_messages_allocated() != 0)
        printf("Did not deallocate all buffers in the Fifo test\n");
    if (!BufferFifo_is_empty())
        printf("Buffer Fifo is not empty\n");

    printf("Testing the List\n");
    _test_olcb_list();
    printf("\nList Test Done\n");
    if (BufferStore_messages_allocated() != 0)
        printf("Did not deallocate all buffers in the List test");
    if (!BufferList_is_empty())
        printf("Buffer List is not empty\n");

    printf("\n\nTesting the CAN List\n");
    _test_can_list();
    printf("\nCAN List Test Done\n");
    if (!CanBufferFifo_is_empty())
        printf("CAN Buffer Fifo is not empty");

    //  if (BufferStore_messages_allocated() != 0) 
    //     printf("Did not deallocate all bufffers in the fifo test");


    olcb_msg_basic = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);
    olcb_msg_datagram = BufferFifo_push(LEN_MESSAGE_BYTES_DATAGRAM);
    olcb_msg_snip = BufferFifo_push(LEN_MESSAGE_BYTES_SNIP);
    olcb_msg_stream = BufferFifo_push(LEN_MESSAGE_BYTES_STREAM);

    olcb_msg_basic = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);
    olcb_msg_datagram = BufferFifo_push(LEN_MESSAGE_BYTES_DATAGRAM);
    olcb_msg_snip = BufferFifo_push(LEN_MESSAGE_BYTES_SNIP);
    olcb_msg_stream = BufferFifo_push(LEN_MESSAGE_BYTES_STREAM);

    olcb_msg_basic = BufferFifo_push(LEN_MESSAGE_BYTES_BASIC);
    olcb_msg_datagram = BufferFifo_push(LEN_MESSAGE_BYTES_DATAGRAM);
    olcb_msg_snip = BufferFifo_push(LEN_MESSAGE_BYTES_SNIP);
    olcb_msg_stream = BufferFifo_push(LEN_MESSAGE_BYTES_STREAM);

    olcb_msg = BufferFifo_pop();
    while (olcb_msg) {

        BufferStore_freeBuffer(olcb_msg);
        olcb_msg = BufferFifo_pop();

    }


    //    TODO:  LIST TEST AND CAN LIST
    //    olcb_msg_basic = BufferList_allocate(LEN_MESSAGE_BYTES_BASIC);
    //    olcb_msg_datagram = BufferList_allocate(LEN_MESSAGE_BYTES_DATAGRAM);
    //    olcb_msg_snip = BufferList_allocate(LEN_MESSAGE_BYTES_SNIP);
    //    olcb_msg_stream = BufferList_allocate(LEN_MESSAGE_BYTES_STREAM);
    //    
    //    olcb_msg_basic = BufferList_allocate(LEN_MESSAGE_BYTES_BASIC);
    //    olcb_msg_datagram = BufferList_allocate(LEN_MESSAGE_BYTES_DATAGRAM);
    //    olcb_msg_snip = BufferList_allocate(LEN_MESSAGE_BYTES_SNIP);
    //    olcb_msg_stream = BufferList_allocate(LEN_MESSAGE_BYTES_STREAM);
    //    
    //    olcb_msg_basic = BufferList_allocate(LEN_MESSAGE_BYTES_BASIC);
    //    olcb_msg_datagram = BufferList_allocate(LEN_MESSAGE_BYTES_DATAGRAM);
    //    olcb_msg_snip = BufferList_allocate(LEN_MESSAGE_BYTES_SNIP);
    //    olcb_msg_stream = BufferList_allocate(LEN_MESSAGE_BYTES_STREAM);
    //    
    //    BufferList_release(&olcb_msg_basic);
    //    BufferList_release(&olcb_msg_datagram);
    //    BufferList_release(&olcb_msg_snip);
    //    BufferList_release(&olcb_msg_stream);

    while (1) {



        return 0;

    }

}
