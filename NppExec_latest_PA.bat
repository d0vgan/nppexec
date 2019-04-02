REM Creates an archive with PluginsAdmin-specific folder structure
REM (starting from Notepad++ 7.6.x)

for /f "tokens=1-3 delims=/.- " %%a in ('DATE /T') do set ARC_DATE=%%c%%b%%a
if not exist %TEMP%\%ARC_DATE% mkdir %TEMP%\%ARC_DATE%
set TMP_DIR=%TEMP%\%ARC_DATE%\NppExec
if exist %TMP_DIR% del /S /Q %TMP_DIR%
if exist %TMP_DIR% rmdir /S /Q %TMP_DIR%
mkdir %TMP_DIR%

mkdir %TMP_DIR%\doc
mkdir %TMP_DIR%\doc\NppExec
mkdir %TMP_DIR%\NppExec
mkdir %TMP_DIR%\NppExec\NppExec

copy README.md %TMP_DIR%\doc\NppExec\README.txt

cd NppExec

copy doc\NppExec\NppExec*.txt %TMP_DIR%\doc\NppExec\
copy doc\NppExec\NppExec_Manual\NppExec_Manual.chm %TMP_DIR%\doc\NppExec\
copy doc\NppExec\*.html %TMP_DIR%\doc\NppExec\
copy doc\NppExec\*.css %TMP_DIR%\doc\NppExec\
copy doc\NppExec\*.png %TMP_DIR%\doc\NppExec\

copy Release\NppExec\*.h %TMP_DIR%\NppExec\NppExec\

set ARC_EXE=7z.exe

set ARC_DLL_NAME=NppExec%ARC_DATE%_dll_PA
set ARC_DLL_NAME_64=NppExec%ARC_DATE%_dll_x64_PA

:check_dll_32bit
if not exist Release\NppExec.dll goto check_dll_64bit
copy Release\NppExec.dll %TMP_DIR%\NppExec\

if exist ..\%ARC_DLL_NAME%.zip del /Q ..\%ARC_DLL_NAME%.zip
%ARC_EXE% a -tzip ..\%ARC_DLL_NAME%.zip %TMP_DIR%\* -mx5
%ARC_EXE% t ..\%ARC_DLL_NAME%.zip

:check_dll_64bit
if not exist x64\Release\NppExec.dll goto finish
copy x64\Release\NppExec.dll %TMP_DIR%\NppExec\ /Y

if exist ..\%ARC_DLL_NAME_64%.zip del /Q ..\%ARC_DLL_NAME_64%.zip
%ARC_EXE% a -tzip ..\%ARC_DLL_NAME_64%.zip %TMP_DIR%\* -mx5
%ARC_EXE% t ..\%ARC_DLL_NAME_64%.zip

:finish
if exist %TMP_DIR% del /S /Q %TMP_DIR%
if exist %TMP_DIR% rmdir /S /Q %TMP_DIR%
