#include "test/main_Test.hxx"

#include "protocol_datagram_handler.h"

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
#define CONFIG_MEM_ADDRESS 0x000000100

void *called_function_ptr = nullptr;
bool lock_shared_resources_called = false;
bool unlock_shared_resources_called = false;

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

interface_openlcb_node_t interface_openlcb_node = {};

void _update_called_function_ptr(void *function_ptr)
{
    called_function_ptr = (void *)((long long)function_ptr + (long long)called_function_ptr);
}

void _memory_read_space_config_description_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_config_description_info);
}

void _memory_read_space_all(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_all);
}

void _memory_read_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_configuration_memory);
}

void _memory_read_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_acdi_manufacturer);
}

void _memory_read_space_acdi_user(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_acdi_user);
}

void _memory_read_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_traction_function_definition_info);
}

void _memory_read_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_space_traction_function_config_memory);
}

void _memory_read_space_config_description_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_config_description_info_reply_ok);
}

void _memory_read_space_all_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_all_reply_ok);
}

void _memory_read_space_configuration_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_configuration_memory_reply_ok);
}

void _memory_read_space_acdi_manufacturer_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_acdi_manufacturer_reply_ok);
}

void _memory_read_space_acdi_user_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_acdi_user_reply_ok);
}

void _memory_read_space_traction_function_definition_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_traction_function_definition_info_reply_ok);
}

void _memory_read_space_traction_function_config_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_traction_function_config_memory_reply_ok);
}

void _memory_read_space_config_description_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_config_description_info_reply_fail);
}

void _memory_read_space_all_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_all_reply_fail);
}

void _memory_read_space_configuration_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_configuration_memory_reply_fail);
}

void _memory_read_space_acdi_manufacturer_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_acdi_manufacturer_reply_fail);
}

void _memory_read_space_acdi_user_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_acdi_user_reply_fail);
}

void _memory_read_space_traction_function_definition_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_traction_function_definition_info_reply_fail);
}

void _memory_read_space_traction_function_config_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_space_traction_function_config_memory_reply_fail);
}

void _memory_read_stream_space_config_description_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_config_description_info);
}

void _memory_read_stream_space_all(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_all);
}

void _memory_read_stream_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_configuration_memory);
}

void _memory_read_stream_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_manufacturer);
}

void _memory_read_stream_space_acdi_user(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_user);
}

void _memory_read_stream_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_definition_info);
}

void _memory_read_stream_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_config_memory);
}

void _memory_read_stream_space_config_description_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_config_description_info_reply_ok);
}

void _memory_read_stream_space_all_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_all_reply_ok);
}

void _memory_read_stream_space_configuration_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_configuration_memory_reply_ok);
}

void _memory_read_stream_space_acdi_manufacturer_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_manufacturer_reply_ok);
}

void _memory_read_stream_space_acdi_user_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_user_reply_ok);
}

void _memory_read_stream_space_traction_function_definition_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_definition_info_reply_ok);
}

void _memory_read_stream_space_traction_function_config_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_config_memory_reply_ok);
}

void _memory_read_stream_space_config_description_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_config_description_info_reply_fail);
}

void _memory_read_stream_space_all_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_all_reply_fail);
}

void _memory_read_stream_space_configuration_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_configuration_memory_reply_fail);
}

void _memory_read_stream_space_acdi_manufacturer_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_manufacturer_reply_fail);
}

void _memory_read_stream_space_acdi_user_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_acdi_user_reply_fail);
}

void _memory_read_stream_space_traction_function_definition_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_definition_info_reply_fail);
}

void _memory_read_stream_space_traction_function_config_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_read_stream_space_traction_function_config_memory_reply_fail);
}

void _memory_write_space_config_description_info(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_config_description_info);
}

void _memory_write_space_all(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_all);
}

void _memory_write_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_configuration_memory);
}

void _memory_write_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_manufacturer);
}

void _memory_write_space_acdi_user(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_user);
}

void _memory_write_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_definition_info);
}

void _memory_write_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_config_memory);
}

void _memory_write_space_firmware_upgrade(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_firmware_upgrade);
}

void _memory_write_space_config_description_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_config_description_info_reply_ok);
}

void _memory_write_space_all_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_all_reply_ok);
}

void _memory_write_space_configuration_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_configuration_memory_reply_ok);
}

void _memory_write_space_acdi_manufacturer_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_manufacturer_reply_ok);
}

void _memory_write_space_acdi_user_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_user_reply_ok);
}

void _memory_write_space_traction_function_definition_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_definition_info_reply_ok);
}

void _memory_write_space_traction_function_config_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_config_memory_reply_ok);
}

void _memory_write_space_config_description_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_config_description_info_reply_fail);
}

void _memory_write_space_all_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_all_reply_fail);
}

void _memory_write_space_configuration_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_configuration_memory_reply_fail);
}

void _memory_write_space_acdi_manufacturer_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_manufacturer_reply_fail);
}

void _memory_write_space_acdi_user_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_acdi_user_reply_fail);
}

void _memory_write_space_traction_function_definition_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_definition_info_reply_fail);
}

void _memory_write_space_traction_function_config_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_space_traction_function_config_memory_reply_fail);
}

void _memory_write_under_mask_space_config_description_info(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_config_description_info);
}

void _memory_write_under_mask_space_all(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_all);
}

void _memory_write_under_mask_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_configuration_memory);
}

void _memory_write_under_mask_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_acdi_manufacturer);
}

void _memory_write_under_mask_space_acdi_user(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_acdi_user);
}

void _memory_write_under_mask_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_traction_function_definition_info);
}

void _memory_write_under_mask_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_traction_function_config_memory);
}

void _memory_write_under_mask_space_firmware_upgrade(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_under_mask_space_firmware_upgrade);
}

void _memory_write_stream_space_config_description_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_config_description_info);
}

void _memory_write_stream_space_all(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_all);
}

void _memory_write_stream_space_configuration_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_configuration_memory);
}

void _memory_write_stream_space_acdi_manufacturer(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_manufacturer);
}

void _memory_write_stream_space_acdi_user(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_user);
}

void _memory_write_stream_space_traction_function_definition_info(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_definition_info);
}

void _memory_write_stream_space_traction_function_config_memory(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_config_memory);
}

void _memory_write_stream_space_firmware_upgrade(openlcb_statemachine_info_t *statemachine_info)
{

    _update_called_function_ptr((void *)&_memory_write_stream_space_firmware_upgrade);
}

void _memory_write_stream_space_config_description_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_config_description_info_reply_ok);
}

void _memory_write_stream_space_all_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_all_reply_ok);
}

void _memory_write_stream_space_configuration_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_configuration_memory_reply_ok);
}

void _memory_write_stream_space_acdi_manufacturer_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_manufacturer_reply_ok);
}

void _memory_write_stream_space_acdi_user_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_user_reply_ok);
}

void _memory_write_stream_space_traction_function_definition_info_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_definition_info_reply_ok);
}

void _memory_write_stream_space_traction_function_config_memory_reply_ok(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_config_memory_reply_ok);
}

void _memory_write_stream_space_config_description_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_config_description_info_reply_fail);
}

void _memory_write_stream_space_all_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_all_reply_fail);
}

void _memory_write_stream_space_configuration_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_configuration_memory_reply_fail);
}

void _memory_write_stream_space_acdi_manufacturer_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_manufacturer_reply_fail);
}

void _memory_write_stream_space_acdi_user_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_acdi_user_reply_fail);
}

void _memory_write_stream_space_traction_function_definition_info_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_definition_info_reply_fail);
}

void _memory_write_stream_space_traction_function_config_memory_reply_fail(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_write_stream_space_traction_function_config_memory_reply_fail);
}

void _memory_options_cmd(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_options_cmd);
}

void _memory_options_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_options_reply);
}

void _memory_get_address_space_info_cmd(openlcb_statemachine_info_t *statemachine_infog)
{
    _update_called_function_ptr((void *)&_memory_get_address_space_info_cmd);
}

void _memory_get_address_space_info_reply_not_present(openlcb_statemachine_info_t *statemachine_infog)
{
    _update_called_function_ptr((void *)&_memory_get_address_space_info_reply_not_present);
}

void _memory_get_address_space_info_reply_present(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_get_address_space_info_reply_present);
}

void _memory_reserve_lock(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_reserve_lock);
}

void _memory_reserve_lock_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_reserve_lock_reply);
}

void _memory_get_unique_id(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_get_unique_id);
}

void _memory_get_unique_id_reply(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_get_unique_id_reply);
}

void _memory_unfreeze(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_unfreeze);
}

void _memory_freeze(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_freeze);
}

void _memory_update_complete(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_update_complete);
}

void _memory_reset_reboot(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_reset_reboot);
}

void _memory_factory_reset(openlcb_statemachine_info_t *statemachine_info)
{
    _update_called_function_ptr((void *)&_memory_factory_reset);
}

void _lock_shared_resources(void)
{

    lock_shared_resources_called = true;
}

void _unlock_shared_resources(void)
{
    unlock_shared_resources_called = true;
}

interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {

    // Config Memory Read
    .memory_read_space_config_description_info = &_memory_read_space_config_description_info,
    .memory_read_space_all = &_memory_read_space_all,
    .memory_read_space_configuration_memory = &_memory_read_space_configuration_memory,
    .memory_read_space_acdi_manufacturer = &_memory_read_space_acdi_manufacturer,
    .memory_read_space_acdi_user = &_memory_read_space_acdi_user,
    .memory_read_space_traction_function_definition_info = &_memory_read_space_traction_function_definition_info,
    .memory_read_space_traction_function_config_memory = &_memory_read_space_traction_function_config_memory,

    // Config Memory Read Reply Ok
    .memory_read_space_config_description_info_reply_ok = &_memory_read_space_config_description_info_reply_ok,
    .memory_read_space_all_reply_ok = &_memory_read_space_all_reply_ok,
    .memory_read_space_configuration_memory_reply_ok = &_memory_read_space_configuration_memory_reply_ok,
    .memory_read_space_acdi_manufacturer_reply_ok = &_memory_read_space_acdi_manufacturer_reply_ok,
    .memory_read_space_acdi_user_reply_ok = &_memory_read_space_acdi_user_reply_ok,
    .memory_read_space_traction_function_definition_info_reply_ok = &_memory_read_space_traction_function_definition_info_reply_ok,
    .memory_read_space_traction_function_config_memory_reply_ok = &_memory_read_space_traction_function_config_memory_reply_ok,

    // Config Memory Read Reply Failed
    .memory_read_space_config_description_info_reply_fail = &_memory_read_space_config_description_info_reply_fail,
    .memory_read_space_all_reply_fail = &_memory_read_space_all_reply_fail,
    .memory_read_space_configuration_memory_reply_fail = &_memory_read_space_configuration_memory_reply_fail,
    .memory_read_space_acdi_manufacturer_reply_fail = &_memory_read_space_acdi_manufacturer_reply_fail,
    .memory_read_space_acdi_user_reply_fail = &_memory_read_space_acdi_user_reply_fail,
    .memory_read_space_traction_function_definition_info_reply_fail = &_memory_read_space_traction_function_definition_info_reply_fail,
    .memory_read_space_traction_function_config_memory_reply_fail = &_memory_read_space_traction_function_config_memory_reply_fail,

    // Config Memory Stream Read
    .memory_read_stream_space_config_description_info = &_memory_read_stream_space_config_description_info,
    .memory_read_stream_space_all = &_memory_read_stream_space_all,
    .memory_read_stream_space_configuration_memory = &_memory_read_stream_space_configuration_memory,
    .memory_read_stream_space_acdi_manufacturer = &_memory_read_stream_space_acdi_manufacturer,
    .memory_read_stream_space_acdi_user = &_memory_read_stream_space_acdi_user,
    .memory_read_stream_space_traction_function_definition_info = &_memory_read_stream_space_traction_function_definition_info,
    .memory_read_stream_space_traction_function_config_memory = &_memory_read_stream_space_traction_function_config_memory,

    // Config Memory Stream Read Reply = Ok
    .memory_read_stream_space_config_description_info_reply_ok = &_memory_read_stream_space_config_description_info_reply_ok,
    .memory_read_stream_space_all_reply_ok = &_memory_read_stream_space_all_reply_ok,
    .memory_read_stream_space_configuration_memory_reply_ok = &_memory_read_stream_space_configuration_memory_reply_ok,
    .memory_read_stream_space_acdi_manufacturer_reply_ok = &_memory_read_stream_space_acdi_manufacturer_reply_ok,
    .memory_read_stream_space_acdi_user_reply_ok = &_memory_read_stream_space_acdi_user_reply_ok,
    .memory_read_stream_space_traction_function_definition_info_reply_ok = &_memory_read_stream_space_traction_function_definition_info_reply_ok,
    .memory_read_stream_space_traction_function_config_memory_reply_ok = &_memory_read_stream_space_traction_function_config_memory_reply_ok,

    // Config Memory Stream Read Reply = Failed
    .memory_read_stream_space_config_description_info_reply_fail = &_memory_read_stream_space_config_description_info_reply_fail,
    .memory_read_stream_space_all_reply_fail = &_memory_read_stream_space_all_reply_fail,
    .memory_read_stream_space_configuration_memory_reply_fail = &_memory_read_stream_space_configuration_memory_reply_fail,
    .memory_read_stream_space_acdi_manufacturer_reply_fail = &_memory_read_stream_space_acdi_manufacturer_reply_fail,
    .memory_read_stream_space_acdi_user_reply_fail = &_memory_read_stream_space_acdi_user_reply_fail,
    .memory_read_stream_space_traction_function_definition_info_reply_fail = &_memory_read_stream_space_traction_function_definition_info_reply_fail,
    .memory_read_stream_space_traction_function_config_memory_reply_fail = &_memory_read_stream_space_traction_function_config_memory_reply_fail,

    // Config Memory Write
    .memory_write_space_config_description_info = &_memory_write_space_config_description_info,
    .memory_write_space_all = &_memory_write_space_all,
    .memory_write_space_configuration_memory = &_memory_write_space_configuration_memory,
    .memory_write_space_acdi_manufacturer = &_memory_write_space_acdi_manufacturer,
    .memory_write_space_acdi_user = &_memory_write_space_acdi_user,
    .memory_write_space_traction_function_definition_info = &_memory_write_space_traction_function_definition_info,
    .memory_write_space_traction_function_config_memory = &_memory_write_space_traction_function_config_memory,
    .memory_write_space_firmware_upgrade = _memory_write_space_firmware_upgrade,

    // Config Memory Write Reply Ok
    .memory_write_space_config_description_info_reply_ok = &_memory_write_space_config_description_info_reply_ok,
    .memory_write_space_all_reply_ok = &_memory_write_space_all_reply_ok,
    .memory_write_space_configuration_memory_reply_ok = &_memory_write_space_configuration_memory_reply_ok,
    .memory_write_space_acdi_manufacturer_reply_ok = &_memory_write_space_acdi_manufacturer_reply_ok,
    .memory_write_space_acdi_user_reply_ok = &_memory_write_space_acdi_user_reply_ok,
    .memory_write_space_traction_function_definition_info_reply_ok = &_memory_write_space_traction_function_definition_info_reply_ok,
    .memory_write_space_traction_function_config_memory_reply_ok = &_memory_write_space_traction_function_config_memory_reply_ok,

    // Config Memory Write Reply Fail
    .memory_write_space_config_description_info_reply_fail = &_memory_write_space_config_description_info_reply_fail,
    .memory_write_space_all_reply_fail = &_memory_write_space_all_reply_fail,
    .memory_write_space_configuration_memory_reply_fail = &_memory_write_space_configuration_memory_reply_fail,
    .memory_write_space_acdi_manufacturer_reply_fail = &_memory_write_space_acdi_manufacturer_reply_fail,
    .memory_write_space_acdi_user_reply_fail = &_memory_write_space_acdi_user_reply_fail,
    .memory_write_space_traction_function_definition_info_reply_fail = &_memory_write_space_traction_function_definition_info_reply_fail,
    .memory_write_space_traction_function_config_memory_reply_fail = &_memory_write_space_traction_function_config_memory_reply_fail,

    // Config Memory Write Under Mask
    .memory_write_under_mask_space_config_description_info = &_memory_write_under_mask_space_config_description_info,
    .memory_write_under_mask_space_all = &_memory_write_under_mask_space_all,
    .memory_write_under_mask_space_configuration_memory = &_memory_write_under_mask_space_configuration_memory,
    .memory_write_under_mask_space_acdi_manufacturer = &_memory_write_under_mask_space_acdi_manufacturer,
    .memory_write_under_mask_space_acdi_user = &_memory_write_under_mask_space_acdi_user,
    .memory_write_under_mask_space_traction_function_definition_info = _memory_write_under_mask_space_traction_function_definition_info,
    .memory_write_under_mask_space_traction_function_config_memory = &_memory_write_under_mask_space_traction_function_config_memory,
    .memory_write_under_mask_space_firmware_upgrade = &_memory_write_under_mask_space_firmware_upgrade,

    // Config Memory Stream Write
    .memory_write_stream_space_config_description_info = &_memory_write_stream_space_config_description_info,
    .memory_write_stream_space_all = &_memory_write_stream_space_all,
    .memory_write_stream_space_configuration_memory = &_memory_write_stream_space_configuration_memory,
    .memory_write_stream_space_acdi_manufacturer = &_memory_write_stream_space_acdi_manufacturer,
    .memory_write_stream_space_acdi_user = &_memory_write_stream_space_acdi_user,
    .memory_write_stream_space_traction_function_definition_info = &_memory_write_stream_space_traction_function_definition_info,
    .memory_write_stream_space_traction_function_config_memory = &_memory_write_stream_space_traction_function_config_memory,
    .memory_write_stream_space_firmware_upgrade = &_memory_write_stream_space_firmware_upgrade,

    // Config Memory Stream Write Reply = Ok
    .memory_write_stream_space_config_description_info_reply_ok = &_memory_write_stream_space_config_description_info_reply_ok,
    .memory_write_stream_space_all_reply_ok = &_memory_write_stream_space_all_reply_ok,
    .memory_write_stream_space_configuration_memory_reply_ok = &_memory_write_stream_space_configuration_memory_reply_ok,
    .memory_write_stream_space_acdi_manufacturer_reply_ok = &_memory_write_stream_space_acdi_manufacturer_reply_ok,
    .memory_write_stream_space_acdi_user_reply_ok = &_memory_write_stream_space_acdi_user_reply_ok,
    .memory_write_stream_space_traction_function_definition_info_reply_ok = &_memory_write_stream_space_traction_function_definition_info_reply_ok,
    .memory_write_stream_space_traction_function_config_memory_reply_ok = &_memory_write_stream_space_traction_function_config_memory_reply_ok,

    // Config Memory Stream Write Reply = Failed
    .memory_write_stream_space_config_description_info_reply_fail = &_memory_write_stream_space_config_description_info_reply_fail,
    .memory_write_stream_space_all_reply_fail = &_memory_write_stream_space_all_reply_fail,
    .memory_write_stream_space_configuration_memory_reply_fail = &_memory_write_stream_space_configuration_memory_reply_fail,
    .memory_write_stream_space_acdi_manufacturer_reply_fail = &_memory_write_stream_space_acdi_manufacturer_reply_fail,
    .memory_write_stream_space_acdi_user_reply_fail = &_memory_write_stream_space_acdi_user_reply_fail,
    .memory_write_stream_space_traction_function_definition_info_reply_fail = &_memory_write_stream_space_traction_function_definition_info_reply_fail,
    .memory_write_stream_space_traction_function_config_memory_reply_fail = &_memory_write_stream_space_traction_function_config_memory_reply_fail,

    // Config Memory Commands
    .memory_options_cmd = &_memory_options_cmd,
    .memory_options_reply = &_memory_options_reply,
    .memory_get_address_space_info = &_memory_get_address_space_info_cmd,
    .memory_get_address_space_info_reply_not_present = &_memory_get_address_space_info_reply_not_present,
    .memory_get_address_space_info_reply_present = &_memory_get_address_space_info_reply_present,
    .memory_reserve_lock = &_memory_reserve_lock,
    .memory_reserve_lock_reply = &_memory_reserve_lock_reply,
    .memory_get_unique_id = &_memory_get_unique_id,
    .memory_get_unique_id_reply = &_memory_get_unique_id_reply,
    .memory_unfreeze = &_memory_unfreeze,
    .memory_freeze = &_memory_freeze,
    .memory_update_complete = &_memory_update_complete,
    .memory_reset_reboot = &_memory_reset_reboot,
    .memory_factory_reset = &_memory_factory_reset,

    .lock_shared_resources = &_lock_shared_resources,     //  HARDWARE INTERFACE
    .unlock_shared_resources = &_unlock_shared_resources, //  HARDWARE INTERFACE
};

interface_protocol_datagram_handler_t interface_protocol_datagram_handler_with_nulls = {

    // Config Memory Read
    .memory_read_space_config_description_info = NULL,
    .memory_read_space_all = NULL,
    .memory_read_space_configuration_memory = NULL,
    .memory_read_space_acdi_manufacturer = NULL,
    .memory_read_space_acdi_user = NULL,
    .memory_read_space_traction_function_definition_info = NULL,
    .memory_read_space_traction_function_config_memory = NULL,

    // Config Memory Read Reply Ok
    .memory_read_space_config_description_info_reply_ok = NULL,
    .memory_read_space_all_reply_ok = NULL,
    .memory_read_space_configuration_memory_reply_ok = NULL,
    .memory_read_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_space_acdi_user_reply_ok = NULL,
    .memory_read_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Read Reply Failed
    .memory_read_space_config_description_info_reply_fail = NULL,
    .memory_read_space_all_reply_fail = NULL,
    .memory_read_space_configuration_memory_reply_fail = NULL,
    .memory_read_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_space_acdi_user_reply_fail = NULL,
    .memory_read_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Stream Read
    .memory_read_stream_space_config_description_info = NULL,
    .memory_read_stream_space_all = NULL,
    .memory_read_stream_space_configuration_memory = NULL,
    .memory_read_stream_space_acdi_manufacturer = NULL,

    // Config Memory Stream Read Reply = Ok
    .memory_read_stream_space_config_description_info_reply_ok = NULL,
    .memory_read_stream_space_all_reply_ok = NULL,
    .memory_read_stream_space_configuration_memory_reply_ok = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_ok = NULL,
    .memory_read_stream_space_acdi_user_reply_ok = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_ok = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Stream Read Reply = Failed
    .memory_read_stream_space_config_description_info_reply_fail = NULL,
    .memory_read_stream_space_all_reply_fail = NULL,
    .memory_read_stream_space_configuration_memory_reply_fail = NULL,
    .memory_read_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_read_stream_space_acdi_user_reply_fail = NULL,
    .memory_read_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_read_stream_space_traction_function_config_memory_reply_fail = NULL,
    // Config Memory Write
    .memory_write_space_config_description_info = NULL,
    .memory_write_space_all = NULL,
    .memory_write_space_configuration_memory = NULL,
    .memory_write_space_acdi_manufacturer = NULL,
    .memory_write_space_acdi_user = NULL,
    .memory_write_space_traction_function_definition_info = NULL,
    .memory_write_space_traction_function_config_memory = NULL,
    .memory_write_space_firmware_upgrade = NULL,

    // Config Memory Write Reply Ok
    .memory_write_space_config_description_info_reply_ok = NULL,
    .memory_write_space_all_reply_ok = NULL,
    .memory_write_space_configuration_memory_reply_ok = NULL,
    .memory_write_space_acdi_manufacturer_reply_ok = NULL,
    .memory_write_space_acdi_user_reply_ok = NULL,
    .memory_write_space_traction_function_definition_info_reply_ok = NULL,
    .memory_write_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Write Reply Fail
    .memory_write_space_config_description_info_reply_fail = NULL,
    .memory_write_space_all_reply_fail = NULL,
    .memory_write_space_configuration_memory_reply_fail = NULL,
    .memory_write_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_space_acdi_user_reply_fail = NULL,
    .memory_write_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Write Under Mask
    .memory_write_under_mask_space_config_description_info = NULL,
    .memory_write_under_mask_space_all = NULL,
    .memory_write_under_mask_space_configuration_memory = NULL,
    .memory_write_under_mask_space_acdi_manufacturer = NULL,
    .memory_write_under_mask_space_acdi_user = NULL,
    .memory_write_under_mask_space_traction_function_definition_info = NULL,
    .memory_write_under_mask_space_traction_function_config_memory = NULL,
    .memory_write_under_mask_space_firmware_upgrade = NULL,

    // Config Memory Stream Write
    .memory_write_stream_space_config_description_info = NULL,
    .memory_write_stream_space_all = NULL,
    .memory_write_stream_space_configuration_memory = NULL,
    .memory_write_stream_space_acdi_manufacturer = NULL,

    // Config Memory Stream Write Reply = Ok
    .memory_write_stream_space_config_description_info_reply_ok = NULL,
    .memory_write_stream_space_all_reply_ok = NULL,
    .memory_write_stream_space_configuration_memory_reply_ok = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_ok = NULL,
    .memory_write_stream_space_acdi_user_reply_ok = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_ok = NULL,
    .memory_write_stream_space_traction_function_config_memory_reply_ok = NULL,

    // Config Memory Stream Write Reply = Failed
    .memory_write_stream_space_config_description_info_reply_fail = NULL,
    .memory_write_stream_space_all_reply_fail = NULL,
    .memory_write_stream_space_configuration_memory_reply_fail = NULL,
    .memory_write_stream_space_acdi_manufacturer_reply_fail = NULL,
    .memory_write_stream_space_acdi_user_reply_fail = NULL,
    .memory_write_stream_space_traction_function_definition_info_reply_fail = NULL,
    .memory_write_stream_space_traction_function_config_memory_reply_fail = NULL,

    // Config Memory Commands
    .memory_options_cmd = NULL,
    .memory_options_reply = NULL,
    .memory_get_address_space_info = NULL,
    .memory_get_address_space_info_reply_not_present = NULL,
    .memory_get_address_space_info_reply_present = NULL,
    .memory_reserve_lock = NULL,
    .memory_reserve_lock_reply = NULL,
    .memory_get_unique_id = NULL,
    .memory_get_unique_id_reply = NULL,
    .memory_unfreeze = NULL,
    .memory_freeze = NULL,
    .memory_update_complete = NULL,
    .memory_reset_reboot = NULL,
    .memory_factory_reset = NULL,

    .lock_shared_resources = &_lock_shared_resources,     //  HARDWARE INTERFACE
    .unlock_shared_resources = &_unlock_shared_resources, //  HARDWARE INTERFACE

};

void _reset_variables(void)
{

    called_function_ptr = nullptr;
    lock_shared_resources_called = false;
    unlock_shared_resources_called = false;
}

void _global_initialize(void)
{

    ProtocolDatagramHandler_initialize(&interface_protocol_datagram_handler);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _global_initialize_with_nulls(void)
{

    ProtocolDatagramHandler_initialize(&interface_protocol_datagram_handler_with_nulls);
    OpenLcbNode_initialize(&interface_openlcb_node);
    OpenLcbBufferFifo_initialize();
    OpenLcbBufferStore_initialize();
}

void _test_for_rejected_datagram(openlcb_statemachine_info_t *statemachine_info)
{

    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->payload_count, 2);
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.valid);
    EXPECT_FALSE(statemachine_info->outgoing_msg_info.enumerate);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0), ERROR_PERMANENT_NOT_IMPLEMENTED_SUBCOMMAND_UNKNOWN);
}

void _test_for_rejected_datagram_bad_command(openlcb_statemachine_info_t *statemachine_info)
{

    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_EQ(statemachine_info->outgoing_msg_info.msg_ptr->payload_count, 2);
    EXPECT_TRUE(statemachine_info->outgoing_msg_info.valid);
    EXPECT_FALSE(statemachine_info->outgoing_msg_info.enumerate);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info->outgoing_msg_info.msg_ptr, 0), ERROR_PERMANENT_NOT_IMPLEMENTED_COMMAND_UNKNOWN);
}

void _read_command_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_manufacturer);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_user);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_definition_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_config_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_command_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_reply_ok_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_manufacturer_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_user_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_definition_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_config_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_reply_ok_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_OK_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_reply_fail_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_manufacturer_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_acdi_user_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_definition_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_traction_function_config_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_reply_fail_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_REPLY_FAIL_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_command_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_manufacturer);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_user);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_definition_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_config_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_command_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invald
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_reply_ok_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_manufacturer_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_user_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_definition_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_config_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invald
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_reply_ok_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_OK_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_reply_fail_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_manufacturer_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_acdi_user_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_definition_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_traction_function_config_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _read_stream_reply_fail_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_STREAM_REPLY_FAIL_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_read_stream_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_command_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_manufacturer);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_user);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_definition_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_config_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_command_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_reply_ok_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_manufacturer_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_user_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_definition_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_config_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_reply_ok_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_OK_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_reply_fail_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_manufacturer_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_acdi_user_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_definition_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_traction_function_config_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_reply_fail_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_REPLY_FAIL_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_under_mask_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_acdi_manufacturer);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_acdi_user);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_traction_function_definition_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_traction_function_config_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_under_mask_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_UNDER_MASK_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_under_mask_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_command_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_manufacturer);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_user);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_definition_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_config_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_command_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_reply_ok_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_manufacturer_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_user_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_definition_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_config_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_reply_ok_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_OK_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory_reply_ok);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_reply_fail_space_in_byte_6(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ALL;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_manufacturer_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_ACDI_USER_ACCESS;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_acdi_user_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_DEFINITION_INFO;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_definition_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = ADDRESS_SPACE_TRACTION_FUNCTION_CONFIGURATION_MEMORY;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_traction_function_config_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_IN_BYTE_6;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[6] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 8;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _write_stream_reply_fail_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_FF;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_config_description_info_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_FE;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_all_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_WRITE_STREAM_REPLY_FAIL_SPACE_FD;
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_write_stream_space_configuration_memory_reply_fail);

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _operations_space(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_OPTIONS_CMD;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_options_cmd);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_OPTIONS_REPLY;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_options_reply);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_ADDRESS_SPACE_INFO_CMD;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_get_address_space_info_cmd);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_ADDRESS_SPACE_INFO_REPLY_PRESENT;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_get_address_space_info_reply_present);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_ADDRESS_SPACE_INFO_REPLY_NOT_PRESENT;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_get_address_space_info_reply_not_present);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_RESERVE_LOCK;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_reserve_lock);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_RESERVE_LOCK_REPLY;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_reserve_lock_reply);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_UNIQUE_ID;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_get_unique_id);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_UNIQUE_ID_REPLY;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_get_unique_id_reply);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_FREEZE;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_freeze);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_UNFREEZE;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_unfreeze);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_UPDATE_COMPLETE;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_update_complete);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_RESET_REBOOT;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_reset_reboot);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_FACTORY_RESET;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 1;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    if (is_null_subcommand)
        _test_for_rejected_datagram(statemachine_info);
    else
        EXPECT_EQ(called_function_ptr, &_memory_factory_reset);

    // ********************************************
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = 0x00; // Invalid
    OpenLcbUtilities_copy_dword_to_openlcb_payload(statemachine_info->incoming_msg_info.msg_ptr, CONFIG_MEM_ADDRESS, 2);
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 7;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

void _invalid_command(openlcb_statemachine_info_t *statemachine_info, bool is_null_subcommand)
{

    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = 0xFF; // invalid
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_GET_UNIQUE_ID;
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 0;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram_bad_command(statemachine_info);
    _reset_variables();
    *statemachine_info->incoming_msg_info.msg_ptr->payload[0] = DATAGRAM_MEMORY_CONFIGURATION;
    *statemachine_info->incoming_msg_info.msg_ptr->payload[1] = DATAGRAM_MEMORY_READ_SPACE_FF + 4; // invalid
    statemachine_info->incoming_msg_info.msg_ptr->payload_count = 2;

    ProtocolDatagramHandler_handle_datagram(statemachine_info);

    _test_for_rejected_datagram(statemachine_info);
}

TEST(ProtocolDatagramHandler, initialize)
{

    _reset_variables();
    _global_initialize();
}

TEST(ProtocolDatagramHandler, initialize_with_nulls)
{

    _reset_variables();
    _global_initialize_with_nulls();
}

TEST(ProtocolDatagramHandler, load_datagram_received_ok)
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
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    ProtocolDatagramHandler_load_datagram_received_ok_message(&statemachine_info, 0x0000);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM_OK_REPLY);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 2);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 0), 0x0000 | DATAGRAM_OK_REPLY_PENDING);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->dest_alias, statemachine_info.incoming_msg_info.msg_ptr->source_alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->dest_id, statemachine_info.incoming_msg_info.msg_ptr->source_id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, statemachine_info.incoming_msg_info.msg_ptr->dest_alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, statemachine_info.incoming_msg_info.msg_ptr->dest_id);
}

TEST(ProtocolDatagramHandler, load_datagram_received_rejected)
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
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    ProtocolDatagramHandler_load_datagram_rejected_message(&statemachine_info, ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);

    EXPECT_TRUE(statemachine_info.outgoing_msg_info.valid);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->mti, MTI_DATAGRAM_REJECTED_REPLY);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->payload_count, 2);
    EXPECT_EQ(OpenLcbUtilities_extract_word_from_openlcb_payload(statemachine_info.outgoing_msg_info.msg_ptr, 0), ERROR_PERMANENT_CONFIG_MEM_ADDRESS_SPACE_UNKNOWN);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->dest_alias, statemachine_info.incoming_msg_info.msg_ptr->source_alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->dest_id, statemachine_info.incoming_msg_info.msg_ptr->source_id);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_alias, statemachine_info.incoming_msg_info.msg_ptr->dest_alias);
    EXPECT_EQ(statemachine_info.outgoing_msg_info.msg_ptr->source_id, statemachine_info.incoming_msg_info.msg_ptr->dest_id);
}

TEST(ProtocolDatagramHandler, handle_datagram)
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
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    incoming_msg->mti = MTI_DATAGRAM;

    // Read Command
    _read_command_space_in_byte_6(&statemachine_info, false);
    _read_command_space(&statemachine_info, false);

    // Read Reply
    _read_reply_ok_space_in_byte_6(&statemachine_info, false);
    _read_reply_ok_space(&statemachine_info, false);

    _read_reply_fail_space_in_byte_6(&statemachine_info, false);
    _read_reply_fail_space(&statemachine_info, false);

    // Read Stream Command
    _read_stream_command_space_in_byte_6(&statemachine_info, false);
    _read_stream_command_space(&statemachine_info, false);

    // Read Reply
    _read_stream_reply_ok_space_in_byte_6(&statemachine_info, false);
    _read_stream_reply_ok_space(&statemachine_info, false);

    _read_stream_reply_fail_space_in_byte_6(&statemachine_info, false);
    _read_stream_reply_fail_space(&statemachine_info, false);

    // Write Command
    _write_command_space_in_byte_6(&statemachine_info, false);
    _write_command_space(&statemachine_info, false);

    // Write Reply
    _write_reply_ok_space_in_byte_6(&statemachine_info, false);
    _write_reply_ok_space(&statemachine_info, false);

    _write_reply_fail_space_in_byte_6(&statemachine_info, false);
    _write_reply_fail_space(&statemachine_info, false);

    _write_under_mask_space_in_byte_6(&statemachine_info, false);
    _write_under_mask_space(&statemachine_info, false);

    // Write Stream Command
    _write_stream_command_space_in_byte_6(&statemachine_info, false);
    _write_stream_command_space(&statemachine_info, false);

    // Write Stream Reply
    _write_stream_reply_ok_space_in_byte_6(&statemachine_info, false);
    _write_stream_reply_ok_space(&statemachine_info, false);

    _write_stream_reply_fail_space_in_byte_6(&statemachine_info, false);
    _write_stream_reply_fail_space(&statemachine_info, false);

    _operations_space(&statemachine_info, false);

    _invalid_command(&statemachine_info, false);
}

TEST(ProtocolDatagramHandler, handle_datagram_null_handlers)
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

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;
    incoming_msg->mti = MTI_DATAGRAM;

    // Read Command
    _read_command_space_in_byte_6(&statemachine_info, true);
    _read_command_space(&statemachine_info, true);

    // Read Reply
    _read_reply_ok_space_in_byte_6(&statemachine_info, true);
    _read_reply_ok_space(&statemachine_info, true);

    _read_reply_fail_space_in_byte_6(&statemachine_info, true);
    _read_reply_fail_space(&statemachine_info, true);

    // Read Stream Command
    _read_stream_command_space_in_byte_6(&statemachine_info, true);
    _read_stream_command_space(&statemachine_info, true);

    // Read Reply
    _read_stream_reply_ok_space_in_byte_6(&statemachine_info, true);
    _read_stream_reply_ok_space(&statemachine_info, true);

    _read_stream_reply_fail_space_in_byte_6(&statemachine_info, true);
    _read_stream_reply_fail_space(&statemachine_info, true);

    // Write Command
    _write_command_space_in_byte_6(&statemachine_info, true);
    _write_command_space(&statemachine_info, true);

    // Write Reply
    _write_reply_ok_space_in_byte_6(&statemachine_info, true);
    _write_reply_ok_space(&statemachine_info, true);

    _write_reply_fail_space_in_byte_6(&statemachine_info, true);
    _write_reply_fail_space(&statemachine_info, true);

    _write_under_mask_space_in_byte_6(&statemachine_info, true);
    _write_under_mask_space(&statemachine_info, true);

    // Write Stream Command
    _write_stream_command_space_in_byte_6(&statemachine_info, true);
    _write_stream_command_space(&statemachine_info, true);

    // Write Stream Reply
    _write_stream_reply_ok_space_in_byte_6(&statemachine_info, true);
    _write_stream_reply_ok_space(&statemachine_info, true);

    _write_stream_reply_fail_space_in_byte_6(&statemachine_info, true);
    _write_stream_reply_fail_space(&statemachine_info, true);

    _operations_space(&statemachine_info, true);

    _invalid_command(&statemachine_info, true);
}

TEST(ProtocolDatagramHandler, handle_datagram_received_ok)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_msg_t *datagram_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    node1->last_received_datagram = datagram_msg;

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);

    Protocol_DatagramHandler_handle_datagram_received_ok(&statemachine_info);

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_FALSE(node1->state.resend_datagram);
}

TEST(ProtocolDatagramHandler, handle_datagram_rejected_temporary)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_msg_t *datagram_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    node1->last_received_datagram = datagram_msg;

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info.incoming_msg_info.msg_ptr, ERROR_TEMPORARY_BUFFER_UNAVAILABLE, 0);
    statemachine_info.incoming_msg_info.msg_ptr->mti = MTI_DATAGRAM_REJECTED_REPLY;
    statemachine_info.incoming_msg_info.msg_ptr->payload_count = 2;

    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);

    ProtocolDatagramHandler_handle_datagram_rejected(&statemachine_info);

    EXPECT_FALSE(lock_shared_resources_called);
    EXPECT_FALSE(unlock_shared_resources_called);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);
    EXPECT_EQ(node1->last_received_datagram, datagram_msg);
    EXPECT_TRUE(node1->state.resend_datagram);
}

TEST(ProtocolDatagramHandler, handle_datagram_rejected_permenent)
{

    _reset_variables();
    _global_initialize();

    openlcb_node_t *node1 = OpenLcbNode_allocate(DEST_ID, &_node_parameters_main_node);
    node1->alias = DEST_ALIAS;

    openlcb_msg_t *incoming_msg = OpenLcbBufferStore_allocate_buffer(BASIC);
    openlcb_msg_t *outgoing_msg = OpenLcbBufferStore_allocate_buffer(SNIP);

    openlcb_msg_t *datagram_msg = OpenLcbBufferStore_allocate_buffer(DATAGRAM);

    node1->last_received_datagram = datagram_msg;

    EXPECT_NE(node1, nullptr);
    EXPECT_NE(incoming_msg, nullptr);
    EXPECT_NE(outgoing_msg, nullptr);

    openlcb_statemachine_info_t statemachine_info;

    statemachine_info.openlcb_node = node1;
    statemachine_info.incoming_msg_info.msg_ptr = incoming_msg;
    statemachine_info.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info.incoming_msg_info.msg_ptr, ERROR_PERMANENT, 0);
    statemachine_info.incoming_msg_info.msg_ptr->mti = MTI_DATAGRAM_REJECTED_REPLY;
    statemachine_info.incoming_msg_info.msg_ptr->payload_count = 2;

    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 1);

    ProtocolDatagramHandler_handle_datagram_rejected(&statemachine_info);

    EXPECT_TRUE(lock_shared_resources_called);
    EXPECT_TRUE(unlock_shared_resources_called);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_FALSE(node1->state.resend_datagram);
}

TEST(ProtocolDatagramHandler, handle_datagram_rejected_temporary_no_resend_message)
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
    statemachine_info.incoming_msg_info.enumerate = false;
    OpenLcbUtilities_copy_word_to_openlcb_payload(statemachine_info.incoming_msg_info.msg_ptr, ERROR_TEMPORARY_BUFFER_UNAVAILABLE, 0);
    statemachine_info.incoming_msg_info.msg_ptr->mti = MTI_DATAGRAM_REJECTED_REPLY;
    statemachine_info.incoming_msg_info.msg_ptr->payload_count = 2;

    statemachine_info.outgoing_msg_info.msg_ptr = outgoing_msg;
    statemachine_info.outgoing_msg_info.enumerate = false;
    statemachine_info.outgoing_msg_info.valid = false;
    incoming_msg->source_id = SOURCE_ID;
    incoming_msg->source_alias = SOURCE_ALIAS;
    incoming_msg->dest_id = DEST_ID;
    incoming_msg->dest_alias = DEST_ALIAS;

    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(node1->last_received_datagram, nullptr);

    ProtocolDatagramHandler_handle_datagram_rejected(&statemachine_info);

    EXPECT_FALSE(lock_shared_resources_called);
    EXPECT_FALSE(unlock_shared_resources_called);
    EXPECT_EQ(OpenLcbBufferStore_datagram_messages_allocated(), 0);
    EXPECT_EQ(node1->last_received_datagram, nullptr);
    EXPECT_FALSE(node1->state.resend_datagram);

    ProtocolDatagramHandler_clear_resend_datagram_message(node1);
}

TEST(ProtocolDatagramHandler, _100ms_timer_tick)
{

    _reset_variables();
    _global_initialize();

    ProtocolDatagramHandler_100ms_timer_tick();
}
