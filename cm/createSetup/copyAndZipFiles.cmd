@echo off
REM Compose file tree, call 7z and create the installation package, which contains only the
REM executable binary files.
REM   Run this batch from the directory cm/createSetup only!

if "%1" == "" goto LUsage
if "%1" == "-h" goto LUsage
if "%1" == "-?" goto LUsage
if "%1" == "/?" goto LUsage
if "%1" == "/h" goto LUsage

setlocal

set tmpFolder=tmp.%1
rmdir /S/Q %tmpFolder% > nul 2> nul

REM Create the directory tree as aimed on the destination system.

mkdir %tmpFolder%
mkdir %tmpFolder%\linNet

REM Go back to the root of the project.
pushd ..\..
set output=cm\createSetup\%tmpFolder%\linNet

REM Global documentation
copy LICENSE %output%
copy readMe.html %output%\linNet.html
copy doc\readMe.forSetup.txt %output%\readMe.txt
copy doc\readMe.forSetup.html %output%\readMe.html
REM The version file needs to be supplied by the caller of this script.
copy cm\createSetup\version.txt %output%
mkdir %output%\doc
copy doc\installation.txt %output%\doc

REM User guide without sources
mkdir %output%\doc
copy doc\userGuide\linNet-userGuide.pdf %output%\doc

REM The binaries
mkdir %output%\bin
xcopy /S bin\*.exe %output%\bin
mkdir %output%\bin\win\DEBUG\private
xcopy /S Octave\private\* %output%\bin\win\DEBUG\private
mkdir %output%\bin\win\PRODUCTION\private
xcopy /S Octave\private\* %output%\bin\win\PRODUCTION\private
REM mkdir %output%\bin\LINUX
REM mkdir %output%\bin\LINUX\DEBUG
REM mkdir %output%\bin\LINUX\DEBUG\private
REM copy bin\LINUX\DEBUG\linNet %output%\bin\LINUX\DEBUG
REM xcopy /S Octave\private\* %output%\bin\LINUX\DEBUG\private
REM mkdir %output%\bin\LINUX\PRODUCTION
REM mkdir %output%\bin\LINUX\PRODUCTION\private
REM copy bin\LINUX\PRODUCTION\linNet %output%\bin\LINUX\PRODUCTION
REM xcopy /S Octave\private\* %output%\bin\LINUX\PRODUCTION\private
attrib +R %output%\bin\* /S
attrib -R %output%\bin\*.exe /S
REM attrib -R %output%\bin\LINUX\DEBUG\linNet
REM attrib -R %output%\bin\LINUX\PRODUCTION\linNet

REM Copy some selected circuit files.
mkdir %output%\circuits
REM copy test\failingTestCases\bandStop.makesOctave3-4-3-win32Crash.cnl %output%\circuits
REM copy test\testCases\octagon.cnl %output%\circuits
REM copy test\testCases\octagon.jpg %output%\circuits
REM copy test\testCases\inverseTF.cnl %output%\circuits
REM copy test\testCases\largeAndSimple.cnl %output%\circuits
REM copy test\testCases\deviceAtSingleNode.cnl %output%\circuits
REM copy test\testCases\recursiveControlledVoltageSrcs.jpg %output%\circuits
REM copy test\testCases\recursiveControlledVoltageSrcs.cnl %output%\circuits
REM copy test\testCases\2poleLP.cnl %output%\circuits
REM copy test\testCases\3poleLP.cnl %output%\circuits
REM copy test\testCases\trickyOP.ckt %output%\circuits
REM copy test\testCases\trickyOP.jpg %output%\circuits
REM copy test\testCases\wire*.cnl %output%\circuits
REM copy test\testCases\wire*.log %output%\circuits
REM copy test\testCases\exampleManual.cnl %output%\circuits
REM copy test\testCases\NgtD.cnl %output%\circuits
REM copy test\testCases\rr.cnl %output%\circuits
xcopy /s/i testCases\*.cnl %output%\circuits\
xcopy /s/i testCases\*.jpg %output%\circuits\
xcopy /s/i testCases\*.svg %output%\circuits\
xcopy /s/i testCases\*.log %output%\circuits\
xcopy /s/i testCases\*.txt %output%\circuits\
xcopy /s/i testCases\*.adoc %output%\circuits\
xcopy /s/i testCases\*.html %output%\circuits\
xcopy /s/i testCases\*.m %output%\circuits\
REM Until all test cases are revised, we better delete those, which are still not in their
REM own, dedicated folder.
del %output%\circuits\*.cnl

REM After completing the aimed target file tree restore the initial working directory.
popd

REM Replace the current archive with the zipped temporary directory tree.
del /F/Q %1 > nul 2> nul
7z a -tzip %1 .\%tmpFolder%\* -r
if ERRORLEVEL 1 (
    echo Error: Files could not be archived completely
    exit /B 1
)

REM Remove the temporary directory tree again.
rmdir /S/Q %tmpFolder%

goto :eof

:LUsage
echo usage: copyAndZipFiles.binaries archive
echo archive designates the name of the aimed archive file to hold the binaries.
goto :eof
