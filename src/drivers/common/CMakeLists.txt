project(drivers_common C)

set(
SOURCES 
    can_buffer_fifo.c 
    can_buffer_store.c 
    can_rx_message_handler.c 
    can_login_message_handler.c 
    can_tx_message_handler.c
    can_main_statemachine.c 
    can_rx_statemachine.c 
    can_login_statemachine.c 
    can_tx_statemachine.c 
    can_utilities.c
)


if (${GTEST})
set(TESTS ${TESTS}
    
    ${ROOT_DIR}/src/drivers/common/can_buffer_fifo_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_buffer_store_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_rx_message_handler_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_login_message_handler_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_tx_message_handler_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_main_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_rx_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_login_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_tx_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/common/can_utilities_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
