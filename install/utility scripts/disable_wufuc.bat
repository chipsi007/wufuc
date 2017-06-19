@echo off
title wufuc utility - disable task
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
set "wufuc_dll=%~dp0..\wufuc32.dll"
goto :disable

:is_x64
set "wufuc_dll=%~dp0..\wufuc64.dll"

:disable
set "wufuc_task=wufuc.{72EEE38B-9997-42BD-85D3-2DD96DA17307}"
rundll32 "%wufuc_dll%",Rundll32Unload
net start Schedule
schtasks /Change /TN "%wufuc_task%" /DISABLE

echo.
echo Disabled wufuc! You will still be able to check for updates until you restart.

:die
echo.
pause
exit
