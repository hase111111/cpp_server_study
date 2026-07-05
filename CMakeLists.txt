cmake_minimum_required(VERSION 3.16)
project(cpp_server_study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(web_server
    ${PROJECT_SOURCES}
)

target_include_directories(web_server PRIVATE include)

target_sources(web_server PRIVATE ${PROJECT_HEADERS})
