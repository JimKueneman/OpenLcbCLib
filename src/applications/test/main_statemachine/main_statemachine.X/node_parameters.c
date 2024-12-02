/*
 * File:   node_definition.c
 * Author: jimkueneman
 *
 * Created on November 21, 2024, 11:14 AM
 */


#include "xc.h"
#include "node_parameters.h"
#include "../../../../openlcb/openlcb_defines.h"

const node_parameters_t NodeParameters_main_node = {

    .consumer_count = 4,
    .producer_count = 4,
    .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
    .snip.name = "Mustangpeak",
    .snip.model = "VT100",
    .snip.hardware_version = "1.0",
    .snip.software_version = "0.2",
    .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same
    .protocol_support = (PSI_DATAGRAM | PSI_MEMORY_CONFIGURATION | PSI_EVENT_EXCHANGE | PSI_EVENT_EXCHANGE | PSI_ABBREVIATED_DEFAULT_CDI | PSI_SIMPLE_NODE_INFORMATION | PSI_CONFIGURATION_DESCRIPTION_INFO | PSI_TRAIN_CONTROL | PSI_FUNCTION_DESCRIPTION | PSI_FUNCTION_CONFIGURATION),


    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY,
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
    .address_space_configuration_definition.low_address = 0, // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 1675, // this is the length of the .cdi file byte array in this initialization structure
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0, // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",


    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 1,
    .address_space_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_config_memory.low_address = 0, // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200,
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",


    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = 1,
    .address_space_acdi_manufacturer.present = 1,
    .address_space_acdi_manufacturer.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 125, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_manufacturer.address_space = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ACDI access manufacturer",


    // Space 0xFB
    .address_space_acdi_user.read_only = 0,
    .address_space_acdi_user.present = 1,
    .address_space_acdi_user.low_address_valid = 0, // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0, // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 128, // Predefined in the Configuration Description Definition Spec
    .address_space_acdi_user.address_space = ADDRESS_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ACDI access user storage",


    // Space 0xFA
    .address_space_train_function_definition.read_only = 1,
    .address_space_train_function_definition.present = 1,
    .address_space_train_function_definition.low_address_valid = 0, // assume the low address starts at 0
    .address_space_train_function_definition.low_address = 0, // ignored if low_address_valid is false
    .address_space_train_function_definition.highest_address = 808, // Length of the FDI file in this structure
    .address_space_train_function_definition.address_space = ADDRESS_SPACE_TRAIN_FUNCTION_DEFINITION_INFO,
    .address_space_train_function_definition.description = "Train function definition info",


    // Space 0xF9
    .address_space_train_function_config_memory.read_only = 0,
    .address_space_train_function_config_memory.present = 1,
    .address_space_train_function_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_train_function_config_memory.low_address = 0x100, // ignored if low_address_valid is false
    .address_space_train_function_config_memory.highest_address = 0x200,
    .address_space_train_function_config_memory.address_space = ADDRESS_SPACE_TRAIN_FUNCTION_CONFIGURATION_MEMORY,
    .address_space_train_function_config_memory.description = "Train function configuration memory storage",

    .cdi =
    {
        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x31, 0x2E, 0x30, 0x22, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x22, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x22, 0x3F, 0x3E, // <?xml version="1.0" encoding="UTF-8"?>
        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x22, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x22, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x74, 0x72, 0x75, 0x6E, 0x6B, 0x2F, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2F, 0x78, 0x6D, 0x6C, 0x2F, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x22, 0x3F, 0x3E, // <?xml-stylesheet type="text/xsl" href="http://openlcb.org/trunk/prototypes/xml/xslt/cdi.xsl"?>
        0x3C, 0x63, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x22, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x22, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x63, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x33, 0x2F, 0x63, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x22, 0x3E, // <cdi xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://openlcb.org/schema/cdi/1/3/cdi.xsd">
        0x3C, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <identification>
        0x3C, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x3C, 0x2F, 0x6D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x3E, // <manufacturer>Mustangpeak</manufacturer>
        0x3C, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E, 0x56, 0x54, 0x31, 0x30, 0x30, 0x3C, 0x2F, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x3E, // <model>VT100</model>
        0x3C, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x31, 0x2E, 0x30, 0x3C, 0x2F, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, // <hardwareVersion>1.0</hardwareVersion>
        0x3C, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, 0x30, 0x2E, 0x32, 0x3C, 0x2F, 0x73, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3E, // <softwareVersion>0.2</softwareVersion>
        0x3C, 0x6D, 0x61, 0x70, 0x3E, // <map>
        0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <relation>
        0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, // <property>Description</property>
        0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x45, 0x78, 0x6F, 0x6D, 0x70, 0x6C, 0x65, 0x20, 0x52, 0x61, 0x73, 0x70, 0x62, 0x65, 0x72, 0x72, 0x79, 0x20, 0x50, 0x69, 0x20, 0x4E, 0x6F, 0x64, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, // <value>Mustangpeak Exomple Raspberry Pi Node</value>
        0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </relation>
        0x3C, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <relation>
        0x3C, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x3C, 0x2F, 0x70, 0x72, 0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x3E, // <property>Status</property>
        0x3C, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, 0x50, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x3C, 0x2F, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x3E, // <value>Prototype</value>
        0x3C, 0x2F, 0x72, 0x65, 0x6C, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </relation>
        0x3C, 0x2F, 0x6D, 0x61, 0x70, 0x3E, // </map>
        0x3C, 0x2F, 0x69, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </identification>
        0x3C, 0x61, 0x63, 0x64, 0x69, 0x20, 0x66, 0x69, 0x78, 0x65, 0x64, 0x3D, 0x22, 0x31, 0x22, 0x20, 0x76, 0x61, 0x72, 0x3D, 0x22, 0x31, 0x22, 0x2F, 0x3E, // <acdi fixed="1" var="1"/>
        0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x33, 0x22, 0x3E, // <segment origin="0" space="253">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4F, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x73, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Options</name>
        0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x4F, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x73, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4D, 0x75, 0x73, 0x74, 0x61, 0x6E, 0x67, 0x70, 0x65, 0x61, 0x6B, 0x20, 0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <description>Options for the Mustangpeak Example</description>
        0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x20, 0x6F, 0x66, 0x66, 0x73, 0x65, 0x74, 0x3D, 0x22, 0x31, 0x22, 0x3E, // <group offset="1">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x61, 0x74, 0x61, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Data</name>
        0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x41, 0x64, 0x64, 0x20, 0x79, 0x6F, 0x75, 0x72, 0x20, 0x6F, 0x77, 0x6E, 0x20, 0x75, 0x6E, 0x69, 0x71, 0x75, 0x65, 0x20, 0x6E, 0x6F, 0x64, 0x65, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x20, 0x68, 0x65, 0x72, 0x65, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <description>Add your own unique node info here</description>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x33, 0x22, 0x3E, // <string size="63">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Name</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x34, 0x22, 0x3E, // <string size="64">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Description</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E, // </group>
        0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E, // </segment>
        0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x32, 0x22, 0x3E, // <segment origin="0" space="252">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x72, 0x6D, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Manufacturer Information</name>
        0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x4D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x2D, 0x70, 0x72, 0x6F, 0x76, 0x69, 0x64, 0x65, 0x64, 0x20, 0x66, 0x69, 0x78, 0x65, 0x64, 0x20, 0x6E, 0x6F, 0x64, 0x65, 0x20, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <description>Manufacturer-provided fixed node description</description>
        0x3C, 0x69, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x31, 0x22, 0x3E, // <int size="1">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Version</name>
        0x3C, 0x2F, 0x69, 0x6E, 0x74, 0x3E, // </int>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x31, 0x32, 0x22, 0x3E, // <string size="12">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Manufacturer Name</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x22, 0x3E, // <string size="6">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x4D, 0x61, 0x6E, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x20, 0x49, 0x6E, 0x66, 0x6F, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Manufacturer Info</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x34, 0x22, 0x3E, // <string size="4">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x48, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x20, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Hardware Version</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x34, 0x22, 0x3E, // <string size="4">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Software Version</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E, // </segment>
        0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x6F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0x30, 0x22, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x22, 0x32, 0x35, 0x31, 0x22, 0x3E, // <segment origin="0" space="251">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x61, 0x74, 0x61, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Data</name>
        0x3C, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, 0x41, 0x64, 0x64, 0x20, 0x79, 0x6F, 0x75, 0x72, 0x20, 0x6F, 0x77, 0x6E, 0x20, 0x75, 0x6E, 0x69, 0x71, 0x75, 0x65, 0x20, 0x6E, 0x6F, 0x64, 0x65, 0x20, 0x69, 0x6E, 0x66, 0x6F, 0x20, 0x68, 0x65, 0x72, 0x65, 0x3C, 0x2F, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <description>Add your own unique node info here</description>
        0x3C, 0x69, 0x6E, 0x74, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x31, 0x22, 0x3E, // <int size="1">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Version</name>
        0x3C, 0x2F, 0x69, 0x6E, 0x74, 0x3E, // </int>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x33, 0x22, 0x3E, // <string size="63">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x4E, 0x61, 0x6D, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Name</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x22, 0x36, 0x34, 0x22, 0x3E, // <string size="64">
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x55, 0x73, 0x65, 0x72, 0x20, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>User Description</name>
        0x3C, 0x2F, 0x73, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x3E, // </string>
        0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E, // </segment>
        0x3C, 0x2F, 0x63, 0x64, 0x69, 0x3E, 0x00 // </cdi>


    },

    .fdi =
    {

        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x31, 0x2E, 0x30, 0x27, 0x20, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3D, 0x27, 0x55, 0x54, 0x46, 0x2D, 0x38, 0x27, 0x3F, 0x3E, // <?xml version='1.0' encoding='UTF-8'?>
        0x3C, 0x3F, 0x78, 0x6D, 0x6C, 0x2D, 0x73, 0x74, 0x79, 0x6C, 0x65, 0x73, 0x68, 0x65, 0x65, 0x74, 0x20, 0x74, 0x79, 0x70, 0x65, 0x3D, 0x27, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x78, 0x73, 0x6C, 0x27, 0x20, 0x68, 0x72, 0x65, 0x66, 0x3D, 0x27, 0x78, 0x73, 0x6C, 0x74, 0x2F, 0x66, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x6C, 0x27, 0x3F, 0x3E, // <?xml-stylesheet type='text/xsl' href='xslt/fdi.xsl'?>
        0x3C, 0x66, 0x64, 0x69, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x73, 0x69, 0x3D, 0x27, 0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x31, 0x2F, 0x58, 0x4D, 0x4C, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2D, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x27, 0x20, 0x78, 0x73, 0x69, 0x3A, 0x6E, 0x6F, 0x4E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65, 0x53, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x4C, 0x6F, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x3D, 0x27, 0x20, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3A, 0x2F, 0x2F, 0x6F, 0x70, 0x65, 0x6E, 0x6C, 0x63, 0x62, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x73, 0x63, 0x68, 0x65, 0x6D, 0x61, 0x2F, 0x66, 0x64, 0x69, 0x2F, 0x31, 0x2F, 0x30, 0x2F, 0x66, 0x64, 0x69, 0x2E, 0x78, 0x73, 0x64, 0x27, 0x3E, // <fdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation=' https://openlcb.org/schema/fdi/1/0/fdi.xsd'>
        0x3C, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x20, 0x73, 0x70, 0x61, 0x63, 0x65, 0x3D, 0x27, 0x32, 0x34, 0x39, 0x27, 0x3E, 0x3C, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x2F, 0x3E, // <segment space='249'><group><name/>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='binary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x48, 0x65, 0x61, 0x64, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Headlight</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x30, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>0</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='binary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x65, 0x6C, 0x6C, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Bell</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>1</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x6D, 0x6F, 0x6D, 0x65, 0x6E, 0x74, 0x61, 0x72, 0x79, 0x27, 0x3E, // <function size='1' kind='momentary'>
        0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x48, 0x6F, 0x72, 0x6E, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <name>Horn</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x32, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, // <number>2</number>
        0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // </function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x44, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x6C, 0x20, 0x46, 0x30, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Directional F0</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x30, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>100</number></function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x6C, 0x61, 0x6E, 0x6B, 0x20, 0x46, 0x30, 0x20, 0x46, 0x6F, 0x72, 0x77, 0x61, 0x72, 0x64, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Blank F0 Forward</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x31, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>101</number></function>
        0x3C, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x73, 0x69, 0x7A, 0x65, 0x3D, 0x27, 0x31, 0x27, 0x20, 0x6B, 0x69, 0x6E, 0x64, 0x3D, 0x27, 0x62, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x27, 0x3E, 0x3C, 0x6E, 0x61, 0x6D, 0x65, 0x3E, 0x42, 0x6C, 0x61, 0x6E, 0x6B, 0x20, 0x46, 0x30, 0x20, 0x52, 0x65, 0x76, 0x65, 0x72, 0x73, 0x65, 0x3C, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x3E, // <function size='1' kind='binary'><name>Blank F0 Reverse</name>
        0x3C, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x31, 0x30, 0x32, 0x3C, 0x2F, 0x6E, 0x75, 0x6D, 0x62, 0x65, 0x72, 0x3E, 0x3C, 0x2F, 0x66, 0x75, 0x6E, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3E, // <number>102</number></function>
        0x3C, 0x2F, 0x67, 0x72, 0x6F, 0x75, 0x70, 0x3E, 0x3C, 0x2F, 0x73, 0x65, 0x67, 0x6D, 0x65, 0x6E, 0x74, 0x3E, 0x3C, 0x2F, 0x66, 0x64, 0x69, 0x3E // </group></segment></fdi>

    }

};
