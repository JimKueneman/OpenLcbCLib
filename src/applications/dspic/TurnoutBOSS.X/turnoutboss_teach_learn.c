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


teach_learn_state_t TurnoutBossTeachLearn_teach_learn_state;

void TurnoutBossTeachLearn_initialize(void) {

    memset(&TurnoutBossTeachLearn_teach_learn_state, 0x00, sizeof (TurnoutBossTeachLearn_teach_learn_state));


}

void TurnoutBossTeachLearn_update_leds(uint8_olcb_t teach_learn_state) {

    switch (teach_learn_state) {

        case STATE_TEACH_LEARN_SET_DEFAULTS_START:
        case STATE_TEACH_LEARN_SET_WAIT_FOR_TEACH:

            LED_BLUE = TRUE;
            LED_YELLOW = TRUE;
            LED_GREEN = !LED_GREEN;

            break;

        case STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE:
        case STATE_TEACH_LEARN_SET_DEFAULTS_DONE:

            LED_BLUE = FALSE;
            LED_YELLOW = FALSE;
            LED_GREEN = TRUE;

            break;

    }


}

void TurnoutBossTeachLearn_run(openlcb_node_t *openlcb_node) {

    switch (TurnoutBossTeachLearn_teach_learn_state.state) {

        case STATE_TEACH_LEARN_SET_DEFAULTS_START:

            // Wait to release the Teach Button
            if (!TEACH_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_WAIT_FOR_TEACH;

            }

            return;

        case STATE_TEACH_LEARN_SET_WAIT_FOR_TEACH:

            // Wait to press the Teach Button
            if (TEACH_BUTTON_PIN) {

                TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE;

            }

            return;

        case STATE_TEACH_LEARN_SET_DEFAULTS_EXECUTE:

            printf("Reseting to Factory Defaults....\n");

            TurnoutBossBoardConfiguration_reset_to_defaults(openlcb_node);

            TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_DONE;

            return;

        case STATE_TEACH_LEARN_SET_DEFAULTS_DONE:

            return;

    }

}

void TurnoutBossTeachLearn_check_for_enable(void) {

    if (LEARN_BUTTON_PIN && TEACH_BUTTON_PIN) {

        printf("Entering Learn mode: Both Buttons Pressed\n");

        TurnoutBossTeachLearn_teach_learn_state.state = STATE_TEACH_LEARN_SET_DEFAULTS_START;

    } else

        if (LEARN_BUTTON_PIN) {

        //     TurnoutBossTeachLearn_teach_learn_state.teach_learn_mode = TRUE;

        printf("Entering Learn mode: Learn Button Pressed\n");

    } else

        if (TEACH_BUTTON_PIN) {

        //    TurnoutBossTeachLearn_teach_learn_state.teach_learn_mode = TRUE;

        printf("Entering Learn mode: Teach Button Pressed\n");

    }

}


