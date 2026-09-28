@echo off
REM See usage message for help. See
REM http://en.wikibooks.org/wiki/Windows_Programming/Programming_CMD for syntax of batch
REM scripts.
goto LStart
:LUsage
echo usage: buildALl
echo   Build the executable files as far as possible (i.e., for the system this script is
echo running on). These files are part of the setup but they are not under source control.
echo This script is part of the setup generation process. The start directory is
echo cm\createSetup.
goto :eof

REM 
REM Copyright (c) 2014-2026, Peter Vranken (mailto:Peter_Vranken@Yahoo.de)
REM 

:LStart
::if /i "%1" == "" goto LUsage
if /i "%1" == "-h" goto LUsage
if /i "%1" == "-?" goto LUsage
if /i "%1" == "/?" goto LUsage
if /i "%1" == "/h" goto LUsage
:: Limit the allowed number of parameters.
if not "%1" == "" goto LUsage

setlocal

REM set CC_BIN=c:\ProgramFiles\mingw-i686-8.1.0-release-win32-dwarf-rt_v6-rev0\mingw32\bin
REM set CC_BIN=c:\ProgramFiles\mingw-w64-x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin
set CC_BIN=C:\ProgramFiles\mingw-w64-gcc-13.2.0-x64\mingw64\bin
REM set CC_BIN=C:\ProgramFiles\mingw-w64-gcc-mcf_20260501_16.1.0_x64-msvcrt\mingw64\bin
REM set CC_BIN=C:\ProgramFiles\mingw-w64-gcc-mcf_20260501_16.1.0_x86-msvcrt\mingw32\bin
set UNIX_TOOLS_BIN=C:\Programme\Git\usr\bin

echo pushd "%tmpCheckoutFolder%"
pushd "%tmpCheckoutFolder%"
echo Compiling the software in all Windows configurations
make -sO CONFIG=DEBUG rebuild
make -sO CONFIG=PRODUCTION rebuild
popd
    exit /B 1
