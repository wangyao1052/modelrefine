@echo off
setlocal

set "ROOT=%~dp0."
set "OCCT_DIR=D:\workspace\0-code\yi3d\3rdParty\bundles\occt"
set "INSTALL_PREFIX=D:\workspace\0-code\yi3d\3rdParty\bundles\ModelRefine"
set "PATH=%OCCT_DIR%\bin;%PATH%"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1

for %%C in (Debug Release) do (
    echo === Configure %%C ===
    cmake -S "%ROOT%" -B "%ROOT%\build-%%C" -G Ninja -DCMAKE_BUILD_TYPE=%%C -DOpenCASCADE_DIR="%OCCT_DIR%/cmake"
    if errorlevel 1 exit /b 1

    echo === Build %%C ===
    cmake --build "%ROOT%\build-%%C"
    if errorlevel 1 exit /b 1

    echo === Test %%C ===
    ctest --test-dir "%ROOT%\build-%%C" --output-on-failure
    if errorlevel 1 exit /b 1

    echo === Install %%C ===
    cmake --install "%ROOT%\build-%%C" --prefix "%INSTALL_PREFIX%"
    if errorlevel 1 exit /b 1
)

echo === Done ===
