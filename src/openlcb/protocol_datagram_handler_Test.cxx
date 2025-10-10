#include "test/main_Test.hxx"

#include "protocol_datagram_handler.h"

#include "protocol_message_network.h"
#include "openlcb_types.h"
#include "openlcb_defines.h"
#include "openlcb_node.h"
#include "openlcb_utilities.h"
#include "openlcb_buffer_store.h"
#include "openlcb_buffer_fifo.h"

// #define AUTO_CREATE_EVENT_COUNT 10
// #define DEST_EVENT_ID 0x0605040302010000
// #define SOURCE_ALIAS 0x222
// #define SOURCE_ID 0x010203040506
// #define DEST_ALIAS 0xBBB
// #define DEST_ID 0x060504030201
// #define SNIP_NAME_FULL "0123456789012345678901234567890123456789"
// #define SNIP_MODEL "Test Model J"

// bool memory_read_message_called = false;
// bool memory_read_reply_ok_message_called = false;
// bool memory_read_reply_fail_message_called = false;
// bool memory_write_message_called = false;
// bool memory_write_reply_ok_message_called = false;
// bool memory_write_reply_fail_message_called = false;
// bool memory_write_under_mask_message = false;
// bool memory_options_cmd_message_called = false;
// bool memory_options_reply_message_called = false;
// bool memory_get_address_space_info_message_called = false;
// bool memory_get_address_space_info_reply_not_present_message_called = false;
// bool memory_get_address_space_info_reply_present_message_called = false;
// bool memory_reserve_lock_message_called = false;
// bool memory_get_unique_id_message_called = false;
// bool memory_unfreeze_message_called = false;
// bool memory_freeze_message_called = false;
// bool memory_update_complete_message_called = false;
// bool memory_reset_reboot_message_called = false;
// bool memory_factory_reset_message_called = false;
// bool send_datagram_rejected_reply_called = false;

// node_parameters_t _node_parameters_main_node = {

//     .consumer_count_autocreate = AUTO_CREATE_EVENT_COUNT,
//     .producer_count_autocreate = AUTO_CREATE_EVENT_COUNT,

//     .snip.mfg_version = 4, // early spec has this as 1, later it was changed to be the number of null present in this section so 4.  must treat them the same
//     .snip.name = SNIP_NAME_FULL,
//     .snip.model = SNIP_MODEL,
//     .snip.hardware_version = "0.001",
//     .snip.software_version = "0.002",
//     .snip.user_version = 2, // early spec has this as 1, later it was changed to be the number of null present in this section so 2.  must treat them the same

//     .protocol_support = (PSI_DATAGRAM |
//                          PSI_FIRMWARE_UPGRADE |
//                          PSI_MEMORY_CONFIGURATION |
//                          PSI_EVENT_EXCHANGE |
//                          PSI_EVENT_EXCHANGE |
//                          PSI_ABBREVIATED_DEFAULT_CDI |
//                          PSI_SIMPLE_NODE_INFORMATION |
//                          PSI_CONFIGURATION_DESCRIPTION_INFO),

//     .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
//     .configuration_options.low_address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,

//     .configuration_options.read_from_manufacturer_space_0xfc_supported = 1,
//     .configuration_options.read_from_user_space_0xfb_supported = 1,
//     .configuration_options.stream_read_write_supported = 0,
//     .configuration_options.unaligned_reads_supported = 1,
//     .configuration_options.unaligned_writes_supported = 1,
//     .configuration_options.write_to_user_space_0xfb_supported = 1,
//     .configuration_options.write_under_mask_supported = 1,
//     .configuration_options.description = "These are options that defined the memory space capabilities",

//     // Space 0xFF
//     // WARNING: The ACDI write always maps to the first 128 bytes (64 Name + 64 Description) of the Config Memory System so
//     //    make sure the CDI maps these 2 items to the first 128 bytes as well
//     .address_space_configuration_definition.read_only = 1,
//     .address_space_configuration_definition.present = 0,
//     .address_space_configuration_definition.low_address_valid = 0,   // assume the low address starts at 0
//     .address_space_configuration_definition.low_address = 0,         // ignored if low_address_valid is false
//     .address_space_configuration_definition.highest_address = 0x200, // length of the .cdi file byte array contents; see USER_DEFINED_CDI_LENGTH for array size
//     .address_space_configuration_definition.address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
//     .address_space_configuration_definition.description = "Configuration definition info",

//     // Space 0xFE
//     .address_space_all.read_only = 1,
//     .address_space_all.present = 0,
//     .address_space_all.low_address_valid = 0, // assume the low address starts at 0
//     .address_space_all.low_address = 0,       // ignored if low_address_valid is false
//     .address_space_all.highest_address = 0,
//     .address_space_all.address_space = ADDRESS_SPACE_ALL,
//     .address_space_all.description = "All memory Info",

//     // Space 0xFD
//     .address_space_config_memory.read_only = 0,
//     .address_space_config_memory.present = 0,
//     .address_space_config_memory.low_address_valid = 0, // assume the low address starts at 0
//     .address_space_config_memory.low_address = 0,       // ignored if low_address_valid is false
//     .address_space_config_memory.highest_address = 0,   // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
//     .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
//     .address_space_config_memory.description = "Configuration memory storage",

//     // Space 0xEF
//     .address_space_firmware.read_only = 0,
//     .address_space_firmware.present = 1,
//     .address_space_firmware.low_address_valid = 0,   // assume the low address starts at 0
//     .address_space_firmware.low_address = 0,         // ignored if low_address_valid is false
//     .address_space_firmware.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
//     .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
//     .address_space_firmware.description = "Firmware Bootloader",

//     .cdi =
//         {
//             // </cdi>
//         },

// };

// interface_openlcb_node_t interface_openlcb_node = {};

// void _memory_read_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail)
// {
//     memory_read_message_called = true;
// }

// void _memory_read_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space)
// {
//     memory_read_reply_ok_message_called = true;
// }

// void _memory_read_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space)
// {
//     memory_read_reply_fail_message_called = true;
// }

// void _memory_write_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail)
// {
//     memory_write_message_called = true;
// }

// void _memory_write_reply_ok_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space)
// {
//     memory_write_reply_ok_message_called = true;
// }

// void _memory_write_reply_fail_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space)
// {
//     memory_write_reply_fail_message_called = true;
// }

// void _memory_write_under_mask_message(openlcb_statemachine_info_t *statemachine_info, uint8_t space, uint8_t return_msg_ok, uint8_t return_msg_fail)
// {
//     memory_write_under_mask_message = true;
// }

// void _memory_options_cmd_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_options_cmd_message_called = true;
// }

// void _memory_options_reply_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_options_reply_message_called = true;
// }

// void _memory_get_address_space_info_message(openlcb_statemachine_info_t *statemachine_infog)
// {
//     memory_get_address_space_info_message_called = true;
// }

// void _memory_get_address_space_info_reply_not_present_message(openlcb_statemachine_info_t *statemachine_infog)
// {
//     memory_get_address_space_info_reply_not_present_message_called = true;
// }

// void _memory_get_address_space_info_reply_present_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_get_address_space_info_reply_present_message_called = true;
// }

// void _memory_reserve_lock_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_reserve_lock_message_called = true;
// }

// void _memory_get_unique_id_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_get_unique_id_message_called = true;
// }

// void _memory_unfreeze_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_unfreeze_message_called = true;
// }

// void _memory_freeze_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_freeze_message_called = true;
// }

// void _memory_update_complete_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_update_complete_message_called = true;
// }

// void _memory_reset_reboot_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_reset_reboot_message_called = true;
// }

// void _memory_factory_reset_message(openlcb_statemachine_info_t *statemachine_info)
// {
//     memory_factory_reset_message_called = true;
// }

// void _send_datagram_rejected_reply(openlcb_statemachine_info_t *statemachine_info, uint16_t error_code)
// {
//     send_datagram_rejected_reply_called = true;
// }

// interface_protocol_datagram_handler_t interface_protocol_datagram_handler = {

//     .memory_read_space_acdi_manufacturer_message = NULL,
//     .memory_read_space_acdi_manufacturer_reply_fail_message = NULL,

//     // .memory_read_message = &_memory_read_message,
//     // .memory_read_reply_ok_message = &_memory_read_reply_ok_message,
//     // .memory_read_reply_fail_message = &_memory_read_reply_fail_message,
//     // .memory_write_message = &_memory_write_message,
//     // .memory_write_reply_ok_message = &_memory_write_reply_ok_message,
//     // .memory_write_reply_fail_message = &_memory_write_reply_fail_message,
//     // .memory_write_under_mask_message = &_memory_write_under_mask_message,
//     .memory_options_cmd_message = &_memory_options_cmd_message,
//     .memory_options_reply_message = &_memory_options_reply_message,
//     .memory_get_address_space_info_message = &_memory_get_address_space_info_message,
//     .memory_get_address_space_info_reply_not_present_message = &_memory_get_address_space_info_reply_not_present_message,
//     .memory_get_address_space_info_reply_present_message = &_memory_get_address_space_info_reply_present_message,
//     .memory_reserve_lock_message = &_memory_reserve_lock_message,
//     .memory_get_unique_id_message = &_memory_get_unique_id_message,
//     .memory_unfreeze_message = &_memory_unfreeze_message,
//     .memory_freeze_message = &_memory_freeze_message,
//     .memory_update_complete_message = &_memory_update_complete_message,
//     .memory_reset_reboot_message = &_memory_reset_reboot_message,
//     .memory_factory_reset_message = &_memory_factory_reset_message,
//     .send_datagram_rejected_reply = &_send_datagram_rejected_reply,
// };

// void _reset_variables(void)
// {

//     memory_read_message_called = false;
//     memory_read_reply_ok_message_called = false;
//     memory_read_reply_fail_message_called = false;
//     memory_write_message_called = false;
//     memory_write_reply_ok_message_called = false;
//     memory_write_reply_fail_message_called = false;
//     memory_write_under_mask_message = false;
//     memory_options_cmd_message_called = false;
//     memory_options_reply_message_called = false;
//     memory_get_address_space_info_message_called = false;
//     memory_get_address_space_info_reply_not_present_message_called = false;
//     memory_get_address_space_info_reply_present_message_called = false;
//     memory_reserve_lock_message_called = false;
//     memory_get_unique_id_message_called = false;
//     memory_unfreeze_message_called = false;
//     memory_freeze_message_called = false;
//     memory_update_complete_message_called = false;
//     memory_reset_reboot_message_called = false;
//     memory_factory_reset_message_called = false;
//     send_datagram_rejected_reply_called = false;
// }

// void _global_initialize(void)
// {

//     ProtocolDatagramHandler_initialize(&interface_protocol_datagram_handler);
//     OpenLcbNode_initialize(&interface_openlcb_node);
//     OpenLcbBufferFifo_initialize();
//     OpenLcbBufferStore_initialize();
// }

// TEST(ProtocolDatagramHandler, initialize)
// {

//     _reset_variables();
//     _global_initialize();
// }
