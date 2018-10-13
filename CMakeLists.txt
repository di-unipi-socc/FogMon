cmake_minimum_required(VERSION 3.0.0)
project(FTPiDiscoverer CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

file(GLOB SRCS_NODE "node/*.cpp")
file(GLOB HDRS_NODE "node/*.hpp")

add_executable(FTPiDiscovererNode ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE})

file(GLOB SRCS_SERVER "server/*.cpp")
file(GLOB HDRS_SERVER "server/*.hpp")

add_executable(FTPiDiscovererServer ${SRCS_SERVER} ${HDRS_SERVER})

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(FTPiDiscovererNode ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscovererNode ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscovererNode libsigar.a)
target_link_libraries(FTPiDiscovererNode -static-libstdc++)