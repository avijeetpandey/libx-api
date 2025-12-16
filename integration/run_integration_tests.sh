#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

echo "Starting services..."
docker compose up --build -d

echo "Waiting for /health..."
for i in {1..60}; do
  if curl -s http://localhost:8080/health | grep -q ok; then break; fi
  sleep 1
done

echo "Requesting token"
TOKEN=$(curl -s -X POST http://localhost:8080/auth/token -H 'Content-Type: application/json' -d '{"username":"admin","password":"password"}' | jq -r .token)
if [ -z "$TOKEN" ] || [ "$TOKEN" = "null" ]; then echo "Failed to get token"; docker compose logs app; docker compose down; exit 1; fi

echo "Creating book"
CREATED=$(curl -s -X POST http://localhost:8080/books -H 'Content-Type: application/json' -d '{"title":"Integration Book","author":"CI","isbn":"INT-1"}')
ID=$(echo "$CREATED" | jq -r .id)
echo "Created book id=$ID"

echo "Assigning book"
curl -s -X POST http://localhost:8080/books/$ID/assign -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' -d '{"assigned_to":"CI","due_date":"2030-01-01T00:00:00Z"}' | jq

echo "Checking assigned book"
curl -s http://localhost:8080/books/$ID | jq

echo "Cleaning up"
docker compose down

echo "Integration tests passed"
#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"

echo "Starting integration stack with docker-compose..."
ADMIN_JWT_SECRET=${ADMIN_JWT_SECRET:-integration-secret}
ADMIN_USER=${ADMIN_USER:-admin}
ADMIN_PASSWORD=${ADMIN_PASSWORD:-password}

docker compose up --build -d

echo "Waiting for health..."
for i in {1..60}; do
  if curl -sS http://localhost:8080/health | grep -q 'ok'; then break; fi
  sleep 1
done

echo "Creating a book..."
BOOK_ID=$(curl -s -X POST http://localhost:8080/books -H 'Content-Type: application/json' -d '{"title":"Integration","author":"Tester","isbn":"INT-001"}' | jq -r '.id')
echo "Book id: $BOOK_ID"

echo "Requesting token..."
TOKEN=$(curl -s -X POST http://localhost:8080/auth/token -H 'Content-Type: application/json' -d '{"username":"'$ADMIN_USER'","password":"'$ADMIN_PASSWORD'"}' | jq -r '.token')
echo "Token: ${TOKEN:0:20}..."

echo "Assigning book"
curl -s -X POST http://localhost:8080/books/${BOOK_ID}/assign -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"assigned_to":"IntegrationUser","due_date":"2026-01-01T00:00:00Z"}' | jq

echo "Verifying assignment"
curl -s http://localhost:8080/books | jq

echo "Returning book"
curl -s -X POST http://localhost:8080/books/${BOOK_ID}/return -H "Authorization: Bearer $TOKEN" | jq

echo "Integration tests finished. Cleaning up..."
docker compose down

echo "Done"
