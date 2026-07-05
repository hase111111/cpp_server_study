//! @file util.cpp
//! @brief Utility helper functions implementation
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#include "util.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

namespace util {

namespace {

constexpr int kPort = 8080;
constexpr const char* kHost = "0.0.0.0";

}  // namespace

std::string ReadFile(const std::string& path) {
    std::filesystem::path requestedPath(path);
    if (requestedPath.is_absolute()) {
        requestedPath = requestedPath.lexically_normal();
    } else {
        requestedPath = std::filesystem::current_path() / requestedPath;
        requestedPath = requestedPath.lexically_normal();
    }

    std::ifstream file(requestedPath, std::ios::binary);
    if (!file) {
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string GetContentType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") {
        return "text/html; charset=utf-8";
    }
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") {
        return "application/javascript; charset=utf-8";
    }
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".wasm") {
        return "application/wasm";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") {
        return "text/css; charset=utf-8";
    }
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".json") {
        return "application/json; charset=utf-8";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") {
        return "image/png";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") {
        return "image/jpeg";
    }
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".svg") {
        return "image/svg+xml";
    }
    return "application/octet-stream";
}

bool SendAll(int socketFd, const std::string& data) {
    const char* ptr = data.data();
    size_t totalSent = 0;
    while (totalSent < data.size()) {
        ssize_t sent = send(socketFd, ptr + totalSent, data.size() - totalSent, 0);
        if (sent <= 0) {
            return false;
        }
        totalSent += static_cast<size_t>(sent);
    }
    return true;
}

void HandleClient(int clientSocket) {
    char buffer[4096];
    std::string request;

    while (true) {
        ssize_t received = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            break;
        }
        request.append(buffer, static_cast<size_t>(received));
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    std::istringstream requestStream(request);
    std::string method, path, version;
    requestStream >> method >> path >> version;

    std::cout << "Request: " << method << " " << path << "\n";

    std::string body;
    std::string statusLine;
    std::string contentType;
    std::string response;
    const bool isHeadRequest = (method == "HEAD");

    if (method == "GET" || method == "HEAD") {
        std::string requestedPath = path;
        const size_t queryPos = requestedPath.find('?');
        if (queryPos != std::string::npos) {
            requestedPath = requestedPath.substr(0, queryPos);
        }

        if (requestedPath.empty() || requestedPath == "/") {
            requestedPath = "index.html";
        } else if (requestedPath[0] == '/') {
            requestedPath = requestedPath.substr(1);
        }

        if (requestedPath.find("..") != std::string::npos) {
            body = "<h1>Forbidden</h1>";
            statusLine = "HTTP/1.1 403 Forbidden\r\n";
            contentType = "text/plain; charset=utf-8";
        } else {
            body = ReadFile(requestedPath);
            if (!body.empty()) {
                statusLine = "HTTP/1.1 200 OK\r\n";
                contentType = GetContentType(requestedPath);
            } else {
                body = "<h1>404 Not Found</h1>";
                statusLine = "HTTP/1.1 404 Not Found\r\n";
                contentType = "text/plain; charset=utf-8";
            }
        }
    } else {
        body = "<h1>404 Not Found</h1>";
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        contentType = "text/plain; charset=utf-8";
    }

    std::ostringstream header;
    header << statusLine
           << "Content-Type: " << contentType << "\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n"
           << "\r\n";

    if (isHeadRequest) {
        body.clear();
    }

    response = header.str() + body;
    SendAll(clientSocket, response);
    close(clientSocket);
}

}  // namespace util
