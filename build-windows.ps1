# Builds the Iron Ring player on Windows and packs a release zip (docs/building.md).
#
#   .\build-windows.ps1            Visual Studio 2022 or 2026, then Dist\IronRing-<version>-windows-x64.zip
#   .\build-windows.ps1 -Test      also run the tests
#   .\build-windows.ps1 -Run       build, then start the game
#   .\build-windows.ps1 -MinGW     use the MinGW-w64 g++ on PATH (MSYS2 UCRT64, CLion) instead
#
# Or double-click build-windows.bat. Needs Visual Studio 2022 or 2026 with "Desktop development
# with C++" (it brings CMake), or MinGW-w64 with cmake and ninja on PATH, and git.
param([switch]$Test, [switch]$Run, [switch]$MinGW)
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

function Fail($message) { Write-Host $message -ForegroundColor Red; exit 1 }
function Invoke-Checked { & $args[0] @($args | Select-Object -Skip 1); if ($LASTEXITCODE -ne 0) { Fail "$($args[0]) failed ($LASTEXITCODE)" } }

# CMake: on PATH, or the copy inside Visual Studio.
$cmake = (Get-Command cmake -ErrorAction SilentlyContinue).Source
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not $cmake -and (Test-Path $vswhere)) {
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $candidate = Join-Path $vs "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if ($vs -and (Test-Path $candidate)) { $cmake = $candidate }
}
if (-not $cmake) { Fail "CMake not found. Install Visual Studio 2022 or 2026 with 'Desktop development with C++', or CMake from cmake.org." }
$ctest = Join-Path (Split-Path $cmake) "ctest.exe"

if ($MinGW) {
    foreach ($tool in "g++", "ninja") {
        if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) { Fail "$tool not on PATH (MSYS2: pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ninja)." }
    }
    $env:CC = "gcc"; $env:CXX = "g++"
    $configure = @("--preset", "release"); $build = "Build\release"; $config = @()
} else {
    # Visual Studio 2022 (17) or 2026 (18), whichever is newest.
    $major = if (Test-Path $vswhere) { (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion) -replace '\..*$', '' }
    switch ($major) {
        "17" { $configure = @("--preset", "vs2022"); $build = "Build\vs2022" }
        "18" { $configure = @("-S", ".", "-B", "Build\vs2026", "-G", "Visual Studio 18 2026", "-A", "x64"); $build = "Build\vs2026" }
        default { Fail "Visual Studio 2022 or newer with 'Desktop development with C++' not found (or pass -MinGW)." }
    }
    $config = @("--config", "Release")
}

$commit = (git rev-parse --short HEAD 2>$null)
Write-Host "== Configuring ($build) =="
Invoke-Checked $cmake @configure
Write-Host "== Building at $commit =="
Invoke-Checked $cmake --build $build @config --parallel
if ($Test) {
    Write-Host "== Testing =="
    Invoke-Checked $ctest --test-dir $build @("-C", "Release") --output-on-failure
}

$playerDir = Join-Path $build "Player\Release"
if ($Run) { & (Join-Path $playerDir "IronRing.exe") @args; exit $LASTEXITCODE }

$fullVersion = (Select-String -Path (Join-Path $build "CMakeCache.txt") -Pattern '^CMAKE_PROJECT_VERSION:STATIC=(.*)$').Matches[0].Groups[1].Value
$version = $fullVersion -replace '^(\d+\.\d+)\.0$', '$1'   # 1.0.0 -> 1.0 (Tools/Build/common.sh)
$name = "IronRing-$version-windows-x64"
$stage = Join-Path "Dist" $name
Write-Host "== Packing $name =="
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item -Recurse (Join-Path $playerDir "*") $stage
Copy-Item LICENSE (Join-Path $stage "LICENSE.txt")
Copy-Item THIRD_PARTY_NOTICES.md $stage
$readme = (Get-Content -Raw Tools\Build\README-windows.txt) + (Get-Content -Raw Tools\Build\README-common.txt)
Set-Content -NoNewline (Join-Path $stage "README.txt") ($readme -replace '@VERSION@', $version -replace '@START@', 'IronRing.exe')
Set-Content -NoNewline (Join-Path $stage ".itch.toml") ((Get-Content -Raw Tools\Build\itch.toml.in) -replace '@START@', 'IronRing.exe')
$zip = Join-Path "Dist" "$name.zip"
if (Test-Path $zip) { Remove-Item $zip }
Compress-Archive -Path $stage -DestinationPath $zip
Write-Host "== Ready: $zip =="
Write-Host "   Unzip anywhere and run $name\IronRing.exe."
