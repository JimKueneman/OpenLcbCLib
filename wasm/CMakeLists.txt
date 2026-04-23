cmake_minimum_required(VERSION 3.14)

# ============================================================================
# OpenLcbCLib WASM build — CAN transport (gridconnect text frames ride a
# WebSocket on the JS side), all protocol features enabled.
# ----------------------------------------------------------------------------
# Must be invoked via emcmake:
#   emcmake cmake -S . -B build
#   cmake --build build
#
# Output: build/openlcb-core.{wasm,mjs} (copied to dist/ by build.sh)
# ============================================================================

project(openlcb-core-wasm C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This build must be configured with emcmake")
endif()

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

# ----------------------------------------------------------------------------
# OpenLCB protocol core (transport-agnostic)
# ----------------------------------------------------------------------------
set(OPENLCB_SOURCES
    ${ROOT_DIR}/src/openlcb/openlcb_application.c
    ${ROOT_DIR}/src/openlcb/openlcb_application_broadcast_time.c
    ${ROOT_DIR}/src/openlcb/openlcb_application_dcc_detector.c
    ${ROOT_DIR}/src/openlcb/openlcb_application_train.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_fifo.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_list.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_store.c
    ${ROOT_DIR}/src/openlcb/openlcb_config.c
    ${ROOT_DIR}/src/openlcb/openlcb_float16.c
    ${ROOT_DIR}/src/openlcb/openlcb_gridconnect.c
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine.c
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine_handler.c
    ${ROOT_DIR}/src/openlcb/openlcb_main_statemachine.c
    ${ROOT_DIR}/src/openlcb/openlcb_node.c
    ${ROOT_DIR}/src/openlcb/openlcb_utilities.c
    ${ROOT_DIR}/src/openlcb/protocol_broadcast_time_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_operations_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_read_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_stream_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_write_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_datagram_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_event_transport.c
    ${ROOT_DIR}/src/openlcb/protocol_message_network.c
    ${ROOT_DIR}/src/openlcb/protocol_snip.c
    ${ROOT_DIR}/src/openlcb/protocol_stream_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_train_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_train_search_handler.c
)

# ----------------------------------------------------------------------------
# CAN transport driver
# ----------------------------------------------------------------------------
set(CANBUS_SOURCES
    ${ROOT_DIR}/src/drivers/canbus/alias_mapping_listener.c
    ${ROOT_DIR}/src/drivers/canbus/alias_mappings.c
    ${ROOT_DIR}/src/drivers/canbus/can_buffer_fifo.c
    ${ROOT_DIR}/src/drivers/canbus/can_buffer_store.c
    ${ROOT_DIR}/src/drivers/canbus/can_config.c
    ${ROOT_DIR}/src/drivers/canbus/can_login_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_login_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_main_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_rx_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_rx_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_tx_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_tx_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_utilities.c
)

set(UTIL_SOURCES
    ${ROOT_DIR}/src/utilities/mustangpeak_endian_helper.c
    ${ROOT_DIR}/src/utilities/mustangpeak_string_helper.c
    ${ROOT_DIR}/src/utilities_pc/threadsafe_stringlist.c
)

add_executable(openlcb-core
    ${CMAKE_SOURCE_DIR}/bindings.c
    ${OPENLCB_SOURCES}
    ${CANBUS_SOURCES}
    ${UTIL_SOURCES}
)

target_include_directories(openlcb-core PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${ROOT_DIR}/src
)

target_compile_options(openlcb-core PRIVATE
    -Wall
    -Wno-unused-parameter
    -Wno-unused-function
    -O2
)

target_link_options(openlcb-core PRIVATE
    -sEXPORTED_FUNCTIONS=['_wasm_initialize','_wasm_run','_wasm_100ms_tick','_wasm_rx_gridconnect','_wasm_node_builder_reset','_wasm_node_set_snip','_wasm_node_set_protocol_support','_wasm_node_set_event_autocreate','_wasm_node_set_configuration_options','_wasm_node_set_address_space','_wasm_node_set_cdi','_wasm_node_set_fdi','_wasm_create_node','_wasm_send_event_pc_report','_wasm_send_event_with_mti','_wasm_send_teach_event','_wasm_send_initialization_event','_wasm_register_consumer_eventid','_wasm_register_producer_eventid','_wasm_clear_consumer_eventids','_wasm_clear_producer_eventids','_wasm_register_consumer_range','_wasm_register_producer_range','_wasm_clear_consumer_ranges','_wasm_clear_producer_ranges','_wasm_bt_is_consumer','_wasm_bt_is_producer','_wasm_bt_start','_wasm_bt_stop','_wasm_bt_send_report_time','_wasm_bt_send_report_date','_wasm_bt_send_report_year','_wasm_bt_send_report_rate','_wasm_bt_send_start','_wasm_bt_send_stop','_wasm_bt_send_date_rollover','_wasm_bt_send_query','_wasm_bt_send_query_reply','_wasm_bt_send_set_time','_wasm_bt_send_set_date','_wasm_bt_send_set_year','_wasm_bt_send_set_rate','_wasm_bt_send_command_start','_wasm_bt_send_command_stop','_wasm_bt_setup_consumer','_wasm_bt_setup_producer','_wasm_bt_trigger_query_reply','_wasm_bt_trigger_sync_delay','_wasm_bt_make_clock_id','_wasm_bt_is_time_event','_wasm_bt_extract_clock_id','_wasm_bt_get_event_type','_wasm_bt_extract_time','_wasm_bt_extract_date','_wasm_bt_extract_year','_wasm_bt_extract_rate','_wasm_bt_create_time_event_id','_wasm_bt_create_date_event_id','_wasm_bt_create_year_event_id','_wasm_bt_create_rate_event_id','_wasm_bt_create_command_event_id','_wasm_train_send_emergency_stop','_wasm_train_send_query_speeds','_wasm_train_send_assign_controller','_wasm_train_send_release_controller','_wasm_train_send_noop','_wasm_train_send_set_speed','_wasm_train_send_set_function','_wasm_train_send_query_function','_wasm_train_set_dcc_address','_wasm_train_get_dcc_address','_wasm_train_is_long_address','_wasm_train_set_speed_steps','_wasm_train_get_speed_steps','_wasm_dcc_encode_event_id','_wasm_dcc_make_short_address','_wasm_dcc_make_consist_address','_wasm_dcc_extract_direction','_wasm_dcc_extract_address_type','_wasm_dcc_extract_raw_address','_wasm_dcc_extract_dcc_address','_wasm_dcc_extract_detector_id','_wasm_dcc_is_track_empty','_wasm_util_generate_event_range_id','_wasm_util_is_producer_event_assigned','_wasm_util_is_consumer_event_assigned','_wasm_util_is_event_in_producer_ranges','_wasm_util_is_event_in_consumer_ranges','_wasm_float16_from_float','_wasm_float16_to_float','_wasm_float16_negate','_wasm_float16_is_nan','_wasm_float16_is_zero','_wasm_float16_speed_with_direction','_wasm_float16_get_speed','_wasm_float16_get_direction','_wasm_train_search_is_search_event','_wasm_train_search_extract_flags','_wasm_train_search_extract_digits','_wasm_train_search_digits_to_address','_wasm_train_search_create_event_id','_malloc','_free']
    -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPU8']
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sEXPORT_NAME=OpenLcbCoreFactory
    -sENVIRONMENT=web,node
    -sALLOW_MEMORY_GROWTH=1
    -sSTACK_SIZE=65536
    -O2
)

set_target_properties(openlcb-core PROPERTIES
    OUTPUT_NAME "openlcb-core"
    SUFFIX ".mjs"
)
