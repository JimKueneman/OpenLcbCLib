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

# GTest and GMock includes
include_directories(
    ${CMAKE_SOURCE_DIR}/ ${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include
)

# Add libraries that have tests disabled.

# Add libraries that have test enabled.
set(GTEST ON)
add_subdirectory(${ROOT_DIR}/src/utilities utilities)
add_subdirectory(${ROOT_DIR}/src/drivers/common/ common_drivers)
add_subdirectory(${ROOT_DIR}/src/drivers/ drivers)
add_subdirectory(${ROOT_DIR}/src/openlcb/ openlcb)


set(LINK_LIBS ${LINK_LIBS}
    utilities
    common_drivers
    drivers
    openlcb
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

# Generate the HTML coverage report
add_custom_command(OUTPUT gcovr/coverage.html
    COMMAND mkdir -p gcovr
    COMMAND gcovr --gcov-ignore-parse-errors=all --html-details gcovr/coverage.html --exclude=_deps.* --exclude=.*_Test.* -r ${ROOT_DIR} .
    COMMAND ln -sf ${CMAKE_SOURCE_DIR}/build/gcovr/coverage.html ${CMAKE_SOURCE_DIR}/
    VERBATIM
)
