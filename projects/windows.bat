@echo off
REM
REM   Set VS vars and run a command
REM

SET PATH=\perl64\bin;\perl\bin;%PATH%

set VS=
if DEFINED VSINSTALLDIR GOTO DONE
for /l %%v in (14, -1, 9) do (
    set VS=%%v
    IF EXIST "%PROGRAMFILES(x86)%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" call "%PROGRAMFILES(x86)%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" %1
    IF EXIST "%PROGRAMFILES(x86)%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" goto :done
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" call "%PROGRAMFILES%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" %1
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" goto :done
)
VS=
:DONE

echo Using Visual Studio %VS%.0
echo %2 %3 %4 %5 %6 %7 %8 %9
%2 %3 %4 %5 %6 %7 %8 %9
