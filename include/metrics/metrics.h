#pragma once

#include <atomic>
#include <string>

class Metrics {
public:
  static void inc_requests();
  static void inc_assigns();
  static void observe_request_duration_ms(double ms);
  static std::string expose();

private:
  static inline std::atomic<uint64_t> requests_{0};
  static inline std::atomic<uint64_t> assigns_{0};
  static inline std::atomic<uint64_t> total_request_ms_{0};
  static inline std::atomic<uint64_t> request_count_{0};
};
