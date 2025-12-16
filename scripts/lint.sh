#!/usr/bin/env bash
set -euo pipefail

files=$(git diff --name-only --cached --diff-filter=ACM | grep -E "\.(cpp|hpp|cc|cxx|h|hh)$" || true)
if [ -z "$files" ]; then echo "No C/C++ files staged for linting"; exit 0; fi
for f in $files; do
  clang-tidy "$f" -- -Iinclude || true
done
