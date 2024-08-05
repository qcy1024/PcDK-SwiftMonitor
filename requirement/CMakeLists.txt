cmake_minimum_required(VERSION 3.10)
project(requirement)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST ${PROJECT_SOURCE_DIR}/*.cpp)

include_directories(/usr/local/include/pcapplusplus)

link_directories(/usr/local/lib)

add_executable(main ${SRC_LIST})

target_link_libraries(main Pcap++ Packet++ Common++ pcap)

