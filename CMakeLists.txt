cmake_minimum_required(VERSION 3.16)
project(cpp_server_study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(web_server
    main.cpp
    src/http_server.cpp
    src/util.cpp
)

target_include_directories(web_server PRIVATE include)
