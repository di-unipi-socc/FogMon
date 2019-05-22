# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_CXX_FLAGS -m32)
set(CMAKE_C_FLAGS -m32)


# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_LIBRARY_PATH "/usr/lib/" CACHE PATH "<desc>")
set(ENV{CMAKE_LIBRARY_PATH} "/usr/lib/")
cmake_minimum_required(VERSION 2.8.0)

project(FogMon CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/node ${CMAKE_CURRENT_SOURCE_DIR}/master_node ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include ${CMAKE_CURRENT_SOURCE_DIR}/sigar/include)

file(GLOB SRCS "*.cpp")
file(GLOB HDRS "*.hpp")

find_library(SQLITE_LIBRARY sqlite3)
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
set_target_properties(FogMonNode PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

file(GLOB SRCS_MASTER "master_node/*.cpp")
file(GLOB HDRS_MASTER "master_node/*.hpp")

add_executable(FogMonServer main/master_main.cpp ${SRCS} ${HDRS} ${SRCS_NODE} ${HDRS_NODE} ${SRCS_MASTER} ${HDRS_MASTER} ${SRCS_MICROBIT} ${HDRS_MICROBIT})
set_target_properties(FogMonServer PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

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