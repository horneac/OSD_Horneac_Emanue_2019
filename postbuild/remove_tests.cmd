@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

set MODULE_NAME=%1

call perl truncate_file.pl %PXE_PATH%\%MODULE_NAME%

:end
:: --- reload initial current directory ---
popd
exit /b %ERRORLEVEL%