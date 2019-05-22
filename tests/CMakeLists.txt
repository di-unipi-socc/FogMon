cmake_minimum_required(VERSION 2.8.0)
project(FogMon_Tests CXX)

set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_C_FLAGS -m32)

ADD_SUBDIRECTORY (googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../node ${CMAKE_CURRENT_SOURCE_DIR}/../master_node)

file(GLOB UNIT_SRCS "unit/*.cpp")
file(GLOB INTEGRATION_SRCS "integration/*.cpp")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp")

file(GLOB SRCS_NODE "${CMAKE_CURRENT_SOURCE_DIR}/../node/*.cpp")

file(GLOB SRCS_MASTER "${CMAKE_CURRENT_SOURCE_DIR}/../master_node/*.cpp")

file(GLOB SRCS_MICROBIT "${CMAKE_CURRENT_SOURCE_DIR}/../microbit/*.cpp")

SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )


add_executable(UnitTests main_test.cpp ${UNIT_SRCS} ${INTEGRATION_SRCS} ${SRCS} ${SRCS_MASTER} ${SRCS_NODE} ${SRCS_MICROBIT})
set_target_properties(UnitTests PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")


find_library(SQLITE_LIBRARY sqlite3)
find_library(SERIALPORT_LIBRARY serialport)
find_library(PTHREAD_LIBRARY pthread)

find_library(SIGAR_LIBRARY sigar)
target_link_libraries(UnitTests ${SQLITE_LIBRARY})
target_link_libraries(UnitTests gtest ${PTHREAD_LIBRARY} ${SIGAR_LIBRARY})
target_link_libraries(UnitTests ${SERIALPORT_LIBRARY})
add_test( UnitTests UnitTests )