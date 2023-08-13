@echo off

set ARC_EXE=7z.exe
for /f "tokens=1-3 delims=/.- " %%a in ('DATE /T') do set ARC_DATE=%%c%%b%%a
set ARC_SRC_NAME=NppExec%ARC_DATE%_src
set ARC_DLL_NAME=NppExec%ARC_DATE%_dll
set ARC_DLL_NAME_64=NppExec%ARC_DATE%_dll_x64

REM 1. NppExec src archive...

%ARC_EXE% u -tzip %ARC_SRC_NAME%.zip @NppExec_src.txt -mx5
%ARC_EXE% t %ARC_SRC_NAME%.zip

REM 2. NppExec dll archives...

if not exist doc\NppExec mkdir doc\NppExec

sfk copy README.md doc\NppExec\README.txt -yes
sfk replace doc\NppExec\README.txt /\*/*/ -yes

copy /Y doc\NppExec*.txt doc\NppExec\
copy /Y doc\NppExec_Manual.chm doc\NppExec\
copy /Y doc\fparser.html doc\NppExec\
copy /Y doc\*.css doc\NppExec\

if not exist NppExec\Release\NppExec.dll goto no_dll_file_exit
cd NppExec\Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME%.zip NppExec.dll NppExec\*.h -mx5
cd ..\..
%ARC_EXE% u -tzip %ARC_DLL_NAME%.zip doc\NppExec\README.txt doc\NppExec\NppExec*.txt doc\NppExec\NppExec*.chm doc\NppExec\*.html doc\NppExec\*.css -mx5
%ARC_EXE% t %ARC_DLL_NAME%.zip

:no_dll_file_exit

if not exist NppExec\x64\Release\NppExec.dll goto no_dll_file_x64_exit
cd NppExec\x64\Release
%ARC_EXE% u -tzip ..\..\..\%ARC_DLL_NAME_64%.zip NppExec.dll -mx5
cd ..\..\Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME_64%.zip NppExec\*.h -mx5
cd ..\..
%ARC_EXE% u -tzip %ARC_DLL_NAME_64%.zip doc\NppExec\README.txt doc\NppExec\NppExec*.txt doc\NppExec\NppExec*.chm doc\NppExec\*.html doc\NppExec\*.css -mx5
%ARC_EXE% t %ARC_DLL_NAME_64%.zip

:no_dll_file_x64_exit

sfk deltree doc\NppExec -yes
