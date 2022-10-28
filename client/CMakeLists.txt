cmake_minimum_required(VERSION 3.16.0)
project(client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH,"/usr/include/x86_64-linux-gnu/qt5")                                                                                                                                                     
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
  
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Gui Widgets Network REQUIRED)

add_compile_options(-pthread)

add_executable(Client
    main.cpp
    Application.cpp
    MatchClient.cpp
    RtcClient.cpp
    Agent.cpp
    AgentClient.cpp
    XControl.cpp
    Controller.cpp
    ControllerClient.cpp
    ControllerWnd.cpp
    MatchWnd.cpp
    ../protocol/signalling.pb.cc
    ../protocol/p2p.pb.cc
    Logger.cpp
)


target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/include/webrtc)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/include/webrtc/third_party/abseil-cpp)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/include/webrtc/third_party/libyuv/include)

target_precompile_headers(Client PRIVATE stdafx.h)

target_link_libraries(Client PRIVATE -lXtst)
target_link_libraries(Client PRIVATE ${CMAKE_SOURCE_DIR}/lib/libwebrtc.a -pthread -lglib-2.0 -ldl -lX11)
target_link_libraries(Client PRIVATE Qt5::Widgets Qt5::Network)
target_link_libraries(Client PRIVATE /usr/local/lib/libprotobuf.a)


