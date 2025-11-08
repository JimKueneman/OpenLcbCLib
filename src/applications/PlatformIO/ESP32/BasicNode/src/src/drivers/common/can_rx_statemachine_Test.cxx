#include "test/main_Test.hxx"

#include "can_rx_statemachine.h"

#include "can_types.h"
#include "can_utilities.h"
#include "../../openlcb/openlcb_defines.h"
#include "../alias_mappings.h"

bool can_legacy_snip_called = false;
bool can_single_frame_called = false;
bool can_first_frame_called = false;
bool can_middle_frame_called = false;
bool can_last_frame_called = false;
bool can_cid_called = false;
bool can_rid_called = false;
bool can_ame_called = false;
bool can_amd_called = false;
bool can_amr_called = false;
bool can_error_information_report_called = false;
bool can_stream_called = false;
bool fail_find_mapping = false;
bool on_receive_called = false;

void _can_frame_message_handler_handle_can_legacy_snip(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type)
{

    can_legacy_snip_called = true;
}

void _can_frame_message_handler_handle_single_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type)
{

    can_single_frame_called = true;
}

void _can_frame_message_handler_handle_first_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type)
{

    can_first_frame_called = true;
}

void _can_frame_message_handler_handle_middle_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index)
{

    can_middle_frame_called = true;
}

void _can_frame_message_handler_handle_last_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index)
{

    can_last_frame_called = true;
}

void _can_frame_message_handler_stream_frame(can_msg_t *can_msg, uint8_t can_buffer_start_index, payload_type_enum data_type)
{

    can_stream_called = true;
}

void _can_frame_message_handler_cid_frame(can_msg_t *can_msg)
{

    can_cid_called = true;
}

void _can_frame_message_handler_rid_frame(can_msg_t *can_msg)
{

    can_rid_called = true;
}

void _can_frame_message_handler_amd_frame(can_msg_t *can_msg)
{

    can_amd_called = true;
}

void _can_frame_message_handler_amr_frame(can_msg_t *can_msg)
{

    can_amr_called = true;
}

void _can_frame_message_handler_ame_frame(can_msg_t *can_msg)
{

    can_ame_called = true;
}

void _can_frame_message_handler_error_information_report_frame(can_msg_t *can_msg)
{

    can_error_information_report_called = true;
}

alias_mapping_t alias_mapping;

alias_mapping_t *_alias_mappings_find_mapping_by_alias(uint16_t alias)
{
    if (fail_find_mapping)
    {

        return nullptr;
    }

    alias_mapping.alias = alias;
    alias_mapping.node_id = 0x010203040506;

    return &alias_mapping;
}

void _on_receive(can_msg_t *can_msg)
{

    on_receive_called = true;
}

void _reset_variables(void)
{

    can_legacy_snip_called = false;
    can_single_frame_called = false;
    can_first_frame_called = false;
    can_middle_frame_called = false;
    can_last_frame_called = false;
    can_cid_called = false;
    can_rid_called = false;
    can_ame_called = false;
    can_amd_called = false;
    can_amr_called = false;
    can_error_information_report_called = false;
    can_stream_called = false;
    fail_find_mapping = false;
    on_receive_called = false;
}

const interface_can_rx_statemachine_t interface_can_rx_statemachine = {

    .handle_can_legacy_snip = &_can_frame_message_handler_handle_can_legacy_snip,
    .handle_single_frame = &_can_frame_message_handler_handle_single_frame,
    .handle_first_frame = &_can_frame_message_handler_handle_first_frame,
    .handle_middle_frame = &_can_frame_message_handler_handle_middle_frame,
    .handle_last_frame = &_can_frame_message_handler_handle_last_frame,
    .handle_cid_frame = &_can_frame_message_handler_cid_frame,
    .handle_rid_frame = &_can_frame_message_handler_rid_frame,
    .handle_ame_frame = &_can_frame_message_handler_ame_frame,
    .handle_amd_frame = &_can_frame_message_handler_amd_frame,
    .handle_amr_frame = &_can_frame_message_handler_amr_frame,
    .handle_error_info_report_frame = &_can_frame_message_handler_error_information_report_frame,
    .handle_stream_frame = &_can_frame_message_handler_stream_frame,
    .alias_mapping_find_mapping_by_alias = &_alias_mappings_find_mapping_by_alias,
    .on_receive = &_on_receive};

const interface_can_rx_statemachine_t interface_can_rx_statemachine_null_rx_target = {

    .handle_can_legacy_snip = nullptr,
    .handle_single_frame = nullptr,
    .handle_first_frame = nullptr,
    .handle_middle_frame = nullptr,
    .handle_last_frame = nullptr,
    .handle_cid_frame = nullptr,
    .handle_rid_frame = nullptr,
    .handle_ame_frame = nullptr,
    .handle_amd_frame = nullptr,
    .handle_amr_frame = nullptr,
    .handle_error_info_report_frame = nullptr,
    .handle_stream_frame = nullptr,
    .alias_mapping_find_mapping_by_alias = &_alias_mappings_find_mapping_by_alias,
    .on_receive = nullptr};

void _test_snip_request(can_msg_t *can_msg)
{

    // ************************************************************************
    // SNIP Request to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19DE8AAA;
    CanUtilities_load_can_message(can_msg, identifier, 2, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_TRUE(on_receive_called);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_TRUE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // SNIP Request to 0x0FFF Not to us
    // ************************************************************************
    _reset_variables();
    fail_find_mapping = true;
    identifier = 0x19DE8AAA;
    CanUtilities_load_can_message(can_msg, identifier, 2, 0x0F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************
}

void _test_legacy_snip(can_msg_t *can_msg)
{

    // ************************************************************************
    // SNIP Reply with no framing bits (legacy) to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x0F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_TRUE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************
}

void _test_snip_framing_bits(can_msg_t *can_msg)
{

    // ************************************************************************
    // SNIP Reply with start framing bits to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x4F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_TRUE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // SNIP Reply with middle framing bits to 0x0FFF
    // ************************************************************************
    identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0xCF, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_TRUE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // SNIP Reply with last framing bits to 0x0FFF
    // ************************************************************************
    identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x8F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_TRUE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************
}

void _test_datagram_framing(can_msg_t *can_msg)
{

    // ************************************************************************
    // Datagram Only Frame to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x1AFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_TRUE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram First Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1BFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_TRUE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram Middle Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1CFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_TRUE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram Last Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1DFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_TRUE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram Only Frame to 0x0FFF Not to Us
    // ************************************************************************
    identifier = 0x1AFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    fail_find_mapping = true;
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram First Frame to 0x0FFF Not to Us
    // ************************************************************************
    identifier = 0x1BFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    fail_find_mapping = true;
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram Middle Frame to 0x0FFF Not to Us
    // ************************************************************************
    identifier = 0x1CFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    fail_find_mapping = true;
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // Datagram Last Frame to 0x0FFF Not to Us
    // ************************************************************************
    identifier = 0x1DFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    fail_find_mapping = true;
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************
}

void _test_traction_control(can_msg_t *can_msg)
{

    uint32_t identifier = 0x195BE6BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x4A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_TRUE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);

    identifier = 0x195BE6BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x8A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_TRUE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_verify_id(can_msg_t *can_msg)
{

    uint32_t identifier = 0x194906BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_TRUE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_reserved(can_msg_t *can_msg)
{

    uint32_t identifier = 0x1EAAA6BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_stream(can_msg_t *can_msg)
{

    uint32_t identifier = 0x1FAAA6BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_TRUE(can_stream_called);

    identifier = 0x1FAAA6BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    fail_find_mapping = true;
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_cid(can_msg_t *can_msg)
{
    // ************************************************************************
    // CID 0x07
    // ************************************************************************
    uint32_t identifier = 0x170506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x06
    // ************************************************************************
    identifier = 0x160506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x05
    // ************************************************************************
    identifier = 0x150506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x04
    // ************************************************************************
    identifier = 0x140506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x03
    // ************************************************************************
    identifier = 0x130506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x02
    // ************************************************************************
    identifier = 0x120506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x01
    // ************************************************************************
    identifier = 0x110506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_TRUE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************

    // ************************************************************************
    // CID 0x00 (error)
    // ************************************************************************
    identifier = 0x100506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
    // ************************************************************************
}

void _test_rid(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107006BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_TRUE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_amd(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107016BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_TRUE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_ame(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107026BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_TRUE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_amr(can_msg_t *can_msg)
{

    uint32_t identifier = 0x107036BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_TRUE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_error_info_report(can_msg_t *can_msg)
{

    uint32_t identifier = 0x107106BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_TRUE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x107116BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_TRUE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x107126BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_TRUE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x107136BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_TRUE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_snip_request_nulls(can_msg_t *can_msg)
{
    // ************************************************************************
    // SNIP Request to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19DE8AAA;
    CanUtilities_load_can_message(can_msg, identifier, 2, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************
}

void _test_legacy_snip_nulls(can_msg_t *can_msg)
{

    // ************************************************************************
    // SNIP Reply with no framing bits (legacy) to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x0F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************
}

void _test_snip_framing_bits_nulls(can_msg_t *can_msg)
{

    // ************************************************************************
    // SNIP Reply with start framing bits to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x4F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // SNIP Reply with middle framing bits to 0x0FFF
    // ************************************************************************
    identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0xCF, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // SNIP Reply with last framing bits to 0x0FFF
    // ************************************************************************
    identifier = 0x19A08AAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x8F, 0xFF, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************
}

void _test_datagram_framing_nulls(can_msg_t *can_msg)
{

    // ************************************************************************
    // Datagram Only Frame to 0x0FFF
    // ************************************************************************
    uint32_t identifier = 0x1AFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // Datagram First Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1BFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // Datagram Middle Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1CFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // Datagram Last Frame to 0x0FFF
    // ************************************************************************
    identifier = 0x1DFFFAAA;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x59, 0x34, 0xCF, 0x60, 0x56, 0x45, 0x023, 0x66);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);

    // ************************************************************************
}

void _test_stream_nulls(can_msg_t *can_msg)
{

    uint32_t identifier = 0x1FAAA6BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

void _test_traction_control_nulls(can_msg_t *can_msg)
{

    uint32_t identifier = 0x195BE6BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x4A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    identifier = 0x195BE6BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x8A, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_verify_id_nulls(can_msg_t *can_msg)
{

    uint32_t identifier = 0x194906BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_cid_nulls(can_msg_t *can_msg)
{
    // ************************************************************************
    // CID 0x07
    // ************************************************************************
    uint32_t identifier = 0x170506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x06
    // ************************************************************************
    identifier = 0x160506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x05
    // ************************************************************************
    identifier = 0x150506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x04
    // ************************************************************************
    identifier = 0x140506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x03
    // ************************************************************************
    identifier = 0x130506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x02
    // ************************************************************************
    identifier = 0x120506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************

    // ************************************************************************
    // CID 0x01
    // ************************************************************************
    identifier = 0x110506BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    // ************************************************************************
}

void _test_rid_nulls(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107006BE;
    CanUtilities_load_can_message(can_msg, identifier, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_amd_nulls(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107016BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_ame_nulls(can_msg_t *can_msg)
{
    uint32_t identifier = 0x107026BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_amr_nulls(can_msg_t *can_msg)
{

    uint32_t identifier = 0x107036BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_error_info_report_nulls(can_msg_t *can_msg)
{

    uint32_t identifier = 0x107106BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    identifier = 0x107116BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    identifier = 0x107126BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);

    identifier = 0x107136BE;
    CanUtilities_load_can_message(can_msg, identifier, 6, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
}

void _test_event_with_payload(can_msg_t *can_msg)
{

    uint32_t identifier = 0x19F166BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_TRUE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x19F156BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_TRUE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x19F146BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_TRUE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    CanRxStatemachine_initialize(&interface_can_rx_statemachine_null_rx_target);

    identifier = 0x19F166BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x19F156BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    identifier = 0x19F146BE;
    CanUtilities_load_can_message(can_msg, identifier, 8, 0x05, 0x01, 0x01, 0x01, 0x07, 0xFF, 0x00, 0x00);
    _reset_variables();
    CanRxStatemachine_incoming_can_driver_callback(can_msg);
    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}

TEST(CanRxStatemachine, initialize)
{

    can_msg_t can_msg;

    _reset_variables();
    CanRxStatemachine_initialize(&interface_can_rx_statemachine);
    _test_snip_request(&can_msg);
    _test_legacy_snip(&can_msg);
    _test_snip_framing_bits(&can_msg);
    _test_datagram_framing(&can_msg);
    _test_traction_control(&can_msg);
    _test_verify_id(&can_msg);
    _test_cid(&can_msg);
    _test_rid(&can_msg);
    _test_amd(&can_msg);
    _test_amr(&can_msg);
    _test_ame(&can_msg);
    _test_error_info_report(&can_msg);
    _test_reserved(&can_msg);
    _test_stream(&can_msg);
    _test_event_with_payload(&can_msg);

    _reset_variables();
    CanRxStatemachine_initialize(&interface_can_rx_statemachine_null_rx_target);

    _test_snip_request_nulls(&can_msg);
    _test_legacy_snip_nulls(&can_msg);
    _test_snip_framing_bits_nulls(&can_msg);
    _test_datagram_framing_nulls(&can_msg);
    _test_traction_control_nulls(&can_msg);
    _test_verify_id_nulls(&can_msg);
    _test_cid_nulls(&can_msg);
    _test_rid_nulls(&can_msg);
    _test_amd_nulls(&can_msg);
    _test_amr_nulls(&can_msg);
    _test_ame_nulls(&can_msg);
    _test_error_info_report_nulls(&can_msg);
    _test_stream_nulls(&can_msg);

    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);

    _reset_variables();
    CanRxStatemachine_initialize(&interface_can_rx_statemachine_null_rx_target);

    EXPECT_FALSE(can_legacy_snip_called);
    EXPECT_FALSE(can_single_frame_called);
    EXPECT_FALSE(can_first_frame_called);
    EXPECT_FALSE(can_middle_frame_called);
    EXPECT_FALSE(can_last_frame_called);
    EXPECT_FALSE(can_cid_called);
    EXPECT_FALSE(can_rid_called);
    EXPECT_FALSE(can_ame_called);
    EXPECT_FALSE(can_amd_called);
    EXPECT_FALSE(can_amr_called);
    EXPECT_FALSE(can_error_information_report_called);
    EXPECT_FALSE(can_stream_called);
}