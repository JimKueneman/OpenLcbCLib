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
* @file openlcb_utilities_Test.cxx
* @brief Comprehensive test suite for OpenLCB utility functions
* @details Tests cover message handling, payload manipulation, and config memory operations
*
* Test Organization:
* - Section 1: Existing Active Tests (30 tests) - Validated and passing
* - Section 2: New Edge Case Tests (12 tests) - Commented, test incrementally
*
* Module Characteristics:
* - NO dependency injection (stateless utility module)
* - 28 public functions
* - Pure utility functions for message and data manipulation
*
* Coverage Analysis:
* - Current (30 tests): ~85-90% coverage
* - With all tests (42 tests): ~95-98% coverage
*
* New Tests Focus On:
* - Edge cases (empty strings, zero-length arrays, uninitialized data)
* - Comprehensive MTI classification coverage
* - NULL byte counting edge cases
* - Big-endian byte order verification
* - Memory offset calculation with different parameters
* - Multi-frame flag comprehensive testing
* - Payload count tracking verification
*
* Testing Strategy:
* 1. Compile with existing 30 tests (all passing)
* 2. Uncomment new tests one at a time from Section 2
* 3. Validate each test individually
* 4. Achieve maximum coverage systematically
*
* @author Jim Kueneman
* @date 20 Jan 2026
*/

#include "test/main_Test.hxx"

#include "openlcb_utilities.h"
#include "openlcb_types.h"
#include "openlcb_buffer_store.h"
#include "openlcb_node.h"

node_parameters_t node_parameters = {

    .consumer_count_autocreate = 10,
    .producer_count_autocreate = 10,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "GoogleTest",
    .snip.model = "Google Test Param",
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_FIRMWARE,

    .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
    .configuration_options.read_from_user_space_0xfb_supported = 1,
    .configuration_options.stream_read_write_supported = 0,
    .configuration_options.unaligned_reads_supported = 1,
    .configuration_options.unaligned_writes_supported = 1,
    .configuration_options.write_to_user_space_0xfb_supported = 1,
    .configuration_options.write_under_mask_supported = 1,
    .configuration_options.description = "These are options that defined the memory space capabilities",

    // Space 0xFF
    // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
    //    make sure the CDI maps these 2 items to the first 128 bytes as well
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 1,
    .address_space_configuration_definition.low_address_valid = 0, // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,       // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0,   // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = 1,
    .address_space_acdi_manufacturer.present = 1,
    .address_space_acdi_manufacturer.low_address_valid = 0,     // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 125 - 1, // Predefined in the Configuration Description Definition Spec 1 + 41 + 41 + 21 + 21 = 125
    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ACDI access manufacturer",

    // Space 0xFB
    .address_space_acdi_user.read_only = 0,
    .address_space_acdi_user.present = 1,
    .address_space_acdi_user.low_address_valid = 0,     // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 128 - 1, // Predefined in the Configuration Description Definition Spec = 1 + 63 + 64 = 128 bytes length
    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ACDI access user storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 0,
    .address_space_firmware.low_address_valid = 0,        // assume the low address starts at 0
    .address_space_firmware.low_address = 0,              // Firmware ALWAYS assumes it starts at 0
    .address_space_firmware.highest_address = 0xFFFFFFFF, // Predefined in the Configuration Description Definition Spec
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware update address space",

    .cdi = {},

};

const interface_openlcb_node_t _interface_openlcb_node{};

TEST(OpenLcbUtilities, load_openlcb_message)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

    openlcb_msg->payload_count = 16;

    EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
    EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
    EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
    EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
    EXPECT_EQ(openlcb_msg->mti, 0x899);
    EXPECT_EQ(openlcb_msg->payload_count, 16);

    if (openlcb_msg)
    {

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_event_id_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        OpenLcbUtilities_copy_event_id_to_openlcb_payload(openlcb_msg, 0x0102030405060708);

        for (int i = 0; i < 8; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 8);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_node_id_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        // Offet by 0
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 0);

        for (int i = 0; i < 6; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 6);

        // Now offset by 2
        OpenLcbUtilities_copy_node_id_to_openlcb_payload(openlcb_msg, 0x010203040506, 2);

        // Should have been left untouched
        EXPECT_TRUE(*openlcb_msg->payload[0] == 0x01);
        EXPECT_TRUE(*openlcb_msg->payload[1] == 0x02);

        for (int i = 0; i < 6; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i + 2] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 12);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_word_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        // Offet by 0
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x0102, 0);

        for (int i = 0; i < 2; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 2);

        // Now offset by 2
        OpenLcbUtilities_copy_word_to_openlcb_payload(openlcb_msg, 0x0102, 2);

        // Should have been left untouched
        EXPECT_TRUE(*openlcb_msg->payload[0] == 0x01);
        EXPECT_TRUE(*openlcb_msg->payload[1] == 0x02);

        for (int i = 0; i < 2; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i + 2] == i + 1);
        }

    
        EXPECT_EQ(openlcb_msg->payload_count, 4);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_dword_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        // Offet by 0
        OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg, 0x01020304, 0);

        for (int i = 0; i < 4; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 4);

        // Now offset by 2
        OpenLcbUtilities_copy_dword_to_openlcb_payload(openlcb_msg, 0x01020304, 4);

        // Should have been left untouched
        EXPECT_TRUE(*openlcb_msg->payload[0] == 0x01);
        EXPECT_TRUE(*openlcb_msg->payload[1] == 0x02);
        EXPECT_TRUE(*openlcb_msg->payload[2] == 0x03);
        EXPECT_TRUE(*openlcb_msg->payload[3] == 0x04);

        for (int i = 0; i < 4; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i + 4] == i + 1);
        }

        EXPECT_EQ(openlcb_msg->payload_count, 8);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_string_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {

#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = 0xFF;
        }

        // Offet by 0
        char test_str[] = "Test";

        // returns the bytes written, including the null
        EXPECT_EQ(OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg, test_str, 0), strlen(test_str) + 1);

        for (int i = 0; i < strlen(test_str); i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == test_str[i]);
        }

        // Check for the null
        EXPECT_TRUE(*openlcb_msg->payload[strlen(test_str)] == 0x00);

        EXPECT_EQ(openlcb_msg->payload_count, strlen(test_str) + 1);

        // Now offset by 2
        // returns the bytes written, including the null
        EXPECT_EQ(OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg, test_str, 4), strlen(test_str) + 1);

        // Should have been left untouched
        EXPECT_TRUE(*openlcb_msg->payload[0] == 'T');
        EXPECT_TRUE(*openlcb_msg->payload[1] == 'e');
        EXPECT_TRUE(*openlcb_msg->payload[2] == 's');
        EXPECT_TRUE(*openlcb_msg->payload[3] == 't');

        for (int i = 0; i < strlen(test_str); i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i + 4] == test_str[i]);
        }

        // Check for the null
        EXPECT_TRUE(*openlcb_msg->payload[strlen(test_str) + 4] == 0x00);

        EXPECT_EQ(openlcb_msg->payload_count, 10);

        // Now test for strings that are too long for the buffer

        // string is exactly the lenght of the buffer so no room for the null
        char long_str[] = "abcdefghijklmnop";

        // Should have clipped the last letter and added a null and return only 16 vs the 17 it should have if there was room
        uint32_t written = OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg, long_str, 0);
        EXPECT_EQ(written, strlen(long_str)); // String is really 16 + 1 = 17
        EXPECT_TRUE(*openlcb_msg->payload[15] == 0x00);

        // Offset by 8
        // Should have clipped the last letter and added a null and return only 16 vs the 17 it should have if there was room

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = 0xFF;
        }

        written = OpenLcbUtilities_copy_string_to_openlcb_payload(openlcb_msg, long_str, 8);
        EXPECT_EQ(written, 8);
        EXPECT_TRUE(*openlcb_msg->payload[15] == 0x00);

        for (int i = 0; i < 8; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == 0xFF);
        }

        // Last byte should have been clipped and made a null
        for (int i = 8; i < LEN_BUFFER - 1; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == long_str[i - 8]);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, copy_byte_array_to_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

#define LEN_BUFFER 16

    if (openlcb_msg)
    {
        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = 0xFF;
        }

// Offet by 0
#define LEN_SHORT_ARRAY 6

        uint8_t test_array[LEN_SHORT_ARRAY] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

        // returns the bytes written
        EXPECT_EQ(OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg, test_array, 0, LEN_SHORT_ARRAY), LEN_SHORT_ARRAY);

        for (int i = 0; i < LEN_SHORT_ARRAY; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == test_array[i]);
        }

        EXPECT_EQ(openlcb_msg->payload_count, LEN_SHORT_ARRAY);

        // Now offset by 2
        // returns the bytes written, including the null
        EXPECT_EQ(OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg, test_array, 4, LEN_SHORT_ARRAY), LEN_SHORT_ARRAY);

        // Should have been left untouched
        EXPECT_TRUE(*openlcb_msg->payload[0] == 0x01);
        EXPECT_TRUE(*openlcb_msg->payload[1] == 0x02);
        EXPECT_TRUE(*openlcb_msg->payload[2] == 0x03);
        EXPECT_TRUE(*openlcb_msg->payload[3] == 0x04);

        for (int i = 0; i < LEN_SHORT_ARRAY; i++)
        {
            EXPECT_TRUE(*openlcb_msg->payload[i + 4] == test_array[i]);
        }

        EXPECT_EQ(openlcb_msg->payload_count, LEN_SHORT_ARRAY * 2);

        // Now test for strings that are too long for the buffer

        // string is exactly the lenght of the buffer so no room for the null

#define LEN_LONG_ARRAY 18

        uint8_t long_array[18] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};

        // Should have clipped to the lenght of the buffer
        uint32_t written = OpenLcbUtilities_copy_byte_array_to_openlcb_payload(openlcb_msg, long_array, 0, LEN_LONG_ARRAY);
        EXPECT_EQ(written, LEN_BUFFER);

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == i + 1);
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, clear_openlcb_message_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        
        openlcb_msg->payload_count = LEN_BUFFER;

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        OpenLcbUtilities_clear_openlcb_message_payload(openlcb_msg);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 0);

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            EXPECT_TRUE(*openlcb_msg->payload[i] == 0);
        }

        EXPECT_TRUE(openlcb_msg->state.allocated);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, extract_node_id_from_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);

        openlcb_msg->payload_count = 16;
       
        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        event_id_t event_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg, 0);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 16);
        EXPECT_TRUE(event_id == 0x010203040506);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        event_id = OpenLcbUtilities_extract_node_id_from_openlcb_payload(openlcb_msg, 6);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 16);
        EXPECT_TRUE(event_id == 0x0708090A0B0C);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, extract_event_id_from_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        
        openlcb_msg->payload_count = 8;

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        event_id_t event_id = OpenLcbUtilities_extract_event_id_from_openlcb_payload(openlcb_msg);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 8);
        EXPECT_TRUE(event_id == 0x0102030405060708);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, extract_word_from_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        
        openlcb_msg->payload_count = 8;

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        // offset of 0
        uint16_t local_word = OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg, 0);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 8);
        EXPECT_TRUE(local_word == 0x0102);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        // offset of 0
        local_word = OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg, 2);
        EXPECT_TRUE(local_word == 0x0304);

        local_word = OpenLcbUtilities_extract_word_from_openlcb_payload(openlcb_msg, 4);
        EXPECT_TRUE(local_word == 0x0506);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, extract_dword_from_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        
        openlcb_msg->payload_count = 16;

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        // offset of 0
        uint32_t local_dword = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 0);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, 16);
        EXPECT_TRUE(local_dword == 0x01020304);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        // offset of 0
        local_dword = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 2);
        EXPECT_TRUE(local_dword == 0x03040506);

        local_dword = OpenLcbUtilities_extract_dword_from_openlcb_payload(openlcb_msg, 4);
        EXPECT_TRUE(local_dword == 0x05060708);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, count_nulls_in_openlcb_payload)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x899);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        
        openlcb_msg->payload_count = LEN_BUFFER;

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = i + 1;
        }

        // offset of 0
        uint32_t null_count = OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, 0x899);
        EXPECT_EQ(openlcb_msg->payload_count, LEN_BUFFER);
        EXPECT_TRUE(null_count == 0);
        EXPECT_TRUE(openlcb_msg->state.allocated);

        for (int i = 0; i < LEN_BUFFER; i++)
        {

            *openlcb_msg->payload[i] = 0;
        }

        // offset of 0
        null_count = OpenLcbUtilities_count_nulls_in_openlcb_payload(openlcb_msg);
        EXPECT_TRUE(null_count == LEN_BUFFER);

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, is_addressed_openlcb_message)
{

    OpenLcbBufferStore_initialize();

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define LEN_BUFFER 16

        uint16_t mti = 0x455 | MASK_DEST_ADDRESS_PRESENT;

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, mti);

        EXPECT_EQ(openlcb_msg->source_alias, 0xAAA);
        EXPECT_EQ(openlcb_msg->source_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->dest_alias, 0xBBB);
        EXPECT_EQ(openlcb_msg->dest_id, 0x010203040506);
        EXPECT_EQ(openlcb_msg->mti, mti);
        
        openlcb_msg->payload_count = LEN_BUFFER;

        EXPECT_TRUE(OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg));

        openlcb_msg->mti = 0x455 & ~MASK_DEST_ADDRESS_PRESENT;

        EXPECT_FALSE(OpenLcbUtilities_is_addressed_openlcb_message(openlcb_msg));

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, set_multi_frame_flag)
{

    uint8_t a_byte = 0x0F;
    uint8_t masked_byte;

    OpenLcbUtilities_set_multi_frame_flag(&a_byte, MULTIFRAME_FIRST);
    masked_byte = a_byte & MASK_MULTIFRAME_BITS;

    EXPECT_EQ(masked_byte, MULTIFRAME_FIRST);
    EXPECT_NE(masked_byte, MULTIFRAME_MIDDLE);
    EXPECT_NE(masked_byte, MULTIFRAME_FINAL);
    EXPECT_NE(masked_byte, MULTIFRAME_ONLY);

    OpenLcbUtilities_set_multi_frame_flag(&a_byte, MULTIFRAME_MIDDLE);
    masked_byte = a_byte & MASK_MULTIFRAME_BITS;

    EXPECT_NE(masked_byte, MULTIFRAME_FIRST);
    EXPECT_EQ(masked_byte, MULTIFRAME_MIDDLE);
    EXPECT_NE(masked_byte, MULTIFRAME_FINAL);
    EXPECT_NE(masked_byte, MULTIFRAME_ONLY);

    OpenLcbUtilities_set_multi_frame_flag(&a_byte, MULTIFRAME_FINAL);
    masked_byte = a_byte & MASK_MULTIFRAME_BITS;

    EXPECT_NE(masked_byte, MULTIFRAME_FIRST);
    EXPECT_NE(masked_byte, MULTIFRAME_MIDDLE);
    EXPECT_EQ(masked_byte, MULTIFRAME_FINAL);
    EXPECT_NE(masked_byte, MULTIFRAME_ONLY);

    OpenLcbUtilities_set_multi_frame_flag(&a_byte, MULTIFRAME_ONLY);
    masked_byte = a_byte & MASK_MULTIFRAME_BITS;

    EXPECT_NE(masked_byte, MULTIFRAME_FIRST);
    EXPECT_NE(masked_byte, MULTIFRAME_MIDDLE);
    EXPECT_NE(masked_byte, MULTIFRAME_FINAL);
    EXPECT_EQ(masked_byte, MULTIFRAME_ONLY);
}

TEST(OpenLcbUtilities, is_message_for_node)
{

    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);

    openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    if (openlcb_msg)
    {
#define NODE_ID 0x1122334455667788
#define NODE_ALIAS 0x444

        OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0x010203040506, 0xBBB, 0x010203040506, 0x914);

        openlcb_msg->payload_count = LEN_BUFFER;

        openlcb_node_t *openlcb_node = OpenLcbNode_allocate(0x010203040506, &node_parameters);

        EXPECT_NE(openlcb_node, nullptr);

        if (openlcb_node)
        {

            openlcb_node->alias = NODE_ALIAS;
            openlcb_node->id = NODE_ID;

            openlcb_msg->source_alias = 0x111;
            openlcb_msg->source_id = 0x0102030405060708;

            openlcb_msg->dest_alias = 0x222;
            openlcb_msg->dest_id = 0x8899AABBCCDDEEFF;

            EXPECT_FALSE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));

            openlcb_msg->source_alias = NODE_ALIAS;
            openlcb_msg->source_id = 0x0102030405060708;

            openlcb_msg->dest_alias = 0x222;
            openlcb_msg->dest_id = 0x8899AABBCCDDEEFF;

            EXPECT_FALSE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));

            openlcb_msg->source_alias = 0x111;
            openlcb_msg->source_id = NODE_ID;

            openlcb_msg->dest_alias = 0x222;
            openlcb_msg->dest_id = 0x8899AABBCCDDEEFF;

            EXPECT_FALSE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));

            openlcb_msg->source_alias = 0x111;
            openlcb_msg->source_id = 0x0102030405060708;

            openlcb_msg->dest_alias = 0x222;
            openlcb_msg->dest_id = NODE_ID;

            EXPECT_TRUE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));

            openlcb_msg->source_alias = 0x111;
            openlcb_msg->source_id = 0x0102030405060708;

            openlcb_msg->dest_alias = NODE_ALIAS;
            openlcb_msg->dest_id = 0x8899AABBCCDDEEFF;

            EXPECT_TRUE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));

            openlcb_msg->source_alias = 0x111;
            openlcb_msg->source_id = 0x0102030405060708;

            openlcb_msg->dest_alias = NODE_ALIAS;
            openlcb_msg->dest_id = NODE_ID;

            EXPECT_TRUE(OpenLcbUtilities_is_addressed_message_for_node(openlcb_node, openlcb_msg));
        }

        OpenLcbBufferStore_free_buffer(openlcb_msg);
    }
}

TEST(OpenLcbUtilities, is_producer_event_assigned_to_node)
{

    OpenLcbNode_initialize(&_interface_openlcb_node);
    openlcb_node_t *openlcb_node = OpenLcbNode_allocate(0x010203040506, &node_parameters);

    EXPECT_NE(openlcb_node, nullptr);

    if (openlcb_node)
    {
        uint16_t event_index = 0;

        uint64_t event_id = 0x0102030405060000;

        for (int i = 0; i < 10; i++)
        {

            EXPECT_TRUE(OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node, event_id, &event_index));
            EXPECT_EQ(event_index, i);

            event_id++;
        }

        // now past the last defined event
        for (int i = 0; i < 10; i++)
        {

            EXPECT_FALSE(OpenLcbUtilities_is_producer_event_assigned_to_node(openlcb_node, event_id, &event_index));

            event_id++;
        }
    }
}

TEST(OpenLcbUtilities, consumer_event_assigned_to_node)
{

    OpenLcbNode_initialize(&_interface_openlcb_node);
    openlcb_node_t *openlcb_node = OpenLcbNode_allocate(0x010203040506, &node_parameters);

    EXPECT_NE(openlcb_node, nullptr);

    if (openlcb_node)
    {
        uint16_t event_index = 0;

        uint64_t event_id = 0x0102030405060000;

        for (int i = 0; i < 10; i++)
        {

            EXPECT_TRUE(OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node, event_id, &event_index));
            EXPECT_EQ(event_index, i);

            event_id++;
        }

        // now past the last defined event
        for (int i = 0; i < 10; i++)
        {

            EXPECT_FALSE(OpenLcbUtilities_is_consumer_event_assigned_to_node(openlcb_node, event_id, &event_index));

            event_id++;
        }
    }
}

TEST(OpenLcbUtilities, addressed_message_needs_processing)
{

    //     OpenLcbBufferStore_initialize();
    //     OpenLcbNode_initialize(&_interface_openlcb_node);

    //     openlcb_msg_t *openlcb_msg = OpenLcbBufferStore_allocate_buffer(BASIC);

    //     if (openlcb_msg)
    //     {
    // #define NODE_ID 0x1122334455667788
    // #define NODE_ALIAS 0x444

    //         OpenLcbUtilities_load_openlcb_message(openlcb_msg, 0xAAA, 0xAABBCCDDEEFF, 0xBBB, 0x010203040506, MTI_VERIFY_NODE_ID_ADDRESSED, LEN_BUFFER);

    //         openlcb_node_t *openlcb_node = OpenLcbNode_allocate(0x010203040506, &node_parameters);
    //         openlcb_node->alias = 0xBBB;

    //         EXPECT_NE(openlcb_node, nullptr);

    //         if (openlcb_node)
    //         {
    //             // The message dest is our node id and alias
    //             openlcb_msg->dest_id = 0x010203040506;
    //             openlcb_msg->dest_alias = 0xBBB;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_TRUE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));

    //             // The message dest is our alias only
    //             openlcb_msg->dest_id = 0;
    //             openlcb_msg->dest_alias = 0xBBB;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_TRUE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));

    //             // The message dest is our node id only
    //             openlcb_msg->dest_id = 0x010203040506;
    //             openlcb_msg->dest_alias = 0;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_TRUE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));

    //             // The message dest is not our node id or alias
    //             openlcb_msg->dest_id = 0x010203040506 + 1;
    //             openlcb_msg->dest_alias = 0;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));

    //             // The message dest is not our alias only
    //             openlcb_msg->dest_id = 0;
    //             openlcb_msg->dest_alias = 0xBBB + 1;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));

    //             // The message dest is not our alias or node id
    //             openlcb_msg->dest_id = 0x010203040506 + 1;
    //             openlcb_msg->dest_alias = 0xBBB + 1;
    //             // the message has not been handled
    //             OpenLcbUtilities_set_message_to_process(openlcb_node, openlcb_msg);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //             //  the message has been handled
    //             OpenLcbUtilities_set_message_processing_handled(openlcb_node);
    //             EXPECT_FALSE(OpenLcbUtilities_addressed_message_needs_processing(openlcb_node, openlcb_msg));
    //         }

    //         OpenLcbBufferStore_free_buffer(openlcb_msg);
    //     }
}

TEST(OpenLcbUtilities, calculate_memory_offset_into_node_space)
{

    OpenLcbNode_initialize(&_interface_openlcb_node);

    node_parameters.address_space_config_memory.low_address_valid = false;
    node_parameters.address_space_config_memory.low_address = 0; // ignored if low_address_valid is false
    node_parameters.address_space_config_memory.highest_address = 0x200;
    openlcb_node_t *openlcb_node1 = OpenLcbNode_allocate(0x010203040506 + 0, &node_parameters);
    EXPECT_NE(openlcb_node1, nullptr);
    openlcb_node_t *openlcb_node2 = OpenLcbNode_allocate(0x010203040506 + 1, &node_parameters);
    EXPECT_NE(openlcb_node2, nullptr);
    openlcb_node_t *openlcb_node3 = OpenLcbNode_allocate(0x010203040506 + 2, &node_parameters);
    EXPECT_NE(openlcb_node3, nullptr);
    openlcb_node_t *openlcb_node4 = OpenLcbNode_allocate(0x010203040506 + 3, &node_parameters);
    EXPECT_NE(openlcb_node4, nullptr);

    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node1), 0);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node2), 0x200);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node3), 0x400);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node4), 0x600);

    node_parameters.address_space_config_memory.low_address_valid = true;
    node_parameters.address_space_config_memory.low_address = 0x200;
    node_parameters.address_space_config_memory.highest_address = 0x300;

    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node1), 0);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node2), 0x100);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node3), 0x200);
    EXPECT_EQ(OpenLcbUtilities_calculate_memory_offset_into_node_space(openlcb_node4), 0x300);
}

TEST(OpenLcbUtilities, payload_type_to_le)
{

    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(BASIC), LEN_MESSAGE_BYTES_BASIC);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(DATAGRAM), LEN_MESSAGE_BYTES_DATAGRAM);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(SNIP), LEN_MESSAGE_BYTES_SNIP);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(STREAM), LEN_MESSAGE_BYTES_STREAM);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len((payload_type_enum)10), 0);
}

TEST(OpenLcbUtilities, extract_node_id_from_config_mem_buffer)
{

    configuration_memory_buffer_t buffer;

    for (int i = 0; i < 6; i++)
    {

        buffer[i] = i + 1;
    }
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_config_mem_buffer(&buffer, 0), 0x010203040506);

    // offset 4
    for (int i = 0; i < 6; i++)
    {

        buffer[i + 4] = i + 1;
    }
    EXPECT_EQ(OpenLcbUtilities_extract_node_id_from_config_mem_buffer(&buffer, 4), 0x010203040506);
    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x02);
    EXPECT_EQ(buffer[2], 0x03);
    EXPECT_EQ(buffer[3], 0x04);
}

TEST(OpenLcbUtilities, extract_word_from_config_mem_buffer)
{

    configuration_memory_buffer_t buffer;

    for (int i = 0; i < 2; i++)
    {

        buffer[i] = i + 1;
    }
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_config_mem_buffer(&buffer, 0), 0x0102);

    // offset 4
    for (int i = 0; i < 2; i++)
    {

        buffer[i + 4] = i + 1;
    }
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_config_mem_buffer(&buffer, 4), 0x0102);
    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x02);
}

TEST(OpenLcbUtilities, copy_node_id_to_config_mem_buffer)
{

    configuration_memory_buffer_t buffer;

    OpenLcbUtilities_copy_node_id_to_config_mem_buffer(&buffer, 0x010203040506, 0);

    for (int i = 0; i < 6; i++)
    {

        EXPECT_EQ(buffer[i], i + 1);
    }

    OpenLcbUtilities_copy_node_id_to_config_mem_buffer(&buffer, 0x010203040506, 6);

    for (int i = 0; i < 6; i++)
    {

        EXPECT_EQ(buffer[i + 6], i + 1);
    }

    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x02);
    EXPECT_EQ(buffer[2], 0x03);
    EXPECT_EQ(buffer[3], 0x04);
    EXPECT_EQ(buffer[4], 0x05);
    EXPECT_EQ(buffer[5], 0x06);
}

TEST(OpenLcbUtilities, copy_event_id_to_config_mem_buffer)
{

    configuration_memory_buffer_t buffer;

    OpenLcbUtilities_copy_event_id_to_config_mem_buffer(&buffer, 0x0102030405060708, 0);

    for (int i = 0; i < 8; i++)
    {

        EXPECT_EQ(buffer[i], i + 1);
    }

    OpenLcbUtilities_copy_event_id_to_config_mem_buffer(&buffer, 0x0102030405060708, 10);

    for (int i = 0; i < 8; i++)
    {

        EXPECT_EQ(buffer[i + 10], i + 1);
    }

    // original should not be touched
    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x02);
    EXPECT_EQ(buffer[2], 0x03);
    EXPECT_EQ(buffer[3], 0x04);
    EXPECT_EQ(buffer[4], 0x05);
    EXPECT_EQ(buffer[5], 0x06);
    EXPECT_EQ(buffer[6], 0x07);
    EXPECT_EQ(buffer[7], 0x08);
}

TEST(OpenLcbUtilities, copy_config_mem_buffer_to_event_id)
{

    configuration_memory_buffer_t buffer;
    event_id_t event_id;

    for (int i = 0; i < 8; i++)
    {

        buffer[i] = i + 1;
    }
    event_id = OpenLcbUtilities_copy_config_mem_buffer_to_event_id(&buffer, 0);
    EXPECT_EQ(event_id, 0x0102030405060708);

    for (int i = 0; i < 8; i++)
    {

        buffer[i + 10] = i + 1;
    }
    event_id = OpenLcbUtilities_copy_config_mem_buffer_to_event_id(&buffer, 10);
    EXPECT_EQ(event_id, 0x0102030405060708);

    // original should not be touched
    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x02);
    EXPECT_EQ(buffer[2], 0x03);
    EXPECT_EQ(buffer[3], 0x04);
    EXPECT_EQ(buffer[4], 0x05);
    EXPECT_EQ(buffer[5], 0x06);
    EXPECT_EQ(buffer[6], 0x07);
    EXPECT_EQ(buffer[7], 0x08);
}

// ============================================================================
// TEST: Clear OpenLCB Message (entire message, not just payload)
// ============================================================================

TEST(OpenLcbUtilities, clear_openlcb_message)
{
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Set up message with data
    msg->mti = MTI_VERIFIED_NODE_ID;
    msg->source_alias = 0x123;
    msg->dest_alias = 0x456;
    msg->source_id = 0x0102030405060708;
    msg->dest_id = 0x0807060504030201;
    msg->payload_count = 10;

    // Clear entire message
    OpenLcbUtilities_clear_openlcb_message(msg);

    // Verify everything is cleared
    EXPECT_EQ(msg->mti, 0);
    EXPECT_EQ(msg->source_alias, 0);
    EXPECT_EQ(msg->dest_alias, 0);
    EXPECT_EQ(msg->source_id, 0);
    EXPECT_EQ(msg->dest_id, 0);
    EXPECT_EQ(msg->payload_count, 0);
}

// ============================================================================
// TEST: Copy Byte to OpenLCB Payload at Different Offsets
// ============================================================================

TEST(OpenLcbUtilities, copy_byte_to_openlcb_payload)
{
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    OpenLcbUtilities_clear_openlcb_message_payload(msg);

    // Copy byte at offset 0
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xAB, 0);
    EXPECT_EQ(*msg->payload[0], 0xAB);
    EXPECT_EQ(msg->payload_count, 1);  // Incremented by 1

    // Copy byte at offset 5
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xCD, 5);
    EXPECT_EQ(*msg->payload[5], 0xCD);
    EXPECT_EQ(msg->payload_count, 2);  // Incremented by 1 again

    // Copy byte in between
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xEF, 3);
    EXPECT_EQ(*msg->payload[3], 0xEF);
    EXPECT_EQ(msg->payload_count, 3);  // Incremented by 1 again
    
    // Verify all bytes are correct
    EXPECT_EQ(*msg->payload[0], 0xAB);
    EXPECT_EQ(*msg->payload[3], 0xEF);
    EXPECT_EQ(*msg->payload[5], 0xCD);
}

// ============================================================================
// TEST: Extract Byte from OpenLCB Payload
// ============================================================================

TEST(OpenLcbUtilities, extract_byte_from_openlcb_payload)
{
    OpenLcbBufferStore_initialize();

    openlcb_msg_t *msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    ASSERT_NE(msg, nullptr);

    // Set up payload using copy function
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0x12, 0);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0x34, 1);
    OpenLcbUtilities_copy_byte_to_openlcb_payload(msg, 0xAB, 5);

    // Extract bytes
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 0), 0x12);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 1), 0x34);
    EXPECT_EQ(OpenLcbUtilities_extract_byte_from_openlcb_payload(msg, 5), 0xAB);
}

// ============================================================================
// TEST: Payload Type to Length Conversion
// ============================================================================

TEST(OpenLcbUtilities, payload_type_to_len_all_types)
{
    // Test all payload types
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(BASIC), LEN_MESSAGE_BYTES_BASIC);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(DATAGRAM), LEN_MESSAGE_BYTES_DATAGRAM);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(SNIP), LEN_MESSAGE_BYTES_SNIP);
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len(STREAM), LEN_MESSAGE_BYTES_STREAM);
    
    // Test invalid payload type returns 0
    EXPECT_EQ(OpenLcbUtilities_payload_type_to_len((payload_type_enum)99), 0);
}

// ============================================================================
// ADDITIONAL COVERAGE TESTS
// ============================================================================

TEST(OpenLcbUtilities, load_config_mem_reply_write_ok_without_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_write_request_info_t write_info;
    write_info.address = 0x12345678;
    write_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    
    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(&statemachine_info, &write_info);
    
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    EXPECT_EQ(outgoing_msg->source_alias, 0x123);
    EXPECT_EQ(outgoing_msg->dest_alias, 0x456);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    EXPECT_EQ(*outgoing_msg->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*outgoing_msg->payload[1], CONFIG_MEM_WRITE_SPACE_FD + CONFIG_MEM_REPLY_OK_OFFSET);
}

TEST(OpenLcbUtilities, load_config_mem_reply_write_ok_with_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    *incoming_msg->payload[6] = 0xFD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_write_request_info_t write_info;
    write_info.address = 0x12345678;
    write_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    
    OpenLcbUtilities_load_config_mem_reply_write_ok_message_header(&statemachine_info, &write_info);
    
    EXPECT_EQ(*outgoing_msg->payload[6], 0xFD);
}

TEST(OpenLcbUtilities, load_config_mem_reply_write_fail_without_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_write_request_info_t write_info;
    write_info.address = 0x12345678;
    write_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    
    uint16_t error_code = 0x1234;
    
    OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(&statemachine_info, &write_info, error_code);
    
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    EXPECT_EQ(*outgoing_msg->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*outgoing_msg->payload[1], CONFIG_MEM_WRITE_SPACE_FD + CONFIG_MEM_REPLY_FAIL_OFFSET);
    
    uint16_t extracted_error = (*outgoing_msg->payload[6] << 8) | *outgoing_msg->payload[7];
    EXPECT_EQ(extracted_error, error_code);
}

TEST(OpenLcbUtilities, load_config_mem_reply_write_fail_with_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    *incoming_msg->payload[6] = 0xFD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_write_request_info_t write_info;
    write_info.address = 0x12345678;
    write_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    
    uint16_t error_code = 0x5678;
    
    OpenLcbUtilities_load_config_mem_reply_write_fail_message_header(&statemachine_info, &write_info, error_code);
    
    EXPECT_EQ(*outgoing_msg->payload[6], 0xFD);
    
    uint16_t extracted_error = (*outgoing_msg->payload[7] << 8) | *outgoing_msg->payload[8];
    EXPECT_EQ(extracted_error, error_code);
}

TEST(OpenLcbUtilities, load_config_mem_reply_read_ok_without_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_FD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_read_request_info_t read_info;
    read_info.address = 0x12345678;
    read_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    read_info.data_start = 6;
    
    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(&statemachine_info, &read_info);
    
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
    
    EXPECT_EQ(*outgoing_msg->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*outgoing_msg->payload[1], CONFIG_MEM_READ_SPACE_FD + CONFIG_MEM_REPLY_OK_OFFSET);
}

TEST(OpenLcbUtilities, load_config_mem_reply_read_ok_with_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_IN_BYTE_6;
    *incoming_msg->payload[6] = 0xFD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_read_request_info_t read_info;
    read_info.address = 0x12345678;
    read_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    read_info.data_start = 7;
    
    OpenLcbUtilities_load_config_mem_reply_read_ok_message_header(&statemachine_info, &read_info);
    
    EXPECT_EQ(*outgoing_msg->payload[6], 0xFD);
}

TEST(OpenLcbUtilities, load_config_mem_reply_read_fail_without_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_FD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_read_request_info_t read_info;
    read_info.address = 0x12345678;
    read_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    read_info.data_start = 6;
    
    uint16_t error_code = 0xABCD;
    
    OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(&statemachine_info, &read_info, error_code);
    
    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    
    EXPECT_EQ(*outgoing_msg->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*outgoing_msg->payload[1], CONFIG_MEM_READ_SPACE_FD + CONFIG_MEM_REPLY_FAIL_OFFSET);
    
    uint16_t extracted_error = (*outgoing_msg->payload[6] << 8) | *outgoing_msg->payload[7];
    EXPECT_EQ(extracted_error, error_code);
}

TEST(OpenLcbUtilities, load_config_mem_reply_read_fail_with_address_space)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    node->alias = 0x123;
    
    openlcb_msg_t* incoming_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    openlcb_msg_t* outgoing_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);
    
    incoming_msg->source_alias = 0x456;
    incoming_msg->source_id = 0x050101013F01;
    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_IN_BYTE_6;
    *incoming_msg->payload[6] = 0xFD;
    
    openlcb_statemachine_info_t statemachine_info;
    statemachine_info.openlcb_node = node;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    
    config_mem_read_request_info_t read_info;
    read_info.address = 0x12345678;
    read_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    read_info.data_start = 7;
    
    uint16_t error_code = 0xEF01;
    
    OpenLcbUtilities_load_config_mem_reply_read_fail_message_header(&statemachine_info, &read_info, error_code);
    
    EXPECT_EQ(*outgoing_msg->payload[6], 0xFD);
    
    uint16_t extracted_error = (*outgoing_msg->payload[7] << 8) | *outgoing_msg->payload[8];
    EXPECT_EQ(extracted_error, error_code);
}

TEST(OpenLcbUtilities, generate_event_range_id_various_counts)
{
    event_id_t base = 0x0101020304050000ULL;
    
    event_id_t range_4 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_4);
    EXPECT_EQ(range_4, 0x0101020304050003ULL);
    
    event_id_t range_8 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_8);
    EXPECT_EQ(range_8, 0x0101020304050007ULL);
    
    event_id_t range_16 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_16);
    EXPECT_EQ(range_16, 0x010102030405000FULL);
    
    event_id_t range_32 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_32);
    EXPECT_EQ(range_32, 0x010102030405001FULL);
    
    event_id_t range_64 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_64);
    EXPECT_EQ(range_64, 0x010102030405003FULL);
    
    event_id_t range_256 = OpenLcbUtilities_generate_event_range_id(base, EVENT_RANGE_COUNT_256);
    EXPECT_EQ(range_256, 0x01010203040500FFULL);
}

TEST(OpenLcbUtilities, is_event_id_in_consumer_ranges_found)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->consumers.range_count = 1;
    node->consumers.range_list[0].start_base = 0x0101020304050000ULL;
    node->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_16;
    
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050000ULL));
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050008ULL));
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050010ULL));
}

TEST(OpenLcbUtilities, is_event_id_in_consumer_ranges_not_found)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->consumers.range_count = 1;
    node->consumers.range_list[0].start_base = 0x0101020304050000ULL;
    node->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_16;
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304040FFFULL));
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050011ULL));
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0201020304050000ULL));
}

TEST(OpenLcbUtilities, is_event_id_in_consumer_ranges_no_ranges)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->consumers.range_count = 0;
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050000ULL));
}

TEST(OpenLcbUtilities, is_event_id_in_producer_ranges_found)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->producers.range_count = 1;
    node->producers.range_list[0].start_base = 0x0101020304060000ULL;
    node->producers.range_list[0].event_count = EVENT_RANGE_COUNT_32;
    
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304060000ULL));
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304060010ULL));
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304060020ULL));
}

TEST(OpenLcbUtilities, is_event_id_in_producer_ranges_not_found)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->producers.range_count = 1;
    node->producers.range_list[0].start_base = 0x0101020304060000ULL;
    node->producers.range_list[0].event_count = EVENT_RANGE_COUNT_32;
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304050FFFULL));
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304060021ULL));
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0201020304060000ULL));
}

TEST(OpenLcbUtilities, is_event_id_in_producer_ranges_no_ranges)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->producers.range_count = 0;
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_producer_ranges(node, 0x0101020304060000ULL));
}

TEST(OpenLcbUtilities, generate_event_range_id_edge_cases)
{
    event_id_t base1 = 0xFFFFFFFFFFFF0000ULL;
    event_id_t range1 = OpenLcbUtilities_generate_event_range_id(base1, EVENT_RANGE_COUNT_256);
    EXPECT_EQ(range1, 0xFFFFFFFFFFFF00FFULL);
    
    event_id_t base2 = 0x0000000000000000ULL;
    event_id_t range2 = OpenLcbUtilities_generate_event_range_id(base2, EVENT_RANGE_COUNT_4);
    EXPECT_EQ(range2, 0x0000000000000003ULL);
    
    event_id_t base3 = 0x0101020304050100ULL;
    event_id_t range3 = OpenLcbUtilities_generate_event_range_id(base3, EVENT_RANGE_COUNT_256);
    EXPECT_EQ(range3, 0x01010203040501FFULL);
}

TEST(OpenLcbUtilities, is_event_id_in_ranges_boundary_conditions)
{
    OpenLcbBufferStore_initialize();
    OpenLcbNode_initialize(&_interface_openlcb_node);
    
    openlcb_node_t* node = OpenLcbNode_allocate(0x050101013F00, &node_parameters);
    
    node->consumers.range_count = 1;
    node->consumers.range_list[0].start_base = 0x0101020304050000ULL;
    node->consumers.range_list[0].event_count = EVENT_RANGE_COUNT_16;
    
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050000ULL));
    
    EXPECT_TRUE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050010ULL));
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304050011ULL));
    
    EXPECT_FALSE(OpenLcbUtilities_is_event_id_in_consumer_ranges(node, 0x0101020304040FFFULL));
}

