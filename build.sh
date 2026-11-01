#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p bin
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/*.cpp -o bin/airports
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc src/Airport.cpp src/AirportBST.cpp src/DataIO.cpp tests/test_airport_bst.cpp -o bin/airport_tests
./bin/airport_tests
