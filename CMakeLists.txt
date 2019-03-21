cmake_minimum_required(VERSION 2.8.0)
project(FTPiDiscoverer CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/node ${CMAKE_CURRENT_SOURCE_DIR}/master_node ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

find_library(SQLITE_LIBRARY sqlite3)
find_library(PTHREAD_LIBRARY pthread)
find_library(SERIALPORT_LIBRARY serialport)

file(GLOB SRCS_NODE "node/*.cpp")
file(GLOB HDRS_NODE "node/*.hpp")

file(GLOB SRCS_MICROBIT "microbit/*.cpp")
file(GLOB HDRS_MICROBIT "microbit/*.hpp")

add_executable(FTPiDiscovererNode main/node_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

file(GLOB SRCS_MASTER "master_node/*.cpp")
file(GLOB HDRS_MASTER "master_node/*.hpp")

add_executable(FTPiDiscovererServer main/master_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_MASTER} ${HDRS_MASTER} ${SRCS_MICROBIT} ${HDRS_MICROBIT})

include_directories(/usr/include)
link_directories(/usr/lib)

if(${LINK_TIRPC})
	target_link_libraries(FTPiDiscovererNode tirpc)
	target_link_libraries(FTPiDiscovererServer tirpc)
endif()

target_link_libraries(FTPiDiscovererNode ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscovererNode ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscovererNode libsigar.a)
target_link_libraries(FTPiDiscovererNode -static-libstdc++)
target_link_libraries(FTPiDiscovererNode ${SERIALPORT_LIBRARY})

target_link_libraries(FTPiDiscovererServer ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscovererServer ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscovererServer libsigar.a)
target_link_libraries(FTPiDiscovererServer -static-libstdc++)
target_link_libraries(FTPiDiscovererServer ${SERIALPORT_LIBRARY})

add_subdirectory(sqlite3)
if(${ENABLE_TESTS})
	add_definitions("-DENABLE_TESTS")
	add_subdirectory(tests)
endif()