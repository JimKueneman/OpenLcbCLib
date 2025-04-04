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
 * \file turnoutboss_teach_learn.c
 *
 * TurnoutBOSS main
 *
 * @author Jim Kueneman
 * @date 1 Mar 2025
 */

#include "turnoutboss_teach_learn.h"

#include "stdio.h"  // printf
#include "string.h"
#include "stdlib.h"

#include "turnoutboss_teach_learn.h"
#include "../TurnoutBossCommon/common_loader_app.h"
#include "turnoutboss_types.h"
#include "turnoutboss_board_configuration.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "turnoutboss_drivers.h"


teach_learn_state_t TurnoutBossTeachLearn_teach_learn_state;

#define LED_ARRAY_LEN 12
#define LED_GREEN_ARRAY_LEN 16

const uint8_olcb_t _led_blue_array_one_blink[LED_ARRAY_LEN] =   {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_olcb_t _led_blue_array_two_blink[LED_ARRAY_LEN] =   {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_olcb_t _led_blue_array_three_blink[LED_ARRAY_LEN] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0};
const uint8_olcb_t _led_blue_array_four_blink[LED_ARRAY_LEN] =  {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};
const uint8_olcb_t _led_blue_array_five_blink[LED_ARRAY_LEN] =  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0};

const uint8_olcb_t _led_green_array_blink[LED_GREEN_ARRAY_LEN] = {1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0};

void _event_learn_callback(openlcb_node_t* openlcb_node, event_id_t* event_id) {
    
    configuration_memory_buffer_t buffer;

    switch (TurnoutBossTeachLearn_teach_learn_state.state) {

        case STATE_TEACH_LEARN_START_LEARN_LH_BOARD:
            
            Utilities_copy_event_id_to_config_mem_buffer(&buffer, *event_id, 0);
            
            if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_ADJACENT_LEFT, 8, &buffer) == 8) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            break;

        case STATE_TEACH_LEARN_START_LEARN_RH_BOARD:
            
            Utilities_copy_event_id_to_config_mem_buffer(&buffer, *event_id, 0);
            
            if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_ADJACENT_RIGHT, 8, &buffer) == 8) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }


            break;

    }

}

void TurnoutBossTeachLearn_initialize(void) {

    memset(&TurnoutBossTeachLearn_teach_learn_state, 0x00, sizeof (TurnoutBossTeachLearn_teach_learn_state));

    ApplicationCallbacks_set_event_learn(&_event_learn_callback);

}

void _inc_led_array_indexer(void) {

    TurnoutBossTeachLearn_teach_learn_state.led_array_index = TurnoutBossTeachLearn_teach_learn_state.led_array_index + 1;

    if (TurnoutBossTeachLearn_teach_learn_state.led_array_index >= LED_ARRAY_LEN) {

        TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;

    }

    TurnoutBossTeachLearn_teach_learn_state.led_green_array_index = TurnoutBossTeachLearn_teach_learn_state.led_green_array_index + 1;

    if (TurnoutBossTeachLearn_teach_learn_state.led_green_array_index >= LED_GREEN_ARRAY_LEN) {

        TurnoutBossTeachLearn_teach_learn_state.led_green_array_index = 0;

    }

}

void TurnoutBossTeachLearn_update_leds(uint8_olcb_t teach_learn_state) {

    switch (teach_learn_state) {

        case STATE_TEACH_LEARN_SET_DEFAULTS_START:
        case STATE_TEACH_LEARN_SET_DEFAULTS_WAIT_FOR_TEACH:

            LED_BLUE = TRUE;
            LED_YELLOW = TRUE;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_START_LEARN:
        case STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START:
        case STATE_TEACH_LEARN_START_LEARN_SIGNALS:
        case STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START_SIGNALS:

            LED_BLUE = FALSE;
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_START_LEARN_LH_BOARD:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_LH_BOARD_START:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_ANODE_START:
        case STATE_TEACH_LEARN_START_SIGNALS_COMMON_ANODE:
      
            LED_BLUE = _led_blue_array_one_blink[TurnoutBossTeachLearn_teach_learn_state.led_array_index];
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_START_LEARN_RH_BOARD:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_RH_BOARD_START:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_CATHODE_START:
        case STATE_TEACH_LEARN_START_SIGNALS_COMMON_CATHODE:
   
            LED_BLUE = _led_blue_array_two_blink[TurnoutBossTeachLearn_teach_learn_state.led_array_index];
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_START_LEARN_BOARD_LH:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_LH_START:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_BI_DIRECTIONAL_START:
        case STATE_TEACH_LEARN_START_SIGNALS_BI_DIRECTIONAL:

            LED_BLUE = _led_blue_array_three_blink[TurnoutBossTeachLearn_teach_learn_state.led_array_index];
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_START_LEARN_BOARD_RH:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_RH_START:
        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_TWO_START:
        case STATE_TEACH_LEARN_START_TURNOUT_CTRL_TWO:

            LED_BLUE = _led_blue_array_four_blink[TurnoutBossTeachLearn_teach_learn_state.led_array_index];
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_ONE_START:
        case STATE_TEACH_LEARN_START_TURNOUT_CTRL_ONE:

            LED_BLUE = _led_blue_array_five_blink[TurnoutBossTeachLearn_teach_learn_state.led_array_index];
            LED_YELLOW = TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence;
            LED_GREEN = _led_green_array_blink[TurnoutBossTeachLearn_teach_learn_state.led_green_array_index];

            break;


        case STATE_TEACH_LEARN_ACTION_DONE:

            LED_BLUE = FALSE;
            LED_YELLOW = FALSE;
            LED_GREEN = TRUE;

            break;


    }

    _inc_led_array_indexer();

    TurnoutBossTeachLearn_teach_learn_state.timer_tick = TurnoutBossTeachLearn_teach_learn_state.timer_tick + 1;

}

void TurnoutBossTeachLearn_run(openlcb_node_t *node) {

    configuration_memory_buffer_t buffer;

    switch (TurnoutBossTeachLearn_teach_learn_state.state) {

            // Jumps in here if starting a Reset to Defaults sequence

        case STATE_TEACH_LEARN_SET_DEFAULTS_START:

            // Wait to release the Teach Button
            if (!TEACH_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_WAIT_FOR_TEACH;

            }

            return;

        case STATE_TEACH_LEARN_SET_DEFAULTS_WAIT_FOR_TEACH:

            // Wait to press the Teach Button
            if (TEACH_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE;

            }

            return;

        case STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE:

            printf("Reseting to Factory Defaults....\n");

            TurnoutBossBoardConfiguration_reset_to_defaults(node);

            TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

            return;


            // Jumps in here if starting a Board Configuration Sequence

        case STATE_TEACH_LEARN_START_LEARN:

            TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence = FALSE;

            // Wait to release the Teach Button
            if (!LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START;

            }

            return;

        case STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START:

            if (LEARN_BUTTON_PIN) {

                printf("Waiting for LH Board to Teach\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_LH_BOARD_START;

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_LH_BOARD_START:

            if (!LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN_LH_BOARD;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_LEARN_LH_BOARD:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_RH_BOARD_START;

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_RH_BOARD_START:

            if (!LEARN_BUTTON_PIN) {
                
                printf("Waiting for RH Board to Teach\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN_RH_BOARD;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_LEARN_RH_BOARD:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_LH_START;

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_LH_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Waiting for TurnoutBoss Board (Left) to Teach\n");
                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN_BOARD_LH;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_LEARN_BOARD_LH:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_RH_START;

            }

            if (TEACH_BUTTON_PIN) {

                printf("This board is now a LH Defined TurnoutBoss\n");
                printf("Teach/Learn sequence done, reboot node\n");

                buffer[0] = BL;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_LOCATION, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_BOARD_RH_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Waiting for TurnoutBoss Board (Right) to Teach\n");
                
                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN_BOARD_RH;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_LEARN_BOARD_RH:

            if (TEACH_BUTTON_PIN) {

                printf("This board is now a RH Defined TurnoutBoss\n");
                printf("Teach/Learn sequence done, reboot node\n");

                buffer[0] = BR;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_LOCATION, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }

            return;


            // Jumps in here if starting a Signal Sequence

        case STATE_TEACH_LEARN_START_LEARN_SIGNALS:

            TurnoutBossTeachLearn_teach_learn_state.is_signal_sequence = TRUE;

            // Wait to release the Teach Button
            if (!LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START_SIGNALS;

            }

            return;

        case STATE_TEACH_LEARN_START_WAIT_FOR_LEARN_START_SIGNALS:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_ANODE_START;

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_ANODE_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Teaching Common Anode Signals\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_SIGNALS_COMMON_ANODE;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_SIGNALS_COMMON_ANODE:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_CATHODE_START;

            }

            if (TEACH_BUTTON_PIN) {

                // Write Common Anode
                buffer[0] = LED_POLARITY_COMMON_ANODE;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_LED_POLARITY, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }


            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_COMMON_CATHODE_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Teaching Common Cathode Signals\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_SIGNALS_COMMON_CATHODE;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;


        case STATE_TEACH_LEARN_START_SIGNALS_COMMON_CATHODE:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_BI_DIRECTIONAL_START;

            }

            if (TEACH_BUTTON_PIN) {

                // Write Common Cathode
                buffer[0] = LED_POLARITY_COMMON_CATHODE;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_LED_POLARITY, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }

            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_SIGNALS_BI_DIRECTIONAL_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Teaching Bi-Directional Signals\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_SIGNALS_BI_DIRECTIONAL;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_SIGNALS_BI_DIRECTIONAL:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_TWO_START;

            }

            if (TEACH_BUTTON_PIN) {

                // Write Bi-Directional
                buffer[0] = LED_POLARITY_BI_DIRECTIONAL;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_LED_POLARITY, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }


            return;


        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_TWO_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Teaching Turnout Control Two Button\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_TURNOUT_CTRL_TWO;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;


        case STATE_TEACH_LEARN_START_TURNOUT_CTRL_TWO:

            if (LEARN_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_ONE_START;

            }

            if (TEACH_BUTTON_PIN) {

                // Write Double Head Mast
                buffer[0] = TURNOUT_CONTROL_TWO_BUTTON;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_PUSHBUTTON_TYPE, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }


            return;

        case STATE_TEACH_LEARN_WAIT_FOR_BUTTON_UP_TURNOUT_CTRL_ONE_START:

            if (!LEARN_BUTTON_PIN) {

                printf("Teaching Points Single Head Signals\n");

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_TURNOUT_CTRL_ONE;
                TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;  // restart the blue blink

            }

            return;

        case STATE_TEACH_LEARN_START_TURNOUT_CTRL_ONE:

            if (TEACH_BUTTON_PIN) {

                // Write Single Head Mast
                buffer[0] = TURNOUT_CONTROL_ONE_BUTTON;
                if (TurnoutBossDrivers_config_mem_write(CONFIG_MEM_ADDRESS_BOARD_PUSHBUTTON_TYPE, 1, &buffer) == 1) {

                    TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_ACTION_DONE;

                }

            }

            return;


        case STATE_TEACH_LEARN_ACTION_DONE:

            return;


    }

}

void TurnoutBossTeachLearn_check_for_enable(void) {

    TurnoutBossTeachLearn_teach_learn_state.timer_tick = 0;

    if (LEARN_BUTTON_PIN && TEACH_BUTTON_PIN) {

        printf("Entering Learn mode: Both Buttons Pressed\n");

        TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_START;

    } else if (LEARN_BUTTON_PIN) {

        TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN;

        while (LEARN_BUTTON_PIN) {

            if (TurnoutBossTeachLearn_teach_learn_state.timer_tick > 30) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_START_LEARN_SIGNALS;

                printf("Entering Learn mode: Learning Signal Information\n");

                break;

            }
        }

        printf("Entering Learn mode: Learning Board Information\n");

    } else

        if (TEACH_BUTTON_PIN) {

        //    TurnoutBossTeachLearn_teach_learn_state.teach_learn_mode = TRUE;

        printf("Entering Learn mode: Teach Button Pressed\n");

    }

    TurnoutBossTeachLearn_teach_learn_state.led_array_index = 0;
    TurnoutBossTeachLearn_teach_learn_state.led_green_array_index = 0;

}


