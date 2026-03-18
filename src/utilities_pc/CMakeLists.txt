project(utilities_pc C)
set(SOURCES threadsafe_stringlist.c)

find_package(Threads REQUIRED)

if (${GTEST})
set(TESTS ${TESTS}
    ${ROOT_DIR}/src/utilities_pc/threadsafe_stringlist_Test.cxx

    PARENT_SCOPE
)
endif()

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ROOT_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    Threads::Threads
)
