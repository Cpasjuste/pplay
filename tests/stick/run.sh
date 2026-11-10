#!/usr/bin/env bash
# Analog stick -> direction (host, no console needed).
set -euo pipefail
H=$(cd "$(dirname "$0")" && pwd); R=$H/../..
W=$(mktemp -d); cd "$W"
g++ -std=c++11 -O1 -Wall -Wextra -I"$R/src" "$H/test.cpp" -o t
./t
