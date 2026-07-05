//! @file util.hpp
//! @brief Utility helper functions
//! Copyright (c) 2026 Taisei Hasegawa. All rights reserved.

#pragma once

#include <filesystem>
#include <string>

namespace my_cpp_server::util {

//! @brief 指定されたパスを公開ディレクトリ配下の実パスへ解決する。
//! @param path リクエストされたパスまたはファイル名
//! @return 解決後のファイルシステムパス
//! @details 例えば、リクエストされたパスが "/index.html" の場合、公開ディレクトリが "public" であれば、
//! この関数は "public/index.html" というパスを返す。
std::filesystem::path ResolvePath(const std::string& path);

//! @brief 指定されたファイルを読み込み、その内容を文字列として返す。
//! @param path 読み込むファイルのパス
//! @return ファイル内容。読み込みに失敗した場合は空文字列
std::string ReadFile(const std::string& path);

//! @brief 拡張子から HTTP レスポンス用の Content-Type を判定する。
//! @param path 対象ファイルのパス
//! @return 対応する Content-Type 文字列
std::string GetContentType(const std::string& path);

//! @brief 指定したソケットへデータを全て送信する。
//! @param socket_fd 送信先のソケットディスクリプタ
//! @param data 送信するデータ
//! @return すべて送信できた場合は true、それ以外は false
bool SendAll(int socket_fd, const std::string& data);

//! @brief クライアントからの HTTP リクエストを処理し、レスポンスを返す。
//! @param client_socket 接続済みのクライアントソケットディスクリプタ
void HandleClient(int client_socket);

}  // namespace my_cpp_server::util
