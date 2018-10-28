cmake_minimum_required(VERSION 2.4.0)

project(libsqlite_extension)

add_library(libsqlite_extension SHARED extension-functions.c)

set_target_properties(libsqlite_extension
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
)

