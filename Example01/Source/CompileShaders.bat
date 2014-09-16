@echo off
set PATH=C:\Program Files (x86)\Windows Kits\8.1\bin\x86
set OPTIONS= /Gfa
rem fxc /?
fxc %OPTIONS% /T vs_5_0 /E main /Fo Test.vso Test.vs
fxc %OPTIONS% /T ps_5_0 /E main /Fo Test.pso Test.ps

pause