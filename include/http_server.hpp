//! @file http_server.hpp
//! @brief HTTP server implementation
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#pragma once

namespace my_cpp_server {

class HttpServer final {
 public:
  HttpServer();
  ~HttpServer();

  int Run();

 private:
  int InitializeSocket();
  int BindAndListen(const char* host, int port);
  void AcceptLoop();

  int server_socket_{-1};
};

}  // namespace my_cpp_server
