#include "test/main_Test.hxx"

#include "openlcb_application.h"
#include "openlcb_types.h"

#include "protocol_config_mem_write_handler.h"

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

bool load_datagram_ok_message_called = false;
bool load_datagram_rejected_message_called = false;
bool on_write_space_config_decscription_info_called = false;
bool on_write_space_all = false;
bool on_write_space_configuration_memory = false;
bool on_write_space_acdi_manufacturer = false;
bool on_write_space_acdi_user = false;
bool on_write_space_traction_config_decscription_info = false;
bool on_write_space_traction_config_memory = false;

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
    .address_space_configuration_definition.present = false,
    .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
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
    .address_space_config_memory.read_only = false,
    .address_space_config_memory.present = false,
    .address_space_config_memory.low_address_valid = false,                            // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,                                      // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = CONFIG_MEM_NODE_ADDRESS_ALLOCATION, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = false,
    .address_space_firmware.present = true,
    .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
    .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware Bootloader",

    .cdi =
        {
            // </cdi>
        },

};

void _ProtocolDatagramHandler_load_datagram_received_ok_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code)
{

    load_datagram_ok_message_called = true;

    ProtocolDatagramHandler_load_datagram_received_ok_message(statemachine_info, return_code);
}

void _ProtocolDatagramHandler_load_datagram_rejected_message(openlcb_statemachine_info_t *statemachine_info, uint16_t return_code)
{

    load_datagram_rejected_message_called = true;

    ProtocolDatagramHandler_load_datagram_rejected_message(statemachine_info, return_code);
}

bool _on_write_space_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_config_decscription_info_called = true;

    return true;
}

bool _on_write_space_all(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_all = true;

    return true;
}

bool _on_write_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_configuration_memory = true;

    return true;
}

bool _on_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_acdi_manufacturer = true;

    return true;
}

bool _on_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_acdi_user = true;

    return true;
}

bool _on_write_space_traction_config_decscription_info(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_traction_config_decscription_info = true;

    return true;
}

bool _on_write_space_traction_config_memory(openlcb_statemachine_info_t *statemachine_info, config_mem_read_request_info_t *config_mem_read_request_info)
{

    on_write_space_traction_config_memory = true;

    return true;
}

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler = {

    .load_datagram_received_ok_message = &_ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_ProtocolDatagramHandler_load_datagram_rejected_message,

    .write_request_config_definition_info = NULL,
    .write_request_all = NULL,
    .write_request_config_mem = NULL,
    .write_request_acdi_manufacturer = NULL,
    .write_request_acdi_user = NULL,
    .write_request_traction_function_config_definition_info = NULL,
    .write_request_traction_function_config_memory = NULL

};

const interface_protocol_config_mem_write_handler_t interface_protocol_config_mem_write_handler_with_callbacks = {

    .load_datagram_received_ok_message = &_ProtocolDatagramHandler_load_datagram_received_ok_message,
    .load_datagram_received_rejected_message = &_ProtocolDatagramHandler_load_datagram_rejected_message,

    // .write_request_configuration_definition_info = &_on_write_space_config_decscription_info,
    // .write_request_all = &_on_write_space_all,
    // .write_request_config_mem = &_on_write_space_configuration_memory,
    // .write_request_acdi_manufacturer = &_on_write_space_acdi_manufacturer,
    // .write_request_acdi_user =  &_on_write_space_acdi_user,
    // .write_request_traction_function_configuration_definition_info = &_on_write_space_traction_config_decscription_info,
    // .write_request_traction_function_configuration_memory = &_on_write_space_traction_config_memory

};

interface_openlcb_node_t interface_openlcb_node = {};

void _reset_variables(void)
{

    load_datagram_ok_message_called = false;
    load_datagram_rejected_message_called = false;
}

void _global_initialize(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_callbacks(void)
{

    ProtocolConfigMemWriteHandler_initialize(&interface_protocol_config_mem_write_handler_with_callbacks);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

TEST(ProtocolConfigMemWriteHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolConfigMemWriteHandler, initialize_with_callbacks)
{

    _reset_variables();
    _global_initialize_with_callbacks();
}