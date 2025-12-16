#include "logging/logger.h"
#include <chrono>
#include <iostream>

static std::string now_iso() {
  using namespace std::chrono;
  auto t = system_clock::now();
  auto s = system_clock::to_time_t(t);
  std::tm tm{};
  gmtime_r(&s, &tm);
  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
  return std::string(buf);
}

void LOG(LogLevel lvl, const std::string &msg, const json &meta) {
  json j;
  j["ts"] = now_iso();
  switch (lvl) {
    case LogLevel::Info: j["level"] = "info"; break;
    case LogLevel::Warn: j["level"] = "warn"; break;
    case LogLevel::Error: j["level"] = "error"; break;
  }
  j["msg"] = msg;
  j["meta"] = meta;
  std::cout << j.dump() << std::endl;
}
