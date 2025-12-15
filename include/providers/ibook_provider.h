#pragma once

#include <vector>
#include <optional>
#include "models/book.h"

class IBookProvider {
public:
  virtual ~IBookProvider() = default;
  virtual Book create_book(const std::string &title, const std::string &author, const std::string &isbn) = 0;
  virtual std::vector<Book> list_books() = 0;
  virtual std::optional<Book> get_book(int id) = 0;
  virtual bool update_book(int id, const std::string &title, const std::string &author, const std::string &isbn) = 0;
  virtual bool delete_book(int id) = 0;
  virtual bool assign_book(int id, const std::string &assigned_to, const std::string &due_date) = 0;
  virtual bool return_book(int id) = 0;
};
