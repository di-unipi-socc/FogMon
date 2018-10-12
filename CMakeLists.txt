cmake_minimum_required(VERSION 3.0.0)
project(FTPiDiscoverer CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

add_executable(FTPiDiscoverer ${SRCS} ${HDRS})

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(FTPiDiscoverer ${SQLITE_LIBRARY})
target_link_libraries(FTPiDiscoverer ${PTHREAD_LIBRARY})
target_link_libraries(FTPiDiscoverer libsigar.a)
target_link_libraries(FTPiDiscoverer -static-libstdc++)