
@echo off
@rem Running cmake on Windows example for VS2019

cls

@SETLOCAL
@rem Zohar's cmake path; change it to yours.
@SET CMAKE_EXEC="c:\prog\cmake-gui\bin\cmake.exe"
@IF EXIST %CMAKE_EXEC% GOTO START
@GOTO ERROR

:START
mkdir build
cd build

%CMAKE_EXEC% ^
-Wno-dev -G"Visual Studio 16" -A"x64" ../work

cd ..
@GOTO EOF

@:ERROR
@echo ERROR: CMake not found.

:EOF

pause

