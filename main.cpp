#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace {

constexpr int kPort = 8080;
constexpr const char* kHost = "0.0.0.0";

std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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

    if (method == "GET" && (path == "/" || path == "/index.html")) {
        body = ReadFile("index.html");
        if (!body.empty()) {
            statusLine = "HTTP/1.1 200 OK\r\n";
            contentType = "text/html; charset=utf-8";
        } else {
            body = "<h1>index.html not found</h1>";
            statusLine = "HTTP/1.1 404 Not Found\r\n";
            contentType = "text/plain; charset=utf-8";
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

    response = header.str() + body;
    SendAll(clientSocket, response);
    close(clientSocket);
}

}  // namespace

int main() {
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
            HandleClient(clientSocket);
        }).detach();
    }

    close(serverSocket);
    return 0;
}
