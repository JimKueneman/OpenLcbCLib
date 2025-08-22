project(openlcb C)

set(SOURCES 
    openlcb_application_callbacks.c 
    openlcb_application.c 
    openlcb_buffer_fifo.c 
    openlcb_buffer_list.c 
    openlcb_buffer_store.c 
    openlcb_gridconnect.c 
    openlcb_main_statemachine.c 
    openlcb_node.c  
    openlcb_utilities.c 
    protocol_datagram_handlers.c 
    protocol_datagram.c 
    protocol_event_transport.c 
    protocol_message_network.c 
    protocol_snip.c
)


if (${GTEST})
set(TESTS ${TESTS}
    
    ${ROOT_DIR}/src/openlcb/openlcb_application_callbacks_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_application_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_fifo_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_list_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_store_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_gridconnect_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_main_statemachine_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_node_Test.cxx
    ${ROOT_DIR}/src/openlcb/openlcb_utilities_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_datagram_handlers_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_datagram_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_event_transport_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_message_network_Test.cxx
    ${ROOT_DIR}/src/openlcb/protocol_snip_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
