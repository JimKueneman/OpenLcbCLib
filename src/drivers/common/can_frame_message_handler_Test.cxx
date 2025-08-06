#include "test/main_Test.hxx"

#include "can_frame_message_handler.h"
#include "../../openlcb/openlcb_buffer_store.h"
#include "../../openlcb/openlcb_node.h"
#include "../../openlcb/openlcb_types.h"
#include "../../openlcb/openlcb_defines.h"
#include "../../drivers/common/can_frame_message_handler.h"

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

    .configuration_options.high_address_space = ADDRESS_SPACE_CONFIGURATION_DEFINITION_INFO,
    .configuration_options.low_address_space = ADDRESS_SPACE_FIRMWARE,

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
    .address_space_config_memory.low_address_valid = 0,   // assume the low address starts at 0
    .address_space_config_memory.low_address = 0,         // ignored if low_address_valid is false
    .address_space_config_memory.highest_address = 0x200, // This is important for multi node applications as the config memory for node N will start at (N * high-low) and they all must be the same for any parameter file in a single app
    .address_space_config_memory.address_space = ADDRESS_SPACE_CONFIGURATION_MEMORY,
    .address_space_config_memory.description = "Configuration memory storage",

    // Space 0xFC
    .address_space_acdi_manufacturer.read_only = 1,
    .address_space_acdi_manufacturer.present = 1,
    .address_space_acdi_manufacturer.low_address_valid = 0,     // assume the low address starts at 0
    .address_space_acdi_manufacturer.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_manufacturer.highest_address = 125 - 1, // Predefined in the Configuration Description Definition Spec 1 + 41 + 41 + 21 + 21 = 125
    .address_space_acdi_manufacturer.address_space = ADDRESS_SPACE_ACDI_MANUFACTURER_ACCESS,
    .address_space_acdi_manufacturer.description = "ACDI access manufacturer",

    // Space 0xFB
    .address_space_acdi_user.read_only = 0,
    .address_space_acdi_user.present = 1,
    .address_space_acdi_user.low_address_valid = 0,     // assume the low address starts at 0
    .address_space_acdi_user.low_address = 0,           // ignored if low_address_valid is false
    .address_space_acdi_user.highest_address = 128 - 1, // Predefined in the Configuration Description Definition Spec = 1 + 63 + 64 = 128 bytes length
    .address_space_acdi_user.address_space = ADDRESS_SPACE_ACDI_USER_ACCESS,
    .address_space_acdi_user.description = "ACDI access user storage",

    // Space 0xEF
    .address_space_firmware.read_only = 0,
    .address_space_firmware.present = 0,
    .address_space_firmware.low_address_valid = 0,        // assume the low address starts at 0
    .address_space_firmware.low_address = 0,              // Firmware ALWAYS assumes it starts at 0
    .address_space_firmware.highest_address = 0xFFFFFFFF, // Predefined in the Configuration Description Definition Spec
    .address_space_firmware.address_space = ADDRESS_SPACE_FIRMWARE,
    .address_space_firmware.description = "Firmware update address space",

    .cdi = {},

};

static bool _try_transmit_can_message(can_msg_t *can_msg)
{

    fprintf(stderr, "\n\n\nTransmitting CAN message\n\n\n");

    return true;
}

const static can_frame_message_handler_interface_t _interface = {

    .try_transmit_can_message = &_try_transmit_can_message};

TEST(CanFrameMessageHandler, cid)
{

#define NODE_ID 0x010203040506

    can_msg_t can_msg;
    can_msg_t worker_msg;

    memset(&can_msg, 0x00, sizeof(can_msg));
    memset(&can_msg, 0x00, sizeof(worker_msg));

    OpenLcbNode_initialize();

    CanFrameMessageHandler_initialize(&_interface);

    openlcb_node_t *openlcb_node = OpenLcbNode_allocate(NODE_ID, &node_parameters);
    EXPECT_NE(openlcb_node, nullptr);
    openlcb_node->alias = 0xAAA; // Make up an alias

    // not our Alias ID
    can_msg.identifier = 0x17050BBB;
    CanFrameMessageHandler_cid(openlcb_node, &can_msg, &worker_msg);

    // our Alias ID
    can_msg.identifier = 0x17050AAA;
    CanFrameMessageHandler_cid(openlcb_node, &can_msg, &worker_msg);

    // our Alias ID
    can_msg.identifier = 0x16050AAA;
    CanFrameMessageHandler_cid(openlcb_node, &can_msg, &worker_msg);

    // our Alias ID
    can_msg.identifier = 0x15050AAA;
    CanFrameMessageHandler_cid(openlcb_node, &can_msg, &worker_msg);

    // our Alias ID
    can_msg.identifier = 0x14050AAA;
    CanFrameMessageHandler_cid(openlcb_node, &can_msg, &worker_msg);
}

TEST(CanFrameMessageHandler, rid)
{
}

TEST(CanFrameMessageHandler, amd)
{
}

TEST(CanFrameMessageHandler, ame)
{
}

TEST(CanFrameMessageHandler, amr)
{
}
