project(utilities C)
set(SOURCES mustangpeak_string_helper.c)

if (${GTEST})
set(TESTS ${TESTS}
    ${ROOT_DIR}/src/utilities/mustangpeak_string_helper_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)
