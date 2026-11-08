#!/usr/bin/env bash
# Phone remote control: protocol + server on loopback (host, no console needed).
set -euo pipefail
H=$(cd "$(dirname "$0")" && pwd); R=$H/../..
W=$(mktemp -d); cd "$W"
g++ -std=c++11 -O1 -Wall -Wextra -I"$R/src" "$H/test.cpp" "$R/src/remote_protocol.cpp" "$R/src/remote_server.cpp" -o t
./t
