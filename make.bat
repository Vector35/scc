@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set PATH=%~dp0\buildenv\msys\bin;%PATH%
make.exe %*
