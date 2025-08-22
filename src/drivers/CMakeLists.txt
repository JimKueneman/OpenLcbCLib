project(drivers C)

set(
    SOURCES 
)


if (${GTEST})
set(TESTS ${TESTS}
    

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
