@echo off

net session >nul 2>&1 || (
    echo This batch script requires administrator privileges. Right-click on
    echo %~nx0 and select "Run as administrator".
    goto :die
)

echo Checking system requirements...
echo.

wmic /output:stdout qfe get hotfixid | find "KB4012219" >nul || (
    echo Detected that update KB4012219 is not installed, please verify that
    echo you are trying to install the right patch file and try again.
    goto :die
)

wmic /output:stdout os get osarchitecture | find "32-bit" >nul || (
    echo Detected that you are not running 32-bit Windows, please verify
    echo that you are trying to install the right patch file and try again.
    goto :die
)

echo This patch is *ONLY* for Windows 8.1 x86 wuaueng.dll,
echo if you have another version of Windows 7 or 8.1, please close this 
echo window and use the appropriate patch file for your OS. I take no 
echo responsibility if you somehow wreck your system.
echo.
set /p CONTINUE=Press 'Y' if you understand, and still want to continue: 
echo.
if /i "%CONTINUE%" NEQ "Y" goto :cancel

:ask
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
set "DELTA_FILE=%~dp0w81x86-wuaueng.dll-%PATCH_TYPE%.xdelta"
set "XDELTA3_EXE=%~dp0xdelta3-3.0.11-i686.exe"
set "SYSTEM32_DIR=%windir%\System32"
set "WUAUENG_DLL=%SYSTEM32_DIR%\wuaueng.dll"

for /f "delims=" %%a in ('wmic os get localdatetime ^| find "."') do set dt=%%a
set "TIMESTAMP=%dt:~0,4%-%dt:~4,2%-%dt:~6,2%_%dt:~8,2%-%dt:~10,2%-%dt:~12,2%"
set "BACKUP_FILE=%WUAUENG_DLL%.bak_%TIMESTAMP%_%random%"
set "PERMISSIONS_TEMP_FILE=%temp%\wuaueng.dll_acl_%TIMESTAMP%_%random%.txt"

net stop wuauserv

takeown /F "%WUAUENG_DLL%" /A
echo Backing up wuaueng.dll file permissions...
icacls "%WUAUENG_DLL%" /save "%PERMISSIONS_TEMP_FILE%"
icacls "%WUAUENG_DLL%" /grant Administrators:F
move "%WUAUENG_DLL%" "%BACKUP_FILE%"
"%XDELTA3_EXE%" -d -s "%BACKUP_FILE%" "%DELTA_FILE%" "%WUAUENG_DLL%"
if errorlevel 1 (
    set "THERE_WAS_AN_ERROR=%errorlevel%"
    move /Y "%BACKUP_FILE%" "%WUAUENG_DLL%"
)
icacls "%WUAUENG_DLL%" /setowner "NT Service\TrustedInstaller"
icacls "%SYSTEM32_DIR%" /restore "%PERMISSIONS_TEMP_FILE%"

net start wuauserv

if defined THERE_WAS_AN_ERROR (
    echo There was an error while %PATCH_TYPE%ing. Nothing has been modified. 
    echo If you didn't screw with the script or anything like that and this
    echo error was unexpected, please create an issue on my GitHub here: 
    echo https://github.com/zeffy/kb4012218-kb4012219/issues
) else (
    echo Successfully %PATCH_TYPE%ed!
    echo If you want to reverse the changes that have been made for whatever
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
