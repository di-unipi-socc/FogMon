cmake_minimum_required(VERSION 2.8.0)
project(FTPiDiscoverer_Tests CXX)

ADD_SUBDIRECTORY (googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(runUnitTests main_test.cpp unit/queueTest.cpp)

target_link_libraries(runUnitTests gtest pthread)
add_test( runUnitTests runUnitTests )