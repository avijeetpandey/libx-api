#include <gtest/gtest.h>
#include <fstream>

TEST(Docs, OpenApiFileExists) {
  std::ifstream f("docs/openapi.json");
  ASSERT_TRUE(f.good());
}
