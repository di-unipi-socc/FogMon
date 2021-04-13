cmake_minimum_required(VERSION 2.4.0)

project(sqlitefunctions)

add_library(sqlitefunctions SHARED extension-functions.c)

set_target_properties(sqlitefunctions
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
)

