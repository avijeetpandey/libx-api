#!/usr/bin/env bash
set -euo pipefail

echo "Waiting for Postgres at ${POSTGRES_HOST:-db}:${POSTGRES_PORT:-5432}..."
until pg_isready -h "${POSTGRES_HOST:-db}" -p "${POSTGRES_PORT:-5432}" -U "${POSTGRES_USER:-postgres}" >/dev/null 2>&1; do
  sleep 1
done

echo "Postgres is up — starting app"
exec /app/build/book_inventory
