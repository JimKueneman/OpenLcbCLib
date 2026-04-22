/** @file openlcb_user_config.c
 *  @brief Node parameters for the WASM build (TCP transport).
 */

#include "openlcb_user_config.h"
#include "openlcb/openlcb_types.h"
#include "openlcb/openlcb_defines.h"

const node_parameters_t OpenLcbUserConfig_node_parameters = {

    .snip = {
        .mfg_version      = 4,
        .name             = "OpenLcbCLib WASM Node",
        .model            = "WASM Core",
        .hardware_version = "0.0.1",
        .software_version = "0.0.1",
        .user_version     = 2
    },

    .protocol_support = (PSI_DATAGRAM |
                         PSI_MEMORY_CONFIGURATION |
                         PSI_EVENT_EXCHANGE |
                         PSI_ABBREVIATED_DEFAULT_CDI |
                         PSI_SIMPLE_NODE_INFORMATION |
                         PSI_CONFIGURATION_DESCRIPTION_INFO |
                         PSI_FIRMWARE_UPGRADE),

    .consumer_count_autocreate = 2,
    .producer_count_autocreate = 2,

    .configuration_options = {
        .write_under_mask_supported                   = true,
        .unaligned_reads_supported                    = true,
        .unaligned_writes_supported                   = true,
        .read_from_manufacturer_space_0xfc_supported  = true,
        .read_from_user_space_0xfb_supported          = true,
        .write_to_user_space_0xfb_supported           = true,
        .stream_read_write_supported                  = false,
        .high_address_space                           = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .low_address_space                            = CONFIG_MEM_SPACE_FIRMWARE,
        .description                                  = "Memory space capabilities"
    },

    .address_space_configuration_definition = {
        .present           = true,
        .read_only         = true,
        .low_address_valid = false,
        .address_space     = CONFIG_MEM_SPACE_CONFIGURATION_DEFINITION_INFO,
        .highest_address   = 0,
        .description       = "Configuration definition info"
    },

    .address_space_all = {
        .present           = true,
        .read_only         = true,
        .low_address_valid = true,
        .address_space     = CONFIG_MEM_SPACE_ALL,
        .highest_address   = 0xFFFF,
        .low_address       = 0,
        .description       = "All memory Info"
    },

    .address_space_config_memory = {
        .present           = true,
        .read_only         = false,
        .low_address_valid = false,
        .address_space     = CONFIG_MEM_SPACE_CONFIGURATION_MEMORY,
        .highest_address   = (0x0200 - 1),
        .low_address       = 0,
        .description       = "Configuration memory storage"
    },

    .address_space_acdi_manufacturer = {
        .present           = true,
        .read_only         = true,
        .low_address_valid = false,
        .address_space     = CONFIG_MEM_SPACE_ACDI_MANUFACTURER_ACCESS,
        .highest_address   = (125 - 1),
        .low_address       = 0,
        .description       = "ACDI access manufacturer"
    },

    .address_space_acdi_user = {
        .present           = true,
        .read_only         = false,
        .low_address_valid = false,
        .address_space     = CONFIG_MEM_SPACE_ACDI_USER_ACCESS,
        .highest_address   = (128 - 1),
        .low_address       = 0,
        .description       = "ACDI access user storage"
    },

    .address_space_firmware = {
        .present           = true,
        .read_only         = false,
        .low_address_valid = false,
        .address_space     = CONFIG_MEM_SPACE_FIRMWARE,
        .highest_address   = 0xFFFFFFFF,
        .low_address       = 0,
        .description       = "Firmware update address space"
    },

    .cdi = NULL,
    .fdi = NULL,
};
