#include "test/main_Test.hxx"

#include "test/main_Test.hxx"

#include "protocol_config_mem_write_handler.h"
#include "openlcb_application.h"
#include "openlcb_types.h"
#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_datagram_handler.h"
#include "protocol_snip.h"

#define AUTO_CREATE_EVENT_COUNT 10
#define DEST_EVENT_ID 0x0605040302010000
#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201
#define SNIP_NAME_FULL "0123456789012345678901234567890123456789"
#define SNIP_MODEL "Test Model J"

#define CONFIG_MEM_START_ADDRESS 0x100
#define CONFIG_MEM_NODE_ADDRESS_ALLOCATION 0x200

#define CONFIG_MEM_ALL_HIGH_MEMORY 0x000A

void *called_function_ptr = nullptr;
bool load_datagram_ok_message_called = false;
bool load_datagram_rejected_message_called = false;
uint16_t datagram_reply_code = 0;
config_mem_write_request_info_t local_config_mem_write_request_info;
bool memory_write_return_zero = false;

uint16_t memory_write_requested_bytes = 0;
uint8_t memory_write_data[1024];

const node_parameters_t _node_parameters_main_node = {

    .consumer_count_autocreate = AUTO_CREATE_EVENT_COUNT,
    .producer_count_autocreate = AUTO_CREATE_EVENT_COUNT,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = SNIP_NAME_FULL,
    .snip.model = SNIP_MODEL,
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

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
    .address_space_configuration_definition.read_only = false,
    .address_space_configuration_definition.present = true,
    .address_space_configuration_definition.low_address_valid = false,  // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,            // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 1098 - 1, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = false,
    .address_space_all.present = true,
    .address_space_all.low_address_valid = false, // assume the low address starts at 0
    .address_space_all.low_address = 0,           // ignored if low_address_valid is false
    .address_space_all.highest_address = CONFIG_MEM_ALL_HIGH_MEMORY,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = false,
    .address_space_config_memory.present = true,
    .address_space_config_memory.low_address_valid = false,                            // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = false,
    .address_space_acdi_manufacturer.present = true,
    .address_space_acdi_manufacturer.low_address_valid = false, // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ADCI Manufacturer storage",

    // Space 0xFB
    .address_space_acdi_user.read_only = false,
    .address_space_acdi_user.present = true,
    .address_space_acdi_user.low_address_valid = false, // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ADCI User storage",

    // Space 0xFA
    .address_space_traction_function_definition_info.read_only = false,
    .address_space_traction_function_definition_info.present = true,
    .address_space_traction_function_definition_info.low_address_valid = false, // assume the low address starts at 0
    .address_space_traction_function_definition_info.low_address = 0,           // ignored if low_address_valid is false
    .address_space_traction_function_definition_info.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_traction_function_definition_info.address_space = CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO,
    .address_space_traction_function_definition_info.description = "Traction Configuration Definition Info",

    // Space 0xF9
    .address_space_traction_function_config_memory.read_only = false,
    .address_space_traction_function_config_memory.present = true,
    .address_space_traction_function_config_memory.low_address_valid = false, // assume the low address starts at 0
    .address_space_traction_function_config_memory.low_address = 0,           // ignored if low_address_valid is false
    .address_space_traction_function_config_memory.highest_address = 0x100,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_traction_function_config_memory.address_space = CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY,
    .address_space_traction_function_config_memory.description = "Traction Configuration Memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = false,
    .address_space_firmware.present = true,
    .address_space_firmware.low_address_valid = false, // assume the low address starts at 0
    .address_space_firmware.low_address = 0,           // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x100,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
            0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30, 0x22, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x22, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x22, 0x3F, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <?xml version="1.0" encoding="UTF-8"?>
            0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x22, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x74, 0x72, 0x75, 0x6E, 0x6B, 0x2F, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2F, 0x78, 0x6D, 0x6C, 0x2F, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x22, 0x3F, 0x3E,                                                                                                                                                                                                                                           // <?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>
            0x3C, 0x63, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x22, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x63, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x34, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x22, 0x3E, // <cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/4/cdi.xsd">
            0x3C, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <identification>
            0x3C, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x4F, 0x70, 0x65, 0x6E, 0x4C, 0x63, 0x62, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <manufacturer>Basic OpenLcb Node</manufacturer>
            0x3C, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E, 0x54, 0x65, 0x73, 0x74, 0x20, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <model>Test Application</model>
            0x3C, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x3C, 0x2F, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <hardwareVersion>0.0.1</hardwareVersion>
            0x3C, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x3C, 0x2F, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <softwareVersion>0.0.1</softwareVersion>
            0x3C, 0x6D, 0x61, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // <map>
            0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <relation>
            0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <property>Description</property>
            0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       // <value>Mustangpeak Test Node</value>
            0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // </relation>
            0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <relation>
            0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // <property>Status</property>
            0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <value>Prototype</value>
            0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // </relation>
            0x3C, 0x2F, 0x6D, 0x61, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // </map>
            0x3C, 0x2F, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </identification>
            0x3C, 0x61, 0x63, 0x64, 0x69, 0x2F, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <acdi/>
            0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <segment origin="0" space="253">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4C, 0x61, 0x79, 0x6F, 0x75, 0x74, 0x20, 0x43, 0x6F, 0x6E, 0x66, 0x69, 0x67, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <name>Layout Configuration Setup</name>
            0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x54, 0x68, 0x65, 0x20, 0x62, 0x61, 0x73, 0x69, 0x63, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x72, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x67, 0x65, 0x74, 0x20, 0x79, 0x6F, 0x75, 0x72, 0x20, 0x54, 0x75, 0x72, 0x6E, 0x6F, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73, 0x20, 0x75, 0x70,                                                                                                                                                                                                                                                                                                                                                                                           // <description>The basic information required to get your TurnoutBoss up
            0x61, 0x6E, 0x64, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x20, 0x61, 0x20, 0x66, 0x75, 0x6C, 0x6C, 0x79, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x61, 0x6C, 0x65, 0x64, 0x20, 0x6C, 0x61, 0x79, 0x6F, 0x75, 0x74, 0x2E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                               // and operational to create a fully signaled layout.</description>
            0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <group>
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <name>User Info</name>
            0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x65, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x6F, 0x20, 0x68, 0x65, 0x6C, 0x70, 0x20, 0x75, 0x6E, 0x69, 0x71, 0x75, 0x65, 0x6C, 0x79, 0x20, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x79, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x54, 0x75, 0x72, 0x6E, 0x6F, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73, 0x2E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                             // <description>Enter a name and description to help uniquely identify this TurnoutBoss.</description>
            0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x33, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <string size="63">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <name>User Name</name>
            0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </string>
            0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x34, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <string size="64">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // <name>User Description</name>
            0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </string>
            0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // </group>
            0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // </segment>
            0x3C, 0x2F, 0x63, 0x64, 0x69, 0x3E, 0x00                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      // </cdi>
        },

};

const node_parameters_t _node_parameters_main_node_all_not_present = {

    .consumer_count_autocreate = AUTO_CREATE_EVENT_COUNT,
    .producer_count_autocreate = AUTO_CREATE_EVENT_COUNT,

    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = SNIP_NAME_FULL,
    .snip.model = SNIP_MODEL,
    .snip.hardware_version = "0.001",
    .snip.software_version = "0.002",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

    .protocol_support = (PSI_DATAGRAM |
                         PSI_FIRMWARE_UPGRADE |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,

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
    .address_space_configuration_definition.read_only = true,
    .address_space_configuration_definition.present = true,
    .address_space_configuration_definition.low_address_valid = false,  // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,            // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 1098 - 1, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = true,
    .address_space_all.present = false,
    .address_space_all.low_address_valid = false, // assume the low address starts at 0
    .address_space_all.low_address = 0,           // ignored if low_address_valid is false
    .address_space_all.highest_address = CONFIG_MEM_ALL_HIGH_MEMORY,
    .address_space_all.address_space = CONFIG_MEM_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = false,
    .address_space_config_memory.present = true,
    .address_space_config_memory.low_address_valid = false,                            // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = true,
    .address_space_acdi_manufacturer.present = true,
    .address_space_acdi_manufacturer.low_address_valid = false, // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_acdi_manufacturer.address_space = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ADCI Manufacturer storage",

    // Space 0xFB
    .address_space_acdi_user.read_only = false,
    .address_space_acdi_user.present = true,
    .address_space_acdi_user.low_address_valid = false, // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_acdi_user.address_space = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ADCI User storage",

    // Space 0xFA
    .address_space_traction_function_definition_info.read_only = true,
    .address_space_traction_function_definition_info.present = true,
    .address_space_traction_function_definition_info.low_address_valid = false, // assume the low address starts at 0
    .address_space_traction_function_definition_info.low_address = 0,           // ignored if low_address_valid is false
    .address_space_traction_function_definition_info.highest_address = 0x0100,  // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_traction_function_definition_info.address_space = CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO,
    .address_space_traction_function_definition_info.description = "Traction Configuration Definition Info",

    // Space 0xF9
    .address_space_traction_function_config_memory.read_only = false,
    .address_space_traction_function_config_memory.present = true,
    .address_space_traction_function_config_memory.low_address_valid = false, // assume the low address starts at 0
    .address_space_traction_function_config_memory.low_address = 0,           // ignored if low_address_valid is false
    .address_space_traction_function_config_memory.highest_address = 0x100,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_traction_function_config_memory.address_space = CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY,
    .address_space_traction_function_config_memory.description = "Traction Configuration Memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = false,
    .address_space_firmware.present = true,
    .address_space_firmware.low_address_valid = false, // assume the low address starts at 0
    .address_space_firmware.low_address = 0,           // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x100,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = CONFIG_MEM_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
            0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30, 0x22, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x22, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x22, 0x3F, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <?xml version="1.0" encoding="UTF-8"?>
            0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x22, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x74, 0x72, 0x75, 0x6E, 0x6B, 0x2F, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2F, 0x78, 0x6D, 0x6C, 0x2F, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x22, 0x3F, 0x3E,                                                                                                                                                                                                                                           // <?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>
            0x3C, 0x63, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x22, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x63, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x34, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x22, 0x3E, // <cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/4/cdi.xsd">
            0x3C, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <identification>
            0x3C, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E, 0x42, 0x61, 0x73, 0x69, 0x63, 0x20, 0x4F, 0x70, 0x65, 0x6E, 0x4C, 0x63, 0x62, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <manufacturer>Basic OpenLcb Node</manufacturer>
            0x3C, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E, 0x54, 0x65, 0x73, 0x74, 0x20, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <model>Test Application</model>
            0x3C, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x3C, 0x2F, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <hardwareVersion>0.0.1</hardwareVersion>
            0x3C, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x30, 0x2E, 0x31, 0x3C, 0x2F, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <softwareVersion>0.0.1</softwareVersion>
            0x3C, 0x6D, 0x61, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // <map>
            0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <relation>
            0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <property>Description</property>
            0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       // <value>Mustangpeak Test Node</value>
            0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // </relation>
            0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <relation>
            0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // <property>Status</property>
            0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <value>Prototype</value>
            0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             // </relation>
            0x3C, 0x2F, 0x6D, 0x61, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // </map>
            0x3C, 0x2F, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </identification>
            0x3C, 0x61, 0x63, 0x64, 0x69, 0x2F, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <acdi/>
            0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // <segment origin="0" space="253">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4C, 0x61, 0x79, 0x6F, 0x75, 0x74, 0x20, 0x43, 0x6F, 0x6E, 0x66, 0x69, 0x67, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x53, 0x65, 0x74, 0x75, 0x70, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <name>Layout Configuration Setup</name>
            0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x54, 0x68, 0x65, 0x20, 0x62, 0x61, 0x73, 0x69, 0x63, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x72, 0x65, 0x71, 0x75, 0x69, 0x72, 0x65, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x67, 0x65, 0x74, 0x20, 0x79, 0x6F, 0x75, 0x72, 0x20, 0x54, 0x75, 0x72, 0x6E, 0x6F, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73, 0x20, 0x75, 0x70,                                                                                                                                                                                                                                                                                                                                                                                           // <description>The basic information required to get your TurnoutBoss up
            0x61, 0x6E, 0x64, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x72, 0x65, 0x61, 0x74, 0x65, 0x20, 0x61, 0x20, 0x66, 0x75, 0x6C, 0x6C, 0x79, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x61, 0x6C, 0x65, 0x64, 0x20, 0x6C, 0x61, 0x79, 0x6F, 0x75, 0x74, 0x2E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                               // and operational to create a fully signaled layout.</description>
            0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     // <group>
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <name>User Info</name>
            0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x45, 0x6E, 0x74, 0x65, 0x72, 0x20, 0x61, 0x20, 0x6E, 0x61, 0x6D, 0x65, 0x20, 0x61, 0x6E, 0x64, 0x20, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x6F, 0x20, 0x68, 0x65, 0x6C, 0x70, 0x20, 0x75, 0x6E, 0x69, 0x71, 0x75, 0x65, 0x6C, 0x79, 0x20, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x79, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x54, 0x75, 0x72, 0x6E, 0x6F, 0x75, 0x74, 0x42, 0x6F, 0x73, 0x73, 0x2E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E,                                                                                                                                                                                                             // <description>Enter a name and description to help uniquely identify this TurnoutBoss.</description>
            0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x33, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <string size="63">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           // <name>User Name</name>
            0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </string>
            0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x34, 0x22, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // <string size="64">
            0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // <name>User Description</name>
            0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         // </string>
            0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // </group>
            0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E,                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   // </segment>
            0x3C, 0x2F, 0x63, 0x64, 0x69, 0x3E, 0x00                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      // </cdi>
        },

};

void _update_called_function_ptr(void *function_ptr)
{
    called_function_ptr = (void *)((long long)function_ptr + (long long)called_function_ptr);
}

void _load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code)
{

    datagram_reply_code = return_code;
    statemachine_info->outgoing_msg_info.valid = false;

    _update_called_function_ptr((void *)&_load_datagram_received_ok_message);
}

void _load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code)
{
    datagram_reply_code = return_code;
    statemachine_info->outgoing_msg_info.valid = false;

    _update_called_function_ptr((void *)&_load_datagram_rejected_message);
}

void _write_request_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_config_decscription_info);
}

void _write_request_all(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_all);
}

void _write_request_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_config_memory);
}

void _write_request_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_acdi_manufacturer);
}

void _write_request_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_acdi_user);
}

void _write_request_traction_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_traction_config_decscription_info);
}

void _write_request_traction_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    statemachine_info->outgoing_msg_info.valid = false;
    local_config_mem_write_request_info = *config_mem_write_request_info;

    _update_called_function_ptr((void *)&_write_request_traction_config_memory);
}

uint16_t _config_memory_write(openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{

    _update_called_function_ptr((void *)&_config_memory_write);

    if (memory_write_return_zero)
    {

        return 0;
    }
    else
    {

        return count;
    }
}

uint16_t _delayed_reply_time(openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info)
{

    return 16000;
}

// uint16_t _snip_user_name_write(uint16_t byte_count, configuration_memory_buffer_t *buffer)
// {

//     _update_called_function_ptr((void *)&_snip_user_name_write);

//     if (memory_write_return_zero)
//     {

//         return 0;
//     }

//     memory_write_requested_bytes = byte_count;

//     for (int i = 0; i < byte_count; i++)
//     {

//         memory_write_data[i] = (*buffer)[i];
//     }

//     return byte_count;
// }

// uint16_t _snip_user_description_write(uint16_t byte_count, configuration_memory_buffer_t *buffer)
// {

//     _update_called_function_ptr((void *)&_snip_user_description_write);

//     if (memory_write_return_zero)
//     {

//         return 0;
//     }

//     memory_write_requested_bytes = byte_count;

//     for (int i = 0; i < byte_count; i++)
//     {

//         memory_write_data[i] = (*buffer)[i];
//     }

//     return byte_count;
// }

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    .write_request_config_definition_info = &_write_request_config_decscription_info,
    .write_request_all = &_write_request_all,
    .write_request_config_mem = &_write_request_config_memory,
    .write_request_acdi_manufacturer = &_write_request_acdi_manufacturer,
    .write_request_acdi_user = &_write_request_acdi_user,
    .write_request_traction_function_config_definition_info = &_write_request_traction_config_decscription_info,
    .write_request_traction_function_config_memory = &_write_request_traction_config_memory,

    .config_memory_write = &_config_memory_write,
    .delayed_reply_time = nullptr

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler_config_memory_write_defined = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    .config_memory_write = &_config_memory_write,

    .write_request_config_definition_info = &_write_request_config_decscription_info,
    .write_request_all = &_write_request_all,
    .write_request_config_mem = &_write_request_config_memory,
    .write_request_acdi_manufacturer = &_write_request_acdi_manufacturer,
    .write_request_acdi_user = &_write_request_acdi_user,
    .write_request_traction_function_config_definition_info = &_write_request_traction_config_decscription_info,
    .write_request_traction_function_config_memory = &_write_request_traction_config_memory,

    .delayed_reply_time = nullptr

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler_config_memory_write_and_delayed_reply_time_defined = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    .write_request_config_definition_info = &_write_request_config_decscription_info,
    .write_request_all = &_write_request_all,
    .write_request_config_mem = &_write_request_config_memory,
    .write_request_acdi_manufacturer = &_write_request_acdi_manufacturer,
    .write_request_acdi_user = &_write_request_acdi_user,
    .write_request_traction_function_config_definition_info = &_write_request_traction_config_decscription_info,
    .write_request_traction_function_config_memory = &_write_request_traction_config_memory,

    .config_memory_write = &_config_memory_write,
    .delayed_reply_time = &_delayed_reply_time

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler_with_nulls = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    .write_request_config_definition_info = nullptr,
    .write_request_all = nullptr,
    .write_request_config_mem = nullptr,
    .write_request_acdi_manufacturer = nullptr,
    .write_request_acdi_user = nullptr,
    .write_request_traction_function_config_definition_info = nullptr,
    .write_request_traction_function_config_memory = nullptr,

    .config_memory_write = nullptr,
    .delayed_reply_time = nullptr

};

interface_openlcb_protocol_snip_t interface_openlcb_protocol_snip = {

    .config_memory_read = nullptr,

};

interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{

    load_datagram_ok_message_called = false;
    load_datagram_rejected_message_called = false;

    datagram_reply_code = 0;
    called_function_ptr = nullptr;
    local_config_mem_write_request_info.bytes = 0;
    local_config_mem_write_request_info.data_start = 0;
    local_config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    local_config_mem_write_request_info.address = 0x00;
    local_config_mem_write_request_info.write_space_func = nullptr;
    local_config_mem_write_request_info.space_info = nullptr;
    memory_write_return_zero = false;
    memory_write_requested_bytes = 0;
}

void _global_initialize(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_nulls(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler_with_nulls);
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_config_memory_write_defined(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler_config_memory_write_defined);
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_config_memory_write_and_delayed_reply_time_defined(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler_config_memory_write_and_delayed_reply_time_defined);
    OpenLcbNode_initialize(&interface_openlcb_node);
    ProtocolSnip_initialize(&interface_openlcb_protocol_snip);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolConfigMemWriteHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolConfigMemWriteHandler, initialize_with_nulls)
{

    _reset_variables();
    _global_initialize_with_nulls();
}

TEST(ProtocolConfigMemWriteHandler, initialize_with_config_memory_write_defined)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_defined();
}

TEST(ProtocolConfigMemWriteHandler, initialize_with_config_memory_write_and_delayed_reply_time_defined)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_and_delayed_reply_time_defined();
}

TEST(ProtocolConfigMemWriteHandler, memory_write_space_config_mem_bad_size_parameter)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO;
    incoming_msg->payload_count = 64 + 7 + 1;  // Invalid number of bytes to read
 
    // *****************************************
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    // *****************************************
    _reset_variables();
    incoming_msg->payload_count = 7; // Invalid number of bytes to read (zero bytes)

    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_INVALID_ARGUMENTS);

    // *****************************************
    _reset_variables();
    *incoming_msg->payload[7] = 64;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, node1->parameters->address_space_configuration_definition.highest_address + 1, 2);

    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);

    // *****************************************
    _reset_variables();
    _global_initialize();

    node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_all_not_present);
    node1->alias = DEST_ALIAS;

    incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ALL;
    *incoming_msg->payload[7] = 64 + 1; // Invalid number of bytes to read
    incoming_msg->payload_count = 8;

    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);

    // *****************************************
    _reset_variables();
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO;
    *incoming_msg->payload[7] = 64;

    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_WRITE_TO_READ_ONLY);
}

TEST(ProtocolConfigMemWriteHandler, memory_write_spaces)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO;
    incoming_msg->payload_count = 64 + 7;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_configuration_definition);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ALL;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_all);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_all);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, CONFIG_MEM_ALL_HIGH_MEMORY + 1); // Check that the bytes to read were clipped to the max address of the space + 1
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_all);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_config_memory);
    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_config_memory);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_config_memory);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_acdi_manufacturer(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_acdi_manufacturer);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_acdi_manufacturer);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_acdi_manufacturer);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_acdi_user(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_acdi_user(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_acdi_user);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_acdi_user);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_acdi_user);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_traction_function_definition_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_traction_function_definition_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_traction_config_decscription_info);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_traction_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_traction_function_definition_info);

    // *****************************************

    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_traction_function_definition_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_traction_function_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_traction_config_memory);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_traction_config_memory);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_traction_function_config_memory);
}

TEST(ProtocolConfigMemWriteHandler, memory_write_spaces_delayed)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_and_delayed_reply_time_defined();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO;
    incoming_msg->payload_count = 64 + 7;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 16000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_6);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_configuration_definition);
}

TEST(ProtocolConfigMemWriteHandler, memory_write_space_config_description_short_form)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    incoming_msg->payload_count = 64 + 6;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_description_info(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_config_decscription_info);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_1);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_configuration_definition);

    // *****************************************

    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_FE;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_all);

    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_all);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, CONFIG_MEM_ALL_HIGH_MEMORY + 1); // Check that the bytes to read were clipped to the max address of the space + 1
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_1);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_all);

    // *****************************************

    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_FD;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_received_ok_message);
    EXPECT_EQ(datagram_reply_code, 0x0000);

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_config_memory(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_write_request_config_memory);
    EXPECT_EQ(local_config_mem_write_request_info.write_space_func, &_write_request_config_memory);
    EXPECT_EQ(local_config_mem_write_request_info.bytes, 64);
    EXPECT_EQ(local_config_mem_write_request_info.encoding, ADDRESS_SPACE_IN_BYTE_1);
    EXPECT_EQ(local_config_mem_write_request_info.address, 0x0000);
    EXPECT_EQ(local_config_mem_write_request_info.space_info, &_node_parameters_main_node.address_space_config_memory);
}

TEST(ProtocolConfigMemWriteHandler, memory_read_spaces_all_space_not_present)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_all_not_present);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ALL;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
}

TEST(ProtocolConfigMemWriteHandler, message_reply_handlers)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_all_not_present);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ALL;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_message(&statemachine_info, CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO, 0x0000, 0x0000);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_reply_ok_message(&statemachine_info, CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_reply_fail_message(&statemachine_info, CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO);
}

TEST(ProtocolConfigMemWriteHandler, message_handlers_null)
{

    _reset_variables();
    _global_initialize_with_nulls();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node_all_not_present);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ALL;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // *****************************************
    _reset_variables();
    ProtocolConfigMemWriteHandler_write_space_all(&statemachine_info);

    EXPECT_EQ(called_function_ptr, (void *)&_load_datagram_rejected_message);
    EXPECT_EQ(datagram_reply_code, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
}

TEST(ProtocolConfigMemWriteHandler, write_request_config_mem)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    config_mem_write_request_info_t config_mem_write_request_info;

    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY;
    *incoming_msg->payload[7] = 0x10;
    incoming_msg->payload_count = 8;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 0x10;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_OK_SPACE_IN_BYTE_6);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 0x17);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = 0x10;
    incoming_msg->payload_count = 7;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 0x10;
    config_mem_write_request_info.data_start = 6;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_OK_SPACE_FD);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 0x16);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

TEST(ProtocolConfigMemWriteHandler, write_request_config_mem_with_configmem_write_defined)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_defined();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    config_mem_write_request_info_t config_mem_write_request_info;

    // ************************************************************************
    //
    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY;
    *incoming_msg->payload[7] = 0x10;
    incoming_msg->payload_count = 8;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 0x10;
    config_mem_write_request_info.data_start = 8;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    memory_write_return_zero = true;
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(called_function_ptr, (void *)&_config_memory_write);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = 0x10;
    incoming_msg->payload_count = 7;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 16;
    config_mem_write_request_info.data_start = 6;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    memory_write_return_zero = false;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(called_function_ptr, (void *)&_config_memory_write);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_OK_SPACE_FD);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6 + 16);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

TEST(ProtocolConfigMemWriteHandler, write_request_config_mem_with_configmem_write_defined_short_form)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_defined();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    config_mem_write_request_info_t config_mem_write_request_info;

    // ************************************************************************
    //
    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = 0x10;
    incoming_msg->payload_count = 7;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 16;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = 0x10;
    incoming_msg->payload_count = 7;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_1;
    config_mem_write_request_info.address = 0x00000000;
    config_mem_write_request_info.bytes = 16;
    config_mem_write_request_info.data_start = 6;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    memory_write_return_zero = false;

    _reset_variables();
    memory_write_return_zero = true;
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(called_function_ptr, (void *)&_config_memory_write);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_FD);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 6), ERROR_TEMPORARY_TRANSFER_ERROR);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 6 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

TEST(ProtocolConfigMemWriteHandler, write_request_acdi_user)
{

    _reset_variables();
    _global_initialize_with_config_memory_write_defined();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    config_mem_write_request_info_t config_mem_write_request_info;

    // ************************************************************************
    // Valid write of Name
    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'N';
    *incoming_msg->payload[8] = 'a';
    *incoming_msg->payload[9] = 'm';
    *incoming_msg->payload[10] = 'e';
    *incoming_msg->payload[11] = 0x00;
    incoming_msg->payload_count = 12;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_NAME_ADDRESS;
    config_mem_write_request_info.bytes = 5;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);

  //  EXPECT_EQ(called_function_ptr, (void *)&_snip_user_name_write);
    // memory_write_requested_bytes = 5;
    // for (int i = 0; i < memory_write_requested_bytes; i++)
    // {
    //     EXPECT_EQ(memory_write_data[i], *incoming_msg->payload[i + 7]);
    // }
    // EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // Valid write of Description
    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'D';
    *incoming_msg->payload[8] = 'e';
    *incoming_msg->payload[9] = 's';
    *incoming_msg->payload[10] = 'c';
    *incoming_msg->payload[11] = 'r';
    *incoming_msg->payload[12] = 'i';
    *incoming_msg->payload[13] = 'p';
    *incoming_msg->payload[14] = 't';
    *incoming_msg->payload[15] = 'i';
    *incoming_msg->payload[16] = 'o';
    *incoming_msg->payload[17] = 'n';
    *incoming_msg->payload[18] = 0x00;
    incoming_msg->payload_count = 19;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS;
    config_mem_write_request_info.bytes = 12;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);

  //  EXPECT_EQ(called_function_ptr, (void *)&_snip_user_description_write);
    // memory_write_requested_bytes = 12;
    // for (int i = 0; i < memory_write_requested_bytes; i++)
    // {
    //     EXPECT_EQ(memory_write_data[i], *incoming_msg->payload[i + 7]);
    // }
    // EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // Failed write of Name
    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'N';
    *incoming_msg->payload[8] = 'a';
    *incoming_msg->payload[9] = 'm';
    *incoming_msg->payload[10] = 'e';
    *incoming_msg->payload[11] = 0x00;
    incoming_msg->payload_count = 12;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_NAME_ADDRESS;
    config_mem_write_request_info.bytes = 5;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    memory_write_return_zero = true;
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);
    memory_write_return_zero = false;

  //  EXPECT_EQ(called_function_ptr, (void *)&_snip_user_name_write);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), CONFIG_MEM_ACDI_USER_NAME_ADDRESS);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_ACDI_USER_ACCESS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_TEMPORARY_TRANSFER_ERROR);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // InValid write of Description
    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'D';
    *incoming_msg->payload[8] = 'e';
    *incoming_msg->payload[9] = 's';
    *incoming_msg->payload[10] = 'c';
    *incoming_msg->payload[11] = 'r';
    *incoming_msg->payload[12] = 'i';
    *incoming_msg->payload[13] = 'p';
    *incoming_msg->payload[14] = 't';
    *incoming_msg->payload[15] = 'i';
    *incoming_msg->payload[16] = 'o';
    *incoming_msg->payload[17] = 'n';
    *incoming_msg->payload[18] = 0x00;
    incoming_msg->payload_count = 19;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS;
    config_mem_write_request_info.bytes = 12;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    memory_write_return_zero = true;
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);
    memory_write_return_zero = false;

  //  EXPECT_EQ(called_function_ptr, (void *)&_snip_user_description_write);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_ACDI_USER_ACCESS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_TEMPORARY_TRANSFER_ERROR);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // Bad Requested Address Sent
    // ************************************************************************

    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS + 1, 2); // Wrong address should do nothing
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'D';
    *incoming_msg->payload[8] = 'e';
    *incoming_msg->payload[9] = 's';
    *incoming_msg->payload[10] = 'c';
    *incoming_msg->payload[11] = 'r';
    *incoming_msg->payload[12] = 'i';
    *incoming_msg->payload[13] = 'p';
    *incoming_msg->payload[14] = 't';
    *incoming_msg->payload[15] = 'i';
    *incoming_msg->payload[16] = 'o';
    *incoming_msg->payload[17] = 'n';
    *incoming_msg->payload[18] = 0x00;
    incoming_msg->payload_count = 19;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS + 1;
    config_mem_write_request_info.bytes = 12;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS + 1);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_ACDI_USER_ACCESS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}

TEST(ProtocolConfigMemWriteHandler, _memory_write_request_equals_null)
{

    _reset_variables();
    _global_initialize_with_nulls();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;
    config_mem_write_request_info_t config_mem_write_request_info;

    // ************************************************************************
    // // Write of ACCI Name Write Dependancy Not defined
    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_NAME_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'N';
    *incoming_msg->payload[8] = 'a';
    *incoming_msg->payload[9] = 'm';
    *incoming_msg->payload[10] = 'e';
    *incoming_msg->payload[11] = 0x00;
    incoming_msg->payload_count = 12;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_NAME_ADDRESS;
    config_mem_write_request_info.bytes = 5;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), CONFIG_MEM_ACDI_USER_NAME_ADDRESS);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_ACDI_USER_ACCESS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_PERMANENT_INVALID_ARGUMENTS);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // Write of ACCI Description Write Dependancy Not defined
    // ************************************************************************
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_ACDI_USER_ACCESS;
    *incoming_msg->payload[7] = 'D';
    *incoming_msg->payload[8] = 'e';
    *incoming_msg->payload[9] = 's';
    *incoming_msg->payload[10] = 'c';
    *incoming_msg->payload[11] = 'r';
    *incoming_msg->payload[12] = 'i';
    *incoming_msg->payload[13] = 'p';
    *incoming_msg->payload[14] = 't';
    *incoming_msg->payload[15] = 'i';
    *incoming_msg->payload[16] = 'o';
    *incoming_msg->payload[17] = 'n';
    *incoming_msg->payload[18] = 0x00;
    incoming_msg->payload_count = 19;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS;
    config_mem_write_request_info.bytes = 12;
    config_mem_write_request_info.data_start = 7;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    ProtocolConfigMemWriteHandler_write_request_acdi_user(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), CONFIG_MEM_ACDI_USER_DESCRIPTION_ADDRESS);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_ACDI_USER_ACCESS);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_PERMANENT_INVALID_ARGUMENTS);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);

    // ************************************************************************
    // Config Mem Write Dependancy Not defined
    // ************************************************************************
    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    incoming_msg->mti = MTI_DATAGRAM;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = CONFIG_MEM_CONFIGURATION;
    *incoming_msg->payload[1] = CONFIG_MEM_WRITE_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x0000FFFF, 2);
    *incoming_msg->payload[6] = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY;
    *incoming_msg->payload[7] = 0x10;
    incoming_msg->payload_count = 8;

    config_mem_write_request_info.encoding = ADDRESS_SPACE_IN_BYTE_6;
    config_mem_write_request_info.address = 0x0000FFFF;
    config_mem_write_request_info.bytes = 0x10;
    config_mem_write_request_info.data_start = 8;
    config_mem_write_request_info.space_info = nullptr;
    config_mem_write_request_info.write_space_func = nullptr;

    _reset_variables();
    memory_write_return_zero = true;
    ProtocolConfigMemWriteHandler_write_request_config_mem(&statemachine_info, &config_mem_write_request_info);

    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[0], CONFIG_MEM_CONFIGURATION);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[1], CONFIG_MEM_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6);
    EXPECT_EQ(OpenLcbUtilities_extract_dword_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 2), 0x0000FFFF);
    EXPECT_EQ(*statemachine_info.outgoing_msg_info.msg_ptr->payload[6], CONFIG_MEM_SPACE_CONFIGURATION_MEMORY);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 7), ERROR_PERMANENT_INVALID_ARGUMENTS);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 7 + 2);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
}
