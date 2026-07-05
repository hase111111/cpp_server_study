//! @file main.cpp
//! @brief Entry point for the HTTP server application
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#include "http_server.hpp"

int main() {
    using my_cpp_server::HttpServer;

    // サーバーを起動する
    HttpServer server{};
    
    return server.Run();
}
