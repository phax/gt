@echo off
setlocal
set PATH=c:\cygwin64\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"
make.exe GT_VC=14 %*
