::
::
::

set FILE_DESTINATION=__EMPTY__
set SYMBOL_PATH=__EMPTY__
set DBG_BASE=__EMPTY__
set SYMSTORE_PATH=__EMPTY__
set PXE_PATH=__EMPTY__
set BOCHS_PATH=__EMPTY__
set PATH_TO_VM_DISK=__EMPTY__
set PATH_TO_VM_TOOLS=__EMPTY__
set VOL_MOUNT_LETTER=__EMPTY__

if _%COMPUTERNAME%_==_AGURZOU-DEV-PC_ goto config_AGURZOU-DEV
if _%COMPUTERNAME%_==_ALEX-PC_ goto config_ALEX-PC
if _%COMPUTERNAME%_==_AGURZOU-LPT_ goto config_AGURZOU-LPT

echo.
echo ERROR: host '%COMPUTERNAME%' not recognized by PATHS.CMD, build FAILED!
echo You need to add a host dedicated section to PATHS.CMD and retry build.
echo.
goto end

:config_ALEX-PC
set PXE_PATH=E:\PXE
set PATH_TO_VM_DISK="e:\workspace\VMware Virtual Machines\Windows 8.1 x64\myAppDisk.vmdk"
set PATH_TO_VM_TOOLS="c:\Program Files (x86)\VMware\VMware Virtual Disk Development Kit\bin\"
set VOL_MOUNT_LETTER=Q:
set PATH_TO_VIX_TOOLS="c:\Program Files (x86)\VMware\VMware VIX"
set PATH_TO_LOG_FILE="e:\workspace\Projects\MiniHV\logs\w81x64.log"
set PATH_TO_VM_FILE="e:\workspace\VMware Virtual Machines\Windows 8.1 x64\Windows 8.1 x64.vmx"

goto end

:config_AGURZOU-DEV

set FILE_DESTINATION=__EMPTY__

set SYMBOL_PATH=e:\Symbols

set DBG_BASE=c:\Program Files (x86)\Windows Kits\10\Debuggers\x64

set SYMSTORE_PATH="%DBG_BASE%\symstore.exe"

set PXE_PATH=__EMPTY__

set PXE_PATH2=pxe-GRZ

set BOCHS_PATH="c:\Program Files (x86)\Bochs-2.6.8\"

goto end

:config_AGURZOU-LPT

set FILE_DESTINATION=__EMPTY__

set SYMBOL_PATH=C:\Symbols

set DBG_BASE=c:\Program Files (x86)\Windows Kits\10\Debuggers\x64

set SYMSTORE_PATH="%DBG_BASE%\symstore.exe"

set PXE_PATH=c:\workspace\PXE\

set PXE_PATH2=pxe-GRZ

set BOCHS_PATH="c:\Program Files (x86)\Bochs-2.6.8\"

goto end

:end