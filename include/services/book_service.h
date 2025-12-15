#pragma once

#include "models/book.h"
#include "providers/ibook_provider.h"
#include <vector>
#include <optional>

class BookService {
public:
  explicit BookService(IBookProvider &provider): provider_(provider) {}

  Book create_book(const std::string &title, const std::string &author, const std::string &isbn) {
    // Basic validation
    if (title.empty() || author.empty() || isbn.empty()) throw std::invalid_argument("title/author/isbn required");
    return provider_.create_book(title, author, isbn);
  }

  std::vector<Book> list_books() { return provider_.list_books(); }
  std::optional<Book> get_book(int id) { return provider_.get_book(id); }
  bool update_book(int id, const std::string &title, const std::string &author, const std::string &isbn) { return provider_.update_book(id, title, author, isbn); }
  bool delete_book(int id) { return provider_.delete_book(id); }

  bool assign_book(int id, const std::string &assigned_to, const std::string &due_date) {
    if (assigned_to.empty() || due_date.empty()) throw std::invalid_argument("assigned_to and due_date required");
    return provider_.assign_book(id, assigned_to, due_date);
  }

  bool return_book(int id) { return provider_.return_book(id); }

private:
  IBookProvider &provider_;
};
