@echo off
set vcvarsallDir="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

call %vcvarsallDir% x64

title MSVC
cd ..\
echo.
echo Run the "build.bat" file to build the project
echo ======================================================================
CMD /K