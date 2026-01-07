@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo       NiceTop_OS Windows Setup
echo ==========================================
echo.

REM Check if WSL is installed
echo [INFO] Checking WSL installation...
wsl --status >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] WSL is not installed.
    echo.
    echo Would you like to install WSL now? This requires administrator privileges.
    echo Press Y to install WSL, or N to exit.
    choice /C YN /N /M "[Y/N]: "
    if errorlevel 2 goto :NoWSL
    if errorlevel 1 goto :InstallWSL
)

:CheckDistro
echo [INFO] WSL is installed. Checking for Ubuntu distribution...
wsl -l -q | findstr -i "Ubuntu" >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARNING] Ubuntu distribution not found in WSL.
    echo Installing Ubuntu...
    wsl --install -d Ubuntu
    echo.
    echo [INFO] Ubuntu installed. Please complete the Ubuntu setup and run this script again.
    pause
    exit /b 0
)

echo [OK] Ubuntu distribution found.
echo.

REM Get current directory and convert to WSL path
set "CURRENT_DIR=%CD%"
set "WSL_PATH=%CURRENT_DIR:\=/%"
set "WSL_PATH=%WSL_PATH:C:=/mnt/c%"
set "WSL_PATH=%WSL_PATH:D:=/mnt/d%"
set "WSL_PATH=%WSL_PATH:E:=/mnt/e%"

echo [INFO] Running NiceTop_OS build in WSL...
echo.

REM Run the start.sh script in WSL
wsl -d Ubuntu bash -c "cd '%WSL_PATH%' && chmod +x start.sh && ./start.sh"

if %errorlevel% neq 0 (
    echo.
    echo [ERROR] Build or execution failed.
    echo Please check the error messages above.
    pause
    exit /b 1
)

echo.
echo [SUCCESS] NiceTop_OS execution completed.
pause
exit /b 0

:InstallWSL
echo.
echo [INFO] Installing WSL...
echo This requires administrator privileges. You may see a UAC prompt.
echo.
powershell -Command "Start-Process wsl -ArgumentList '--install' -Verb RunAs -Wait"
echo.
echo [INFO] WSL installation completed.
echo Please restart your computer and run this script again.
pause
exit /b 0

:NoWSL
echo.
echo [INFO] Exiting without installing WSL.
echo.
echo To use NiceTop_OS on Windows, you need WSL (Windows Subsystem for Linux).
echo You can install it manually by running this command in PowerShell (as Administrator):
echo     wsl --install
echo.
pause
exit /b 1
