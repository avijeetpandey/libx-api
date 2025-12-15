#pragma once

#include <pqxx/pqxx>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Book {
  int id;
  std::string title;
  std::string author;
  std::string isbn;
  std::optional<std::string> assigned_to;
  std::optional<std::string> due_date; // ISO string
};

inline void to_json(nlohmann::json &j, const Book &b) {
  j = nlohmann::json{
    {"id", b.id},
    {"title", b.title},
    {"author", b.author},
    {"isbn", b.isbn}
  };
  if (b.assigned_to) j["assigned_to"] = *b.assigned_to; else j["assigned_to"] = nullptr;
  if (b.due_date) j["due_date"] = *b.due_date; else j["due_date"] = nullptr;
}

inline void from_json(const nlohmann::json &j, Book &b) {
  if (j.contains("id") && !j["id"].is_null()) b.id = j["id"].get<int>();
  if (j.contains("title") && !j["title"].is_null()) b.title = j["title"].get<std::string>();
  if (j.contains("author") && !j["author"].is_null()) b.author = j["author"].get<std::string>();
  if (j.contains("isbn") && !j["isbn"].is_null()) b.isbn = j["isbn"].get<std::string>();
  if (j.contains("assigned_to") && !j["assigned_to"].is_null()) b.assigned_to = j["assigned_to"].get<std::string>();
  else b.assigned_to.reset();
  if (j.contains("due_date") && !j["due_date"].is_null()) b.due_date = j["due_date"].get<std::string>();
  else b.due_date.reset();
}

class DB {
public:
  DB(const std::string &conninfo);
  void init();
  Book create_book(const std::string &title, const std::string &author, const std::string &isbn);
  std::vector<Book> list_books();
  std::optional<Book> get_book(int id);
  bool update_book(int id, const std::string &title, const std::string &author, const std::string &isbn);
  bool delete_book(int id);
  bool assign_book(int id, const std::string &assigned_to, const std::string &due_date);
  bool return_book(int id);
private:
  pqxx::connection m_conn;
};
