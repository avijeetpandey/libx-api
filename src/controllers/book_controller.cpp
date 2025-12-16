#include "controllers/book_controller.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

BookController::BookController(httplib::Server &srv, BookService &service, AuthService &auth)
  : service_(service), auth_(auth) {
  register_routes(srv);
}

void BookController::register_routes(httplib::Server &srv) {
  srv.Post("/books", [&](const httplib::Request &req, httplib::Response &res){
    try {
      auto j = json::parse(req.body);
      if (!j.contains("title") || !j.contains("author") || !j.contains("isbn")) {
        res.status = 400; res.set_content(json({{"error","title, author and isbn required"}}).dump(), "application/json"); return;
      }
      auto b = service_.create_book(j["title"].get<std::string>(), j["author"].get<std::string>(), j["isbn"].get<std::string>());
      res.status = 201; res.set_content(json(b).dump(), "application/json");
    } catch (const std::exception &e) { res.status=500; res.set_content(json({{"error", e.what()}}).dump(), "application/json"); }
  });

  srv.Get("/books", [&](const httplib::Request &req, httplib::Response &res){ res.set_content(json(service_.list_books()).dump(), "application/json"); });

  srv.Get(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    auto b = service_.get_book(id);
    if (!b) { res.status=404; res.set_content(json({{"error","not found"}}).dump(), "application/json"); return; }
    res.set_content(json(*b).dump(), "application/json");
  });

  srv.Put(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    try {
      auto j = json::parse(req.body);
      if (!j.contains("title") || !j.contains("author") || !j.contains("isbn")) { res.status=400; res.set_content(json({{"error","title, author and isbn required"}}).dump(), "application/json"); return; }
      bool ok = service_.update_book(id, j["title"].get<std::string>(), j["author"].get<std::string>(), j["isbn"].get<std::string>());
      if (!ok) { res.status = 404; res.set_content(json({{"error","not found"}}).dump(), "application/json"); return; }
      res.set_content("{}", "application/json");
    } catch (const std::exception &e) { res.status=500; res.set_content(json({{"error", e.what()}}).dump(), "application/json"); }
  });

  srv.Delete(R"(/books/(\d+))", [&](const httplib::Request &req, httplib::Response &res){
    int id = std::stoi(req.matches[1]);
    bool ok = service_.delete_book(id);
    if (!ok) { res.status=404; res.set_content(json({{"error","not found"}}).dump(), "application/json"); return; }
    res.set_content("{}", "application/json");
  });

  auto check_jwt = [&](const httplib::Request &req)->bool{
    auto auth_hdr = req.get_header_value("Authorization");
    if (auth_hdr.rfind("Bearer ", 0) != 0) return false;
    auto token = auth_hdr.substr(7);
    try {
      auto payload = auth_.verify_token(token);
      if (!payload.contains("role")) return false;
      return payload["role"].get<std::string>() == "admin";
    } catch (...) {
      return false;
    }
  };

  srv.Post(R"(/books/(\d+)/assign)", [&](const httplib::Request &req, httplib::Response &res){
    if (!check_jwt(req)) { res.status=401; res.set_content(json({{"error","unauthorized"}}).dump(), "application/json"); return; }
    int id = std::stoi(req.matches[1]);
    try {
      auto j = json::parse(req.body);
      if (!j.contains("assigned_to") || !j.contains("due_date")) { res.status=400; res.set_content(json({{"error","assigned_to and due_date required"}}).dump(), "application/json"); return; }
      bool ok = service_.assign_book(id, j["assigned_to"].get<std::string>(), j["due_date"].get<std::string>());
      if (!ok) { res.status = 404; res.set_content(json({{"error","not found"}}).dump(), "application/json"); return; }
      res.set_content("{}", "application/json");
    } catch (const std::exception &e) { res.status=500; res.set_content(json({{"error", e.what()}}).dump(), "application/json"); }
  });

  srv.Post(R"(/books/(\d+)/return)", [&](const httplib::Request &req, httplib::Response &res){
    if (!check_jwt(req)) { res.status=401; res.set_content(json({{"error","unauthorized"}}).dump(), "application/json"); return; }
    int id = std::stoi(req.matches[1]);
    bool ok = service_.return_book(id);
    if (!ok) { res.status = 404; res.set_content(json({{"error","not found"}}).dump(), "application/json"); return; }
    res.set_content("{}", "application/json");
  });

  // Token issuance endpoint
  srv.Post("/auth/token", [&](const httplib::Request &req, httplib::Response &res){
    try {
      auto j = json::parse(req.body);
      if (!j.contains("username") || !j.contains("password")) { res.status=400; res.set_content(json({{"error","username and password required"}}).dump(), "application/json"); return; }
      // simple credential check against environment variables
      auto env_user = std::getenv("ADMIN_USER");
      auto env_pass = std::getenv("ADMIN_PASSWORD");
      std::string provided_user = j["username"].get<std::string>();
      std::string provided_pass = j["password"].get<std::string>();
      if (!env_user || !env_pass || provided_user != env_user || provided_pass != env_pass) {
        res.status = 401; res.set_content(json({{"error","invalid credentials"}}).dump(), "application/json"); return;
      }
      auto token = auth_.issue_token(provided_user, "admin");
      res.set_content(json({{"token", token}}).dump(), "application/json");
    } catch (const std::exception &e) { res.status=500; res.set_content(json({{"error", e.what()}}).dump(), "application/json"); }
  });

  srv.Get("/health", [&](const httplib::Request &req, httplib::Response &res){ res.set_content(json({{"status","ok"}}).dump(), "application/json"); });
}
