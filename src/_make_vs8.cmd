@echo off
setlocal
set PATH=c:\cygwin64\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"
make.exe %*
