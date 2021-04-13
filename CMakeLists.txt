cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
endif()

project(FogMon CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/follower ${CMAKE_CURRENT_SOURCE_DIR}/src/leader ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "src/*.cpp")
file(GLOB HDRS "src/*.hpp")

find_library(UUID_LIBRARY uuid)
find_library(PTHREAD_LIBRARY pthread)
find_library(SERIALPORT_LIBRARY serialport)
find_library(SIGAR_LIBRARY sigar)
find_library(SQLITE_LIBRARY sqlite3)
find_library(CURL_LIBRARY curl)

message(${UUID_LIBRARY})
message(${PTHREAD_LIBRARY})
message(${SQLITE_LIBRARY})
message(${SERIALPORT_LIBRARY})
message(${SIGAR_LIBRARY})
message(${CURL_LIBRARY})

file(GLOB SRCS_NODE "src/follower/*.cpp")
file(GLOB HDRS_NODE "src/follower/*.hpp")

file(GLOB SRCS_MICROBIT "src/microbit/*.cpp")
file(GLOB HDRS_MICROBIT "src/microbit/*.hpp")

file(GLOB SRCS_LEADER "src/leader/*.cpp")
file(GLOB HDRS_LEADER "src/leader/*.hpp")

add_executable(FogMon src/main/main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER}  ${SRCS_MICROBIT} ${HDRS_MICROBIT})

#add_executable(FogMonNode src/main/node_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER}  ${SRCS_MICROBIT} ${HDRS_MICROBIT})

#add_executable(FogMonServer src/main/leader_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(FogMon ${UUID_LIBRARY})
target_link_libraries(FogMon ${SQLITE_LIBRARY})
target_link_libraries(FogMon ${PTHREAD_LIBRARY})
target_link_libraries(FogMon ${SIGAR_LIBRARY})
target_link_libraries(FogMon ${SERIALPORT_LIBRARY})
target_link_libraries(FogMon ${CURL_LIBRARY})

#target_link_libraries(FogMonNode ${UUID_LIBRARY})
#target_link_libraries(FogMonNode ${SQLITE_LIBRARY})
#target_link_libraries(FogMonNode ${PTHREAD_LIBRARY})
#target_link_libraries(FogMonNode ${SIGAR_LIBRARY})
#target_link_libraries(FogMonNode ${SERIALPORT_LIBRARY})

#target_link_libraries(FogMonServer ${UUID_LIBRARY})
#target_link_libraries(FogMonServer ${SQLITE_LIBRARY})
#target_link_libraries(FogMonServer ${PTHREAD_LIBRARY})
#target_link_libraries(FogMonServer ${SIGAR_LIBRARY})
#target_link_libraries(FogMonServer ${SERIALPORT_LIBRARY})

add_subdirectory(sqlite3)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions("-DENABLE_TESTS")
	add_subdirectory(tests)
endif()