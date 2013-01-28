@echo off
REM
REM   Set VS vars and invoke nmake   
REM

if DEFINED VSINSTALLDIR GOTO DONE
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" call "%PROGRAMFILES%\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"
if DEFINED VSINSTALLDIR GOTO DONE
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" call "%PROGRAMFILES%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
if DEFINED VSINSTALLDIR GOTO DONE
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" call "%PROGRAMFILES%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:DONE

echo nmake -nologo %1 %2 %3 %4 %5 %6 %7 %8 %9
nmake -nologo %1 %2 %3 %4 %5 %6 %7 %8 %9
