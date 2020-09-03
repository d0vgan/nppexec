call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd .\NppExec
msbuild NppExec_VC2017.sln /t:Clean /p:Configuration=Release;Platform=x64
msbuild NppExec_VC2017.sln /t:Build /p:Configuration=Release;Platform=x64
