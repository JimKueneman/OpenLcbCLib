#include "test/main_Test.hxx"

#include "openlcb_application.h"
#include "openlcb_types.h"

#include "protocol_config_mem_read_handler.h"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"
#include "protocol_datagram_handler.h"

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

void *called_function_ptr = nullptr;

bool load_datagram_ok_message_called = false;
bool load_datagram_rejected_message_called = false;

node_parameters_t _node_parameters_main_node = {

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
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO),

    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,

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
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = true,
    .address_space_all.present = false,
    .address_space_all.low_address_valid = false, // assume the low address starts at 0
    .address_space_all.low_address = 0,           // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = true,
    .address_space_config_memory.present = true,
    .address_space_config_memory.low_address_valid = false,                            // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = false,
    .address_space_firmware.present = true,
    .address_space_firmware.low_address_valid = false, // assume the low address starts at 0
    .address_space_firmware.low_address = 0,           // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
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

    _update_called_function_ptr((void *)&_load_datagram_received_ok_message);

    ProtocolDatagramHandler_load_datagram_received_ok_message(statemachine_info, return_code);
}

void _load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code)
{

    _update_called_function_ptr((void *)&_load_datagram_rejected_message);

    ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, return_code);
}

bool _on_read_space_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_config_decscription_info);

    return true;
}

bool _on_read_space_all(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_all);

    return true;
}

bool _on_read_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_configuration_memory);

    return true;
}

bool _on_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_acdi_manufacturer);

    return true;
}

bool _on_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_acdi_user);

    return true;
}

bool _on_read_space_traction_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_traction_config_decscription_info);

    return true;
}

bool _on_read_space_traction_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    _update_called_function_ptr((void *)&_on_read_space_traction_config_memory);

    return true;
}

const interface_protocol_config_mem_read_handler_t interface_protocol_config_mem_read_handler = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    // Callbacks
    .on_read_space_config_decscription_info = NULL,
    .on_read_space_all = NULL,
    .on_read_space_configuration_memory = NULL,
    .on_read_space_acdi_manufacturer = NULL,
    .on_read_space_acdi_user = NULL,
    .on_read_space_traction_config_decscription_info = NULL,
    .on_read_space_traction_config_memory = NULL

};

const interface_protocol_config_mem_read_handler_t interface_protocol_config_mem_read_handler_with_callbacks = {

    .load_datagram_received_ok_message = &_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_load_datagram_rejected_message,

    // Callbacks
    .on_read_space_config_decscription_info = &_on_read_space_config_decscription_info,
    .on_read_space_all = &_on_read_space_all,
    .on_read_space_configuration_memory = &_on_read_space_configuration_memory,
    .on_read_space_acdi_manufacturer = &_on_read_space_acdi_manufacturer,
    .on_read_space_acdi_user = &_on_read_space_acdi_user,
    .on_read_space_traction_config_decscription_info = &_on_read_space_traction_config_decscription_info,
    .on_read_space_traction_config_memory = &_on_read_space_traction_config_memory

};

interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{

    load_datagram_ok_message_called = false;
    load_datagram_rejected_message_called = false;

    called_function_ptr = nullptr;
}

void _global_initialize(void)
{

    ProtocolConfigMemReadHandler_initialize(&interface_protocol_config_mem_read_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_callbacks(void)
{

    ProtocolConfigMemReadHandler_initialize(&interface_protocol_config_mem_read_handler_with_callbacks);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolConfigMemReadHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolConfigMemReadHandler, initialize_with_callbacks)
{

    _reset_variables();
    _global_initialize_with_callbacks();
}

TEST(ProtocolConfigMemReadHandler, memory_read_space_config_description_info)
{

    _reset_variables();
    _global_initialize();

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
    *incoming_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *incoming_msg->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_OK_REPLY);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_TRUE(statemachine_info.incoming_msg_info.enumerate);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(statemachine_info.incoming_msg_info.enumerate);
    EXPECT_EQ(outgoing_msg->payload_count, 64 + 7);
    for (int i = 0; i < 64; i++)
    {

        EXPECT_EQ(*outgoing_msg->payload[i + 7], _node_parameters_main_node.cdi[i]);
    }
}

TEST(ProtocolConfigMemReadHandler, memory_read_space_config_description_info_bad_size_parameter)
{

    _reset_variables();
    _global_initialize();

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
    *incoming_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *incoming_msg->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    *incoming_msg->payload[7] = 64 + 1; // Invalid
    incoming_msg->payload_count = 8;

    // *****************************************
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(statemachine_info.incoming_msg_info.enumerate);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_msg, 0), ERROR_PERMANENT_INVALID_ARGUMENTS);

    // *****************************************
    _reset_variables();
    *incoming_msg->payload[7] = 0; // Invalid

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(statemachine_info.incoming_msg_info.enumerate);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_msg, 0), ERROR_PERMANENT_INVALID_ARGUMENTS);

    // *****************************************
    _reset_variables();
    *incoming_msg->payload[7] = 64;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, node1->parameters->address_space_configuration_definition.highest_address + 1, 2);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(statemachine_info.incoming_msg_info.enumerate);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(outgoing_msg, 0), ERROR_PERMANENT_CONFIG_MEM_OUT_OF_BOUNDS_INVALID_ADDRESS);

    // *****************************************
    _reset_variables();
    *incoming_msg->payload[7] = 64;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, node1->parameters->address_space_configuration_definition.highest_address, 2);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM_OK_REPLY);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_TRUE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_TRUE(statemachine_info.incoming_msg_info.enumerate);

    ProtocolConfigMemReadHandler_memory_read_space_config_description_info(&statemachine_info);

    EXPECT_EQ(outgoing_msg->mti, MTI_DATAGRAM);
    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);
    EXPECT_FALSE(statemachine_info.incoming_msg_info.enumerate);
    EXPECT_EQ(outgoing_msg->payload_count, 1 + 7); // Will clip the requested 64 to just the last byte as the addres is parameters->address_space_configuration_definition.highest_address
}

TEST(ProtocolConfigMemReadHandler, memory_read_space_all)
{

    _reset_variables();
    _global_initialize();

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
    *incoming_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *incoming_msg->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = ADDRESS_SPACE_ALL;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    ProtocolConfigMemReadHandler_memory_read_space_all(&statemachine_info);

    // EXPECT_EQ(called_function_ptr, nullptr);

    _reset_variables();
    _global_initialize_with_callbacks();

    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    *incoming_msg->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *incoming_msg->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(incoming_msg, 0x00000000, 2);
    *incoming_msg->payload[6] = ADDRESS_SPACE_ALL;
    *incoming_msg->payload[7] = 64;
    incoming_msg->payload_count = 8;

    // EXPECT_FALSE(node1->state.openlcb_datagram_ack_sent);

    // ProtocolConfigMemReadHandler_memory_read_space_all(&statemachine_info);

    // EXPECT_EQ(called_function_ptr, &_on_read_space_config_decscription_info);
    // EXPECT_FALSE(statemachine_info.outgoing_msg_info.valid);
}