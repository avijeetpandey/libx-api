#include <gtest/gtest.h>
#include "services/migration_service.h"
#include <filesystem>

TEST(MigrationService, ListMigrations) {
  MigrationService m("migrations");
  auto files = m.list_migrations();
  // expect at least our two migrations to be present and sorted
  ASSERT_GE(files.size(), 2);
  auto a = std::filesystem::path(files[0]).filename().string();
  auto b = std::filesystem::path(files[1]).filename().string();
  EXPECT_LT(a, b);
}
