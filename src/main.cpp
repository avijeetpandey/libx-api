#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "db.h"
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

  DB db(conn.str());
  try {
    db.init();
  } catch (const std::exception &e) {
    std::cerr << "DB init failed: " << e.what() << std::endl;
    return 1;
  }

  httplib::Server svr;

  svr.Post("/books", [&](const httplib::Request &req, httplib::Response &res){
    try {
      auto j = json::parse(req.body);
      if (!j.contains("title") || !j.contains("author") || !j.contains("isbn")) {
        res.status = 400;
        res.set_content("{\"error\":\"title, author and isbn required\"}", "application/json");
        return;
      }
      auto b = db.create_book(j["title"].get<std::string>(), j["author"].get<std::string>(), j["isbn"].get<std::string>());
      res.status = 201;
      res.set_content(json(b).dump(), "application/json");
    } catch (const std::exception &e) {
      res.status = 500;
      res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
  });

  svr.Get(R"(/books/?(")", [&](const httplib::Request &req, httplib::Response &res){
    auto vec = db.list_books();
    res.set_content(json(vec).dump(), "application/json");
  });

  svr.Get(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    auto b = db.get_book(id);
    if (!b) {
      res.status = 404;
      res.set_content(json({{"error","not found"}}).dump(), "application/json");
      return;
    }
    res.set_content(json(*b).dump(), "application/json");
  });

  svr.Put(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    try {
      auto j = json::parse(req.body);
      if (!j.contains("title") || !j.contains("author") || !j.contains("isbn")) {
        res.status = 400;
        res.set_content("{\"error\":\"title, author and isbn required\"}", "application/json");
        return;
      }
      bool ok = db.update_book(id, j["title"].get<std::string>(), j["author"].get<std::string>(), j["isbn"].get<std::string>());
      if (!ok) { res.status = 404; res.set_content("{\"error\":\"not found\"}", "application/json"); return; }
      res.set_content("{}", "application/json");
    } catch (const std::exception &e) {
      res.status = 500;
      res.set_content(json({{"error", e.what()}}).dump(), "application/json");
    }
  });

  svr.Delete(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    bool ok = db.delete_book(id);
    if (!ok) { res.status = 404; res.set_content("{\"error\":\"not found\"}", "application/json"); return; }
    res.set_content("{}", "application/json");
  });

  // Protected endpoints: require API key
  auto check_api_key = [&](const httplib::Request &req)->bool{
    auto provided = req.get_header_value("X-API-Key");
    auto expected = getenv_or("ADMIN_API_KEY", "");
    return !expected.empty() && provided == expected;
  };

  svr.Post(R"(/books/(\d+)/assign)", [&](const httplib::Request &req, httplib::Response &res){
    if (!check_api_key(req)) { res.status = 401; res.set_content("{\"error\":\"unauthorized\"}", "application/json"); return; }
    int id = std::stoi(req.matches[1]);
    try {
      auto j = json::parse(req.body);
      if (!j.contains("assigned_to") || !j.contains("due_date")) { res.status=400; res.set_content("{\"error\":\"assigned_to and due_date required\"}", "application/json"); return; }
      bool ok = db.assign_book(id, j["assigned_to"].get<std::string>(), j["due_date"].get<std::string>());
      if (!ok) { res.status = 404; res.set_content("{\"error\":\"not found\"}", "application/json"); return; }
      res.set_content("{}", "application/json");
    } catch (const std::exception &e) { res.status=500; res.set_content(json({{"error", e.what()}}).dump(), "application/json"); }
  });

  svr.Post(R"(/books/(\d+)/return)", [&](const httplib::Request &req, httplib::Response &res){
    if (!check_api_key(req)) { res.status = 401; res.set_content("{\"error\":\"unauthorized\"}", "application/json"); return; }
    int id = std::stoi(req.matches[1]);
    bool ok = db.return_book(id);
    if (!ok) { res.status = 404; res.set_content("{\"error\":\"not found\"}", "application/json"); return; }
    res.set_content("{}", "application/json");
  });

  svr.Get("/health", [&](const httplib::Request &req, httplib::Response &res){ res.set_content("{\"status\":\"ok\"}", "application/json"); });

  std::cout << "Starting server on 0.0.0.0:8080" << std::endl;
  svr.listen("0.0.0.0", 8080);

  return 0;
}
