# PC Build Verification Record (Windows CLI)

**Date**: 2026-05-29  
**Author**: Copilot + user verification  
**Target**: TimeGrapher (Qt 6.11.1, MinGW 64-bit)  
**Scope**: Reproduce Qt Creator Release build/run using CLI only on Windows

---

## 1. Environment

| Item | Version / Path |
|------|----------------|
| OS | Windows |
| Qt | `C:\Qt\6.11.1\mingw_64` |
| MinGW | `C:\Qt\Tools\mingw1310_64` |
| CMake | `C:\Qt\Tools\CMake_64\bin\cmake.exe` |
| Generator | MinGW Makefiles |
| Source Root | `D:\sw_architect\project\2026-3-sw-architect-studio-project\src` |

---

## 2. Goal

Reproduce the same Release build and run behavior in Windows CLI that already succeeded in Qt Creator.

---

## 3. What Happened (Actual Verification Log)

### 3-1. Existing Qt Creator Release Build Directory Rebuild Attempt

Target build directory:

`D:\sw_architect\project\2026-3-sw-architect-studio-project\src\build\Desktop_Qt_6_11_1_MinGW_64_bit-Release`

Command:

```powershell
cmake --build .
```

Observed result:

- Build stopped at:
  - `TimeGrapher_autogen/mocs_compilation.cpp.obj`
- `mingw32-make` reported `Error 1`, but compiler diagnostic text was not shown.

### 3-2. Root Cause Investigation

1. Retried with `cmake --build . --verbose`, but detailed compiler diagnostics were still not visible.
2. Used standalone `g++` checks to validate C++ front-end execution.
3. Direct `cc1plus.exe` execution result:

```text
exit=-1073741515
```

Interpretation:

- `-1073741515` (`0xC0000135`) usually indicates missing required DLLs.
- In this case, MinGW runtime DLL paths were missing from PATH in the CLI session, causing compilation to fail.

### 3-3. Why Qt Creator Worked but CLI Failed

Qt Creator runs builds in an environment where toolchain paths (especially MinGW/Qt bin) are already injected.
In a normal PowerShell session, the same command can fail if those PATH entries are not present.

---

## 4. Final Working CLI Procedure (Windows)

The following procedure successfully reproduced clean Release build and run from CLI.

### 4-1. Set PATH for current shell session

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;" + $env:Path
```

### 4-2. Configure (clean CLI release dir)

```powershell
Set-Location "D:\sw_architect\project\2026-3-sw-architect-studio-project\src"

if (Test-Path .\build\cli-release) {
    Remove-Item .\build\cli-release -Recurse -Force
}

C:\Qt\Tools\CMake_64\bin\cmake.exe `
  -S . `
  -B build\cli-release `
  -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" `
  -DCMAKE_C_COMPILER="C:/Qt/Tools/mingw1310_64/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" `
  -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe"
```

Configuration result:

- `-- The CXX compiler identification is GNU 13.1.0`
- `-- Configuring done`
- `-- Generating done`

### 4-3. Build (Release)

```powershell
C:\Qt\Tools\CMake_64\bin\cmake.exe --build .\build\cli-release -j8
```

Build result:

- All translation units progressed and executable generated.
- Verification:

```powershell
Test-Path "D:\sw_architect\project\2026-3-sw-architect-studio-project\src\build\cli-release\TimeGrapher.exe"
```

- Output: `True`

### 4-4. Run (Release binary)

```powershell
Set-Location "D:\sw_architect\project\2026-3-sw-architect-studio-project\src\build\cli-release"
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;" + $env:Path

$p = Start-Process -FilePath .\TimeGrapher.exe -PassThru
"started_pid=$($p.Id)"
Start-Sleep -Milliseconds 700
if (Get-Process -Id $p.Id -ErrorAction SilentlyContinue) { "running=true" } else { "running=false" }
```

Run result:

- `running=true` was confirmed.
- Application shutdown was then confirmed by user manual close.

---

## 5. One-shot Command Set (Reusable)

Run the following commands in order in PowerShell to reproduce the same process:

```powershell
Set-Location "D:\sw_architect\project\2026-3-sw-architect-studio-project\src"
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;" + $env:Path

if (Test-Path .\build\cli-release) {
    Remove-Item .\build\cli-release -Recurse -Force
}

C:\Qt\Tools\CMake_64\bin\cmake.exe -S . -B build\cli-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" -DCMAKE_C_COMPILER="C:/Qt/Tools/mingw1310_64/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe"

C:\Qt\Tools\CMake_64\bin\cmake.exe --build .\build\cli-release -j8

Set-Location .\build\cli-release
Start-Process .\TimeGrapher.exe
```

---

## 6. Troubleshooting Notes

1. Symptom: `mingw32-make` only shows `Error 1` without clear compile diagnostics.  
Likely cause: compiler front-end (`cc1plus`) failed to start due to missing runtime DLL in PATH.

2. Quick check:

```powershell
C:\Qt\Tools\mingw1310_64\libexec\gcc\x86_64-w64-mingw32\13.1.0\cc1plus.exe --version
"exit=$LASTEXITCODE"
```

If non-zero (especially `-1073741515`), fix PATH first.

3. Always set PATH in the same shell where configure/build/run is executed.

---

## 7. Conclusion

Release build and run were successfully reproduced in Windows CLI with behavior matching Qt Creator.

The two key requirements were:

1. Inject MinGW/Qt bin paths into PATH in the current CLI session first.
2. Configure and build using a clean CLI-specific build directory (`build/cli-release`).
