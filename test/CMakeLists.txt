cmake_minimum_required(VERSION 3.14)

# Setup directory paths.
set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)

# Specify project.
project(openlcbclib-test)

# Enable the required language standards.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Uncomment out the following line to get verbose build output.
set(CMAKE_VERBOSE_MAKEFILE ON)

# Get a copy of GoogleTest.
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

# Compile flags.
add_compile_options(
    -fPIC -g -O0 -Wall -Werror --coverage -DGTEST -D__psv__=
    -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -Wno-attributes -D_GNU_SOURCE
)
# Compile flags. The "sys_common.h" include from the mla library makes an
# improper definition of uintptr_t. This suppresses that include and works
# around the one type dependency needed for this test build.
add_compile_options(-D_SYS_COMMON_H -DSYS_TASKS_PRIORITY=int)

# GTest and GMock includes + typical user config for non-bootloader tests
include_directories(
    ${CMAKE_SOURCE_DIR}/user_config/typical ${CMAKE_SOURCE_DIR}/ ${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include
)

# Add libraries that have tests disabled.

# Add libraries that have test enabled.
set(GTEST ON)
add_subdirectory(${ROOT_DIR}/src/utilities utilities)
add_subdirectory(${ROOT_DIR}/src/utilities_pc utilities_pc)
add_subdirectory(${ROOT_DIR}/src/drivers/canbus canbus)
add_subdirectory(${ROOT_DIR}/src/openlcb openlcb)


set(LINK_LIBS ${LINK_LIBS}
    utilities
    utilities_pc
    canbus
    openlcb
)

# =============================================================================
# Bootloader test — separate library compiled with bootloader openlcb_user_config.h
# =============================================================================

set(BOOTLOADER_CONFIG_DIR ${CMAKE_SOURCE_DIR}/user_config/bootloader)

    # Bootloader library sources — mirrors what a real bootloader application
    # would compile.  protocol_config_mem_read_handler.c is deliberately
    # excluded: the bootloader never reads config memory spaces, and omitting
    # it here catches any unguarded references at link time (the same failure
    # mode a real CubeIDE / PlatformIO bootloader build would hit).
set(BOOTLOADER_LIB_SOURCES
    ${ROOT_DIR}/src/openlcb/openlcb_application.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_fifo.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_list.c
    ${ROOT_DIR}/src/openlcb/openlcb_buffer_store.c
    ${ROOT_DIR}/src/openlcb/openlcb_gridconnect.c
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine_handler.c
    ${ROOT_DIR}/src/openlcb/openlcb_login_statemachine.c
    ${ROOT_DIR}/src/openlcb/openlcb_main_statemachine.c
    ${ROOT_DIR}/src/openlcb/openlcb_node.c
    ${ROOT_DIR}/src/openlcb/openlcb_utilities.c
    ${ROOT_DIR}/src/openlcb/openlcb_float16.c
    ${ROOT_DIR}/src/openlcb/protocol_datagram_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_event_transport.c
    ${ROOT_DIR}/src/openlcb/protocol_message_network.c
    ${ROOT_DIR}/src/openlcb/protocol_snip.c
    # protocol_config_mem_read_handler.c excluded — not used by bootloader
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_write_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_config_mem_operations_handler.c
    ${ROOT_DIR}/src/openlcb/protocol_broadcast_time_handler.c
    ${ROOT_DIR}/src/openlcb/openlcb_application_broadcast_time.c
    ${ROOT_DIR}/src/openlcb/protocol_train_handler.c
    ${ROOT_DIR}/src/openlcb/openlcb_application_train.c
    ${ROOT_DIR}/src/openlcb/protocol_train_search_handler.c
    ${ROOT_DIR}/src/openlcb/openlcb_config.c
    ${ROOT_DIR}/src/drivers/canbus/alias_mapping_listener.c
    ${ROOT_DIR}/src/drivers/canbus/alias_mappings.c
    ${ROOT_DIR}/src/drivers/canbus/can_buffer_fifo.c
    ${ROOT_DIR}/src/drivers/canbus/can_buffer_store.c
    ${ROOT_DIR}/src/drivers/canbus/can_login_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_login_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_main_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_rx_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_rx_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_tx_message_handler.c
    ${ROOT_DIR}/src/drivers/canbus/can_tx_statemachine.c
    ${ROOT_DIR}/src/drivers/canbus/can_utilities.c
    ${ROOT_DIR}/src/utilities/mustangpeak_endian_helper.c
    ${ROOT_DIR}/src/utilities/mustangpeak_string_helper.c
    ${ROOT_DIR}/src/utilities_pc/threadsafe_stringlist.c
)

add_library(openlcb_bootloader STATIC ${BOOTLOADER_LIB_SOURCES})
target_include_directories(openlcb_bootloader
    BEFORE PUBLIC
        ${BOOTLOADER_CONFIG_DIR}
        ${ROOT_DIR}/src
)

add_executable(openlcb_bootloader_Test
    ${ROOT_DIR}/src/openlcb/openlcb_bootloader_Test.cxx
)
target_include_directories(openlcb_bootloader_Test
    BEFORE PUBLIC
        ${BOOTLOADER_CONFIG_DIR}
        ${ROOT_DIR}/src
)
target_link_libraries(openlcb_bootloader_Test
    GTest::gtest_main
    GTest::gmock_main
    -fPIC
    --coverage
    openlcb_bootloader
)
add_custom_command(TARGET openlcb_bootloader_Test
    POST_BUILD
    COMMAND ./openlcb_bootloader_Test
    COMMAND rm -rf gcovr
)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
# Mac OS X specific configuration
set(START_GROUP)
set(END_GROUP)
add_link_options(--coverage)
else ()
# Linux specific configuration
set(START_GROUP "-Wl,--start-group")
set(END_GROUP "-Wl,--end-group")
set(LINK_LIBS ${LINK_LIBS}
    -lgcov
)
endif ()

# Setup gcov target.
add_custom_target(TARGET_GCOV ALL
    DEPENDS gcovr/coverage.html
)

# Build an executable for each test file.
foreach(testsourcefile ${TESTS})
    # Manipulate path to result in unique test names.
    get_filename_component(testname ${testsourcefile} NAME_WE)
    get_filename_component(testdir ${testsourcefile} DIRECTORY)
    get_filename_component(testdir ${testdir} NAME)
    set(testname "${testdir}_${testname}")

    add_executable(${testname} ${testsourcefile})
    target_include_directories(${testname}
        PUBLIC
            ${ROOT_DIR}/src
    )
    target_link_libraries(${testname}
        GTest::gtest_main
        GTest::gmock_main

        -fPIC
        ${START_GROUP}
        ${LINK_LIBS}
        ${END_GROUP}
    )
    add_custom_command(TARGET ${testname}
        POST_BUILD
        COMMAND ./${testname}
        # Forces the coverage report to be regenerated if tests are run.
        COMMAND rm -rf gcovr
    )
    add_dependencies(TARGET_GCOV ${testname})
endforeach(testsourcefile ${TESTS})

# Add bootloader test to gcov dependency chain
add_dependencies(TARGET_GCOV openlcb_bootloader_Test)

# Generate the HTML coverage report
add_custom_command(OUTPUT gcovr/coverage.html
    COMMAND mkdir -p gcovr
    COMMAND gcovr --gcov-ignore-parse-errors=all --html-details gcovr/coverage.html --exclude=_deps.* --exclude=.*_Test.* -r ${ROOT_DIR} .
    COMMAND ln -sf ${CMAKE_SOURCE_DIR}/build/gcovr/coverage.html ${CMAKE_SOURCE_DIR}/
    VERBATIM
)
