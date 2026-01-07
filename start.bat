@echo off
setlocal

echo ==========================================
echo       NiceTop_OS Windows Setup
echo ==========================================

REM Check if WSL is installed
wsl --status >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] WSL is not installed or not running.
    echo Please install WSL by opening PowerShell as Administrator and running:
    echo     wsl --install
    echo Then restart your computer and run this script again.
    pause
    exit /b 1
)

echo [INFO] WSL detected.
echo [INFO] Launching start.sh in WSL...

REM Convert current path to WSL path syntax and run start.sh
REM The "." argument tells wsl to start in the current directory.
wsl -d Ubuntu -e bash -c "./start.sh"

if %errorlevel% neq 0 (
    echo.
    echo [WARNING] If the above failed, you might not have the 'Ubuntu' distro installed.
    echo You can try running 'wsl --install -d Ubuntu' or just 'wsl' to set up a default distro.
    echo.
    echo Press any key to exit...
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Script finished.
pause
