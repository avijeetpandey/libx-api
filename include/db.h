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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Book, id, title, author, isbn, assigned_to, due_date)

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
