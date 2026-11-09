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
copy doc\linNet.forSetup.html %output%\linNet.html
copy doc\readMe.forSetup.txt %output%\readMe.txt
copy doc\readMe.forSetup.html %output%\readMe.html
REM The version file needs to be supplied by the caller of this script.
copy cm\createSetup\version.txt %output%
mkdir %output%\doc
copy doc\installation.txt %output%\doc
copy doc\userGuide\sources\templates\linNet.reg %output%\doc

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
REM copy test\circuits\octagon.cnl %output%\circuits
REM copy test\circuits\octagon.jpg %output%\circuits
REM copy test\circuits\inverseTF.cnl %output%\circuits
REM copy test\circuits\largeAndSimple.cnl %output%\circuits
REM copy test\circuits\deviceAtSingleNode.cnl %output%\circuits
REM copy test\circuits\recursiveControlledVoltageSrcs.jpg %output%\circuits
REM copy test\circuits\recursiveControlledVoltageSrcs.cnl %output%\circuits
REM copy test\circuits\2poleLP.cnl %output%\circuits
REM copy test\circuits\3poleLP.cnl %output%\circuits
REM copy test\circuits\trickyOP.ckt %output%\circuits
REM copy test\circuits\trickyOP.jpg %output%\circuits
REM copy test\circuits\wire*.cnl %output%\circuits
REM copy test\circuits\wire*.log %output%\circuits
REM copy test\circuits\exampleManual.cnl %output%\circuits
REM copy test\circuits\NgtD.cnl %output%\circuits
REM copy test\circuits\rr.cnl %output%\circuits
xcopy /s/i circuits\*.cnl %output%\circuits\
xcopy /s/i circuits\*.pdf %output%\circuits\
xcopy /s/i circuits\*.jpg %output%\circuits\
xcopy /s/i circuits\*.svg %output%\circuits\
xcopy /s/i circuits\*.log %output%\circuits\
xcopy /s/i circuits\*.txt %output%\circuits\
xcopy /s/i circuits\*.adoc %output%\circuits\
xcopy /s/i circuits\*.html %output%\circuits\
xcopy /s/i circuits\*.m %output%\circuits\
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
