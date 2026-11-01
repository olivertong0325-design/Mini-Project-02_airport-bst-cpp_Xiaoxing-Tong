param([string]$Compiler = "g++")
$ErrorActionPreference = "Stop"
Push-Location (Join-Path $PSScriptRoot "..")
try {
    New-Item -ItemType Directory -Force -Path "bin" | Out-Null
    & $Compiler -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc src/Airport.cpp src/AirportBST.cpp src/DataIO.cpp tests/test_airport_bst.cpp -o bin/airport_tests.exe
    if ($LASTEXITCODE -ne 0) { throw "Test build failed" }
    & .\bin\airport_tests.exe | Tee-Object -FilePath docs/test-results.txt
    if ($LASTEXITCODE -ne 0) { throw "Tests failed" }
} finally { Pop-Location }
