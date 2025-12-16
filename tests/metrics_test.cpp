#include <gtest/gtest.h>
#include "metrics/metrics.h"

TEST(Metrics, CountersAndExpose) {
  // reset by reading current exposure (not perfect but ok for unit test isolation here)
  Metrics::inc_requests();
  Metrics::inc_assigns();
  Metrics::observe_request_duration_ms(50.0);
  auto out = Metrics::expose();
  EXPECT_NE(out.find("app_requests_total"), std::string::npos);
  EXPECT_NE(out.find("app_assigns_total"), std::string::npos);
  EXPECT_NE(out.find("app_request_duration_ms_avg"), std::string::npos);
}
