cmake_minimum_required(VERSION 3.14)
project(bootloader_openlcb C)

set(SOURCES
    bootloader.c
    bootloader_openlcb_statemachine.c
)

if (${GTEST})
set(TESTS ${TESTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_Test.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_openlcb_statemachine_Test.cxx
    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
