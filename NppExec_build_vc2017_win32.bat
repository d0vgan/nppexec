@echo off

set VC_ROOT=%ProgramFiles(x86)%\Microsoft Visual Studio\2017

if exist "%VC_ROOT%\Professional\VC\Auxiliary\Build\vcvarsall.bat" goto UseVcProfessional
if exist "%VC_ROOT%\Community\VC\Auxiliary\Build\vcvarsall.bat" goto UseVcCommunity

goto ErrorNoVcVarsAll

:UseVcProfessional
call "%VC_ROOT%\Professional\VC\Auxiliary\Build\vcvarsall.bat" x86
goto Building

:UseVcCommunity
call "%VC_ROOT%\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
goto Building

:Building
cd .\NppExec
msbuild NppExec_VC2017.sln /t:Clean /p:Configuration=Release;Platform=Win32 /m:2
msbuild NppExec_VC2017.sln /t:Build /p:Configuration=Release;Platform=Win32 /m:2
goto End

:ErrorNoVcVarsAll
echo ERROR: Could not find "vcvarsall.bat"
goto End

:End
