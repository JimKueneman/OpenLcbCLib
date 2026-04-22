cmake_minimum_required(VERSION 3.14)

# ============================================================================
# OpenLcbCLib WASM build — CAN transport (gridconnect text frames ride a
# WebSocket on the JS side), all protocol features enabled.
# ----------------------------------------------------------------------------
# Must be invoked via emcmake:
#   emcmake cmake -S . -B build
#   cmake --build build
#
# Output: build/openlcb-core.{wasm,js} (copied to dist/ by build.sh)
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
    ${CMAKE_SOURCE_DIR}/openlcb_user_config.c
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
    -sEXPORTED_FUNCTIONS=['_wasm_init','_wasm_run','_wasm_100ms_tick','_wasm_rx_gridconnect','_malloc','_free']
    -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPU8']
    -sMODULARIZE=1
    -sEXPORT_NAME=OpenLcbCoreFactory
    -sENVIRONMENT=web,node
    -sALLOW_MEMORY_GROWTH=1
    -sSTACK_SIZE=65536
    -O2
)

set_target_properties(openlcb-core PROPERTIES
    OUTPUT_NAME "openlcb-core"
    SUFFIX ".js"
)
