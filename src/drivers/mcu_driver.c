
/*
 * File:   mcu_drv.c
 * Author: Jim Kueneman
 *
 * Created on February 13, 2024, 6:01 PM
 */

// Global that things like libpic30.h use to calculate delay functions and such.
// MUST be before libpic30c.h

#define FCY 40000000UL

#include "xc.h"
#include <libpic30.h>
#include "mcu_driver.h"
#include "common/can_buffer_fifo.h"
#include "common/can_types.h"
#include "../openlcb/openlcb_types.h"


#include "stdio.h"  // printf


// Oscillator ------------------------------------------------------------------
// Fine tune to get exactly 40Mhz

#define PLLDIV_OFFSET -2


// UART ------------------------------------------------------------------------
//#define FCY 40000000UL

#define FP FCY
#define BAUDRATE 333333
#define BRGVAL_BRGH_L ((FP/BAUDRATE)/16)-1
#define BRGVAL_BRGH_H ((FP/BAUDRATE)/4)-1

#define BRG_OFFSET 0


// ECAN1 ------------------------------------------------------------------------
// First buffer index that is a RX buffer
const uint8_t FIFO_RX_START_INDEX = 8; // (8-31)

// ECAN 80 Mhz oscillator
#define   ECAN_SWJ 2-1
#define   ECAN_BRP 15
// These are 0 indexed so need to subtract one from the value in the ECAN Bit Rate Calculator Tool
#define   ECAN_PROP_SEG 3-1  
#define   ECAN_PHASESEG_1 3-1
#define   ECAN_PHASESEG_2 3-1 
#define   ECAN_TRIPLE_SAMPLE 1
#define   ECAN_PHASESEG_2_PROGRAMMAGLE 1

/* CAN Message Buffer Configuration */
#define ECAN1_MSG_BUF_LENGTH   32
#define ECAN1_MSG_LENGTH_BYTES 8
#define ECAN1_FIFO_LENGTH_BYTES (ECAN1_MSG_BUF_LENGTH * ECAN1_MSG_LENGTH_BYTES * 2)

#define MAX_CAN_FIFO_BUFFER  31
#define MIN_CAN_FIFO_BUFFER  8

#define X 0b0000000000000000;

const uint16_t FIFO_FLAG_MASKS[16] = {0b1111111111111110,
    0b1111111111111101,
    0b1111111111111011,
    0b1111111111110111,
    0b1111111111101111,
    0b1111111111011111,
    0b1111111110111111,
    0b1111111101111111,
    0b1111111011111111,
    0b1111110111111111,
    0b1111101111111111,
    0b1111011111111111,
    0b1110111111111111,
    0b1101111111111111,
    0b1011111111111111,
    0b0111111111111111};

// Internal Types
typedef uint16_t ECAN1MSGBUF[ECAN1_MSG_BUF_LENGTH][ECAN1_MSG_LENGTH_BYTES];


// Internal Variables depending on chip capabilities
#ifdef _HAS_DMA_
__eds__ ECAN1MSGBUF ecan1msgBuf __attribute__((eds, space(dma), aligned(ECAN1_FIFO_LENGTH_BYTES)));
#else
__eds__ ECAN1MSGBUF ecan1msgBuf __attribute__((eds, space(xmemory), aligned(ECAN1_FIFO_LENGTH_BYTES)));
#endif


can_rx_callback_func_t McuDriver_can_rx_callback_func;
_100ms_timer_callback_func_t McuDriver_100ms_timer_callback_func;
uart_rx_callback_func_t McuDriver_uart_rx_callback_func;

/******************************************************************************
 * Function:     void Ecan1WriteRxAcptFilter(int16_t n, int32_t identifier,
 *               uint16_t exide,uint16_t bufPnt,uint16_t maskSel)
 *
 * PreCondition:  None
 *
 * Input:         n-> Filter number [0-15]
 *                identifier-> Bit ordering is given below
 *                Filter Identifier (29-bits) :
 *                0b000f ffff ffff ffff ffff ffff ffff ffff
 *                     |____________|_____________________|
 *                        SID10:0          EID17:0
 *
 *               Filter Identifier (11-bits) :
 *               0b0000 0000 0000 0000 0000 0fff ffff ffff
 *                                           |___________|
 *                                             SID10:
 *               exide -> "0" for standard identifier
 *                        "1" for Extended identifier
 *               bufPnt -> Message buffer to store filtered message [0-15]
 *               maskSel -> Optional Masking of identifier bits [0-3]
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      Configures Acceptance filter "n" for ECAN1.
 *****************************************************************************/
void Ecan1WriteRxAcptFilter(int16_t n, int32_t identifier, uint16_t exide, uint16_t bufPnt, uint16_t maskSel) {

    uint32_t sid10_0 = 0;

    uint32_t eid15_0 = 0;

    uint32_t eid17_16 = 0;
    uint16_t *sidRegAddr;
    uint16_t *bufPntRegAddr;
    uint16_t *maskSelRegAddr;
    uint16_t *fltEnRegAddr;

    C1CTRL1bits.WIN = 1;

    // Obtain the Address of CiRXFnSID, CiBUFPNTn, CiFMSKSELn and CiFEN register for a given filter number "n"
    sidRegAddr = (uint16_t *) (&C1RXF0SID + (n << 1));
    bufPntRegAddr = (uint16_t *) (&C1BUFPNT1 + (n >> 2));
    maskSelRegAddr = (uint16_t *) (&C1FMSKSEL1 + (n >> 3));
    fltEnRegAddr = (uint16_t *) (&C1FEN1);

    // Bit-filed manipulation to write to Filter identifier register
    if (exide == 1) { // Filter Extended Identifier
        eid15_0 = (identifier & 0xFFFF);
        eid17_16 = (identifier >> 16) & 0x3;
        sid10_0 = (identifier >> 18) & 0x7FF;

        *sidRegAddr = (((sid10_0) << 5) + 0x8) + eid17_16; // Write to CiRXFnSID Register
        *(sidRegAddr + 1) = eid15_0; // Write to CiRXFnEID Register
    } else { // Filter Standard Identifier
        sid10_0 = (identifier & 0x7FF);
        *sidRegAddr = (sid10_0) << 5; // Write to CiRXFnSID Register
        *(sidRegAddr + 1) = 0; // Write to CiRXFnEID Register
    }

    *bufPntRegAddr = (*bufPntRegAddr) & (0xFFFF - (0xF << (4 * (n & 3)))); // clear nibble
    *bufPntRegAddr = ((bufPnt << (4 * (n & 3))) | (*bufPntRegAddr)); // Write to C1BUFPNTn Register
    *maskSelRegAddr = (*maskSelRegAddr) & (0xFFFF - (0x3 << ((n & 7) * 2))); // clear 2 bits
    *maskSelRegAddr = ((maskSel << (2 * (n & 7))) | (*maskSelRegAddr)); // Write to C1FMSKSELn Register
    *fltEnRegAddr = ((0x1 << n) | (*fltEnRegAddr)); // Write to C1FEN1 Register
    C1CTRL1bits.WIN = 0;

    return;

}

/******************************************************************************
 * Function:     void Ecan1WriteRxAcptMask(int16_t m, int32_t identifier,
 *               uint16_t mide, uint16_t exide)
 *
 * PreCondition:  None
 *
 * Input:        m-> Mask number [0-2]
                 identifier-> Bit ordering is given below n-> Filter number [0-15]
 *                identifier-> Bit ordering is given below
 *                Filter mask Identifier (29-bits) :
 *                0b000f ffff ffff ffff ffff ffff ffff ffff
 *                     |____________|_____________________|
 *                        SID10:0          EID17:0
 *
 *               Filter mask Identifier (11-bits) :
 *               0b0000 0000 0000 0000 0000 0fff ffff ffff
 *                                           |___________|
 *                                             SID10:
 *               mide ->  "0"  Match either standard or extended address message
 *                             if filters match
 *                        "1"  Match only message types that correpond to
 *                             'exide' bit in filter
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      Configures Acceptance filter "n" for ECAN1.
 *****************************************************************************/
void Ecan1WriteRxAcptMask(int16_t m, int32_t identifier, uint16_t mide, uint16_t exide) {
    uint32_t sid10_0 = 0;

    uint32_t eid15_0 = 0;

    uint32_t eid17_16 = 0;
    uint16_t *maskRegAddr;

    C1CTRL1bits.WIN = 1;

    // Obtain the Address of CiRXMmSID register for given Mask number "m"
    maskRegAddr = (uint16_t *) (&C1RXM0SID + (m << 1));

    // Bit-filed manipulation to write to Filter Mask register
    if (exide == 1) { // Filter Extended Identifier
        eid15_0 = (identifier & 0xFFFF);
        eid17_16 = (identifier >> 16) & 0x3;
        sid10_0 = (identifier >> 18) & 0x7FF;

        if (mide == 1) {
            *maskRegAddr = ((sid10_0) << 5) + 0x0008 + eid17_16; // Write to CiRXMnSID Register
        } else {
            *maskRegAddr = ((sid10_0) << 5) + eid17_16; // Write to CiRXMnSID Register
        }

        *(maskRegAddr + 1) = eid15_0; // Write to CiRXMnEID Register
    } else { // Filter Standard Identifier
        sid10_0 = (identifier & 0x7FF);
        if (mide == 1) {
            *maskRegAddr = ((sid10_0) << 5) + 0x0008; // Write to CiRXMnSID Register
        } else {
            *maskRegAddr = (sid10_0) << 5; // Write to CiRXMnSID Register
        }

        *(maskRegAddr + 1) = 0; // Write to CiRXMnEID Register
    }

    C1CTRL1bits.WIN = 0;

    return;
}

void Ecan1TxBufferSetTransmit(uint16_t buf) {

    switch (buf) {
        case 0:
        {
            C1TR01CONbits.TXREQ0 = 1;
            break;
        }
        case 1:
        {
            C1TR01CONbits.TXREQ1 = 1;
            break;
        }
        case 2:
        {
            C1TR23CONbits.TXREQ2 = 1;
            break;
        }
        case 3:
        {
            C1TR23CONbits.TXREQ3 = 1;
            break;
        }
        case 4:
        {
            C1TR45CONbits.TXREQ4 = 1;
            break;
        }
        case 5:
        {
            C1TR45CONbits.TXREQ5 = 1;
            break;
        }
        case 6:
        {
            C1TR67CONbits.TXREQ6 = 1;
            break;
        }
        case 7:
        {
            C1TR67CONbits.TXREQ7 = 1;
            break;
        }
    }
}

void Ecan1TxBufferClearTransmit(uint16_t buf) {

    switch (buf) {
        case 0:
        {
            C1TR01CONbits.TXREQ0 = 0;
            break;
        }
        case 1:
        {
            C1TR01CONbits.TXREQ1 = 0;
            break;
        }
        case 2:
        {
            C1TR23CONbits.TXREQ2 = 0;
            break;
        }
        case 3:
        {
            C1TR23CONbits.TXREQ3 = 0;
            break;
        }
        case 4:
        {
            C1TR45CONbits.TXREQ4 = 0;
            break;
        }
        case 5:
        {
            C1TR45CONbits.TXREQ5 = 0;
            break;
        }
        case 6:
        {
            C1TR67CONbits.TXREQ6 = 0;
            break;
        }
        case 7:
        {
            C1TR67CONbits.TXREQ7 = 0;
            break;
        }
    }
}

uint8_t Is_Ecan1_TxBuffer_Clear(uint16_t buf) {

    switch (buf) {
        case 0: return (C1TR01CONbits.TXREQ0 == 0);
        case 1: return (C1TR01CONbits.TXREQ1 == 0);
        case 2: return (C1TR23CONbits.TXREQ2 == 0);
        case 3: return (C1TR23CONbits.TXREQ3 == 0);
        case 4: return (C1TR45CONbits.TXREQ4 == 0);
        case 5: return (C1TR45CONbits.TXREQ5 == 0);
        case 6: return (C1TR67CONbits.TXREQ6 == 0);
        case 7: return (C1TR67CONbits.TXREQ7 == 0);
        default: return 0;
    }

}

uint8_t McuDriver_is_can_tx_buffer_clear(uint16_t Channel) {

    return Is_Ecan1_TxBuffer_Clear(Channel);

}

/******************************************************************************
 * Function:     void Ecan1WriteTxMsgBufId(uint16_t buf, int32_t txIdentifier, uint16_t ide,
 *               uint16_t remoteTransmit)
 *
 * PreCondition:  None
 *
 * Input:        buf    -> Transmit Buffer Number
 *               txIdentifier ->
 *               Extended Identifier (29-bits):
 *                0b000f ffff ffff ffff ffff ffff ffff ffff
 *                     |____________|_____________________|
 *                        SID10:0          EID17:0
 *
 *               Standard Identifier (11-bits) :
 *               0b0000 0000 0000 0000 0000 0fff ffff ffff
 *                                           |___________|
 *                                             SID10:
 *                 Standard Message Format:
 *                                             Word0 : 0b000f ffff ffff ffff
 *                                                          |____________|||___
 *                                                             SID10:0   SRR   IDE
 *                                             Word1 : 0b0000 0000 0000 0000
 *                                                            |____________|
 *                                                               EID17:6
 *                                             Word2 : 0b0000 00f0 0000 ffff
 *                                                       |_____||           |__|
 *                                                       EID5:0 RTR         DLC
 *                Extended Message Format:
 *                                          Word0 : 0b000f ffff ffff ffff
 *                                                       |____________|||___
 *                                                          SID10:0   SRR   IDE
 *                                          Word1 : 0b0000 ffff ffff ffff
 *                                                         |____________|
 *                                                               EID17:6
 *                                          Word2 : 0bffff fff0 0000 ffff
 *                                                    |_____||           |__|
 *                                                    EID5:0 RTR         DLC
 *             ide -> "0"  Message will transmit standard identifier
 *                    "1"  Message will transmit extended identifier
 *
 *            remoteTransmit -> "0" Message transmitted is a normal message
 *                              "1" Message transmitted is a remote message
 *            Standard Message Format:
 *                                          Word0 : 0b000f ffff ffff ff1f
 *                                                       |____________|||___
 *                                                          SID10:0   SRR   IDE
 *                                          Word1 : 0b0000 0000 0000 0000
 *                                                         |____________|
 *                                                            EID17:6
 *                                          Word2 : 0b0000 0010 0000 ffff
 *                                                  |_____||           |__|
 *                                                  EID5:0 RTR         DLC
 *
 *         Extended Message Format:
 *                                         Word0 : 0b000f ffff ffff ff1f
 *                                                      |____________|||___
 *                                                        SID10:0   SRR   IDE
 *                                         Word1 : 0b0000 ffff ffff ffff
 *                                                        |____________|
 *                                                          EID17:6
 *                                         Word2 : 0bffff ff10 0000 ffff
 *                                                   |_____||           |__|
 *                                                   EID5:0 RTR         DLC
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      This function configures ECAN1 message buffer.
 *****************************************************************************/
void Ecan1WriteTxMsgBufId(uint16_t buf, int32_t txIdentifier, uint16_t ide, uint16_t remoteTransmit) {
    uint32_t word0 = 0;

    uint32_t word1 = 0;

    uint32_t word2 = 0;
    uint32_t sid10_0 = 0;
    uint32_t eid5_0 = 0;
    uint32_t eid17_6 = 0;

    if (ide) {
        eid5_0 = (txIdentifier & 0x3F);
        eid17_6 = (txIdentifier >> 6) & 0xFFF;
        sid10_0 = (txIdentifier >> 18) & 0x7FF;
        word1 = eid17_6;
    } else {
        sid10_0 = (txIdentifier & 0x7FF);
    }

    if (remoteTransmit == 1) { // Transmit Remote Frame
        word0 = ((sid10_0 << 2) | ide | 0x2); // IDE and SRR are 1
        word2 = ((eid5_0 << 10) | 0x0200); // RTR is 1
    } else {
        word0 = ((sid10_0 << 2) | ide); // IDE is 1 and SRR is 0
        word2 = (eid5_0 << 10); // RTR is 0
    }

    // Obtain the Address of Transmit Buffer in DMA RAM for a given Transmit Buffer number
    if (ide) {
        ecan1msgBuf[buf][0] = (word0 | 0x0002); // SRR is 1
    } else {
        ecan1msgBuf[buf][0] = word0; // SRR is 0
    }

    ecan1msgBuf[buf][1] = word1;
    ecan1msgBuf[buf][2] = word2; // RB1, RB0 are set to 0.   DCL is initialized to 0;

    return;

}

/******************************************************************************
 * Function:     void Ecan1WriteTxMsgBufData(uint16_t buf, uint16_t dataLength,
 *    uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4)
 *
 * PreCondition:  None
 *
 * Input:            buf    -> Transmit Buffer Number
 *              dataLength  -> data length in bytes.
 *    	        actual data -> data1, data2, data3, data4
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      This function transmits ECAN data.
 *****************************************************************************/
void Ecan1WriteTxMsgBufData(uint16_t buf, uint16_t data_length, payload_bytes_can_t* data) {


    ecan1msgBuf[buf][2] = ((ecan1msgBuf[buf][2] & 0xFFF0) + data_length); // DCL = number of valid data bytes

    if ((data_length > 0) && data) {

        ecan1msgBuf[buf][3] = ((*data)[1] << 8) | (*data)[0];
        ecan1msgBuf[buf][4] = ((*data)[3] << 8) | (*data)[2];
        ecan1msgBuf[buf][5] = ((*data)[5] << 8) | (*data)[4];
        ecan1msgBuf[buf][6] = ((*data)[7] << 8) | (*data)[6];
    }

}

/******************************************************************************
 * Function:     void Ecan1WriteTxMsgBufData(uint16_t buf, uint16_t dataLength,
 *    uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4)
 *
 * PreCondition:  None
 *
 * Input:            buf    -> Transmit Buffer Number
 *              dataLength  -> data length in bytes.
 *    	        actual data -> data1, data2, data3, data4
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:      This function transmits ECAN data.
 *****************************************************************************/
void Ecan1WriteTxMsgBufDataWord(uint16_t buf, uint16_t data_length, uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4) {

    ecan1msgBuf[buf][2] = ((ecan1msgBuf[buf][2] & 0xFFF0) + data_length); // DCL = number of valid data bytes

    ecan1msgBuf[buf][3] = data1;
    ecan1msgBuf[buf][4] = data2;
    ecan1msgBuf[buf][5] = data3;
    ecan1msgBuf[buf][6] = data4;

}

void Ecan1WriteTxMsgBufDataByte(uint16_t buf, uint16_t data_length, uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4, uint16_t data5, uint16_t data6, uint16_t data7, uint16_t data8) {

    ecan1msgBuf[buf][2] = ((ecan1msgBuf[buf][2] & 0xFFF0) + data_length); // DCL = number of valid data bytes

    if (data_length > 0) {

        ecan1msgBuf[buf][3] = (data2 << 8) | data1;
        ecan1msgBuf[buf][4] = (data4 << 8) | data3;
        ecan1msgBuf[buf][5] = (data6 << 8) | data5;
        ecan1msgBuf[buf][6] = (data8 << 8) | data7;

    }

}

/*
 * buf:     [IN] The buffer index to read
 * *rxData: [OUT] A pointer to the Data payload for the CAN message
 * *ide:    [OUT} A pointer to a boolean that indicates if the message is extended or not
 */
void Ecan1ReadRxMsgBufId(uint16_t buf, can_msg_t *rxData, uint16_t *ide) {

    uint32_t sid, eid_17_6, eid_5_0;

    sid = (0x1FFC & ecan1msgBuf[buf][0]) >> 2; // 0b0001111111111100 ;
    eid_17_6 = ecan1msgBuf[buf][1];
    eid_5_0 = (ecan1msgBuf[buf][2] >> 10);

    *(ide) = ecan1msgBuf[buf][0] & 0x0001;

    // Is the IDE bit set?
    if (*ide) {

        rxData->identifier = (sid << 18) | (eid_17_6 << 6) | eid_5_0;

    } else {

        rxData->identifier = sid;

    }

    return;

}

/*
 * buf:     [IN] The buffer index to read
 * *rxData: [OUT] A pointer to the Data payload for the CAN message
 */
void Ecan1ReadRxMsgBufData(uint16_t buf, can_msg_t *rxData) {

    rxData->payload_count = ecan1msgBuf[buf][2] & 0x000F;

    rxData->payload[0] = (uint8_t) ecan1msgBuf[buf][3];
    rxData->payload[1] = (uint8_t) (ecan1msgBuf[buf][3] >> 8);

    rxData->payload[2] = (uint8_t) ecan1msgBuf[buf][4];
    rxData->payload[3] = (uint8_t) (ecan1msgBuf[buf][4] >> 8);

    rxData->payload[4] = (uint8_t) ecan1msgBuf[buf][5];
    rxData->payload[5] = (uint8_t) (ecan1msgBuf[buf][5] >> 8);

    rxData->payload[6] = (uint8_t) ecan1msgBuf[buf][6];
    rxData->payload[7] = (uint8_t) (ecan1msgBuf[buf][6] >> 8);

    return;

}

uint8_t McuDriver_transmit_raw_can_frame(uint8_t channel, can_msg_t* msg) {


    if (Is_Ecan1_TxBuffer_Clear(channel)) {

#ifndef DEBUG

        Ecan1WriteTxMsgBufId(channel, msg->identifier, 1, 0);
        Ecan1WriteTxMsgBufData(channel, msg->payload_count, &msg->payload);
        Ecan1TxBufferSetTransmit(channel);

#endif

        return 1;
    }

    return 0;
}


// Timer 2 Interrupt 100ms timer

void __attribute__((interrupt(no_auto_psv))) _T2Interrupt(void) {

    IFS0bits.T2IF = 0; // Clear T2IF

    // Increment any timer counters assigned
    if (McuDriver_100ms_timer_callback_func)
        McuDriver_100ms_timer_callback_func();

    return;
}

// UART1 Transmit Interrupt

void __attribute__((interrupt(no_auto_psv))) _U1TXInterrupt(void) {

    IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag  

    return;
}

// UART1 Receive Interrupt

void __attribute__((interrupt(no_auto_psv))) _U1RXInterrupt(void) {

    IFS0bits.U1RXIF = 0; // Clear RX Interrupt flag 

    if (U1STAbits.URXDA == 1) {

        if (McuDriver_uart_rx_callback_func)
            McuDriver_uart_rx_callback_func(U1RXREG);

    }
    return;
}

// CAN 1 Interrupt

uint8_t max_can_fifo_depth = 0;

void __attribute__((interrupt(no_auto_psv))) _C1Interrupt(void) {

    /* clear interrupt flag */
    IFS2bits.C1IF = 0; // clear interrupt flag

    if (C1INTFbits.RBIF) { // RX Interrupt

        // Snag all the buffers that have data that are associated with this interrupt
        uint8_t buffer_tail = _FNRB;
        uint8_t buffer_head = _FBP;

        // Now reset the interrupt so anything that comes in from here on will reset the interrupt.  
        // Believe this will keep from missing a message
        C1INTFbits.RBIF = 0;

        uint8_t fifo_size = 0;
        uint16_t ide = 0;
        can_msg_t ecan_msg;

        while (buffer_tail != buffer_head) {

            Ecan1ReadRxMsgBufId(buffer_tail, &ecan_msg, &ide);
            Ecan1ReadRxMsgBufData(buffer_tail, &ecan_msg);

            if ((ide) && (McuDriver_can_rx_callback_func))
                McuDriver_can_rx_callback_func(buffer_tail, &ecan_msg);

            // Clear Full/OV flags on any bit that is set, there is a race condition for this.  See the errata
            // You can only clear (set a 0) to the flags so if we write a 1 it won't do anything
            // Don't do anything that reads/modified/writes like a BitSet.  Needs to be atomic
            if (buffer_tail < 16) {
                C1RXFUL1 = FIFO_FLAG_MASKS[buffer_tail];
                C1RXOVF1 = FIFO_FLAG_MASKS[buffer_tail];
            } else {
                C1RXFUL2 = FIFO_FLAG_MASKS[buffer_tail - 16];
                C1RXOVF2 = FIFO_FLAG_MASKS[buffer_tail - 16];
            }

            buffer_tail = buffer_tail + 1;
            if (buffer_tail > MAX_CAN_FIFO_BUFFER)
                buffer_tail = MIN_CAN_FIFO_BUFFER;

            fifo_size = fifo_size + 1;

            if (fifo_size > max_can_fifo_depth)
                max_can_fifo_depth = fifo_size;

        };

    } else { // TX Interrupt
        if (C1INTFbits.TBIF) {

            C1INTFbits.TBIF = 0;

        }

    }

    return;

}

// DMA2 Interrupt = CAN Receive

void __attribute__((interrupt(no_auto_psv))) _DMA2Interrupt(void) {

    IFS1bits.DMA2IF = 0;
    //   printf("DMA2 IRQ CAN RX\n");

    return;
}


// DMA0 Interrupt = CAN Transmit

void __attribute__((interrupt(no_auto_psv))) _DMA0Interrupt(void) {

    IFS0bits.DMA0IF = 0;
    //   printf("DMA0 IRQ CAN TX\n");

    return;
}

void McuDriver_initialization(void) {

    // SPI1 Initialize ---------------------------------------------------------
    // -------------------------------------------------------------------------


    _TRISB7 = 0; // CLK
    _TRISB8 = 0; // SDO
    _TRISB6 = 0; // CS

    _RB7 = 0;
    _RB8 = 0;
    _RB6 = 1;

    IFS0bits.SPI1IF = 0; // Clear the Interrupt flag
    IEC0bits.SPI1IE = 0; // Disable the interrupt

    SPI1CON1bits.SPRE = 0b000;
    SPI1CON1bits.PPRE = 0b10;
    
    SPI1CON1bits.DISSCK = 0; // Internal serial clock is enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 0; // Communication is byte-wide (8 bits)
    SPI1CON1bits.MSTEN = 1; // Master mode enabled
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time
    SPI1CON1bits.CKE = 1; // Serial output data changes on transition from
    // Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for clock is a low level;
    // active state is a high level
    SPI1STATbits.SPIEN = 1; // Enable SPI module
   

    // -------------------------------------------------------------------------


    // UART Initialize ---------------------------------------------------------
    // -------------------------------------------------------------------------
    U1MODEbits.STSEL = 0; // 1-Stop bit
    U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled
    U1MODEbits.BRGH = 1; //Speed mode 1 = High
    U1BRG = BRGVAL_BRGH_H + BRG_OFFSET; // Baud Rate setting

    U1STAbits.UTXISEL0 = 0; // Interrupt after one TX character is transmitted
    U1STAbits.UTXISEL1 = 0;
    IEC0bits.U1TXIE = 1; // Enable UART TX interrupt


    IEC0bits.U1RXIE = 1; // Enable UART RX interrupt
    U1STAbits.URXISEL0 = 0; // Interrupt after one RX character is received;
    U1STAbits.URXISEL1 = 0;

    U1MODEbits.UARTEN = 1; // Enable UART
    U1STAbits.UTXEN = 1; // Enable UART TX .. must be after the overall UART Enable

    /* Wait at least 4.3 microseconds (1/230400) before sending first char */
    __delay_us(10);
    // -------------------------------------------------------------------------


    // ECAN1 Initialize --------------------------------------------------------
    // -------------------------------------------------------------------------

    /* Request Configuration Mode */
    C1CTRL1bits.REQOP = 4;
    while (C1CTRL1bits.OPMODE != 4);

    /* Synchronization Jump Width */
    C1CFG1bits.SJW = ECAN_SWJ;
    /* Baud Rate Prescaler */
    C1CFG1bits.BRP = ECAN_BRP;
    /* Phase Segment 1 time  */
    C1CFG2bits.SEG1PH = ECAN_PHASESEG_1;
    /* Phase Segment 2 time is set to be programmable or fixed */
    C1CFG2bits.SEG2PHTS = ECAN_PHASESEG_2_PROGRAMMAGLE;
    /* Phase Segment 2 time  */
    C1CFG2bits.SEG2PH = ECAN_PHASESEG_2;
    /* Propagation Segment time  */
    C1CFG2bits.PRSEG = ECAN_PROP_SEG;
    /* Bus line is sampled three times/one time at the sample point */
    C1CFG2bits.SAM = ECAN_TRIPLE_SAMPLE;
    // Full rate clock, no divide by 2
    C1CTRL1bits.CANCKS = 0x0;


    // Make sure to update these defines above if changed
    //  #define MAX_CAN_FIFO_BUFFER = 31;
    //  #define MIN_CAN_FIFO_BUFFER = 8;
    C1FCTRLbits.FSA = 0b01000; // FIFO Start Area: RX FIFO Starts at Message Buffer 8 (0-7 are TX) 
    C1FCTRLbits.DMABS = 0b111; // 32 CAN Message Buffers in DMA RAM, 8 TX (0-7) and 24 RX (8-31)

    // Need to setup a NULL filter/Mask to receive messages
    // Filter 0, Filter SID/EID = 0x00000000, Extended Message = 1, FIFO index to write it to the next available buffer (and not to a specific buffer index), No Mask SID/EID used
    Ecan1WriteRxAcptFilter(0, 0x00000000, 1, 0b1111, 0);

    /* Enter Normal Mode */
    C1CTRL1bits.REQOP = 0;
    while (C1CTRL1bits.OPMODE != 0);

    /* ECAN transmit/receive message control */
    C1RXFUL1 = C1RXFUL2 = C1RXOVF1 = C1RXOVF2 = 0x0000;

    /*
    bit 7 - TXENm: TX/RX Buffer Selection bit
       1 = Buffer TRBn is a transmit buffer 0 = Buffer TRBn is a receive buffer
    bit 6 - TXABTm: Message Aborted bit(1)
       1 = Message was aborted
       0 = Message completed transmission successfully
    bit 5 - TXLARBm: Message Lost Arbitration bit(1)
       1 = Message lost arbitration while being sent
       0 = Message did not lose arbitration while being sent
    bit 4 - TXERRm: Error Detected During Transmission bit(1)
       1 = A bus error occurred while the message was being sent
       0 = A bus error did not occur while the message was being sent
    bit 3 - TXREQm: Message Send Request bit
       1 = Requests that a message be sent; the bit automatically clears when the message is successfully sent
       0 = Clearing the bit to ?0? while set requests a message abort
    bit 2 - RTRENm: Auto-Remote Transmit Enable bit
       1 = When a remote transmit is received, TXREQ will be set
       0 = When a remote transmit is received, TXREQ will be unaffected
    bit 1-0 - TXmPRI<1:0>:: Auto-Remote Transmit Enable bit
       1 = When a remote transmit is received, TXREQ will be set
       0 = When a remote transmit is received, TXREQ will be unaffected TXmPRI<1:0>: Message Transmission Priority bits
       11 = Highest message priority
       10 = High intermediate message priority 01 = Low intermediate message priority 00 = Lowest message priority 
     */

    C1TR01CON = 0x8382; // ECAN1 Buffer 0 and 1 is a TX Buffer and Highest Priority
    C1TR23CON = 0x8180; // ECAN1 Buffer 2 and 3 is a TX Buffer and Higher Priority
    C1TR45CON = 0x8080; // ECAN1 Buffer 4 and 5 is a TX Buffer and Low Priority
    C1TR67CON = 0x8080; // ECAN1 Buffer 6 and 7 is a TX Buffer and Lowest Priority


    /* Enable ECAN1 Interrupts */
    IEC2bits.C1IE = 1; // Enable CAN1 interrupts globally
    C1INTEbits.TBIE = 1; // Enable CAN1 TX
    C1INTEbits.RBIE = 1; // Enable CAN1 RX
    //
    //    C1INTEbits.ERRIE = 1;  // Enable Error Interrupts
    //    C1INTEbits.RBOVIE = 1; // Enable Rx Error Interrupt
    //    C1INTEbits.FIFOIE = 1; // Enable FIFO Overflow Interrupt
    //    C1INTEbits.WAKIE = 1;  // Enable LowPower Wakeup Interrupt
    //    C1INTEbits.IVRIE = 1;  // Enable General Error (like if the baud rates don't match) Interrupt

    // -------------------------------------------------------------------------


    // Timer Initialize --------------------------------------------------------
    // -------------------------------------------------------------------------

    IPC1bits.T2IP0 = 1; // Timer 2 Interrupt Priority = 5   (1 means off)
    IPC1bits.T2IP1 = 0;
    IPC1bits.T2IP2 = 1;

    T2CONbits.TCS = 0; // internal clock
    T2CONbits.TCKPS0 = 1; // 256 Prescaler
    T2CONbits.TCKPS1 = 1;
    PR2 = 15625; // Clock ticks every (1/80MHz * 2 * 256 * 15625 = 100.00091ms interrupts

    IFS0bits.T2IF = 0; // Clear T2IF
    IEC0bits.T2IE = 1; // Enable the Interrupt

    T2CONbits.TON = 1; // Turn on 100ms Timer

    // -------------------------------------------------------------------------

    // Oscillator Initialize ---------------------------------------------------
    // -------------------------------------------------------------------------
    // Make sure the Fuse bits are set to

    //   011 = Primary Oscillator with PLL (XTPLL, HSPLL, ECPLL)

    // Setting output frequency to 140MHz
    PLLFBDbits.PLLDIV = 60 + PLLDIV_OFFSET; // This should be 60 for 80 Mhz.  Need 80 Mhz because the CAN module is limited to Fcy = 40 Mhz
    CLKDIV = 0x0001; // PreScaler divide by 3; Post Scaler divide by 2

    // PLLFBDbits.PLLDIV = 68;             // PLL multiplier M=68 140Mh
    // CLKDIV = 0x0000;         // PLL prescaler N1=2, PLL postscaler N2=2
    // -------------------------------------------------------------------------


    // IO Pin Initialize -------------------------------------------------------
    // -------------------------------------------------------------------------

    ANSELA = 0x00; // Convert all I/O pins to digital
    ANSELB = 0x00;
    // -------------------------------------------------------------------------


    // Peripheral Pin Select Initialize ----------------------------------------
    // -------------------------------------------------------------------------

    // Make sure PPS Multiple reconfigurations is selected in the Configuration Fuse Bits

    // CAN Pins
    RPINR26bits.C1RXR = 45; // RPI45 CAN RX
    RPOR4bits.RP43R = _RPOUT_C1TX; // RP43 CAN TX

    // UART Pins
    RPINR18bits.U1RXR = 44; // RPI44 UART RX
    RPOR4bits.RP42R = _RPOUT_U1TX; // RP42  UART TX
    // -------------------------------------------------------------------------



    // DMA 2 Initialize (CAN RX) -----------------------------------------------
    // -------------------------------------------------------------------------

    //    DMA2CONbits.CHEN  = 0; // Disabled
    //    DMA2CONbits.SIZE  = 0; // Word
    //    DMA2CONbits.DIR   = 0; // Read from Peripheral address, write to DPSRAM (or RAM) address
    //    DMA2CONbits.HALF  = 0; // Initiate interrupt when all of the data has been moved
    //    DMA2CONbits.NULLW = 0; // 0 = Normal operation
    //    DMA2CONbits.AMODE = 2; // Peripheral Indirect Addressing mode
    //    DMA2CONbits.MODE  = 0; // Continuous, Ping-Pong modes disabled
    DMA2CON = 0x0020;
    DMA2PAD = (uint16_t) & C1RXD; // (C1RXD) associate this DMA Channel to the peripheral data register associated with CAN1 RX
    DMA2CNT = 0x0007; // 7 data bytes
    DMA2REQ = 0x0022; // associate this DMA Channel to the peripheral interrupt associated with CAN1 RX, DMAxREQ.IRQSEL

#ifdef _HAS_DMA_
    DMA2STAL = __builtin_dmaoffset(ecan1msgBuf);
    DMA2STAH = __builtin_dmapage(ecan1msgBuf);
#else
    DMA2STAL = (uint16_t) (int_least24_t) (&ecan1msgBuf);
    DMA2STAH = 0; // 32k of RAM in any chip being used for this project
#endif

    IFS1bits.DMA2IF = 0; // Clear interrupt flag
    IEC1bits.DMA2IE = 1; // Enable interrupt

    DMA2CONbits.CHEN = 1;
    // -------------------------------------------------------------------------


    // DMA0 Initialize (CAN TX) ------------------------------------------------
    // -------------------------------------------------------------------------

    //    DMA0CONbits.CHEN  = 0; // Disabled
    //    DMA0CONbits.SIZE  = 0; // Word
    //    DMA0CONbits.DIR   = 1; // Read from DPSRAM (or RAM) address, write to peripheral address
    //    DMA0CONbits.HALF  = 0; // Initiate interrupt when all of the data has been moved
    //    DMA0CONbits.NULLW = 0; // 0 = Normal operation
    //    DMA0CONbits.AMODE = 2; // Peripheral Indirect Addressing mode
    //    DMA0CONbits.MODE  = 0; // Continuous, Ping-Pong modes disabled
    DMA0CON = 0x2020;
    DMA0PAD = (uint16_t) & C1TXD; // ECAN 1 (C1TXD) associate this DMA Channel to the peripheral data interrupt associated with CAN1 TX
    DMA0CNT = 0x0007; // 7 data bytes
    DMA0REQ = 0x0046; // associate this DMA Channel to the peripheral interrupt associated with CAN1 TX, DMAxREQ.IRQSEL

#ifdef _HAS_DMA_
    DMA0STAL = __builtin_dmaoffset(ecan1msgBuf);
    DMA0STAH = __builtin_dmapage(ecan1msgBuf);
#else
    DMA0STAL = (uint16_t) (int_least24_t) (&ecan1msgBuf);
    DMA0STAH = 0; // 32k of RAM in any chip being used for this project
#endif

    IFS0bits.DMA0IF = 0; // Clear interrupt flag
    IEC0bits.DMA0IE = 1; // Enable interrupt

    DMA0CONbits.CHEN = 1;
    // -------------------------------------------------------------------------

    return;

};

/******************************************************************************
 * Function:     void Ecan1DisableRXFilter(int16_t n)
 *
 * PreCondition:  None
 *
 * Input:          n -> Filter number [0-15]
 *
 * Output:        None
 *
 * Side Effects:  None
 *
 * Overview:          Disables RX Acceptance Filter.
 *****************************************************************************/
void Ecan1DisableRXFilter(int16_t n) {
    uint16_t *fltEnRegAddr;
    C1CTRL1bits.WIN = 1;
    fltEnRegAddr = (uint16_t *) (&C1FEN1);
    *fltEnRegAddr = (*fltEnRegAddr) & (0xFFFF - (0x1 << n));
    C1CTRL1bits.WIN = 0;

    return;

}

void McuDriver_pause_can_rx() {

    C1INTEbits.RBIE = 0; // Enable CAN1 RX 

};

void McuDriver_resume_can_rx() {

    C1INTEbits.RBIE = 1; // Enable CAN1 RX

};

void McuDriver_pause_can_tx_complete_notify() {

    C1INTEbits.TBIE = 0; // Enable CAN1 TX

};

void McuDriver_resume_can_tx_complete_notify() {

    C1INTEbits.TBIE = 1; // Enable CAN1 TX

};

void McuDriver_pause_100ms_timer() {

    IEC0bits.T2IE = 0;

}

void McuDriver_resume_100ms_timer() {

    IEC0bits.T2IE = 1;

}

void McuDriver_read_eeprom(uint32_t address, uint16_t count, _eeprom_read_buffer_t* buffer) {
    
    _25AA1024_Driver_read(address, count, buffer);
    
}

void McuDriver_write_eeprom(uint32_t address, uint16_t count, _eeprom_read_buffer_t* buffer) {
    
    _25AA1024_Driver_write_latch_enable();
    _25AA1024_Driver_write(address, count, buffer);
    
    while (_25AA1024_Driver_write_in_progress()) {
        
    }
    
}



