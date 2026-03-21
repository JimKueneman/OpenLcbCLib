/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * @file dspic33_can_drivers.c
 *
 * CAN hardware interface for dsPIC33 ECAN module.  Handles ECAN register
 * setup, DMA buffer management, bit-timing, transmit, receive, and the
 * C1 interrupt service routine.
 *
 * @author Jim Kueneman
 * @date 19 Mar 2026
 */

#include "dspic33_can_drivers.h"

#include "xc.h"

#include <libpic30.h>
#include <stdbool.h>
#include <stdint.h>

#include "../openlcb_c_lib/drivers/canbus/can_rx_statemachine.h"

static uint8_t _max_can_fifo_depth = 0;

#define TX_CHANNEL_CAN_CONTROL 0

// First buffer index that is a RX buffer
const uint8_t FIFO_RX_START_INDEX = 8; // (8-31)

// ECAN 80 Mhz oscillator
// Make sure FCY is defined in the compiler macros and set to 40000000UL (80Mhz/2)

#define ECAN_SWJ 2 - 1
#define ECAN_BRP 15
// These are 0 indexed so need to subtract one from the value in the ECAN Bit Rate Calculator Tool
#define ECAN_PROP_SEG 3 - 1
#define ECAN_PHASESEG_1 3 - 1
#define ECAN_PHASESEG_2 3 - 1
#define ECAN_TRIPLE_SAMPLE 1
#define ECAN_PHASESEG_2_PROGRAMMAGLE 1

/* CAN Message Buffer Configuration */
#define ECAN1_MSG_BUF_LENGTH 32
#define ECAN1_MSG_LENGTH_BYTES 8
#define ECAN1_FIFO_LENGTH_BYTES (ECAN1_MSG_BUF_LENGTH * ECAN1_MSG_LENGTH_BYTES * 2)

#define MAX_CAN_FIFO_BUFFER 31
#define MIN_CAN_FIFO_BUFFER 8

const uint16_t FIFO_FLAG_MASKS[16] = {

    0b1111111111111110,
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
    0b0111111111111111

};

// Internal Types
typedef uint16_t ECAN1MSGBUF[ECAN1_MSG_BUF_LENGTH][ECAN1_MSG_LENGTH_BYTES];

// Internal Variables depending on chip capabilities
#ifdef _HAS_DMA_
__eds__ ECAN1MSGBUF ecan1msgBuf __attribute__((eds, space(dma), aligned(ECAN1_FIFO_LENGTH_BYTES)));
#else
__eds__ ECAN1MSGBUF ecan1msgBuf __attribute__((eds, space(xmemory), aligned(ECAN1_FIFO_LENGTH_BYTES)));
#endif

static void _ecan1_write_rx_acpt_filter(int16_t n, uint32_t identifier, uint16_t exide, uint16_t bufPnt, uint16_t maskSel)
{

    uint32_t sid10_0 = 0;
    uint32_t eid15_0 = 0;
    uint32_t eid17_16 = 0;
    uint16_t *sidRegAddr;
    uint16_t *bufPntRegAddr;
    uint16_t *maskSelRegAddr;
    uint16_t *fltEnRegAddr;

    C1CTRL1bits.WIN = 1;

    // Obtain the Address of CiRXFnSID, CiBUFPNTn, CiFMSKSELn and CiFEN register for a given filter number "n"
    sidRegAddr = (uint16_t *)(&C1RXF0SID + (n << 1));
    bufPntRegAddr = (uint16_t *)(&C1BUFPNT1 + (n >> 2));
    maskSelRegAddr = (uint16_t *)(&C1FMSKSEL1 + (n >> 3));
    fltEnRegAddr = (uint16_t *)(&C1FEN1);

    // Bit-filed manipulation to write to Filter identifier register
    if (exide == 1)
    { // Filter Extended Identifier

        eid15_0 = (identifier & 0xFFFF);
        eid17_16 = (identifier >> 16) & 0x3;
        sid10_0 = (identifier >> 18) & 0x7FF;

        *sidRegAddr = (((sid10_0) << 5) + 0x8) + eid17_16; // Write to CiRXFnSID Register
        *(sidRegAddr + 1) = eid15_0;                       // Write to CiRXFnEID Register

    }
    else
    { // Filter Standard Identifier

        sid10_0 = (identifier & 0x7FF);
        *sidRegAddr = (sid10_0) << 5; // Write to CiRXFnSID Register
        *(sidRegAddr + 1) = 0;        // Write to CiRXFnEID Register

    }

    *bufPntRegAddr = (*bufPntRegAddr) & (0xFFFF - (0xF << (4 * (n & 3))));   // clear nibble
    *bufPntRegAddr = ((bufPnt << (4 * (n & 3))) | (*bufPntRegAddr));         // Write to C1BUFPNTn Register
    *maskSelRegAddr = (*maskSelRegAddr) & (0xFFFF - (0x3 << ((n & 7) * 2))); // clear 2 bits
    *maskSelRegAddr = ((maskSel << (2 * (n & 7))) | (*maskSelRegAddr));      // Write to C1FMSKSELn Register
    *fltEnRegAddr = ((0x1 << n) | (*fltEnRegAddr));                          // Write to C1FEN1 Register
    C1CTRL1bits.WIN = 0;

}

static void _ecan1_tx_buffer_set_transmit(uint16_t buf)
{

    switch (buf)
    {

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

static void _ecan1_write_tx_msg_buf_id(uint16_t buf, uint32_t txIdentifier, uint16_t ide, uint16_t remoteTransmit)
{

    uint32_t word0 = 0;
    uint32_t word1 = 0;
    uint32_t word2 = 0;
    uint32_t sid10_0 = 0;
    uint32_t eid5_0 = 0;
    uint32_t eid17_6 = 0;

    if (ide)
    {

        eid5_0 = (txIdentifier & 0x3F);
        eid17_6 = (txIdentifier >> 6) & 0xFFF;
        sid10_0 = (txIdentifier >> 18) & 0x7FF;
        word1 = eid17_6;

    }
    else
    {

        sid10_0 = (txIdentifier & 0x7FF);

    }

    if (remoteTransmit == 1)
    {                                         // Transmit Remote Frame

        word0 = ((sid10_0 << 2) | ide | 0x2); // IDE and SRR are 1
        word2 = ((eid5_0 << 10) | 0x0200);    // RTR is 1

    }
    else
    {

        word0 = ((sid10_0 << 2) | ide); // IDE is 1 and SRR is 0
        word2 = (eid5_0 << 10);         // RTR is 0

    }

    // Obtain the Address of Transmit Buffer in DMA RAM for a given Transmit Buffer number
    if (ide)
    {

        ecan1msgBuf[buf][0] = (word0 | 0x0002); // SRR is 1

    }
    else
    {

        ecan1msgBuf[buf][0] = word0; // SRR is 0

    }

    ecan1msgBuf[buf][1] = word1;
    ecan1msgBuf[buf][2] = word2; // RB1, RB0 are set to 0.   DCL is initialized to 0;

}

static void _ecan1_write_tx_msg_buf_data(uint16_t buf, uint16_t data_length, payload_bytes_can_t *data)
{

    ecan1msgBuf[buf][2] = ((ecan1msgBuf[buf][2] & 0xFFF0) + data_length); // DCL = number of valid data bytes

    if ((data_length > 0) && data)
    {

        ecan1msgBuf[buf][3] = ((*data)[1] << 8) | (*data)[0];
        ecan1msgBuf[buf][4] = ((*data)[3] << 8) | (*data)[2];
        ecan1msgBuf[buf][5] = ((*data)[5] << 8) | (*data)[4];
        ecan1msgBuf[buf][6] = ((*data)[7] << 8) | (*data)[6];

    }

}

static void _ecan1_read_rx_msg_buf_id(uint16_t buf, can_msg_t *rxData, uint16_t *ide)
{

    uint32_t sid, eid_17_6, eid_5_0;

    sid = (0x1FFC & ecan1msgBuf[buf][0]) >> 2;
    eid_17_6 = ecan1msgBuf[buf][1];
    eid_5_0 = (ecan1msgBuf[buf][2] >> 10);

    *(ide) = ecan1msgBuf[buf][0] & 0x0001;

    if (*ide)
    {

        rxData->identifier = (sid << 18) | (eid_17_6 << 6) | eid_5_0;

    }
    else
    {

        rxData->identifier = sid;

    }

}

static void _ecan1_read_rx_msg_buf_data(uint16_t buf, can_msg_t *rxData)
{

    rxData->payload_count = ecan1msgBuf[buf][2] & 0x000F;

    rxData->payload[0] = (uint8_t)ecan1msgBuf[buf][3];
    rxData->payload[1] = (uint8_t)(ecan1msgBuf[buf][3] >> 8);

    rxData->payload[2] = (uint8_t)ecan1msgBuf[buf][4];
    rxData->payload[3] = (uint8_t)(ecan1msgBuf[buf][4] >> 8);

    rxData->payload[4] = (uint8_t)ecan1msgBuf[buf][5];
    rxData->payload[5] = (uint8_t)(ecan1msgBuf[buf][5] >> 8);

    rxData->payload[6] = (uint8_t)ecan1msgBuf[buf][6];
    rxData->payload[7] = (uint8_t)(ecan1msgBuf[buf][6] >> 8);

}

static bool _is_can_tx_buffer_clear(uint16_t channel)
{

    switch (channel)
    {

        case 0:
            return (C1TR01CONbits.TXREQ0 == 0);

        case 1:
            return (C1TR01CONbits.TXREQ1 == 0);

        case 2:
            return (C1TR23CONbits.TXREQ2 == 0);

        case 3:
            return (C1TR23CONbits.TXREQ3 == 0);

        case 4:
            return (C1TR45CONbits.TXREQ4 == 0);

        case 5:
            return (C1TR45CONbits.TXREQ5 == 0);

        case 6:
            return (C1TR67CONbits.TXREQ6 == 0);

        case 7:
            return (C1TR67CONbits.TXREQ7 == 0);

        default:
            return false;

    }

}

bool Dspic33CanDriver_is_can_tx_buffer_clear(void)
{

    return _is_can_tx_buffer_clear(TX_CHANNEL_CAN_CONTROL);

}

static bool _transmit_can_frame(uint8_t channel, can_msg_t *msg)
{

    if (_is_can_tx_buffer_clear(channel))
    {

#ifndef DEBUG

        _ecan1_write_tx_msg_buf_id(channel, msg->identifier, 1, 0);
        _ecan1_write_tx_msg_buf_data(channel, msg->payload_count, &msg->payload);
        _ecan1_tx_buffer_set_transmit(channel);

#endif

        return true;

    }

    return false;

}

bool Dspic33CanDriver_transmit_can_frame(can_msg_t *msg)
{

    return _transmit_can_frame(TX_CHANNEL_CAN_CONTROL, msg);

}

void Dspic33CanDriver_pause_can_rx(void)
{

    C1INTEbits.RBIE = 0;

}

void Dspic33CanDriver_resume_can_rx(void)
{

    C1INTEbits.RBIE = 1;

}

void Dspic33CanDriver_initialize(void)
{

    /* Request Configuration Mode */
    C1CTRL1bits.REQOP = 4;
    while (C1CTRL1bits.OPMODE != 4)
        ;

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

    C1FCTRLbits.FSA = 0b01000; // FIFO Start Area: RX FIFO Starts at Message Buffer 8 (0-7 are TX)
    C1FCTRLbits.DMABS = 0b111; // 32 CAN Message Buffers in DMA RAM, 8 TX (0-7) and 24 RX (8-31)

    // Filter 0, Filter SID/EID = 0x00000000, Extended Message = 1, FIFO, No Mask
    _ecan1_write_rx_acpt_filter(0, 0x00000000, 1, 0b1111, 0);

    /* Enter Normal Mode */
    C1CTRL1bits.REQOP = 0;
    while (C1CTRL1bits.OPMODE != 0)
        ;

    /* ECAN transmit/receive message control */
    C1RXFUL1 = C1RXFUL2 = C1RXOVF1 = C1RXOVF2 = 0x0000;

    C1TR01CON = 0x8382; // Buffer 0 and 1 is TX, Highest Priority
    C1TR23CON = 0x8180; // Buffer 2 and 3 is TX, Higher Priority
    C1TR45CON = 0x8080; // Buffer 4 and 5 is TX, Low Priority
    C1TR67CON = 0x8080; // Buffer 6 and 7 is TX, Lowest Priority

    /* Enable ECAN1 Interrupts */
    IEC2bits.C1IE = 1;   // Enable CAN1 interrupts globally
    C1INTEbits.TBIE = 1; // Enable CAN1 TX
    C1INTEbits.RBIE = 1; // Enable CAN1 RX

    // DMA 2 Initialize (CAN RX)
    DMA2CON = 0x0020;
    DMA2PAD = (uint16_t)&C1RXD;
    DMA2CNT = 0x0007;
    DMA2REQ = 0x0022;

#ifdef _HAS_DMA_
    DMA2STAL = __builtin_dmaoffset(ecan1msgBuf);
    DMA2STAH = __builtin_dmapage(ecan1msgBuf);
#else
    DMA2STAL = (uint16_t)(int_least24_t)(&ecan1msgBuf);
    DMA2STAH = 0;
#endif

    DMA2CONbits.CHEN = 1;

    // DMA0 Initialize (CAN TX)
    DMA0CON = 0x2020;
    DMA0PAD = (uint16_t)&C1TXD;
    DMA0CNT = 0x0007;
    DMA0REQ = 0x0046;

#ifdef _HAS_DMA_
    DMA0STAL = __builtin_dmaoffset(ecan1msgBuf);
    DMA0STAH = __builtin_dmapage(ecan1msgBuf);
#else
    DMA0STAL = (uint16_t)(int_least24_t)(&ecan1msgBuf);
    DMA0STAH = 0;
#endif

    DMA0CONbits.CHEN = 1;

}

void Dspic33CanDriver_c1_interrupt_handler(void)
{

    /* clear interrupt flag */
    IFS2bits.C1IF = 0;

    if (C1INTFbits.RBIF)
    { // RX Interrupt

        uint8_t buffer_tail = _FNRB;
        uint8_t buffer_head = _FBP;

        // Reset the interrupt so anything that comes in from here on will re-trigger
        C1INTFbits.RBIF = 0;

        uint8_t fifo_size = 0;
        uint16_t ide = 0;
        can_msg_t ecan_msg;

        while (buffer_tail != buffer_head)
        {

            _ecan1_read_rx_msg_buf_id(buffer_tail, &ecan_msg, &ide);
            _ecan1_read_rx_msg_buf_data(buffer_tail, &ecan_msg);

            if (ide)
            {

                CanRxStatemachine_incoming_can_driver_callback(&ecan_msg);

            }

            // Clear Full/OV flags atomically (see errata)
            if (buffer_tail < 16)
            {

                C1RXFUL1 = FIFO_FLAG_MASKS[buffer_tail];
                C1RXOVF1 = FIFO_FLAG_MASKS[buffer_tail];

            }
            else
            {

                C1RXFUL2 = FIFO_FLAG_MASKS[buffer_tail - 16];
                C1RXOVF2 = FIFO_FLAG_MASKS[buffer_tail - 16];

            }

            buffer_tail = buffer_tail + 1;
            if (buffer_tail > MAX_CAN_FIFO_BUFFER)
                buffer_tail = MIN_CAN_FIFO_BUFFER;

            fifo_size = fifo_size + 1;

            if (fifo_size > _max_can_fifo_depth)
                _max_can_fifo_depth = fifo_size;

        };

    }
    else
    { // TX Interrupt

        if (C1INTFbits.TBIF)
        {

            C1INTFbits.TBIF = 0;

        }

    }

}

// CAN 1 Interrupt
void __attribute__((interrupt(no_auto_psv))) _C1Interrupt(void)
{

    // Allows a bootloader to call the normal function from its interrupt
    Dspic33CanDriver_c1_interrupt_handler();

}

uint8_t Dspic33CanDriver_get_max_can_fifo_depth(void)
{

    return _max_can_fifo_depth;

}
