@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

rem execute_tests.pl E:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\threads e:\workspace\Projects\MiniHV\logs\w81x64.log "c:\Program Files (x86)\VMware\VMware VIX" "E:\workspace\VMware Virtual Machines\Windows 8.1 x64\Windows 8.1 x64.vmx" E:\PXE\Tests.module /run

set SOLUTION_DIRECTORY=%1
set TESTS_BASE_FOLDER=%2
set MODULE_NAME=%3
set PREFIX=%4
set SUFFIX=%5
set TIMEOUT=%6

cd %TESTS_BASE_FOLDER%\..

call perl execute_tests.pl %TESTS_BASE_FOLDER% %PATH_TO_LOG_FILE% %PATH_TO_VIX_TOOLS% %PATH_TO_VM_FILE% %PXE_PATH%\%MODULE_NAME% %PREFIX% %SUFFIX% %TIMEOUT%


:end
:: --- reload initial current directory ---
popd
exit /b %ERRORLEVEL%