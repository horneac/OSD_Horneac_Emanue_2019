@echo off

setlocal

pushd %CD%
cd %~dp0%


set prj_name=%1
set prj_dir=%2
set prj_platform=%3
set prj_config=%4
set sol_name=%5
set target_name=%6
set target_ext=%7
set pxe_upload=%8
set full_project_path=""

if [%FILE_DESTINATION%]==[__EMPTY__] goto no_share
	set share_path=%FILE_DESTINATION%\%sol_name%
:no_share

echo "Project name: [%prj_name%]"
echo "Project dir: [%prj_dir%]"
echo "Platform name: [%prj_platform%]"
echo "Configuration name: [%prj_config%]"
echo "Solution name: [%sol_name%]"
echo "Target name: [%target_name%]"
echo "Target extension: [%target_ext%]"

set int_platform=x64

set full_project_path=%prj_dir%..\bin\%prj_platform%\%prj_config%\%prj_name%

echo "Full project path: [%full_project_path%]"

:: Saving scripts
echo About to save scripts

if NOT EXIST %prj_dir%\script goto move_scripts_done
copy %prj_dir%\script\*.* %full_share_path% >nul 2>nul

:move_scripts_done

:: PXE upload section
if [%pxe_upload%]==[] goto pre_end

if [%PXE_PATH%]==[__EMPTY__] goto no_pxe
if [%PXE_PATH%]==[] goto no_pxe
xcopy /F /Y %full_project_path%\%target_name%%target_ext% %PXE_PATH%\
:no_pxe

:pre_end
echo --INFO: build done!
goto end


:end
:: --- reload initial current directory ---
popd
exit /b %ERRORLEVEL%