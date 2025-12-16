CREATE TABLE IF NOT EXISTS books (
  id SERIAL PRIMARY KEY,
  title TEXT NOT NULL,
  author TEXT,
  isbn TEXT UNIQUE,
  assigned_to TEXT,
  due_date TIMESTAMP WITH TIME ZONE
);
