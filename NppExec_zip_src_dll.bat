@echo off

sfk copy README.md NppExec\doc\NppExec\README.txt -yes
sfk copy NppExec\doc\NppExec\NppExec_Manual\NppExec_Manual.chm NppExec\doc\NppExec\ -yes

set ARC_EXE=7z.exe
for /f "tokens=1-3 delims=/.- " %%a in ('DATE /T') do set ARC_DATE=%%c%%b%%a
set ARC_SRC_NAME=NppExec%ARC_DATE%_src
set ARC_DLL_NAME=NppExec%ARC_DATE%_dll
set ARC_DLL_NAME_64=NppExec%ARC_DATE%_dll_x64

%ARC_EXE% u -tzip %ARC_SRC_NAME%.zip @NppExec_src.txt -mx5
%ARC_EXE% t %ARC_SRC_NAME%.zip

if not exist NppExec\Release\NppExec.dll goto no_dll_file_exit
cd NppExec\Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME%.zip NppExec.dll NppExec\*.h -mx5
cd ..
%ARC_EXE% u -tzip ..\%ARC_DLL_NAME%.zip doc\NppExec\README.txt doc\NppExec\NppExec*.txt doc\NppExec\NppExec*.chm doc\NppExec\*.html doc\NppExec\*.css doc\NppExec\*.png -mx5
cd ..
%ARC_EXE% t %ARC_DLL_NAME%.zip

:no_dll_file_exit

if not exist NppExec\x64\Release\NppExec.dll goto no_dll_file_x64_exit
cd NppExec\x64\Release
%ARC_EXE% u -tzip ..\..\..\%ARC_DLL_NAME_64%.zip NppExec.dll -mx5
cd ..\..
%ARC_EXE% u -tzip ..\%ARC_DLL_NAME_64%.zip doc\NppExec\README.txt doc\NppExec\NppExec*.txt doc\NppExec\NppExec*.chm doc\NppExec\*.html doc\NppExec\*.css doc\NppExec\*.png -mx5
cd Release
%ARC_EXE% u -tzip ..\..\%ARC_DLL_NAME_64%.zip NppExec\*.h -mx5
cd ..\..
%ARC_EXE% t %ARC_DLL_NAME_64%.zip

:no_dll_file_x64_exit
