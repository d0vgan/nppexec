@echo off
set VC_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022

cd "%~dp0..\NppExec"

if exist "%VC_ROOT%\Professional\MSBuild\Current\Bin\MSBuild.exe" goto UseVcProfessional
if exist "%VC_ROOT%\Community\MSBuild\Current\Bin\MSBuild.exe" goto UseVcCommunity

echo Oops! Neither Professional nor Community edition of VS2022 found.
goto End

:UseVcProfessional
"%VC_ROOT%\Professional\MSBuild\Current\Bin\MSBuild.exe" %*
goto End

:UseVcCommunity
"%VC_ROOT%\Community\MSBuild\Current\Bin\MSBuild.exe" %*
goto End

:End
