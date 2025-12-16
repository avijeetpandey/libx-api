#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class LogLevel { Info, Warn, Error };

void LOG(LogLevel lvl, const std::string &msg, const json &meta = json::object());

inline void LOG_INFO(const std::string &msg, const json &meta = json::object()) { LOG(LogLevel::Info, msg, meta); }
inline void LOG_WARN(const std::string &msg, const json &meta = json::object()) { LOG(LogLevel::Warn, msg, meta); }
inline void LOG_ERROR(const std::string &msg, const json &meta = json::object()) { LOG(LogLevel::Error, msg, meta); }
