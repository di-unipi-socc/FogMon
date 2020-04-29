cmake_minimum_required(VERSION 2.8.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FogMon CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/follower ${CMAKE_CURRENT_SOURCE_DIR}/leader ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

find_library(UUID_LIBRARY uuid)
find_library(PTHREAD_LIBRARY pthread)
find_library(SERIALPORT_LIBRARY serialport)
find_library(SIGAR_LIBRARY sigar)
find_library(SQLITE_LIBRARY sqlite3)

message(${UUID_LIBRARY})
message(${PTHREAD_LIBRARY})
message(${SQLITE_LIBRARY})
message(${SERIALPORT_LIBRARY})
message(${SIGAR_LIBRARY})

file(GLOB SRCS_NODE "follower/*.cpp")
file(GLOB HDRS_NODE "follower/*.hpp")

file(GLOB SRCS_MICROBIT "microbit/*.cpp")
file(GLOB HDRS_MICROBIT "microbit/*.hpp")

file(GLOB SRCS_LEADER "leader/*.cpp")
file(GLOB HDRS_LEADER "leader/*.hpp")

add_executable(FogMon main/main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER}  ${SRCS_MICROBIT} ${HDRS_MICROBIT})

#add_executable(FogMonNode main/node_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER}  ${SRCS_MICROBIT} ${HDRS_MICROBIT})

#add_executable(FogMonServer main/leader_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_LEADER} ${HDRS_LEADER} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(FogMon ${UUID_LIBRARY})
target_link_libraries(FogMon ${SQLITE_LIBRARY})
target_link_libraries(FogMon ${PTHREAD_LIBRARY})
target_link_libraries(FogMon ${SIGAR_LIBRARY})
target_link_libraries(FogMon ${SERIALPORT_LIBRARY})

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
if(${ENABLE_TESTS})
	add_definitions("-DENABLE_TESTS")
	add_subdirectory(tests)
endif()