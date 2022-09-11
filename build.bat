@echo off

SET premakePath="vendor/premake/premake5.exe"
SET solutionFile="ScalGame.sln"
SET debugPath="bin/Debug-windows-x86_64"
SET releasePath="bin/Release-windows-x86_64"

IF "%~1" == "" GOTO PrintHelp
if "%~1" == "gen_vscode" GOTO GenerateVSCode
IF "%~1" == "gen" GOTO Generate
IF "%~1" == "build" GOTO Build
IF "%~1" == "run" GOTO Run

GOTO Done

:PrintHelp
echo "Use: build.bat [gen, build, run]"
GOTO Done

:GenerateVSCode
%premakePath% gmake2
%premakePath% vscode
GOTO Done

:Generate
echo "Generating %solutionFile% with vs2022"
%premakePath% vs2022
GOTO Done

:Build
echo "Building %solutionFile% in %debugPath%..."
msbuild -m %solutionFile%
GOTO Done

:Run
echo "Building %solutionFile% in %debugPath%..."
msbuild -m %solutionFile%
echo "Running %solutionFile%..."
%debugPath%/Game/Game.exe
GOTO Done

:Done
