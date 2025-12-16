#pragma once

#include <string>
#include <vector>

class MigrationService {
public:
  explicit MigrationService(std::string migrations_dir);
  // run migrations against the given connection string (pg)
  void run(const std::string &conninfo) const;

  // helper to list migration files (for tests)
  std::vector<std::string> list_migrations() const;

private:
  std::string dir_;
};
