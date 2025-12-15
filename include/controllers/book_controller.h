#pragma once

#include <httplib.h>
#include "services/book_service.h"
#include <string>

class BookController {
public:
  BookController(httplib::Server &srv, BookService &service, const std::string &admin_key);
private:
  void register_routes(httplib::Server &srv);
  BookService &service_;
  std::string admin_key_;
};
