param([string]$Compiler = "g++")
$ErrorActionPreference = "Stop"
Push-Location (Join-Path $PSScriptRoot "..")
try {
    New-Item -ItemType Directory -Force -Path "bin" | Out-Null
    $sourceFiles = @(Get-ChildItem -LiteralPath "src" -Filter "*.cpp" | ForEach-Object { $_.FullName })
    & $Compiler -std=c++17 -O2 -Wall -Wextra -Wpedantic @sourceFiles -o bin/airports.exe
    if ($LASTEXITCODE -ne 0) { throw "C++ build failed" }
    Write-Output "Built bin/airports.exe"
} finally { Pop-Location }
