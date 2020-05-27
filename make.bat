@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
set PATH=%~dp0\buildenv\msys\bin;%PATH%
make.exe %*
