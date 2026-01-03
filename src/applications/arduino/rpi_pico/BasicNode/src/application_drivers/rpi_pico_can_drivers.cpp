#include <stdbool.h>
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
 * \file rpi_pico_can_drivers.cpp
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 31 Dec 2025
 */

#define ARDUINO_COMPATIBLE

#include "rpi_pico_can_drivers.h"

#include "../drivers/canbus/can_rx_statemachine.h"
#include "../drivers/canbus/can_types.h"
#include "../openlcb/openlcb_gridconnect.h"
#include "../utilities/mustangpeak_string_helper.h"

#ifdef ARDUINO_COMPATIBLE
// include any header files the Raspberry Pi Pico need to compile under Arduino/PlatformIO
#include "Arduino.h"
#include <ACAN2517.h>
#include <SPI.h>
#endif

// Uncomment to enable logging
//#define LOG_SETUP
//#define LOG_SUCCESSFUL_SETUP_PARAMETERS

static const byte MCP2517_CS = 17;   // CS input of MCP2517
static const byte MCP2517_INT = 20;  // INT output of MCP2517

// Create a ACAN2517 Object
static ACAN2517 can(MCP2517_CS, SPI, MCP2517_INT);

void RPiPicoCanDriver_setup(void) {

  // Initialize Raspberry Pi Pico CAN features

  SPI.begin(true);
  // Setup for 125kHz
  ACAN2517Settings settings(ACAN2517Settings::OSC_40MHz, 125UL * 1000UL);

  const uint16_t errorCode = can.begin(settings, [] {
    can.isr();
  });
#ifdef LOG_SETUP  // Uncomment the define above to enable
  Serial.print("\nerrorCode=");
  Serial.println(errorCode);
  if (errorCode == 0) {
#ifdef LOG_SUCCESSFUL_SETUP_PARAMETERS  // Uncomment the define above to enable
    Serial.print("\nBit Rate prescaler: ");
    Serial.println(settings.mBitRatePrescaler);
    Serial.print("Phase segment 1: ");
    Serial.println(settings.mPhaseSegment1);
    Serial.print("Phase segment 2: ");
    Serial.println(settings.mPhaseSegment2);
    Serial.print("SJW: ");
    Serial.println(settings.mSJW);
    Serial.print("Actual bit rate: ");
    Serial.print(settings.actualBitRate());
    Serial.println(" bit/s");
    Serial.print("Exact bit rate ? ");
    Serial.println(settings.exactBitRate() ? "yes" : "no");
    Serial.print("Sample point: ");
    Serial.print(settings.samplePointFromBitStart());
    Serial.println("%");
#endif
  } else {
    Serial.print("\n\nACAN ERROR: Configuration error 0x");
    Serial.println(errorCode, HEX);
  }
#endif
}

void RPiPicoCanDriver_process_receive(void) {

  can_msg_t can_msg;
  CANMessage frame;

  if (can.available()) {

    printf("CAN Rx Availible\n");

    if (can.receive(frame)) {

      printf("CAN Rx Received\n");

      if (frame.ext) {  // Only Extended messages

        printf("CAN Extended Frame\n");

        can_msg.state.allocated = true;
        can_msg.payload_count = frame.len;
        can_msg.identifier = frame.id;

        for (int i = 0; i < frame.len; i++) {

          can_msg.payload[i] = frame.data[i];
        }

        CanRxStatemachine_incoming_can_driver_callback(&can_msg);
      }
    }
  }
}

bool RPiPicoCanDriver_is_can_tx_buffer_clear(void) {

  // ACAN Library does not have a method to know if the buffer is full, I believe OpenLcbCLib will 
  // function correctly if the Transmit fails as well.  This was just for a short cut if available.

  return true;
}

bool RPiPicoCanDriver_transmit_raw_can_frame(can_msg_t *msg) {

  CANMessage frame;

  frame.ext = true;
  frame.id = msg->identifier;
  frame.len = msg->payload_count;
  for (int i = 0; i < frame.len; i++) {

    frame.data[i] = msg->payload[i];
  }

  return can.tryToSend(frame);
}

void RPiPicoCanDriver_pause_can_rx(void) {

  // Not required as the ACAN2517 library uses and interrupt to get the next message in the background and we access it from the mainloop
}

void RPiPicoCanDriver_resume_can_rx(void) {

  // Not required as the ACAN2517 library uses and interrupt to get the next message in the background and we access it from the mainloop
}
