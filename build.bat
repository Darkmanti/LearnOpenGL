@echo off

REM main path and other options
set L_FilePath="%cd%\OpenGL\unity_build.cpp"

set MainDir="%cd%"
set GameName="LearnOpenGL"
set L_NameExe="OpenGL"
set L_MapFile="OpenGL"

set SharedFilesPath="%cd%\shared"
set glfwPath="%cd%\shared\vendors\glfw"
set gladPath="%cd%\shared\vendors\glad"
set stbPath="%cd%\shared\vendors\stb"
set glmPath="%cd%\shared\vendors\glm"

set IncludesPath=/I%SharedFilesPath% /I%glfwPath% /I%gladPath% /I%stbPath% /I%glmPath%

REM setting build directory
set BuildDir=build\
set BuildDirRe=%BuildDir%release\
set BuildDirDe=%BuildDir%debug\

REM libs
set L_AddLib=user32.lib gdi32.lib shell32.lib vcruntime.lib msvcrt.lib "%glfwPath%\glfw3.lib"
set L_AddDll=%L_RES_FilePath%

REM Launcher keys
set L_ReKeys=/permissive- /nologo %IncludesPath% /FC /Oi /GR- /GL /MT /EHa /WX /W4 /Zc:wchar_t /wd4100 /wd4189 /wd4201 /O2 /Fe%L_NameExe%
set L_DeKeys=/permissive- /nologo %IncludesPath% /FC /Oi /GR- /MTd /EHa /WX /W4 /Zc:wchar_t /wd4100 /wd4189 /wd4201 -Fm%L_MapFile% /Z7 /Od /Fe%L_NameExe%
set L_ReLinkKeys=/link /NOLOGO /OPT:REF /SUBSYSTEM:CONSOLE /NODEFAULTLIB:LIBCMT
set L_DeLinkKeys=/link /NOLOGO /OPT:REF /SUBSYSTEM:CONSOLE /NODEFAULTLIB:LIBCMTD

REM write "/D" and without spaces write a definitions
set L_ReDef=
set L_DeDef=

REM parsing command line arguments

IF /I "%1" EQU "clr" goto Clear

IF NOT EXIST %BuildDir% mkdir %BuildDir%
IF NOT EXIST %BuildDirRe% mkdir %BuildDirRe%
IF NOT EXIST %BuildDirDe% mkdir %BuildDirDe%

IF "%1" EQU "/?" call :Help
IF "%1" EQU "?" call :Help
IF "%1" EQU "help" call :Help
IF /I "%1" EQU "re" call :Release
IF /I "%1" EQU "de" call :Debug
IF /I "%1" EQU "cpy" call :AllResourcesCopy
IF /I "%1" EQU "link" call :MakeSymLink

goto Exit

:Release
pushd %BuildDirRe%
cl %L_ReDef% %L_ReKeys% %L_FilePath% %L_ReLinkKeys% %L_AddLib% %L_AddDll%
popd
exit /b

:Debug
pushd %BuildDirDe%
cl %L_DeDef% %L_DeKeys% %L_FilePath% %L_DeLinkKeys% %L_AddLib% %L_AddDll%
popd
exit /b

:Clear
rmdir /S/Q %BuildDir%
goto Exit

:AllResourcesCopy
xcopy /I/Y/E/Q %MainDir%\res %BuildDir%res
xcopy /I/Y/E/Q %MainDir%\res %ShipGameDir%\res
exit /b

:MakeSymLink
mklink /D %BuildDir%res %MainDir%\res
exit /b

:Help
echo "de" - debug build without any optimization and with debug info
echo "re" - release build with all optimization and without debug info
echo "clr" cleaning all build directory
echo "link" make symlink to original resources directory
echo "cpy" copying resources to build directory and ship directory
exit /b

:Exit