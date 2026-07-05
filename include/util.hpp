//! @file util.hpp
//! @brief Utility helper functions
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#pragma once

#include <filesystem>
#include <string>

namespace my_cpp_server::util {

std::filesystem::path ResolvePath(const std::string& path);

std::string ReadFile(const std::string& path);

std::string GetContentType(const std::string& path);

bool SendAll(int socketFd, const std::string& data);

void HandleClient(int clientSocket);

}  // namespace my_cpp_server::util
