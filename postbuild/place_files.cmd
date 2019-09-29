@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

call copy_files.cmd %1 %2 %3 %4 %5 %6 %7 %8

popd