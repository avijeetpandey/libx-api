#include "metrics/metrics.h"
#include <sstream>

void Metrics::inc_requests() { requests_.fetch_add(1, std::memory_order_relaxed); }
void Metrics::inc_assigns() { assigns_.fetch_add(1, std::memory_order_relaxed); }
void Metrics::observe_request_duration_ms(double ms) {
  total_request_ms_.fetch_add(static_cast<uint64_t>(ms), std::memory_order_relaxed);
  request_count_.fetch_add(1, std::memory_order_relaxed);
}

std::string Metrics::expose() {
  std::ostringstream ss;
  ss << "# HELP app_requests_total Total number of HTTP requests\n";
  ss << "# TYPE app_requests_total counter\n";
  ss << "app_requests_total " << requests_.load() << "\n";
  ss << "# HELP app_assigns_total Total number of book assigns\n";
  ss << "# TYPE app_assigns_total counter\n";
  ss << "app_assigns_total " << assigns_.load() << "\n";
  uint64_t count = request_count_.load();
  uint64_t avg = (count == 0 ? 0 : total_request_ms_.load() / count);
  ss << "# HELP app_request_duration_ms_avg Average request duration in ms\n";
  ss << "# TYPE app_request_duration_ms_avg gauge\n";
  ss << "app_request_duration_ms_avg " << avg << "\n";
  return ss.str();
}
