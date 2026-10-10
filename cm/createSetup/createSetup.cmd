@echo off
REM  Get all files which are relevant for the end user from the Git repository and pack them
REM  into an archive for shipping. See usage message at file end for more details.

REM if "%1" == "" goto LUsage
if "%1" == "--help" goto LUsage
if "%1" == "-h" goto LUsage
if "%1" == "-?" goto LUsage
if "%1" == "/?" goto LUsage
if "%1" == "/h" goto LUsage
if not "%2" == "" goto LUsage

setlocal

REM Check prerequisites for successful run of this script.
if "%TEMP%" == "" (
    echo To run this script the environment variable TEMP needs to be set properly.
    echo TEMP needs to point to a directory with write access for temporarily used files.
    goto :eof
)
REM Availability of Git command line tool.
git --version > nul
if ERRORLEVEL 1 (
    echo The command line tool Git could not be found. Please install the GIT command line
    echo tool and restart this script.
    goto :eof
)

REM Calculate time information and use it to make file and folder names unique.
set now=%date%
set timeInfo=%now:~-2%%now:~-7,2%%now:~-10,2%
set now=%time%
set timeInfo=%timeInfo%%now:~0,2%%now:~3,2%%now:~6,2%%now:~9,2%
set tmpCheckoutFolder=%TEMP%\linNet-%timeInfo%

REM Evaluate the command line. The revision to use can be designated by a commit hash or a
REM echo tag name. Default is commit HEAD.
if /I "%1" == "" (
    set rev=HEAD
)

REM The file name of the aimed setup is tagged with the version information.
set setupFileName=linNet-%rev%.zip

set url=https://github.com/PeterVranken/linNet
REM echo url=%url%, rev=%rev%, setup=%setupFileName%
REM goto :eof

REM Get the wanted version, put it into the TEMP dir. Then go there and run the
REM setup-creating script there: this way, the setup creation process really fits to the
REM wanted version of the file set. Do not use the current working version of this script!

REM goto :LSkipClone
git clone %url% "%tmpCheckoutFolder%" && git -C "%tmpCheckoutFolder%" checkout --detach %rev%
if ERRORLEVEL 1 (
    echo Setup could not be created. Git returned an error. Please inspect the screen output
    echo for details.
    goto :eof
)
goto :LEndSkipClone
:LSkipClone
set tmpCheckoutFolder="C:\tmp\linNet-26100922334519"
:LEndSkipClone
REM echo tmpCheckoutFolder=%tmpCheckoutFolder%

REM Go to the directory in the got file tree where the setup creation will take place.
pushd "%tmpCheckoutFolder%\cm\createSetup" || (echo Directory %tmpCheckoutFolder% doesn't exist & goto :eof)

REM Create a version description.
echo Generating version file from Git information
GitWCRev.exe . template_version.txt version.txt -MU

REM Complete the version description.
echo Archive Compilation Date:>> version.txt
date /T>> version.txt
time /T>> version.txt
echo Source: >> version.txt
if /I not "%rev%" == "HEAD" (
    echo %url%, %rev%>> version.txt
) else (
    echo %url%>> version.txt
)

REM Build the software in all Windows configurations from the got source files. (The
REM binaries for the other systems need to be uploaded in Git and can be copied only.)
REM   An external script is applied although it just contains a few, simple commands: As
REM the other script is part of the exported revision, these commands can become subject to
REM changes in the life cycle of the software (as opposed to this script which must not
REM undergo any essential changes).
call buildAll.cmd

REM Create the dependent documentary files which are not under source control but which need
REM to be in the setup. An external script is applied although it just contains a few,
REM simple commands: As the other script is part of the exported revision, these commands
REM can become subject to changes in the life cycle of the software (as opposed to this
REM script which must not undergo any essential changes).
call makeDoc.cmd

REM Now create the archive by running the file copying script. The executed script is the
REM one got from Git, thus the very revision, which defines the file set required in the
REM exported software version (as opposed to in the current HEAD of the repository).
call copyAndZipFiles.cmd %setupFileName%
if ERRORLEVEL 1 (
    echo Setup could not be created. copyAndZipFiles.cmd returned an error. Please inspect
    echo the screen output for details.
    popd
    goto :eof
)

REM Move target file from temporary directory to final location.
popd
move "%tmpCheckoutFolder%\cm\createSetup\%setupFileName%" .

REM Remove temporarily used files. Suppress related messages.
REM rmdir /S/Q "%tmpCheckoutFolder%" > nul 2> nul

echo *****************************************************************************************
echo Please, find the newly created archive %setupFileName% in your local working
echo directory:
dir *.zip
echo *****************************************************************************************

goto :eof

:LUsage
echo usage: createSetup [commitOrGitTag]
echo   CAUTION: Run this script exclusively from the directory cm\createSetup!
echo   The script creates an archive containing the linNet installation. The bundled files
echo are taken from the project's GIT repository and are addressed by a commit ID or the
echo wanted version's tag. If commitOrGitTag is omitted then the HEAD of the current branch
echo is used.
echo   The created archive linNet-*.zip is found in the current working directory.
