@echo off
set PATH=C:\Program Files (x86)\Windows Kits\8.1\bin\x86
set OPTIONS= /Gfa /nologo
rem fxc /?
fxc %OPTIONS% /D VERTEX_SHADER=1 /T vs_5_0 /E main Test.hlsl
fxc %OPTIONS% /D PIXEL_SHADER=1  /T ps_5_0 /E main Test.hlsl

pause