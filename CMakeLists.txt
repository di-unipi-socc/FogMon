cmake_minimum_required(VERSION 2.8.0)
project(FogMon CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/node ${CMAKE_CURRENT_SOURCE_DIR}/master_node ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

find_library(PTHREAD_LIBRARY pthread)
find_library(SERIALPORT_LIBRARY serialport)
find_library(SIGAR_LIBRARY sigar)

message(${PTHREAD_LIBRARY})
message(${SQLITE_LIBRARY})
message(${SERIALPORT_LIBRARY})
message(${SIGAR_LIBRARY})

file(GLOB SRCS_NODE "node/*.cpp")
file(GLOB HDRS_NODE "node/*.hpp")

file(GLOB SRCS_MICROBIT "microbit/*.cpp")
file(GLOB HDRS_MICROBIT "microbit/*.hpp")

add_executable(FogMonNode main/node_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

file(GLOB SRCS_MASTER "master_node/*.cpp")
file(GLOB HDRS_MASTER "master_node/*.hpp")

add_executable(FogMonServer main/master_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_MASTER} ${HDRS_MASTER} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

include_directories(/usr/include)
link_directories(/usr/lib)



target_link_libraries(FogMonNode ${SQLITE_LIBRARY})
target_link_libraries(FogMonNode ${PTHREAD_LIBRARY})
target_link_libraries(FogMonNode ${SIGAR_LIBRARY})
target_link_libraries(FogMonNode ${SERIALPORT_LIBRARY})

target_link_libraries(FogMonServer ${SQLITE_LIBRARY})
target_link_libraries(FogMonServer ${PTHREAD_LIBRARY})
target_link_libraries(FogMonServer ${SIGAR_LIBRARY})
target_link_libraries(FogMonServer ${SERIALPORT_LIBRARY})

add_subdirectory(sqlite3)
if(${ENABLE_TESTS})
	add_definitions("-DENABLE_TESTS")
	add_subdirectory(tests)
endif()