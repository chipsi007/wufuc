@echo off

net session >nul 2>&1 || (
    echo This batch script requires administrator privileges. Right-click on
    echo %~nx0 and select "Run as administrator".
    goto :die
)

if not exist "%~dp0patches\" (
    echo Patches folder not found! Make sure you extracted all the files from
    echo the .zip, and the `patches` folder is in the same location as 
    echo %~nx0, and then try again.
    goto :die
)

echo Checking system requirements...

:check_bitness
wmic /output:stdout os get osarchitecture | find "64-bit" >nul && (
    set "WINDOWS_BITNESS=x64"
    set "XDELTA3_EXE=%~dp0xdelta3-3.0.11-x86_64.exe"
    goto :check_version
)
wmic /output:stdout os get osarchitecture | find "32-bit" >nul && (
    set "WINDOWS_BITNESS=x86"
    set "XDELTA3_EXE=%~dp0xdelta3-3.0.11-i686.exe"
    goto :check_version
)
goto :unsupported

:check_version
echo.
wmic /output:stdout os get version | findstr "^6\.1\." >nul && (
    set "WINDOWS_VER=6.1"
    set "SUPPORTED_HOTFIXES=KB4015549 KB4015546 KB4012218"
    echo Detected supported operating system: Windows 7 %WINDOWS_BITNESS%
    goto :check_hotfix
)
wmic /output:stdout os get version | findstr "^6\.3\." >nul && (
    set "WINDOWS_VER=8.1"
    set "SUPPORTED_HOTFIXES=KB4015550 KB4015547 KB4012219"
    echo Detected supported operating system: Windows 8.1 %WINDOWS_BITNESS%
    goto :check_hotfix
)

:unsupported
echo Detected that you are using an unsupported version of Windows.
echo This patch only works on the following versions:
echo - Windows 7 (x64 and x86)
echo - Windows 8.1 (x64 and x86)
echo - Windows Server 2008 R2
echo - Windows Server 2012 R2
goto :die

:check_hotfix
echo.
for %%a in (%SUPPORTED_HOTFIXES%) do (
    wmic /output:stdout qfe get hotfixid | find "%%a" >nul && (
        set "INSTALLED_HOTFIX=%%a"
        echo Detected supported update installed: %%a
        goto :confirmation
    )
)

echo Detected that no supported updates are installed! If you
echo are getting unsupported hardware errors in Windows Update, please
echo create an issue and post a list of any recently installed
echo Windows Updates that could have caused it, and I will try
echo to make a new patch for the update as soon as I can!
echo https://github.com/zeffy/kb4012218-19/issues
goto :die

:confirmation
echo.
echo This patch is for Windows 7 and 8.1 (x64 and x86), as well as
echo Windows Server 2008 R2 and Server 2012 R2, if you have another version
echo of Windows, please close this window immediately.
echo.
echo I take no responsibility if you somehow ruin your PC with this script.
echo.
set /p CONTINUE=Enter 'Y' if you understand, and still want to continue: 
if /i "%CONTINUE%" NEQ "Y" goto :cancel

:ask
echo.
echo Would you like to install the patch or uninstall it?
echo.
echo 1. Install
echo 2. Uninstall
echo.
set /p CHOICE=Enter your choice: 
if /i "%CHOICE%" EQU "1" (
    set "PATCH_TYPE=patch"
    goto :begin
)
if /i "%CHOICE%" EQU "2" (
    set "PATCH_TYPE=unpatch"
    goto :begin
)
echo Invalid choice, please try again...
goto :ask

:begin
echo.
set "DELTA_FILE=%~dp0patches\Windows%WINDOWS_VER%-%INSTALLED_HOTFIX%-%WINDOWS_BITNESS%-%PATCH_TYPE%.xdelta"
set "SYSTEM32_DIR=%windir%\System32"
set "WUAUENG_DLL=%SYSTEM32_DIR%\wuaueng.dll"

for /f "delims=" %%a in ('wmic os get localdatetime ^| find "."') do set dt=%%a
set "TIMESTAMP=%dt:~0,4%-%dt:~4,2%-%dt:~6,2%_%dt:~8,2%-%dt:~10,2%-%dt:~12,2%"
set "BACKUP_FILE=%WUAUENG_DLL%_%TIMESTAMP%_%random%.bak"
set "ACL_TEMP_FILE=%temp%\wuaueng.dll_acl_%TIMESTAMP%_%random%.txt"

net stop wuauserv

takeown /F "%WUAUENG_DLL%" /A
echo Backing up wuaueng.dll file permissions to `%ACL_TEMP_FILE%`...
icacls "%WUAUENG_DLL%" /save "%ACL_TEMP_FILE%"
icacls "%WUAUENG_DLL%" /grant Administrators:F
move "%WUAUENG_DLL%" "%BACKUP_FILE%"

"%XDELTA3_EXE%" -d -s "%BACKUP_FILE%" "%DELTA_FILE%" "%WUAUENG_DLL%"
if errorlevel 1 (
    set "THERE_WAS_AN_ERROR=%errorlevel%"
    move /Y "%BACKUP_FILE%" "%WUAUENG_DLL%"
)

icacls "%WUAUENG_DLL%" /setowner "NT Service\TrustedInstaller"
icacls "%SYSTEM32_DIR%" /restore "%ACL_TEMP_FILE%"

net start wuauserv

if defined THERE_WAS_AN_ERROR (
    echo There was an error while %PATCH_TYPE%ing. Nothing has been modified. 
    echo If you didn't screw with the script or anything like that and this
    echo error was unexpected, please create an issue on my GitHub here: 
    echo https://github.com/zeffy/kb4012218-19/issues
) else (
    echo Successfully %PATCH_TYPE%ed!
    echo If you want to revert the changes that have been made for whatever
    echo reason, you can run this script again. Or, you can also manually
    echo restore the backup file located at `%BACKUP_FILE%`, by renaming it
    echo back to `wuaueng.dll` and restoring the owner and permissions on the
    echo file.
)

:die
echo.
echo Press any key to close . . .
pause >nul
exit

:cancel
echo.
echo Canceled by user input, press any key to close . . .
pause >nul
exit
