#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "models/book.h"

using json = nlohmann::json;

TEST(BookModel, JsonRoundtrip) {
  Book b;
  b.id = 42;
  b.title = "Test";
  b.author = "Author";
  b.isbn = "ISBN";
  b.assigned_to = std::optional<std::string>("Alice");
  b.due_date = std::optional<std::string>("2026-01-01T00:00:00Z");

  json j = b;
  Book b2 = j.get<Book>();

  EXPECT_EQ(b2.id, 42);
  EXPECT_EQ(b2.title, "Test");
  EXPECT_EQ(b2.author, "Author");
  EXPECT_EQ(b2.isbn, "ISBN");
  ASSERT_TRUE(b2.assigned_to.has_value());
  EXPECT_EQ(b2.assigned_to.value(), "Alice");
}
