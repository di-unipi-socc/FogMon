cmake_minimum_required(VERSION 2.8.0)
project(FTPiDiscoverer CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

find_library(SQLITE_LIBRARY sqlite3)
find_library(PTHREAD_LIBRARY pthread)

file(GLOB SRCS_NODE "node/*.cpp")
file(GLOB HDRS_NODE "node/*.hpp")

add_executable(FTPiDiscovererNode ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE})

file(GLOB SRCS_MASTER "master_node/*.cpp")
file(GLOB HDRS_MASTER "master_node/*.hpp")

add_executable(FTPiDiscovererServer ${SRCS} ${HDRS} ${SRCS_MASTER} ${HDRS_MASTER})

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(FTPiDiscovererNode ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscovererNode ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscovererNode libsigar.a)
target_link_libraries(FTPiDiscovererNode -static-libstdc++)

target_link_libraries(FTPiDiscovererServer ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscovererServer ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscovererServer libsigar.a)
target_link_libraries(FTPiDiscovererServer -static-libstdc++)

add_subdirectory(sqlite3)
