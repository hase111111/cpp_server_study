//! @file http_server.cpp
//! @brief HTTP server implementation
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#include "http_server.hpp"
#include "util.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

namespace {

constexpr int kPort = 8080;
constexpr const char* kHost = "0.0.0.0";

}  // namespace

int my_cpp_server::Run() {
    signal(SIGPIPE, SIG_IGN);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "failed to create socket" << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    const char* hostEnv = std::getenv("HOST");
    const char* portEnv = std::getenv("PORT");
    const char* host = (hostEnv && *hostEnv) ? hostEnv : kHost;
    int port = kPort;
    if (portEnv && *portEnv) {
        port = std::atoi(portEnv);
    }
    if (port <= 0 || port > 65535) {
        port = kPort;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<uint16_t>(port));
    serverAddress.sin_addr.s_addr = inet_addr(host);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) < 0) {
        std::cerr << "listen failed" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server running at http://" << host << ":" << port << std::endl;

    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
        if (clientSocket < 0) {
            std::cerr << "accept failed" << std::endl;
            continue;
        }

        std::thread([clientSocket]() {
            util::HandleClient(clientSocket);
        }).detach();
    }

    close(serverSocket);
    return 0;
}
