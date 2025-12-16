#pragma once

#include <httplib.h>
#include "services/book_service.h"
#include "services/auth_service.h"
#include <string>

class BookController {
public:
  BookController(httplib::Server &srv, BookService &service, AuthService &auth);
private:
  void register_routes(httplib::Server &srv);
  BookService &service_;
  AuthService &auth_;
};
