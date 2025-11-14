cmake_minimum_required(VERSION 3.15)
project(enet C)

# ENet source files
add_library(enet STATIC
    callbacks.c
    compress.c
    host.c
    list.c
    packet.c
    peer.c
    protocol.c
    unix.c
    win32.c
)

target_include_directories(enet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Platform-specific configuration
if(WIN32)
    target_link_libraries(enet PRIVATE ws2_32 winmm)
    target_compile_definitions(enet PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
else()
    target_compile_definitions(enet PRIVATE HAS_SOCKLEN_T)
endif()

# Compiler warnings
target_compile_options(enet PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
)
