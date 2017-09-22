@echo off
title wufuc uninstaller
:: Copyright (C) 2017 zeffy

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

echo Copyright ^(C^) 2017 zeffy
echo This program comes with ABSOLUTELY NO WARRANTY.
echo This is free software, and you are welcome to redistribute it
echo under certain conditions; see COPYING.txt for details.
echo.

fltmc >nul 2>&1 || (
    echo This batch script requires administrator privileges. Right-click on
    echo %~nx0 and select "Run as administrator".
    goto :die
)

if /I "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    goto :is_x64
) else (
    if /I "%PROCESSOR_ARCHITEW6432%"=="AMD64" (
        goto :is_x64
    )
    if /I "%PROCESSOR_ARCHITECTURE%"=="x86" (
        goto :is_x86
    )
)
goto :die

:is_x86
set "wufuc_dll=%~dp0wufuc32.dll"
goto :get_ver

:is_x64
set "wufuc_dll=%~dp0wufuc64.dll"

:get_ver
for /f "tokens=*" %%i in ('wmic /output:stdout datafile where "name='%wufuc_dll:\=\\%'" get Version /value ^| find "="') do set "%%i"
title wufuc uninstaller - v%Version%

:loop
if [%1]==[] goto :confirmation
if /I "%1"=="/UNATTENDED" goto :uninstall
shift
goto :loop

:confirmation
set /p CONTINUE=Enter 'Y' if you want to uninstall wufuc: 
if /I not "%CONTINUE%"=="Y" goto :cancel
echo.

:uninstall
set "wufuc_task=wufuc.{72EEE38B-9997-42BD-85D3-2DD96DA17307}"
rundll32 "%wufuc_dll%",Rundll32Unload
net start Schedule
schtasks /Delete /TN "%wufuc_task%" /F

echo.
echo Unloaded and uninstalled wufuc. :^(

:die
echo.
echo Press any key to exit...
pause >nul
exit

:cancel
echo.
echo Canceled by user, press any key to exit...
pause >nul
exit
