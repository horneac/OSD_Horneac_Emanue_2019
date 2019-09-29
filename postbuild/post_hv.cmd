@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

set path_to_boot_loader="%2..\loader\bin\boot.bin"

echo Will copy files on share and PXE
call copy_files.cmd %1 %2 %3 %4 %5 %6 %7 1

popd