#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "providers/postgres_provider.h"
#include "services/book_service.h"
#include "controllers/book_controller.h"
#include <httplib.h>

using json = nlohmann::json;

static std::string getenv_or(const char* k, const std::string &def="") {
  auto v = std::getenv(k);
  if (v) return std::string(v);
  return def;
}

int main() {
  // DB connection info from env
  std::string db_host = getenv_or("POSTGRES_HOST", "db");
  std::string db_port = getenv_or("POSTGRES_PORT", "5432");
  std::string db_name = getenv_or("POSTGRES_DB", "bookdb");
  std::string db_user = getenv_or("POSTGRES_USER", "postgres");
  std::string db_pass = getenv_or("POSTGRES_PASSWORD", "postgres");

  std::ostringstream conn;
  conn << "host=" << db_host << " port=" << db_port << " dbname=" << db_name
       << " user=" << db_user << " password=" << db_pass;

  PostgresProvider pg(conn.str());
  try {
    pg.init();
  } catch (const std::exception &e) {
    std::cerr << "DB init failed: " << e.what() << std::endl;
    return 1;
  }

  BookService service(pg);

  httplib::Server svr;
  std::string admin_key = getenv_or("ADMIN_API_KEY", "");
  BookController controller(svr, service, admin_key);

  std::cout << "Starting server on 0.0.0.0:8080" << std::endl;
  svr.listen("0.0.0.0", 8080);

  return 0;
}
