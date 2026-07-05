//! @file util.hpp
//! @brief Utility helper functions
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#pragma once

#include <string>

namespace util {

std::string ReadFile(const std::string& path);
std::string GetContentType(const std::string& path);
bool SendAll(int socketFd, const std::string& data);
void HandleClient(int clientSocket);

}  // namespace util
