#include "test/main_Test.hxx"

#include "protocol_datagram_config_mem_handler.h"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

#define AUTO_CREATE_EVENT_COUNT 10
#define DEST_EVENT_ID 0x0605040302010000
#define SOURCE_ALIAS 0x222
#define SOURCE_ID 0x010203040506
#define DEST_ALIAS 0xBBB
#define DEST_ID 0x060504030201
#define SNIP_NAME_FULL "0123456789012345678901234567890123456789"
#define SNIP_MODEL "Test Model J"

bool lock_node_list_called = false;
bool unlock_node_list_called = false;

bool configuration_memory_read_called = false;
bool configuration_memory_write = false;
bool reboot_called = false;
bool configuration_memory_factory_reset = false;
bool snip_load_manufacturer_version_id_called = false;
bool snip_load_name_called = false;
bool snip_load_model_called = true;
bool snip_load_hardware_version_called = false;
bool snip_load_software_version_called = false;
bool snip_load_user_version_id_called = false;
bool snip_load_user_name_called = false;
bool snip_load_user_description_called = false;
bool on_configuration_memory_factory_reset_called = false;
bool on_config_mem_write_called = false;
bool on_config_mem_freeze_firmware_update_called = false;
bool on_config_mem_unfreeze_firmware_update_called = false;

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
    .address_space_configuration_definition.read_only = 1,
    .address_space_configuration_definition.present = 0,
    .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
    .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
    .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .address_space_configuration_definition.description = "Configuration definition info",

    // Space 0xFE
    .address_space_all.read_only = 1,
    .address_space_all.present = 0,
    .address_space_all.low_address_valid = 0, // assume the low address starts at 0
    .address_space_all.low_address = 0,       // ignored if low_address_valid is false
    .address_space_all.highest_address = 0,
    .address_space_all.address_space = ADDRESS_SPACE_ALL,
    .address_space_all.description = "All memory Info",

    // Space 0xFD
    .address_space_config_memory.read_only = 0,
    .address_space_config_memory.present = 0,
    .address_space_config_memory.low_address_valid = 0, // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,       // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 1,
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

void lock_node_list(void)
{

    lock_node_list_called = true;
}

void unlock_node_list(void)
{

    unlock_node_list_called = true;
}

interface_openlcb_node_t interface_openlcb_node = {

    .lock_node_list = &lock_node_list,
    .unlock_node_list = &unlock_node_list};

void _reset_variables(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;
}

uint16_t _configuration_memory_read(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{
    configuration_memory_read_called = true;
    return 0;
}

uint16_t _configuration_memory_write(uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer)
{
    configuration_memory_write = true;
    return 0;
}

void _reboot(void)
{
    reboot_called = true;
}

void _configuration_memory_factory_reset(void)
{
    configuration_memory_factory_reset = true;
}

uint16_t _snip_load_manufacturer_version_id(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_manufacturer_version_id_called = true;
    return 0;
}

uint16_t _snip_load_name(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_name_called = true;
    return 0;
}

uint16_t _snip_load_model(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_model_called = true;
    return 0;
}

uint16_t _snip_load_hardware_version(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_hardware_version_called = true;
    return 0;
}

uint16_t _snip_load_software_version(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_software_version_called = true;
    return 0;
}

uint16_t _snip_load_user_version_id(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_user_version_id_called = true;
    return 0;
}

uint16_t _snip_load_user_name(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_user_name_called = true;
    return 0;
}

uint16_t _snip_load_user_description(openlcb_node_t *openlcb_node, openlcb_msg_t *worker_msg, uint16_t payload_index, uint16_t requested_bytes)
{
    snip_load_user_description_called = true;
    return 0;
}

// Callback events
void _on_configuration_memory_factory_reset(void)
{
    on_configuration_memory_factory_reset_called = true;
}

void _on_config_mem_write(uint32_t address, uint16_t bytes_written, configuration_memory_buffer_t *config_mem_buffer)
{
    on_config_mem_write_called = true;
}

void _on_config_mem_freeze_firmware_update(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg)
{
    on_config_mem_freeze_firmware_update_called = true;
}

void _on_config_mem_unfreeze_firmware_update(openlcb_node_t *openlcb_node, openlcb_msg_t *openlcb_msg, openlcb_msg_t *worker_msg)
{
    on_config_mem_unfreeze_firmware_update_called = true;
}

interface_openlcb_protocol_datagram_config_mem_handler_t interface_openlcb_protocol_datagram_config_mem_handler = {

    .configuration_memory_read = &_configuration_memory_read,
    .configuration_memory_write = &_configuration_memory_write,
    .reboot = &_reboot,
    .configuration_memory_factory_reset = &_configuration_memory_factory_reset,
    .snip_load_manufacturer_version_id = &_snip_load_manufacturer_version_id,
    .snip_load_name = &_snip_load_name,
    .snip_load_model = &_snip_load_model,
    .snip_load_hardware_version = &_snip_load_hardware_version,
    .snip_load_software_version = &_snip_load_software_version,
    .snip_load_user_version_id = &_snip_load_user_version_id,
    .snip_load_user_name = &_snip_load_user_name,
    .snip_load_user_description = &_snip_load_user_description,
    // Callback events
    .on_configuration_memory_factory_reset = &_on_configuration_memory_factory_reset,
    .on_config_mem_write = &_on_config_mem_write,
    .on_config_mem_freeze_firmware_update = &_on_config_mem_freeze_firmware_update,
    .on_config_mem_unfreeze_firmware_update = &_on_config_mem_unfreeze_firmware_update};

void _global_initialize(void)
{

    ProtocolDatagramConfigMemHandler_initialize(&interface_openlcb_protocol_datagram_config_mem_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _reset_variaibles(void)
{

    lock_node_list_called = false;
    unlock_node_list_called = false;

    configuration_memory_read_called = false;
    configuration_memory_write = false;
    reboot_called = false;
    configuration_memory_factory_reset = false;
    snip_load_manufacturer_version_id_called = false;
    snip_load_name_called = false;
    snip_load_model_called = true;
    snip_load_hardware_version_called = false;
    snip_load_software_version_called = false;
    snip_load_user_version_id_called = false;
    snip_load_user_name_called = false;
    snip_load_user_description_called = false;
    on_configuration_memory_factory_reset_called = false;
    on_config_mem_write_called = false;
    on_config_mem_freeze_firmware_update_called = false;
    on_config_mem_unfreeze_firmware_update_called = false;
}

TEST(ProtocolDatagramConfigMemHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}