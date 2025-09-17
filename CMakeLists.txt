cmake_minimum_required(VERSION 3.10)
project(TinyShell C)

# Option to build the main executable (for development)
option(TINYSHELL_BUILD_MAIN "Build TinyShell main executable" ON)

# Library source files
set(TINYSHELL_SRC
    src/tinyshell.c
    # Add other library source files here
)

# Create the library target
add_library(tinyshell STATIC ${TINYSHELL_SRC})
target_include_directories(tinyshell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Only build the main executable if requested
if(TINYSHELL_BUILD_MAIN)
    add_executable(tinyshell_main main.c)
    target_link_libraries(tinyshell_main PRIVATE tinyshell)
    target_include_directories(tinyshell_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


    enable_testing()
    add_executable(test_tinyshell tests/test_tinyshell.c)
    target_link_libraries(test_tinyshell PRIVATE tinyshell)
    target_include_directories(test_tinyshell PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME tinyshellTest COMMAND test_tinyshell)
endif()

