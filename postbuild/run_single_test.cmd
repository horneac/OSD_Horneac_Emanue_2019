@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

rem execute_tests.pl E:\WORKSPACE\PROJECTS\VS2015\HAL9000\TESTS\threads e:\workspace\Projects\MiniHV\logs\w81x64.log "c:\Program Files (x86)\VMware\VMware VIX" "E:\workspace\VMware Virtual Machines\Windows 8.1 x64\Windows 8.1 x64.vmx" E:\PXE\Tests.module /run

set TESTS_BASE_FOLDER=%CD%\..\..\tests\%1
set PREFIX=__EMPTY__
set SUFFIX=__EMPTY__

cd %TESTS_BASE_FOLDER%\..

if /I %1==threads (
    set PREFIX=run
    set SUFFIX=16
) else (
    set PREFIX=proctest
    set SUFFIX=
)

call perl execute_tests.pl %TESTS_BASE_FOLDER%\%2 %PATH_TO_LOG_FILE% %PATH_TO_VIX_TOOLS% %PATH_TO_VM_FILE% %PXE_PATH%\Tests.module /%PREFIX% /%SUFFIX% 90


:end
:: --- reload initial current directory ---
popd
exit /b %ERRORLEVEL%