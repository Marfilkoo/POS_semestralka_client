cmake_minimum_required(VERSION 3.18)
project(socket_clients)

set(CMAKE_CXX_STANDARD 17)

add_executable(socket_clients main.cpp my_socket.cpp my_socket.h)

target_link_libraries(socket_clients wsock32 ws2_32)
