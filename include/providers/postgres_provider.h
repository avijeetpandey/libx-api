#pragma once

#include "ibook_provider.h"
#include <pqxx/pqxx>

class PostgresProvider : public IBookProvider {
public:
  explicit PostgresProvider(const std::string &conninfo);
  void init();

  Book create_book(const std::string &title, const std::string &author, const std::string &isbn) override;
  std::vector<Book> list_books() override;
  std::optional<Book> get_book(int id) override;
  bool update_book(int id, const std::string &title, const std::string &author, const std::string &isbn) override;
  bool delete_book(int id) override;
  bool assign_book(int id, const std::string &assigned_to, const std::string &due_date) override;
  bool return_book(int id) override;

private:
  pqxx::connection m_conn;
};
