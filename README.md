# Book Inventory (C++) — REST Backend

This project implements a simple REST backend for managing a bookstore inventory in modern C++ (C++17) with PostgreSQL as the datastore. The application supports adding, removing, updating books and assigning a book to someone (with due date). Assignment and return operations are protected by an API key (header: `X-API-Key`).

All components run in Docker (the app is built inside a container) and a `docker-compose.yml` is provided to run Postgres + app.

-- Quick start (Linux / macOS with Docker):

```bash
# build and start services
docker compose up --build

# Wait until app reports health on http://localhost:8080/health
```

API is available at http://localhost:8080

Default admin API key (for demo in docker-compose): `secret-api-key-please-change` — set `ADMIN_API_KEY` in compose file or pass an env var.

Endpoints

- POST /books — create a book
- GET /books — list all books
- GET /books/{id} — get a book
- PUT /books/{id} — update a book
- DELETE /books/{id} — delete a book
- POST /books/{id}/assign — assign book to someone (protected by API key)
- POST /books/{id}/return — return a book (protected by API key)

Sample CURLs

# Create book
curl -s -X POST http://localhost:8080/books -H 'Content-Type: application/json' \
  -d '{"title":"The C++ Programming Language","author":"Bjarne Stroustrup","isbn":"9780321563842"}' | jq

# List books
curl -s http://localhost:8080/books | jq

# Assign book id 1 (protected)
curl -s -X POST http://localhost:8080/books/1/assign \
  -H 'Content-Type: application/json' -H 'X-API-Key: secret-api-key-please-change' \
  -d '{"assigned_to":"Alice","due_date":"2026-01-31T23:59:00Z"}' | jq

# Return book id 1
curl -s -X POST http://localhost:8080/books/1/return -H 'X-API-Key: secret-api-key-please-change' | jq

# Delete book
curl -s -X DELETE http://localhost:8080/books/1 | jq

Notes & Security

- The API key approach is used for simplicity. For production use, consider JWT, OAuth2, or at least rotating API keys stored securely.
- Passwords / secrets should be provided via a secret manager / environment variables and not checked into version control.
- The DB init uses a simple SQL to create the `books` table. You can replace/migrate with a proper migration tool.

Development

To build locally (if you prefer not to use Docker): ensure `libpqxx-dev`, `libssl-dev`, `cmake`, and a C++17 toolchain are installed, then:

```bash
mkdir build && cd build
cmake ..
make -j
./book_inventory
```
