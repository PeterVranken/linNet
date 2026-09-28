@echo off
REM See usage message for help. See
REM http://en.wikibooks.org/wiki/Windows_Programming/Programming_CMD for syntax of batch
REM scripts.
goto LStart
:LUsage
echo usage: makeDoc
echo   Generate the (dependent) documentation files, which are part of the setup but which 
echo are not under source control. This script is part of the setup generation process. The
echo start directory is cm\createSetup.
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

echo Compiling the documentation
echo pushd "%tmpCheckoutFolder%\doc\userGuide"
pushd "%tmpCheckoutFolder%\doc\userGuide"
make
popd
pushd "%tmpCheckoutFolder%\doc\doxygen"
doxygen
popd




