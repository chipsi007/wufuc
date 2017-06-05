@echo off
title install wufuc ^(repair wuaueng.dll^) - v0.6
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

:confirmation
echo You may want to use this script if you previously modified wuaueng.dll
echo with "aio-wuaueng.dll-patch.bat" or by other means.
echo.
echo This will run the sfc utility and it will restore any changes that were made.
echo.

set /p CONTINUE=Enter 'Y' if you want to repair wuaueng.dll: 
if /I not "%CONTINUE%"=="Y" goto :cancel

sfc /SCANFILE="%systemroot%\System32\wuaueng.dll"

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
