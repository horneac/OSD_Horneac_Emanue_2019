@echo off

setlocal

pushd %CD%
cd %~dp0%

call paths.cmd

if [%PATH_TO_VM_DISK%]==[__EMPTY__] goto fail

set PATH_TO_BINARIES=%1
set FULL_PATH_TO_DESTINATION=[MountLetter]:\%2

echo "    --------------------------- IMPORTANT ------------------------------"
echo "    1. Please manually mount (with write permissions) the disk of HAL9000 found at: %PATH_TO_VM_DISK%"
echo "    2. Copy the executables found at %PATH_TO_BINARIES% to %FULL_PATH_TO_DESTINATION%"
echo "    3. Disconnect the mounted disk. Now you can run HAL9000 with updated user applications!"
echo "    ------------------------------ END ---------------------------------"

goto end

:fail

echo "Failed, not all paths are valid!"
exit /b 1

:end

:: --- reload initial current directory, always exit with status != 0 so that build repeats this step ---
popd
exit /b 1