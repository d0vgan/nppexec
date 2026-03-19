@echo off
set VC_ROOT=%ProgramFiles(x86)%\Microsoft Visual Studio\2017

cd "%~dp0..\NppExec"

if exist "%VC_ROOT%\Professional\MSBuild\15.0\Bin\MSBuild.exe" goto UseVcProfessional
if exist "%VC_ROOT%\Community\MSBuild\15.0\Bin\MSBuild.exe" goto UseVcCommunity

echo Oops! Neither Professional nor Community edition of VS2017 found.
goto End

:UseVcProfessional
"%VC_ROOT%\Professional\MSBuild\15.0\Bin\MSBuild.exe" %*
goto End

:UseVcCommunity
"%VC_ROOT%\Community\MSBuild\15.0\Bin\MSBuild.exe" %*
goto End

:End
