# LibX :tada: :rocket:

This project implements a simple REST backend for managing a bookstore inventory in modern C++ (C++17) with PostgreSQL as the datastore. The application supports adding, removing, updating books and assigning a book to someone (with due date). Assignment and return operations are protected by JWT-based authentication.

All components run in Docker (the app is built inside a container) and a `docker-compose.yml` is provided to run Postgres + app.

-- Quick start (Linux / macOS with Docker):

```bash
# build and start services
docker compose up --build

# Wait until app reports health on http://localhost:8080/health
```

API is available at http://localhost:8080

Default admin credentials and JWT secret for the demo are set in `docker-compose.yml` (see `ADMIN_JWT_SECRET`, `ADMIN_USER`, `ADMIN_PASSWORD`).

Endpoints

- POST /books — create a book
- GET /books — list all books
- GET /books/{id} — get a book
- PUT /books/{id} — update a book
- DELETE /books/{id} — delete a book
-- POST /auth/token — obtain JWT token (provide JSON {"username","password"})
-- POST /books/{id}/assign — assign book to someone (protected by JWT `Authorization: Bearer <token>`)
-- POST /books/{id}/return — return a book (protected by JWT)

Sample CURLs

# Create book
curl -s -X POST http://localhost:8080/books -H 'Content-Type: application/json' \
  -d '{"title":"The C++ Programming Language","author":"Bjarne Stroustrup","isbn":"9780321563842"}' | jq

# List books
curl -s http://localhost:8080/books | jq

## Obtain JWT token

```bash
# get token (example using default creds set in docker-compose)
TOKEN=$(curl -s -X POST http://localhost:8080/auth/token -H 'Content-Type: application/json' \
  -d '{"username":"admin","password":"password"}' | jq -r .token)
```

# Assign book id 1 (protected with JWT)
```bash
curl -s -X POST http://localhost:8080/books/1/assign \
  -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" \
  -d '{"assigned_to":"Alice","due_date":"2026-01-31T23:59:00Z"}' | jq
```

# Return book id 1
```bash
curl -s -X POST http://localhost:8080/books/1/return -H "Authorization: Bearer $TOKEN" | jq
```

# Delete book
curl -s -X DELETE http://localhost:8080/books/1 | jq

Notes & Security

 - Authentication uses JWT tokens. Use `POST /auth/token` to exchange username/password for a token. For production, consider an OAuth2 provider and secure rotation/storage of secrets.
- Passwords / secrets should be provided via a secret manager / environment variables and not checked into version control.
- The DB init uses a simple SQL to create the `books` table. You can replace/migrate with a proper migration tool.

Architecture
------------

The project follows a clear layered design:

- models: `include/models/book.h` (data structures and JSON conversions)
- providers: `include/providers/*` — data access layer (e.g., `PostgresProvider` implements `IBookProvider`)
- services: `include/services/*` — business logic and validation (`BookService`)
- controllers: `include/controllers/*` — HTTP routing and request/response handling (`BookController`)

This separation makes the code testable, clean, and easy to extend.

Running unit tests
------------------

Unit tests use GoogleTest. If you build locally with CMake you can run:

```bash
mkdir -p build && cd build
cmake ..
make -j
ctest --output-on-failure
```

Note: unit tests are self-contained and use a small fake provider to avoid requiring a running Postgres instance. Integration tests (which spin up Postgres + app) are included under `integration/` and can be run locally.

Integration tests
-----------------

Run the integration checks with:

```bash
./integration/run_integration_tests.sh
```

This script will build the app image, start the `db` and `app` services, wait for readiness, exercise a create/token/assign/return flow, and tear down the stack.

Formatting & linting

Install `pre-commit` and enable the repository hooks to run `clang-format`, `clang-tidy`, and tests before commits:

```bash
pip install pre-commit
pre-commit install
```

Development

To build locally (if you prefer not to use Docker): ensure `libpqxx-dev`, `libssl-dev`, `cmake`, and a C++17 toolchain are installed, then:

```bash
mkdir build && cd build
cmake ..
make -j
./book_inventory
```
