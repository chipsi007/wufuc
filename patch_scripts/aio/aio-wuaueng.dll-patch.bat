@echo off
:: Copyright (C) 2017 zeffy <https://github.com/zeffy>

:: This program is free software: you can redistribute it and/or modify
:: it under the terms of the GNU General Public License as published by
:: the Free Software Foundation, either version 3 of the License, or
:: (at your option) any later version.

:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: GNU General Public License for more details.

:: You should have received a copy of the GNU General Public License
:: along with this program.  If not, see <http://www.gnu.org/licenses/>.

net session >nul 2>&1 || (
    echo This batch script requires administrator privileges. Right-click on
    echo %~nx0 and select "Run as administrator".
    goto :die
)

if not exist "%~dp0patches\" (
    echo Patches folder not found! Make sure you extracted all the files from
    echo the .zip, and the 'patches' folder is in the same location
    echo as %~nx0, and try again.
    goto :die
)

echo Checking system requirements...

:check_bitness
if /I "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    goto :is_x64
) else (
    if /I "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
        goto :is_x64
    )
    if /I "%PROCESSOR_ARCHITECTURE%"=="x86" (
        set "WINDOWS_BITNESS=x86"
        set "XDELTA3_EXE=%~dp0xdelta3-3.0.11-i686.exe"
        goto :find_xdelta
    )
)
goto :unsupported

:is_x64
set "WINDOWS_BITNESS=x64"
set "XDELTA3_EXE=%~dp0xdelta3-3.0.11-x86_64.exe"

:find_xdelta
echo.
if not exist "%XDELTA3_EXE%" (
    echo xdelta3 %WINDOWS_BITNESS% not found! Make sure you extracted all the files from
    echo the .zip. Make sure both of the xdelta3 .exe files are in the same location
    echo as %~nx0, and try again.
    goto :die
)

wmic /output:stdout os get version | findstr "^6\.1\." >nul && (
    set "WINDOWS_VER=6.1"
    set "SUPPORTED_HOTFIXES=KB4015552 KB4015549 KB4015546 KB4012218"
    echo Detected supported operating system: Windows 7 %WINDOWS_BITNESS%
    goto :check_hotfix
)
wmic /output:stdout os get version | findstr "^6\.3\." >nul && (
    set "WINDOWS_VER=8.1"
    set "SUPPORTED_HOTFIXES=KB4015553 KB4015550 KB4015547 KB4012219"
    echo Detected supported operating system: Windows 8.1 %WINDOWS_BITNESS%
    goto :check_hotfix
)

:unsupported
echo Detected that you are using an unsupported operating system.
echo.
echo This patch only works on the following versions of Windows:
echo.
echo - Windows 7 (x64 and x86)
echo - Windows 8.1 (x64 and x86)
echo - Windows Server 2008 R2 (reported as Windows 7 x64)
echo - Windows Server 2012 R2 (reported as Windows 8.1 x64)
goto :die

:check_hotfix
echo.
for %%a in (%SUPPORTED_HOTFIXES%) do (
    wmic /output:stdout qfe get hotfixid | find "%%a" >nul && (
        set "INSTALLED_HOTFIX=%%a"
        echo Detected installed supported update: %%a
        goto :confirmation
    )
)

echo Detected that no supported updates are installed! If you
echo are getting unsupported hardware errors in Windows Update, please
echo create an issue and post a list of any recently installed
echo Windows updates that could have introduced it, and I will try
echo to help you out.
echo https://github.com/zeffy/kb4012218-19/issues
goto :die

:confirmation
echo.
echo This patch only works on the following versions of Windows:
echo.
echo - Windows 7 (x64 and x86)
echo - Windows 8.1 (x64 and x86)
echo - Windows Server 2008 R2 (reported as Windows 7 x64)
echo - Windows Server 2012 R2 (reported as Windows 8.1 x64)
echo.
echo If you have another version of Windows, please close this window immediately.
echo.
echo By continuing, you acknowledge that you want to modify wuaueng.dll.
echo.
echo I take no responsibility if you somehow ruin your PC with this script.
echo.
set /p CONTINUE=Enter 'Y' if you understand, and still want to continue: 
if /I not "%CONTINUE%"=="Y" goto :cancel

:ask
echo.
echo Would you like to install the patch or uninstall it?
echo.
echo 1. Install
echo 2. Uninstall
echo.
set /p CHOICE=Enter your choice: 
if "%CHOICE%"=="1" (
    set "PATCH_TYPE=patch"
    goto :begin
)
if "%CHOICE%"=="2" (
    set "PATCH_TYPE=unpatch"
    goto :begin
)
echo Invalid choice, please try again...
goto :ask

:begin
echo.
set "DELTA_FILE=%~dp0patches\Windows%WINDOWS_VER%-%INSTALLED_HOTFIX%-%WINDOWS_BITNESS%-%PATCH_TYPE%.xdelta"
set "SYSTEM32_DIR=%systemroot%\System32"
set "WUAUENG_DLL=%SYSTEM32_DIR%\wuaueng.dll"

for /f "delims=" %%a in ('wmic os get localdatetime ^| find "."') do set dt=%%a
set "TIMESTAMP=%dt:~0,4%-%dt:~4,2%-%dt:~6,2%_%dt:~8,2%-%dt:~10,2%-%dt:~12,2%"
set "BACKUP_FILE=%WUAUENG_DLL%_%TIMESTAMP%_%random%_before-%PATCH_TYPE%.bak"
set "ACL_TEMP_FILE=%temp%\wuaueng.dll_acl_%TIMESTAMP%_%random%.txt"

net stop wuauserv

takeown /F "%WUAUENG_DLL%" /A
icacls "%WUAUENG_DLL%" /save "%ACL_TEMP_FILE%"

:: Administrators group SID
icacls "%WUAUENG_DLL%" /grant *S-1-5-32-544:F
move "%WUAUENG_DLL%" "%BACKUP_FILE%"

"%XDELTA3_EXE%" -d -s "%BACKUP_FILE%" "%DELTA_FILE%" "%WUAUENG_DLL%"
if errorlevel 1 (
    set "THERE_WAS_AN_ERROR=%errorlevel%"
    move /Y "%BACKUP_FILE%" "%WUAUENG_DLL%"
)

:: "NT Service\TrustedInstaller" SID
icacls "%WUAUENG_DLL%" /setowner *S-1-5-80-956008885-3418522649-1831038044-1853292631-2271478464
icacls "%SYSTEM32_DIR%" /restore "%ACL_TEMP_FILE%"

net start wuauserv

echo.
if defined THERE_WAS_AN_ERROR (
    echo There was an error while %PATCH_TYPE%ing. Nothing has been modified. 
    echo If you didn't screw with the script or anything like that and this
    echo error was unexpected, please create an issue and include the output
    echo of this window in your post.
    echo https://github.com/zeffy/kb4012218-19/issues
) else (
    echo Successfully %PATCH_TYPE%ed!
    echo If you want to revert the changes that have been made for whatever
    echo reason, you can run this script again and pick the other option.
    echo.
    echo You can also manually restore the backup file located at 
    echo '%BACKUP_FILE%'
    echo by renaming it back to wuaueng.dll, changing the owner back to 
    echo "NT Service\TrustedInstaller",  and restoring the original permissions from 
    echo '%ACL_TEMP_FILE%'. 
    echo However, be careful to make sure you only restore the backup that is the same 
    echo version as the current wuaueng.dll, or you could corrupt the WinSxS component
    echo store.
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
