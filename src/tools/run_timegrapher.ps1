<#
 .SYNOPSIS
   TimeGrapher build & run script (Windows / MinGW)

 .USAGE
   .\run_timegrapher.ps1            # build + run
   .\run_timegrapher.ps1 build      # build only
   .\run_timegrapher.ps1 run        # run only (skip build)
   .\run_timegrapher.ps1 rebuild    # clean build dir + build + run
#>
param(
    [ValidateSet('all','build','run','rebuild')]
    [string]$Mode = 'all'
)
$ErrorActionPreference = 'Stop'

# ── 설정 (경로 바뀌면 여기만 수정) ────────────────────────────
$QtPrefix    = 'C:\Qt\6.11.1\mingw_64'
$MingwBin    = 'C:\Qt\Tools\mingw1310_64\bin'
$CMakeBin    = 'C:\Qt\Tools\CMake_64\bin'
$Jobs        = 4

# src 디렉토리 = 이 스크립트(src\tools\)의 한 단계 위 (실행 위치 무관)
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcDir    = Split-Path -Parent $ScriptDir          # src\  (CMakeLists.txt 위치)
$BuildDir  = Join-Path $SrcDir 'build'
$Bin       = Join-Path $BuildDir 'TimeGrapher.exe'

# ── PATH 설정 (mingw, cmake) ──────────────────────────────────
$env:Path = "$MingwBin;$CMakeBin;$QtPrefix\bin;" + $env:Path

function Do-Build {
    Write-Host "[build] SrcDir=$SrcDir"
    if (-not (Test-Path $BuildDir)) { New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null }

    # 캐시가 옛 경로를 가리키면 stale -> 재configure
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
