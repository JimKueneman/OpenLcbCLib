
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
 * \odx_drivers.c
 *
 *
 *
 * @author Jim Kueneman
 * @date 7 Nov 2025
 */

#include "esp32_drivers.h"
#include "esp32_can_drivers.h"

#include "src/openlcb/openlcb_types.h"
#include "src/utilities/mustangpeak_string_helper.h"
#include "src/openlcb/openlcb_node.h"
#include "src/openlcb/protocol_datagram_handler.h"

#include "Arduino.h"
#include "esp_pm.h"
#include "esp32-hal-cpu.h"
#include "esp_system.h"
#include "esp32-hal-timer.h"

bool _is_100ms_timer_running = false;

hw_timer_t *Timer0_Cfg = NULL;

void IRAM_ATTR Timer0_ISR() {
  _is_100ms_timer_running = true;

  OpenLcbNode_100ms_timer_tick();
  ProtocolDatagramHandler_100ms_timer_tick();
}

bool Esp32Drivers_100ms_running() {

  return _is_100ms_timer_running;
}

void Esp32Drivers_setup(void) {

  // Speed kills!
  setCpuFrequencyMhz(240);

  // 100 ms Timer
  // APB_CLK = Advanced Periphial Bus Clock and is typically 80Mhz
  // true = count up
  // Tout = TimerTicks * Prescaler/APB_CLK
  // TimerTicks = Tout * APB_CLK/Prescaler
  // TimerTicks = 100ms * 80Mhz/80
#ifdef PLATFORMIO
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, false);  // level trigged (not edge as the example was true but that is not supported here)
  timerAlarmWrite(Timer0_Cfg, 100000, true);             // Alarm (call ISR) when the timer hit the passed value then auto reload and restart it
  timerAlarmEnable(Timer0_Cfg);                          // kick it off.
#else
  Timer0_Cfg = timerBegin(1000000);               // 1Mhz
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR);  // level trigged (not edge as the example was true but that is not supported here)
  timerAlarm(Timer0_Cfg, 20000, true, 0);         // No idea why this gives 100ms but it does... poor documentation on these newer functions
#endif
}

void Esp32Drivers_reboot(openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info) {
  
  esp_restart();

}

char user_name[11] = "ESP32 Node";

uint16_t Esp32Drivers_config_mem_read(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // Todo

  for (int i = 0; i < count; i++)
    (*buffer)[i] = 0x00;

  return count;
}

uint16_t Esp32Drivers_config_mem_write(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer) {

  // Todo

  return count;
}

void Esp32Drivers_lock_shared_resources(void) {
#ifdef PLATFORMIO
  timerAlarmDisable(Timer0_Cfg);
#else
  timerStop(Timer0_Cfg);
#endif

  Esp32CanDriver_pause_can_rx();
}

void Esp32Drivers_unlock_shared_resources(void) {
#ifdef PLATFORMIO
  timerAlarmEnable(Timer0_Cfg);
#else
  timerStart(Timer0_Cfg);
#endif

  Esp32CanDriver_resume_can_rx();
}