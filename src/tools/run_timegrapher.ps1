<#
 .SYNOPSIS
   TimeGrapher build & run script (Windows / MinGW)

 .USAGE
   .\run_timegrapher.ps1                 # build + run (no logging)
   .\run_timegrapher.ps1 build           # build only
   .\run_timegrapher.ps1 run             # run only (skip build)
   .\run_timegrapher.ps1 rebuild         # clean build dir + build + run
   .\run_timegrapher.ps1 build --log     # build with performance logging
   .\run_timegrapher.ps1 all --log       # build + run with logging
 (--log uses a separate build dir: build-log/)
#>
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$CmdArgs
)
$ErrorActionPreference = 'Stop'

# ── Parse args: mode + optional --log flag (any order) ────────
$Mode    = 'all'
$Logging = $false
foreach ($a in $CmdArgs) {
    if     ($a -in @('build','run','rebuild','all')) { $Mode = $a }
    elseif ($a -eq '--log' -or $a -eq 'logging')     { $Logging = $true }
    else { Write-Host "[warn] ignoring unknown arg: $a" }
}

# ── Config (edit here if paths change) ────────────────────────
$QtPrefix    = 'C:\Qt\6.11.1\mingw_64'
$MingwBin    = 'C:\Qt\Tools\mingw1310_64\bin'
$CMakeBin    = 'C:\Qt\Tools\CMake_64\bin'
$Jobs        = 4

# src dir = one level up from this script (src\tools\), independent of cwd
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcDir    = Split-Path -Parent $ScriptDir          # src\  (location of CMakeLists.txt)
$BuildName = if ($Logging) { 'build-log' } else { 'build' }
$BuildDir  = Join-Path $SrcDir $BuildName
$Bin       = Join-Path $BuildDir 'TimeGrapher.exe'
$LoggingFlag = if ($Logging) { 'ON' } else { 'OFF' }

# ── PATH setup (mingw, cmake) ─────────────────────────────────
$env:Path = "$MingwBin;$CMakeBin;$QtPrefix\bin;" + $env:Path

function Do-Build {
    Write-Host "[build] SrcDir=$SrcDir  ENABLE_LOGGING=$LoggingFlag  ($BuildName)"
    if (-not (Test-Path $BuildDir)) { New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null }

    # If cache points to an old path (stale), reconfigure
    $cache = Join-Path $BuildDir 'CMakeCache.txt'
    $cachedSrc = ''
    if (Test-Path $cache) {
        $line = Select-String -Path $cache -Pattern '^CMAKE_HOME_DIRECTORY' -ErrorAction SilentlyContinue
        if ($line) { $cachedSrc = ($line.Line -split '=',2)[1] }
    }
    $srcCmake = ($SrcDir -replace '\\','/')

    if ((-not (Test-Path $cache)) -or ($cachedSrc -and $cachedSrc -ne $srcCmake)) {
        if ($cachedSrc -and $cachedSrc -ne $srcCmake) {
            Write-Host "[build] stale cache ($cachedSrc != $srcCmake) -> reconfigure"
            Remove-Item $cache -Force
        }
        Write-Host "[build] configuring..."
        cmake -S $SrcDir -B $BuildDir -G "MinGW Makefiles" `
              -DCMAKE_BUILD_TYPE=Release `
              -DENABLE_LOGGING=$LoggingFlag `
              -DCMAKE_PREFIX_PATH="$QtPrefix"
    }

    Write-Host "[build] compiling (-j$Jobs)..."
    cmake --build $BuildDir -j $Jobs
    Write-Host "[build] done -> $Bin"
}

function Do-Run {
    if (-not (Test-Path $Bin)) {
        Write-Host "[run] binary not found: $Bin"
        Write-Host "[run] run build first."
        exit 1
    }
    Write-Host "[run] launching TimeGrapher..."
    & $Bin
}

switch ($Mode) {
    'build'   { Do-Build }
    'run'     { Do-Run }
    'rebuild' {
        Write-Host "[rebuild] removing $BuildDir"
        if (Test-Path $BuildDir) { Remove-Item $BuildDir -Recurse -Force }
        Do-Build; Do-Run
    }
    'all'     { Do-Build; Do-Run }
}
