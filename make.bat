@echo off
call "c:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
set PATH=%~dp0\buildenv\msys\bin;%PATH%
make.exe %*
