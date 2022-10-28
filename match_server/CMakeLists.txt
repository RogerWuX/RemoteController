cmake_minimum_required(VERSION 3.16.0)
project(match_server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include/asio)

add_executable(MatchServer
    main.cpp
    Server.cpp
    Session.cpp
    ../protocol/signalling.pb.cc
)

target_compile_definitions(MatchServer PRIVATE ASIO_STANDALONE)
target_precompile_headers(MatchServer PRIVATE stdafx.h)

target_link_libraries(MatchServer PRIVATE /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(MatchServer PRIVATE /usr/lib/x86_64-linux-gnu/libcrypto.a -ldl)

target_link_libraries(MatchServer PRIVATE /usr/local/lib/libprotobuf.a)
target_link_libraries(MatchServer PRIVATE -pthread)

