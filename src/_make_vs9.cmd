@echo off
setlocal
set PATH=c:\cygwin64\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
set win
::C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include\
make.exe %*
