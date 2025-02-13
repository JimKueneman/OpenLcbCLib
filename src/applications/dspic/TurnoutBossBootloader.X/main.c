/** \copyright
 * Copyright (c) 2024, Jim Kueneman
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
 * \file main.c
 *
 * TurnoutBoss Bootloader main
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */


// DSPIC33EP512GP504 Configuration Bit Settings

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


#include <libpic30.h>

#include "xc.h"
#include <stdio.h>  // printf
#include <string.h>
#include <stdlib.h>

#include "../../../drivers/common/can_main_statemachine.h"
#include "../../../drivers/common/../driver_mcu.h"
#include "../../../drivers/driver_can.h"
#include "../../../openlcb/openlcb_main_statemachine.h"
#include "../../../openlcb/openlcb_node.h"
#include "../../../openlcb/application_callbacks.h"
#include "../../../openlcb/application.h"
#include "../../../openlcb/openlcb_utilities.h"
#include "ecan1_bootloader_helper.h"
#include "turnoutboss_drivers.h"
#include "debug.h"
#include "uart_handler.h"
#include "traps.h"
#include "mcc_generated_files/memory/flash.h"

#include "node_parameters.h"

// Where the erase page begins if you have to reflash the configuration bits, used to 
// NOT flash anything past this address

// Bootloader program code needs to be limited to not go past this in the linker file, example
// program (xr)   : ORIGIN = 0x200,         LENGTH = 0xAFFF
#define FLASH_ERASE_PAGE_START_APPLICATION_BITS_DSPIC33EP512GP504 0x00000B000

// Bootloader won't write any addresses past this address as to not destroy the configuration bits
// Note Bootloader AND Application MUST HAVE THE SAME CONFIGURATION BITS
#define FLASH_ERASE_PAGE_START_CONFIGURATION_BITS_DSPIC33EP512GP504 0x00055800




#define FLASH_ERASE_PAGE_START_APPLICATION_BITS FLASH_ERASE_PAGE_START_APPLICATION_BITS_DSPIC33EP512GP504
#define FLASH_ERASE_PAGE_START_CONFIGURATION_BITS FLASH_ERASE_PAGE_START_CONFIGURATION_BITS_DSPIC33EP512GP504


#define HEX_STATE_FIND_COLON 0

#define HEX_STATE_READ_ADDRESS_CHAR_0 1
#define HEX_STATE_READ_ADDRESS_CHAR_1 2
#define HEX_STATE_READ_ADDRESS_CHAR_2 3
#define HEX_STATE_READ_ADDRESS_CHAR_3 4
#define HEX_STATE_READ_ADDRESS_CHAR_4 5
#define HEX_STATE_READ_ADDRESS_CHAR_5 6
#define HEX_STATE_READ_ADDRESS_CHAR_6 7
#define HEX_STATE_READ_ADDRESS_CHAR_FINAL 8

#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_0 9
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_1 10
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_2 11
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_3 12
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_4 13
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_5 14
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_6 15
#define HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_FINAL 16

#define HEX_STATE_READ_DATA_CHAR_0 17
#define HEX_STATE_READ_DATA_CHAR_1 18
#define HEX_STATE_READ_DATA_CHAR_2 19
#define HEX_STATE_READ_DATA_CHAR_3 20
#define HEX_STATE_READ_DATA_CHAR_4 21
#define HEX_STATE_READ_DATA_CHAR_5 22
#define HEX_STATE_READ_DATA_CHAR_6 23
#define HEX_STATE_READ_DATA_CHAR_FINAL 24



uint8_olcb_t state_machine_state = HEX_STATE_FIND_COLON;

char temp[16] = {};
uint8_olcb_t byte_index = 0;


uint32_olcb_t start_erase_block_address = 0;
uint32_olcb_t instruction_count = 0;
uint32_olcb_t data1 = 0;
uint32_olcb_t data2 = 0;
uint8_olcb_t is_data_1 = TRUE;
uint16_olcb_t running_instruction_count = 0;
uint32_olcb_t running_address = 0;

uint8_olcb_t _run_hex_file_state_machine(uint8_olcb_t next_char) {



    switch (state_machine_state) {

        case HEX_STATE_FIND_COLON:
        {

            if (next_char == ':') {

                memset(&temp, 0x00, sizeof ( temp));

                state_machine_state = state_machine_state + 1;

            }

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_ADDRESS_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }


        case HEX_STATE_READ_ADDRESS_CHAR_FINAL:
        {

            temp[7] = next_char;

            start_erase_block_address = strtol((char*) &temp, NULL, 16);

            // prepare for next
            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_INSTRUCTION_COUNT_CHAR_FINAL:
        {

            temp[7] = next_char;
            instruction_count = strtol((char*) &temp, NULL, 16);

            // prepare for next
            is_data_1 = TRUE;
            running_instruction_count = 0;
            running_address = start_erase_block_address;

            if ((start_erase_block_address >= FLASH_ERASE_PAGE_START_APPLICATION_BITS) && (start_erase_block_address < FLASH_ERASE_PAGE_START_CONFIGURATION_BITS))
            {
                FLASH_ErasePage(start_erase_block_address);
                
                FLASH_Unlock(FLASH_UNLOCK_KEY);
            }

            memset(&temp, 0x00, sizeof ( temp));
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_0:
        {

            temp[0] = next_char;
            state_machine_state = state_machine_state + 1;

            break;
        }

        case HEX_STATE_READ_DATA_CHAR_1:
        {

            temp[1] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_2:
        {

            temp[2] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_3:
        {

            temp[3] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_4:
        {

            temp[4] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_5:
        {

            temp[5] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_6:
        {

            temp[6] = next_char;
            state_machine_state = state_machine_state + 1;

            break;

        }

        case HEX_STATE_READ_DATA_CHAR_FINAL:
        {

            temp[7] = next_char;
            if (is_data_1) {

                data1 = strtol((char*) &temp, NULL, 16);

                running_instruction_count = running_instruction_count + 1;
                is_data_1 = FALSE; // ping pong

                // Need to find the second byte, the dsHEX format ensures we have a power of 2
                state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

            } else {

                data2 = strtol((char*) &temp, NULL, 16);

                running_instruction_count = running_instruction_count + 1;
                is_data_1 = TRUE; // ping pong

                FLASH_WriteDoubleWord24(running_address, data1, data2);

                if (FLASH_ReadWord24(running_address) != data1) {

                    printf("Error on Read-back data1\n");

                }

                if (FLASH_ReadWord24(running_address + 2) != data2) {

                    printf("Error on Read-back data2\n");

                }

                running_address = running_address + 4;

                if (running_instruction_count < instruction_count) {

                    state_machine_state = HEX_STATE_READ_DATA_CHAR_0;

                } else {

                    FLASH_Lock();

                    state_machine_state = HEX_STATE_FIND_COLON;

                }

            }

            // prepare for next
            memset(&temp, 0x00, sizeof ( temp));


            break;

        }
    }

    return FALSE;
}

uint16_olcb_t _config_mem_write(uint32_olcb_t address, uint16_olcb_t count, configuration_memory_buffer_t* buffer) {

    for (int i = 0; i < count; i++) {

        _run_hex_file_state_machine((*buffer)[i]);

    }

    return count;
}


uint64_olcb_t node_id_base = 0x0507010100AA;

void _alias_change_callback(uint16_olcb_t new_alias, uint64_olcb_t node_id) {

    printf("Alias Allocation: 0x%02X  ", new_alias);
    PrintNodeID(node_id);
    printf("\n");

}

int main(void) {


//    char* test = ":0000B000000004000004B016000000000000B14C0000B1500000B1540000B1580000B15C0000B1600000B1640000B1680000B16C002652EF0027FF0E0088010E000000000020000000E00000003200020020010000880220000700140007018000E00000003A0003002E37200020001100370002002000000020000100700F81003200010007000F0020000000E000000032000200020000000000000002DD860000000100DA400000FE00000026408000E0000000320002002020200088019000060000008802A10078048000EB00000037001F004484E200B4A05400BA0599004484E200B4A05400BA0699004484E200B4A05400EB060000DE6F4700B207FD008801AE00E16C63003A00030001D80A00EB00000037000E00E16C60003A000800EB4D0000E8050A0039000200EC203400A0F00A00E9058B003EFFF90037000400E168610032000100EB86000007000B00BAC71900E0040E00BA0519003A000300E0000A003AFFDB0006000000B207FE008802AE0078048A0037FFF500A3D00C004D0FE2003B001C0078028900BA4D3500E8050A0039000200EC203400A0F00A00E9058B0032000F00BA4D2500E8050A0039000200EC203400A0F00A00E9058B0032000800E0000C0032000600BACD1500E8050A0039000200EC203400A0F00A00E9058B00E8848900B4A05400E0000B003AFFE20006000000BA029900785D0500E9058B0032FFF700DE2AC800785D0500E9058B0032FFF300E0000C0032FFF100BADD190037FFEE0004AE52000000010004F1EA00000001000488280000000100048820000000010004872C00000001000487E40000000100048830000000010004E13C000000010004E134000000010004E108000000010004E0EC000000010004E0D8000000010004DF96000000010004A338000000010004DD58000000010004F27A000000010004F280000000010004F286000000010004F28C000000010004F292000000010004E168000000010004E14C000000010004E144000000010004892C000000010004E19C000000000004C890000000010004BEB0000000000004B954000000000004B45C0000000000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002892C0000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002F27A0000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002F2800000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002F2860000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002F28C0000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002F2920000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002E1440000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002E14C0000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000F8003600BE9F8000BE9F8200BE9F8400BE9F8600FA00000002E1680000000100FA800000BE034F00BE024F00BE014F00BE004F00F900360006400000DA400000FE000000FA000A00200011009FBFB1009FBFC00057806A00070A580097B84F00FA80000006000000FA000A00500FE2003E003400808FD000500FE10032000300500FE2003A00110037000C0020220300E00003003A000100DA400000E202200035000200EF222800A8E22100AB4223003A000D00A842230037000B0020230300E00003003A000100DA400000E202300035000200EF223800A8E23100AB4233003A000100A84233007801820037000B00FB023100500FE1003A000400AB2223003AFFFE008811240037000300AB2233003AFFFE008811A400E9018300E00003003AFFF300500FE1003A000300AB02230032FFFE0037000B00AB02330032FFFE0037000800200033009FBFB3009FBFC0009FBFD1009FBFE20057806A00070A190097B96F0078000200FA8000000600000005000000FA0000005781680097B8CF0083251000780010000702E800FA800000060000000700F90007015400070209000702330007018A000703670037030E00FA0000005781680097B8CF000702E900FA800000060000000700EC00070147000701FC000702260007017D0007035A003703010078018000370002007859B100E9010200E00002003AFFFC0006000000BE9F8800BE9F8A0078048000E000020032005B0078400100784C8100448182009FF9F100510FE20036005500984491009844A1009FF9E1009FF9D100510FE60036004F009844B1009FF9C100510FE80036004B00EA0509006505630044840A0051050A002FFFC10065050100FB800000EB0080002010120020101300070A0700BE8C000044010A009FB960009FB97100550FE80036003A00980420009804310098044000980451009FB920009FB931009FB940009FB95100550FF800360030009804600098047100980C0000980C1100980C2000980C3100980C4000980C51009FB120009FB131009FB140009FB151009FB160009FB171009FB900009FB9110064016400410178004404020055050200B8116000BE030200BE020000BE030400B82260007200000072808100730102007381830037001000BE9C0000BE94020098044000980451009804620098047300980C0000980C1100980C2200980C3300980C4000980C5100980C6200980C7300B1020A00B0020800550FFF003EFFEE0078000900BE054F00BE044F000600000083251100780091003708010083251100780091000705900020001100E00000003500050083251000780090002000A0000707F700DE00CF00EA0001000600000006000000264A80000600000007FFFD008B2530008B25200006000000BE9F8800BE0400007800080007000D0044000000780089000700030078000800BE044F0006000000781F8800780400007800080007089D007800080078044F00060000007800800037000400E00431003A000200510000000600000078010100A30801003AFFF90078018100780131002FEFF40042020200EA81020062010200280804006101040032FFF7007800830078010100E00431003AFFFD005100000006000000FA004400BE9F8800781F8A00BE040000780502009FBF94009FBFA5009FBFB6009FBFC700EB0000009F9FC0009FAFA8009F9FD800EB8000009FAFB000B8226000B83360002FFB600040000F000707B10097BA1F0097BAAF0097BB3F0097BBCF002000120078008A002FFB600040000F0007056F00E00009003200060097B23F0097AAAF0042820400979ADF0052020500440C840078054F00BE044F00FA800000060000002FFFF4002FFFF500BE03040037FFD300B8226000200006002800070037FFCF002FFFF4002FFFF500B833600037FFCB002000040028000500B833600037FFC70037FFF30037FFEE004787EC00BE9F8800BE9F8A00BE9F8C00781F8E0078060000780401009007180090052800780698009FB7CD009005B8007802120090029200900322009003B200BE000400BE010600DE184F00B8116000EB0080009FB7D0009FB7E1009FB7F2009FBF830078048000E000090032000600B8006000B811600050020400588285005903060059838700E0000E0035000200A1100D009FB7CD00E00009003A00050097B04F00A32800003A000200A338000032000200E9008A003700010078008A005781F8002000A2007800080007008B00E00009003A00030097B0CF00A328010032000800E900000045810000B3C2B100E000090032000100B3C2D1007849010037000600A338010032000400E900000045808000B3C202007848820090014800E9010200510100004580800097B24F0078018A0078000C0007026F0090009C00A38801003200040090009800500F81003D0001007800010078074F00BE064F00BE054F00BE044F00B100CF0006000000FA000200BE9F8800BE9F8A007805800078040100780002009001180090052800780098009FBFB1009004B800E000020035000200A11001009FBFB1005781EA002000820078008A007802100090029000900320009003B0007800080007004B0097B93F006100F00032000400E900000044808000B3C302007848820097B93F006100E80032000400E900000044808000B3C202007848820090014800E9010200510100004480800097BA3F0078018A0078000B000702340090009B00A38801003200040090009800500F81003D00010078000100BE054F00BE044F00FA800000060000004787EA00BE9F8800BE9F8A00781F8E0078040000780003005787720026E2840026C75500BE9F04002296C300780F030057876C00EB4F0000900511009004A10078059100E000020032000200780702003700010057877200E00000003A00020078000E0007FEFE00E0000A0035000300500F8A003600010078000A0078020B00780189007801000078008E00780008000702040078074F00BE054F00BE044F00B100AF00060000004787F400BE9F8800BE9F8A00BE9F8C00781F8E00BE050600BE0404009FBF81009FB782009FB7F3009000B0009FB7E100780113009FB792009006900090004000E900000078060000E0000D003D00010020001D0097B1EF0041800000EB48000020001E0037001F00E9060C0097B00F00DE80CF00B90961009FB7A0009FB7B1009FB7C2009FB7D300BE030200BE020000BE000800BE010A0007090A007807000097B0EF0040800C00B3C3030047480300E9068D0097B80F00E90000009FBF800097B22F0097B2BF0097B34F0097B3DF00BE000800BE010A000708F300BE050200BE040000E0000C0034000D00540FE0005C8FE0005D0FE0005D8FE0003AFFDA00E0000D003CFFD80097B88F00E00001003400030097B11F00A31802003AFFD200E0000E003A00020097B1FF00A140130078000C0078074F00BE064F00BE054F00BE044F00B1014F0006000000FA000200BE9F8800BE9F8A00780500007804010078000200900118009004A800780098009FBFB1009005B800E000020035000200A11001009FBFB1005781EA002000A200780089007802100090029000900320009003B0007800080007FF950090014800E9010200510100004580800097BA3F007801890078000A0007018C0090009A00A38801003200040090009800500F81003D00010078000100BE054F00BE044F00FA800000060000004787FA00BE9F8800BE9F8A00BE9F8C00781F8E007804800078040100780582009001180090052800780718009000B800E000020035000200A1100E003700010020001200670070009FB7E00032000B0078021B0090029B0090032B009003BB00520FE0005A8FE0005B0FE0005B8FE00032000200E9868A003700010078068A006702E8009FB7F50032000100E9068D0078021B0090029B0090032B009003BB009FAFE4009FAFF5";
//
//    for (int i = 0; i < strlen(test); i++)
//        _run_hex_file_state_machine(test[i]);
//
//    test = ":10AFE000CFFF0000FFFF00007FFF00009EFF00007A";
//
//    for (int i = 0; i < strlen(test); i++)
//        _run_hex_file_state_machine(test[i]);

    // RB7 and RB8 are test outputs
    // we also have the LED variable for RB9 and the LED output
    _TRISB7 = 0;
    _RB7 = 0;
    _TRISB8 = 0;
    _RB8 = 0;
    
    TurnoutBossDrivers_app_running = FALSE;


    CanMainStatemachine_initialize(
            &Ecan1Helper_setup,
            &Ecan1Helper_transmit_raw_can_frame,
            &Ecan1Helper_is_can_tx_buffer_clear,
            &Ecan1Helper_pause_can_rx,
            &Ecan1Helper_resume_can_rx
            );
    MainStatemachine_initialize(
            &TurnoutBossDrivers_setup,
            &TurnoutBossDrivers_reboot,
            &TurnoutBossDrivers_config_mem_read,
            &_config_mem_write,
            &TurnoutBossDrivers_pause_100ms_timer,
            &TurnoutBossDrivers_resume_100ms_timer
            );

    TurnoutBossDrivers_assign_uart_rx_callback(&UartHandler_handle_rx);

    Application_Callbacks_set_alias_change(&_alias_change_callback);

    printf("\nBooted\n");
    openlcb_node_t * node = Node_allocate(node_id_base, &NodeParameters_main_node);
    printf("Node Created\n");
    
    
    uint16_t applicationISRAddress = __builtin_tblrdl(0xB00E);
    void (*app_t2_interrupt_func)() = (void*) applicationISRAddress;   
    printf("0xB00E: 0x%04X\n", applicationISRAddress);
    
    applicationISRAddress = __builtin_tblrdl(0xB010);
    void (*app_u1_rx_interrupt_func)() = (void*) applicationISRAddress;   
    printf("0xB010: 0x%04X\n", applicationISRAddress);
    
    applicationISRAddress = __builtin_tblrdl(0xB012);
    void (*app_u2_tx_interrupt_func)() = (void*) applicationISRAddress;   
    printf("0xB012: 0x%04X\n", applicationISRAddress);
    
    applicationISRAddress = __builtin_tblrdl(0xB014);
    void (*app_c1_interrupt_func)() = (void*) applicationISRAddress;   
    printf("0xB014: 0x%04X\n", applicationISRAddress);


    while (1) {

        CanMainStateMachine_run(); // Running a CAN input for running it with pure OpenLcb Messages use MainStatemachine_run();

    }

}
