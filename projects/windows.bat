@echo off
REM
REM   Set VS vars and invoke nmake   
REM

if DEFINED VSINSTALLDIR GOTO DONE
for /l %%v in (9, 1, 14) do (
    IF EXIST "%PROGRAMFILES(x86)%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" call "%PROGRAMFILES(x86)%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" %1
    IF EXIST "%PROGRAMFILES%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" call "%PROGRAMFILES%\Microsoft Visual Studio %%v.0\VC\vcvarsall.bat" %1
)
:DONE

echo nmake %2 %3 %4 %5 %6 %7 %8 %9
nmake %2 %3 %4 %5 %6 %7 %8 %9
