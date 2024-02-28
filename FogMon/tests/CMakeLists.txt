cmake_minimum_required(VERSION 3.16.0)
project(FogMon_Tests CXX)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/follower ${CMAKE_CURRENT_SOURCE_DIR}/../src/leader)

file(GLOB UNIT_SRCS "unit/*.cpp")
file(GLOB INTEGRATION_SRCS "integration/*.cpp")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp")

file(GLOB SRCS_NODE "${CMAKE_CURRENT_SOURCE_DIR}/../src/follower/*.cpp")

file(GLOB SRCS_LEADER "${CMAKE_CURRENT_SOURCE_DIR}/../src/leader/*.cpp")

file(GLOB SRCS_MICROBIT "${CMAKE_CURRENT_SOURCE_DIR}/../src/microbit/*.cpp")



SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")

SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-coverage -lgcov")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -fprofile-arcs -ftest-coverage" )


add_executable(Tests main_test.cpp ${UNIT_SRCS} ${INTEGRATION_SRCS} ${SRCS} ${SRCS_LEADER} ${SRCS_NODE} ${SRCS_MICROBIT})

find_library(SQLITE_LIBRARY sqlite3)
find_library(SERIALPORT_LIBRARY serialport)
find_library(PTHREAD_LIBRARY pthread)
find_library(SIGAR_LIBRARY sigar)
find_library(CURL_LIBRARY curl)
find_library(UUID_LIBRARY uuid)

target_link_libraries(Tests ${UUID_LIBRARY})
target_link_libraries(Tests ${SQLITE_LIBRARY})
target_link_libraries(Tests ${PTHREAD_LIBRARY} ${SIGAR_LIBRARY})
target_link_libraries(Tests ${SERIALPORT_LIBRARY})
target_link_libraries(Tests ${CURL_LIBRARY})
target_link_libraries(Tests GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests( Tests )

if(CMAKE_COMPILER_IS_GNUCXX)
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake_modules")
include(CodeCoverage)
APPEND_COVERAGE_COMPILER_FLAGS()
SETUP_TARGET_FOR_COVERAGE_LCOV(NAME tests_coverage
                              EXECUTABLE Tests
                              DEPENDENCIES Tests
                              BASE_DIRECTORY "../src/"
                              EXCLUDE "../rapidjson/*" "../tests/*" "/usr/*")
endif()

add_custom_command(TARGET Tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:Tests>/scripts)
add_custom_command(TARGET Tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_BINARY_DIR}/../libsqlitefunctions.so $<TARGET_FILE_DIR:Tests>)
add_custom_command(TARGET Tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/test/assolo_rcv $<TARGET_FILE_DIR:Tests>)
add_custom_command(TARGET Tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/test/assolo_run $<TARGET_FILE_DIR:Tests>)
add_custom_command(TARGET Tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/test/assolo_snd $<TARGET_FILE_DIR:Tests>)