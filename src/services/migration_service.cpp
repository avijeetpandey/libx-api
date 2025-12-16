#include "services/migration_service.h"
#include <filesystem>
#include <regex>
#include <algorithm>
#include <pqxx/pqxx>
#include <fstream>
#include "logging/logger.h"

namespace fs = std::filesystem;

MigrationService::MigrationService(std::string migrations_dir) : dir_(std::move(migrations_dir)) {}

std::vector<std::string> MigrationService::list_migrations() const {
  std::vector<std::string> out;
  std::regex re(R"((\d+)_.*\.sql)$");
  for (auto &p : fs::directory_iterator(dir_)) {
    if (!p.is_regular_file()) continue;
    auto name = p.path().filename().string();
    if (std::regex_match(name, re)) out.push_back(p.path().string());
  }
  std::sort(out.begin(), out.end());
  return out;
}

void MigrationService::run(const std::string &conninfo) const {
  LOG_INFO("running migrations", { {"dir", dir_} });
  auto files = list_migrations();
  if (files.empty()) { LOG_INFO("no migrations found", {}); return; }
  pqxx::connection conn(conninfo);
  pqxx::work tx(conn);
  tx.exec("CREATE TABLE IF NOT EXISTS schema_migrations (name TEXT PRIMARY KEY, applied_at TIMESTAMP WITH TIME ZONE DEFAULT now())");
  tx.commit();

  for (auto &f : files) {
    std::ifstream in(f);
    if (!in) { LOG_WARN("could not open migration", {{"file", f}}); continue; }
    std::string name = fs::path(f).filename().string();
    pqxx::work check(conn);
    auto r = check.exec_params("SELECT name FROM schema_migrations WHERE name = $1", name);
    if (!r.empty()) { LOG_INFO("migration already applied", {{"file", name}}); continue; }
    check.commit();

    std::string sql((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    try {
      pqxx::work apply(conn);
      apply.exec(sql);
      apply.exec_params("INSERT INTO schema_migrations (name) VALUES ($1)", name);
      apply.commit();
      LOG_INFO("applied migration", {{"file", name}});
    } catch (const std::exception &e) {
      LOG_ERROR("migration failed", {{"file", name}, {"error", e.what()}});
      throw;
    }
  }
}
