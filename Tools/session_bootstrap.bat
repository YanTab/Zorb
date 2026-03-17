@echo off
setlocal

set SCRIPT_DIR=%~dp0
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%session_bootstrap.ps1" %*
set EXIT_CODE=%ERRORLEVEL%

if not "%EXIT_CODE%"=="0" (
  echo.
  echo [session] Bootstrap failed with code %EXIT_CODE%.
) else (
  echo.
  echo [session] Bootstrap finished successfully.
)

exit /b %EXIT_CODE%
