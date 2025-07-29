project(drivers C)

set(SOURCES driver_100ms_clock.c driver_can.c driver_configuration_memory.c driver_mcu.c)


if (${GTEST})
set(TESTS ${TESTS}
    
    ${ROOT_DIR}/src/drivers/driver_100ms_clock_Test.cxx
    ${ROOT_DIR}/src/drivers/driver_can_Test.cxx
    ${ROOT_DIR}/src/drivers/driver_configuration_memory_Test.cxx
    ${ROOT_DIR}/src/drivers/driver_mcu_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
