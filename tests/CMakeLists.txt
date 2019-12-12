cmake_minimum_required(VERSION 2.8.0)
project(FogMon_Tests CXX)

ADD_SUBDIRECTORY (googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../follower ${CMAKE_CURRENT_SOURCE_DIR}/../leader)

file(GLOB UNIT_SRCS "unit/*.cpp")
file(GLOB INTEGRATION_SRCS "integration/*.cpp")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp")

file(GLOB SRCS_NODE "${CMAKE_CURRENT_SOURCE_DIR}/../follower/*.cpp")

file(GLOB SRCS_LEADER "${CMAKE_CURRENT_SOURCE_DIR}/../leader/*.cpp")

file(GLOB SRCS_MICROBIT "${CMAKE_CURRENT_SOURCE_DIR}/../microbit/*.cpp")

SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )


add_executable(UnitTests main_test.cpp ${UNIT_SRCS} ${INTEGRATION_SRCS} ${SRCS} ${SRCS_LEADER} ${SRCS_NODE} ${SRCS_MICROBIT})


find_library(SQLITE_LIBRARY sqlite3)
find_library(SERIALPORT_LIBRARY serialport)
find_library(PTHREAD_LIBRARY pthread)
find_library(SIGAR_LIBRARY sigar)

find_library(UUID_LIBRARY uuid)
target_link_libraries(UnitTests ${UUID_LIBRARY})
target_link_libraries(UnitTests ${SQLITE_LIBRARY})
target_link_libraries(UnitTests gtest ${PTHREAD_LIBRARY} ${SIGAR_LIBRARY})
target_link_libraries(UnitTests ${SERIALPORT_LIBRARY})
add_test( UnitTests UnitTests )