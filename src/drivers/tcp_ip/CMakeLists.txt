project(tcp_ip C)

set(
SOURCES
    tcp_utilities.c
    tcp_rx_statemachine.c
    tcp_tx_statemachine.c
    tcp_link_control.c
    tcp_login_statemachine.c
    tcp_main_statemachine.c
    tcp_config.c
    tcp_mdns.c
)

if (${GTEST})
set(TESTS ${TESTS}

    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_utilities_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_tx_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_rx_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_link_control_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_login_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_main_statemachine_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_config_Test.cxx
    ${ROOT_DIR}/src/drivers/tcp_ip/tcp_mdns_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
