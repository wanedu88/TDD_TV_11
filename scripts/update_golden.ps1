# Regenerate Golden Master approved files (TextTest-style approval update).
# Usage (from repo root):
#   .\scripts\update_golden.ps1

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $root

if (-not (Test-Path build)) {
    cmake -S . -B build
}
cmake --build build --config Release

$env:TV_UPDATE_GOLDEN = "1"
& .\build\TVControllerGoldenTest.exe
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Golden files updated under test/golden/approved/"
Write-Host "Review diff, then commit *.approved.txt"
