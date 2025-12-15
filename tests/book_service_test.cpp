#include <gtest/gtest.h>
#include "models/book.h"
#include "services/book_service.h"
#include "providers/ibook_provider.h"

// Fake provider for tests
class FakeProvider : public IBookProvider {
public:
  Book create_book(const std::string &title, const std::string &author, const std::string &isbn) override {
    Book b; b.id = ++last_id; b.title = title; b.author = author; b.isbn = isbn; return b;
  }
  std::vector<Book> list_books() override { return books; }
  std::optional<Book> get_book(int id) override { for (auto &b : books) if (b.id==id) return b; return std::nullopt; }
  bool update_book(int id, const std::string &title, const std::string &author, const std::string &isbn) override {
    for (auto &b : books) if (b.id==id) { b.title=title; b.author=author; b.isbn=isbn; return true; } return false;
  }
  bool delete_book(int id) override { for (auto it=books.begin(); it!=books.end(); ++it) if (it->id==id) { books.erase(it); return true; } return false; }
  bool assign_book(int id, const std::string &assigned_to, const std::string &due_date) override {
    for (auto &b : books) if (b.id==id) { b.assigned_to = assigned_to; b.due_date = due_date; return true; } return false;
  }
  bool return_book(int id) override { for (auto &b : books) if (b.id==id) { b.assigned_to.reset(); b.due_date.reset(); return true; } return false; }

  std::vector<Book> books;
  int last_id = 0;
};

TEST(BookService, CreateAndAssign) {
  FakeProvider fake;
  BookService svc(fake);

  Book b = svc.create_book("T","A","I");
  EXPECT_EQ(b.id, 1);

  // Add created book to provider storage to simulate persistence
  fake.books.push_back(b);

  bool ok = svc.assign_book(1, "Bob", "2026-01-01T00:00:00Z");
  EXPECT_TRUE(ok);
  auto res = fake.get_book(1);
  ASSERT_TRUE(res.has_value());
  EXPECT_TRUE(res->assigned_to.has_value());
  EXPECT_EQ(res->assigned_to.value(), "Bob");
}
