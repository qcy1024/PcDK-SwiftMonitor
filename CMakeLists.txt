# PcDK-SwiftMonitor
cmake_minimum_required(VERSION 3.10)

project(requirement)

set(CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/app/*.cpp ${PROJECT_SOURCE_DIR}/core/*.cpp ${PROJECT_SOURCE_DIR}/IOWrap/*.cpp)

include_directories(/usr/local/include/pcapplusplus ${PROJECT_SOURCE_DIR}/include)

link_directories(/usr/local/lib)

add_executable("${PROJECT_NAME}" ${SRC_LIST})

target_link_libraries("${PROJECT_NAME}" Pcap++ Packet++ Common++ pcap)

