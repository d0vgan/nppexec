@echo off

echo 1. Parsing the Help topics ...
python NppExec_parse_help_topics.py -i 2 -a -s
if "%ERRORLEVEL%"=="0" goto Compile_Chm
echo Error invoking NppExec_parse_help_topics.py
goto End

:Compile_Chm
echo.
echo 2. Compiling NppExec_Manual.chm ...
cd docs
cd NppExec_Manual
if not exist "C:\Program Files (x86)\HTML Help Workshop\hhc.exe" goto Compile_Chm_Alt
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" NppExec_Manual.hhp
goto Copy_Chm
:Compile_Chm_Alt
if not exist "C:\Program Files\HTML Help Workshop\hhc.exe" goto Error_Invoking_hhc
"C:\Program Files\HTML Help Workshop\hhc.exe" NppExec_Manual.hhp
goto Copy_Chm
:Error_Invoking_hhc
echo Error invoking HTML Help Workshop
goto End

:Copy_Chm
echo.
echo 3. Copying NppExec_Manual.chm ...
copy /Y NppExec_Manual.chm ..\NppExec_Manual.chm
if "%ERRORLEVEL%"=="0" goto OK
echo Error copying NppExec_Manual.chm
goto End

:OK
echo.
echo Done!

:End
