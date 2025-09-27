project(drivers C)

set(
SOURCES 
    alias_mappings.c
)


if (${GTEST})
set(TESTS ${TESTS}
    
    ${ROOT_DIR}/src/drivers/alias_mappings_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
