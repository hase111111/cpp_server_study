//! @file util.cpp
//! @brief Utility helper functions implementation
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#include "util.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>

#include "constants.hpp"

namespace my_cpp_server::util {

std::filesystem::path ResolvePath(const std::string& path) {
    std::filesystem::path requested_path(path);

    if (requested_path.is_absolute()) {
        requested_path = requested_path.lexically_normal();
    } else {
        requested_path = std::filesystem::path(constants::kPublicDir) / requested_path;
        requested_path = requested_path.lexically_normal();
    }

    return requested_path;
}

std::string ReadFile(const std::string& path) {
    const std::filesystem::path requestedPath{ResolvePath(path)};

    std::ifstream file(requestedPath, std::ios::binary);
    if (!file) {
        return "";
    }

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string GetContentType(const std::string& path) {
    const std::unordered_map<std::string, std::string> content_types{
        {".html", "text/html; charset=utf-8"},
        {".js", "application/javascript; charset=utf-8"},
        {".wasm", "application/wasm"},
        {".css", "text/css; charset=utf-8"},
        {".json", "application/json; charset=utf-8"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".svg", "image/svg+xml"},
    };

    const size_t last_dot{path.find_last_of('.')};
    if (last_dot == std::string::npos) {
        return "application/octet-stream";
    }

    const std::string extension{path.substr(last_dot)};
    if (const auto it{content_types.find(extension)}; it != content_types.end()) {
        return it->second;
    }

    return "application/octet-stream";
}

bool SendAll(const int socket_fd, const std::string& data) {
    const char* const ptr{data.data()};
    size_t total_sent{0};

    while (total_sent < data.size()) {
        const ssize_t sent{send(socket_fd, ptr + total_sent, data.size() - total_sent, 0)};
        if (sent <= 0) {
            return false;
        }
        total_sent += static_cast<size_t>(sent);
    }

    return true;
}

void HandleClient(const int client_socket) {
    std::string request{};
    
    {
        char buffer[4096];
        while (true) {
            const ssize_t received{recv(client_socket, buffer, sizeof(buffer), 0)};
            if (received <= 0) {
                break;
            }
            request.append(buffer, static_cast<size_t>(received));
            if (request.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
    }

    std::istringstream request_stream(request);
    std::string method{}, path{}, version{};
    request_stream >> method >> path >> version;

    std::cout << "Request Raw Text: " << std::endl 
        << request << std::endl << "------------------------" << std::endl;
    std::cout << "Request: " << method << " " << path << " " << version << std::endl
        << "------------------------" << std::endl;

    std::string body{}, status_line{}, content_type{}, response{};
    const bool is_head_request{(method == "HEAD")};

    if (method == "GET" || method == "HEAD") {
        std::string requested_path{path};
        const size_t queryPos{requested_path.find('?')};
        if (queryPos != std::string::npos) {
            requested_path = requested_path.substr(0, queryPos);
        }

        if (requested_path.empty() || requested_path == "/") {
            requested_path = "index.html";
        } else if (requested_path[0] == '/') {
            requested_path = requested_path.substr(1);
        }

        const std::filesystem::path resolved_path{ResolvePath(requested_path)};

        if (requested_path.find("..") != std::string::npos) {
            body = "<h1>Forbidden</h1>";
            status_line = "HTTP/1.1 403 Forbidden\r\n";
            content_type = "text/plain; charset=utf-8";
        } else {
            body = ReadFile(requested_path);
            if (!body.empty()) {
                status_line = "HTTP/1.1 200 OK\r\n";
                content_type = GetContentType(requested_path);
            } else {
                body = "<h1>404 Not Found</h1>";
                status_line = "HTTP/1.1 404 Not Found\r\n";
                content_type = "text/plain; charset=utf-8";
            }
        }
    } else {
        body = "<h1>404 Not Found</h1>";
        status_line = "HTTP/1.1 404 Not Found\r\n";
        content_type = "text/plain; charset=utf-8";
    }

    std::ostringstream header{};
    header << status_line
           << "Content-Type: " << content_type << "\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n"
           << "\r\n";

    if (is_head_request) {
        body.clear();
    }

    response = header.str() + body;

    SendAll(client_socket, response);
    close(client_socket);
}

}  // namespace my_cpp_server::util
