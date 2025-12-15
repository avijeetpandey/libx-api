#include "db.h"
#include <pqxx/pqxx>
#include <iostream>

DB::DB(const std::string &conninfo) : m_conn(conninfo) {}

void DB::init() {
  pqxx::work txn(m_conn);
  txn.exec(R"(
    CREATE TABLE IF NOT EXISTS books (
      id SERIAL PRIMARY KEY,
      title TEXT NOT NULL,
      author TEXT NOT NULL,
      isbn TEXT NOT NULL,
      assigned_to TEXT,
      due_date TIMESTAMP WITH TIME ZONE,
      created_at TIMESTAMP WITH TIME ZONE DEFAULT now(),
      updated_at TIMESTAMP WITH TIME ZONE DEFAULT now()
    );
  )");
  txn.commit();
}

Book DB::create_book(const std::string &title, const std::string &author, const std::string &isbn) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params(
    "INSERT INTO books (title, author, isbn) VALUES ($1, $2, $3) RETURNING id, title, author, isbn, assigned_to, due_date",
    title, author, isbn
  );
  txn.commit();
  Book b;
  auto row = r[0];
  b.id = row[0].as<int>();
  b.title = row[1].as<std::string>();
  b.author = row[2].as<std::string>();
  b.isbn = row[3].as<std::string>();
  if (!row[4].is_null()) b.assigned_to = row[4].as<std::string>();
  if (!row[5].is_null()) b.due_date = row[5].as<std::string>();
  return b;
}

std::vector<Book> DB::list_books() {
  pqxx::work txn(m_conn);
  auto r = txn.exec("SELECT id, title, author, isbn, assigned_to, due_date FROM books ORDER BY id");
  std::vector<Book> res;
  for (auto row: r) {
    Book b;
    b.id = row[0].as<int>();
    b.title = row[1].as<std::string>();
    b.author = row[2].as<std::string>();
    b.isbn = row[3].as<std::string>();
    if (!row[4].is_null()) b.assigned_to = row[4].as<std::string>();
    if (!row[5].is_null()) b.due_date = row[5].as<std::string>();
    res.push_back(b);
  }
  return res;
}

std::optional<Book> DB::get_book(int id) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params("SELECT id, title, author, isbn, assigned_to, due_date FROM books WHERE id=$1", id);
  if (r.empty()) return std::nullopt;
  auto row = r[0];
  Book b;
  b.id = row[0].as<int>();
  b.title = row[1].as<std::string>();
  b.author = row[2].as<std::string>();
  b.isbn = row[3].as<std::string>();
  if (!row[4].is_null()) b.assigned_to = row[4].as<std::string>();
  if (!row[5].is_null()) b.due_date = row[5].as<std::string>();
  return b;
}

bool DB::update_book(int id, const std::string &title, const std::string &author, const std::string &isbn) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params("UPDATE books SET title=$1, author=$2, isbn=$3, updated_at=now() WHERE id=$4", title, author, isbn, id);
  txn.commit();
  return r.affected_rows() > 0;
}

bool DB::delete_book(int id) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params("DELETE FROM books WHERE id=$1", id);
  txn.commit();
  return r.affected_rows() > 0;
}

bool DB::assign_book(int id, const std::string &assigned_to, const std::string &due_date) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params("UPDATE books SET assigned_to=$1, due_date=$2, updated_at=now() WHERE id=$3", assigned_to, due_date, id);
  txn.commit();
  return r.affected_rows() > 0;
}

bool DB::return_book(int id) {
  pqxx::work txn(m_conn);
  auto r = txn.exec_params("UPDATE books SET assigned_to=NULL, due_date=NULL, updated_at=now() WHERE id=$1", id);
  txn.commit();
  return r.affected_rows() > 0;
}
