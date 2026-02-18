project(openlcb C)

set(SOURCES 
    openlcb_application.c 
    openlcb_buffer_fifo.c 
    openlcb_buffer_list.c 
    openlcb_buffer_store.c 
    openlcb_gridconnect.c 
    openlcb_login_statemachine_handler.c
    openlcb_login_statemachine.c
    openlcb_main_statemachine.c 
    openlcb_node.c  
    openlcb_utilities.c 
    protocol_datagram_handler.c 
    protocol_event_transport.c 
    protocol_message_network.c 
    protocol_snip.c
    protocol_config_mem_read_handler.c
    protocol_config_mem_write_handler.c
    protocol_config_mem_operations_handler.c
    protocol_broadcast_time_handler.c
    openlcb_application_broadcast_time.c
    protocol_train_handler.c
    openlcb_application_train.c
    openlcb_float16.c

)


if (${GTEST})
set(TESTS ${TESTS}
    
    ${ROOT_DIR}/src/openlcb/openlcb_application_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_fifo_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_list_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_store_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_gridconnect_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_main_statemachine_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_node_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_utilities_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_datagram_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_event_transport_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_message_network_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_snip_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_read_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_write_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_operations_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_broadcast_time_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_application_broadcast_time_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_train_handler_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_application_train_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_float16_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
