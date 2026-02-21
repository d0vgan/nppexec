@echo off

set USE_CHM_MANUAL=1
set ARC_EXE=7z.exe

for /f "tokens=1-3 delims=/.- " %%a in ('DATE /T') do set ARC_DATE=%%c%%b%%a
set ARC_SRC_NAME=NppExec%ARC_DATE%_src
set ARC_DLL_NAME=NppExec%ARC_DATE%_dll
set ARC_DLL_NAME_64=NppExec%ARC_DATE%_dll_x64

REM 1. NppExec src archive...

%ARC_EXE% u -tzip %ARC_SRC_NAME%.zip @NppExec_src.txt -mx5
%ARC_EXE% t %ARC_SRC_NAME%.zip

REM 2. NppExec dll archives...

if not exist NppExec\doc mkdir NppExec\doc
if not exist NppExec\doc\NppExec mkdir NppExec\doc\NppExec

sfk copy README.md NppExec\doc\NppExec\README.txt -yes
sfk replace NppExec\doc\NppExec\README.txt /\*/*/ -yes

copy /Y docs\NppExec*.txt NppExec\doc\NppExec\
copy /Y docs\*.css NppExec\doc\NppExec\
if "%USE_CHM_MANUAL%" == "1" (
    copy /Y docs\NppExec_Manual.chm NppExec\doc\NppExec\
    copy /Y docs\fparser*.html NppExec\doc\NppExec\
) else (
    copy /Y docs\*.html NppExec\doc\NppExec\
    if not exist NppExec\doc\NppExec\NppExec_Manual mkdir NppExec\doc\NppExec\NppExec_Manual
    if not exist NppExec\doc\NppExec\NppExec_Manual\images mkdir NppExec\doc\NppExec\NppExec_Manual\images
    copy /Y docs\NppExec_Manual\*.css  NppExec\doc\NppExec\NppExec_Manual\
    copy /Y docs\NppExec_Manual\*.html NppExec\doc\NppExec\NppExec_Manual\
    copy /Y docs\NppExec_Manual\*.js   NppExec\doc\NppExec\NppExec_Manual\
    copy /Y docs\NppExec_Manual\images\*.* NppExec\doc\NppExec\NppExec_Manual\images\
)

if not exist NppExec\Release\NppExec.dll goto no_dll_file_exit
cd NppExec\Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME%.zip NppExec.dll NppExec\*.h -mx5
cd ..
%ARC_EXE% u -tzip ..\%ARC_DLL_NAME%.zip doc\NppExec\** -mx5
cd ..
%ARC_EXE% t %ARC_DLL_NAME%.zip

:no_dll_file_exit

if not exist NppExec\x64\Release\NppExec.dll goto no_dll_file_x64_exit
cd NppExec\x64\Release
%ARC_EXE% u -tzip ..\..\..\%ARC_DLL_NAME_64%.zip NppExec.dll -mx5
cd ..\..\Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME_64%.zip NppExec\*.h -mx5
cd ..
%ARC_EXE% u -tzip ..\%ARC_DLL_NAME_64%.zip doc\NppExec\** -mx5
cd ..
%ARC_EXE% t %ARC_DLL_NAME_64%.zip

:no_dll_file_x64_exit

sfk deltree NppExec\doc -yes
