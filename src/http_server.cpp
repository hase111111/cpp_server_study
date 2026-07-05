//! @file http_server.cpp
//! @brief HTTP server implementation
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#include "http_server.hpp"
#include "util.hpp"
#include "constants.hpp"

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


namespace my_cpp_server {

HttpServer::HttpServer() {}

HttpServer::~HttpServer() {
    // サーバーソケットが有効な場合は閉じる。
    if (server_socket_ >= 0) {
        close(server_socket_);
    }
}

int HttpServer::Run() {
    // SIGPIPEシグナルを無視する。
    // これにより、クライアントが接続を切断した場合でも、サーバーがクラッシュしないようにする。
    signal(SIGPIPE, SIG_IGN);

    // ソケットを初期化する。
    if (InitializeSocket() != 0) {
        return 1;
    }

    // 環境変数からホストとポートを取得する。指定されていない場合はデフォルト値を使用する。
    const char* host_env{std::getenv("HOST")};
    const char* port_env{std::getenv("PORT")};
    const char* host{(host_env && *host_env) ? host_env : constants::kHost};

    int port{constants::kPort};
    if (port_env && *port_env) {
        port = std::atoi(port_env);
    }
    if (port <= 0 || port > 65535) {
        port = constants::kPort;
    }

    // ソケットをバインドしてリッスンする
    if (BindAndListen(host, port) != 0) {
        return 1;
    }

    std::cout << "Server running at http://" << host << ":" << port << std::endl;
    AcceptLoop();

    close(server_socket_);
    return 0;
}

int HttpServer::InitializeSocket() {
    // ソケットを作成する。
    // AF_INET: IPv4, SOCK_STREAM: TCP, 0: プロトコルは自動選択
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket_ < 0) {
        std::cerr << "failed to create socket" << std::endl;
        return 1;
    }

    // ソケットオプションを設定する。
    // SOL_SOCKET: ソケットレベルのオプション,  SO_REUSEADDR: アドレスの再利用を許可
    // これにより、サーバーを再起動する際に "Address already in use" エラーを回避できる。
    int opt{1};
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return 0;
}

int HttpServer::BindAndListen(const char* host, const int port) {
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<uint16_t>(port));
    serverAddress.sin_addr.s_addr = inet_addr(host);

    if (bind(server_socket_, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return 1;
    }

    if (listen(server_socket_, 10) < 0) {
        std::cerr << "listen failed" << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return 1;
    }

    return 0;
}

void HttpServer::AcceptLoop() {
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLen{sizeof(clientAddress)};
        int clientSocket{accept(server_socket_, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen)};

        if (clientSocket < 0) {
            std::cerr << "accept failed" << std::endl;
            continue;
        }

        std::thread([clientSocket]() {
            util::HandleClient(clientSocket);
        }).detach();
    }
}

}  // namespace my_cpp_server
